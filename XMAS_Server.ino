#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif

#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>

#include "Ornament.h"

#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#include "Syslog.h"

// Syslog server connection info
#define SYSLOG_SERVER "172.16.10.100"
#define SYSLOG_PORT 514

// This device info
#define DEVICE_HOSTNAME "xmas-server"
#define APP_NAME "xmas-app"

//The udp library class
WiFiUDP udpClient;

Syslog syslog(udpClient, SYSLOG_SERVER, SYSLOG_PORT, DEVICE_HOSTNAME, APP_NAME, LOG_KERN);

const char * udpAddress = "172.16.10.100";
const int udpPort = 514;


ornament Train;
ornament Snowman;


AsyncWebServer server(80); // server port 80
WebSocketsServer websockets(81);

bool autoMode = false;
bool alternateMode = false;

long alternateDelay = 5000;
unsigned long alternateTimeNow = 0;

long updateDelay = 10000;
unsigned long updateTimeNow = 0;


int LED1_status = 0;
int LED2_status = 0;
int LED3_status = 0;
int LED4_status = 0;

const char *PWD = "4Xoozzop";
const char *SSID = "127.0.0.1";

char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<style>

#titletext {
  /* CSS by GenerateCSS.com */ 
 font-family: Georgia; 
 font-size: 89px; 
 font-style: normal; 
 font-variant: normal; 
 font-weight: normal; 
}

#headertext {
  /* CSS by GenerateCSS.com */ 
 font-family: Georgia; 
 font-size: 89px; 
 font-style: normal; 
 font-variant: normal; 
 font-weight: normal; 
}

.button_green,.button_red {
  
  color: #ffffff;
  font-size: 1.3em;
  font-weight: 600;
  position: relative;
  outline: none;
  border-radius: 50px;
  display: flex;
  justify-content: center;
  align-items: center;
  cursor: pointer;
  height: 150px;
  width: 250px;
}

.button_green {background-color: #00BF00;}
.button_red {background-color: #C50000;}
  
}

</style>

<script>

var connection = new WebSocket('ws://'+location.hostname+':81/');

var button_1_status = 0;
var button_2_status = 0;
var button_3_status = 0;
var button_4_status = 0;

function button_1_on()
{
  button_1_status = 1; 
  send_data();
}

function button_1_off()
{
  button_1_status = 0;
  send_data();
}

function button_2_on()
{
  button_2_status = 1; 
  send_data();
}

function button_2_off()
{
  button_2_status = 0;
  send_data();
}

function button_3_on()
{
  button_3_status = 1; 
  send_data();
}

function button_3_off()
{
  button_3_status = 0;
  send_data();
}

function button_4_on()
{
  button_4_status = 1; 
  send_data();
}

function button_4_off()
{
  button_4_status = 0;
  send_data();
}

function send_data()
{
  var full_data = '{"LED1" :'+button_1_status+',"LED2":'+button_2_status+',"LED3":'+button_3_status+',"LED4":'+button_4_status+'}';
  connection.send(full_data);  
}


</script>
<body>

<center>
<div id="titletext">XMAS v0.02</div>
<br>
<div id="headertext">Snowman
<button class="button_green" onclick="button_1_on()">On</button><button class="button_red" onclick="button_1_off()">Off</button>
</div>

<div id="headertext">Train
<button class="button_green" onclick="button_2_on()">On</button><button class="button_red" onclick="button_2_off()">Off</button>
</div>

<div id="headertext">Random
<button class="button_green" onclick="button_3_on()">On</button><button class="button_red" onclick="button_3_off()">Off</button>
</div>

<div id="headertext">Alternate
<button class="button_green" onclick="button_4_on()">On</button><button class="button_red" onclick="button_4_off()">Off</button>
</div>


</center>
</body>
</html>
)=====";

void doLog(String message) {

  String fullMessage = "[Today] [XMAS-Server]" + message;
  if ( fullMessage.length() < 80 ) {
   
    char Buf[100];
    fullMessage.toCharArray(Buf, 100);
  
    udpClient.beginPacket(udpAddress,udpPort);
    udpClient.printf(Buf);
    udpClient.endPacket();  
  }
}

//void notFound(AsyncWebServerRequest *request)
//{
//  request->send(404, "text/plain", "Page Not found");
//}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);

      DynamicJsonDocument doc(200);
      // deserialize the data
      DeserializationError error = deserializeJson(doc, message);
      // parse the parameters we expect to receive (TO-DO: error handling)
      // Test if parsing succeeds.
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }
    
    LED1_status = doc["LED1"];
    LED2_status = doc["LED2"];
    LED3_status = doc["LED3"];
    LED4_status = doc["LED4"];
    
    if ( Train.ledStatus != doc["LED1"] ) {
      Train.toggle();
      autoMode = false;
      doLog("Toggle Train via browser");
    }

    if ( Snowman.ledStatus != doc["LED2"] ) {
      Snowman.toggle();
      autoMode = false;
      doLog("Toggle Snowman via browser");
    }

    if ( LED3_status ) {
      autoMode = true;
      alternateMode = false;
      doLog("Setting Auto Mode ON via browser");
      LED4_status = false;
    }

    if ( LED4_status ) {
      Snowman.turnOff();
      Train.turnOn();
      doLog("SHOULD BE REVERSE NOW");
//      delay(10000);
      
      autoMode = false;
      alternateMode = true;
      doLog("Setting Alternate ON via browser");
    }
  }
}



void setup(void) {

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("xmasserver");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  }); 
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  
  Train.LED = 14;
  Train.setInterval();
  Train.Setup();
  
  Snowman.LED = 4;
  Snowman.setInterval();
  Snowman.Setup();
  
  
  Serial.begin(115200);
  
  Serial.println("Connecting to ");
  Serial.println(SSID);
  
  WiFi.begin(SSID, PWD);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("X");
    delay(500);
    count++;
    if (count > 30) {
      ESP.restart();
    }
  }

  Serial.println(WiFi.localIP());

  if (MDNS.begin("ESP")) { //esp.local/
    Serial.println("MDNS responder started");
  }

  server.on("/", [](AsyncWebServerRequest * request)
  { 
   
  request->send_P(200, "text/html", webpage);
  });


//  server.onNotFound(notFound);

  server.begin();  // it will start webserver
  websockets.begin();
  websockets.onEvent(webSocketEvent); // 

}

void loop(void) {
  websockets.loop();
  ArduinoOTA.handle();


  // 10 second update
  if(millis() >= updateTimeNow + updateDelay) {
    updateTimeNow += updateDelay;
    doLog ("Hello World");
    doLog ("Alternate " + String(alternateMode));
  }
  
  
  
  if (autoMode) {
    Snowman.checkStatus();
    Train.checkStatus();
  }

  if (alternateMode) {
    if(millis() >= alternateTimeNow + alternateDelay) {
      alternateTimeNow += alternateDelay;
      if (Snowman.ornStatus()) {
        Snowman.turnOff();
        Train.turnOn();
      } else {
        Snowman.turnOn();
        Train.turnOff();        
      }
    }
  }
}
