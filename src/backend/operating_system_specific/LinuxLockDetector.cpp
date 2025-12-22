#include "LinuxLockDetector.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

namespace EyeRest {

bool LinuxLockDetector::isScreenLocked() {
  if (!m_preferredMethod.empty()) {
    return runProbe(m_preferredMethod);
  }
  // Try all known methods
  for (const auto &probe : getProbes()) {
    if (runProbe(probe.name)) {
      return true;
    }
  }
  return false;
}

std::vector<LockProbe> LinuxLockDetector::getProbes() const {
  return {{"gnome", "GNOME ScreenSaver via DBus"},
          {"cinnamon", "Cinnamon ScreenSaver via DBus"},
          {"kde", "KDE ScreenSaver via DBus"}};
}

bool LinuxLockDetector::runProbe(const std::string &probeName) {
  if (probeName == "gnome") {
    return checkDBus("org.gnome.ScreenSaver", "/org/gnome/ScreenSaver",
                     "org.gnome.ScreenSaver", "GetActive");
  } else if (probeName == "cinnamon") {
    return checkDBus("org.cinnamon.ScreenSaver", "/org/cinnamon/ScreenSaver",
                     "org.cinnamon.ScreenSaver", "GetActive");
  } else if (probeName == "kde") {
    return checkDBus("org.freedesktop.ScreenSaver",
                     "/org/freedesktop/ScreenSaver",
                     "org.freedesktop.ScreenSaver", "GetActive");
  }
  return false;
}

bool LinuxLockDetector::checkDBus(const std::string &service,
                                  const std::string &path,
                                  const std::string &interface,
                                  const std::string &method) {
  std::string command = "dbus-send --print-reply --dest=" + service + " " +
                        path + " " + interface + "." + method + " 2>/dev/null";

  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                pclose);

  if (!pipe) {
    return false;
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  // If the call was successful, the result should contain "boolean true" or
  // "boolean false"
  if (result.find("boolean true") != std::string::npos) {
    return true;
  }

  return false;
}

} // namespace EyeRest
