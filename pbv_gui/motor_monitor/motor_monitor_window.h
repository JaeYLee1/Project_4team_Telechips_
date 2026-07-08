#ifndef MOTOR_MONITOR_WINDOW_H
#define MOTOR_MONITOR_WINDOW_H

#include <QElapsedTimer>
#include <QMainWindow>

class QLabel;
class QPushButton;
class QChart;
class QChartView;
class QLineSeries;
class QValueAxis;
class TelemetryClient;

class MotorMonitorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MotorMonitorWindow(QWidget *parent = nullptr);

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
    void resetRecording();

private:
    void updateMotorState(int motorRunning, int motorSpeedLevel);
    void updateRpmAxis(double targetRpm, double currentRpm);
    void removeOldPoints(QLineSeries *series, double elapsedSeconds);

    TelemetryClient *m_telemetryClient;

    QLabel *m_connectionLabel;
    QLabel *m_motorStateLabel;
    QLabel *m_speedProfileLabel;

    QLabel *m_targetRpmLabel;
    QLabel *m_currentRpmLabel;
    QLabel *m_pwmDutyLabel;

    QChart *m_rpmChart;
    QChartView *m_rpmChartView;

    QLineSeries *m_targetRpmSeries;
    QLineSeries *m_currentRpmSeries;

    QValueAxis *m_rpmAxisX;
    QValueAxis *m_rpmAxisY;

    QChart *m_dutyChart;
    QChartView *m_dutyChartView;

    QLineSeries *m_dutySeries;

    QValueAxis *m_dutyAxisX;
    QValueAxis *m_dutyAxisY;

    QElapsedTimer m_elapsedTimer;

    static constexpr double DISPLAY_WINDOW_SEC = 30.0;
};

#endif