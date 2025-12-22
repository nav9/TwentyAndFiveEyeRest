#pragma once

#include <string>

namespace EyeRest {

class IOSInfoProvider {
public:
  virtual ~IOSInfoProvider() = default;
  virtual std::string getOSName() const = 0;
  // Add other OS-specific properties if needed later
};

} // namespace EyeRest
