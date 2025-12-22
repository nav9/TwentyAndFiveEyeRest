#pragma once
#include "DefaultTimer/DefaultTimer.h"
#include "Filesystem.h"
#include "ITimeProvider.h"
#include "Settings.h"
#include "SystemTimeProvider.h"
#include <atomic>
#include <memory>
#include <spdlog/spdlog.h>
#include <thread>

namespace EyeRest {

class Core {
public:
  Core(std::shared_ptr<Settings> settings, std::shared_ptr<Filesystem> fs,
       std::shared_ptr<ITimeProvider> timeProvider, bool debugMode = false);
  ~Core();

  void start();
  void stop();
  bool isRunning() const;

private:
  void runLoop();

  std::shared_ptr<Settings> m_settings;
  std::shared_ptr<Filesystem> m_fs;
  std::shared_ptr<ITimeProvider> m_timeProvider;

  // Timer instances
  std::shared_ptr<class DefaultTimer>
      m_defaultTimer; // Forward declaration or include

  std::atomic<bool> m_running;
  std::thread m_thread;
  std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace EyeRest
