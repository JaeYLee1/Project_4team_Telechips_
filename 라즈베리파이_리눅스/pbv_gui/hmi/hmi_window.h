#ifndef HMI_WINDOW_H
#define HMI_WINDOW_H

#include <QMainWindow>

class QStackedWidget;
class QLocalSocket;
class QDialog;
class QString;

class TelemetryClient;
class IdlePage;
class ModuleAPage;
class ModuleBPage;

class HmiWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HmiWindow(QWidget *parent = nullptr);

private slots:
    void onTelemetryReceived(int moduleType,
                             int authResult,
                             int weight,
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

    void onAiWarningReceived(int level,
                             int sleepFlag);

    void onPowerProtectionReceived(int powerWarningCount,
                                   int powerFault,
                                   int totalPowerMw,
                                   int powerLimitMw);

private:
    void sendCommand(const QString &command);

    void showAiWarningDialog(int level,
                             int sleepFlag);

    void closeAiWarningDialog();

    void showPowerCutoffDialog(int totalPowerMw,
                               int powerLimitMw);

    /*
     * Cooling INA226 저전류 기반 펠티어 단선/구동 이상 경고창.
     * STM32에서 이미 부저/LED를 켜므로,
     * GUI는 팝업 표시 후 ACK 시 K만 전송한다.
     */
    void showCoolingFaultDialog();

private:
    QStackedWidget *m_stack;

    TelemetryClient *m_telemetryClient;

    IdlePage *m_idlePage;
    ModuleAPage *m_moduleAPage;
    ModuleBPage *m_moduleBPage;

    QLocalSocket *m_commandSocket;

    QDialog *m_aiWarningBox;
    QDialog *m_powerWarningBox;

    int m_currentAiWarningLevel;

    bool m_aiWarningEnabled;
    bool m_powerCutoffDialogShown;
    bool m_coolingFaultDialogShown;
    bool m_aiDialogSuppressed;
};

#endif