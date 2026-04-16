#ifndef TASKS_H
#define TASKS_H
#include <Arduino.h>
#include <vector>
#include <algorithm>
extern void toggle(bool status);
class schedule {
public:
  uint8_t hour:5;
  uint8_t minute:6;
  bool triggeredToday:1;
  bool action:1;
  schedule(uint8_t h, uint8_t m, bool t, bool a)
    : hour(h), minute(m), triggeredToday(t), action(a) {}
  uint16_t totalMinutes() const {
    return hour * 60 + minute;
  }
  void runTask() {
    toggle(this->action);
  }
};
void sortTasks();
void loadTasksFile();
void loadBackupTasks();
extern std::vector<schedule> backupTasks;
extern std::vector<schedule> tasks;
#endif