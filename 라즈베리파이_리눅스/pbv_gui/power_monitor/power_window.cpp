#include "power_window.h"

#include "telemetry_client.h"

#include <QColor>
#include <QElapsedTimer>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <QtMath>

static QString makeStatusStyle(const QString &background,
                               const QString &color,
                               const QString &border)
{
    return
        "background: " + background + ";"
        "color: " + color + ";"
        "border: 1px solid " + border + ";"
        "border-radius: 14px;"
        "padding: 10px 18px;"
        "font-size: 20px;"
        "font-weight: 900;"
        "letter-spacing: 1px;";
}

struct VehiclePowerSample
{
    double timeSec;
    double basePowerW;
    double motorPowerW;
};

struct ModuleBPowerSample
{
    double timeSec;
    double moduleBPowerW;
    double coolingPowerW;
    double totalPowerW;
};

class VehiclePowerChartWidget : public QWidget
{
public:
    explicit VehiclePowerChartWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(330);
        m_elapsedTimer.start();
    }

    void addSample(double basePowerW,
                   double motorPowerW)
    {
        const double nowSec =
            m_elapsedTimer.elapsed() / 1000.0;

        VehiclePowerSample sample;
        sample.timeSec = nowSec;
        sample.basePowerW = basePowerW;
        sample.motorPowerW = motorPowerW;

        m_samples.append(sample);

        while ((!m_samples.isEmpty()) &&
               ((nowSec - m_samples.first().timeSec) > 30.0))
        {
            m_samples.removeFirst();
        }

        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.fillRect(rect(), QColor("#050B12"));

        QRect outer = rect().adjusted(10, 10, -10, -10);

        painter.setPen(QPen(QColor("#DDE3EA"), 1));
        painter.setBrush(QColor("#FFFFFF"));
        painter.drawRoundedRect(outer, 14, 14);

        painter.setPen(QColor("#17212B"));

        QFont titleFont = painter.font();
        titleFont.setPointSize(13);
        titleFont.setBold(true);
        painter.setFont(titleFont);

        painter.drawText(outer.adjusted(18, 12, -18, -12),
                         Qt::AlignTop | Qt::AlignHCenter,
                         "VEHICLE POWER TREND");

        QFont legendFont = painter.font();
        legendFont.setPointSize(9);
        legendFont.setBold(true);
        painter.setFont(legendFont);

        const int legendY = outer.top() + 48;
        const int legendStartX = outer.center().x() - 130;

        drawLegendItem(painter,
                       legendStartX,
                       legendY,
                       QColor("#45D7FF"),
                       "Base MCU");

        drawLegendItem(painter,
                       legendStartX + 130,
                       legendY,
                       QColor("#FFB84D"),
                       "Motor");

        QRect plot = outer.adjusted(86, 86, -28, -64);

        painter.setPen(QPen(QColor("#DDE3EA"), 1));
        painter.setBrush(QColor("#FFFFFF"));
        painter.drawRect(plot);

        double maxValue = 1.0;

        updateMaxValue(maxValue);

        maxValue = qMax(1.0, maxValue * 1.25);

        const double nowSec =
            m_elapsedTimer.elapsed() / 1000.0;

        const double xMax = qMax(30.0, nowSec);
        const double xMin = qMax(0.0, xMax - 30.0);

        drawGridAndAxis(painter, outer, plot, maxValue, xMin, xMax);

        drawSeries(painter,
                   plot,
                   maxValue,
                   xMin,
                   xMax,
                   QColor("#45D7FF"),
                   2,
                   true);

        drawSeries(painter,
                   plot,
                   maxValue,
                   xMin,
                   xMax,
                   QColor("#FFB84D"),
                   2,
                   false);
    }

