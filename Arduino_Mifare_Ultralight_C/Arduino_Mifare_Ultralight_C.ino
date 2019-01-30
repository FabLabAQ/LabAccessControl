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
