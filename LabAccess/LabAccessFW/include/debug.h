#include <Arduino.h>

#ifndef DEBUG_FLAG
#define DEBUG_FLAG "[DEBUG]"
#endif

#ifdef LAB_ACCESS_DEBUG
#define DPRINTF(fmt, ...) Serial.printf_P(DEBUG_FLAG " " fmt, ##__VA_ARGS__)

#warning "Debugging enabled!"
#else
#define DPRINTF(FMT, ...) // empty
#endif

