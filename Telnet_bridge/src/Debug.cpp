#include <Arduino.h>
#include <Debug.h>
#include <RemoteDebug.h>

RemoteDebug Debug;

#define HOST_NAME "SerialBridge"

void setupDebug(const char* password) {
    #ifdef DEBUG_ESP_PORT
        DEBUG_ESP_PORT.begin(115200);
    #endif

    Debug.begin(HOST_NAME);
    Debug.setResetCmdEnabled(true);
    Debug.showColors(true);
    Debug.showProfiler(true);
    Debug.setPassword(password);
}

void handleDebug() {
    Debug.handle(); 
}