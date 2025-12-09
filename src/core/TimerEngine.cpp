#include "TimerEngine.h"
#include "../common/Logger.h"
#include "../common/Constants.h"
#include "States.h" 

#include <iostream>

namespace iRest {

    TimerEngine::TimerEngine(const ITimeSource& timeSource, 
                             std::unique_ptr<IOsUtils> osUtils,
                             const TimerConfig& config)
        : m_timeSource(timeSource),
          m_osUtils(std::move(osUtils)),
          m_config(config),
          m_logger(timeSource),
          m_strainedDuration(0),
          m_running(false),
          m_pausedByUser(false),
          m_lastTickTimestamp(0),
          m_lastLogTimestamp(0)
    {
        // Initial state logic should be determined by HistoryAnalyzer (TODO)
        // For now, start in StrainedState or whatever default
        // We defer setting initial state to 'start()'
    }

    void TimerEngine::start() {
        if (m_running) return;
        m_running = true;
        m_lastTickTimestamp = m_timeSource.timestamp();
        m_lastLogTimestamp = m_lastTickTimestamp;
        
        // Default start state
        changeState(std::make_shared<StrainedState>()); // Forward declared in header, included in cpp
    }

    void TimerEngine::stop() {
        m_running = false;
        if (m_currentState) {
            m_currentState->exit(*this);
            m_currentState = nullptr;
        }
    }

    void TimerEngine::changeState(std::shared_ptr<IState> newState) {
        if (m_currentState) {
            m_currentState->exit(*this);
        }
        m_currentState = newState;
        if (m_currentState) {
            m_currentState->enter(*this);
            if (m_stateChangeCb) m_stateChangeCb(m_currentState->getName());
        }
    }

    void TimerEngine::update() {
        if (!m_running || !m_currentState) return;

        double currentTimestamp = m_timeSource.timestamp();
        double deltaTime = currentTimestamp - m_lastTickTimestamp;
        m_lastTickTimestamp = currentTimestamp;

        // Delegate update to current state
        m_currentState->update(*this, deltaTime);

        // Logging at intervals
        if (currentTimestamp - m_lastLogTimestamp >= m_config.logInterval) {
            m_logger.log(m_currentState->getName(), (m_currentState->getName() == Constants::Activities::STRAINED ? m_strainedDuration : deltaTime)); // Simplification: logging accumulating strain or delta? Requirements say "elapsed_time is number of seconds program has been in for any state". 
            // Correct interpretation: For 'activity': 'strained', elapsed_time usually means duration of that specific log block?
            // "elapsed_time is the number of seconds that the program has been in for any state"
            // Actually, the example shows small values like 1, 2.18, 16.02. It seems to be the delta since last log.
            // Let's log the actual delta since last log for simplicity and accuracy.
            
            // Re-evaluating requirement: 
            // "strained duration keeps decreasing... until it reaches 00" -> This is the 'accumulator'.
            // "elapsed_time for any state" for Logging -> This is likely the duration spent IN THAT STATE since last check.
            
            m_logger.log(m_currentState->getName(), currentTimestamp - m_lastLogTimestamp);
            m_lastLogTimestamp = currentTimestamp;
        }

        // Check global interrupts (Screen Lock, Pause)
        // Note: State classes should handle specific transitions, but high-level overrides can happen here
        // or be queried by states. Use IOsUtils.
        
        bool locked = m_osUtils->isScreenLocked();
        if (locked && m_currentState->getName() != Constants::Activities::SCREEN_LOCKED) {
             changeState(std::make_shared<LockedState>());
        }
        else if (!locked && m_currentState->getName() == Constants::Activities::SCREEN_LOCKED) {
             // Return to previous state logic or default Strained?
             // Since we don't need a full pushdown automaton, simple return to Strained is usually fine, 
             // but if user was Paused, we should return to Paused.
             if (m_pausedByUser) changeState(std::make_shared<PausedState>());
             else changeState(std::make_shared<StrainedState>());
        }

        if (m_timeUpdateCb) m_timeUpdateCb(m_strainedDuration);
    }

    void TimerEngine::pause() {
        m_pausedByUser = true;
        changeState(std::make_shared<PausedState>());
    }

    void TimerEngine::resume() {
        m_pausedByUser = false;
        changeState(std::make_shared<StrainedState>());
    }

    void TimerEngine::reset() {
        m_strainedDuration = 0;
        changeState(std::make_shared<ResetState>());
        // ResetState might transition immediately back to Strained or Rest
    }
    
    double TimerEngine::getStrainedDuration() const { return m_strainedDuration; }
    void TimerEngine::setStrainedDuration(double duration) { m_strainedDuration = duration; }
    
    double TimerEngine::getMaxStrainDuration() const { return m_config.maxStrainDuration; }
    double TimerEngine::getMinRestDuration() const { return m_config.minRestDuration; }

    std::string TimerEngine::getCurrentStateName() const {
        return m_currentState ? m_currentState->getName() : "Unknown";
    }

    void TimerEngine::setStateChangeCallback(StateChangeCallback cb) { m_stateChangeCb = cb; }
    void TimerEngine::setTimeUpdateCallback(TimeUpdateCallback cb) { m_timeUpdateCb = cb; }

}
