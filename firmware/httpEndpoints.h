#ifndef HTTP_ENDPOINTS_H
#define HTTP_ENDPOINTS_H

#include <WebServer.h>
#include "constants.h"
#include "display.h"
#include "tasks.h"
void handleRoot();
void handleToggle();
void handleList();
void handleStats();
void handleStatus();

extern WebServer server;
extern std::vector<schedule> tasks;

#endif