#include <Arduino.h>
#include <Global.h>
#include <Config.h>
#include <Wifi.h>
#include <Debug.h>

long lastCheck = 0;
DynamicJsonDocument configData(CONFIG_Size);

const int led = 2; //LED_BUILTIN

WiFiServer server(24);
const int clientCount = 10;
WiFiClient *clients[clientCount];

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);

  setupConfig();

  int serialSpeed = configData["serialSpeed"];
  Serial.begin(serialSpeed);

  server.begin();
  digitalWrite(led, LOW);
}

void checkClient() {
  if (server.hasClient()) {
    WiFiClient newClient = server.available();
    debugInfo("New client!");

    for(int i = 0; i < clientCount; i++) {
      if(!clients[i] || !clients[i]->connected()) {
        clients[i] = new WiFiClient(newClient);
        debugInfo("Adding client to slot: %i", i);
        break;
      }    
    }
  }
}

void checkClientData()
{
  for(int i = 0; i < clientCount; i++) {
    if(clients[i] && clients[i]->connected()) {
      size_t len = clients[i]->available();
      if(len) {
        if(len > 128) {
          len = 128;
        }

        char * buf = (char *) malloc(len);
        len = clients[i]->readBytes(buf, len);
        Serial.write(buf, len);

        free(buf);
      }
    }
  }
}

void checkSerialData() {
  size_t len = Serial.available();

  if(len) {
    if(len > 128) {
      len = 128;
    }

    char * buf = (char *) malloc(len);
    len = Serial.readBytes(buf, len);
   
    for(int i = 0; i < clientCount; i++) {
      if(clients[i] && clients[i]->connected()) {
        clients[i]->write_P(buf, len);
      }
    }

    free(buf);
  }
}

void loop() {
  checkClient();
  checkClientData();
  checkSerialData();

  handleDebug();
  handleCaptivePortal();
  handleOTA();
  handleConfigClient();
}
