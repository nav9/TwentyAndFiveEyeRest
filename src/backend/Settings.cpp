#include "Settings.h"
#include <iostream>
#include <spdlog/spdlog.h>

namespace EyeRest {

Settings::Settings(std::shared_ptr<Filesystem> fs, const std::string &filePath)
    : m_fs(std::move(fs)), m_filePath(filePath) {}

Settings::~Settings() { save(); }

void Settings::registerSetting(const SettingDefinition &def) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_definitions[def.key] = def;
  // Ensure value exists, if not use default
  if (!m_values.contains(def.key)) {
    m_values[def.key] = def.defaultValue;
  }
}

void Settings::initializeDefaults() {
  // Register Application Settings
  registerSetting({"theme", "Theme", "Visual theme of the application",
                   "Choose between Light and Dark", SettingType::STRING, "", "",
                   "Dark", nullptr, nullptr, "Application"});
  registerSetting({"time_files_dir", "Time Files Directory",
                   "Directory where time files are stored",
                   "Absolute or relative path", SettingType::STRING, "", "",
                   ".", nullptr, nullptr, "Application"});

  // Register DefaultTimer Settings
  registerSetting({"work_minutes", "Work Minutes",
                   "Duration of work before a break", "Time in minutes",
                   SettingType::INTEGER, "minutes", "minutes", 20, 1, 120,
                   "DefaultTimer"});
  registerSetting({"rest_minutes", "Rest Minutes", "Duration of rest break",
                   "Time in minutes", SettingType::INTEGER, "minutes",
                   "minutes", 5, 1, 60, "DefaultTimer"});
  registerSetting({"notification_interval", "Notification Interval",
                   "Interval between repeated notifications", "Time in minutes",
                   SettingType::INTEGER, "minutes", "minutes", 2, 1, 60,
                   "DefaultTimer"});
  registerSetting({"queue_length", "Queue Length",
                   "Length of the timestamp queue", "Number of entries",
                   SettingType::INTEGER, "", "", 360, 60, 3600,
                   "DefaultTimer"});
  registerSetting({"data_write_interval", "Data Write Interval",
                   "Interval to write data to file", "Time in seconds",
                   SettingType::INTEGER, "seconds", "seconds", 60, 1, 600,
                   "DefaultTimer"});
  registerSetting({"timer_file_max_lines", "Timer File Max Lines",
                   "Max lines in timer file before archiving",
                   "Number of lines", SettingType::INTEGER, "", "", 100000,
                   1000, 1000000, "DefaultTimer"});
  registerSetting({"sample_interval", "Sample Interval",
                   "Interval to sample queue", "Time in seconds",
                   SettingType::INTEGER, "seconds", "seconds", 10, 1, 60,
                   "DefaultTimer"});

  // Logging Settings
  registerSetting({"log_file_size", "Log File Size",
                   "Max size of rotating log file", "Size in MB",
                   SettingType::INTEGER, "MB", "MB", 5, 1, 100, "Logging"});
  registerSetting({"log_file_count", "Log File Count",
                   "Number of rotating log files to keep", "Count",
                   SettingType::INTEGER, "", "", 3, 1, 20, "Logging"});
}

void Settings::load() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_fs->fileExists(m_filePath)) {
    try {
      std::string content = m_fs->readFile(m_filePath);
      m_values = nlohmann::json::parse(content);
    } catch (const std::exception &e) {
      spdlog::error("Failed to parse settings file: {}. Using defaults.",
                    e.what());
      // Reset to defaults? Or partial? For now, we trust values that are
      // loaded, others invoke default
    }
  } else {
    spdlog::info("Settings file not found, using defaults.");
  }
  validateAllAndFix();
}

void Settings::save() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_fs) {
    m_fs->writeToFile(m_filePath, m_values.dump(4));
  }
}

bool Settings::validateValue(const SettingDefinition &def,
                             const nlohmann::json &value) const {
  if (def.type == SettingType::INTEGER) {
    if (!value.is_number_integer()) {
      return false;
    }
    long long v = value.get<long long>();
    if (!def.minValue.is_null() && v < def.minValue.get<long long>()) {
      return false;
    }
    if (!def.maxValue.is_null() && v > def.maxValue.get<long long>()) {
      return false;
    }
  } else if (def.type == SettingType::FLOAT) {
    if (!value.is_number()) {
      return false;
    }
    double v = value.get<double>();
    if (!def.minValue.is_null() && v < def.minValue.get<double>()) {
      return false;
    }
    if (!def.maxValue.is_null() && v > def.maxValue.get<double>()) {
      return false;
    }
  }
  return true;
}

void Settings::validateAllAndFix() {
  for (const auto &[key, def] : m_definitions) {
    bool needsFix = false;
    if (!m_values.contains(key)) {
      m_values[key] = def.defaultValue;
      needsFix = true;
    } else {
      if (!validateValue(def, m_values[key])) {
        spdlog::warn("Setting '{}' value {} is out of range or invalid type. "
                     "Resetting to default {}.",
                     key, m_values[key].dump(), def.defaultValue.dump());
        m_values[key] = def.defaultValue;
        needsFix = true;
      }
    }
    if (needsFix) {
      // We can't immediately save here if we are inside load() called by
      // Constructor potentially? But valid to mark dirty.
    }
  }
}

void Settings::addObserver(std::shared_ptr<ISettingsObserver> observer) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_observers.push_back(observer);
}

void Settings::removeObserver(std::shared_ptr<ISettingsObserver> observer) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_observers.erase(
      std::remove_if(m_observers.begin(), m_observers.end(),
                     [&](const std::weak_ptr<ISettingsObserver> &wptr) {
                       return wptr.expired() || wptr.lock() == observer;
                     }),
      m_observers.end());
}

void Settings::notifyObservers(const std::string &key,
                               const nlohmann::json &value) {
  // Copy observers to avoid deadlock if observer modifies settings (re-entrant
  // lock) Actually we released lock before calling this in set(), but we need
  // to check usage

  // In set(), we do: { lock; update; } notify; save;
  // So valid.

  std::vector<std::shared_ptr<ISettingsObserver>> activeObservers;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &wptr : m_observers) {
      if (auto sptr = wptr.lock()) {
        activeObservers.push_back(sptr);
      }
    }
  }

  for (const auto &observer : activeObservers) {
    observer->onSettingChanged(key, value);
  }
}

} // namespace EyeRest
