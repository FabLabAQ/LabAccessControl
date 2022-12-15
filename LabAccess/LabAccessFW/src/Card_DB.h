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

#ifndef CARD_DB_H_
#define CARD_DB_H_

#include <LittleFS.h>
#include <MFRC522.h>
#include <Arduino.h>
#include "hex_utils.h"
#include "string.h"

#include "logging.h"
#include "constants.h"

#define DEBUG_FLAG "CARD_DB"
#include "la_debug.h"

// start and stop sequences with the same 8 char length as UIDs
PGM_STR UIDstartSequence[] = "UIDSTART";
PGM_STR UIDstopSequence[] =  "UID_STOP";

void reconnect() {
	if (!redirect.connected()) {

		int res1 = redirect.connect(script_server_host, 443);
		int res2;
		if (res1)
			res2 = LOG(LOG_MSG_SERVER_RECONNECTION);
		LA_DPRINTF("Connecting to %s, result %d, log result %d\n", script_server_host, res1, res2);
	}
}

bool cardExistsInDB(const byte* UID) {
	// 1 char for "/", 8 chars for uid string, 1 terminating char
	char path[2*uid_len+2] = "";
	// append uid in characters to the path
	hex_to_char(UID, uid_len, path);
	LA_DPRINTF("Searching for %s\n", path);
	// check if card exists
	return LittleFS.exists(path);
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
			Dir dir = LittleFS.openDir("/");
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
					LittleFS.remove(fileName);
				}
			}
			// --- Now add the new UIDs to the DB
			// loop until the other end sequence is reached
			while(!stringUID.endsWith(UIDstartSequence)) {
				// this loop can take some time
				yield();
				// create new file and then close it to save, name it with the last UID in the string
				LittleFS.open(stringUID.substring(stringUID.length()-8), "w").close();
				// remove the UID from the end of the string
				stringUID.remove(stringUID.length()-8);
			}
			stringUID.remove(0);
		}
		LOG(LOG_MSG_DB_UPDATED);
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
		LA_DPRINTF("-------- UIDs received ---------\n%s\n", stringUID.c_str());
		stringUID.trim();
		// check if we got a valid UID string
		if(stringUID.startsWith(UIDstartSequence) && stringUID.endsWith(UIDstopSequence)) {
			// ok, so remove the stop sequence (last 8 chars)
			stringUID.remove(0,sizeof(UIDstartSequence)-1);
			LA_DPRINTF("----Received valid UIDs----\n%s\n",stringUID.c_str());
			//---------- First remove UIDs not in the string from the DB ----------
			// TODO: check if removing files while iterating can cause problems
			// e.g. removing the current file makes a jump to the next
			// if so, place bool found in a nested loop to jump an iteration of next
			// TODO: check if removing a file alterates file order, if so restart

			// open directory to iterate over files
			Dir dir = LittleFS.openDir("");
			while (dir.next()) {
				// this loop can take some time
				yield();
				// get the current file name
				String fileName = dir.fileName();
				if (fileName == config_filename) {
					LA_DPRINTF("Skipping config file\n");
					continue;
				}
				String substring;
				int i = 0;
				bool found = false;
				// iterate over the new UID list received

				do {
					yield();
					// get the last UID in the string
					substring = stringUID.substring(i,uid_len*2);
					// if the card in DB is not present in the new UID list remove it
					if (substring == fileName) {
						found = true;
						stringUID.remove(i, uid_len*2);
						LA_DPRINTF("%s\n", stringUID.c_str());
					}
					i+=(uid_len*2);
				} while (!(substring.equals(UIDstopSequence) || found));
				// if the UID has not been found in the string, then remove it
				if (!found) {
					LittleFS.remove(fileName);
					LA_DPRINTF("Removed %s\n", fileName.c_str());
				}
			}
			// --- Now add the new UIDs to the DB
			// loop until the other end sequence is reached
			while(!stringUID.startsWith(UIDstopSequence)) {
				// this loop can take some time
				yield();
				// create new file and then close it to save, name it with the last UID in the string
				String fileName = stringUID.substring(0,uid_len*2);
				File newFile = LittleFS.open(fileName, "w");
				newFile.close();
				LA_DPRINTF("Added %s\n", fileName.c_str());
				// remove the UID from the end of the string
				stringUID.remove(0,uid_len*2);
				LA_DPRINTF("%s\n", stringUID.c_str());
			}
			stringUID.remove(0);
			LA_DPRINTF("%s\n", stringUID.c_str());

			LOG(LOG_MSG_DB_UPDATED);
			LA_DPRINTF("Update complete.\n");
		}

	}
}
#endif

#endif /* CARD_DB_H_ */
