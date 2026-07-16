#include "weight_monitor_window.h"
#include "telemetry_client.h"

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <QBrush>
#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_telemetryClient(new TelemetryClient(this)),
      m_connectionLabel(new QLabel("UART SERVICE : CONNECTING", this)),
      m_moduleLabel(new QLabel("MODULE : --", this)),
      m_authLabel(new QLabel("AUTH : --", this)),
      m_pressureValueLabel(new QLabel("-- g", this)),
      m_minValueLabel(new QLabel("MIN : -- g", this)),
      m_maxValueLabel(new QLabel("MAX : -- g", this)),
      m_chart(new QChart()),
      m_chartView(new QChartView(m_chart, this)),
      m_pressureSeries(new QLineSeries()),
      m_axisX(new QValueAxis()),
      m_axisY(new QValueAxis())
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);

    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(10);

    /* ============================================================
     * Header
     * ============================================================ */
    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *systemTitle =
        new QLabel("PBV DOCKING MODULE CONTROL SYSTEM");

    systemTitle->setStyleSheet(
        "font-size: 20px;"
        "font-weight: 700;"
        "color: #EAF6FF;"
    );

    QLabel *pageTitle =
        new QLabel("MODULE A / WEIGHT MONITOR");

    pageTitle->setStyleSheet(
        "font-size: 14px;"
        "font-weight: 700;"
        "color: #46D8FF;"
    );

    headerLayout->addWidget(systemTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(pageTitle);

    /* ============================================================
     * System status
     * ============================================================ */
    QHBoxLayout *statusLayout = new QHBoxLayout();

    m_connectionLabel->setStyleSheet(
        "padding: 8px 14px;"
        "border: 1px solid #384A5A;"
        "border-radius: 8px;"
        "color: #AAB8C7;"
        "background: #17212B;"
    );

    m_moduleLabel->setStyleSheet(
        "padding: 8px 14px;"
        "border: 1px solid #384A5A;"
        "border-radius: 8px;"
        "color: #AAB8C7;"
        "background: #17212B;"
    );

    m_authLabel->setStyleSheet(
        "padding: 8px 14px;"
        "border: 1px solid #384A5A;"
        "border-radius: 8px;"
        "color: #AAB8C7;"
        "background: #17212B;"
    );

    statusLayout->addWidget(m_connectionLabel);
    statusLayout->addWidget(m_moduleLabel);
    statusLayout->addWidget(m_authLabel);
    statusLayout->addStretch();

    /* ============================================================
     * Current weight / graph statistics
     * ============================================================ */
    QHBoxLayout *infoLayout = new QHBoxLayout();

    QFrame *currentCard = new QFrame();

    currentCard->setStyleSheet(
        "QFrame {"
        " background: #17212B;"
        " border: 1px solid #314353;"
        " border-radius: 12px;"
        "}"
    );

    QVBoxLayout *currentCardLayout =
        new QVBoxLayout(currentCard);

    QLabel *weightLabel =
        new QLabel("CURRENT WEIGHT");

    weightLabel->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "border: none;"
        "background: transparent;"
    );

    m_pressureValueLabel->setAlignment(Qt::AlignCenter);

    m_pressureValueLabel->setStyleSheet(
        "font-size: 56px;"
        "font-weight: 700;"
        "color: #35CFFF;"
        "border: none;"
        "background: transparent;"
    );

    QLabel *weightUnit =
        new QLabel("GRAM");

    weightUnit->setAlignment(Qt::AlignCenter);

    weightUnit->setStyleSheet(
        "font-size: 13px;"
        "color: #8EA5B8;"
        "border: none;"
        "background: transparent;"
    );

    currentCardLayout->addWidget(weightLabel);
    currentCardLayout->addStretch();
    currentCardLayout->addWidget(m_pressureValueLabel);
    currentCardLayout->addWidget(weightUnit);
    currentCardLayout->addStretch();

    QFrame *statCard = new QFrame();

    statCard->setStyleSheet(
        "QFrame {"
        " background: #17212B;"
        " border: 1px solid #314353;"
        " border-radius: 12px;"
        "}"
    );

    QVBoxLayout *statCardLayout =
        new QVBoxLayout(statCard);

    QLabel *statTitle =
        new QLabel("GRAPH STATISTICS");

    statTitle->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "border: none;"
        "background: transparent;"
    );

    m_minValueLabel->setStyleSheet(
        "font-size: 17px;"
        "font-weight: 700;"
        "color: #DCEBFA;"
        "padding: 4px;"
        "border: 1px solid #314353;"
        "border-radius: 8px;"
        "background: #14202A;"
    );

    m_maxValueLabel->setStyleSheet(
        "font-size: 17px;"
        "font-weight: 700;"
        "color: #DCEBFA;"
        "padding: 4px;"
        "border: 1px solid #314353;"
        "border-radius: 8px;"
        "background: #14202A;"
    );

    statCardLayout->addWidget(statTitle);
    statCardLayout->addStretch();
    statCardLayout->addWidget(m_minValueLabel);
    statCardLayout->addWidget(m_maxValueLabel);
    statCardLayout->addStretch();

    infoLayout->addWidget(currentCard, 3);
    infoLayout->addWidget(statCard, 1);

    /* ============================================================
     * Weight graph
     * ============================================================ */
    m_pressureSeries->setName("Weight");

    QPen linePen(QColor("#009FD5"));
    linePen.setWidth(3);

    m_pressureSeries->setPen(linePen);

    m_chart->addSeries(m_pressureSeries);

    m_chart->setTitle("Weight Trend (Real-Time)");

    QFont chartTitleFont;
    chartTitleFont.setPointSize(13);
    chartTitleFont.setBold(true);

    m_chart->setTitleFont(chartTitleFont);
    m_chart->setTitleBrush(QBrush(QColor("#1B2733")));

    m_chart->setBackgroundVisible(true);
    m_chart->setBackgroundBrush(QBrush(Qt::white));
    m_chart->setBackgroundRoundness(0);

    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->setPlotAreaBackgroundBrush(QBrush(Qt::white));

    m_chart->setMargins(QMargins(12, 12, 12, 12));

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignTop);
    m_chart->legend()->setLabelColor(QColor("#263541"));

    /* X axis */
    m_axisX->setTitleText("Time (s)");
    m_axisX->setLabelFormat("%.0f");
    m_axisX->setTickCount(7);
    m_axisX->setRange(0.0, DISPLAY_WINDOW_SEC);

    m_axisX->setTitleBrush(QBrush(QColor("#1B2733")));
    m_axisX->setLabelsBrush(QBrush(QColor("#1B2733")));
    m_axisX->setLabelsColor(QColor("#1B2733"));
    m_axisX->setGridLineColor(QColor("#D5DDE4"));
    m_axisX->setLinePenColor(QColor("#4C5A65"));

    /* Y axis: 0 ~ 1000 g fixed */
    m_axisY->setTitleText("Weight (g)");
    m_axisY->setLabelFormat("%.0f");
    m_axisY->setTickCount(6);
    m_axisY->setRange(0.0, 750.0);

    m_axisY->setTitleBrush(QBrush(QColor("#1B2733")));
    m_axisY->setLabelsBrush(QBrush(QColor("#1B2733")));
    m_axisY->setLabelsColor(QColor("#1B2733"));
    m_axisY->setGridLineColor(QColor("#D5DDE4"));
    m_axisY->setLinePenColor(QColor("#4C5A65"));

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_pressureSeries->attachAxis(m_axisX);
    m_pressureSeries->attachAxis(m_axisY);

    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(470);

    m_chartView->setStyleSheet(
        "background-color: white;"
        "border: 1px solid #314353;"
        "border-radius: 12px;"
    );

    /* ============================================================
     * Button
     * ============================================================ */
    QPushButton *clearButton =
        new QPushButton("START / RESET RECORDING");

    clearButton->setStyleSheet(
        "QPushButton {"
        " background: #1D3444;"
        " color: #EAF6FF;"
        " border: 1px solid #46D8FF;"
        " border-radius: 8px;"
        " padding: 10px 20px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover {"
        " background: #27516B;"
        "}"
    );

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    buttonLayout->addStretch();
    buttonLayout->addWidget(clearButton);

    rootLayout->addLayout(headerLayout);
    rootLayout->addLayout(statusLayout);
    rootLayout->addLayout(infoLayout);
    rootLayout->addWidget(m_chartView, 1);
    rootLayout->addLayout(buttonLayout);

    setCentralWidget(centralWidget);

    setWindowTitle("PBV Weight Monitor");
    resize(1280, 820);

    setStyleSheet(
        "QMainWindow { background: #0D141B; }"
        "QLabel { color: #EAF6FF; }"
    );

    m_elapsedTimer.start();

    connect(m_telemetryClient,
            &TelemetryClient::telemetryReceived,
            this,
            &MainWindow::onTelemetryReceived);

    connect(m_telemetryClient,
            &TelemetryClient::connectionChanged,
            this,
            &MainWindow::onConnectionChanged);

    connect(clearButton,
            &QPushButton::clicked,
            this,
            &MainWindow::clearGraph);
}

