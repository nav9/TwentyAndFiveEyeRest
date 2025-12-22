#include "TimeFileManager.h"
#include "Constants.h"
#include <algorithm>
#include <iostream>
#include <spdlog/spdlog.h>

namespace EyeRest {

TimeFileManager::TimeFileManager(std::shared_ptr<Filesystem> fs,
                                 std::shared_ptr<Settings> settings,
                                 const std::string &timerName)
    : m_fs(std::move(fs)), m_settings(std::move(settings)),
      m_timerName(timerName) {
  ensureTimeFilesDirectory();

  // Load initial queue
  int queueLength = m_settings->get<int>("queue_length");
  std::vector<std::string> lines =
      m_fs->readLines(getTimerFilePath(), queueLength);

  std::lock_guard<std::mutex> lock(m_queueMutex);
  for (const auto &line : lines) {
    if (line.empty())
      continue;
    try {
      nlohmann::json j = nlohmann::json::parse(line);
      m_queue.push_back(TimeEntry::fromJson(j));
    } catch (const std::exception &e) {
      spdlog::error("Failed to parse line in time file: {}", e.what());
    }
  }
}

TimeFileManager::~TimeFileManager() {}

nlohmann::json TimeFileManager::TimeEntry::toJson() const {
  return {{Constants::KEY_DATETIME, datetime},
          {Constants::KEY_TIMESTAMP, timestamp},
          {Constants::KEY_ELAPSED_TIME, elapsed_time},
          {Constants::KEY_ACTIVITY, activity}};
}

TimeFileManager::TimeEntry
TimeFileManager::TimeEntry::fromJson(const nlohmann::json &j) {
  TimeEntry entry;
  if (j.contains(Constants::KEY_DATETIME))
    entry.datetime = j[Constants::KEY_DATETIME].get<std::string>();
  if (j.contains(Constants::KEY_TIMESTAMP))
    entry.timestamp = j[Constants::KEY_TIMESTAMP].get<double>();
  if (j.contains(Constants::KEY_ELAPSED_TIME))
    entry.elapsed_time = j[Constants::KEY_ELAPSED_TIME].get<double>();
  if (j.contains(Constants::KEY_ACTIVITY))
    entry.activity = j[Constants::KEY_ACTIVITY].get<std::string>();
  return entry;
}

void TimeFileManager::addEntry(const TimeEntry &entry) {
  spdlog::debug("TimeFileManager: Adding entry for {} at {}", m_timerName,
                entry.datetime);
  // Update in-memory queue
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.push_back(entry);
    int queueLength = m_settings->get<int>("queue_length");
    while (m_queue.size() > static_cast<size_t>(queueLength)) {
      m_queue.pop_front();
    }
  }

  // Write to file
  checkAndArchive();
  std::string line = entry.toJson().dump();
  std::string filePath = getTimerFilePath();
  spdlog::debug("TimeFileManager: Writing to file: {}", filePath);
  if (m_fs->writeToFile(filePath, line + "\n", true)) {
    spdlog::debug("TimeFileManager: Successfully wrote to file: {}", filePath);
  } else {
    spdlog::error("TimeFileManager: Failed to write to file: {}", filePath);
  }
}

std::deque<TimeFileManager::TimeEntry> TimeFileManager::getQueue() const {
  std::lock_guard<std::mutex> lock(m_queueMutex);
  return m_queue;
}

void TimeFileManager::checkAndArchive() {
  int maxLines = m_settings->get<int>("timer_file_max_lines");
  // Implement optimization: don't read all lines every time?
  // For now, prompt implementation was reading all. We can rely on fs
  // implementation.
  std::vector<std::string> lines = m_fs->readLines(getTimerFilePath());

  if (lines.size() >= static_cast<size_t>(maxLines)) {
    int ordinal = 1;
    while (m_fs->fileExists(getArchiveFilePath(ordinal))) {
      ordinal++;
    }

    std::string archivePath = getArchiveFilePath(ordinal);
    if (m_fs->renameFile(getTimerFilePath(), archivePath)) {
      spdlog::info("Archived timer file to {}", archivePath);
    }
  }
}

void TimeFileManager::ensureTimeFilesDirectory() {
  std::string baseDir = m_settings->get<std::string>("time_files_dir");
  std::string timeFilesDir = baseDir + "/" + Constants::DIR_TIME_FILES;
  if (!m_fs->directoryExists(timeFilesDir)) {
    if (!m_fs->createDirectory(timeFilesDir)) {
      spdlog::error("Failed to create time files directory: {}", timeFilesDir);
    }
  }
}

std::string TimeFileManager::getTimerFilePath() const {
  std::string baseDir = m_settings->get<std::string>("time_files_dir");
  return baseDir + "/" + Constants::DIR_TIME_FILES + "/" +
         Constants::FILE_PREFIX_TIMER + m_timerName + ".txt";
}

std::string TimeFileManager::getArchiveFilePath(int ordinal) const {
  std::string baseDir = m_settings->get<std::string>("time_files_dir");
  return baseDir + "/" + Constants::DIR_TIME_FILES + "/" +
         Constants::FILE_PREFIX_ARCHIVE + std::to_string(ordinal) + "_" +
         m_timerName + ".txt";
}

} // namespace EyeRest
