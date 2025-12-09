#pragma once

#include <string>

namespace iRest {
    class TimerEngine;

    class IState {
    public:
        virtual ~IState() = default;
        virtual void enter(TimerEngine& engine) = 0;
        virtual void update(TimerEngine& engine, double deltaTime) = 0;
        virtual void exit(TimerEngine& engine) = 0;
        virtual std::string getName() const = 0;
    };
}