void MainWindow::onTelemetryReceived(int moduleType,
                                     int authResult,
                                     int pressure,
                                     int currentTempX10,
                                     int targetTempX10,
                                     int peltierDuty,
                                     int motorRunning,
                                     int motorSpeedLevel,
                                     int targetRpmX10,
                                     int currentRpmX10,
                                     int motorDutyX10)
{
    Q_UNUSED(currentTempX10)
    Q_UNUSED(targetTempX10)
    Q_UNUSED(peltierDuty)
    Q_UNUSED(motorRunning)
    Q_UNUSED(motorSpeedLevel)
    Q_UNUSED(targetRpmX10)
    Q_UNUSED(currentRpmX10)
    Q_UNUSED(motorDutyX10)

    const double elapsedSeconds =
        static_cast<double>(m_elapsedTimer.elapsed()) / 1000.0;

    m_pressureValueLabel->setText(
        QString("%1 g").arg(pressure)
    );

    m_moduleLabel->setText(
        QString("MODULE : %1")
            .arg(moduleType == 1 ? "MODULE A" :
                 moduleType == 2 ? "MODULE B" : "UNKNOWN")
    );

    m_authLabel->setText(
        QString("AUTH : %1")
            .arg(authResult == 1 ? "ACCEPTED" : "REJECTED")
    );

    m_pressureSeries->append(elapsedSeconds, pressure);

    while (!m_pressureSeries->points().isEmpty() &&
           m_pressureSeries->points().first().x() <
               (elapsedSeconds - DISPLAY_WINDOW_SEC))
    {
        m_pressureSeries->remove(0);
    }

    if (elapsedSeconds <= DISPLAY_WINDOW_SEC)
    {
        m_axisX->setRange(0.0, DISPLAY_WINDOW_SEC);
    }
    else
    {
        m_axisX->setRange(elapsedSeconds - DISPLAY_WINDOW_SEC,
                          elapsedSeconds);
    }

    updatePressureAxis();
}

