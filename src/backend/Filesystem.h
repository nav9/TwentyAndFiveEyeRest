#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace EyeRest {

class Filesystem {
public:
  Filesystem();
  ~Filesystem();

  bool createDirectory(const std::string &path);
  bool directoryExists(const std::string &path);
  bool fileExists(const std::string &path);
  bool writeToFile(const std::string &path, const std::string &content,
                   bool append = false);
  std::string readFile(const std::string &path);
  std::vector<std::string> readLines(const std::string &path,
                                     int maxLines = -1);
  bool renameFile(const std::string &oldPath, const std::string &newPath);

  // OS detection
  std::string getOperatingSystemName() const;
  bool isScreenLocked() const;

private:
  // Helper functions for specific OS implementations if needed
};

} // namespace EyeRest
