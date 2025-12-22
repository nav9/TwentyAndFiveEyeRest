#pragma once
#include "ITimeProvider.h"

namespace EyeRest {

class SystemTimeProvider : public ITimeProvider {
public:
  std::chrono::steady_clock::time_point now() const override {
    return std::chrono::steady_clock::now();
  }
};

} // namespace EyeRest
