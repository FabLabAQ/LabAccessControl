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

MifareUltralight Ultralight(/*SS*/10, /*RST*/9);  // Create MFRC522 instance

uint8_t defaultKey[] = {'B','R','E','A','K','M','E','I','F','Y','O','U','C','A','N','!'};
uint8_t newKey[] =     {'B','R','E','A','K','I','T','I','F','Y','O','U','C','A','N','!'};

void setup() {
	Serial.begin(115200);
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

	
	if (Ultralight.Authenticate(defaultKey, false)) {
		Serial.println(F("OK"));
	}
	
}
