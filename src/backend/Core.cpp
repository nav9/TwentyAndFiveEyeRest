#include "Core.h"
#include "Constants.h"
#include "LockDetectorFactory.h"
#include "TimeFileManager.h"
#include "operating_system_specific/OSInfoProvider.h"
#include <chrono>
#include <iostream>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

namespace EyeRest {

Core::Core(std::shared_ptr<Settings> settings, std::shared_ptr<Filesystem> fs,
           std::shared_ptr<ITimeProvider> timeProvider, bool debugMode)
    : m_settings(std::move(settings)), m_fs(std::move(fs)),
      m_timeProvider(std::move(timeProvider)), m_running(false) {
  // Configure logging based on settings
  try {
    int fileSizeMB = m_settings->get<int>("log_file_size");
    int fileCount = m_settings->get<int>("log_file_count");

    // Ensure logs directory exists
    std::string logDir = "logs";
    if (!m_fs->directoryExists(logDir)) {
      m_fs->createDirectory(logDir);
    }

    // Rotating sink
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        Constants::LOG_FILE_NAME, fileSizeMB * 1024 * 1024, fileCount);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(console_sink);
    sinks.push_back(file_sink);

    m_logger =
        std::make_shared<spdlog::logger>("Core", sinks.begin(), sinks.end());
    spdlog::register_logger(m_logger);
    spdlog::set_default_logger(m_logger);

    if (debugMode) {
      m_logger->set_level(spdlog::level::debug);
      m_logger->debug("Core debug logging enabled via flag");
    } else {
      m_logger->set_level(spdlog::level::info);
    }

    m_logger->info("Core initialized");
  } catch (const std::exception &e) { // Fallback
    m_logger = spdlog::stdout_color_mt("console");
    m_logger->error("Failed to setup rotating log: {}", e.what());
  }

  // Initialize OS specific components
  m_osInfo = std::make_shared<OSInfoProvider>();
  m_lockDetector = LockDetectorFactory::createLockDetector(*m_osInfo);
  if (m_lockDetector) {
    std::string preferred =
        m_settings->get<std::string>("preferred_lock_detection_method");
    if (!preferred.empty()) {
      m_lockDetector->setPreferredMethod(preferred);
      m_logger->info("Lock detector initialized with preferred method: {} ({})",
                     m_lockDetector->getDetectorName(), preferred);
    } else {
      m_logger->info("Lock detector initialized: {}",
                     m_lockDetector->getDetectorName());
    }
  } else {
    m_logger->warn("No lock detector available for OS: {}",
                   m_osInfo->getOSName());
  }

  // Initialize Timer. Create TimeFileManager
  auto timeFileManager =
      std::make_shared<TimeFileManager>(m_fs, m_settings, "DefaultTimer");
  m_defaultTimer = std::make_shared<DefaultTimer>(
      m_settings, m_fs, m_timeProvider, timeFileManager);
}

Core::~Core() { stop(); }

void Core::start() {
  if (m_running) {
    return;
  }
  m_running = true;
  m_logger->info("Starting Core service...");
  m_defaultTimer->start();
  m_thread = std::thread(&Core::runLoop, this);
}

void Core::stop() {
  if (!m_running) {
    return;
  }
  m_running = false;
  m_logger->info("Stopping Core service...");
  if (m_defaultTimer) {
    m_defaultTimer->stop();
  }
  if (m_thread.joinable()) {
    m_thread.join();
  }
  m_logger->info("Core service stopped");
}

bool Core::isRunning() const { return m_running; }

void Core::runLoop() {
  m_logger->info("Core orchestration loop started");
  while (m_running) {
    if (m_lockDetector) {
      bool locked = m_lockDetector->isScreenLocked();
      m_defaultTimer->setScreenLocked(locked);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

} // namespace EyeRest
