#include "sdCard.h"
#include "errorFlags.h"
#include "constants.h"
#include "networking.h"
void beginSD() {
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)){

    e.set(0,true);
    e.set(1,true);
    e.set(4,true);
    e.set(5,true);
  } 
  else e.set(0, false);
}

void startUpLog() {
  File logs = SD.open("/startup.log", FILE_APPEND);
  char logText[32];
  if (!logs) {
   e.set(5, true);
  }else{
    e.set(5,false);
  }
  if (getLocalTime(&timeInfo))
    sprintf(logText, "%04d%02d%02d%02d%02d%02d%02d", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec, e.code());
  else sprintf(logText, "%02d", e.code());
  logs.println(logText);
  logs.close();
}