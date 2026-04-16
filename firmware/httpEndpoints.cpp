#include "httpEndpoints.h"

void handleRoot() {
  server.send(200);
}
void handleToggle() {
  digitalWrite(relayPin, !digitalRead(relayPin));
  server.send(200, "text/plain", digitalRead(relayPin) ? "OFF" : "ON");
  display.setFont(u8g2_font_10x20_mr);
  clearSection(60, 10, 30, 20);
  display.drawStr(60, 30, digitalRead(relayPin) ? "OFF" : "ON");
  display.sendBuffer();
}
void handleStatus() {
  server.send(200, "text/plain", digitalRead(relayPin) ? "OFF" : "ON");
}
void handleList() {
  String json = "[";
  for (int i = 0; i < tasks.size(); i++) {
    json += "{\"hour\":" + String(tasks[i].hour) + ",\"minute\":" + String(tasks[i].minute) + ", \"Action\":\"" + (tasks[i].action ? "Off" : "On") + "\"},";
  }
  json[json.length() - 1] = ']';
  server.send(200, "application/json", json);
}