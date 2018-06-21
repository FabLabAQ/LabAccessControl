/*
 * settings.h
 *
 *  Created on: 20 giu 2018
 *      Author: luca
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#define BOARD_SPECIALIZATION 0

#define PIN_INTERRUPT 4
#define PIN_RELAY 16
#define PIN_SWITCH 5
#define PIN_LED 2

#if BOARD_SPECIALIZATION == 0
	#define BOARD_NAME "LabAccess-MainDoor"
#endif

#endif /* SETTINGS_H_ */