private:
    void drawLegendItem(QPainter &painter,
                        int x,
                        int y,
                        const QColor &color,
                        const QString &text)
    {
        painter.setBrush(color);
        painter.setPen(color);
        painter.drawRect(x, y - 8, 10, 10);

        painter.setPen(QColor("#52606D"));
        painter.drawText(x + 16,
                         y + 2,
                         text);
    }

    void updateMaxValue(double &maxValue)
    {
        for (const VehiclePowerSample &sample : m_samples)
        {
            if (sample.basePowerW > maxValue)
            {
                maxValue = sample.basePowerW;
            }

            if (sample.motorPowerW > maxValue)
            {
                maxValue = sample.motorPowerW;
            }
        }
    }

    void drawGridAndAxis(QPainter &painter,
                         const QRect &outer,
                         const QRect &plot,
                         double maxValue,
                         double xMin,
                         double xMax)
    {
        QFont axisFont = painter.font();
        axisFont.setPointSize(10);
        axisFont.setBold(false);
        painter.setFont(axisFont);

        for (int i = 0; i <= 4; i++)
        {
            const int y = plot.top() + (plot.height() * i / 4);
            const double labelValue =
                maxValue * (4 - i) / 4.0;

            painter.setPen(QPen(QColor("#E1E7EE"), 1));
            painter.drawLine(plot.left(), y, plot.right(), y);

            painter.setPen(QColor("#6B7683"));

            QRect yTickRect(plot.left() - 66,
                            y - 10,
                            54,
                            20);

            painter.drawText(yTickRect,
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(labelValue, 'f', 1) + " W");
        }

        const double xRange = qMax(1.0, xMax - xMin);

        for (int i = 0; i <= 6; i++)
        {
            const int x = plot.left() + (plot.width() * i / 6);
            const double labelTime =
                xMin + (xRange * static_cast<double>(i) / 6.0);

            painter.setPen(QPen(QColor("#E1E7EE"), 1));
            painter.drawLine(x, plot.top(), x, plot.bottom());

            painter.setPen(QColor("#6B7683"));

            QRect xTickRect(x - 22,
                            plot.bottom() + 8,
                            44,
                            18);

            painter.drawText(xTickRect,
                             Qt::AlignCenter,
                             QString::number(labelTime, 'f', 0));
        }

        painter.setPen(QColor("#17212B"));

        QFont labelFont = painter.font();
        labelFont.setPointSize(10);
        labelFont.setBold(true);
        painter.setFont(labelFont);

        QRect xLabelRect(plot.center().x() - 70,
                         outer.bottom() - 28,
                         140,
                         18);

        painter.drawText(xLabelRect,
                         Qt::AlignCenter,
                         "Elapsed Time (s)");

        painter.save();
        painter.translate(outer.left() + 22,
                          plot.center().y());
        painter.rotate(-90);

        QRect yLabelRect(-60, -10, 120, 20);

        painter.drawText(yLabelRect,
                         Qt::AlignCenter,
                         "Power (W)");

        painter.restore();
    }

    void drawSeries(QPainter &painter,
                    const QRect &plot,
                    double maxValue,
                    double xMin,
                    double xMax,
                    const QColor &color,
                    int width,
                    bool drawBase)
    {
        if (m_samples.size() < 2)
        {
            return;
        }

        const double xRange = qMax(1.0, xMax - xMin);

        QPolygonF polyline;

        for (const VehiclePowerSample &sample : m_samples)
        {
            if ((sample.timeSec < xMin) || (sample.timeSec > xMax))
            {
                continue;
            }

            const double value =
                drawBase ? sample.basePowerW : sample.motorPowerW;

            const double xRatio =
                (sample.timeSec - xMin) / xRange;

            const double yRatio =
                value / maxValue;

            const double x =
                plot.left() + xRatio * plot.width();

            const double y =
                plot.bottom() - yRatio * plot.height();

            polyline.append(QPointF(x, y));
        }

        if (polyline.size() < 2)
        {
            return;
        }

        painter.setPen(QPen(color, width));
        painter.drawPolyline(polyline);
    }

private:
    QElapsedTimer m_elapsedTimer;
    QVector<VehiclePowerSample> m_samples;
};

class ModuleBProtectionChartWidget : public QWidget
{
public:
    explicit ModuleBProtectionChartWidget(QWidget *parent = nullptr)
        : QWidget(parent),
          m_powerLimitW(0.0)
    {
        setMinimumHeight(330);
        m_elapsedTimer.start();
    }

