#ifndef NETWORKING_H
#define NETWORKING_H
#include <WebServer.h>
#include <WiFi.h>
#include "errorFlags.h"

extern WebServer server;
extern struct tm timeInfo;
extern ErrorFlags e;

void syncTime();
void setUpWiFi();
void connectWiFi();
void loadWiFiFile();

class WiFiEntry {
public:
  char* ssid;
  char* password;

  WiFiEntry(const char* s,const char* p) {

    ssid = strdup(s);
    password = p? strdup(p): nullptr;
  }
};
#endif