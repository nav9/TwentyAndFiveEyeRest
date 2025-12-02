#define BOOST_TEST_MODULE DefaultTimerTest
#include <boost/test/included/unit_test.hpp>
#include "../src/common/TimeProvider.hpp"
#include "../src/defaultTimer/DefaultTimer.hpp"
#include <memory>
#include <chrono>

// Fake time provider for tests
class FakeTimeProvider : public TimeProvider {
public:
    FakeTimeProvider(std::chrono::system_clock::time_point start) : t(start) {}
    TimeProvider::time_point now() const noexcept override { return t; }
    void advance(std::chrono::seconds s) { t += s; }
private:
    mutable std::chrono::system_clock::time_point t;
};

BOOST_AUTO_TEST_CASE(basic_write_and_queue) {
    auto start = std::chrono::system_clock::now();
    auto ftp = std::make_shared<FakeTimeProvider>(start);
    DefaultTimer dt(ftp, "./test_timefiles");
    dt.setWriteInterval(std::chrono::seconds(1));
    dt.poll(); // one second (initial)
    ftp->advance(std::chrono::seconds(1));
    dt.poll();
    // check that querying strained duration returns ints without crash
    auto hm = dt.strainedDurationHrsMins();
    BOOST_CHECK(hm.first >= 0);
    BOOST_CHECK(hm.second >= 0);
    dt.stop();
}