    void addSample(double moduleBPowerW,
                   double coolingPowerW,
                   double totalPowerW,
                   double powerLimitW)
    {
        const double nowSec =
            m_elapsedTimer.elapsed() / 1000.0;

        ModuleBPowerSample sample;
        sample.timeSec = nowSec;
        sample.moduleBPowerW = moduleBPowerW;
        sample.coolingPowerW = coolingPowerW;
        sample.totalPowerW = totalPowerW;

        m_samples.append(sample);
        m_powerLimitW = powerLimitW;

        while ((!m_samples.isEmpty()) &&
               ((nowSec - m_samples.first().timeSec) > 30.0))
        {
            m_samples.removeFirst();
        }

        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.fillRect(rect(), QColor("#050B12"));

        QRect outer = rect().adjusted(10, 10, -10, -10);

        painter.setPen(QPen(QColor("#DDE3EA"), 1));
        painter.setBrush(QColor("#FFFFFF"));
        painter.drawRoundedRect(outer, 14, 14);

        painter.setPen(QColor("#17212B"));

        QFont titleFont = painter.font();
        titleFont.setPointSize(13);
        titleFont.setBold(true);
        painter.setFont(titleFont);

        painter.drawText(outer.adjusted(18, 12, -18, -12),
                         Qt::AlignTop | Qt::AlignHCenter,
                         "MODULE B POWER PROTECTION");

        QFont legendFont = painter.font();
        legendFont.setPointSize(8);
        legendFont.setBold(true);
        painter.setFont(legendFont);

        const int legendY = outer.top() + 48;
        const int legendStartX = outer.center().x() - 245;

        drawLegendItem(painter,
                       legendStartX,
                       legendY,
                       QColor("#45D7FF"),
                       "Module MCU");

        drawLegendItem(painter,
                       legendStartX + 120,
                       legendY,
                       QColor("#FFB84D"),
                       "Cooling");

        drawLegendItem(painter,
                       legendStartX + 215,
                       legendY,
                       QColor("#20E0B5"),
                       "Module Total");

        drawLegendItem(painter,
                       legendStartX + 345,
                       legendY,
                       QColor("#FF3B3B"),
                       "Power Limit");

        QRect plot = outer.adjusted(86, 86, -28, -64);

        painter.setPen(QPen(QColor("#DDE3EA"), 1));
        painter.setBrush(QColor("#FFFFFF"));
        painter.drawRect(plot);

        double maxValue = 1.0;

        updateMaxValue(maxValue);

        if (m_powerLimitW > maxValue)
        {
            maxValue = m_powerLimitW;
        }

        maxValue = qMax(1.0, maxValue * 1.25);

        const double nowSec =
            m_elapsedTimer.elapsed() / 1000.0;

        const double xMax = qMax(30.0, nowSec);
        const double xMin = qMax(0.0, xMax - 30.0);

        drawGridAndAxis(painter, outer, plot, maxValue, xMin, xMax);

        drawLimitLine(painter, plot, maxValue);

        /*
         * Total과 Module MCU가 같은 값이면 선이 완전히 겹침.
         * 따라서 Total을 먼저 그리고, Module MCU를 마지막에 점선으로 그려서
         * 파란 Module MCU 선이 초록 Total 선 위에 보이도록 함.
         */
        drawSeries(painter,
                   plot,
                   maxValue,
                   xMin,
                   xMax,
                   QColor("#20E0B5"),
                   4,
                   Qt::SolidLine,
                   2);

        drawSeries(painter,
                   plot,
                   maxValue,
                   xMin,
                   xMax,
                   QColor("#FFB84D"),
                   2,
                   Qt::SolidLine,
                   1);

        drawSeries(painter,
                   plot,
                   maxValue,
                   xMin,
                   xMax,
                   QColor("#45D7FF"),
                   3,
                   Qt::DashLine,
                   0);

        drawCurrentValueLabel(painter,
                              plot,
                              maxValue);
    }

private:
    void drawLegendItem(QPainter &painter,
                        int x,
                        int y,
                        const QColor &color,
                        const QString &text)
    {
        painter.setBrush(color);
        painter.setPen(color);
        painter.drawRect(x, y - 8, 9, 9);

        painter.setPen(QColor("#52606D"));
        painter.drawText(x + 14,
                         y + 2,
                         text);
    }

