
#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <EEPROM.h>

#define SKETCH_VERSION "1.0.0"

#define   MESH_PREFIX     "drift-x"
#define   MESH_PASSWORD   "your-secret-password"
#define   MESH_PORT       5555

#define ECHOPIN D6                            // Pin to receive echo pulse
#define TRIGPIN D7                           // Pin to send trigger pulse

#define LED_PIN 2               // Flash the LED connected to GPIO2


template <class DATATYPE, unsigned char SIZE> class PushPop {
  private:
    const unsigned char m_Size = SIZE;
    DATATYPE m_Array[SIZE];
    int m_Index;
  public:
    PushPop() {
      m_Index = -1;
    }
    int Size() {
      return m_Index;
    }
    void Clear() {
      m_Index = 0;
    };
    bool CanPush() {
      return (m_Index < m_Size - 1);
    }
    bool CanPop() {
      return (m_Index > -1);
    }
    void Push(DATATYPE what) {
      if (m_Index < m_Size) m_Array[++m_Index] = what;
    }
    DATATYPE Pop() {
      return m_Array[m_Index--];
    }
    DATATYPE Peek() {
      return m_Array[m_Index - 1];
    }
};

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
String NODE_ID = "node99"; //MAX 8
int tc = 0;
int frameCount = 0;
PushPop<int, 220> pp;
boolean recording = false;
int startTime;
int triggerZone = 170;
int acceptanceZone = 150;
int masterNode;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

int systemArmId = 0 ;
boolean systemArm = false;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void sonar() ; // Prototype so PlatformIO doesn't complain
void locate() ;

//Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskLocate(  30 , TASK_ONCE, &locate );
Task taskReadDistance(  30 , TASK_FOREVER, &sonar );

const char compile_date_file[] = __DATE__ " " __TIME__ " " __FILE__;


//void sendMessage() {
//  String msg = "Hello from " + NODE_ID + " mesh id: ";
//  msg += mesh.getNodeId();
//  msg += " myFreeMemory: " + String(ESP.getFreeHeap());
//  mesh.sendBroadcast(msg);
//
//  //  if (calc_delay) {
//  //    SimpleList<uint32_t>::iterator node = nodes.begin();
//  //    while (node != nodes.end()) {
//  //      mesh.startDelayMeas(*node);
//  //      node++;
//  //    }
//  //    calc_delay = false;
//  //  }
//  //
//  //  Serial.printf("Sending message: %s\n", msg.c_str());
//
//  //taskSendMessage.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
//}

