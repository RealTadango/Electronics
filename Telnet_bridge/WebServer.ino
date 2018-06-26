

String getContentType(String filename){
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

void handleFileRequest(String fileName){
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

void setAPConfig()
{
  String ssid = webserver.arg("ssid");
  String password = webserver.arg("password");

  ssid.toCharArray(config.apSsid, ssid.length() + 1);
  password.toCharArray(config.apPassword, password.length() + 1);
  
  saveConfig();
  webserver.send ( 200, "application/json", "true" );

  WiFi.softAP(config.apSsid, config.apPassword);
}

void setNetwork()
{
  String ssid = webserver.arg("ssid");
  String password = webserver.arg("password");
  String index = webserver.arg("index");

  int networkIndex = index.toInt();

  ssid.toCharArray(config.networks[networkIndex].ssid, ssid.length() + 1);
  password.toCharArray(config.networks[networkIndex].password, password.length() + 1);
  
  saveConfig();
  webserver.send ( 200, "application/json", "true" );
}

void setSerialSpeed()
{
  String speed = webserver.arg("speed");

  config.serialSpeed = speed.toInt();

  webserver.send ( 200, "application/json", "true" );
  saveConfig();

  Serial.begin(config.serialSpeed);
}

void getStatus()
{
  String content = "{";

  content += " \"apIP\": \"" + WiFi.softAPIP().toString() + "\"";
  content += ", \"networkIP\": \"" + WiFi.localIP().toString() + "\"";
  
  content += "}";

  webserver.send ( 200, "application/json", content );
}

void getConfig()
{
  String content = "{";

  content += " \"apSsid\": \"" + String(config.apSsid) + "\"";
  content += ", \"apPassword\": \"" + String(config.apPassword) + "\"";
  content += ", \"serialSpeed\": \"" + String(config.serialSpeed) + "\"";

  bool first = true;

  //Networks
  content += ", \"networks\": [";
  for(int i = 0; i < sizeof(config.networks) / sizeof(Network); i++)
  {
    Network net = config.networks[i];

    if(!first)
    {
      content += ", ";  
    }
    
    content += "{ \"ssid\": \"" + String(net.ssid) + "\", \"password\": \"" + String(net.password) + "\" }";

    first = false;
  }

  content += "]";

  content += "}";

  webserver.send ( 200, "application/json", content );
}

void setupWebserver()
{
  webserver.on("/", []() { handleFileRequest("/index.htm"); });
  webserver.on("/jquery.js", []() { handleFileRequest("/jquery.js"); });
  webserver.on("/script.js", []() { handleFileRequest("/script.js"); });
  webserver.on("/style.css", []() { handleFileRequest("/style.css"); });
  webserver.on("/setap", setAPConfig);
  webserver.on("/setnetwork", setNetwork);
  webserver.on("/getconfig", getConfig);
  webserver.on("/getstatus", getStatus);
  webserver.on("/setserialspeed", setSerialSpeed);

  webserver.onNotFound ( []() { webserver.send ( 404, "text/plain", "Unknown request" ); });
 
  webserver.begin();
}

