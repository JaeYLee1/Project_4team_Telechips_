#ifndef RPM_GAUGE_WIDGET_H
#define RPM_GAUGE_WIDGET_H

#include <QWidget>

class RpmGaugeWidget : public QWidget
{
public:
    explicit RpmGaugeWidget(QWidget *parent = nullptr);

    void setValues(bool running,
                   double currentRpm,
                   double targetRpm,
                   double duty,
                   int motorSpeedLevel);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_running;
    double m_currentRpm;
    double m_targetRpm;
    double m_duty;
    int m_motorSpeedLevel;
};

#endif