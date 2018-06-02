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

void loop()
{
	wait_for_new_card(RFID.uid);
	if (RFID.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 3, &MIFARE_default_key, &RFID.uid) == MFRC522::STATUS_OK) {
		if (RFID.MIFARE_Write(3, (byte*)&new_sector_0_trailer, 16)  == MFRC522::STATUS_OK) {
			if (RFID.MIFARE_Write(2, new_block_2, 16)  == MFRC522::STATUS_OK) {
				if (RFID.MIFARE_Write(1, new_block_1, 16)  == MFRC522::STATUS_OK) {
					char uid_char[5];
					hex_to_char(RFID.uid.uidByte, RFID.uid.size, uid_char);
					Keyboard.print(uid_char);
				}
				else {
					Keyboard.print(F("step 4 of 4: failed to write block 1"));
				}
			}
			else {
				Keyboard.print(F("step 3 of 4: failed to write block 2"));
			}
		}
		else {
			Keyboard.print(F("step 2 of 4: failed to write block 3 (sector 0 trailer)"));
		}
	}
	else {
		Keyboard.print(F("step 1 of 4: failed to authenticate using default key"));
	}
	RFID.PICC_HaltA();
	RFID.PCD_StopCrypto1();
}
