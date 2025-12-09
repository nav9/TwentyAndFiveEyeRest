#pragma once

#include <string>
#include <iostream>
#include <mutex>

namespace iRest {
    class Logger {
    public:
        static void log(const std::string& message) {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::cout << "[iRest] " << message << std::endl;
        }

        static void error(const std::string& message) {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::cerr << "[iRest ERROR] " << message << std::endl;
        }

    private:
        static std::mutex m_mutex;
    };
}
