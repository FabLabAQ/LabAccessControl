#include "Arduino.h"

#ifndef TRUERANDOM_H
#define TRUERANDOM_H

#if defined(__SAM3X8E__)

void TrueRandomSetup() {
	pmc_enable_periph_clk(ID_TRNG);
  	trng_enable(TRNG);
}

inline uint32_t TrueRandomGet32() {
	return trng_read_output_data(TRNG);
}

#elif defined(ESP8266)

void TrueRandomSetup() {
	// nothing to do
}

inline uint32_t TrueRandomGet32() {
	return RANDOM_REG32;
}

#elif defined(AVR)


void TrueRandomSetup() {
  
  const uint8_t seedPin = A0;
  uint8_t  seedBitValue  = 0;
  uint8_t  seedByteValue = 0;
  uint32_t seedWordValue = 0;

  for (uint8_t wordShift = 0; wordShift < 4; wordShift++)     // 4 bytes in a 32 bit word
  {
    for (uint8_t byteShift = 0; byteShift < 8; byteShift++)   // 8 bits in a byte
    {
      for (uint8_t bitSum = 0; bitSum <= 8; bitSum++)         // 8 samples of analog pin
      {
        seedBitValue = seedBitValue + (analogRead(seedPin) & 0x01);                // Flip the coin eight times, adding the results together
      }
      delay(1);                                                                    // Delay a single millisecond to allow the pin to fluctuate
      seedByteValue = seedByteValue | ((seedBitValue & 0x01) << byteShift);        // Build a stack of eight flipped coins
      seedBitValue = 0;                                                            // Clear out the previous coin value
    }
    seedWordValue = seedWordValue | (uint32_t)seedByteValue << (8 * wordShift);    // Build a stack of four sets of 8 coins (shifting right creates a larger number so cast to 32bit)
    seedByteValue = 0;                                                             // Clear out the previous stack value
  }
  randomSeed(seedWordValue);

}

inline uint32_t TrueRandomGet32() {
  return random();
}


#endif

void TrueRandomGet64(uint8_t* ptr) {
	uint32_t rnd = TrueRandomGet32();
	memcpy(ptr, &rnd, 4);
	rnd = TrueRandomGet32();
	memcpy(ptr+4, &rnd, 4);
}

#endif //TRUERANDOM_H
