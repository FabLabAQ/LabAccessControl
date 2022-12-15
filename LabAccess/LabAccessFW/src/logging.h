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

#ifndef LOGGING_H_
#define LOGGING_H_

#include <Arduino.h>
#include "constants.h"
#include "sys/time.h"
#include "time.h"
#include "user_interface.h"
#include <queue>
#include <ArduinoJson.h>
#include "config.h"
#include "HexPrint.h"

#define DEBUG_FLAG "LOG"
#include "la_debug.h"

#define LOG_BOARD_IDENTIFIER "board_id"
#define LOG_UID_IDENTIFIER "uid"
#define LOG_TIME_IDENTIFIER "time"
#define LOG_MESSAGE_IDENTIFIER "msg"

enum logging_status_code {LOG_MEMORY_FULL, LOG_SENT, LOG_APPENDED, LOG_NOT_SENT, LOG_QUEUE_EMPTY};

const uint8_t uid_len = 7;
static const byte zero_uid[uid_len] = {0};
struct log_queue_t
{
	const char *text;
	byte UID[uid_len];
	time_t time;
};

std::queue<log_queue_t> log_queue;

bool logToSheet(log_queue_t* log) {
	LA_DPRINTF("logToSheet\n");
	// allocate static json buffer
	StaticJsonDocument<JSON_OBJECT_SIZE(6)> logJson;
	// new json object
	//JsonObject& logJson = logJsonBuffer.createObject();
	// add board id
	logJson[LOG_BOARD_IDENTIFIER] = dev_name;
	// add timestamp
	logJson[LOG_TIME_IDENTIFIER] = (long)log->time;
	// add message
	logJson[LOG_MESSAGE_IDENTIFIER] = log->text;
	// add uid
	//logJson[LOG_UID_IDENTIFIER] = "";
	if (memcmp(log->UID, zero_uid, uid_len) != 0) {
		char UIDchar[uid_len*2+1] = "";
		hex_to_char(log->UID, uid_len, UIDchar);
		LA_DPRINTF("UIDchar: %s\n", UIDchar);
		logJson[LOG_UID_IDENTIFIER] = UIDchar;
	}
	// send to sheet
	redirect.setContentTypeHeader("application/json");
	String payload;
	serializeJson(logJson, payload);
	LA_DPRINTF("Payload: %s\n", payload.c_str());
	bool ret = redirect.POST(logScriptURL, script_server_host, payload);
	//redirect.end();
	return ret;
}

int LOG() {
	// if the queue is empty return
	if (log_queue.empty()) {
		return LOG_QUEUE_EMPTY;
	}
	// else if the connection is not alive return
	else if (!redirect.connected()) {
		return LOG_NOT_SENT;
	}
	// finally if the connection is alive and there are logs in the queue
	while (redirect.connected() && !log_queue.empty()) {
		yield(); // this loop can take some time
		// log and free memory
		if (!logToSheet(&log_queue.front())){
			return LOG_NOT_SENT;
		}
		log_queue.pop();
	}
	return LOG_SENT;
}

int LOG(const char* text) {
	LA_DPRINTF("new entry: %s\n", text);
	// if there isn't enough available RAM then do nothing
	if (ESP.getFreeHeap() < minimum_free_heap) {
		return LOG_MEMORY_FULL;
	}
	// else enqueue the new log
	log_queue_t last_log = {
		.text = text,
		.time = time(nullptr),
	};
	memcpy(last_log.UID, zero_uid, uid_len);
	log_queue.push(last_log);
	// and if the connection is not alive return
	if (!redirect.connected()) {
		return LOG_APPENDED;
	}
	else return LOG();
}
int LOG(const char* text, const byte* UID) {

	char UIDchar[uid_len*2+1] = "";
	hex_to_char(UID, uid_len, UIDchar);
	LA_DPRINTF("new entry: %s UID %s\n", text, UIDchar);
	
	// if there isn't enough available RAM then do nothing
	if (ESP.getFreeHeap() < minimum_free_heap) {
		return LOG_MEMORY_FULL;
	}
	// else enqueue the new log
	log_queue_t last_log;
	last_log.text = text;
	memcpy(last_log.UID, UID, uid_len);
	last_log.time = time(nullptr);
	log_queue.push(last_log);
	// and if the connection is not alive return
	if (!redirect.connected()) {
		return LOG_APPENDED;
	}
	else return LOG();
}

#endif /* LOGGING_H_ */
