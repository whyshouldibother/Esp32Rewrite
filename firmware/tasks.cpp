#include "tasks.h"
#include "sdCard.h"
#include "errorFlags.h"
std::vector<schedule> tasks;

std::vector<schedule> backupTasks = {
  { 0, 0, false, HIGH },
  { 5, 0, false, LOW },
  { 6, 30, false, HIGH },
  { 18, 0, false, LOW },
};

void sortTasks() {
  std::sort(tasks.begin(), tasks.end(), [](const schedule& a, const schedule& b) {
    return a.totalMinutes() < b.totalMinutes();
  });
}

void loadTasksFile(){
  if (SD.exists("/tasks.dat")) {
    e.set(4, false);
    File taskFile = SD.open("/tasks.dat", FILE_READ);
    String line;
    while (taskFile.available()) {
      line = taskFile.readStringUntil('\n');
      if (line.length() < 5) continue;
      tasks.push_back(schedule(line.substring(0, 2).toInt(), line.substring(2, 4).toInt(), false, line[4] - '0'));
    }
  } else {
    loadBackupTasks();
  }
}
void loadBackupTasks(){
  e.set(4, true);
  tasks=backupTasks;
}