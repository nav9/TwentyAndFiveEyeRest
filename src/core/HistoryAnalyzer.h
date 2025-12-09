#pragma once

#include "TimeLogger.h" // For LogEntry and TimeLogger access if needed
#include <deque>
#include <string>
#include <vector>

namespace iRest {
    class HistoryAnalyzer {
    public:
        // Returns last N entries from current and archive files
        static std::deque<LogEntry> getLastEntries(int count);
        
        // Parses a single line
        static LogEntry parseLine(const std::string& line);
    };
}
