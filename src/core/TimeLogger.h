#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include "../common/ITimeSource.h"

namespace iRest {
    struct LogEntry {
        double timestamp;
        double elapsedTime;
        std::string activity;
        std::string datetime;
    };

    class TimeLogger {
    public:
        TimeLogger(const ITimeSource& timeSource);
        void log(const std::string& activity, double elapsedTime);
        
    private:
        void rotateFileIfNeeded();
        std::string formatDateTime(double timestamp) const;
        std::string getFilePath() const;
        std::string getArchiveFilePath(int index) const;
        int countLines(const std::string& path);
        int getNextArchiveIndex();

        const ITimeSource& m_timeSource;
        std::mutex m_fileMutex;
    };
}
