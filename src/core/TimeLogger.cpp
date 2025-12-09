#include "TimeLogger.h"
#include "../common/Constants.h"
#include "../common/Logger.h"
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace iRest {

    TimeLogger::TimeLogger(const ITimeSource& timeSource) : m_timeSource(timeSource) {
        if (!fs::exists(Constants::TIME_FILE_FOLDER)) {
            fs::create_directories(Constants::TIME_FILE_FOLDER);
        }
    }

    std::string TimeLogger::getFilePath() const {
        return (fs::path(Constants::TIME_FILE_FOLDER) / Constants::TIME_FILE_NAME).string();
    }

    std::string TimeLogger::getArchiveFilePath(int index) const {
        std::string name = "Archive" + std::to_string(index) + "_" + Constants::TIME_FILE_NAME;
        return (fs::path(Constants::TIME_FILE_FOLDER) / name).string();
    }

    std::string TimeLogger::formatDateTime(double timestamp) const {
        std::time_t t = static_cast<std::time_t>(timestamp);
        std::tm tm_buf;
        localtime_r(&t, &tm_buf); 
        std::stringstream ss;
        ss << std::put_time(&tm_buf, "%d %b %Y %H:%M:%S");
        return ss.str();
    }

    int TimeLogger::countLines(const std::string& path) {
        std::ifstream f(path);
        if(!f.is_open()) return 0;
        return std::count(std::istreambuf_iterator<char>(f), 
                          std::istreambuf_iterator<char>(), '\n');
    }

    int TimeLogger::getNextArchiveIndex() {
        int index = 1;
        while(fs::exists(getArchiveFilePath(index))) {
            index++;
        }
        return index;
    }

    void TimeLogger::rotateFileIfNeeded() {
        std::string path = getFilePath();
        if (!fs::exists(path)) return;

        if (countLines(path) >= Constants::ARCHIVE_LINE_LIMIT) {
            int archiveIdx = getNextArchiveIndex();
            std::string archivePath = getArchiveFilePath(archiveIdx);
            try {
                fs::rename(path, archivePath);
                Logger::log("Archived timeFile to " + archivePath);
            } catch (const fs::filesystem_error& e) {
                Logger::error(std::string("Failed to rotate log: ") + e.what());
            }
        }
    }

    void TimeLogger::log(const std::string& activity, double elapsedTime) {
        std::lock_guard<std::mutex> lock(m_fileMutex);
        
        rotateFileIfNeeded();

        std::string path = getFilePath();
        std::ofstream outfile(path, std::ios_base::app);
        
        double currentTimestamp = m_timeSource.timestamp();
        std::string datetime = formatDateTime(currentTimestamp);

        // Format: {'datetime': '...', 'timestamp': ..., 'activity': ...}
        // Using fixed precision for floats to match requirement example style
        outfile << "{'datetime': '" << datetime << "', "
                << "'timestamp': " << std::fixed << std::setprecision(7) << currentTimestamp << ", "
                << "'elapsed_time': " << std::setprecision(4) << elapsedTime << ", "
                << "'activity': '" << activity << "'}" << std::endl;
    }
}
