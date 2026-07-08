#ifndef MODULE_A_PAGE_H
#define MODULE_A_PAGE_H

#include <QWidget>

class QLabel;
class QPushButton;
class QProgressBar;
class QLocalSocket;
class QFrame;

class ModuleAPage : public QWidget
{
    Q_OBJECT

public:
    explicit ModuleAPage(QWidget *parent = nullptr);

    void setServiceConnected(bool connected);

    void setSystemStatus(int moduleType,
                         int authResult,
                         int detectState,
                         int relayState,
                         int fsmState);

    void setCargoStatus(int weightG);

    void setMotorStatus(int motorRunning,
                        int motorSpeedLevel,
                        double targetRpm,
                        double currentRpm,
                        double duty);

private slots:
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
    void updateDriveProfile(int level);
    void updateSpeedButtonStyle(int selectedLevel);

private:
    QLocalSocket *m_socket;

    QLabel *m_moduleLabel;
    QLabel *m_authLabel;
    QLabel *m_relayLabel;
    QLabel *m_dockLabel;
    QLabel *m_systemLabel;

    QLabel *m_cargoPercentLabel;
    QLabel *m_cargoStateLabel;
    QLabel *m_weightLabel;

    QLabel *m_driveStateLabel;
    QLabel *m_speedProfileLabel;
    QLabel *m_targetRpmLabel;
    QLabel *m_currentRpmLabel;
    QLabel *m_dutyLabel;

    QProgressBar *m_cargoBar;

    QPushButton *m_lowButton;
    QPushButton *m_midButton;
    QPushButton *m_highButton;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_unmountButton;
};

#endif