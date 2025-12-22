#include "OSInfoProvider.h"

namespace EyeRest {

std::string OSInfoProvider::getOSName() const {
#ifdef __linux__
  return "Linux";
#elif _WIN32
  return "Windows";
#elif __APPLE__
  return "MacOS";
#else
  return "Unknown";
#endif
}

} // namespace EyeRest
