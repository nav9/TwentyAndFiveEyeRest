#include "DefaultTimer.h"
#include "../Constants.h"
#include "ITimerState.h"
#include "TimerStates.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>

namespace EyeRest {

DefaultTimer::DefaultTimer(std::shared_ptr<Settings> settings,
                           std::shared_ptr<Filesystem> fs,
                           std::shared_ptr<ITimeProvider> timeProvider,
                           std::shared_ptr<TimeFileManager> timeFileManager)
    : m_settings(std::move(settings)), m_fs(std::move(fs)),
      m_timeProvider(std::move(timeProvider)),
      m_timeFileManager(std::move(timeFileManager)), m_running(false),
      m_paused(false), m_screenLocked(false),
      m_currentStateName(Constants::STATE_PROGRAM_NOT_RUNNING) {

  // Initialize states
  m_states[Constants::STATE_STRAINED] = std::make_unique<StrainedState>();
  m_states[Constants::STATE_PAUSED] = std::make_unique<PausedState>();
  m_states[Constants::STATE_SCREEN_LOCKED] =
      std::make_unique<ScreenLockedState>();

  // Initial state
  transitionTo(Constants::STATE_STRAINED);
}

DefaultTimer::~DefaultTimer() { stop(); }

void DefaultTimer::start() {
  if (m_running) {
    return;
  }
  m_running = true;
  m_thread = std::thread(&DefaultTimer::runLoop, this);
}

void DefaultTimer::stop() {
  m_running = false;
  if (m_thread.joinable()) {
    m_thread.join();
  }
}

void DefaultTimer::pause() { m_paused = true; }

void DefaultTimer::resume() { m_paused = false; }

void DefaultTimer::setScreenLocked(bool locked) { m_screenLocked = locked; }

void DefaultTimer::runLoop() {
  m_lastWriteTime = std::chrono::duration<double, std::ratio<1>>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();

  static double lastTickTime = m_lastWriteTime;

  while (m_running) {
    auto now = std::chrono::system_clock::now();
    double currentTimestamp =
        std::chrono::duration<double, std::ratio<1>>(now.time_since_epoch())
            .count();
    double delta = currentTimestamp - lastTickTime;
    if (delta < 0)
      delta = 0;
    lastTickTime = currentTimestamp;

    processState();

    if (m_state) {
      m_state->update(*this, delta);
    }

    int sampleInterval = m_settings->get<int>("sample_interval");
    std::this_thread::sleep_for(std::chrono::seconds(sampleInterval));
  }
}

void DefaultTimer::processState() {
  auto now = std::chrono::system_clock::now();
  double currentTimestamp =
      std::chrono::duration<double, std::ratio<1>>(now.time_since_epoch())
          .count();
  std::time_t t_now = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&t_now), "%d %b %Y %H:%M:%S");
  std::string datetime = ss.str();

  // State delegation
  if (m_state) {
    std::string nextStateName =
        m_state->handleInput(*this, m_paused, m_screenLocked);
    if (nextStateName != m_currentStateName) {
      spdlog::debug("State Transition: {} -> {} at {}", m_currentStateName,
                    nextStateName, datetime);
      std::cout << "\nState Transition: " << m_currentStateName << " -> "
                << nextStateName << " at " << datetime << std::endl;
      transitionTo(nextStateName);
      m_lastStateChangeTime = currentTimestamp;
    }
  }

  // Debug log and terminal output
  spdlog::debug("State: {}, Strained Time: {:.2f}s, Rest Time: {:.2f}s",
                m_currentStateName, m_strainedTime, m_restTime);
  std::cout << "\rState: " << m_currentStateName
            << " | Strained: " << std::fixed << std::setprecision(1)
            << m_strainedTime << "s | Rest: " << m_restTime << "s          "
            << std::flush;

  // Notification logic
  int workMinutes = m_settings->get<int>("work_minutes");
  if (m_currentStateName == Constants::STATE_STRAINED &&
      m_strainedTime >= workMinutes * 60) {
    // Frequency check (every 2 minutes)
    static double lastNotificationTime = 0;
    int notifyInterval = m_settings->get<int>("notification_interval");
    if (currentTimestamp - lastNotificationTime >= notifyInterval * 60) {
      spdlog::info("Take a break! You have been strained for {:.1f} minutes.",
                   m_strainedTime / 60.0);
      lastNotificationTime = currentTimestamp;
    }
  }

  int restMinutes = m_settings->get<int>("rest_minutes");
  if ((m_currentStateName == Constants::STATE_PAUSED ||
       m_currentStateName == Constants::STATE_SCREEN_LOCKED) &&
      m_restTime >= restMinutes * 60) {
    static bool restNotified = false;
    if (!restNotified) {
      spdlog::info("Rest period over! You have rested for {:.1f} minutes.",
                   m_restTime / 60.0);
      restNotified = true;
    }
  }

  // Periodic write to file
  int writeInterval = m_settings->get<int>("data_write_interval");
  if (currentTimestamp - m_lastWriteTime >= writeInterval) {
    TimeFileManager::TimeEntry entry{datetime, currentTimestamp,
                                     currentTimestamp - m_lastWriteTime,
                                     m_currentStateName};

    m_timeFileManager->addEntry(entry);
    m_lastWriteTime = currentTimestamp;
  }
}

void DefaultTimer::transitionTo(const std::string &stateName) {
  auto it = m_states.find(stateName);
  if (it != m_states.end()) {
    m_currentStateName = stateName;
    m_state = it->second.get();
    m_state->enter(*this);
  }
}

} // namespace EyeRest