    void updateMaxValue(double &maxValue)
    {
        for (const ModuleBPowerSample &sample : m_samples)
        {
            if (sample.moduleBPowerW > maxValue)
            {
                maxValue = sample.moduleBPowerW;
            }

            if (sample.coolingPowerW > maxValue)
            {
                maxValue = sample.coolingPowerW;
            }

            if (sample.totalPowerW > maxValue)
            {
                maxValue = sample.totalPowerW;
            }
        }
    }

    void drawGridAndAxis(QPainter &painter,
                         const QRect &outer,
                         const QRect &plot,
                         double maxValue,
                         double xMin,
                         double xMax)
    {
        QFont axisFont = painter.font();
        axisFont.setPointSize(10);
        axisFont.setBold(false);
        painter.setFont(axisFont);

        for (int i = 0; i <= 4; i++)
        {
            const int y = plot.top() + (plot.height() * i / 4);
            const double labelValue =
                maxValue * (4 - i) / 4.0;

            painter.setPen(QPen(QColor("#E1E7EE"), 1));
            painter.drawLine(plot.left(), y, plot.right(), y);

            painter.setPen(QColor("#6B7683"));

            QRect yTickRect(plot.left() - 66,
                            y - 10,
                            54,
                            20);

            painter.drawText(yTickRect,
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(labelValue, 'f', 1) + " W");
        }

        const double xRange = qMax(1.0, xMax - xMin);

        for (int i = 0; i <= 6; i++)
        {
            const int x = plot.left() + (plot.width() * i / 6);
            const double labelTime =
                xMin + (xRange * static_cast<double>(i) / 6.0);

            painter.setPen(QPen(QColor("#E1E7EE"), 1));
            painter.drawLine(x, plot.top(), x, plot.bottom());

            painter.setPen(QColor("#6B7683"));

            QRect xTickRect(x - 22,
                            plot.bottom() + 8,
                            44,
                            18);

            painter.drawText(xTickRect,
                             Qt::AlignCenter,
                             QString::number(labelTime, 'f', 0));
        }

        painter.setPen(QColor("#17212B"));

        QFont labelFont = painter.font();
        labelFont.setPointSize(10);
        labelFont.setBold(true);
        painter.setFont(labelFont);

        QRect xLabelRect(plot.center().x() - 70,
                         outer.bottom() - 28,
                         140,
                         18);

        painter.drawText(xLabelRect,
                         Qt::AlignCenter,
                         "Elapsed Time (s)");

        painter.save();
        painter.translate(outer.left() + 22,
                          plot.center().y());
        painter.rotate(-90);

        QRect yLabelRect(-60, -10, 120, 20);

        painter.drawText(yLabelRect,
                         Qt::AlignCenter,
                         "Power (W)");

        painter.restore();
    }

    void drawLimitLine(QPainter &painter,
                       const QRect &plot,
                       double maxValue)
    {
        if (m_powerLimitW <= 0.0)
        {
            return;
        }

        const int limitY =
            plot.bottom() -
            static_cast<int>((m_powerLimitW / maxValue) * plot.height());

        QPen limitPen(QColor("#FF3B3B"));
        limitPen.setWidth(2);
        limitPen.setStyle(Qt::DashLine);

        painter.setPen(limitPen);
        painter.drawLine(plot.left(), limitY, plot.right(), limitY);

        QFont limitFont = painter.font();
        limitFont.setPointSize(9);
        limitFont.setBold(true);
        painter.setFont(limitFont);

        painter.setPen(QColor("#FF3B3B"));

        QRect limitTextRect(plot.right() - 135,
                            limitY - 24,
                            130,
                            20);

        painter.drawText(limitTextRect,
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString("LIMIT %1 W").arg(m_powerLimitW, 0, 'f', 1));
    }

    void drawCurrentValueLabel(QPainter &painter,
                               const QRect &plot,
                               double maxValue)
    {
        if (m_samples.isEmpty())
        {
            return;
        }

        const double latestValue = m_samples.last().totalPowerW;
        const int y =
            plot.bottom() -
            static_cast<int>((latestValue / maxValue) * plot.height());

        painter.setPen(QColor("#20E0B5"));

        QFont valueFont = painter.font();
        valueFont.setPointSize(9);
        valueFont.setBold(true);
        painter.setFont(valueFont);

        QRect textRect(plot.right() - 120,
                       y - 22,
                       115,
                       20);

        painter.drawText(textRect,
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString("TOTAL %1 W").arg(latestValue, 0, 'f', 2));
    }

