#include "../src/backend/Filesystem.h"
#include "../src/backend/Settings.h"
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

int main() {
  auto fs = std::make_shared<EyeRest::Filesystem>();
  auto settings =
      std::make_shared<EyeRest::Settings>(fs, "test_new_settings.json");

  settings->initializeDefaults();
  settings->save();

  std::cout << "Settings saved to test_new_settings.json" << std::endl;
  return 0;
}
