#pragma once

#include "../ILockDetector.h"
#include <string>
#include <vector>

namespace EyeRest {

class LinuxLockDetector : public ILockDetector {
public:
  bool isScreenLocked() override;
  std::string getDetectorName() const override { return "LinuxLockDetector"; }

  std::vector<LockProbe> getProbes() const override;
  bool runProbe(const std::string &probeName) override;
  void setPreferredMethod(const std::string &method) override {
    m_preferredMethod = method;
  }

private:
  std::string m_preferredMethod;
  bool checkDBus(const std::string &service, const std::string &path,
                 const std::string &interface, const std::string &method);
};

} // namespace EyeRest