    void drawSeries(QPainter &painter,
                    const QRect &plot,
                    double maxValue,
                    double xMin,
                    double xMax,
                    const QColor &color,
                    int width,
                    Qt::PenStyle style,
                    int seriesType)
    {
        if (m_samples.size() < 2)
        {
            return;
        }

        const double xRange = qMax(1.0, xMax - xMin);

        QPolygonF polyline;

        for (const ModuleBPowerSample &sample : m_samples)
        {
            if ((sample.timeSec < xMin) || (sample.timeSec > xMax))
            {
                continue;
            }

            double value = sample.moduleBPowerW;

            if (seriesType == 1)
            {
                value = sample.coolingPowerW;
            }
            else if (seriesType == 2)
            {
                value = sample.totalPowerW;
            }

            const double xRatio =
                (sample.timeSec - xMin) / xRange;

            const double yRatio =
                value / maxValue;

            const double x =
                plot.left() + xRatio * plot.width();

            const double y =
                plot.bottom() - yRatio * plot.height();

            polyline.append(QPointF(x, y));
        }

        if (polyline.size() < 2)
        {
            return;
        }

        QPen pen(color, width);
        pen.setStyle(style);

        painter.setPen(pen);
        painter.drawPolyline(polyline);
    }

private:
    QElapsedTimer m_elapsedTimer;
    QVector<ModuleBPowerSample> m_samples;

    double m_powerLimitW;
};

