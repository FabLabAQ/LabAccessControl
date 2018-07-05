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

#define BOARD_ID '0'

#define PIN_INTERRUPT 4
#define PIN_RELAY 16
#define PIN_SWITCH 5
#define PIN_LED 2
#define SWITCH_DEBOUNCE_INT 500

char BOARD_BOOT_TEXT = '0';
char SERVER_RECONNECTION_TEXT = '1';
char AUTH_SUCCESS_TEXT = '2';
char AUTH_ERROR_TEXT = '3';
char CLOSED_DOOR_TEXT = '4';
char PRESSED_SWITCH_TEXT = '5';
char OPEN_DOOR_AUTH_TEXT = '6';
char OPEN_DOOR_NOAUTH_TEXT = '7';
char DB_UPDATED_TEXT = '8';

const uint32 minimum_free_heap = 4000;
const unsigned long updateInterval = 60000, cardCheckDelay = 500;
HTTPSRedirect redirect(443);
PGM_STR script_server_host[] = "script.google.com";

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
