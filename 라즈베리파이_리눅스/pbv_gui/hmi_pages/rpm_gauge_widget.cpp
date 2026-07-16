#include "rpm_gauge_widget.h"

#include <algorithm>

#include <QPainter>
#include <QPen>
#include <QFont>
#include <QRectF>

RpmGaugeWidget::RpmGaugeWidget(QWidget *parent)
    : QWidget(parent),
      m_running(false),
      m_currentRpm(0.0),
      m_targetRpm(0.0),
      m_duty(0.0),
      m_motorSpeedLevel(0)
{
    setMinimumHeight(170);
}

void RpmGaugeWidget::setValues(bool running,
                               double currentRpm,
                               double targetRpm,
                               double duty,
                               int motorSpeedLevel)
{
    m_running = running;
    m_currentRpm = currentRpm;
    m_targetRpm = targetRpm;
    m_duty = duty;
    m_motorSpeedLevel = motorSpeedLevel;

    update();
}

void RpmGaugeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int side = std::min(width(), height() + 40);
    const int gaugeSize = std::min(side - 20, 190);

    QRectF gaugeRect((width() - gaugeSize) / 2.0,
                     8.0,
                     gaugeSize,
                     gaugeSize);

    gaugeRect.adjust(18, 18, -18, -18);

    double maxRpm = m_targetRpm;

    if (maxRpm < 100.0)
    {
        maxRpm = 300.0;
    }

    double ratio = m_currentRpm / maxRpm;

    if (ratio < 0.0)
    {
        ratio = 0.0;
    }

    if (ratio > 1.0)
    {
        ratio = 1.0;
    }

    const int startAngle = 225 * 16;
    const int totalSpan = -270 * 16;
    const int valueSpan = static_cast<int>(totalSpan * ratio);

    // Background arc
    QPen bgPen(QColor("#1B3347"), 15);
    bgPen.setCapStyle(Qt::RoundCap);
    p.setPen(bgPen);
    p.drawArc(gaugeRect, startAngle, totalSpan);

    // Value arc
    QColor valueColor = m_running ? QColor("#28D7FF") : QColor("#52616F");

    QPen valuePen(valueColor, 15);
    valuePen.setCapStyle(Qt::RoundCap);
    p.setPen(valuePen);
    p.drawArc(gaugeRect, startAngle, valueSpan);

    // Inner glow circle
    p.setPen(QPen(QColor("#243B50"), 1));
    p.setBrush(QColor("#0A1521"));
    p.drawEllipse(gaugeRect.adjusted(26, 26, -26, -26));

    // RPM value
    QFont rpmFont;
    rpmFont.setPointSize(28);
    rpmFont.setBold(true);
    p.setFont(rpmFont);
    p.setPen(QColor("#EAF6FF"));

    QRectF valueTextRect(gaugeRect.left(),
                         gaugeRect.top() + gaugeRect.height() * 0.32,
                         gaugeRect.width(),
                         42);

    p.drawText(valueTextRect,
               Qt::AlignCenter,
               QString("%1").arg(m_currentRpm, 0, 'f', 1));

    // RPM unit
    QFont unitFont;
    unitFont.setPointSize(11);
    unitFont.setBold(true);
    p.setFont(unitFont);
    p.setPen(QColor("#8EA5B8"));

    QRectF unitTextRect(gaugeRect.left(),
                        gaugeRect.top() + gaugeRect.height() * 0.56,
                        gaugeRect.width(),
                        22);

    p.drawText(unitTextRect,
               Qt::AlignCenter,
               "RPM");

    // Running state
    QFont stateFont;
    stateFont.setPointSize(10);
    stateFont.setBold(true);
    p.setFont(stateFont);

    QColor stateColor = m_running ? QColor("#37D996") : QColor("#8EA5B8");
    p.setPen(stateColor);

    QRectF stateRect(gaugeRect.left(),
                     gaugeRect.bottom() - 14,
                     gaugeRect.width(),
                     24);

    p.drawText(stateRect,
               Qt::AlignCenter,
               m_running ? "RUNNING" : "STOPPED");

    // Bottom mini info
    QString modeText;

    switch (m_motorSpeedLevel)
    {
        case 1:
            modeText = "LOW";
            break;

        case 2:
            modeText = "MID";
            break;

        case 3:
            modeText = "HIGH";
            break;

        default:
            modeText = "--";
            break;
    }

    QFont infoFont;
    infoFont.setPointSize(9);
    infoFont.setBold(true);
    p.setFont(infoFont);
    p.setPen(QColor("#AAB8C7"));

    QRectF infoRect(0,
                    height() - 28,
                    width(),
                    22);

    p.drawText(infoRect,
               Qt::AlignCenter,
               QString("TARGET %1 RPM   |   PWM %2%   |   MODE %3")
                   .arg(m_targetRpm, 0, 'f', 1)
                   .arg(m_duty, 0, 'f', 1)
                   .arg(modeText));
}