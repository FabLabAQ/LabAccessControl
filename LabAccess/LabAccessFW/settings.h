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

#define PGM_STR const char //PROGMEM //__attribute__((aligned(4)))

#define BOARD_ID_MAIN_DOOR	0
#define BOARD_ID_CNC		1
#define BOARD_ID_LASER		2
#define BOARD_ID_ULTIFAKER	3
#define BOARD_ID_SAW		4

#define BOARD_ID BOARD_ID_MAIN_DOOR

#if BOARD_ID == BOARD_ID_MAIN_DOOR
#define BOARD_NAME "MainDoor"
#elif BOARD_ID == BOARD_ID_CNC
#define BOARD_NAME "CNC"
#elif BOARD_ID == BOARD_ID_LASER
#define BOARD_NAME "Laser"
#elif BOARD_ID == BOARD_ID_ULTIFAKER
#define BOARD_NAME "Ultifaker"
#elif BOARD_ID == BOARD_ID_SAW
#define BOARD_NAME "Saw"
#endif

#define PIN_INTERRUPT 4
#define PIN_RELAY 16
#define PIN_SWITCH 5
#define PIN_LED 2
#define SWITCH_DEBOUNCE_INT 500

PGM_STR LOG_MSG_BOARD_BOOT[] = "Board booted successfully";
PGM_STR LOG_MSG_SERVER_RECONNECTION[] = "Server reconnected";
PGM_STR LOG_MSG_AUTH_SUCCESS[] = "Authentication successful";
PGM_STR LOG_MSG_AUTH_ERROR[] = "Authentication ERROR";
PGM_STR LOG_MSG_CLOSED_DOOR[] = "Door closed";
PGM_STR LOG_MSG_PRESSED_SWITCH[] = "Machine turned OFF";
PGM_STR LOG_MSG_OPEN_DOOR_AUTH[] = "Door opened with authentication";
PGM_STR LOG_MSG_OPEN_DOOR_NOAUTH[] = "Door opened WITHOUT authentication";
PGM_STR LOG_MSG_DB_UPDATED[] = "Card database updated";

//#define LOG_MSG_BOARD_BOOT F("Board booted successfully")
//#define LOG_MSG_SERVER_RECONNECTION F("Server reconnected")
//#define LOG_MSG_AUTH_SUCCESS F("Authentication successful")
//#define LOG_MSG_AUTH_ERROR F("Authentication ERROR")
//#define LOG_MSG_CLOSED_DOOR F("Door closed")
//#define LOG_MSG_PRESSED_SWITCH F("Machine turned OFF")
//#define LOG_MSG_OPEN_DOOR_AUTH F("Door opened with authentication")
//#define LOG_MSG_OPEN_DOOR_NOAUTH F("Door opened WITHOUT authentication")
//#define LOG_MSG_DB_UPDATED F("Card database updated")

//#define LOG_MSG_BOARD_BOOT "Board booted successfully"
//#define LOG_MSG_SERVER_RECONNECTION "Server reconnected"
//#define LOG_MSG_AUTH_SUCCESS "Authentication successful"
//#define LOG_MSG_AUTH_ERROR "Authentication ERROR"
//#define LOG_MSG_CLOSED_DOOR "Door closed"
//#define LOG_MSG_PRESSED_SWITCH "Machine turned OFF"
//#define LOG_MSG_OPEN_DOOR_AUTH "Door opened with authentication"
//#define LOG_MSG_OPEN_DOOR_NOAUTH "Door opened WITHOUT authentication"
//#define LOG_MSG_DB_UPDATED "Card database updated"

#define STR_T const char *

const uint32_t minimum_free_heap = 4000;
const unsigned long updateInterval = 30000, cardCheckDelay = 500;
HTTPSRedirect redirect(443);
const char script_server_host[] = "script.google.com";
PGM_STR logScriptURL[] = "/macros/s/" SECRET_LOG_SCRIPT_ID "/exec?log";
PGM_STR accessScriptURL[] = "/macros/s/" SECRET_ACCESS_SCRIPT_ID "/exec?boardName=" BOARD_NAME;

const char NTP_server_address[] = "pool.ntp.org";
#define TZ              1       // (utc+) TZ in hours
#define DST_MN          60
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#endif /* SETTINGS_H_ */
