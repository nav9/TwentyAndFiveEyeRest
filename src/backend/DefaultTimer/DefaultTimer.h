#pragma once

#include "Filesystem.h"
#include "ITimeProvider.h"
#include "Settings.h"
#include "TimeFileManager.h"
#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

namespace EyeRest {

class ITimerState;

class DefaultTimer {
  friend class StrainedState;
  friend class PausedState;
  friend class ScreenLockedState;

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

  // Called by Core
  void setScreenLocked(bool locked);

private:
  void runLoop();
  void processState();
  void transitionTo(const std::string &stateName);

  std::shared_ptr<Settings> m_settings;
  std::shared_ptr<Filesystem> m_fs;
  std::shared_ptr<ITimeProvider> m_timeProvider;
  std::shared_ptr<TimeFileManager> m_timeFileManager;

  std::atomic<bool> m_running;
  std::thread m_thread;
  std::atomic<bool> m_paused;
  std::atomic<bool> m_screenLocked;

  double m_strainedTime = 0;
  double m_restTime = 0;

  ITimerState *m_state = nullptr;
  std::string m_currentStateName;

  double m_lastStateChangeTime = 0;
  double m_lastWriteTime = 0;

  std::map<std::string, std::unique_ptr<ITimerState>> m_states;
};

} // namespace EyeRest
