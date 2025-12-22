#include "Filesystem.h"
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

namespace EyeRest {

Filesystem::Filesystem() {}

Filesystem::~Filesystem() {}

bool Filesystem::createDirectory(const std::string &path) {
  try {
    return std::filesystem::create_directories(path);
  } catch (const std::exception &e) {
    spdlog::error("Failed to create directory {}: {}", path, e.what());
    return false;
  }
}

bool Filesystem::directoryExists(const std::string &path) {
  return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

bool Filesystem::fileExists(const std::string &path) {
  return std::filesystem::exists(path);
}

bool Filesystem::writeToFile(const std::string &path,
                             const std::string &content, bool append) {
  std::ofstream file;
  if (append) {
    file.open(path, std::ios::app);
  } else {
    file.open(path);
  }

  if (!file.is_open()) {
    spdlog::error("Failed to open file for writing: {}", path);
    return false;
  }

  file << content;
  file.close();
  return true;
}

std::string Filesystem::readFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    spdlog::error("Failed to open file for reading: {}", path);
    return "";
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  return content;
}

std::vector<std::string> Filesystem::readLines(const std::string &path,
                                               int maxLines) {
  std::vector<std::string> lines;
  if (!fileExists(path)) {
    // Not an error, just means no history yet or file missing
    return lines;
  }
  std::ifstream file(path);
  if (!file.is_open()) {
    spdlog::error("Failed to open file for reading lines: {}", path);
    return lines;
  }

  std::string line;
  // Implementation to read last N lines efficiently would be updating the file
  // pointer, but for simplicity we read all and take last N if maxLines is
  // refined, or the user asked for logic in DefaultTimer to load last 360
  // lines. Here we provide a generic read. Optimization for reading last N
  // lines can be complex with variable line lengths. For now simple
  // implementation.

  // If usage requires reading from end, we might need a specific function.
  // The user said "load the last 360 (queue length) lines".
  // Let's implement reading all lines for now.

  while (std::getline(file, line)) {
    lines.push_back(line);
  }

  if (maxLines > 0 && lines.size() > static_cast<size_t>(maxLines)) {
    std::vector<std::string> lastLines(lines.end() - maxLines, lines.end());
    return lastLines;
  }

  return lines;
}

bool Filesystem::renameFile(const std::string &oldPath,
                            const std::string &newPath) {
  try {
    std::filesystem::rename(oldPath, newPath);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to rename file {} to {}: {}", oldPath, newPath,
                  e.what());
    return false;
  }
}

std::string Filesystem::getOperatingSystemName() const {
#ifdef __linux__
  return "Linux";
#elif _WIN32
  return "Windows";
#elif __APPLE__
  return "MacOS";
#else
  return "Unknown";
#endif
}

bool Filesystem::isScreenLocked() const {
  // Placeholder for actual implementation which might depend on OS specific
  // commands For now detecting screensaver or lock state needs specific OS API
  // calls. We will return false by default and log a warning that it's not
  // implemented. In a real scenario we'd call dbus on Linux or Windows APIs.
  return false;
}

} // namespace EyeRest
