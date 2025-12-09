#pragma once

#include <string>

namespace iRest {
    namespace Constants {
        constexpr int DEFAULT_STRAIN_DURATION_MINUTES = 20;
        constexpr int MINIMUM_REST_TIME_MINUTES = 5;
        constexpr int LOG_INTERVAL_SECONDS = 30;
        constexpr int ARCHIVE_LINE_LIMIT = 10000;
        const std::string TIME_FILE_FOLDER = "timeFiles";
        const std::string TIME_FILE_NAME = "timeFile.txt";

        namespace Activities {
            const std::string STRAINED = "strained";
            const std::string SCREEN_LOCKED = "screen_locked";
            const std::string PAUSED = "paused";
            const std::string PROGRAM_NOT_RUNNING = "program_not_running";
            const std::string SUSPENDED = "suspended";
            const std::string RESET = "reset";
        }
    }
}
