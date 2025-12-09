#include <QApplication>
#include <QSettings>
#include <memory>
#include "MainWindow.h"
#include "../common/RealTimeSource.h"
#include "../platform/OsUtils_Linux.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false); // Important for tray app

    // Setup Config
    QSettings settings("iRest", "Config");
    iRest::TimerConfig config;
    config.maxStrainDuration = settings.value("maxStrain", 1200.0).toDouble(); // 20 mins default
    config.minRestDuration = settings.value("minRest", 20.0).toDouble();       // 20 sec default
    config.logInterval = settings.value("logInterval", 60).toInt();

    // Dependencies
    iRest::RealTimeSource timeSource;
    // Platform-specific Utils (Linux for now, could use factory)
    auto osUtils = std::make_unique<iRest::OsUtils_Linux>();

    // Engine
    iRest::TimerEngine engine(timeSource, std::move(osUtils), config);

    // Main Window
    iRest::MainWindow window(engine);
    // Start minimsed ? Or show status?
    // Let's show it initially so user knows it started.
    window.show();

    return app.exec();
}
