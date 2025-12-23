#include "Constants.h"
#include "Core.h"
#include "Filesystem.h"
#include "Settings.h"
#include "SystemTimeProvider.h"
#include <CLI/CLI.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <thread>

std::unique_ptr<EyeRest::Core> coreService;

void signal_handler(int signal) {
  if (coreService) {
    spdlog::info("Signal {} received, stopping service...", signal);
    coreService->stop();
  }
}

void runLockScreenCheck(std::shared_ptr<EyeRest::Filesystem> fs,
                        std::shared_ptr<EyeRest::Settings> settings);

int main(int argc, char **argv) {
  CLI::App app{"TwentyAndFive Eye Rest"};

  bool debug = false;
  std::string configPath = "settings.json";

  app.add_flag("-d,--debug", debug, "Enable debug logging");
  app.add_option("-c,--config", configPath, "Path to configuration file");

  int logFileSize = -1;
  app.add_option("--log-file-size", logFileSize,
                 "Max size of rotating log file in MB");

  int logFileCount = -1;
  app.add_option("--log-file-count", logFileCount,
                 "Number of rotating log files to keep");

  bool checkLockscreen = false;
  app.add_flag("--check-lockscreen", checkLockscreen,
               "Interactively check and verify screen lock detection");

  CLI11_PARSE(app, argc, argv);

  // Bootstrap logging for startup
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("Bootstrap", console_sink);
  spdlog::register_logger(logger);
  spdlog::set_default_logger(logger);

  if (debug) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Debug logging enabled");
  }

  spdlog::info("Starting TwentyAndFive Eye Rest");

  // Initialize Dependencies
  auto fs = std::make_shared<EyeRest::Filesystem>();

  auto settings = std::make_shared<EyeRest::Settings>(fs, configPath);

  // Initialize defaults first
  settings->initializeDefaults();

  // Load file (overwrites defaults with file values)
  settings->load();
  settings->save(); // Force update to new format

  // Apply CLI overrides (overwrites file values)
  if (logFileSize > 0) {
    settings->set("log_file_size", logFileSize);
  }
  if (logFileCount > 0) {
    settings->set("log_file_count", logFileCount);
  }

  // Setup Logging with Settings
  try {
    int sizeMB = settings->get<int>("log_file_size");
    int count = settings->get<int>("log_file_count");

    // Ensure logs directory exists
    if (!fs->directoryExists("logs")) {
      fs->createDirectory("logs");
    }

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        EyeRest::Constants::LOG_FILE_NAME, sizeMB * 1024 * 1024, count);
    auto console_sink_main =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(console_sink_main);
    sinks.push_back(file_sink);

    auto logger_main =
        std::make_shared<spdlog::logger>("EyeRest", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger_main);

    if (debug) {
      spdlog::set_level(spdlog::level::debug);
      spdlog::debug("Debug logging enabled");
    }

    spdlog::info("Logging initialized using Settings: Size={}MB, Count={}",
                 sizeMB, count);

  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize logging: " << e.what() << std::endl;
  }

  if (checkLockscreen) {
    runLockScreenCheck(fs, settings);
    return 0;
  }

  // Start Core
  auto timeProvider = std::make_shared<EyeRest::SystemTimeProvider>();
  coreService =
      std::make_unique<EyeRest::Core>(settings, fs, timeProvider, debug);

  // Register signal handler for clean shutdown
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  coreService->start();

  // Keep main thread alive while service runs
  while (coreService->isRunning()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  spdlog::info("Exiting application");
  return 0;
}

#include "LockDetectorFactory.h"
#include "operating_system_specific/OSInfoProvider.h"

void runLockScreenCheck(std::shared_ptr<EyeRest::Filesystem> fs,
                        std::shared_ptr<EyeRest::Settings> settings) {
  std::cout << "\n=== Screen Lock Detection Check ===\n" << std::endl;

  EyeRest::OSInfoProvider osInfo;
  auto detector = EyeRest::LockDetectorFactory::createLockDetector(osInfo);

  if (!detector) {
    std::cout << "Error: Could not create a lock detector for your OS ("
              << osInfo.getOSName() << ")." << std::endl;
    return;
  }

  auto probes = detector->getProbes();
  if (probes.empty()) {
    std::cout << "This detector (" << detector->getDetectorName()
              << ") does not have granular probes yet." << std::endl;
    std::cout << "General check: "
              << (detector->isScreenLocked() ? "LOCKED" : "UNLOCKED")
              << std::endl;
    return;
  }

  std::cout << "Detected OS: " << osInfo.getOSName() << std::endl;
  std::cout << "Detector: " << detector->getDetectorName() << std::endl;
  std::cout << "Available probes: " << probes.size() << "\n" << std::endl;

  std::cout << "I will now enter a loop. Please LOCK your screen within 5 "
               "seconds when prompted."
            << std::endl;
  std::cout << "Press Enter to start..." << std::endl;
  std::cin.get();

  std::cout << "LOCK YOUR SCREEN NOW! Checking in 5 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  std::string successfulMethod = "";
  for (const auto &probe : probes) {
    std::cout << "Testing probe: " << probe.description << " [" << probe.name
              << "]... ";
    bool locked = detector->runProbe(probe.name);
    if (locked) {
      std::cout << "SUCCESS (Detected LOCKED)" << std::endl;
      successfulMethod = probe.name;
    } else {
      std::cout << "FAILED (Detected UNLOCKED)" << std::endl;
    }
  }

  if (!successfulMethod.empty()) {
    std::cout << "\nFound a working method: " << successfulMethod << std::endl;
    std::cout << "Do you want to save this as the preferred method? (y/n): ";
    std::string response;
    std::cin >> response;
    if (response == "y" || response == "Y") {
      settings->set("preferred_lock_detection_method", successfulMethod);
      settings->save();
      std::cout << "Preferred method saved: " << successfulMethod << std::endl;
    }
  } else {
    std::cout << "\nNone of the probes detected a locked screen." << std::endl;
    std::cout << "If your screen was locked, please report this issue."
              << std::endl;
  }

  std::cout << "\nCheck complete. Exiting." << std::endl;
}
