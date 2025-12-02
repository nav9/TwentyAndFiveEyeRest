#pragma once
#include "../common/TimeProvider.hpp"
#include "../common/Constants.hpp"
#include <memory>
#include <deque>
#include <mutex>
#include <chrono>
#include <functional>
#include <string>

enum class DefaultTimerStateId {
    EYES_STRAINED,
    SCREEN_LOCKED,
    PAUSED_VIA_GUI,
    PROGRAM_NOT_RUNNING,
    SUSPENDED,
    RESET
};

struct ActivityRecord {
    double timestamp; // seconds since epoch with fraction
    double elapsed_time; // seconds
    DefaultTimerStateId state;
};

class IDefaultTimerState; // forward

class DefaultTimer {
public:
    DefaultTimer(std::shared_ptr<TimeProvider> tp, const std::string& basePath);
    ~DefaultTimer();

    // Poll to be called by UI periodically (e.g., QTimer every 1s)
    void poll();

    // User controls
    void play();   // resume / normal mode
    void pause();  // pause/rest mode
    void reset();  // reset strained duration to zero

    // settings
    void setWriteInterval(std::chrono::seconds s);
    void setQueueLength(std::size_t n);

    // info for UI
    std::pair<int,int> strainedDurationHrsMins() const; // hours, minutes

    // stop safely
    void stop();

    // register callback to notify UI about state change
    void setStateChangeCallback(std::function<void(DefaultTimerStateId)> cb);

private:
    void loadRecentHistory();
    void writeActivityToFile(const ActivityRecord& rec);
    void rotateLogsIfNeeded();
    void pushRecord(const ActivityRecord& rec);

    mutable std::mutex mtx_;
    std::shared_ptr<TimeProvider> tp_;
    std::string basePath_;
    std::deque<ActivityRecord> queue_;
    std::size_t queueLen_;
    std::chrono::seconds writeInterval_;
    std::chrono::time_point<std::chrono::system_clock> lastWrite_;
    bool running_;

    // basic counters
    double strained_seconds_; // current strained accumulated seconds
    int allowed_strain_seconds_;
    int minimum_rest_seconds_;

    // archive
    std::size_t archive_lines_limit_;
    int archive_index_;

    std::function<void(DefaultTimerStateId)> state_cb_;

    std::unique_ptr<IDefaultTimerState> state_;
};
