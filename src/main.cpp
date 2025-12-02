#include <QApplication>
#include "ui/QtMainWindow.hpp"
#include "common/TimeProvider.hpp"
#include "defaultTimer/DefaultTimer.hpp"
#include <memory>
#include <iostream>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    auto timeProvider = std::make_shared<SystemTimeProvider>();
    auto timer = std::make_shared<DefaultTimer>(timeProvider, "./timeFiles-TwentyAndFiveEyeRest");

    QtMainWindow mainWin(timer);
    mainWin.show();

    int res = app.exec();

    // Ensure orderly cleanup
    timer->stop();

    return res;
}
