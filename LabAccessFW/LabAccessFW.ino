#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "secret_keys.h"
#include "settings.h"
#include <ArduinoOTA.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Bounce2.h>
#include <FS.h>

MFRC522 RFID;
Bounce debounced_switch;

void setup()
{
	// Input/Output
	/* TODO: implement interrupt driven version
	 * pinMode(PIN_INTERRUPT, INPUT_PULLUP);
	 * attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), ???, FALLING);
	 */
	pinMode(PIN_RELAY, OUTPUT);
	debounced_switch.attach(PIN_SWITCH);
	debounced_switch.interval(100);


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
	}


}


void loop()
{
	ArduinoOTA.handle();

}
