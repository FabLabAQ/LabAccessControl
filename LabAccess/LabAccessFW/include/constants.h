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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Arduino.h"
#include "HTTPSRedirect.h"

#define PGM_STR const char * //PROGMEM //__attribute__((aligned(4)))

#define PIN_INTERRUPT 4
#define PIN_EXTSW 5
#define PIN_RELAY 15
#define PIN_SWITCH 0
#define PIN_LED 2
#define PIN_CS 16
const int extsw_deb_time = 500;
const int intsw_deb_time = 100;
#ifdef LAB_ACCESS_DEBUG
const int setsw_deb_time = 1000;
#else
const int setsw_deb_time = 10000;
#endif

PGM_STR LOG_MSG_BOARD_BOOT = "Board booted successfully";
PGM_STR LOG_MSG_SERVER_RECONNECTION = "Server reconnected";
PGM_STR LOG_MSG_AUTH_SUCCESS = "Authentication successful";
PGM_STR LOG_MSG_AUTH_ERROR = "Authentication ERROR";
PGM_STR LOG_MSG_CLOSED_DOOR = "Door closed";
PGM_STR LOG_MSG_PRESSED_SWITCH = "Machine turned OFF";
PGM_STR LOG_MSG_OPEN_DOOR_AUTH = "Door opened with authentication";
PGM_STR LOG_MSG_OPEN_DOOR_NOAUTH = "Door opened WITHOUT authentication";
PGM_STR LOG_MSG_DB_UPDATED = "Card database updated";

const uint32_t minimum_free_heap = 4000;
const unsigned long updateInterval = 90000, cardCheckDelay = 500;
const uint32_t http_update_led_blink = 200;
HTTPSRedirect redirect(443);
const char script_server_host[] = "script.google.com";
//PGM_STR logScriptURL[] = "/macros/s/" SECRET_LOG_SCRIPT_ID "/exec?log";
//PGM_STR accessScriptURL[] = "/macros/s/" SECRET_ACCESS_SCRIPT_ID "/exec?boardName=" BOARD_NAME;

// script API commands
PGM_STR api_key_prov = "keyProv";
PGM_STR api_key_rec = "keyRec";


static const char* config_filename = "config.json";
const uint16_t config_size = 1024;

const char NTP_server_address[] = "pool.ntp.org";
#define TZ              1       // (utc+) TZ in hours
#define DST_MN          60
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#endif /* SETTINGS_H_ */
