#ifndef TEMPERATUREWINDOW_H
#define TEMPERATUREWINDOW_H

#include <QElapsedTimer>
#include <QMainWindow>

class QLabel;
class QChart;
class QChartView;
class QLineSeries;
class QValueAxis;
class TelemetryClient;

class TemperatureWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TemperatureWindow(QWidget *parent = nullptr);

private slots:
    void onTelemetryReceived(int moduleType,
                             int authResult,
                             int pressure,
                             int currentTempX10,
                             int targetTempX10,
                             int peltierDuty);

    void onConnectionChanged(bool connected);
    void resetRecording();

private:
    TelemetryClient *m_telemetryClient;

    QLabel *m_connectionLabel;
    QLabel *m_moduleLabel;
    QLabel *m_authLabel;

    QLabel *m_currentTempLabel;
    QLabel *m_targetTempLabel;

    QChart *m_chart;
    QChartView *m_chartView;

    QLineSeries *m_currentTempSeries;
    QLineSeries *m_targetTempSeries;
    QLineSeries *m_restartThresholdSeries;

    QValueAxis *m_axisX;
    QValueAxis *m_axisY;

    QElapsedTimer m_elapsedTimer;

    static constexpr double DISPLAY_WINDOW_SEC = 30.0;
    static constexpr double TEMP_Y_MIN = 0.0;
    static constexpr double TEMP_Y_MAX = 30.0;
};

#endif