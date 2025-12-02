#pragma once
#include <QObject>

class SystemTrayController : public QObject {
    Q_OBJECT
public:
    explicit SystemTrayController(QObject* parent = nullptr);
    ~SystemTrayController() override;
};
