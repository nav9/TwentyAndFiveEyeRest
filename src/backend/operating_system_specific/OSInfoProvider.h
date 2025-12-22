#pragma once

#include "../IOSInfoProvider.h"

namespace EyeRest {

class OSInfoProvider : public IOSInfoProvider {
public:
  std::string getOSName() const override;
};

} // namespace EyeRest
