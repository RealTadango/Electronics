#include <Arduino.h>
#include <Config.h>
#include <Global.h>
#include <Wifi.h>
#include <Debug.h>
#include <FS.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

ESP8266WebServer webserver (80);

void setupConfig() {
  SPIFFS.begin();
  
  bool configOk = loadConfig();

  if(!configOk)
  {
    debugError("Config not loaded, using defaults\n");
  }

  const char* apPassword = configData["apPassword"];

  setupDebug(apPassword);
  startWifi(apPassword);

  debugInfo("Starting webserver\n");
  setupWebserver();
}

bool loadConfig() {
  EEPROM.begin(CONFIG_Size);

  String configDataString = "";

  int i = 0;
  byte b = EEPROM.read(i);

 
  while(b != 0 && i < CONFIG_Size) {
      configDataString += (char)b;
      i++;
      b = EEPROM.read(i);
  }

  EEPROM.end();

  DeserializationError error = deserializeJson(configData, configDataString);

  if (error == DeserializationError::Ok)
  {
    return true;
  }
  else
  {
    configData["serialSpeed"] = 115200;
    saveConfig();
    return false;
  }
}

void saveConfig() {
  char content[CONFIG_Size];
  serializeJson(configData, content);

  debugInfo("Config data: %s\n", content);
  
  EEPROM.begin(CONFIG_Size);

  for(int i = 0; i < CONFIG_Size; i++)
  {
    EEPROM.write(i, content[i]);
  }
  
  EEPROM.end();
}

String getContentType(String filename) {
  if(webserver.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void handleFileRequest(String fileName) {
  String contentType = getContentType(fileName);

  if(SPIFFS.exists(fileName))
  {
    File file = SPIFFS.open(fileName, "r");
    webserver.streamFile(file, contentType);
    file.close();
  }
  else
  {
    webserver.send ( 404, "text/html", "File " + fileName + " not found" );
  }
}

void setConfig() {
  const String newConfig = webserver.arg("plain");
  deserializeJson(configData, newConfig);
  saveConfig();
  webserver.send ( 200, "application/json", "true" );

  const char* apPassword = configData["apPassword"];
  startWifi(apPassword);

  int serialSpeed = configData["serialSpeed"];
  Serial.begin(serialSpeed);
}

void getStatus() {
  String content = "{";

  content += " \"apIP\": \"" + WiFi.softAPIP().toString() + "\"";
  content += ", \"networkIP\": \"" + WiFi.localIP().toString() + "\"";
  content += ", \"rssi\": \"" + String(WiFi.RSSI()) + "\"";
  
  content += "}";

  webserver.send ( 200, "application/json", content );
}

void getConfig() {
  char content[CONFIG_Size];
  serializeJson(configData, content);

  webserver.send ( 200, "application/json", content );
}

void setupWebserver() {
  //Static resources
  webserver.on("/jquery.js", []() { handleFileRequest("/jquery.js"); });
  webserver.on("/script.js", []() { handleFileRequest("/script.js"); });
  webserver.on("/style.css", []() { handleFileRequest("/style.css"); });

  //Api
  webserver.on("/setconfig", setConfig);
  webserver.on("/getconfig", getConfig);
  webserver.on("/getstatus", getStatus);

  //fallback to index (Captive portal connections)
  webserver.onNotFound ([]() { handleFileRequest("/index.htm"); });
  webserver.begin();
}

void handleConfigClient() {
  webserver.handleClient();
}
