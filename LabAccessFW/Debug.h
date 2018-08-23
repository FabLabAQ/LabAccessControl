/*
 * Debug.h
 *
 *  Created on: 03 lug 2018
 *      Author: luca
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#pragma once

#define DEBUG

#ifdef DEBUG
  #define DSTART()				Serial.begin(115200)
  #define DPRINT(...)			Serial.print(__VA_ARGS__)
  #define DPRINTF(...)	Serial.printf(__VA_ARGS__)
  #define DPRINTLN(...)			Serial.println(__VA_ARGS__)
#else
  #define DSTART()
  #define DPRINT(...)
  #define DPRINTLN(...)
#endif

#endif /* DEBUG_H_ */
