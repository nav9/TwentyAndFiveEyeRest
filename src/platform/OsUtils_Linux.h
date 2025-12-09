#pragma once
#include "IOsUtils.h"
#include <string>

namespace iRest {
    class OsUtils_Linux : public IOsUtils {
    public:
        bool isScreenLocked() const override;
        bool isSystemIdle() const override;

    private:
        std::string exec(const char* cmd) const;
    };
}
