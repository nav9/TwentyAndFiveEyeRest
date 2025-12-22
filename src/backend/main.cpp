#include "Constants.h"
#include "Core.h"
#include "Filesystem.h"
#include "Settings.h"
#include "SystemTimeProvider.h"
#include <CLI/CLI.hpp>
#include <csignal>
#include <iostream>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

std::unique_ptr<EyeRest::Core> coreService;

void signal_handler(int signal) {
  if (coreService) {
    spdlog::info("Signal {} received, stopping service...", signal);
    coreService->stop();
  }
}

int main(int argc, char **argv) {
  CLI::App app{"TwentyAndFive Eye Rest"};

  bool debug = false;
  std::string configPath = "settings.json";

  app.add_flag("-d,--debug", debug, "Enable debug logging");
  app.add_option("-c,--config", configPath, "Path to configuration file");

  // We can add overrides here if we want to parse specific settings from CLI
  // and injected into settings But for now keeping it simple or using a generic
  // approach would require more logic. The prompt says "When the user specifies
  // any parameters via the commandline, those corresponding values should be
  // stored in settings." This implies we should be able to set ANY setting? Or
  // specific ones? "The parameters of the rotating log...".

  int logFileSize = -1;
  app.add_option("--log-file-size", logFileSize,
                 "Max size of rotating log file in MB");

  int logFileCount = -1;
  app.add_option("--log-file-count", logFileCount,
                 "Number of rotating log files to keep");

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

  // Resolve absolute path for config?
  // For now use what is provided.
  auto settings = std::make_shared<EyeRest::Settings>(fs, configPath);

  // Initialize defaults first
  settings->initializeDefaults();

  // Load file (overwrites defaults with file values)
  settings->load();

  // Apply CLI overrides (overwrites file values)
  bool settingsChanged = false;
  if (logFileSize > 0) {
    settings->set("log_file_size", logFileSize);
    settingsChanged = true;
  }
  if (logFileCount > 0) {
    settings->set("log_file_count", logFileCount);
    settingsChanged = true;
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
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(console_sink);
    sinks.push_back(file_sink);

    auto logger =
        std::make_shared<spdlog::logger>("EyeRest", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);

    if (debug) {
      spdlog::set_level(spdlog::level::debug);
      spdlog::debug("Debug logging enabled");
    }

    spdlog::info("Logging initialized using Settings: Size={}MB, Count={}",
                 sizeMB, count);

  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize logging: " << e.what() << std::endl;
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
