#include "Arduino.h"

#ifndef TRUERANDOM_H
#define TRUERANDOM_H

#if defined(__SAM3X8E__)

void TrueRandomSetup() {
	pmc_enable_periph_clk(ID_TRNG);
  	trng_enable(TRNG);
}

uint32_t TrueRandomGet32() {
	return trng_read_output_data(TRNG);
}

#elif defined(ESP8266)

void TrueRandomSetup() {
	// nothing to do
}

uint32_t TrueRandomGet32() {
	return RANDOM_REG32;
}

#endif

void TrueRandomGet64(uint8_t* ptr) {
	uint32_t rnd = TrueRandomGet32();
	memcpy(ptr, &rnd, 4);
	rnd = TrueRandomGet32();
	memcpy(ptr+4, &rnd, 4);
}

#endif //TRUERANDOM_H
