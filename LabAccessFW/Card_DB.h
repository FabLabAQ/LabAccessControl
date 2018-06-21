/*
 * Card_DB.h
 *
 *  Created on: 21 giu 2018
 *      Author: luca
 */

#ifndef CARD_DB_H_
#define CARD_DB_H_

#include <FS.h>
#include <MFRC522.h>
#include <Arduino.h>
#include "hex_utils.h"

//                                             Helper functions
//----------------------------------------------------------------------------------------------------------

// MFRC522::Uid specialization

inline void uidToPath(char* path, const MFRC522::Uid UID) {
	hex_to_char(UID.uidByte, 4, path+1);
}

inline void pathToUID(MFRC522::Uid UID, const char* path) {
	char_to_hex(path+1, 4, UID.uidByte);
	UID.size = 4;
}

inline bool compare(const MFRC522::Uid UID1, const MFRC522::Uid UID2) {
	// check only first 4 bytes
	return ( *((uint32_t*)UID1.uidByte) == *((uint32_t*)UID2.uidByte));
}

// char* specialization

inline void uidToPath(char* path, const char* UID) {
	strcpy(path+1, UID);
}

inline void pathToUID(char* UID, const char* path) {
	strcpy(UID, path+1);
}

inline bool compare(const char* UID1, const char* UID2) {
	return (strcmp(UID1, UID2) == 0);
}

// uint8_t* specialization

inline void uidToPath(char* path, const uint8_t* UID) {
	hex_to_char(UID, 4, path+1);
}

inline void pathToUID(uint8_t* UID, const char* path) {
	char_to_hex(path+1, 4, UID);
}

inline bool compare(const uint8_t* UID1, const uint8_t* UID2) {
	// check only first 4 bytes
	return ( *((uint32_t*)UID1) == *((uint32_t*)UID2));
}

// TODO: check endianness

// uint32_t specialization

inline void uidToPath(char* path, const uint32_t UID) {
	hex_to_char((uint8_t*)&UID, 4, path+1);
}

inline void pathToUID(uint32_t UID, const char* path) {
	char_to_hex(path+1, 4, (uint8_t*)&UID);
}

inline bool compare(uint32_t UID1, uint32_t UID2) {
	return (UID1 == UID2);
}

// uint32_t* specialization

inline void uidToPath(char* path, const uint32_t* UID) {
	hex_to_char((uint8_t*)UID, 4, path+1);
}

inline void pathToUID(uint32_t* UID, const char* path) {
	char_to_hex(path+1, 4, (uint8_t*)UID);
}

inline bool compare(uint32_t* UID1, uint32_t* UID2) {
	return (*UID1 == *UID2);
}

//-----------------------------------------------------------------------------------------------------------------


template<typename T> bool fileNameMatch(const char* fileName, const T UID) {
	// 1 char for "/", 8 chars for uid string, 1 terminating char
	char path[10] = "/";
	// append uid in characters to the path
	uidToPath(path, UID);
	// check if name matches
	return strcmp(fileName, path) == 0;
}

template<typename T> bool cardExistsInDB(const T UID) {
	// 1 char for "/", 8 chars for uid string, 1 terminating char
	char path[10] = "/";
	// append uid in characters to the path
	uidToPath(path, UID);
	// check if card exists
	return SPIFFS.exists(path);
}

template<typename T> void addCardToDB(const T UID) {
	// 1 char for "/", 8 chars for uid string, 1 terminating char
	char path[10] = "/";
	// append uid in characters to the path
	uidToPath(path, UID);
	// create new file and then close it
	SPIFFS.open(path, "w").close();
}

template<typename T> void updateDB(T* UIDarray) {
	// first delete removed cards from DB
	Dir dir = SPIFFS.openDir("/");
	// TODO: check if removing files while iterating can cause problems
	// e.g. removing the current file makes a jump to the next
	// if so, place bool found in a nested loop to jump an iteration of next
	while (dir.next()) {
		T UID;
		pathToUID(UID, dir.fileName().c_str());
		uint16_t i = 0;
		bool found = false;
		// iterate over the new UID list received
		while(UIDarray[i] && !found) {
			found = compare(UIDarray[i], UID);
			i++;
		}
		// if the card in DB is not present in the new UID list remove it
		if (!found) {
			SPIFFS.remove(dir.fileName());
		}
	}

	// add new cards found in the UID list
	uint16_t i = 0;
	// iterate over the new UID list received
	while(UIDarray[i]) {
		if (!cardExistsInDB(UIDarray[i])) {
			addCardToDB(UIDarray[i]);
		}
		i++;
	}
}

#endif /* CARD_DB_H_ */
