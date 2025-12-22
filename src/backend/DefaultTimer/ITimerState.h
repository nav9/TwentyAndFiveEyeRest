#pragma once

#include <string>

namespace EyeRest {

class DefaultTimer; // Forward declaration

class ITimerState {
public:
  virtual ~ITimerState() = default;
  virtual void enter(DefaultTimer &timer) = 0;
  virtual void update(DefaultTimer &timer, double delta) = 0;
  virtual std::string getName() const = 0;

  // Transitions are handled by the states themselves or by the context (timer)
  // For better extensibility, let's allow states to suggest a next state
  virtual std::string handleInput(DefaultTimer &timer, bool isPaused,
                                  bool isLocked) = 0;
};

} // namespace EyeRest
