#pragma once
#include "../src/platform/IOsUtils.h"

namespace iRest {
    class MockOsUtils : public IOsUtils {
    public:
        bool isScreenLocked() const override { return m_locked; }
        bool isSystemIdle() const override { return false; }
        
        void setLocked(bool l) { m_locked = l; }

    private:
        bool m_locked = false;
    };
}
