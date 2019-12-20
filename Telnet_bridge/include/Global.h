#include <ArduinoJson.h>

#ifndef GLOBAL_H
#define GLOBAL_H

// #ifdef DEBUG_ESP_PORT
// #define DEBUG_MSG(...) DEBUG_ESP_PORT.println(__VA_ARGS__ )
// #else
// #define DEBUG_MSG(...)
// #endif

#define CONFIG_Size 2000

extern DynamicJsonDocument configData;

#endif