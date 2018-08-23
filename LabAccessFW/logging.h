/*
 * logging.h
 *
 *  Created on: 22 giu 2018
 *      Author: Luca Anastasio
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <Arduino.h>
#include "settings.h"
#include "sys/time.h"
#include "time.h"
#include "user_interface.h"
#include <queue>
#include <ArduinoJson.h>

#define LOG_BOARD_IDENTIFIER "board_id"
#define LOG_UID_IDENTIFIER "uid"
#define LOG_TIME_IDENTIFIER "time"
#define LOG_MESSAGE_IDENTIFIER "msg"

enum logging_status_code {LOG_MEMORY_FULL, LOG_SENT, LOG_APPENDED, LOG_NOT_SENT, LOG_QUEUE_EMPTY};

struct log_queue_t {
	const char* text;
	byte UID[4] = {0, 0, 0, 0};
	time_t time;
};

std::queue<log_queue_t> log_queue;

void logToSheet(log_queue_t* log) {
	// allocate static json buffer
	StaticJsonBuffer<JSON_OBJECT_SIZE(6)> logJsonBuffer;
	// new json object
	JsonObject& logJson = logJsonBuffer.createObject();
	// add board id
	logJson[LOG_BOARD_IDENTIFIER] = F(BOARD_NAME);
	// add timestamp
	logJson[LOG_TIME_IDENTIFIER] = (long)log->time;
	// add message
	logJson[LOG_MESSAGE_IDENTIFIER] = log->text;
	// add uid
	//logJson[LOG_UID_IDENTIFIER] = "";
	if (log->UID[0] || log->UID[1] || log->UID[2] || log->UID[3]) {
		char UIDchar[9] = "";
		hex_to_char(log->UID, 4, UIDchar);
		DPRINTF("UIDchar: %s\n", UIDchar);
		logJson[LOG_UID_IDENTIFIER] = String(UIDchar);
	}
	// send to sheet
	redirect.setContentTypeHeader("application/json");
	String payload;
	logJson.printTo(payload);
	DPRINTF("Payload: %s\n", payload.c_str());
	redirect.POST(String(logScriptURL), script_server_host, payload);
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
		logToSheet(&log_queue.front());
		log_queue.pop();
	}
	return LOG_SENT;
}

int LOG(const char* text) {
	log_queue_t last_log;
	// if there isn't enough available RAM then do nothing
	if (ESP.getFreeHeap() < minimum_free_heap) {
		return LOG_MEMORY_FULL;
	}
	// else enqueue the new log
	last_log.text = text;
	last_log.time = time(nullptr);
	log_queue.push(last_log);
	// and if the connection is not alive return
	if (!redirect.connected()) {
		return LOG_APPENDED;
	}
	else return LOG();
}
int LOG(const char* text, const byte* UID) {
	log_queue_t last_log;
	// if there isn't enough available RAM then do nothing
	if (ESP.getFreeHeap() < minimum_free_heap) {
		return LOG_MEMORY_FULL;
	}
	// else enqueue the new log
	last_log.text = text;
	last_log.UID[0] = UID[0];
	last_log.UID[1] = UID[1];
	last_log.UID[2] = UID[2];
	last_log.UID[3] = UID[3];
	last_log.time = time(nullptr);
	log_queue.push(last_log);
	// and if the connection is not alive return
	if (!redirect.connected()) {
		return LOG_APPENDED;
	}
	else return LOG();
}

#endif /* LOGGING_H_ */
