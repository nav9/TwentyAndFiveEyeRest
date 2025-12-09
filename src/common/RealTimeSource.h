#pragma once
#include "ITimeSource.h"

namespace iRest {
    class RealTimeSource : public ITimeSource {
    public:
        std::chrono::system_clock::time_point now() const override {
            return std::chrono::system_clock::now();
        }
        
        double timestamp() const override {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            return std::chrono::duration<double>(duration).count();
        }
    };
}
