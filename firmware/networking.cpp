#include <sstream>
#include "networking.h"
#include "secrets.h"
#include "constants.h"
#include "errorFlags.h"
#include "sdCard.h"
#include <vector>
WebServer server(80);
struct tm timeInfo;


//WiFi
const IPAddress localIp(192, 168, 1, 254);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress primaryDns(8, 8, 8, 8);
const IPAddress secondaryDns(8, 8, 4, 4);




//WiFi vector
std::vector<WiFiEntry> WiFiList;
void syncTime() {
  uint32_t tmpTime = millis();

  while (!getLocalTime(&timeInfo) && (millis() - tmpTime <= timeWait)) {
    delay(100);
  }
  if (getLocalTime(&timeInfo)) e.set(3, 0);
  else e.set(3, 1);
}

void setUpWiFi() {
  WiFi.config(localIp, gateway, subnet, primaryDns, secondaryDns);
}

void connectWiFi() {
  uint32_t tmpTime;
  for (const WiFiEntry& entry: WiFiList) {
    if (entry.password == nullptr) WiFi.begin(entry.ssid);
    else WiFi.begin(entry.ssid, entry.password);

    tmpTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - tmpTime < wifiWait) {}

    if (WiFi.status() == WL_CONNECTED) {
      e.set(2, 0);
      return;
    } else {
      WiFi.disconnect();
    }
  }
  WiFi.begin(bkpSsid, bkpPassword);
  tmpTime = millis();
  tmpTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - tmpTime < wifiWait) {delay(100);}

  if (WiFi.status() == WL_CONNECTED) {
    e.set(2, 0);
    return;
  } else {
    e.set(2,1);
  }
}
void loadWiFiFile() {
  File file = SD.open("/wifi.dat", FILE_READ);
  if (!file) {
    e.set(1, true);
  } else {
    e.set(1, 0);
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();

      if (line.length() == 0 || line.startsWith("#")) continue;

      int firstSpaceIndex = line.indexOf(' ');

      if (firstSpaceIndex == -1) {
        WiFiList.emplace_back(line.c_str(), nullptr);
      } else {
        String s = line.substring(0, firstSpaceIndex);
        String p = line.substring(firstSpaceIndex);
        p.trim();
        WiFiList.emplace_back(s.c_str(), p.c_str());
      }
    }
  }
  file.close();
}