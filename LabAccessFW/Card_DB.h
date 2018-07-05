/*
 * Card_DB.h
 *
 *  Created on: 21 giu 2018
 *      Author: Luca Anastasio
 */

#ifndef CARD_DB_H_
#define CARD_DB_H_

#include <FS.h>
#include <MFRC522.h>
#include <Arduino.h>
#include "hex_utils.h"
#include "secret_keys.h"
#include "string.h"
#include "Debug.h"
#include "logging.h"

PGM_STR accessScriptURL[] = "/macros/s/" SECRET_ACCESS_SCRIPT_ID "/exec?getUIDlist";
// start and stop sequences with the same 8 char length as UIDs
PGM_STR UIDstartSequence[] = "UID_START";
PGM_STR UIDstopSequence[] =  "UID__STOP";

void reconnect() {
	if (!redirect.connected()) {
		DPRINT("Connecting to script_server_host, result: ");
		bool connected = redirect.connect(script_server_host, 443);
		if (connected) LOG(SERVER_RECONNECTION_TEXT);
		DPRINTLN(connected);
	}
}

bool cardExistsInDB(const byte* UID) {
	// 1 char for "/", 8 chars for uid string, 1 terminating char
	char path[10] = "/";
	// append uid in characters to the path
	hex_to_char(UID, 4, path+1);
	// check if card exists
	return SPIFFS.exists(path);
}

//#define REVERSE_DB_SEARCH

#ifdef REVERSE_DB_SEARCH
void updateDB() {
	if (redirect.connected()) {
		redirect.GET(accessScriptURL, script_server_host);
		String stringUID = redirect.getResponseBody();
		// check if we got a valid UID string
		if(stringUID.startsWith(UIDstartSequence) && stringUID.endsWith(UIDstopSequence)) {
			// ok, so remove the stop sequence (last 8 chars)
			stringUID.remove(stringUID.length()-8);
			//---------- First remove UIDs not in the string from the DB ----------
			// TODO: check if removing files while iterating can cause problems
			// e.g. removing the current file makes a jump to the next
			// if so, place bool found in a nested loop to jump an iteration of next
			// TODO: check if removing a file alterates file order, if so restart

			// open directory to iterate over files
			Dir dir = SPIFFS.openDir("/");
			while (dir.next()) {
				// this loop can take some time
				yield();
				// get the current file name
				String fileName = dir.fileName();
				String substring;
				int i = stringUID.length() -8;
				bool found = false;
				// iterate over the new UID list received

				do {
					// get the last UID in the string
					substring = stringUID.substring(i,8);
					i-=8;
					// if the card in DB is not present in the new UID list remove it
					if (substring == fileName) {
						found = true;
						stringUID.remove(i,8);
					}
				} while (!(substring.equals(UIDstartSequence) || found));
				// if the UID has not been found in the string, then remove it
				if (!found) {
					SPIFFS.remove(fileName);
				}
			}
			// --- Now add the new UIDs to the DB
			// loop until the other end sequence is reached
			while(!stringUID.endsWith(UIDstartSequence)) {
				// this loop can take some time
				yield();
				// create new file and then close it to save, name it with the last UID in the string
				SPIFFS.open(stringUID.substring(stringUID.length()-8), "w").close();
				// remove the UID from the end of the string
				stringUID.remove(stringUID.length()-8);
			}
			stringUID.remove(0);
		}
		LOG(DB_UPDATED_TEXT);
		DPRINTLN("DB updated");
	}
}
#else
void updateDB() {
	// if connection is alive
	if (redirect.connected()) {
		// get data form spreadsheet
		redirect.GET(accessScriptURL, script_server_host);
		String stringUID = redirect.getResponseBody();
		// check if we got a valid UID string
		if(stringUID.startsWith(UIDstartSequence) && stringUID.endsWith(UIDstopSequence)) {
			// ok, so remove the stop sequence (last 8 chars)
			stringUID.remove(0,8);
			//---------- First remove UIDs not in the string from the DB ----------
			// TODO: check if removing files while iterating can cause problems
			// e.g. removing the current file makes a jump to the next
			// if so, place bool found in a nested loop to jump an iteration of next
			// TODO: check if removing a file alterates file order, if so restart

			// open directory to iterate over files
			Dir dir = SPIFFS.openDir("/");
			while (dir.next()) {
				// this loop can take some time
				yield();
				// get the current file name
				String fileName = dir.fileName();
				String substring;
				int i = 0;
				bool found = false;
				// iterate over the new UID list received

				do {
					// get the last UID in the string
					substring = stringUID.substring(i,8);
					i+=8;
					// if the card in DB is not present in the new UID list remove it
					if (substring == fileName) {
						found = true;
						stringUID.remove(i,8);
					}
				} while (!(substring.equals(UIDstopSequence) || found));
				// if the UID has not been found in the string, then remove it
				if (!found) {
					SPIFFS.remove(fileName);
				}
			}
			// --- Now add the new UIDs to the DB
			// loop until the other end sequence is reached
			while(!stringUID.startsWith(UIDstopSequence)) {
				// this loop can take some time
				yield();
				// create new file and then close it to save, name it with the last UID in the string
				SPIFFS.open(stringUID.substring(0,8), "w").close();
				// remove the UID from the end of the string
				stringUID.remove(0,8);
			}
			stringUID.remove(0);
		}
		LOG(DB_UPDATED_TEXT);
		DPRINTLN("DB updated");
	}
}
#endif

#endif /* CARD_DB_H_ */
