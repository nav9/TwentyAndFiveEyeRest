#include "DefaultTimer.hpp"
#include "DefaultTimerState.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>

namespace fs = std::filesystem;

static std::string stateToString(DefaultTimerStateId s) {
    switch(s) {
        case DefaultTimerStateId::EYES_STRAINED: return "strained";
        case DefaultTimerStateId::SCREEN_LOCKED: return "screen_locked";
        case DefaultTimerStateId::PAUSED_VIA_GUI: return "paused";
        case DefaultTimerStateId::PROGRAM_NOT_RUNNING: return "program_not_running";
        case DefaultTimerStateId::SUSPENDED: return "suspended";
        case DefaultTimerStateId::RESET: return "reset";
    }
    return "unknown";
}

DefaultTimer::DefaultTimer(std::shared_ptr<TimeProvider> tp, const std::string& basePath)
: tp_(std::move(tp)), basePath_(basePath), queueLen_(tfe::constants::DEFAULT_QUEUE_LENGTH),
  writeInterval_(tfe::constants::DEFAULT_WRITE_INTERVAL),
  lastWrite_(std::chrono::system_clock::now()), running_(true),
  strained_seconds_(0.0), allowed_strain_seconds_(tfe::constants::DEFAULT_ALLOWED_STRAIN_SECONDS),
  minimum_rest_seconds_(tfe::constants::DEFAULT_MINIMUM_REST_SECONDS),
  archive_lines_limit_(tfe::constants::DEFAULT_ARCHIVE_LINES),
  archive_index_(0)
{
    try { fs::create_directories(basePath_); } catch(...) {}
    loadRecentHistory();
    // initial state (can be improved based on history)
    state_ = makeStrainedState();
}

DefaultTimer::~DefaultTimer() {
    stop();
}

void DefaultTimer::stop() {
    std::lock_guard<std::mutex> lk(mtx_);
    running_ = false;
    // flush queue last item
    if(!queue_.empty()) {
        try { writeActivityToFile(queue_.back()); } catch(...) {}
    }
}

void DefaultTimer::setWriteInterval(std::chrono::seconds s) {
    std::lock_guard<std::mutex> lk(mtx_);
    writeInterval_ = s;
}

void DefaultTimer::setQueueLength(std::size_t n) {
    std::lock_guard<std::mutex> lk(mtx_);
    if(n < 10) n = 10;
    queueLen_ = n;
    while(queue_.size() > queueLen_) queue_.pop_front();
}

void DefaultTimer::play() {
    std::lock_guard<std::mutex> lk(mtx_);
    // transition to strained state
    if(state_ && state_->id() != DefaultTimerStateId::EYES_STRAINED) {
        state_->onExit(*this);
        state_ = makeStrainedState();
        state_->onEnter(*this);
        if(state_cb_) state_cb_(state_->id());
    }
}

void DefaultTimer::pause() {
    std::lock_guard<std::mutex> lk(mtx_);
    if(state_ && state_->id() != DefaultTimerStateId::PAUSED_VIA_GUI) {
        state_->onExit(*this);
        state_ = makePausedState();
        state_->onEnter(*this);
        if(state_cb_) state_cb_(state_->id());
    }
}

void DefaultTimer::reset() {
    std::lock_guard<std::mutex> lk(mtx_);
    strained_seconds_ = 0.0;
    // write reset record immediately
    ActivityRecord rec;
    auto now = tp_->now();
    rec.timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count();
    rec.elapsed_time = 0.0;
    rec.state = DefaultTimerStateId::RESET;
    pushRecord(rec);
    try { writeActivityToFile(rec); } catch(...) {}
}

void DefaultTimer::pushRecord(const ActivityRecord& rec) {
    queue_.push_back(rec);
    if(queue_.size() > queueLen_) queue_.pop_front();
}

void DefaultTimer::poll() {
    std::lock_guard<std::mutex> lk(mtx_);
    if(!running_) return;

    auto now = tp_->now();
    ActivityRecord rec;
    rec.timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count();

    // Basic behavior: if in PAUSED state, decay strained_seconds_; else increment
    if(state_ && state_->id() == DefaultTimerStateId::PAUSED_VIA_GUI) {
        // decay per second: amount = MINIMUM_REST_TIME / ALLOWED_STRAIN per minute; we'll map to seconds
        double decay_per_sec = static_cast<double>(minimum_rest_seconds_) / static_cast<double>(allowed_strain_seconds_) / 60.0;
        if(decay_per_sec < 0) decay_per_sec = 0;
        strained_seconds_ -= decay_per_sec;
        if(strained_seconds_ < 0) strained_seconds_ = 0;
        rec.state = DefaultTimerStateId::PAUSED_VIA_GUI;
        rec.elapsed_time = decay_per_sec;
    } else {
        // normal: increment strained time by 1 second per poll (assuming poll called once per second from UI)
        strained_seconds_ += 1.0;
        rec.state = DefaultTimerStateId::EYES_STRAINED;
        rec.elapsed_time = 1.0;
    }

    pushRecord(rec);

    // if it's time to write
    auto sec_since_write = std::chrono::duration_cast<std::chrono::seconds>(now - lastWrite_).count();
    if(sec_since_write >= writeInterval_.count()) {
        try { writeActivityToFile(rec); } catch(...) {}
        lastWrite_ = now;
        rotateLogsIfNeeded();
    }
}

