#include <WebServer.h>
#include <WiFi.h>
#include "time.h"
#include "secrets.h"
#include "tasks.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define sda 21
#define sdc 22

#define relayPin 15

#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18

//Scheduler routine
// 30000 = 1000 * 30=s 30 sec
#define schedulerInterval 10000


WebServer server(80);
const IPAddress localIp(192, 168, 1, 254);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress primaryDns(8, 8, 8, 8);
const IPAddress secondaryDns(8, 8, 4, 4);

const char* ntpServer = "time.google.com";
struct tm timeInfo;

unsigned long tmpTime = 0, prevScheduler = 0;

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);

uint8_t taskIndex = 0;

String tmpText = "";
String faces = "(0_0)";

std::vector<schedule> tasks;


inline void clearSection(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  display.setDrawColor(0);
  display.drawBox(x, y, w, h);
  display.setDrawColor(1);
}
inline void handleRoot() {
  server.send(200);
}
inline void handleToggle() {
  digitalWrite(relayPin, !digitalRead(relayPin));
  server.send(200, "text/plain", digitalRead(relayPin) ? "OFF" : "ON");
  display.setFont(u8g2_font_10x20_mr);
  clearSection(60, 10, 30, 20);
  display.drawStr(60, 30, digitalRead(relayPin) ? "OFF" : "ON");
  display.sendBuffer();
}
inline void handleStatus() {
  server.send(200, "text/plain", digitalRead(relayPin) ? "OFF" : "ON");
}
inline void listTasks() {
  String json = "[";
  for (int i = 0; i < tasks.size(); i++) {
    json += "{\"hour\":" + String(tasks[i].hour) + ",\"minute\":" + String(tasks[i].minute) + ", \"Action\":\"" + (tasks[i].action ? "Off" : "On") + "\"},";
  }
  json[json.length() - 1] = ']';
  server.send(200, "application/json", json);
}
inline void syncTime() {
  tmpTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - tmpTime <= 3000) {}
  if (WiFi.status() != WL_CONNECTED) return;
  tmpTime = millis();
  configTime(long(gmtOffsetSec), 0, ntpServer);
  while (!getLocalTime(&timeInfo) && millis() - tmpTime <= 3000) {}
}
inline void handleStats() {
  tmpTime = millis();
  //24 * 60 * 60 * 1000 = 86400000;
  uint8_t day = tmpTime / 864000000;
  uint8_t hour = (tmpTime % 86400000) / 3600000;
  uint8_t minute = (tmpTime % 3600000) / 60000;
  server.send(200, "application/json",
              "{\"Uptime\":{\"day\":" + String(day) + ", \"hour\":" + String(hour) + ", \"minutes\":" + String(minute) + "},\"Realtime\":" + (getLocalTime(&timeInfo) ? ("{\"day\":" + String(timeInfo.tm_mday) + ", \"hour\":" + String(timeInfo.tm_hour) + ", \"minutes\":" + String(timeInfo.tm_min) + "}") : "\"Time Error\"") + "}");
}
void toggle(bool status) {
  digitalWrite(relayPin, status);
}
inline void defaultSetup() {
  sortTasks(tasks);
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
  Wire.begin(sda, sdc);
  Wire.setClock(100000);

  display.begin();
  display.clearBuffer();
  display.setDrawColor(1);
  display.setFont(u8g2_font_5x8_mr);

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    display.drawStr(90, 10, "SD:Error");
  } else {
    display.drawStr(90, 10, "SD:OK");
  }

  pinMode(2, OUTPUT);
  pinMode(relayPin, OUTPUT);
  WiFi.config(localIp, gateway, subnet, primaryDns, secondaryDns);
  WiFi.begin(ssid, password);
  digitalWrite(relayPin, LOW);
  digitalWrite(2, HIGH);
  display.drawStr(0, 10, "Connecting to wifi");
  display.sendBuffer();
  while (WiFi.status() != WL_CONNECTED) {}
  strcpy(ipAddr, WiFi.localIP().toString().c_str());
  clearSection(0, 0, 90, 10);
  display.drawStr(0, 10, ipAddr);
  display.sendBuffer();
  server.begin();
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/status", handleStatus);
  server.on("/listTasks", listTasks);
  server.on("/stats", handleStats);
  syncTime();
  display.drawStr(0, 20, "Time Synced");
  display.sendBuffer();
  digitalWrite(2, LOW);
  digitalWrite(relayPin, HIGH);
  {
    File logs = SD.open("/startup.log", FILE_APPEND);
    char logText[16];
    if (getLocalTime(&timeInfo))
      sprintf(logText, "%04d%02d%02d%02d%02d%02dOK", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    else strcpy(logText, "Time Error");
    logs.println(logText);
    logs.close();
    if(SD.exists("/tasks.dat")){

    File taskFile = SD.open("/tasks.dat", FILE_READ);
    String line;
    while (taskFile.available()) {
      line = taskFile.readStringUntil('\n');
      if (line.length() < 5) continue;
      tasks.push_back(schedule(line.substring(0, 2).toInt(), line.substring(2, 4).toInt(), false, line[4] - '0'));
    }
    }else{
      tasks=backupTasks;
      display.drawStr(90, 20, "BackUp");
      display.drawStr(90, 30, "Loaded");
      display.sendBuffer();
      
    }
  }
  defaultSetup();
}

inline String twoDigit(int n) {
  return (n < 10 ? "0" : "") + String(n);
}
void loop() {
  server.handleClient();
  if (millis() - prevScheduler >= schedulerInterval) {
    prevScheduler = millis();
    if (!(getLocalTime(&timeInfo))) syncTime();
    else {
      display.setFont(u8g2_font_10x20_mr);
      tmpText = ((timeInfo.tm_sec / 15) % 2) ? (twoDigit(timeInfo.tm_hour) + ":" + twoDigit(timeInfo.tm_min)) : faces;
      display.drawStr(0, 48, tmpText.c_str());
      if (timeInfo.tm_min == 0 && timeInfo.tm_hour % 3 == 0 && timeInfo.tm_sec > 45) syncTime();
      for (int i = 0; i < tasks.size(); i++) {
        schedule& task = tasks[i];
        if (timeInfo.tm_hour == task.hour && timeInfo.tm_min == task.minute && !task.triggeredToday) {
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
    }
    display.sendBuffer();
  }
}
