#ifndef HMI_WINDOW_H
#define HMI_WINDOW_H

#include <QMainWindow>

class QStackedWidget;
class TelemetryClient;
class IdlePage;
class ModuleAPage;

class HmiWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HmiWindow(QWidget *parent = nullptr);

private slots:
    void onTelemetryReceived(int moduleType,
                             int authResult,
                             int pressure,
                             int currentTempX10,
                             int targetTempX10,
                             int peltierDuty,
                             int motorRunning,
                             int motorSpeedLevel,
                             int targetRpmX10,
                             int currentRpmX10,
                             int motorDutyX10,
                             int detectState,
                             int relayState,
                             int fsmState);

    void onConnectionChanged(bool connected);

private:
    QStackedWidget *m_stack;
    TelemetryClient *m_telemetryClient;
    IdlePage *m_idlePage;
    ModuleAPage *m_moduleAPage;
};

#endif