PowerWindow::PowerWindow(QWidget *parent)
    : QMainWindow(parent),
      m_telemetryClient(new TelemetryClient(this)),
      m_connectionLabel(new QLabel("UART SERVICE : WAITING", this)),
      m_basePowerLabel(nullptr),
      m_motorPowerLabel(nullptr),
      m_moduleBPowerLabel(nullptr),
      m_coolingPowerLabel(nullptr),
      m_statusLabel(nullptr),
      m_vehicleChart(nullptr),
      m_moduleBProtectionChart(nullptr)
{
    setWindowTitle("PBV Power Monitor");
    resize(1280, 720);

    QWidget *root = new QWidget(this);
    root->setObjectName("PowerRoot");
    root->setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout *mainLayout = new QVBoxLayout(root);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(16);

    QLabel *titleLabel = new QLabel("PBV POWER MONITOR", root);
    titleLabel->setObjectName("TitleLabel");

    m_connectionLabel->setObjectName("ConnectionLabel");
    m_connectionLabel->hide();

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    QGridLayout *cardLayout = new QGridLayout();
    cardLayout->setHorizontalSpacing(14);
    cardLayout->setVerticalSpacing(14);

    cardLayout->addWidget(createCard("BASE (MCU) POWER", &m_basePowerLabel), 0, 0);
    cardLayout->addWidget(createCard("MOTOR POWER", &m_motorPowerLabel), 0, 1);
    cardLayout->addWidget(createCard("MODULE (MCU) POWER", &m_moduleBPowerLabel), 0, 2);
    cardLayout->addWidget(createCard("COOLING POWER", &m_coolingPowerLabel), 0, 3);

    mainLayout->addLayout(cardLayout);

    m_statusLabel = new QLabel("POWER NORMAL", root);
    m_statusLabel->setObjectName("PowerStatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setMinimumHeight(44);

    /*
     * Power 상태는 Module B HMI 화면에서 표시.
     * Power Monitor는 전력 수치와 그래프만 표시한다.
     */
    m_statusLabel->hide();

    QHBoxLayout *chartLayout = new QHBoxLayout();
    chartLayout->setSpacing(16);

    m_vehicleChart = new VehiclePowerChartWidget(root);
    m_moduleBProtectionChart = new ModuleBProtectionChartWidget(root);

    chartLayout->addWidget(m_vehicleChart);
    chartLayout->addWidget(m_moduleBProtectionChart);

    mainLayout->addLayout(chartLayout, 1);

    setCentralWidget(root);

    root->setStyleSheet(
        "#PowerRoot {"
        " background: #050B12;"
        " color: #EAF6FF;"
        " font-family: Arial;"
        "}"
        "#TitleLabel {"
        " color: #EAF6FF;"
        " font-size: 28px;"
        " font-weight: 900;"
        " letter-spacing: 2px;"
        "}"
        "#ConnectionLabel {"
        " color: #8DA5B8;"
        " background: #0B1826;"
        " border: 1px solid #1E3A56;"
        " border-radius: 12px;"
        " padding: 8px 14px;"
        " font-size: 12px;"
        " font-weight: 700;"
        "}"
        "QFrame#PowerCard {"
        " background: #0B1826;"
        " border: 1px solid #1E3A56;"
        " border-radius: 18px;"
        "}"
        "QLabel#CardTitle {"
        " color: #8DA5B8;"
        " font-size: 12px;"
        " font-weight: 800;"
        " letter-spacing: 1px;"
        "}"
        "QLabel#CardValue {"
        " color: #45D7FF;"
        " font-size: 30px;"
        " font-weight: 900;"
        "}"
        "QLabel#PowerStatusLabel {"
        " background: #0B1826;"
        " color: #20E0B5;"
        " border: 1px solid #1E3A56;"
        " border-radius: 14px;"
        " padding: 10px 18px;"
        " font-size: 20px;"
        " font-weight: 900;"
        " letter-spacing: 1px;"
        "}"
    );

    connect(m_telemetryClient,
            &TelemetryClient::powerTelemetryReceived,
            this,
            &PowerWindow::onPowerTelemetryReceived);

    connect(m_telemetryClient,
            &TelemetryClient::connectionChanged,
            this,
            &PowerWindow::onConnectionChanged);
}

QWidget *PowerWindow::createCard(const QString &title,
                                 QLabel **valueLabel)
{
    QFrame *card = new QFrame(this);
    card->setObjectName("PowerCard");
    card->setMinimumHeight(108);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(8);

    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("CardTitle");

    QLabel *value = new QLabel("0.00 W", card);
    value->setObjectName("CardValue");

    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(value);

    *valueLabel = value;

    return card;
}

void PowerWindow::onPowerTelemetryReceived(int basePowerMw,
                                           int moduleBPowerMw,
                                           int coolingPowerMw,
                                           int motorPowerMw,
                                           int totalPowerMw,
                                           int powerWarningCount,
                                           int powerFault,
                                           int powerLimitMw)
{
    const double basePowerW = basePowerMw / 1000.0;
    const double moduleBPowerW = moduleBPowerMw / 1000.0;
    const double coolingPowerW = coolingPowerMw / 1000.0;
    const double motorPowerW = motorPowerMw / 1000.0;
    const double moduleBTotalW = totalPowerMw / 1000.0;
    const double powerLimitW = powerLimitMw / 1000.0;

    m_basePowerLabel->setText(
        QString::number(basePowerW, 'f', 2) + " W"
    );

    m_motorPowerLabel->setText(
        QString::number(motorPowerW, 'f', 2) + " W"
    );

    m_moduleBPowerLabel->setText(
        QString::number(moduleBPowerW, 'f', 2) + " W"
    );

    m_coolingPowerLabel->setText(
        QString::number(coolingPowerW, 'f', 2) + " W"
    );

    if (powerFault == 1)
    {
        m_statusLabel->setText("POWER CUTOFF");

        m_statusLabel->setStyleSheet(
            makeStatusStyle("#2A0B0B",
                            "#FF5A5A",
                            "#FF3B3B")
        );
    }
    else if (powerWarningCount > 0)
    {
        m_statusLabel->setText(
            QString("POWER WARNING %1 / 4").arg(powerWarningCount)
        );

        m_statusLabel->setStyleSheet(
            makeStatusStyle("#2A220B",
                            "#FFD45A",
                            "#FFB800")
        );
    }
    else
    {
        m_statusLabel->setText("POWER NORMAL");

        m_statusLabel->setStyleSheet(
            makeStatusStyle("#0B1826",
                            "#20E0B5",
                            "#1E3A56")
        );
    }

    m_vehicleChart->addSample(basePowerW,
                              motorPowerW);

    m_moduleBProtectionChart->addSample(moduleBPowerW,
                                        coolingPowerW,
                                        moduleBTotalW,
                                        powerLimitW);
}

void PowerWindow::onConnectionChanged(bool connected)
{
    Q_UNUSED(connected);
}