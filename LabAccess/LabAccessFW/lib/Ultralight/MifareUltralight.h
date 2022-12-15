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

#include "Arduino.h"
#include <MFRC522.h>
#include "TrueRandom.h"
//#include "DES.h"
#include "HexPrint.h"
#include "mbed_des.h"

#ifndef MIFAREULTRALIGHTAUTH_H
#define MIFAREULTRALIGHTAUTH_H

#define DEBUG_FLAG "NFC"
#include "la_debug.h"


#define DBGHEX(str, hex, len) LA_DPRINTF(str "%s\n", hexArray(hex, len).c_str())

class MifareUltralight : public MFRC522 {
  private:

    uint8_t key[24];
	mbedtls_des3_context ctx3;

  public:

	void dump(auto *d){
		#ifdef LAB_ACCESS_DEBUG
		this->PICC_DumpDetailsToSerial(d);
		#endif
	}

	MifareUltralight(uint8_t SS, uint8_t RST) : MFRC522(SS, RST) {
      TrueRandomSetup();
    }

	MifareUltralight() : MFRC522() {
      TrueRandomSetup();
    }

    StatusCode UlTransceive(uint8_t* sendData, uint8_t sendLen, uint8_t* retData, uint8_t* retLen) {
      StatusCode result;
      byte cmdBuffer[sendLen+2];
      memcpy(cmdBuffer, sendData, sendLen);
      result = PCD_CalculateCRC(cmdBuffer, sendLen, &cmdBuffer[sendLen]);
      if (result != STATUS_OK) {
        return result;
      }
      sendLen += 2;
      result = PCD_TransceiveData(cmdBuffer, sendLen, retData, retLen, NULL, 0, true);
      return result;
    }

    void SetKey(const uint8_t* _key, bool rotate = false) {
      if (rotate) {
        for (uint8_t i = 0; i < 8; i++) {
          key[i] = _key[7 - i];
        }
        for (uint8_t i = 0; i < 8; i++) {
          key[8 + i] = _key[15 - i];
        }
      }
      else memcpy(key, _key, 16);
      // expand key for 3DES
	  char str[33];
	  hex_to_char(key, 16, str);
	  LA_DPRINTF("new key: %s\n", str);
	}

	// TODO: return appropriate codes

    StatusCode Authenticate(const uint8_t* _key, bool rotate = false) {
    	SetKey(_key, rotate);
    	return Authenticate();
    }

    StatusCode Authenticate() {

      StatusCode result;
	  const size_t ctx_sz = sizeof(mbedtls_des3_context);
	  mbedtls_des3_init(&ctx3);
	  mbedtls_des3_set2key_dec( &ctx3, key );
	
      DBGHEX("Authenticating with key (16 byte): ",key, 16);


      // send authentication command and get the encoded random number: ek(rndB)
      uint8_t retData[9];
      uint8_t AuthCMD[] = {0x1A, 0x00};
      DBGHEX("Authentication phase 1: ",AuthCMD, 2);
      uint8_t retLen = 11;
      if ((result = UlTransceive(AuthCMD, 2, retData, &retLen)) != STATUS_OK) return result;
      DBGHEX("Received: ",retData, 9);

      // check if the response is correct
      if (retData[0] != 0xAF || retLen != 11) return STATUS_ERROR;
      // decrypt the random number (skip the first byte)
      uint8_t RndB[9];
      // RndB <= decrypt(retData+1)

		byte iv[8] = {0,0,0,0,0,0,0,0};
		mbedtls_des3_crypt_cbc( &ctx3, MBEDTLS_DES_DECRYPT, 8, iv, retData+1, RndB );
      
      DBGHEX("Decrypted RndB: ",RndB, 8);

      // shift left
      RndB[8] = RndB[0];
      // build the second authentication random number: RndA | RndB'
      uint8_t RndA[9];
      TrueRandomGet64(RndA);
      uint8_t Rnd2[16];
      memcpy(Rnd2, RndA, 8);
      memcpy(Rnd2 + 8, RndB + 1, 8);
      DBGHEX("RndA | RndB': ",Rnd2, 16);

      uint8_t AuthCMD2[17] = {0xAF};
      // encrypt RndA | RndB+1
      // (AuthCMD2+1) <= encrypt(Rnd2)
      //des.set_IV(RndB);
		mbedtls_des3_set2key_enc( &ctx3, key );
      mbedtls_des3_crypt_cbc( &ctx3, MBEDTLS_DES_ENCRYPT, 16, iv, Rnd2, AuthCMD2+1);
      //des.do_3des_encrypt(Rnd2, 16, AuthCMD2 + 1, key);
      // send second authentication command: 0xAF | ek(RndA | RndB')
      // and receive encrypted RndA' (shifted)
      DBGHEX("Authentication phase 2: ",AuthCMD2, 17);
      if ((result = UlTransceive(AuthCMD2, 17, retData, &retLen)) != STATUS_OK) return result;
      DBGHEX("Received: ",retData, 9);
      // check if the response is correct
      if (retData[0] != 0x00 || retLen != 11) return STATUS_ERROR;

      // decrypt RndA'
      uint8_t RndA1[8];
      // RndA1 <= decrypt(retData+1)
      //des.set_IV(AuthCMD2 + 9);
      //uint64_t iv;
      //memcpy(&iv, AuthCMD2 + 9, 8);
      mbedtls_des3_set2key_dec( &ctx3, key );
      mbedtls_des3_crypt_cbc( &ctx3, MBEDTLS_DES_DECRYPT, 8, iv, retData+1, RndA1 );
      //des.do_3des_decrypt(retData+1, 8, RndA1, key, iv);
      DBGHEX("Decrypted RndA': ",RndA1, 8);
      // check if RndA' matches
      RndA[8] = RndA[0];
      if (strncmp((char*)RndA + 1, (char*)RndA1, 8) != 0) return STATUS_ERROR;
      return STATUS_OK;
    }

};

#endif //MIFAREULTRALIGHTAUTH_H
