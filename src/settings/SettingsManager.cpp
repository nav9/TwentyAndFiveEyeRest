#include "SettingsManager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

SettingsManager::SettingsManager(const std::string& settingsFile) : file_(settingsFile) {
    load();
}

SettingsManager::~SettingsManager() {
    save();
}

void SettingsManager::load() {
    std::lock_guard<std::mutex> lk(mtx_);
    items_.clear();
    if(!fs::exists(file_)) return;
    std::ifstream ifs(file_);
    std::string line;
    while(std::getline(ifs, line)) {
        if(line.empty()) continue;
        // simple key=value format
        auto pos = line.find('=');
        if(pos == std::string::npos) continue;
        SettingItem it;
        it.key = line.substr(0,pos);
        it.value = line.substr(pos+1);
        items_.push_back(it);
    }
}

void SettingsManager::save() {
    std::lock_guard<std::mutex> lk(mtx_);
    std::ofstream ofs(file_, std::ios::trunc);
    for(const auto& it : items_) {
        ofs << it.key << "=" << it.value << "\n";
    }
}

void SettingsManager::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = std::find_if(items_.begin(), items_.end(), [&](const SettingItem& s){ return s.key == key; });
    if(it != items_.end()) it->value = value;
    else items_.push_back({key, "", value});
    save();
    if(changeCb_) changeCb_(key, value);
}

std::string SettingsManager::get(const std::string& key, const std::string& defaultVal) const {
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = std::find_if(items_.begin(), items_.end(), [&](const SettingItem& s){ return s.key == key; });
    if(it != items_.end()) return it->value;
    return defaultVal;
}

std::vector<SettingItem> SettingsManager::search(const std::string& query, int maxResults) const {
    std::lock_guard<std::mutex> lk(mtx_);
    std::vector<std::pair<int, SettingItem>> scored;
    for(const auto& s : items_) {
        int scoreKey = levenshtein(query, s.key);
        int scoreCat = levenshtein(query, s.category);
        int best = std::min(scoreKey, scoreCat);
        scored.push_back({best, s});
    }
    std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b){ return a.first < b.first; });
    std::vector<SettingItem> res;
    for(size_t i=0;i<scored.size() && (int)res.size() < maxResults;i++) res.push_back(scored[i].second);
    return res;
}

void SettingsManager::setChangeCallback(std::function<void(const std::string&, const std::string&)> cb) {
    changeCb_ = cb;
}
