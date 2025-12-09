#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>

namespace iRest {

    SettingsDialog::SettingsDialog(QWidget* parent) 
        : QDialog(parent) 
    {
        setWindowTitle("iRest Settings");
        setupUi();
        resize(400, 300);
    }

    void SettingsDialog::setupUi() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Search Bar
        m_searchBar = new QLineEdit(this);
        m_searchBar->setPlaceholderText("Search settings...");
        mainLayout->addWidget(m_searchBar);

        // Form Layout for settings
        QFormLayout* formLayout = new QFormLayout();
        
        m_maxStrainBox = new QSpinBox(this);
        m_maxStrainBox->setRange(1, 3600); // 1 sec to 1 hour
        m_maxStrainBox->setSuffix(" sec");
        
        m_minRestBox = new QSpinBox(this);
        m_minRestBox->setRange(1, 600); // 1 sec to 10 min
        m_minRestBox->setSuffix(" sec");

        m_logIntervalBox = new QSpinBox(this);
        m_logIntervalBox->setRange(1, 3600);
        m_logIntervalBox->setSuffix(" sec");

        // Add rows helpers
        auto addRow = [&](const QString& label, QWidget* widget) {
            QLabel* lbl = new QLabel(label, this);
            formLayout->addRow(lbl, widget);
            m_rows.append(lbl);
            m_rows.append(widget);
            m_rowKeywords.append(label.toLower());
        };

        addRow("Maximum Strain Duration:", m_maxStrainBox);
        addRow("Minimum Rest Duration:", m_minRestBox);
        addRow("Log Interval:", m_logIntervalBox);

        mainLayout->addLayout(formLayout);

        // Buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        mainLayout->addWidget(buttonBox);

        // Search functionality
        connect(m_searchBar, &QLineEdit::textChanged, this, &SettingsDialog::filterSettings);
    }

    void SettingsDialog::filterSettings(const QString& text) {
        QString lower = text.toLower();
        // Assuming label is at index i, widget at i+1
        for (int i = 0; i < m_rowKeywords.size(); ++i) {
            bool visible = m_rowKeywords[i].contains(lower);
            // Label
            m_rows[i * 2]->setVisible(visible);
            // Widget
            m_rows[i * 2 + 1]->setVisible(visible);
        }
    }

    TimerConfig SettingsDialog::getConfig() const {
        TimerConfig cfg;
        cfg.maxStrainDuration = m_maxStrainBox->value();
        cfg.minRestDuration = m_minRestBox->value();
        cfg.logInterval = m_logIntervalBox->value();
        return cfg;
    }

    void SettingsDialog::setConfig(const TimerConfig& config) {
        m_maxStrainBox->setValue(static_cast<int>(config.maxStrainDuration));
        m_minRestBox->setValue(static_cast<int>(config.minRestDuration));
        m_logIntervalBox->setValue(config.logInterval);
    }
}
