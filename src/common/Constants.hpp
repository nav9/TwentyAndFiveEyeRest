#pragma once
#include <chrono>

namespace tfe {
namespace constants {
static constexpr std::chrono::seconds DEFAULT_WRITE_INTERVAL{30};
static constexpr std::size_t DEFAULT_QUEUE_LENGTH = 100;
static constexpr std::size_t DEFAULT_ARCHIVE_LINES = 1000;
static constexpr int DEFAULT_ALLOWED_STRAIN_SECONDS = 20 * 60; // 20 min
static constexpr int DEFAULT_MINIMUM_REST_SECONDS = 5 * 60; // 5 min
} // namespace constants
} // namespace tfe
