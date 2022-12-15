#include "Arduino.h"

#ifndef HEXPRINT_H
#define HEXPRINT_H

String hexArray(const uint8_t* hex, uint8_t len)
{
	String s;
  for (int i = 0; i < len; i++)
  {
    if (hex[i] < 0x10) s += '0';
    s += String(hex[i], HEX);
    if (i < len-1) s += ':';
  }
  return s;
}

#endif //HEXPRINT_H
