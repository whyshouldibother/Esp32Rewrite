#include "httpEndpoints.h"

void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Online");
}
void handleToggle(AsyncWebServerRequest *request) {
  digitalWrite(relayPin, !digitalRead(relayPin));
  request->send(200, "text/plain", digitalRead(relayPin) ? "OFF" : "ON");
  ws.textAll(digitalRead(relayPin)?"OFF":"ON");
  display.setFont(u8g2_font_10x20_mr);
  clearSection(60, 10, 30, 20);
  display.drawStr(60, 30, digitalRead(relayPin) ? "OFF" : "ON");
  display.sendBuffer();
}
void handleStatus(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", digitalRead(relayPin) ? "OFF" : "ON");
}
void handleList(AsyncWebServerRequest *request) {
  String json = "[";
  for (int i = 0; i < tasks.size(); i++) {
    json += "{\"hour\":" + String(tasks[i].hour) + ",\"minute\":" + String(tasks[i].minute) + ", \"Action\":\"" + (tasks[i].action ? "Off" : "On") + "\"},";
  }
  json[json.length() - 1] = ']';
  request->send(200, "application/json", json);
}

void handleLights(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t* data, size_t len){
  if(type == WS_EVT_DATA){
  if(len == 2)
  {
    char code = (char)data[0];
    if(code == '0'){
      digitalWrite(relayPin, 1);
    }else if(code == '1'){
      digitalWrite(relayPin, 0);
    }else if(code == '!'){
      digitalWrite(relayPin, !digitalRead(relayPin));
    }
  }
  }
    ws.textAll(digitalRead(relayPin) ? "OFF" : "ON");
  
}