#include "IPAddress.h"
#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <TimeLib.h>

#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>


#define   MESH_PREFIX     "drift-x"
#define   MESH_PASSWORD   "your-secret-password"
#define   MESH_PORT       5555

#define HOSTNAME "HTTP_BRIDGE"
const long utcOffsetInSeconds = -4 * 3600;

bool shouldReboot = false;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

//long reqNumber = 0;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);

bool calc_delay = false;
SimpleList<uint32_t> nodes;

//file trafnser allocation
byte fileTransfer[1024 * 512]; // 512k buffer

// User stub
//void sendMessage() ; // Prototype so PlatformIO doesn't complain
//Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
//  String msg = "Hello from node ";
//  msg += mesh.getNodeId();
//  msg += " myFreeMemory: " + String(ESP.getFreeHeap());
//  mesh.sendBroadcast(msg);
//
//  if (calc_delay) {
//    SimpleList<uint32_t>::iterator node = nodes.begin();
//    while (node != nodes.end()) {
//      mesh.startDelayMeas(*node);
//      node++;
//    }
//    calc_delay = false;
//  }

  //Serial.printf("Sending message: %s\n", msg.c_str());
  //outputDateTime();
  //taskSendMessage.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
}

void setup() {
  Serial.begin(115200);

  //mesh.setDebugMsgTypes(ERROR | STARTUP | MESH_STATUS | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  //WIFI_STA no dns
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA);
  //mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_STA, 6);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  Serial.print("Configuring access point...");
  mesh.setRoot(true);
  mesh.setContainsRoot(true);

  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());

  //Async webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
    if (request->hasArg("BROADCAST")){
      String msg = request->arg("BROADCAST");
      broadcastToNodes(msg);
    }
  });

  // attach AsyncWebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // respond to GET requests on URL /heap
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  // upload a file to /upload
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200);
  }, onUpload);

  // Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n", index+len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.onNotFound(onRequest);
  server.onFileUpload(onUpload);
  server.onRequestBody(onBody);
  
  server.begin();

  Serial.println("HTTP server started");
}

void outputDateTime() {
  time_t now = time(nullptr);
  struct tm * timeinfo;  
  timeinfo = localtime(&now); 
  Serial.printf("%s %s", tzname[0], asctime(timeinfo));
}

void loop() {
  if(shouldReboot){
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
  if (Serial.available() > 0) {
    // read the incoming byte:
    String incomingByte = Serial.readString();
//    Serial.println(">" + incomingByte);
    serialController(incomingByte);
    
  }
  //server.handleClient();
  mesh.update();
  //sonar();
}
