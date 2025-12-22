#pragma once

#include "Filesystem.h"
#include "ITimeProvider.h"
#include "Settings.h"
#include "TimeFileManager.h"
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace EyeRest {

class DefaultTimer {
public:
  DefaultTimer(std::shared_ptr<Settings> settings,
               std::shared_ptr<Filesystem> fs,
               std::shared_ptr<ITimeProvider> timeProvider,
               std::shared_ptr<TimeFileManager> timeFileManager);
  ~DefaultTimer();

  void start();
  void stop();

  // Called by GUI
  void pause();
  void resume();

private:
  void runLoop();
  void processState();
  std::string determineState(double currentTime, double lastTime);

  std::shared_ptr<Settings> m_settings;
  std::shared_ptr<Filesystem> m_fs;
  std::shared_ptr<ITimeProvider> m_timeProvider;
  std::shared_ptr<TimeFileManager> m_timeFileManager;

  std::atomic<bool> m_running;
  std::thread m_thread;
  std::atomic<bool> m_paused;

  double m_strainedTime = 0;
  double m_restTime = 0;
  std::string m_currentState;
  double m_lastStateChangeTime = 0;
  double m_lastWriteTime = 0;
};

} // namespace EyeRest
