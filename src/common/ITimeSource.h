#pragma once

#include <chrono>

namespace iRest {
    class ITimeSource {
    public:
        virtual ~ITimeSource() = default;
        virtual std::chrono::system_clock::time_point now() const = 0;
        virtual double timestamp() const = 0;
    };
}
