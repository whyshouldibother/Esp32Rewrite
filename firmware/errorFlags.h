#ifndef ERROR_FLAGS_H
#define ERROR_FLAGS_H
#include <Arduino.h>
class ErrorFlags {
  private:
    uint8_t error : 5 = 0;
  public:
    ErrorFlags() {
      error = 0;
    }
    void set(uint8_t pos, bool code) {
      error = (error & ~(1 << pos)) | (code << pos);
    }
    bool get(uint8_t pos) {
      return (error >> pos) & 1;
    }
    uint8_t code() {
      return error;
    }
    const char* bin() {
      static char buf[6];
      for (int i = 4; i >= 0; i--)
        buf[4 - i] = '0' + ((error >> i) & 1);
      buf[5] = '\0';
      return buf;
  }
};
extern ErrorFlags e;
#endif