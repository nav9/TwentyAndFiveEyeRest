#pragma once
#include <QMainWindow>
#include <memory>

class QLabel;
class QPushButton;
class QToolButton;
class QSystemTrayIcon;
class DefaultTimer;

class QtMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit QtMainWindow(std::shared_ptr<DefaultTimer> timer, QWidget* parent = nullptr);
    ~QtMainWindow() override;

private slots:
    void onPlayPauseClicked();
    void onResetClicked();
    void onSettingsClicked();
    void onTimerTick(); // called by QTimer to poll backend

private:
    void updateUI();

    std::shared_ptr<DefaultTimer> timer_;
    QLabel* ledLabel_;
    QLabel* timeLabel_;
    QToolButton* playPauseBtn_;
    QPushButton* resetBtn_;
    QToolButton* settingsBtn_;
    QSystemTrayIcon* trayIcon_;
    bool isPaused_;
};
