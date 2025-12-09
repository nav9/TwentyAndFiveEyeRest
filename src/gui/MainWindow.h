#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <memory>
#include "../core/TimerEngine.h"

namespace iRest {

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(TimerEngine& engine, QWidget* parent = nullptr);
        ~MainWindow();

    protected:
        void closeEvent(QCloseEvent* event) override;

    private slots:
        void onTimerTick();

        void openSettings();
        void togglePause();
        void resetTimer();
        void updateUi(double remainingTime);
        void onStateChanged(const std::string& newState);

    private:
        void createTrayIcon();
        void setupUi();
        void loadSettings();
        void saveSettingsInQSettings(const TimerConfig& cfg);

        TimerEngine& m_engine;
        QTimer* m_tickTimer;
        
        QSystemTrayIcon* m_trayIcon;
        QMenu* m_trayMenu;
        
        QLabel* m_statusLabel;
        QLabel* m_timeLabel;
        QPushButton* m_pauseButton;
        QPushButton* m_resetButton;
        QPushButton* m_settingsButton;

        // UI Helpers
        QString formatTime(double seconds);
    };
}
