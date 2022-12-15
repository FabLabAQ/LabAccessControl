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

#ifndef HEX_UTILS_H_
#define HEX_UTILS_H_

#include <Arduino.h>

void hex_to_char(const uint8_t* hex_array, const uint8_t length, char* char_array) {
  const char hex_to_char_array[] = "0123456789ABCDEF";
  for (uint8_t i = 0; i < length; i++) {
    char_array[i*2] = hex_to_char_array[ hex_array[i]>>4 ];
    char_array[(i*2)+1] = hex_to_char_array[ hex_array[i] & 0x0F ];
  }
  char_array[length*2] = 0;
}

// inline char* hex_to_char(const uint8_t* hex_array, const uint8_t length) {
//   char* char_array;
//   char_array = new char[(length*2)+1];
//   hex_to_char(hex_array, length, char_array);
//   return char_array;
// }

static inline uint8_t hex_val(char char_val) {
	return (char_val>'9' ? char_val-'A'+10 : char_val-'0');
}

void char_to_hex(const char* char_array, uint8_t* hex_array) {
	uint8_t i = 0;
	while(char_array[i*2]) {
	hex_array[i] = hex_val(char_array[i*2])<<4 | hex_val(char_array[(i*2)+1]);
	i++;
	}
}

void char_to_hex(const char* char_array, const uint8_t length, uint8_t* hex_array) {
	for (uint8_t i = 0; i < length; i++) {
		hex_array[i] = hex_val(char_array[i*2])<<4 | hex_val(char_array[(i*2)+1]);
	}
}

inline uint8_t* char_to_hex(const char* char_array) {
	uint8_t* hex_array;
	hex_array = new uint8_t[strlen(char_array)/2];
	char_to_hex(char_array, hex_array);
	return hex_array;
}

#endif /* HEX_UTILS_H_ */