void MainWindow::onConnectionChanged(bool connected)
{
    if (connected)
    {
        m_connectionLabel->setText("UART SERVICE : CONNECTED");

        m_connectionLabel->setStyleSheet(
            "padding: 8px 14px;"
            "border: 1px solid #37D996;"
            "border-radius: 8px;"
            "color: #BFF7DD;"
            "background: #163126;"
        );
    }
    else
    {
        m_connectionLabel->setText("UART SERVICE : WAITING");

        m_connectionLabel->setStyleSheet(
            "padding: 8px 14px;"
            "border: 1px solid #384A5A;"
            "border-radius: 8px;"
            "color: #AAB8C7;"
            "background: #17212B;"
        );
    }
}

void MainWindow::clearGraph()
{
    m_pressureSeries->clear();

    m_elapsedTimer.restart();

    m_axisX->setRange(0.0, DISPLAY_WINDOW_SEC);
    m_axisY->setRange(0.0, 750.0);

    m_pressureValueLabel->setText("-- g");
    m_minValueLabel->setText("MIN : -- g");
    m_maxValueLabel->setText("MAX : -- g");
}

void MainWindow::updatePressureAxis()
{
    const QList<QPointF> points =
        m_pressureSeries->points();

    m_axisY->setRange(0.0, 750.0);

    if (points.isEmpty())
    {
        m_minValueLabel->setText("MIN : -- g");
        m_maxValueLabel->setText("MAX : -- g");
        return;
    }

    double minValue = points.first().y();
    double maxValue = points.first().y();

    for (const QPointF &point : points)
    {
        minValue = qMin(minValue, point.y());
        maxValue = qMax(maxValue, point.y());
    }

    m_minValueLabel->setText(
        QString("MIN : %1 g").arg(minValue, 0, 'f', 0)
    );

    m_maxValueLabel->setText(
        QString("MAX : %1 g").arg(maxValue, 0, 'f', 0)
    );
}