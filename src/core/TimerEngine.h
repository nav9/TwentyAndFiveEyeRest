#pragma once

#include "IState.h"
#include "TimeLogger.h"
#include "../common/ITimeSource.h"
#include "../platform/IOsUtils.h"
#include <memory>
#include <functional>

namespace iRest {
    
    struct TimerConfig {
        double maxStrainDuration; // Seconds
        double minRestDuration;   // Seconds
        int logInterval;          // Seconds
    };

    class TimerEngine {
    public:
        TimerEngine(const ITimeSource& timeSource, 
                   std::unique_ptr<IOsUtils> osUtils,
                   const TimerConfig& config);
        
        void start();
        void stop();
        void update(); // Main loop tick
        
        // Commands
        void pause();
        void resume();
        void reset();
        
        // State management
        void changeState(std::shared_ptr<IState> newState);
        std::string getCurrentStateName() const;

        // Data access
        double getStrainedDuration() const;
        void setStrainedDuration(double duration);
        double getMaxStrainDuration() const;
        double getMinRestDuration() const;
        
        // Callbacks for UI
        using StateChangeCallback = std::function<void(const std::string&)>;
        using TimeUpdateCallback = std::function<void(double)>;
        
        void setStateChangeCallback(StateChangeCallback cb);
        void setTimeUpdateCallback(TimeUpdateCallback cb);

    private:
        const ITimeSource& m_timeSource;
        std::unique_ptr<IOsUtils> m_osUtils;
        TimerConfig m_config;
        TimeLogger m_logger;
        
        std::shared_ptr<IState> m_currentState;
        
        double m_strainedDuration; // Accumulated strain
        bool m_running;
        bool m_pausedByUser;
        
        double m_lastTickTimestamp;
        double m_lastLogTimestamp;

        StateChangeCallback m_stateChangeCb;
        TimeUpdateCallback m_timeUpdateCb;
        
        friend class StrainedState;
        friend class RestState;
        friend class LockedState;
        // ... friend other states
    };
}