void locate(){
  digitalWrite(LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  delay(500);                      // Wait for a second
  digitalWrite(LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(500); 

  digitalWrite(LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  delay(500);                      // Wait for a second
  digitalWrite(LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(500); 

  digitalWrite(LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  delay(500);                      // Wait for a second
  digitalWrite(LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(500); 

  digitalWrite(LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  delay(500);                      // Wait for a second
  digitalWrite(LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(500); 

  digitalWrite(LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  delay(500);                      // Wait for a second
  digitalWrite(LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(500); 

  digitalWrite(LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  delay(500);                      // Wait for a second
  digitalWrite(LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(500); 
}


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  //locate();
  StaticJsonDocument<200> input;
  deserializeJson(input, msg.c_str());
  String action = input["action"];

  if (action == "set_config")
  {
    triggerZone = input["triggerZone"];
    acceptanceZone = input["acceptanceZone"];
    masterNode = input["masterNode"];
  } else if (action == "ping")
  {
    JsonVariant _hr = input["hr"];
    JsonVariant _min = input["min"];
    JsonVariant _sec = input["sec"];
    JsonVariant _day = input["day"];
    JsonVariant _month = input["month"];
    JsonVariant _yr = input["yr"];
    
    struct tm tm;
    tm.tm_year = _yr.as<int>() - 1900;
    tm.tm_mon = _month.as<int>();
    tm.tm_mday = _day.as<int>();
    tm.tm_hour = _hr.as<int>();
    tm.tm_min = _min.as<int>();
    tm.tm_sec = _sec.as<int>();

    time_t t = mktime(&tm);
    printf("Setting time: %s", asctime(&tm));
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, NULL);
    
    time_t nowt = time(nullptr);
    struct tm * timeinfo;
    timeinfo = localtime(&nowt);
    
    StaticJsonDocument<200> doc;
    doc["msgType"] = "ping";
    doc["msg"] = "Hello from " + NODE_ID + " mesh id: " + mesh.getNodeId();
    doc["timeZone"] = tzname[0];
    doc["time"] = asctime(timeinfo);
    doc["name"] = NODE_ID;
    doc["nodeId"] = mesh.getNodeId();
    doc["freeMem"] = String(ESP.getFreeHeap());

    String json_string;
    serializeJson(doc, json_string);
    mesh.sendBroadcast(json_string);
  } else if (action == "read_config")
  {
    time_t now = time(nullptr);
    struct tm * timeinfo;
    timeinfo = localtime(&now);
    Serial.printf("%s %s", tzname[0], asctime(timeinfo));

    StaticJsonDocument<200> doc;
    doc["msgType"] = "config";
    doc["triggerZone"] = triggerZone;
    doc["acceptanceZone"] = acceptanceZone;
    doc["timeZone"] = tzname[0];
    doc["time"] = asctime(timeinfo);
    doc["version"] = SKETCH_VERSION;
    doc["compileData"] = compile_date_file;
    doc["name"] = NODE_ID;
    doc["nodeId"] = mesh.getNodeId();

    String json_string;
    serializeJson(doc, json_string);
    mesh.sendBroadcast(json_string);
  } else if (action == "arm_system")
  {
    systemArm = true;
    systemArmId = input["systemArmId"];
    StaticJsonDocument<200> doc;
    doc["systemArm"] = true;
    doc["name"] = NODE_ID;
    doc["nodeId"] = mesh.getNodeId();
    doc["msgType"] = "arm_system_ack";

    String json_string;
    serializeJson(doc, json_string);
    mesh.sendBroadcast(json_string);
  }
  else if (action == "dis_arm_system")
  {
    systemArm = false;
    systemArmId = 0;
    StaticJsonDocument<200> doc;
    doc["systemArm"] = false;
    doc["name"] = NODE_ID;
    doc["nodeId"] = mesh.getNodeId();
    doc["msgType"] = "disarm_system_ack";

    String json_string;
    serializeJson(doc, json_string);
    mesh.sendBroadcast(json_string);
  }
  else if (action == "locate")
  {
    taskLocate.enable();
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
  //  nodes = mesh.getNodeList();
  //
  //  Serial.printf("Num nodes: %d\n", nodes.size());
  //  Serial.printf("Connection list:");
  //
  //  SimpleList<uint32_t>::iterator node = nodes.begin();
  //  while (node != nodes.end()) {
  //    Serial.printf(" %u", *node);
  //    node++;
  //  }
  //  Serial.println();
  //  calc_delay = true;
}


void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}

void eepromBurn(String buffer, int n) {
  EEPROM.begin(8); 
  delay(10);
  for (int i = 0; i < 8; ++i) {
    EEPROM.write(n + i, buffer[i]);
  }
  EEPROM.commit();
}
//
String eepromReed(int min, int max) {
  EEPROM.begin(8); 
  delay(10); 
  String buffer;
  for (int i = min; i < max; ++i)
    if (isAlphaNumeric(EEPROM.read(i)))
      buffer += char(EEPROM.read(i));
  return buffer;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
  NODE_ID = eepromReed(0, 8);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  mesh.setRoot(false);
  mesh.initOTAReceive("otareceiver");

  //userScheduler.addTask( taskSendMessage );
  userScheduler.addTask( taskLocate );
  userScheduler.addTask( taskReadDistance );
  //taskSendMessage.enable();
  taskReadDistance.enable();
  taskLocate.enable();
  Serial.println("Ready to join mesh as "+NODE_ID+" ... ");
}



void loop() {
  // it will run the user scheduler as well
  mesh.update();
  //sonar();
}
