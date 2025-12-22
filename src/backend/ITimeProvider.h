#pragma once
#include <chrono>

namespace EyeRest {

class ITimeProvider {
public:
    virtual std::chrono::steady_clock::time_point now() const = 0;
    virtual ~ITimeProvider() = default;
};

} // namespace EyeRest
