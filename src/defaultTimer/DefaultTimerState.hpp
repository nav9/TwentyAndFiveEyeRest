#pragma once
#include "DefaultTimer.hpp"
#include <memory>

class IDefaultTimerState {
public:
    virtual ~IDefaultTimerState() = default;
    virtual DefaultTimerStateId id() const noexcept = 0;
    virtual void onEnter(DefaultTimer& ctx) = 0;
    virtual void onExit(DefaultTimer& ctx) = 0;
    virtual void update(DefaultTimer& ctx) = 0;
};

std::unique_ptr<IDefaultTimerState> makeStrainedState();
std::unique_ptr<IDefaultTimerState> makePausedState();
