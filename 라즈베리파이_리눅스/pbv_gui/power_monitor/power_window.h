#ifndef POWER_WINDOW_H
#define POWER_WINDOW_H

#include <QMainWindow>

class QLabel;
class QString;
class QWidget;
class TelemetryClient;
class VehiclePowerChartWidget;
class ModuleBProtectionChartWidget;

class PowerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PowerWindow(QWidget *parent = nullptr);

private slots:
    void onPowerTelemetryReceived(int basePowerMw,
                                  int moduleBPowerMw,
                                  int coolingPowerMw,
                                  int motorPowerMw,
                                  int totalPowerMw,
                                  int powerWarningCount,
                                  int powerFault,
                                  int powerLimitMw);

    void onConnectionChanged(bool connected);

private:
    QWidget *createCard(const QString &title,
                        QLabel **valueLabel);

private:
    TelemetryClient *m_telemetryClient;

    QLabel *m_connectionLabel;

    QLabel *m_basePowerLabel;
    QLabel *m_motorPowerLabel;
    QLabel *m_moduleBPowerLabel;
    QLabel *m_coolingPowerLabel;
    QLabel *m_statusLabel;

    VehiclePowerChartWidget *m_vehicleChart;
    ModuleBProtectionChartWidget *m_moduleBProtectionChart;
};

#endif