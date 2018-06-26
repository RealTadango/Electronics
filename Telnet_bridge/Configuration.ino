
bool loadConfig()
{
    File configFile = SPIFFS.open("/config", "r");
    if (!configFile)
    {
      return false;
    }
    
    size_t size = configFile.size();

    configFile.readBytes((char*)&config, size);
    configFile.close();
    
    return true;
}

void saveConfig()
{
    File configFile = SPIFFS.open("/config", "w+");
    configFile.write((unsigned char*)&config, sizeof(config));
    configFile.close();
}

void connectWifi()
{
  Serial.println("Setup WiFi.....");

  WiFi.softAP(config.apSsid, config.apPassword);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP SSID: ");
  Serial.println(config.apSsid);
  Serial.print("IP address: ");
  Serial.println(myIP);

  //FIX Buggie wifi?
  //WiFi.begin("UenH", "UenHGeheimWW");
  //delay(500);

  Serial.print("Networks: ");
  for(int i = 0; i < sizeof(config.networks) / sizeof(Network); i++)
  {
    Network net = config.networks[i];

    if(net.ssid[0])
    {
      Serial.print(net.ssid);
      Serial.print(", ");
    }
  }

  Serial.println("");
  
  for(int i = 0; i < sizeof(config.networks) / sizeof(Network); i++)
  {
    Network net = config.networks[i];

    if(net.ssid[0])
    {
      Serial.print("Connecting to ");
      Serial.println(net.ssid);
  
      WiFi.begin(net.ssid, net.password);
      int timeout = millis() + 20 * 1000;
    
      while (WiFi.status() != WL_CONNECTED && millis() < timeout) 
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
  
      if(WiFi.status() == WL_CONNECTED)
      {
        Serial.print("Connected to ");
        Serial.println(net.ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        break;
      }
      else
      {
        Serial.print("Failed to connect to ");
        Serial.println(net.ssid);
      }
    }
  }
}

