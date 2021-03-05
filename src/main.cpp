#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJSON.h>
#include <AsyncElegantOTA.h>

//#define Wifi_AP_Access
#ifdef Wifi_AP_Access
const char* ssid = "ESP8266Test";
const char* password = "";
#else
const char* ssid = "SKYPEMHG";
const char* password = "8NHetSWQAJ75";
#endif

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
String strLastMessageSent = "";
int PWMTemp = 0;

int controlledAnalogRead()
{ //read Analog input max once every 100ms 
  //or Wifi disconnects!  
  static unsigned long ulLastRead = 0;
  static int iLastValue = 0;
  if(ulLastRead < millis())
  {
    ulLastRead = millis()+100;
    iLastValue = analogRead(A0);
  }
  return(iLastValue);
}

// Initialize WiFi
void initWiFi() {

#ifdef Wifi_AP_Access
    WiFi.mode(WIFI_AP);
    WiFi.begin(ssid, password);
    Serial.println("AP Mode, ip 192.168.4.1");
#else    
// Connect to existing network
  WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);  }
    Serial.println(WiFi.localIP());
#endif
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) { 
    data[len] = 0;
    StaticJsonDocument<100> jsonReceived;
    deserializeJson(jsonReceived,(char*)data);
     // setup IO from message received
      digitalWrite(D0,jsonReceived["D0"]);
      digitalWrite(D1,jsonReceived["D1"]);
      //save pwm value, it cant be read back from IO pin
      PWMTemp = jsonReceived["D4"];
      analogWrite(D4,PWMTemp);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
 void *arg, uint8_t *data, size_t len) {
 switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      //force server to send current status to browser(s)
      strLastMessageSent="";
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
    break;
  }
}

void sendMessage()
  {
  StaticJsonDocument<100> jsonSend;
  //set up message from IO and send to browser(s)
    jsonSend["D0"] = digitalRead(D0);  
    jsonSend["D1"] = digitalRead(D1);
    jsonSend["D2"] = digitalRead(D2);
    jsonSend["D3"] = digitalRead(D3);
    jsonSend["D4"] = PWMTemp;
    //don't read Analogue directly
    jsonSend["A0"] = controlledAnalogRead();
  
  String strNewMessage = "";
  serializeJson(jsonSend,strNewMessage);
  //only send if the contents have changed from previous message
  if (strNewMessage != strLastMessageSent)
    {
        ws.textAll(strNewMessage);
        strLastMessageSent = strNewMessage;
    }
  }
void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(D0,OUTPUT);
  pinMode(D1,OUTPUT);
  pinMode(D2,INPUT_PULLUP);
  pinMode(D3,INPUT_PULLUP);
  pinMode(D4,OUTPUT);
  //A0 Analogue input 

  LittleFS.begin();
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  initWiFi();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html",false); });
  server.serveStatic("/", LittleFS, "/");
  AsyncElegantOTA.begin(&server);
  server.begin();
}  
void loop() {
  ws.cleanupClients();
  sendMessage(); 
  AsyncElegantOTA.loop();
}

