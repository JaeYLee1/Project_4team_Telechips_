#ifndef MOTORWINDOW_H
#define MOTORWINDOW_H

#include <QMainWindow>
#include <QString>

class QLabel;
class QPushButton;
class MotorClient;

class MotorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MotorWindow(QWidget *parent = nullptr);

private slots:
    void startMotor();
    void stopMotor();

    void selectSlow();
    void selectNormal();
    void selectFast();

    void onConnectionChanged(bool connected);
    void onServiceLineReceived(const QString &line);
    void onCommandFailed(const QString &reason);

private:
    enum class SpeedMode
    {
        Slow,
        Normal,
        Fast
    };

    void setSpeedMode(SpeedMode mode);
    void updateSpeedButtons();
    void updateDriveState(const QString &state);
    void sendSelectedSpeed();

    QString selectedCommand() const;
    QString selectedSpeedText() const;

    MotorClient *m_motorClient;

    QLabel *m_connectionLabel;
    QLabel *m_driveStateLabel;
    QLabel *m_speedLabel;
    QLabel *m_responseLabel;

    QPushButton *m_slowButton;
    QPushButton *m_normalButton;
    QPushButton *m_fastButton;

    QPushButton *m_startButton;
    QPushButton *m_stopButton;

    SpeedMode m_selectedSpeed;
    bool m_motorRunning;
};

#endif