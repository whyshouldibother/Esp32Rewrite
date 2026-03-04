#include <WebServer.h>
#include<WiFi.h>
#include "time.h"
#include "secrets.h"
#define relayPin 15
WebServer server(80);
IPAddress localIp(192,168,1,254);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress primaryDns(8,8,8,8);
IPAddress secondaryDns(8,8,4,4);
const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 19800;

inline void handleRoot(){
  server.send(200);
}
inline void handleToggle(){
  digitalWrite(relayPin, !digitalRead(relayPin));
  server.send(200, "text/plain", digitalRead(relayPin)? "OFF": "ON");
}
inline void handleStatus(){
  server.send(200, "text/plain", digitalRead(relayPin)? "OFF": "ON");
}
inline void toggle(bool status){
  digitalWrite(relayPin, status);
}
struct schedule{
  int hour; int minute; void(*task)();bool triggeredToday;
};
schedule tasks[]={
  {0, 0, [](){toggle(HIGH);}, false},
  {5,0,[](){toggle(LOW);}, false},
  {6, 30, [](){toggle(HIGH);}, false},
  {18, 0, [](){toggle(LOW);}, false},
};
void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(relayPin, OUTPUT);
  WiFi.config(localIp, gateway, subnet, primaryDns, secondaryDns);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  server.begin();
  server.on("/",HTTP_GET, handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/status", handleStatus);
  configTime(gmtOffsetSec, 0, ntpServer);
}
void loop() {
  server.handleClient();
  struct tm timeInfo; 
  if(!getLocalTime(&timeInfo)){
    Serial.println("Failed to get time");
    delay(500);
    Serial.print(".");
  }
  for(int i=0; i<4; i++){
    schedule &task = tasks[i];
    if(timeInfo.tm_hour == task.hour && timeInfo.tm_min == task.minute && !task.triggeredToday){
      task.task();
      task.triggeredToday = true;
    }
    if(timeInfo.tm_min != task.minute){
      task.triggeredToday = false;
    }
  }
}
