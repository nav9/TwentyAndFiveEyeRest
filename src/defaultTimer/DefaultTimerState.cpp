#include "DefaultTimerState.hpp"
#include <iostream>

class StrainedState : public IDefaultTimerState {
public:
    DefaultTimerStateId id() const noexcept override { return DefaultTimerStateId::EYES_STRAINED; }
    void onEnter(DefaultTimer& ctx) override { (void)ctx; /* no-op */ }
    void onExit(DefaultTimer& ctx) override { (void)ctx; /* no-op */ }
    void update(DefaultTimer& ctx) override { (void)ctx; /* update handled centrally */ }
};

class PausedState : public IDefaultTimerState {
public:
    DefaultTimerStateId id() const noexcept override { return DefaultTimerStateId::PAUSED_VIA_GUI; }
    void onEnter(DefaultTimer& ctx) override { (void)ctx; /* apply initial actions */ }
    void onExit(DefaultTimer& ctx) override { (void)ctx; /* cleanup */ }
    void update(DefaultTimer& ctx) override { (void)ctx; /* decay handled centrally */ }
};

std::unique_ptr<IDefaultTimerState> makeStrainedState() {
    return std::make_unique<StrainedState>();
}
std::unique_ptr<IDefaultTimerState> makePausedState() {
    return std::make_unique<PausedState>();
}
