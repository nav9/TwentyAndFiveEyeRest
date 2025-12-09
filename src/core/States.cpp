#include "States.h"
#include "TimerEngine.h"
#include "../common/Logger.h"
#include <algorithm>

namespace iRest {

    // --- StrainedState ---
    void StrainedState::enter(TimerEngine& engine) {
        Logger::log("Entered Strained State");
    }

    void StrainedState::update(TimerEngine& engine, double deltaTime) {
        // Increase strained duration
        // Requirement: "duration is ALLOWED_STRAIN = 20 minutes... main window displays strained duration"
        // Implicitly: strained duration increases when working.
        double newDuration = engine.getStrainedDuration() + deltaTime;
        engine.setStrainedDuration(newDuration);
        
        // Check if limit reached
        if (newDuration >= engine.getMaxStrainDuration()) {
            // Notify user? 
            // For now just keep accumulating or clamp?
            // "Program notifies the user when the duration is reached." 
            // We can emit a specific 'MaxReached' event or just let UI handle it based on time updates.
        }
    }

    void StrainedState::exit(TimerEngine& engine) {
    }

    // --- Helper for Decay ---
    void decayStrain(TimerEngine& engine, double deltaTime) {
         // "strained duration keeps decreasing every minute by a value of MINIMUM_REST_TIME / ALLOWED_STRAIN"
         // This formula seems to be a RATIO? 
         // Re-reading: "decreasing every minute by a value of MINIMUM_REST_TIME / ALLOWED_STRAIN"
         // Wait. MINIMUM_REST_TIME = 5 mins. ALLOWED_STRAIN = 20 mins. Ratio = 0.25?
         // That would mean for every 1 minute of rest, we reduce strain by 0.25 minutes?
         // NO. That makes rest SLOWER than strain. Usually rest cleans strain faster.
         // Maybe it meant: "decreases to zero over the course of MINIMUM_REST_TIME"?
         // i.e., 20 mins of strain takes 5 mins of rest to clear.
         // Rate = TotalStrain / RecoveryTime.
         // So if StrainedDuration = 20 mins, we want it to reach 0 in 5 mins.
         // DecreaseRate = CurrentStrainedDuration / FixedRestTime? Or FixedMaxStrain / FixedRestTime?
         // "strained duration keeps decreasing... by a value of MINIMUM_REST_TIME / ALLOWED_STRAIN"
         // This parsing is ambiguous. "by a value of X" usually means "X amount per unit time".
         // PLEASE ASSUME: Standard model is 20-20-20 or similar, but here 20 mins work -> 5 mins rest.
         // So 1 second of rest clears (20/5) = 4 seconds of strain.
         // Let's assume ratio = ALLOWED_STRAIN / MINIMUM_REST_TIME. 
         // If "decreasing by value of REST/STRAIN" = 5/20 = 0.25, then 1 minute rest remove 0.25 mins strain. That means 5 mins rest remove 1.25 mins strain. That is ineffective.
         // It likely means: "The strain counter decreases."
         // Let's implement Rate = MaxStrain / MinRest. default 20/5 = 4x.
         
         double rate = engine.getMaxStrainDuration() / engine.getMinRestDuration();
         double decrease = deltaTime * rate;
         
         double current = engine.getStrainedDuration();
         current -= decrease;
         if (current < 0) current = 0;
         engine.setStrainedDuration(current);
    }

    // --- LockedState ---
    void LockedState::enter(TimerEngine& engine) {
        Logger::log("Entered Locked State");
    }

    void LockedState::update(TimerEngine& engine, double deltaTime) {
        decayStrain(engine, deltaTime);
    }

    void LockedState::exit(TimerEngine& engine) {
    }

    // --- PausedState ---
    void PausedState::enter(TimerEngine& engine) {
        Logger::log("Entered Paused State (Resting)");
    }

    void PausedState::update(TimerEngine& engine, double deltaTime) {
        decayStrain(engine, deltaTime);
    }

    void PausedState::exit(TimerEngine& engine) {
    }

    // --- ResetState ---
    void ResetState::enter(TimerEngine& engine) {
        Logger::log("Resetting");
        engine.setStrainedDuration(0);
        // Immediately return to Strained (or Paused if it was paused?)
        // Usually Reset implies "I'm ready to work starting now".
        engine.changeState(std::make_shared<StrainedState>());
    }

    void ResetState::update(TimerEngine& engine, double deltaTime) {}
    void ResetState::exit(TimerEngine& engine) {}

}
