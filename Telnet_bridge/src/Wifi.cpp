#include <Wifi.h>
#include <Global.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Debug.h>

#define AP_SSID "SerialBridge_"
#define OTA_Name "SerialBridge_"

IPAddress AP_IP(192,168,4,1);
DNSServer dnsServer;
const byte DNS_PORT = 53;

#define WIFI_TIMEOUT 15

void startWifi(const char* password) {
  debugInfo("Networks: ");
  for(int i = 0; i < 99; i++) {
    const char* ssid = configData["networks"][i]["ssid"];

    if(ssid == 0) {
      break;
    }
    
    debugInfo("%s, ", ssid);
  }
  debugInfo("\n");
  
  for(int i = 0; i < 99; i++)
  {
    const char* ssid = configData["networks"][i]["ssid"];
    if(ssid == 0) {
      break;
    }
    const char* password = configData["networks"][i]["password"];
    debugInfo("Connecting to %s\n", ssid);

    WiFi.begin(ssid, password);
    unsigned long timeout = millis() + (WIFI_TIMEOUT * 1000);
  
    while (WiFi.status() != WL_CONNECTED && millis() < timeout) 
    {
      delay(500);
    }

    if(WiFi.status() == WL_CONNECTED)
    {
      debugInfo("Connected to %s\n", ssid);
      break;
    }
    else
    {
      debugError("Failed to connect to %s\n", ssid);
    }
  }

  //If no initial connection, prevent WIFI connection after soft AP has started.
  if (!wifiConnected()) {
    WiFi.disconnect();
  }  

  startAP(password);
  setupOTA(password);
}

void setupOTA(const char* password) {
  char idChar[16];
  itoa(ESP.getChipId(), idChar, 10);

  char fullOTAName[strlen(OTA_Name)+strlen(idChar)+1];
  strcpy(fullOTAName, OTA_Name);
  strcat(fullOTAName, idChar);

  ArduinoOTA.setHostname(fullOTAName);
  ArduinoOTA.setPassword(password);

  ArduinoOTA.onStart([]() {
    if (ArduinoOTA.getCommand() == U_FLASH) {
      debugInfo("Start updating sketch\n");
    } else { // U_SPIFFS
      debugInfo("Start updating filesystem\n");
      SPIFFS.end();
    }
  });
  ArduinoOTA.onEnd([]() {
    debugInfo("\n End");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    debugError("OTA Error: ");
    if (error == OTA_AUTH_ERROR) {
      debugError("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      debugError("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      debugError("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      debugError("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      debugError("End Failed");
    }
  });

  ArduinoOTA.begin();
}

void startAP(const char* password) {
  char idChar[16];
  itoa(ESP.getChipId(), idChar, 10);

  char fullSSIDName[strlen(AP_SSID)+strlen(idChar)+1];
  strcpy(fullSSIDName, AP_SSID);
  strcat(fullSSIDName, idChar);

  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(fullSSIDName, password);

  dnsServer.start(DNS_PORT, "*", AP_IP);
}

void handleOTA() {
  ArduinoOTA.handle();
}

bool wifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void handleCaptivePortal() {
  dnsServer.processNextRequest();
}
