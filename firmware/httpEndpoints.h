#ifndef HTTP_ENDPOINTS_H
#define HTTP_ENDPOINTS_H

#include <ESPAsyncWebServer.h>
#include "constants.h"
#include "display.h"
#include "tasks.h"
void handleRoot(AsyncWebServerRequest *request);
void handleToggle(AsyncWebServerRequest *request);
void handleList(AsyncWebServerRequest *request);
void handleStats(AsyncWebServerRequest *request);
void handleStatus(AsyncWebServerRequest *request);
void handleLights(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t* data, size_t len);
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern std::vector<schedule> tasks;

#endif