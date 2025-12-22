#include "TimerStates.h"
#include "../Constants.h"
#include "DefaultTimer.h"
#include <iostream>
#include <spdlog/spdlog.h>

namespace EyeRest {

// --- StrainedState ---
StrainedState::StrainedState() : TimerStateBase(Constants::STATE_STRAINED) {}

void StrainedState::update(DefaultTimer &timer, double delta) {
  timer.m_strainedTime += delta;
  timer.m_restTime = 0;
}

std::string StrainedState::handleInput(DefaultTimer &timer, bool isPaused,
                                       bool isLocked) {
  if (isPaused)
    return Constants::STATE_PAUSED;
  if (isLocked)
    return Constants::STATE_SCREEN_LOCKED;
  return getName();
}

// --- PausedState ---
PausedState::PausedState() : TimerStateBase(Constants::STATE_PAUSED) {}

void PausedState::update(DefaultTimer &timer, double delta) {
  timer.m_restTime += delta;

  // Calculate decrease rate based on work/rest ratio
  int workMinutes = timer.m_settings->get<int>("work_minutes");
  int restMinutes = timer.m_settings->get<int>("rest_minutes");
  double ratio = (restMinutes > 0)
                     ? (static_cast<double>(workMinutes) / restMinutes)
                     : 1.0;

  double strainDecrease = delta * ratio;
  timer.m_strainedTime -= strainDecrease;
  if (timer.m_strainedTime < 0)
    timer.m_strainedTime = 0;

  spdlog::debug("Strain decreasing (Paused): -{:.2f}s, Current: {:.2f}s",
                strainDecrease, timer.m_strainedTime);
  spdlog::debug("Rest increasing (Paused): +{:.2f}s, Current: {:.2f}s", delta,
                timer.m_restTime);
}

std::string PausedState::handleInput(DefaultTimer &timer, bool isPaused,
                                     bool isLocked) {
  if (!isPaused) {
    if (isLocked)
      return Constants::STATE_SCREEN_LOCKED;
    return Constants::STATE_STRAINED;
  }
  return getName();
}

// --- ScreenLockedState ---
ScreenLockedState::ScreenLockedState()
    : TimerStateBase(Constants::STATE_SCREEN_LOCKED) {}

void ScreenLockedState::update(DefaultTimer &timer, double delta) {
  timer.m_restTime += delta;

  // Same decrease logic as Paused
  int workMinutes = timer.m_settings->get<int>("work_minutes");
  int restMinutes = timer.m_settings->get<int>("rest_minutes");
  double ratio = (restMinutes > 0)
                     ? (static_cast<double>(workMinutes) / restMinutes)
                     : 1.0;

  double strainDecrease = delta * ratio;
  timer.m_strainedTime -= strainDecrease;
  if (timer.m_strainedTime < 0)
    timer.m_strainedTime = 0;

  spdlog::debug("Strain decreasing (Locked): -{:.2f}s, Current: {:.2f}s",
                strainDecrease, timer.m_strainedTime);
  spdlog::debug("Rest increasing (Locked): +{:.2f}s, Current: {:.2f}s", delta,
                timer.m_restTime);
}

std::string ScreenLockedState::handleInput(DefaultTimer &timer, bool isPaused,
                                           bool isLocked) {
  if (!isLocked) {
    if (isPaused)
      return Constants::STATE_PAUSED;
    return Constants::STATE_STRAINED;
  }
  return getName();
}

} // namespace EyeRest
