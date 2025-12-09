#pragma once

#include "IState.h"
#include "../common/Constants.h"

namespace iRest {

    class StrainedState : public IState {
    public:
        void enter(TimerEngine& engine) override;
        void update(TimerEngine& engine, double deltaTime) override;
        void exit(TimerEngine& engine) override;
        std::string getName() const override { return Constants::Activities::STRAINED; }
    };

    class RestState : public IState {
         // Maybe implicitly treated as 'Locked' or 'Paused' is rest?
         // Requirement says: "Pressing pause... program will switch to the rest mode where strained duration keeps decreasing"
         // So Paused IS a Rest state. Locked IS a Rest state.
         // Do we need a dedicated "RestState" that happens automatically?
         // Maybe when duration reaches limit? "Program notifies the user when the duration is reached."
         // It doesn't force a lock. It just notifies.
         // But effectively, if user stops interacting, it becomes Rest.
         // Let's assume RestState is what happens when Timer forces a break? 
         // Or is it just a generalized base for Paused/Locked?
    };

    class LockedState : public IState {
    public:
        void enter(TimerEngine& engine) override;
        void update(TimerEngine& engine, double deltaTime) override;
        void exit(TimerEngine& engine) override;
        std::string getName() const override { return Constants::Activities::SCREEN_LOCKED; }
    };

    class PausedState : public IState {
    public:
        void enter(TimerEngine& engine) override;
        void update(TimerEngine& engine, double deltaTime) override;
        void exit(TimerEngine& engine) override;
        std::string getName() const override { return Constants::Activities::PAUSED; }
    };
    
    class ResetState : public IState {
    public:
         void enter(TimerEngine& engine) override;
         void update(TimerEngine& engine, double deltaTime) override;
         void exit(TimerEngine& engine) override;
         std::string getName() const override { return Constants::Activities::RESET; }
    };
}
