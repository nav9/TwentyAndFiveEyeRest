#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include "../core/TimerEngine.h"

namespace iRest {

    class SettingsDialog : public QDialog {
        Q_OBJECT
    public:
        explicit SettingsDialog(QWidget* parent = nullptr);
        
        TimerConfig getConfig() const;
        void setConfig(const TimerConfig& config);

    private:
        void setupUi();
        void filterSettings(const QString& text);

        QLineEdit* m_searchBar;
        QSpinBox* m_maxStrainBox;
        QSpinBox* m_minRestBox;
        QSpinBox* m_logIntervalBox;
        
        // Helper to hide/show rows based on search
        QList<QWidget*> m_rows; 
        QList<QString> m_rowKeywords;
    };
}
