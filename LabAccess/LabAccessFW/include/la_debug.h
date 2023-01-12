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

#ifndef DEBUG_FLAG
#define DEBUG_FLAG "DEBUG"
#warning "debug included without defining DEBUG_FLAG"
#endif

#define DEBUG_START_DELAY 3000

#ifdef LAB_ACCESS_DEBUG

#ifndef DEBUG_ESP_HTTP_CLIENT
#define DEBUG_ESP_HTTP_CLIENT
#define DEBUG_ESP_PORT Serial
#endif

#define STARTUP_STRING "\n------------------ LAB ACCESS DEBUG ------------------\n" \
					   "Starting version " LAB_ACCESS_VERSION                       \
					   "\nCompiled on " __DATE__ " " __TIME__ "\n"

#define LA_DPRINTF(fmt, ...) \
	Serial.printf_P("[" DEBUG_FLAG "] " fmt, ##__VA_ARGS__)

#define LA_DSTART(s)                     \
	Serial.begin(s);                     \
	while (millis() < DEBUG_START_DELAY) \
		yield();                         \
	Serial.printf_P(STARTUP_STRING);

#define LA_DJSON(j)                       \
	Serial.print(F("[" DEBUG_FLAG "] ")); \
	serializeJsonPretty(j, Serial);

#define LA_DPRINTHEX(str, hex)                    \
	{                                        \
		char tmp[2 * sizeof(hex) + 1];       \
		hex_to_char(hex, sizeof(hex), tmp);  \
		Serial.printf_P("[" DEBUG_FLAG "] " str ": %s\n", tmp); \
	}

#warning "Debugging enabled!"

#else
#define DPRINTF(FMT, ...) // empty
#define DSTART(s)
#define LA_DJSON(j)
#define LA_DPRINTHEX(hex)
#endif