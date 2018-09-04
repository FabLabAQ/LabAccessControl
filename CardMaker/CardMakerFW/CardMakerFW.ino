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

#include "Arduino.h"
#include "secret_keys.h"
#include <MFRC522.h>
#include <SPI.h>
#include "hex_utils.h"
#include "Keyboard.h"

#define MFRC522_SS_PIN          10
#define MFRC522_RST_PIN         9

MFRC522 RFID(MFRC522_SS_PIN, MFRC522_RST_PIN);

struct sector_trailer {
	byte key_a[6];
	byte access_bytes[4];
	byte key_b[6];
};

#define CARD_ACCESS_BYTES {0xFF, 0x07, 0x80, 0x69}
const sector_trailer new_sector_0_trailer = {SECRET_CARD_PRIVATE_KEY_A, CARD_ACCESS_BYTES, SECRET_CARD_PRIVATE_KEY_B};
const byte new_block_2[] = SECRET_CARD_PUBLIC_KEY_B;
const byte new_block_1[] = SECRET_CARD_PUBLIC_KEY_A;
const MFRC522::MIFARE_Key MIFARE_default_key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void setup()
{
	Keyboard.begin();
	SPI.begin();
	RFID.PCD_Init();
}

void wait_for_new_card(MFRC522::Uid old_uid) {
	while(strcmp((char*)RFID.uid.uidByte, (char*)old_uid.uidByte) == 0) {
		if(RFID.PICC_IsNewCardPresent()) {
			RFID.PICC_ReadCardSerial();
		}
	}
}

void printUID() {
  char uid_char[5];
  hex_to_char(RFID.uid.uidByte, RFID.uid.size, uid_char);
  Keyboard.print(uid_char);
}

void loop()
{
	wait_for_new_card(RFID.uid);
	if (RFID.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 3, &MIFARE_default_key, &RFID.uid) == MFRC522::STATUS_OK) {
		if (RFID.MIFARE_Write(3, (byte*)&new_sector_0_trailer, 16)  == MFRC522::STATUS_OK) {
			if (RFID.MIFARE_Write(2, new_block_2, 16)  == MFRC522::STATUS_OK) {
				if (RFID.MIFARE_Write(1, new_block_1, 16)  == MFRC522::STATUS_OK) {
					printUID();
				}
				else {
					Keyboard.print(F("Error, step 4 of 4: failed to write block 1; UID: "));
          printUID();
				}
			}
			else {
				Keyboard.print(F("Error, step 3 of 4: failed to write block 2; UID: "));
        printUID();
			}
		}
		else {
			Keyboard.print(F("Error, step 2 of 4: failed to write block 3 (sector 0 trailer); UID: "));
      printUID();
		}
	}
	else {
		Keyboard.print(F("Error, step 1 of 4: failed to authenticate using default key; UID: "));
    printUID();
	}
	RFID.PICC_HaltA();
	RFID.PCD_StopCrypto1();
}
