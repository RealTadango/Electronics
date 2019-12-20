#include <Arduino.h>

#ifndef CONFIG_H
#define CONFIG_H

void setupConfig();
bool loadConfig();
void saveConfig();
String getContentType(String filename);
void handleFileRequest(String fileName);
void setConfig();
void getStatus();
void getConfig();
void setupWebserver();
void handleConfigClient();

#endif