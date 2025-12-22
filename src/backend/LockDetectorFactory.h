#pragma once

#include "ILockDetector.h"
#include "IOSInfoProvider.h"
#include "operating_system_specific/LinuxLockDetector.h"
#include "operating_system_specific/OSInfoProvider.h"
#include <memory>

namespace EyeRest {

class LockDetectorFactory {
public:
  static std::unique_ptr<ILockDetector>
  createLockDetector(const IOSInfoProvider &osInfo) {
    std::string osName = osInfo.getOSName();
    if (osName == "Linux") {
      return std::make_unique<LinuxLockDetector>();
    }
    // Fallback or other OS implementations
    return nullptr;
  }
};

} // namespace EyeRest
