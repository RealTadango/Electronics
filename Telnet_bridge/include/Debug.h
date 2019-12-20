#include <Arduino.h>
#include <RemoteDebug.h>

#ifndef DEBUG_H
#define DEBUG_H

extern RemoteDebug Debug;

void setupDebug(const char* password);
void handleDebug();

#ifdef DEBUG_ESP_PORT
    #define debugInfo(...) debugI(__VA_ARGS__); DEBUG_ESP_PORT.printf(__VA_ARGS__)
    #define debugError(...) debugE(__VA_ARGS__); DEBUG_ESP_PORT.printf(__VA_ARGS__)
    #define debugDebug(...) debugD(__VA_ARGS__); DEBUG_ESP_PORT.printf(__VA_ARGS__)
    #define debugInfoMsg(...) Debug.println(__VA_ARGS__); DEBUG_ESP_PORT.println(__VA_ARGS__)
    #define debugErrorMsg(...) Debug.println(__VA_ARGS__); DEBUG_ESP_PORT.println(__VA_ARGS__)
    #define debugDebugMsg(...) Debug.println(__VA_ARGS__); DEBUG_ESP_PORT.println(__VA_ARGS__)
#else
    #define debugInfo(...) debugI(__VA_ARGS__)
    #define debugError(...) debugE(__VA_ARGS__)
    #define debugDebug(...) debugD(__VA_ARGS__)
    #define debugInfoMsg(...) Debug.println(__VA_ARGS__)
    #define debugErrorMsg(...) Debug.println(__VA_ARGS__)
    #define debugDebugMsg(...) Debug.println(__VA_ARGS__)
#endif
#endif
