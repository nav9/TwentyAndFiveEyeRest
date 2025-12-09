#include "MainWindow.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>
#include <QSettings>

namespace iRest {

    MainWindow::MainWindow(TimerEngine& engine, QWidget* parent)
        : QMainWindow(parent), m_engine(engine)
    {
        setWindowTitle("iRest");
        resize(300, 200);

        setupUi();
        createTrayIcon();

        // Load settings from QSettings or defaults
        loadSettings();

        // UI updates from engine
        m_engine.setStateChangeCallback([this](const std::string& s){
            // Must run on main thread
            QMetaObject::invokeMethod(this, [this, s](){
                onStateChanged(s);
            });
        });

        // Setup ticking
        m_tickTimer = new QTimer(this);
        connect(m_tickTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);
        m_tickTimer->start(100); // 100ms resolution
        
        m_engine.start(); // Start engine
        updateUi(0); // Initial
    }

    MainWindow::~MainWindow() {
        m_engine.stop();
    }

    void MainWindow::setupUi() {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QVBoxLayout* layout = new QVBoxLayout(centralWidget);

        m_statusLabel = new QLabel("Status: Ready", this);
        m_statusLabel->setAlignment(Qt::AlignCenter);
        QFont f = m_statusLabel->font();
        f.setPointSize(12);
        f.setBold(true);
        m_statusLabel->setFont(f);
        layout->addWidget(m_statusLabel);

        m_timeLabel = new QLabel("00:00", this);
        m_timeLabel->setAlignment(Qt::AlignCenter);
        f.setPointSize(24);
        m_timeLabel->setFont(f);
        layout->addWidget(m_timeLabel);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        m_pauseButton = new QPushButton("Pause", this);
        connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::togglePause);
        btnLayout->addWidget(m_pauseButton);

        m_resetButton = new QPushButton("Reset", this);
        connect(m_resetButton, &QPushButton::clicked, this, &MainWindow::resetTimer);
        btnLayout->addWidget(m_resetButton);
        layout->addLayout(btnLayout);

        m_settingsButton = new QPushButton("Settings", this);
        connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
        layout->addWidget(m_settingsButton);
    }

    void MainWindow::createTrayIcon() {
        m_trayMenu = new QMenu(this);
        
        QAction* viewAction = new QAction("Open", this);
        connect(viewAction, &QAction::triggered, this, &MainWindow::showNormal);
        m_trayMenu->addAction(viewAction);

        QAction* quitAction = new QAction("Exit", this);
        connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
        m_trayMenu->addAction(quitAction);

        m_trayIcon = new QSystemTrayIcon(this);
        // We need an icon. For now, let's use a standard system icon if possible or empty.
        // In a real app we'd load a resource.
        m_trayIcon->setIcon(QIcon::fromTheme("appointment-new")); 
        m_trayIcon->setContextMenu(m_trayMenu);
        
        connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason){
            if(reason == QSystemTrayIcon::Trigger) {
                if(this->isVisible()) this->hide();
                else {
                    this->showNormal();
                    this->activateWindow();
                }
            }
        });

        m_trayIcon->show();
    }

    void MainWindow::closeEvent(QCloseEvent* event) {
        if (m_trayIcon->isVisible()) {
            QMessageBox::information(this, "iRest", "The program will keep running in the system tray. Use 'Exit' from the tray menu to close it fully.");
            hide();
            event->ignore();
        } else {
            event->accept();
        }
    }

    void MainWindow::onTimerTick() {
        m_engine.update();
        
        // Calculate display time based on state
        // If Strained: show accumulated
        // If Rest: show remaining rest? or just accumulated rest?
        // Let's use getStrainedDuration for Strained
        
        double val = m_engine.getStrainedDuration();
        // If in rest, we might want to show how much rest we've taken or how much left.
        // The engine doesn't explicitly give "time remaining" for rest, 
        // but we know minRestDuration.
        
        // Simple logic:
        std::string state = m_engine.getCurrentStateName();
        if (state == "Strained") {
             // Show strain vs max
             double max = m_engine.getMaxStrainDuration();
             double remain = max - val;
             updateUi(remain > 0 ? remain : 0);
        } else {
             // We don't track "rest duration" in public API of engine easily?
             // Actually, ITimeSource and engine don't expose accumulated rest publicly?
             // Wait, the engine has `update()` but state tracking is internal to states.
             
             // For now, let's just show strain duration.
             // If we are in Rest, strain should be decreasing.
             updateUi(val);
        }
    }

    void MainWindow::onStateChanged(const std::string& newState) {
        m_statusLabel->setText(QString::fromStdString(newState));
        
        if (newState == "Strained") {
            m_pauseButton->setText("Pause");
            m_trayIcon->setToolTip("iRest: Strained");
        } else if (newState == "Rest") { // Assuming RestState name
             m_trayIcon->showMessage("Take a Break", "Eye rest required!", QSystemTrayIcon::Information, 3000);
             m_trayIcon->setToolTip("iRest: Resting");
        }
    }
    
    void MainWindow::updateUi(double timeVal) {
        m_timeLabel->setText(formatTime(timeVal));
    }

    QString MainWindow::formatTime(double seconds) {
        int s = static_cast<int>(seconds);
        int m = s / 60;
        s = s % 60;
        return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    }

    void MainWindow::togglePause() {
        // Since we don't have isPaused, we check state name
        if (m_engine.getCurrentStateName() == "Paused") { 
            m_engine.resume();
            m_pauseButton->setText("Pause");
        } else {
            m_engine.pause();
            m_pauseButton->setText("Resume");
        }
    }

    void MainWindow::resetTimer() {
        m_engine.reset();
        // Reset usually goes to Strained or initial state
    }

    void MainWindow::openSettings() {
        SettingsDialog dlg(this);
        
        TimerConfig current; 
        current.maxStrainDuration = m_engine.getMaxStrainDuration();
        current.minRestDuration = m_engine.getMinRestDuration();
        // logInterval isn't exposed via getter in TimerEngine? 
        // Let's assume we can get it or just look at what's in settings.
        // Actually TimerEngine doesn't have getLogInterval.
        // We'll trust QSettings or defaults.
        
        QSettings s("iRest", "Config");
        current.logInterval = s.value("logInterval", 60).toInt();
        
        dlg.setConfig(current);
        
        if (dlg.exec() == QDialog::Accepted) {
            TimerConfig newCfg = dlg.getConfig();
            
            // Re-init engine or set values?
            // Engine doesn't have setters for config!
            // We need to restart the app or add setters to Engine.
            // For now, let's save to QSettings and tell user to restart, 
            // OR we can rebuild the engine (harder in main).
            // Better: Add setters to TimerEngine? 
            // Wait, looking at TimerEngine.h:
            // It has setStrainedDuration but no setters for MaxStrain/MinRest in the class definition I saw.
            
            // Checking TimerEngine.h content from earlier:
            // It has getters but no setters for limits.
            
            // Workaround: Save to QSettings. The engine is init'd from QSettings in main.
            // So we inform user.
            
            saveSettingsInQSettings(newCfg);
            QMessageBox::information(this, "Settings", "Settings saved. Please restart application to apply changes completely.");
        }
    }
    
    void MainWindow::loadSettings() {
         // This is handled in main.cpp for Engine init, 
         // but if we had setters we'd use them here.
    }
    
    void MainWindow::saveSettingsInQSettings(const TimerConfig& cfg) {
        QSettings s("iRest", "Config");
        s.setValue("maxStrain", cfg.maxStrainDuration);
        s.setValue("minRest", cfg.minRestDuration);
        s.setValue("logInterval", cfg.logInterval);
    }
}
