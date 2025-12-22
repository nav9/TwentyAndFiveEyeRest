#include "DefaultTimer.h"
#include "../Constants.h"
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
      m_paused(false), m_currentState(Constants::STATE_PROGRAM_NOT_RUNNING) {}

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

void DefaultTimer::runLoop() {
  m_lastWriteTime = std::chrono::duration<double>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();

  while (m_running) {
    processState();

    int sampleInterval = m_settings->get<int>("sample_interval");
    std::this_thread::sleep_for(std::chrono::seconds(sampleInterval));
  }
}

void DefaultTimer::processState() {
  auto now = std::chrono::system_clock::now();
  double currentTimestamp =
      std::chrono::duration<double>(now.time_since_epoch()).count();
  std::time_t t_now = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&t_now), "%d %b %Y %H:%M:%S");
  std::string datetime = ss.str();

  // Determine state
  std::string newState = Constants::STATE_STRAINED;

  if (m_paused) {
    newState = Constants::STATE_PAUSED;
  } else if (m_fs->isScreenLocked()) {
    newState = Constants::STATE_SCREEN_LOCKED;
  }

  // State transition logic
  if (newState != m_currentState) {
    spdlog::debug("State Transition: {} -> {} at {}", m_currentState, newState,
                  datetime);
    std::cout << "State Transition: " << m_currentState << " -> " << newState
              << " at " << datetime << std::endl;
    m_currentState = newState;
    m_lastStateChangeTime = currentTimestamp;
  }

  // Calculate delta since last Loop call
  // We use delta for counter updates.
  // However, we should also track m_lastTickTime to be accurate between
  // samples. For simplicity, let's just use current - previous loop's time.
  static double lastTickTime = currentTimestamp;
  double delta = currentTimestamp - lastTickTime;
  if (delta < 0)
    delta = 0;
  lastTickTime = currentTimestamp;

  if (m_currentState == Constants::STATE_STRAINED) {
    m_strainedTime += delta;
    m_restTime = 0; // Reset rest time when strained
  } else if (m_currentState == Constants::STATE_PAUSED ||
             m_currentState == Constants::STATE_SCREEN_LOCKED) {
    m_restTime += delta;
    // Strained time decreases during rest
    m_strainedTime -= delta;
    if (m_strainedTime < 0)
      m_strainedTime = 0;
  }

  // Debug log and terminal output
  spdlog::debug("State: {}, Strained Time: {:.2f}s, Rest Time: {:.2f}s",
                m_currentState, m_strainedTime, m_restTime);
  std::cout << "\rState: " << m_currentState << " | Strained: " << std::fixed
            << std::setprecision(1) << m_strainedTime
            << "s | Rest: " << m_restTime << "s          " << std::flush;

  // Notification logic
  int workMinutes = m_settings->get<int>("work_minutes");
  if (m_currentState == Constants::STATE_STRAINED &&
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
  if ((m_currentState == Constants::STATE_PAUSED ||
       m_currentState == Constants::STATE_SCREEN_LOCKED) &&
      m_restTime >= restMinutes * 60) {
    static bool restNotified = false;
    if (!restNotified) {
      spdlog::info("Rest period over! You have rested for {:.1f} minutes.",
                   m_restTime / 60.0);
      restNotified = true;
    }
  } else {
    // Logic to reset restNotified could be here if they switch back to strained
  }

  // Periodic write to file
  int writeInterval = m_settings->get<int>("data_write_interval");
  if (currentTimestamp - m_lastWriteTime >= writeInterval) {
    TimeFileManager::TimeEntry entry{datetime, currentTimestamp,
                                     currentTimestamp - m_lastWriteTime,
                                     m_currentState};

    m_timeFileManager->addEntry(entry);
    m_lastWriteTime = currentTimestamp;
  }
}

std::string DefaultTimer::determineState(double currentTime, double lastTime) {
  // Helper if needed
  return Constants::STATE_STRAINED;
}

} // namespace EyeRest
