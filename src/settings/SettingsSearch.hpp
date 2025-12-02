#pragma once
#include <string>
#include <vector>

int levenshtein(const std::string& a, const std::string& b);

struct SettingItem {
    std::string key;
    std::string category;
    std::string value;
};
