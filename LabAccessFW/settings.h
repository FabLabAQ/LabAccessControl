/*
 * settings.h
 *
 *  Created on: 20 giu 2018
 *      Author: Luca Anastasio
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Arduino.h"
#include "HTTPSRedirect.h"

#define PGM_STR const char PROGMEM

enum board_ids : char {	BOARD_ID_MAIN_DOOR = '0',
					BOARD_ID_CNC,
					BOARD_ID_LASER,
					BOARD_ID_ULTIFAKER,
					BOARD_ID_ETC
};

#define BOARD_ID BOARD_ID_MAIN_DOOR

#define PIN_INTERRUPT 4
#define PIN_RELAY 16
#define PIN_SWITCH 5
#define PIN_LED 2
#define SWITCH_DEBOUNCE_INT 500

enum log_msgs {	LOG_MSG_BOARD_BOOT,
				LOG_MSG_SERVER_RECONNECTION,
				LOG_MSG_AUTH_SUCCESS,
				LOG_MSG_AUTH_ERROR,
				LOG_MSG_CLOSED_DOOR,
				LOG_MSG_PRESSED_SWITCH,
				LOG_MSG_OPEN_DOOR_AUTH,
				LOG_MSG_OPEN_DOOR_NOAUTH,
				LOG_MSG_DB_UPDATED
};

const uint32 minimum_free_heap = 4000;
const unsigned long updateInterval = 60000, cardCheckDelay = 500;
HTTPSRedirect redirect(443);
PGM_STR script_server_host[] = "script.google.com";
PGM_STR scriptURL[] = "/macros/s/" SECRET_LOG_SCRIPT_ID "/exec?";

PGM_STR NTP_server_address[] = "pool.ntp.org";
#define TZ              2       // (utc+) TZ in hours
#define DST_MN          60
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

// bard names for network identification
#if BOARD_ID == '0'
	#define BOARD_NAME "MainDoor"
#endif

#endif /* SETTINGS_H_ */
