#pragma once

#include <string>

namespace EyeRest {

class Constants {
public:
  inline static const std::string KEY_DATETIME = "datetime";
  inline static const std::string KEY_TIMESTAMP = "timestamp";
  inline static const std::string KEY_ELAPSED_TIME = "elapsed_time";
  inline static const std::string KEY_ACTIVITY = "activity";

  inline static const std::string STATE_STRAINED = "strained";
  inline static const std::string STATE_PAUSED = "paused";
  inline static const std::string STATE_SUSPENDED = "suspended";
  inline static const std::string STATE_SCREEN_LOCKED = "screen_locked";
  inline static const std::string STATE_PROGRAM_NOT_RUNNING =
      "program_not_running";

  inline static const std::string LOG_FILE_NAME = "logs/eyerest.log";
  inline static const std::string DIR_TIME_FILES = "timerFile";
  inline static const std::string FILE_PREFIX_TIMER = "timeFile_";
  inline static const std::string FILE_PREFIX_ARCHIVE = "Archive";
};

} // namespace EyeRest
