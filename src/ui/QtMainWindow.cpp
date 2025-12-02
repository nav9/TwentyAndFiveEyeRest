#include "QtMainWindow.hpp"
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QIcon>
#include <QPixmap>
#include <QSystemTrayIcon>

#include "../defaultTimer/DefaultTimer.hpp"

QtMainWindow::QtMainWindow(std::shared_ptr<DefaultTimer> timer, QWidget* parent)
: QMainWindow(parent), timer_(std::move(timer)), isPaused_(false)
{
    QWidget* w = new QWidget(this);
    auto* vbox = new QVBoxLayout(w);

    // top bar
    auto* topbar = new QHBoxLayout;
    settingsBtn_ = new QToolButton(this);
    settingsBtn_->setIcon(QIcon(":/resources/icons/app_icon.svg"));
    settingsBtn_->setToolTip("Settings");
    connect(settingsBtn_, &QToolButton::clicked, this, &QtMainWindow::onSettingsClicked);

    topbar->addStretch();
    topbar->addWidget(settingsBtn_);
    vbox->addLayout(topbar);

    // LED & status
    ledLabel_ = new QLabel("●", this);
    ledLabel_->setAlignment(Qt::AlignCenter);
    ledLabel_->setStyleSheet("font-size: 36px; color: green;");

    timeLabel_ = new QLabel("00 hr : 00 min", this);
    timeLabel_->setAlignment(Qt::AlignCenter);
    timeLabel_->setStyleSheet("font-size: 18px;");

    vbox->addWidget(ledLabel_);
    vbox->addWidget(timeLabel_);

    // controls
    auto* ctrls = new QHBoxLayout;
    playPauseBtn_ = new QToolButton(this);
    playPauseBtn_->setText("Pause");
    connect(playPauseBtn_, &QToolButton::clicked, this, &QtMainWindow::onPlayPauseClicked);

    resetBtn_ = new QPushButton("Reset", this);
    connect(resetBtn_, &QPushButton::clicked, this, &QtMainWindow::onResetClicked);

    ctrls->addWidget(playPauseBtn_);
    ctrls->addWidget(resetBtn_);
    vbox->addLayout(ctrls);

    setCentralWidget(w);
    setWindowTitle("TwentyAndFiveEyeRest");

    // System tray (simple)
    trayIcon_ = new QSystemTrayIcon(QIcon(":/resources/icons/app_icon.svg"), this);
    trayIcon_->show();

    // Timer to call poll() once per second
    QTimer* t = new QTimer(this);
    connect(t, &QTimer::timeout, this, &QtMainWindow::onTimerTick);
    t->start(1000);

    // Register backend callback for state changes
    if(timer_) {
        timer_->setStateChangeCallback([this](DefaultTimerStateId id){
            QMetaObject::invokeMethod(this, [this, id](){ updateUI(); }, Qt::QueuedConnection);
        });
    }
    updateUI();
}

QtMainWindow::~QtMainWindow() {
    // Qt parent-child will destruct child widgets
}

void QtMainWindow::onPlayPauseClicked() {
    if(!timer_) return;
    if(isPaused_) {
        timer_->play();
        playPauseBtn_->setText("Pause");
        isPaused_ = false;
    } else {
        timer_->pause();
        playPauseBtn_->setText("Play");
        isPaused_ = true;
    }
    updateUI();
}

void QtMainWindow::onResetClicked() {
    if(timer_) timer_->reset();
    updateUI();
}

void QtMainWindow::onSettingsClicked() {
    // For this scaffold, show a minimal modal or do nothing.
    // In complete implementation, open a Settings dialog with search.
}

void QtMainWindow::onTimerTick() {
    if(timer_) timer_->poll();
    updateUI();
}

void QtMainWindow::updateUI() {
    if(!timer_) return;
    auto [hrs, mins] = timer_->strainedDurationHrsMins();
    QString txt = QString("%1 hr : %2 min").arg(QString::number(hrs).rightJustified(2,'0'))
                                          .arg(QString::number(mins).rightJustified(2,'0'));
    timeLabel_->setText(txt);

    // LED color: green if below allowed, red if reached
    // Fetch allowed vs current using strainedDuration; using heuristic
    auto [cH, cM] = timer_->strainedDurationHrsMins();
    int totalSecs = cH * 3600 + cM * 60;
    // Compare with allowed (in seconds)
    // We can't access allowed directly here; naive color logic:
    if(totalSecs == 0) ledLabel_->setStyleSheet("font-size: 36px; color: gray;");
    else if(totalSecs < 20*60) ledLabel_->setStyleSheet("font-size: 36px; color: green;");
    else ledLabel_->setStyleSheet("font-size: 36px; color: red;");
}
