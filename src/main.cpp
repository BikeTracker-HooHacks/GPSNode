#include <TinyGPS++.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
void displayInfo();
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
String msg = "";
long lastLat = 0;
long lastLng = 0;
bool send = true;
int tim = 0;
DynamicJsonDocument doc(200);


// The TinyGPS++ object
TinyGPSPlus gps;
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );



void sendMessage() {
  /*String msg = "Hello from node ";
  msg += mesh.getNodeId();*/
  if (msg != NULL){
    if(send){
      
      mesh.sendBroadcast( msg );
      taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
      Serial.println(msg);
    }
  }
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}


void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXPin, TXPin);
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

 
}

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  if (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
  
    mesh.update();
  
    userScheduler.addTask( taskSendMessage );
    taskSendMessage.enable();
  
  
  msg = "";
}

void displayInfo()
{
  
  msg = msg + "'mac':'"+ WiFi.macAddress();
  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.value());
    /*Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());*/
    msg = msg + "','date':'" + gps.date.year() + "-" + gps.date.month() + "-" + gps.date.day();
    doc["month"] = gps.date.month();
    doc["day"] = gps.date.day();
    doc["year"] = gps.date.year();
  }
  else
  {
    Serial.print(F("Invalid date"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    Serial.print(gps.time.value()); 
    if (gps.time.minute() == tim){
      send = false;
      tim = gps.time.minute();
    }
    else{
      send = true;
       msg = msg + "," + "'time':'" + gps.time.hour() + ":" + gps.time.minute() + ":" + gps.time.second() +"',";
       tim = gps.time.minute();
       doc["hour"] = gps.time.hour();
      doc["minute"] = gps.time.minute();
      doc["second"] = gps.time.second();
      
    }
  }

  
    
    Serial.print(F("Location:")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
    if (lastLat == gps.location.lat() && lastLng == gps.location.lng()){
      send = false;
    }
    else{
      
      String la = String(gps.location.lat(),5);
      String lo = String(gps.location.lng(),5);
      msg = msg + "'lat':'" + la + "','long':'" + lo + "'";
      lastLat = gps.location.lat();
      lastLng = gps.location.lng();
      doc["lat"] = gps.location.lat();
      doc["lng"] = gps.location.lng();    }

  }
  else
  {
    Serial.println(F("Invalid Location"));
  }
   
   
  
  
  
  
  
}