std::pair<int,int> DefaultTimer::strainedDurationHrsMins() const {
    std::lock_guard<std::mutex> lk(mtx_);
    int total = static_cast<int>(strained_seconds_);
    int hrs = total / 3600;
    int mins = (total % 3600) / 60;
    return {hrs, mins};
}

void DefaultTimer::setStateChangeCallback(std::function<void(DefaultTimerStateId)> cb) {
    state_cb_ = cb;
}

void DefaultTimer::writeActivityToFile(const ActivityRecord& rec) {
    fs::path file = fs::path(basePath_) / "timeFile.txt";
    std::ofstream ofs;
    ofs.open(file, std::ios::app);
    if(!ofs.is_open()) throw std::runtime_error("timeFile open failed");
    // Build human readable datetime from timestamp
    std::time_t tsec = static_cast<std::time_t>(rec.timestamp);
    std::tm tm = *std::localtime(&tsec);
    std::ostringstream dtoss;
    dtoss << std::put_time(&tm, "%d %b %Y %H:%M:%S");
    ofs << "{'datetime': '" << dtoss.str()
        << "', 'timestamp': " << std::fixed << std::setprecision(6) << rec.timestamp
        << ", 'elapsed_time': " << rec.elapsed_time
        << ", 'activity': '" << stateToString(rec.state) << "'}" << "\n";
    ofs.close();
}

void DefaultTimer::rotateLogsIfNeeded() {
    fs::path file = fs::path(basePath_) / "timeFile.txt";
    if(!fs::exists(file)) return;
    // Count lines
    std::ifstream ifs(file);
    std::size_t lines = 0;
    std::string tmp;
    while(std::getline(ifs, tmp)) ++lines;
    if(lines >= archive_lines_limit_) {
        ++archive_index_;
        fs::path dest = fs::path(basePath_) / ("Archive" + std::to_string(archive_index_) + "_timeFile.txt");
        fs::rename(file, dest);
        std::ofstream ofs(file); ofs.close();
    }
}

void DefaultTimer::loadRecentHistory() {
    fs::path file = fs::path(basePath_) / "timeFile.txt";
    std::vector<std::string> lines;
    if(fs::exists(file)) {
        std::ifstream ifs(file);
        std::string ln;
        while(std::getline(ifs, ln)) {
            lines.push_back(ln);
            if(lines.size() > queueLen_) lines.erase(lines.begin());
        }
    } else {
        // try archives: gather ArchiveN files in directory lexicographically
        for(auto &entry : fs::directory_iterator(basePath_)) {
            auto name = entry.path().filename().string();
            if(name.rfind("Archive", 0) == 0) {
                std::ifstream ifs(entry.path());
                std::string ln;
                while(std::getline(ifs, ln)) {
                    lines.push_back(ln);
                    if(lines.size() > queueLen_) lines.erase(lines.begin());
                }
            }
        }
    }

    // parse entries (best-effort)
    for(const auto &ln : lines) {
        try {
            size_t posAct = ln.find("'activity':");
            if(posAct == std::string::npos) continue;
            size_t q1 = ln.find("'", posAct + 11);
            size_t q2 = ln.find("'", q1 + 1);
            std::string activity = ln.substr(q1 + 1, q2 - q1 - 1);

            size_t posTs = ln.find("'timestamp':");
            double ts = 0.0;
            if(posTs != std::string::npos) {
                size_t comma = ln.find(",", posTs);
                std::string tsStr = ln.substr(posTs + 12, comma - posTs - 12);
                ts = std::stod(tsStr);
            }

            size_t posEl = ln.find("'elapsed_time':");
            double elapsed = 0.0;
            if(posEl != std::string::npos) {
                size_t comma2 = ln.find(",", posEl);
                std::string elStr = ln.substr(posEl + 16, comma2 - posEl - 16);
                elapsed = std::stod(elStr);
            }

            DefaultTimerStateId sid = DefaultTimerStateId::EYES_STRAINED;
            if(activity == "strained") sid = DefaultTimerStateId::EYES_STRAINED;
            else if(activity == "screen_locked") sid = DefaultTimerStateId::SCREEN_LOCKED;
            else if(activity == "paused") sid = DefaultTimerStateId::PAUSED_VIA_GUI;
            else if(activity == "program_not_running") sid = DefaultTimerStateId::PROGRAM_NOT_RUNNING;
            else if(activity == "suspended") sid = DefaultTimerStateId::SUSPENDED;
            else if(activity == "reset") sid = DefaultTimerStateId::RESET;

            ActivityRecord rec{ts, elapsed, sid};
            pushRecord(rec);
            // update strained_seconds_ conservatively
            if(sid == DefaultTimerStateId::EYES_STRAINED) strained_seconds_ += elapsed;
        } catch(...) {
            continue;
        }
    }
}
