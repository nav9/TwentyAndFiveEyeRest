#pragma once

#include <string>

namespace iRest {
    class IOsUtils {
    public:
        virtual ~IOsUtils() = default;
        virtual bool isScreenLocked() const = 0;
        virtual bool isSystemIdle() const = 0; // Simple idle check
    };
}
