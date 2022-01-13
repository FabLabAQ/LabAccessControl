/*
 * (c) 2019 Luca Anastasio
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

#include <SPI.h>
//#include <MFRC522.h>

#include "MifareUltralight.h"

#if defined(ESP8266)
#define SS_PIN 4
#define RST_PIN 5
#define LED_PIN 2
#define LED_ON 0
#else

#endif

#define LED_OFF !LED_ON

MifareUltralight Ultralight(SS_PIN, RST_PIN);  // Create MFRC522 instance

const uint8_t n_keys = 3;
const uint8_t defaultKeys[n_keys][16] = {
	{'B','R','E','A','K','M','E','I','F','Y','O','U','C','A','N','!'},
	{0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  },
	{'B','R','E','A','K','I','T','I','F','Y','O','U','C','A','N','!'}
};
uint8_t newKey[] =     {'B','R','E','A','K','I','T','I','F','Y','O','U','C','A','N','!'};

void setup() {
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LED_OFF);
	Serial.begin(115200);
	Serial.println(F("Mifare Ultralight C test"));
	SPI.begin();
	
	Ultralight.PCD_Init();
	//Ultralight.PCD_SetAntennaGain(Ultralight.RxGain_max);
	Ultralight.PCD_DumpVersionToSerial();
	
}

void wait_for_new_card(MFRC522::Uid old_uid) {
	while(strncmp((char*)Ultralight.uid.uidByte, (char*)old_uid.uidByte, 7) == 0) {
		if(Ultralight.PICC_IsNewCardPresent()) {
			Ultralight.PICC_ReadCardSerial();
		}
	}
}

void loop() {
	wait_for_new_card(Ultralight.uid);
	Ultralight.PICC_DumpDetailsToSerial(&Ultralight.uid);

	bool authenticated = false;
	for(int i = 0; i < n_keys; i++) {
		Serial.print(F("Attempting to authenticate with key number "));
		Serial.println(i+1);
		MifareUltralight::StatusCode status = Ultralight.Authenticate(defaultKeys[i], true);
		if (status == MifareUltralight::STATUS_OK) {
			authenticated = true;
			break;
		}
		else {
			Serial.println(Ultralight.GetStatusCodeName(status));
			Ultralight.PICC_HaltA();
			byte bufferATQA[2];
	byte bufferSize = sizeof(bufferATQA);
			Ultralight.PICC_WakeupA(bufferATQA, &bufferSize);
			Ultralight.PICC_ReadCardSerial();
		}
	}

	if (authenticated) {
		Serial.println(F("Auth OK"));
		for (int i = 0; i <5; i++) {
			digitalWrite(LED_PIN, LED_ON);
			delay(500);
			digitalWrite(LED_PIN, LED_OFF);
			delay(500);
		}
		
	}
	else {
		Serial.println(F("Auth FAILED"));
			digitalWrite(LED_PIN, LED_ON);
			delay(3000);
			digitalWrite(LED_PIN, LED_OFF);
	}
	
}
