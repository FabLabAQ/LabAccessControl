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

PGM_STR logScriptURL[] = "/macros/s/" SECRET_LOG_SCRIPT_ID "/exec?getUIDlist";

#define LOG_START_SEQUENCE "LOG_ID"
#define LOG_UID_IDENTIFIER "UID"
#define LOG_TIME_IDENTIFIER "TIME"
#define LOG_MESSAGE_IDENTIFIER "MSG"

enum logging_status_code {LOG_MEMORY_FULL, LOG_SENT, LOG_APPENDED, LOG_NOT_SENT, LOG_QUEUE_EMPTY};

struct log_queue_t {
	char text;
	uint32 UID;
	time_t time;
};

std::queue<log_queue_t> log_queue;

void logToSheet(log_queue_t* log) {
	// allocate message string
	char payload[40] = LOG_START_SEQUENCE;
	// append board ID
	payload[strlen(payload)] = BOARD_ID;
	// append time in seconds
	strcat(payload, LOG_TIME_IDENTIFIER);
	itoa(log->time, payload+strlen(payload), 10);
	// append message
	strcat(payload, LOG_MESSAGE_IDENTIFIER);
	payload[strlen(payload)+1] = log->text;
	// if UID is not 0
	if (log->UID) {
		// append it to the message
		strcat(payload, LOG_UID_IDENTIFIER);
		hex_to_char((byte*)&log->UID, 4, payload+strlen(payload));
	}
	// send to sheet
	redirect.POST(logScriptURL, script_server_host, payload);
	DPRINTLN(payload);
}

int LOG(const char text = 0, const byte* UID = nullptr) {
	log_queue_t last_log;
	// check if it's a message or we are making a call to dequeue the logs
	if (text != 0) {
		// if there isn't enough available RAM then do nothing
		if (system_get_free_heap_size() < minimum_free_heap) {
			return LOG_MEMORY_FULL;
		}
		// else enqueue the new log
		last_log.text = text;
		last_log.UID = (uint32)*UID;
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
