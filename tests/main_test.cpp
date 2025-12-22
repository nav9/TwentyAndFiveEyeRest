#define BOOST_TEST_MODULE EyeRestTests
#include "Core.h"
#include "ITimeProvider.h"
#include <boost/test/included/unit_test.hpp>

// Mock Time Provider
class MockTimeProvider : public EyeRest::ITimeProvider {
public:
  std::chrono::steady_clock::time_point now() const override { return m_time; }

  void setTime(std::chrono::steady_clock::time_point time) { m_time = time; }

private:
  std::chrono::steady_clock::time_point m_time;
};

BOOST_AUTO_TEST_CASE(test_hello_world) { BOOST_TEST(true, "Hello World Test"); }

BOOST_AUTO_TEST_CASE(test_core_initialization) {
  auto mockTime = std::make_shared<MockTimeProvider>();
  auto fs = std::make_shared<EyeRest::Filesystem>();
  // Use a temporary settings file for testing or default
  auto settings = std::make_shared<EyeRest::Settings>(fs, "test_settings.json");
  settings->initializeDefaults();
  EyeRest::Core core(settings, fs, mockTime, false);

  BOOST_TEST(core.isRunning() == false);

  core.start();
  BOOST_TEST(core.isRunning() == true);

  core.stop();
  BOOST_TEST(core.isRunning() == false);
}
