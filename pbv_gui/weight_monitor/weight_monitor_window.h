#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QElapsedTimer>
#include <QMainWindow>

class QLabel;
class QChart;
class QChartView;
class QLineSeries;
class QValueAxis;
class TelemetryClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

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
                            int motorDutyX10);

    void onConnectionChanged(bool connected);
    void clearGraph();

private:
    void updatePressureAxis();

    TelemetryClient *m_telemetryClient;

    QLabel *m_connectionLabel;
    QLabel *m_moduleLabel;
    QLabel *m_authLabel;
    QLabel *m_pressureValueLabel;
    QLabel *m_minValueLabel;
    QLabel *m_maxValueLabel;

    QChart *m_chart;
    QChartView *m_chartView;
    QLineSeries *m_pressureSeries;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;

    QElapsedTimer m_elapsedTimer;

    static constexpr double DISPLAY_WINDOW_SEC = 30.0;
};

#endif