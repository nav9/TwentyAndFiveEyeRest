#pragma once
#include "../src/common/ITimeSource.h"

namespace iRest {
    class MockTimeSource : public ITimeSource {
    public:
        void setTime(double ts) { m_timestamp = ts; }
        void advance(double seconds) { m_timestamp += seconds; }
        
        std::chrono::system_clock::time_point now() const override {
            return std::chrono::system_clock::time_point(); // Not used by logic depending on timestamp()
        }
        
        double timestamp() const override {
            return m_timestamp;
        }

    private:
        double m_timestamp = 1000000.0; // Start at some arbitrary non-zero time
    };
}
