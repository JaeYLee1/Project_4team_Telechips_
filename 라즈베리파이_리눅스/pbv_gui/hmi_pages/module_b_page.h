#ifndef MODULE_B_PAGE_H
#define MODULE_B_PAGE_H

#include <QWidget>

class QLabel;
class QPushButton;
class QProgressBar;
class QLocalSocket;
class QFrame;

class ModuleBPage : public QWidget
{
    Q_OBJECT

public:
    explicit ModuleBPage(QWidget *parent = nullptr);

    void setServiceConnected(bool connected);

    void setSystemStatus(int moduleType,
                         int authResult,
                         int detectState,
                         int relayState,
                         int fsmState);

    void setTemperatureStatus(int currentTempX10,
                              int targetTempX10);

    void setMotorStatus(int motorRunning,
                        int motorSpeedLevel,
                        double targetRpm,
                        double currentRpm,
                        double duty);
                        
    void setPowerProtectionStatus(int powerWarningCount,
                                  int powerFault,
                                  int totalPowerMw,
                                  int powerLimitMw);

private slots:
    void increaseTargetTemp();
    void decreaseTargetTemp();

    void driveStart();
    void driveStop();
    void selectLow();
    void selectMid();
    void selectHigh();
    void unmountModule();

private:
    QLabel *createStatusChip(const QString &text);
    QPushButton *createNavButton(const QString &text);
    QFrame *createPanel();

    void sendCommand(const QString &command);
    void sendTargetTempToBase();
    void updateDriveProfile(int level);
    void updateSpeedButtonStyle(int selectedLevel);
    void updateTargetTempView();

private:
    QLocalSocket *m_socket;

    QLabel *m_moduleLabel;
    QLabel *m_authLabel;
    QLabel *m_relayLabel;
    QLabel *m_dockLabel;
    QLabel *m_systemLabel;

    QLabel *m_currentTempLabel;
    QLabel *m_targetTempLabel;
    QLabel *m_peltierLabel;
    QLabel *m_tempStateLabel;

    QLabel *m_driveStateLabel;
    QLabel *m_speedProfileLabel;
    QLabel *m_targetRpmLabel;
    QLabel *m_currentRpmLabel;
    QLabel *m_dutyLabel;

    QProgressBar *m_peltierBar;

    QPushButton *m_tempDownButton;
    QPushButton *m_tempUpButton;

    QPushButton *m_lowButton;
    QPushButton *m_midButton;
    QPushButton *m_highButton;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_unmountButton;

    QFrame *m_powerProtectionCard;
    QLabel *m_powerStateLabel;
    QLabel *m_powerTotalLabel;
    QLabel *m_powerLimitLabel;
    QLabel *m_powerWarningLabel;

    bool m_moduleActive;
    bool m_targetInitialized;
    int m_targetTempC;
};

#endif