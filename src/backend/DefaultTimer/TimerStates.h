#pragma once

#include "../Constants.h"
#include "ITimerState.h"
#include <string>

namespace EyeRest {

class TimerStateBase : public ITimerState {
public:
  void enter(DefaultTimer &timer) override {}
  std::string getName() const override { return m_name; }

protected:
  explicit TimerStateBase(std::string name) : m_name(std::move(name)) {}
  std::string m_name;
};

class StrainedState : public TimerStateBase {
public:
  StrainedState();
  void update(DefaultTimer &timer, double delta) override;
  std::string handleInput(DefaultTimer &timer, bool isPaused,
                          bool isLocked) override;
};

class PausedState : public TimerStateBase {
public:
  PausedState();
  void update(DefaultTimer &timer, double delta) override;
  std::string handleInput(DefaultTimer &timer, bool isPaused,
                          bool isLocked) override;
};

class ScreenLockedState : public TimerStateBase {
public:
  ScreenLockedState();
  void update(DefaultTimer &timer, double delta) override;
  std::string handleInput(DefaultTimer &timer, bool isPaused,
                          bool isLocked) override;
};

} // namespace EyeRest
