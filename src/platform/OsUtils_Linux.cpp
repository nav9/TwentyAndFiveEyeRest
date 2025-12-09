#include "OsUtils_Linux.h"
#include <array>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include "../common/Logger.h"

namespace iRest {

    std::string OsUtils_Linux::exec(const char* cmd) const {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            Logger::error("popen() failed!");
            return "";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    bool OsUtils_Linux::isScreenLocked() const {
        // Method 1: Check distinct screensaver processes
        // Method 2: Check DBus (Gnome/KDE/Freedesktop)
        
        // This is a robust fallback chain
        std::string out = exec("gnome-screensaver-command -q 2>/dev/null");
        if (out.find("is active") != std::string::npos) return true;

        out = exec("qdbus org.freedesktop.ScreenSaver /org/freedesktop/ScreenSaver GetActive 2>/dev/null");
        if (out.find("true") != std::string::npos) return true;

        out = exec("qdbus org.gnome.ScreenSaver /org/gnome/ScreenSaver GetActive 2>/dev/null");
        if (out.find("true") != std::string::npos) return true;
        
        // Check for specific lock processes if simple queries fail
        // This is less accurate but a catch-all
        // Not implemented to avoid false positives
        
        return false;
    }

    bool OsUtils_Linux::isSystemIdle() const {
        // Checking for idle time involves XScreenSaverQueryInfo on X11
        // On Wayland it's harder.
        // For simplicity in this iteration, returning false or using a basic check.
        // Implementation for valid idle detection:
        
        /* 
           long idleTime = 0;
           // X11 logic can go here using X11/extensions/scrnsaver.h
           // but requires linking Xss and X11.
        */
        return false; 
    }
}
