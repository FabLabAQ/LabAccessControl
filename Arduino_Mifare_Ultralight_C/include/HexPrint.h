#include "Arduino.h"

#ifndef HEXPRINT_H
#define HEXPRINT_H

void printHexArray(const uint8_t* output, uint8_t len)
{
	String s;
  for (int i = 0; i < len; i++)
  {
    if (output[i] < 0x10) s += '0';
    s += String(output[i], HEX);
    if (i < len-1) s += ':';
  }
  Serial.println(s);
}

#endif //HEXPRINT_H
