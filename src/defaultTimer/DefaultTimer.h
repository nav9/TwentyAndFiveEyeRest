#pragma once

#include "../core/TimerEngine.h"

namespace iRest {
    // This class might just be a wrapper or concrete instantiation of TimerEngine
    // with specific default configuration, effectively the "controller" for the default timer.
    class DefaultTimer {
    public:
        using StateChangeCallback = TimerEngine::StateChangeCallback;
        using TimeUpdateCallback = TimerEngine::TimeUpdateCallback;

        DefaultTimer(const ITimeSource& timeSource, std::unique_ptr<IOsUtils> osUtils);
        
        void start();
        void stop();
        void update();
        void pause();
        void play(); // Resume
        void reset();
        
        void setCallbacks(StateChangeCallback stateCb, TimeUpdateCallback timeCb);
        
        // Expose internally for testing if needed, or proxy methods
        double getStrainedDuration() const;

    private:
        std::unique_ptr<TimerEngine> m_engine;
    };
}
