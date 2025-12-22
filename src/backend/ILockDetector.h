#pragma once

#include <string>
#include <vector>

namespace EyeRest {

struct LockProbe {
  std::string name;
  std::string description;
};

class ILockDetector {
public:
  virtual ~ILockDetector() = default;
  virtual bool isScreenLocked() = 0;
  virtual std::string getDetectorName() const = 0;
  virtual void setPreferredMethod(const std::string &method) {}

  virtual std::vector<LockProbe> getProbes() const { return {}; }
  virtual bool runProbe(const std::string &probeName) { return false; }
};

} // namespace EyeRest
