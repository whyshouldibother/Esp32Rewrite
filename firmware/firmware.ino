#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "time.h"
#include "secrets.h"
#include "tasks.h"
#include "constants.h"
#include "errorFlags.h"
#include "httpEndpoints.h"
#include "display.h"
#include "networking.h"
#include "sdCard.h"

unsigned long tmpTime = 0, prevScheduler = 0, prevWiFiCheck, prevToggleCheck =0;
bool prevToggle;
//Time Server
const char* ntpServer = "time.google.com";

uint8_t taskIndex = 0;

String tmpText = "";
String faces = "(0_0)";


inline void handleStats(AsyncWebServerRequest* request) {
  tmpTime = millis();
  uint8_t day = tmpTime / 864000000;
  uint8_t hour = (tmpTime % 86400000) / 3600000;
  uint8_t minute = (tmpTime % 3600000) / 60000;
  request->send(200, "application/json",
                "{\"Uptime\":{\"day\":" + String(day) + ", \"hour\":" + String(hour) + ", \"minutes\":" + String(minute) + "},\"Realtime\":" + (getLocalTime(&timeInfo) ? ("{\"day\":" + String(timeInfo.tm_mday) + ", \"hour\":" + String(timeInfo.tm_hour) + ", \"minutes\":" + String(timeInfo.tm_min) + "}") : "\"Time Error\"") + "}");
}
void toggle( int pin,bool status) {
  digitalWrite(pin, status);
  ws.textAll(status?"OFF":"ON");
}
inline void defaultSetup() {
  sortTasks();
  tmpTime = timeInfo.tm_hour * 60 + timeInfo.tm_min;
  taskIndex = 0;
  for (int i = 0; i < tasks.size(); i++) {
    if (tasks[i].totalMinutes() <= tmpTime) {
      taskIndex = i;
    } else {
      break;
    }
  }
  if (!tasks[taskIndex].triggeredToday) {
    tasks[taskIndex].runTask();
    tasks[taskIndex].triggeredToday = true;
  }
  display.setFont(u8g2_font_5x8_mr);
  clearSection(0, 10, 60, 20);
  tmpText = "Now>" + twoDigit(tasks[taskIndex].hour) + ":" + twoDigit(tasks[taskIndex].minute);
  display.drawStr(0, 20, tmpText.c_str());
  tmpText = "Next>" + twoDigit(tasks[(taskIndex + 1) % tasks.size()].hour) + ":" + twoDigit(tasks[(taskIndex + 1) % tasks.size()].minute);
  display.drawStr(0, 30, tmpText.c_str());
  display.setFont(u8g2_font_10x20_mr);
  clearSection(60, 10, 30, 20);
  display.drawStr(60, 30, digitalRead(relayPin) ? "OFF" : "ON");
  display.sendBuffer();
}
char ipAddr[19];
void setup() {

  //Set Relay to correct pin mode
  pinMode(relayPin, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  prevToggle = digitalRead(buttonPin)?0:1;
  //setup display
  beginDisplay();

  //setup sd card
  beginSD();

  //Setup WiFi
  setUpWiFi();
  if (e.get(0)) {
    connectWiFi();
    loadBackupTasks();
  } else {
    loadWiFiFile();
    connectWiFi();
    loadTasksFile();
  }
  sortTasks();

  if (e.get(2)) {
    digitalWrite(relayPin, LOW);
    return;
  }
  // Open http endponts
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/listTasks", HTTP_GET, handleList);
  server.on("/stats", HTTP_GET, handleStats);
  ws.onEvent(handleLights);
  server.addHandler(&ws);
  server.begin();


  // Time syncing
  configTime(long(gmtOffsetSec), 0, ntpServer);
  syncTime();
  if (e.get(3)) {
    digitalWrite(relayPin, LOW);
    return;
  }

  //default setup
  defaultSetup();
  if (!e.get(0))
    startUpLog();

  //display errors
  display.setFont(u8g2_font_5x8_mr);
  display.drawStr(95, 10, e.bin());
  display.sendBuffer();
}

inline String twoDigit(int n) {
  return (n < 10 ? "0" : "") + String(n);
}

void loop() {

  //Clean up
  ws.cleanupClients();

  if(millis() - prevToggleCheck > 500){
    prevToggleCheck = millis();
    if(digitalRead(buttonPin)!=prevToggle){
      toggle(relayPin, !digitalRead(relayPin));
      prevToggle = digitalRead(buttonPin);
    }
  }
  //check for schedular
  if (millis() - prevScheduler >= schedulerInterval) {
    prevScheduler = millis();

    //sync time
    if (!(getLocalTime(&timeInfo))) {
      syncTime();
      if (e.get(3)) goto WiFiCheck;
    }

    display.setFont(u8g2_font_10x20_mr);
    tmpText = ((timeInfo.tm_sec / 15) % 2) ? (twoDigit(timeInfo.tm_hour) + ":" + twoDigit(timeInfo.tm_min)) : faces;
    display.drawStr(0, 48, tmpText.c_str());

    //time align
    for (int i = 0; i < tasks.size(); i++) {
      schedule& task = tasks[i];
      if (timeInfo.tm_hour == task.hour && timeInfo.tm_min == task.minute && !task.triggeredToday) {

        //trigger event
        task.runTask();
        task.triggeredToday = true;
        taskIndex = i;
        display.setFont(u8g2_font_5x8_mr);
        clearSection(0, 10, 60, 20);
        tmpText = "Now>" + twoDigit(tasks[taskIndex].hour) + ":" + twoDigit(tasks[taskIndex].minute);
        display.drawStr(0, 20, tmpText.c_str());
        tmpText = "Next" + twoDigit(tasks[(taskIndex + 1) % tasks.size()].hour) + ":" + twoDigit(tasks[(taskIndex + 1) % tasks.size()].minute);
        display.drawStr(0, 30, tmpText.c_str());
      } else if (timeInfo.tm_min != task.minute) {
        task.triggeredToday = false;
      }
    }


WiFiCheck:
    //wifi interval
    if (millis() - prevWiFiCheck >= wiFiInterval) {
      prevWiFiCheck = millis();
      if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
      }
    }
    display.setFont(u8g2_font_5x8_mr);
    display.drawStr(95, 10, e.bin());
    display.sendBuffer();
  }
}
