#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "secret_keys.h"
#include "settings.h"
#include <ArduinoOTA.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Bounce2.h>
#include <FS.h>
#include "Card_DB.h"
#include "Debug.h"
#include "HTTPSRedirect.h"
#include "logging.h"
#include <time.h>
#include <sys/time.h>
#include <coredecls.h>

// TODO add time keeping with NTP and timestamp on logs

MFRC522 RFID;
Bounce debounced_switch;

/*const*/ MFRC522::MIFARE_Key private_key_b = SECRET_CARD_PRIVATE_KEY_B;
const byte public_key_a[] = SECRET_CARD_PUBLIC_KEY_A;
const byte public_key_b[] = SECRET_CARD_PUBLIC_KEY_B;

void setup()
{
	DSTART();

	// Input/Output
	/* TODO: implement interrupt driven version
	 * pinMode(PIN_INTERRUPT, INPUT_PULLUP);
	 * attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), ???, FALLING);
	 */

	pinMode(PIN_RELAY, OUTPUT);
	debounced_switch.attach(PIN_SWITCH);
	debounced_switch.interval(SWITCH_DEBOUNCE_INT);

	redirect.setPrintResponseBody(false);

	// Wi-Fi, reconnect handled automatically
	WiFi.mode(WIFI_STA);
	WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSPHRASE);

	// OTA, with password and individual hostname
	ArduinoOTA.setPassword(SECRET_OTA_PASSWORD);
	ArduinoOTA.setHostname(BOARD_NAME);
	ArduinoOTA.begin();

	// SPI and RFID
	SPI.begin();
	SPI.setHwCs(true);
	RFID.PCD_Init();

	// SPIFFS
	if (!SPIFFS.begin()) {
		SPIFFS.format();
		SPIFFS.begin();
	}

	configTime(TZ_SEC, DST_SEC, NTP_server_address);

	LOG(BOARD_BOOT_TEXT);
	DPRINT("Board finished booting");
}


void loop()
{
#if BOARD_ID == '0'
	static bool authenticated = false;
#endif

	static unsigned long lastUpdate = 0;
	if (millis() > updateInterval + lastUpdate) {
		reconnect();
		updateDB();
		LOG();
#if BOARD_ID == '0'
		digitalWrite(PIN_RELAY, LOW);
		authenticated = false;
#endif
		lastUpdate = millis();
	}

	ArduinoOTA.handle();

	static unsigned long lastCheck = 0;
	if(RFID.PICC_IsNewCardPresent() && millis() > cardCheckDelay + lastCheck) {
		byte buffer[18];
		byte bufsize = 18;
		if (RFID.PICC_ReadCardSerial() &&
				RFID.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, 2, &private_key_b, &RFID.uid) == MFRC522::STATUS_OK &&
				RFID.MIFARE_Read(2, buffer, &bufsize) == MFRC522::STATUS_OK &&
				strncmp((char*)public_key_b, (char*)buffer, 16) == 0 &&
				cardExistsInDB(RFID.uid.uidByte)) {
			// card authentication succeeded
			digitalWrite(PIN_RELAY, HIGH);
#if BOARD_ID == '0'
			authenticated = true;
			lastUpdate = millis();
#endif
			LOG(AUTH_SUCCESS_TEXT, RFID.uid.uidByte);
		}
		else {
			// authentication error
			LOG(AUTH_ERROR_TEXT);
		}

		RFID.PICC_HaltA();
		RFID.PCD_StopCrypto1();
		lastCheck = millis();
	}

	if (debounced_switch.update()) {
		// if the door closes or the switch is pressed
		if (debounced_switch.fell()) {
#if BOARD_ID == '0'
			LOG(CLOSED_DOOR_TEXT);
#else
			LOG(PRESSED_SWITCH_TEXT);
#endif
		}
		// else if the door opens
#if BOARD_ID == '0'
		else {
			if (authenticated) {
				LOG(OPEN_DOOR_AUTH_TEXT);
			}
			else {
				LOG(OPEN_DOOR_NOAUTH_TEXT);
			}
		}
		authenticated = false;
#endif
		digitalWrite(PIN_RELAY, LOW);
	}
}
