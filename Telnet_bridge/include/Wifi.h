#ifndef WIFI_H
#define WIFI_H

void setupOTA(const char* password);
void startWifi(const char* password);
void startAP(const char* password);
void handleOTA();
bool wifiConnected();
void handleCaptivePortal();

#endif