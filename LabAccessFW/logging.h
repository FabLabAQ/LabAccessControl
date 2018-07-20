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

#define LOG_BOARD_IDENTIFIER "log_id"
#define LOG_UID_IDENTIFIER "uid"
#define LOG_TIME_IDENTIFIER "time"
#define LOG_MESSAGE_IDENTIFIER "msg"

enum logging_status_code {LOG_MEMORY_FULL, LOG_SENT, LOG_APPENDED, LOG_NOT_SENT, LOG_QUEUE_EMPTY};

struct log_queue_t {
	uint8 text;
	uint32 UID;
	time_t time;
};

std::queue<log_queue_t> log_queue;

void logToSheet(log_queue_t* log) {
	// allocate static json buffer
	StaticJsonBuffer<JSON_OBJECT_SIZE(4)> logJsonBuffer();
	// new json object
	JsonObject& logJson = logJsonBuffer.createObject();
	// add board id
	logJson[LOG_BOARD_IDENTIFIER] = BOARD_ID;
	// add timestamp
	logJson[LOG_TIME_IDENTIFIER] = log->time;
	// add message
	logJson[LOG_MESSAGE_IDENTIFIER] = log->text;
	// add uid
	if (log->UID) {
		char UIDchar[9] = "";
		hex_to_char((byte*)&log->UID, 4, UIDchar);
		logJson[LOG_UID_IDENTIFIER] = UIDchar;
	}
	// send to sheet
	redirect.setContentTypeHeader("application/json");
	String payload;
	logJson.printTo(payload);
	redirect.POST(String(scriptURL)+"log", script_server_host, payload);
	DPRINTLN(payload);
}

int log(const char text = 0, const byte* UID = nullptr) {
	log_queue_t last_log;
	// check if it's a message or we are making a call to dequeue the logs
	if (text != 0) {
		// if there isn't enough available RAM then do nothing
		if (system_get_free_heap_size() < minimum_free_heap) {
			return LOG_MEMORY_FULL;
		}
		// else enqueue the new log
		last_log.text = text;
		last_log.UID = (uint32)*UID; // TODO: check memory alignment
		last_log.time = time(nullptr);
		log_queue.push(last_log);
		// and if the connection is not alive return
		if (!redirect.connected()) {
			return LOG_APPENDED;
		}
	}
	else {
		// if the queue is empty return
		if (log_queue.empty()) {
			return LOG_QUEUE_EMPTY;
		}
		// else if the connection is not alive return
		else if (!redirect.connected()) {
			return LOG_NOT_SENT;
		}
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

#endif /* LOGGING_H_ */
