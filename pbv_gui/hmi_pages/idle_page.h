#ifndef IDLE_PAGE_H
#define IDLE_PAGE_H

#include <QWidget>

void setRelayState(bool relayOn);
void setFsmState(int fsmState, int authResult);

class QLabel;
class QPushButton;
class QLocalSocket;

class VehicleView : public QWidget
{
    Q_OBJECT

public:
    explicit VehicleView(QWidget *parent = nullptr);

    void setDockDetected(bool detected);
    void setRelayOn(bool relayOn);
    void setMotorRunning(bool running);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_dockDetected;
    bool m_relayOn;
    bool m_motorRunning;
};

class IdlePage : public QWidget
{
    Q_OBJECT

public:
    explicit IdlePage(QWidget *parent = nullptr);

    void setServiceConnected(bool connected);
    void setDockDetected(bool detected);
    void setAuthStatus(const QString &status);

    void setRelayState(bool relayOn);
    void setFsmState(int fsmState, int authResult);

    void setMotorStatus(int motorRunning,
                        int motorSpeedLevel,
                        double targetRpm,
                        double currentRpm,
                        double duty);

private slots:
    void connectModule();
    void driveStart();
    void driveStop();
    void selectLow();
    void selectMid();
    void selectHigh();

private:
    QPushButton *createNavButton(const QString &text);
    QLabel *createStatusChip(const QString &text);
    void sendCommand(const QString &command);
    void updateDriveProfile(int level);

private:
    QLocalSocket *m_socket;

    VehicleView *m_vehicleView;

    QLabel *m_timeLabel;
    QLabel *m_serviceLabel;
    QLabel *m_moduleLabel;
    QLabel *m_dockingLabel;
    QLabel *m_relayLabel;
    QLabel *m_authLabel;

    QLabel *m_driveStateLabel;
    QLabel *m_speedProfileLabel;
    QLabel *m_targetRpmLabel;
    QLabel *m_currentRpmLabel;
    QLabel *m_dutyLabel;

    QPushButton *m_connectButton;
    QPushButton *m_lowButton;
    QPushButton *m_midButton;
    QPushButton *m_highButton;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;

    bool m_dockDetected;
    bool m_relayOn;
};

#endif