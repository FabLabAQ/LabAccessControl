/*
 * Lab Access System
 * (c) 2018 Luca Anastasio
 * anastasio.lu@gmail.com
 * www.fablaquila.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "constants.h"
#include <ArduinoOTA.h>
#include <SPI.h>
#include <TrueRandom.h>
#include <Bounce2.h>
// #include <FS.h>
#include <LittleFS.h>
#include "Card_DB.h"

#include <HTTPSRedirect.h>
#include "logging.h"
#include <time.h>
#include <sys/time.h>
#include <LabAccessOTA.h>
#include <LabAccessWiFi.h>
#include "MifareUltralight.h"
#include "config.h"
// #include <coredecls.h>

#define DEBUG_FLAG "MAIN"
#include "debug.h"

MifareUltralight RFID;
Bounce ext_sw, int_sw, set_sw;

void setup()
{
	LA_DSTART(115200);
	setupWiFi();
#ifdef LAB_ACCESS_DEBUG
	redirect.setInsecure();
	redirect.setPrintResponseBody(true);
#else
	redirect.setPrintResponseBody(false);
#endif
	setupOTA();
#ifdef LAB_ACCESS_DEBUG
	String fs = "FS files list: \n";
	Dir dir = LittleFS.openDir("");
	while (dir.next())
	{
		fs += "\t" + dir.fileName() + "\n";
	}
	LA_DPRINTF("%s", fs.c_str());
#endif
	// Input/Output
	/* TODO: implement interrupt driven version
	 * pinMode(pin.irq, INPUT_PULLUP);
	 * attachInterrupt(digitalPinToInterrupt(pin.irq), ???, FALLING);
	 */
	pinMode(pin.out, OUTPUT);
	pinMode(pin.led, OUTPUT);
	ext_sw.attach(pin.esw);
	ext_sw.interval(extsw_deb_time);
	// int_sw.attach(pin.isw);
	// int_sw.interval(intsw_deb_time);
	set_sw.attach(0);
	set_sw.interval(setsw_deb_time);
	RFID.SetKey(nfc_key, true);
	SPI.begin();
	// SPI.setHwCs(true);
	RFID.PCD_Init(pin.csn, pin.rst);
// RFID.PCD_SetAntennaGain(Ultralight.RxGain_max);
#ifdef LAB_ACCESS_DEBUG
	RFID.PCD_DumpVersionToSerial();
#endif
	configTime(TZ_SEC, DST_SEC, NTP_server_address);
	LOG(LOG_MSG_BOARD_BOOT);
	LA_DPRINTF("Setup completed\n");
}

void wait_for_new_card(MifareUltralight::Uid old_uid)
{
	while (strncmp((char *)RFID.uid.uidByte, (char *)old_uid.uidByte, 7) == 0)
	{
		if (RFID.PICC_IsNewCardPresent())
		{
			RFID.PICC_ReadCardSerial();
		}
	}
}

int auth_card()
{
	if (!RFID.PICC_IsNewCardPresent())
		return 1;
	if (!RFID.PICC_ReadCardSerial())
	{
		LA_DPRINTF("Failed to read card serial\n");
		return -1;
	}
#ifdef LAB_ACCESS_DEBUG
	RFID.PICC_DumpDetailsToSerial(&RFID.uid);
#endif
	delayMicroseconds(TrueRandomGet32() & 0x00000FFF);
	LA_DPRINTF("New card detected, authenticating...\n");
	RFID.dump(&RFID.uid);
	MifareUltralight::StatusCode status = RFID.Authenticate();
	RFID.PICC_HaltA();
	// reactivate the card and retry?
	// byte bufferATQA[2];
	// byte bufferSize = sizeof(bufferATQA);
	// Ultralight.PICC_WakeupA(bufferATQA, &bufferSize);
	// Ultralight.PICC_ReadCardSerial();
	if (status != MifareUltralight::STATUS_OK)
	{
		char buf[48];
		const __FlashStringHelper *cname = RFID.GetStatusCodeName(status);
		strcpy_P(buf, (const char *)cname);
		LA_DPRINTF("Authentication failed: %s\n", buf);
		return -1;
	}
	delayMicroseconds(TrueRandomGet32() & 0x00000FFF);
	if (!cardExistsInDB(RFID.uid.uidByte))
	{
		LA_DPRINTF("Card not found in DB\n");
		return -1;
	}
	return 0;
}

void loop()
{
	wm.process();
	handleOTA();
	static bool authenticated = false;
	static unsigned long lastUpdate = millis();

	static uint32_t tprint = millis();
	if (millis() > tprint + 1000)
	{
		LA_DPRINTF("Free heap %u free stack %u\n", ESP.getFreeHeap(), ESP.getFreeContStack());
		tprint = millis();
	}

	if (millis() > updateInterval + lastUpdate)
	{
		reconnect();
		updateDB();
		if (dev_mode == LA_MODE_DOOR)
		{
			digitalWrite(pin.out, LOW);
			digitalWrite(pin.led, HIGH);
			authenticated = false;
		}
		lastUpdate = millis();
	}

	static unsigned long lastLog = millis();
	if (millis() > updateInterval + lastLog)
	{
		LOG();
		lastLog = millis();
	}

	static unsigned long lastCheck = millis();
	if (millis() > cardCheckDelay + lastCheck)
	{
		int res = auth_card();
		if (res == 0)
		{
			// card authentication succeeded
			digitalWrite(pin.out, HIGH);
			digitalWrite(pin.led, LOW);
			if (dev_mode == LA_MODE_DOOR)
			{
				authenticated = true;
				lastUpdate = millis();
			}
			LOG(LOG_MSG_AUTH_SUCCESS, RFID.uid.uidByte);
		}
		else if (res == -1)
		{
			LOG(LOG_MSG_AUTH_ERROR);
		}
		else
		{
			// no card
		}
		lastCheck = millis();
	}

	ext_sw.update();
	// if the door closes or the switch is pressed
	if (ext_sw.fell())
	{
		if (dev_mode == LA_MODE_DOOR)
		{
			LOG(LOG_MSG_CLOSED_DOOR);
		}
		else
		{
			LOG(LOG_MSG_PRESSED_SWITCH);
			digitalWrite(pin.out, LOW);
			digitalWrite(pin.led, HIGH);
			authenticated = false;
		}
	}
	// else if the door opens
	else if (ext_sw.rose() && (dev_mode == LA_MODE_DOOR))
	{
		if (authenticated)
		{
			LOG(LOG_MSG_OPEN_DOOR_AUTH);
		}
		else
		{
			LOG(LOG_MSG_OPEN_DOOR_NOAUTH);
		}
		digitalWrite(pin.out, LOW);
		digitalWrite(pin.led, HIGH);
		authenticated = false;
	}

	set_sw.update();
	if (set_sw.fell())
	{
		LA_DPRINTF("Entering setup\n");
		conf_done = false;
		redirect.stopAll();
		LA_DPRINTF("Free heap %u free stack %u\n", ESP.getFreeHeap(), ESP.getFreeContStack());
		while (!conf_done)
		{
			wm.startConfigPortal(dev_name.c_str(), ap_key.c_str());
			wait_config();
		}
		ESP.restart();
	}
}
