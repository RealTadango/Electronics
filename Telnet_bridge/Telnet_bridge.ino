#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <String.h>


struct Network
{
  char  ssid[50];
  char  password[50];
};

struct Config
{
  char apSsid[50];
  char apPassword[50];
  int serialSpeed;
  Network networks[5];
} config = 
{
  "TelnetBridge",
  "TelnetBridge",
  57600
};

const int led = 2; //LED_BUILTIN

WiFiServer server(23);
WiFiClient telnetClient;
ESP8266WebServer webserver (80);

void checkClient()
{
  if (server.hasClient())
  {
    if(telnetClient.connected())
    {
      telnetClient.stop();
    }
    
    Serial.println("New client!");
    telnetClient = server.available();
  }
}

void checkClientData()
{
  if(telnetClient.connected())
  {
    digitalWrite(led, HIGH);

    size_t len = telnetClient.available();

    if(len)
    {
      if(len > 128)
      {
        len = 128;
      }

      char * buf = (char *) malloc(len);
      
      len = telnetClient.readBytes(buf, len);

      Serial.write(buf, len);

      free(buf);
    }
  }
  else
  {
    digitalWrite(led, LOW);
  }
}

void checkSerialData()
{
  size_t len = Serial.available();

  if(len)
  {
    if(len > 128)
    {
      len = 128;
    }

    char * buf = (char *) malloc(len);

    len = Serial.readBytes(buf, len);
    
    if(telnetClient.connected())
    {
      telnetClient.write_P(buf, len);
    }

    free(buf);
  }
}

void setup(void)
{
  SPIFFS.begin();
  
  bool configOk = loadConfig();
    
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  
  Serial.begin(config.serialSpeed);
  Serial.println("");

  if(!configOk)
  {
    Serial.println("Config not loaded, using defaults");
  }

  connectWifi();

  Serial.println("Starting webserver");
  setupWebserver();
  
  Serial.println("Starting serial gateway on port 23");
  server.begin();
  digitalWrite(led, LOW);
}

void loop(void)
{
  checkClient();
  checkClientData();
  checkSerialData();
  webserver.handleClient();
}

