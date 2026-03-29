#include <vector>
#ifndef TASKS_H
#define TASKS_H
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
inline void sortTasks(std::vector<schedule>& tasks) {
  std::sort(tasks.begin(), tasks.end(), [](const schedule& a, const schedule& b) {
    return a.totalMinutes() < b.totalMinutes();
  });
}
std::vector<schedule> backupTasks = {
  { 0, 0, false, HIGH },
  { 5, 0, false, LOW },
  { 6, 30, false, HIGH },
  { 18, 0, false, LOW },
};
#endif