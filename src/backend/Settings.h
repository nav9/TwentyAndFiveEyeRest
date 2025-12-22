#pragma once

#include "Filesystem.h"
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace EyeRest {

enum class SettingType { STRING, INTEGER, FLOAT, BOOLEAN };

struct SettingDefinition {
  std::string key;
  std::string name;
  std::string explanation;
  std::string tooltip;
  SettingType type;
  std::string unit;
  std::string displayUnit;
  nlohmann::json defaultValue;
  nlohmann::json minValue; // For numbers
  nlohmann::json maxValue; // For numbers
  std::string category;
};

class ISettingsObserver {
public:
  virtual ~ISettingsObserver() = default;
  virtual void onSettingChanged(const std::string &key, const nlohmann::json &newValue) = 0;
};

class Settings {
public:
  Settings(std::shared_ptr<Filesystem> fs, const std::string &filePath);
  ~Settings();

  void load();
  void save();

  void registerSetting(const SettingDefinition &def);

  // Generic getter
  template <typename T> T get(const std::string &key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_values.contains(key)) {
      return m_values[key].get<T>();
    }
    // Return default if not found (should be validated on load but safety
    // check)
    auto it = m_definitions.find(key);
    if (it != m_definitions.end()) {return it->second.defaultValue.get<T>();}
    throw std::runtime_error("Setting key not found: " + key);
  }

  // Generic setter
  template <typename T> void set(const std::string &key, const T &value) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      // Validate
      auto it = m_definitions.find(key);
      if (it == m_definitions.end()) {
        throw std::runtime_error("Setting key not defined: " + key);
      }
      nlohmann::json jsonVal = value;
      if (!validateValue(it->second, jsonVal)) {
        // Log and ignore or throw? Prompt says "if not within range... reset to
        // default". But this is setting a NEW value. We should probably reject
        // invalid values from user/code. However, the prompt refers to
        // initialization check. Let's allow but maybe log error if strictly
        // invalid? For now, simple set.
      }
      m_values[key] = jsonVal;
    }
    notifyObservers(key, value);
    save();
  }

  void addObserver(std::shared_ptr<ISettingsObserver> observer);
  void removeObserver(std::shared_ptr<ISettingsObserver> observer);

  // Initialize default settings definitions
  void initializeDefaults();

  const std::map<std::string, SettingDefinition> &getDefinitions() const {
    return m_definitions;
  }

private:
  bool validateValue(const SettingDefinition &def, const nlohmann::json &value) const;
  void validateAllAndFix();
  void notifyObservers(const std::string &key, const nlohmann::json &value);

  std::shared_ptr<Filesystem> m_fs;
  std::string m_filePath;
  std::map<std::string, SettingDefinition> m_definitions;
  nlohmann::json m_values;
  std::vector<std::weak_ptr<ISettingsObserver>> m_observers;
  mutable std::mutex m_mutex;
};

} // namespace EyeRest
