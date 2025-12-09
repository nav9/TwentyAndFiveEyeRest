#include "HistoryAnalyzer.h"
#include "../common/Constants.h"
#include "../common/Logger.h"
#include <filesystem>
#include <fstream>
#include <regex>
#include <iostream>

namespace fs = std::filesystem;

namespace iRest {

    LogEntry HistoryAnalyzer::parseLine(const std::string& line) {
        LogEntry entry;
        try {
            // Regex to parse the dict-like string
            // {'datetime': '...', 'timestamp': 123.456, 'elapsed_time': 1.2, 'activity': '...'}
            // Simplified regex for robustness
            std::regex ts_regex("'timestamp':\\s*([0-9.]+)");
            std::regex et_regex("'elapsed_time':\\s*([0-9.]+)");
            std::regex act_regex("'activity':\\s*'([^']+)'");
            std::smatch match;

            if (std::regex_search(line, match, ts_regex) && match.size() > 1)
                entry.timestamp = std::stod(match.str(1));
            
            if (std::regex_search(line, match, et_regex) && match.size() > 1)
                entry.elapsedTime = std::stod(match.str(1));

            if (std::regex_search(line, match, act_regex) && match.size() > 1)
                entry.activity = match.str(1);

        } catch (...) {
            Logger::error("Failed to parse line: " + line);
        }
        return entry;
    }

    std::deque<LogEntry> HistoryAnalyzer::getLastEntries(int count) {
        std::deque<LogEntry> entries;
        std::vector<std::string> filesToRead;
        
        // Add current file
        filesToRead.push_back((fs::path(Constants::TIME_FILE_FOLDER) / Constants::TIME_FILE_NAME).string());
        
        // Add archives in reverse order (ArchiveN, ArchiveN-1...) if needed
        // For simplicity, just reading current file and maybe the last archive if current is small
        // The user requirement says "check any of the latest Archive timeFiles"
        // Let's sweep Archive1 to ArchiveN, allow sorting by modification time perhaps?
        // Or assume sequential numbering.
        
        // Simplified approach: scan dir for Archives, sort by number desc
        std::vector<std::pair<int, std::string>> archives;
        for (const auto& entry : fs::directory_iterator(Constants::TIME_FILE_FOLDER)) {
            std::string name = entry.path().filename().string();
            if (name.find("Archive") == 0) {
                // Extract number
                int num = 0;
                try {
                    size_t underscore = name.find('_');
                    std::string numStr = name.substr(7, underscore - 7);
                    num = std::stoi(numStr);
                    archives.push_back({num, entry.path().string()});
                } catch(...) {}
            }
        }
        std::sort(archives.rbegin(), archives.rend()); // Highest number first
        
        for (auto& pair : archives) {
            filesToRead.push_back(pair.second);
        }
        
        // Now read files until we have enough entries
        // Note: filesToRead has Current, then ArchiveMax, ArchiveMax-1... which is reverse chronological blocks? 
        // No, Current is latest. ArchiveMax is previous latest. So yes, order is correct.
        
        for (const auto& path : filesToRead) {
            if (entries.size() >= (size_t)count) break;
            
            std::ifstream file(path);
            if (!file.is_open()) continue;
            
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(file, line)) {
                if (!line.empty()) lines.push_back(line);
            }
            
            // Read lines from end
            for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
                if (entries.size() >= (size_t)count) break;
                entries.push_back(parseLine(*it));
            }
        }
        
        return entries;
    }
}
