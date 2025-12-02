#pragma once
#include "SettingsSearch.hpp"
#include <vector>
#include <string>
#include <mutex>
#include <functional>

class SettingsManager {
public:
    SettingsManager(const std::string& settingsFile);
    ~SettingsManager();

    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key, const std::string& defaultVal = "") const;
    std::vector<SettingItem> search(const std::string& query, int maxResults = 10) const;

    void setChangeCallback(std::function<void(const std::string&, const std::string&)> cb);

private:
    void load();
    void save();

    std::string file_;
    std::vector<SettingItem> items_;
    mutable std::mutex mtx_;
    std::function<void(const std::string&, const std::string&)> changeCb_;
};
