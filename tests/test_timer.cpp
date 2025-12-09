#define BOOST_TEST_MODULE TimerTests
#include <boost/test/included/unit_test.hpp>
#include "../src/core/TimerEngine.h"
#include "../src/core/States.h"
#include "MockTimeSource.h"
#include "MockOsUtils.h"

using namespace iRest;

BOOST_AUTO_TEST_CASE(test_initial_state) {
    MockTimeSource ts;
    auto os = std::make_unique<MockOsUtils>();
    TimerConfig config{1200, 300, 30}; // 20 mins, 5 mins, 30s
    
    TimerEngine engine(ts, std::move(os), config);
    engine.start();
    
    BOOST_CHECK_EQUAL(engine.getCurrentStateName(), Constants::Activities::STRAINED);
    BOOST_CHECK_EQUAL(engine.getStrainedDuration(), 0.0);
}

BOOST_AUTO_TEST_CASE(test_strain_accumulation) {
    MockTimeSource ts;
    auto os = std::make_unique<MockOsUtils>();
    TimerConfig config{1200, 300, 30};
    
    TimerEngine engine(ts, std::move(os), config);
    engine.start();
    
    // Simulate 10 seconds
    ts.advance(10.0);
    engine.update();
    
    BOOST_CHECK_CLOSE(engine.getStrainedDuration(), 10.0, 0.001);
}

BOOST_AUTO_TEST_CASE(test_pause_decays_strain) {
    MockTimeSource ts;
    auto os = std::make_unique<MockOsUtils>();
    TimerConfig config{100, 25, 30}; // Max 100s, MinRest 25s. Rate = 4x.
    
    TimerEngine engine(ts, std::move(os), config);
    engine.start();
    
    // Strain for 50s
    ts.advance(50.0);
    engine.update();
    BOOST_CHECK_CLOSE(engine.getStrainedDuration(), 50.0, 0.001);
    
    // Pause
    engine.pause();
    BOOST_CHECK_EQUAL(engine.getCurrentStateName(), Constants::Activities::PAUSED);
    
    // Rest for 5s. Should reduce by 5 * 4 = 20s.
    ts.advance(5.0);
    engine.update();
    
    // Expected: 50 - 20 = 30
    BOOST_CHECK_CLOSE(engine.getStrainedDuration(), 30.0, 0.001);
}

BOOST_AUTO_TEST_CASE(test_screen_lock_decays_strain) {
    MockTimeSource ts;
    auto osMockRaw = new MockOsUtils(); // Keep pointer to control it
    std::unique_ptr<IOsUtils> os(osMockRaw);
    
    TimerConfig config{100, 25, 30}; 
    TimerEngine engine(ts, std::move(os), config);
    engine.start();
    
    ts.advance(40.0);
    engine.update();
    BOOST_CHECK_CLOSE(engine.getStrainedDuration(), 40.0, 0.001);
    
    // Lock screen
    osMockRaw->setLocked(true);
    engine.update(); // Detects lock
    BOOST_CHECK_EQUAL(engine.getCurrentStateName(), Constants::Activities::SCREEN_LOCKED);
    
    // Rest for 2s. Reduce by 2*4 = 8s.
    ts.advance(2.0);
    engine.update();
    
    // Expected: 40 - 8 = 32
    BOOST_CHECK_CLOSE(engine.getStrainedDuration(), 32.0, 0.001);
    
    // Unlock
    osMockRaw->setLocked(false);
    engine.update();
    BOOST_CHECK_EQUAL(engine.getCurrentStateName(), Constants::Activities::STRAINED); // Or previous state
}
