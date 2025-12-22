#pragma once

#include "Filesystem.h"
#include "Settings.h"
#include <deque>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace EyeRest {

class TimeFileManager {
public:
  TimeFileManager(std::shared_ptr<Filesystem> fs,
                  std::shared_ptr<Settings> settings,
                  const std::string &timerName);
  ~TimeFileManager();

  struct TimeEntry {
    std::string datetime;
    double timestamp;
    double elapsed_time;
    std::string activity;

    nlohmann::json toJson() const;
    static TimeEntry fromJson(const nlohmann::json &j);
  };

  void addEntry(const TimeEntry &entry);
  std::deque<TimeEntry> getQueue() const;

private:
  void checkAndArchive();
  std::string getTimerFilePath() const;
  std::string getArchiveFilePath(int ordinal) const;
  void ensureTimeFilesDirectory();

  std::shared_ptr<Filesystem> m_fs;
  std::shared_ptr<Settings> m_settings;
  std::string m_timerName;
  std::deque<TimeEntry> m_queue;
  mutable std::mutex m_queueMutex;
};

} // namespace EyeRest
