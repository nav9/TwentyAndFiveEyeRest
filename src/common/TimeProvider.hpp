#pragma once
#include <chrono>
#include <memory>

class TimeProvider {
public:
    using time_point = std::chrono::system_clock::time_point;
    virtual ~TimeProvider() = default;
    virtual time_point now() const noexcept = 0;
};

class SystemTimeProvider : public TimeProvider {
public:
    TimeProvider::time_point now() const noexcept override {
        return std::chrono::system_clock::now();
    }
};
