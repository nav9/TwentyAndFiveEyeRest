#include "DefaultTimer.h"
#include "../common/Constants.h"

namespace iRest {

    DefaultTimer::DefaultTimer(const ITimeSource& timeSource, std::unique_ptr<IOsUtils> osUtils) {
        TimerConfig config;
        config.maxStrainDuration = Constants::DEFAULT_STRAIN_DURATION_MINUTES * 60;
        config.minRestDuration = Constants::MINIMUM_REST_TIME_MINUTES * 60;
        config.logInterval = Constants::LOG_INTERVAL_SECONDS;
        
        m_engine = std::make_unique<TimerEngine>(timeSource, std::move(osUtils), config);
    }

    void DefaultTimer::start() { m_engine->start(); }
    void DefaultTimer::stop() { m_engine->stop(); }
    void DefaultTimer::update() { m_engine->update(); }
    void DefaultTimer::pause() { m_engine->pause(); }
    void DefaultTimer::play() { m_engine->resume(); }
    void DefaultTimer::reset() { m_engine->reset(); }

    void DefaultTimer::setCallbacks(StateChangeCallback stateCb, TimeUpdateCallback timeCb) {
        m_engine->setStateChangeCallback(stateCb);
        m_engine->setTimeUpdateCallback(timeCb);
    }

    double DefaultTimer::getStrainedDuration() const {
        return m_engine->getStrainedDuration();
    }
}
