#include "temperaturewindow.h"
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

TemperatureWindow::TemperatureWindow(QWidget *parent)
    : QMainWindow(parent),
      m_telemetryClient(new TelemetryClient(this)),
      m_connectionLabel(new QLabel("UART SERVICE : CONNECTING", this)),
      m_moduleLabel(new QLabel("MODULE : WAITING FOR MODULE B", this)),
      m_authLabel(new QLabel("AUTH : --", this)),
      m_currentTempLabel(new QLabel("--.- °C", this)),
      m_targetTempLabel(new QLabel("--.- °C", this)),
      m_chart(new QChart()),
      m_chartView(new QChartView(m_chart, this)),
      m_currentTempSeries(new QLineSeries()),
      m_targetTempSeries(new QLineSeries()),
      m_restartThresholdSeries(new QLineSeries()),
      m_axisX(new QValueAxis()),
      m_axisY(new QValueAxis())
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);

    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(10);

    // ============================================================
    // Header
    // ============================================================
    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *systemTitle =
        new QLabel("PBV DOCKING MODULE CONTROL SYSTEM");

    systemTitle->setStyleSheet(
        "font-size: 20px;"
        "font-weight: 700;"
        "color: #EAF6FF;"
    );

    QLabel *pageTitle =
        new QLabel("MODULE B / TEMPERATURE MONITOR");

    pageTitle->setStyleSheet(
        "font-size: 14px;"
        "font-weight: 700;"
        "color: #46D8FF;"
    );

    headerLayout->addWidget(systemTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(pageTitle);

    // ============================================================
    // Status row
    // ============================================================
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

    // ============================================================
    // Temperature cards
    // ============================================================
    QHBoxLayout *infoLayout = new QHBoxLayout();

    QFrame *currentCard = new QFrame();
    QFrame *targetCard = new QFrame();

    const QString cardStyle =
        "QFrame {"
        " background: #17212B;"
        " border: 1px solid #314353;"
        " border-radius: 12px;"
        "}";

    currentCard->setStyleSheet(cardStyle);
    targetCard->setStyleSheet(cardStyle);

    QVBoxLayout *currentLayout = new QVBoxLayout(currentCard);
    QVBoxLayout *targetLayout = new QVBoxLayout(targetCard);

    QLabel *currentTitle = new QLabel("CURRENT TEMPERATURE");
    QLabel *targetTitle = new QLabel("TARGET TEMPERATURE");

    const QString titleStyle =
        "font-size: 13px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "border: none;"
        "background: transparent;";

    currentTitle->setStyleSheet(titleStyle);
    targetTitle->setStyleSheet(titleStyle);

    const QString valueStyle =
        "font-size: 52px;"
        "font-weight: 700;"
        "color: #35CFFF;"
        "border: none;"
        "background: transparent;";

    m_currentTempLabel->setAlignment(Qt::AlignCenter);
    m_targetTempLabel->setAlignment(Qt::AlignCenter);

    m_currentTempLabel->setStyleSheet(valueStyle);
    m_targetTempLabel->setStyleSheet(valueStyle);

    currentLayout->addWidget(currentTitle);
    currentLayout->addStretch();
    currentLayout->addWidget(m_currentTempLabel);
    currentLayout->addStretch();

    targetLayout->addWidget(targetTitle);
    targetLayout->addStretch();
    targetLayout->addWidget(m_targetTempLabel);
    targetLayout->addStretch();

    infoLayout->addWidget(currentCard);
    infoLayout->addWidget(targetCard);

    // ============================================================
    // Temperature graph
    // ============================================================
    m_currentTempSeries->setName("Current Temperature");
    m_targetTempSeries->setName("Target Temperature");
    m_restartThresholdSeries->setName(
        "Cooling Restart Threshold (+3°C)"
    );

    QPen currentPen(QColor("#009FD5"));
    currentPen.setWidth(3);

    QPen targetPen(QColor("#E57C23"));
    targetPen.setWidth(3);
    targetPen.setStyle(Qt::DashLine);

    QPen restartThresholdPen(QColor("#E53935"));
    restartThresholdPen.setWidth(2);
    restartThresholdPen.setStyle(Qt::DashDotLine);

    m_currentTempSeries->setPen(currentPen);
    m_targetTempSeries->setPen(targetPen);
    m_restartThresholdSeries->setPen(restartThresholdPen);

    m_chart->addSeries(m_currentTempSeries);
    m_chart->addSeries(m_targetTempSeries);
    m_chart->addSeries(m_restartThresholdSeries);

    m_chart->setTitle("1. Temperature Trend (Real-Time)");

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

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignTop);
    m_chart->legend()->setLabelColor(QColor("#263541"));

    // X axis
    m_axisX->setTitleText("Time (s)");
    m_axisX->setLabelFormat("%.0f");
    m_axisX->setTickCount(7);
    m_axisX->setRange(0.0, DISPLAY_WINDOW_SEC);

    m_axisX->setTitleBrush(QBrush(QColor("#1B2733")));
    m_axisX->setLabelsBrush(QBrush(QColor("#1B2733")));
    m_axisX->setLabelsColor(QColor("#1B2733"));
    m_axisX->setGridLineColor(QColor("#D5DDE4"));
    m_axisX->setLinePenColor(QColor("#4C5A65"));

    // Y axis
    m_axisY->setTitleText("Temperature (°C)");
    m_axisY->setLabelFormat("%.0f");
    m_axisY->setTickCount(6);
    m_axisY->setRange(TEMP_Y_MIN, TEMP_Y_MAX);

    m_axisY->setTitleBrush(QBrush(QColor("#1B2733")));
    m_axisY->setLabelsBrush(QBrush(QColor("#1B2733")));
    m_axisY->setLabelsColor(QColor("#1B2733"));
    m_axisY->setGridLineColor(QColor("#D5DDE4"));
    m_axisY->setLinePenColor(QColor("#4C5A65"));

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_currentTempSeries->attachAxis(m_axisX);
    m_currentTempSeries->attachAxis(m_axisY);

    m_targetTempSeries->attachAxis(m_axisX);
    m_targetTempSeries->attachAxis(m_axisY);

    m_restartThresholdSeries->attachAxis(m_axisX);
    m_restartThresholdSeries->attachAxis(m_axisY);

    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(470);

    m_chartView->setStyleSheet(
        "background-color: white;"
        "border: 1px solid #314353;"
        "border-radius: 12px;"
    );

    // ============================================================
    // Reset button
    // ============================================================
    QPushButton *resetButton =
        new QPushButton("START / RESET RECORDING");

    resetButton->setStyleSheet(
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
    buttonLayout->addWidget(resetButton);

    rootLayout->addLayout(headerLayout);
    rootLayout->addLayout(statusLayout);
    rootLayout->addLayout(infoLayout);
    rootLayout->addWidget(m_chartView, 1);
    rootLayout->addLayout(buttonLayout);

    setCentralWidget(centralWidget);

    setWindowTitle("PBV Temperature Monitor");
    resize(1280, 820);

    setStyleSheet(
        "QMainWindow { background: #0D141B; }"
        "QLabel { color: #EAF6FF; }"
    );

    m_elapsedTimer.start();

    connect(m_telemetryClient,
            &TelemetryClient::telemetryReceived,
            this,
            &TemperatureWindow::onTelemetryReceived);

    connect(m_telemetryClient,
            &TelemetryClient::connectionChanged,
            this,
            &TemperatureWindow::onConnectionChanged);

    connect(resetButton,
            &QPushButton::clicked,
            this,
            &TemperatureWindow::resetRecording);
}

void TemperatureWindow::onTelemetryReceived(int moduleType,
                                            int authResult,
                                            int pressure,
                                            int currentTempX10,
                                            int targetTempX10,
                                            int peltierDuty)
{
    Q_UNUSED(pressure)
    Q_UNUSED(peltierDuty)

    if (moduleType != 2)
    {
        m_moduleLabel->setText("MODULE : WAITING FOR MODULE B");
        return;
    }

    const double currentTemp =
        static_cast<double>(currentTempX10) / 10.0;

    const double targetTemp =
        static_cast<double>(targetTempX10) / 10.0;

    const double coolingRestartTemp = targetTemp + 3.0;

    const double elapsedSeconds =
        static_cast<double>(m_elapsedTimer.elapsed()) / 1000.0;

    m_currentTempLabel->setText(
        QString("%1 °C").arg(currentTemp, 0, 'f', 1)
    );

    m_targetTempLabel->setText(
        QString("%1 °C").arg(targetTemp, 0, 'f', 1)
    );

    m_moduleLabel->setText("MODULE : MODULE B");

    m_authLabel->setText(
        QString("AUTH : %1")
            .arg(authResult == 1 ? "ACCEPTED" : "REJECTED")
    );

    m_currentTempSeries->append(elapsedSeconds, currentTemp);
    m_targetTempSeries->append(elapsedSeconds, targetTemp);

    m_restartThresholdSeries->append(
        elapsedSeconds,
        coolingRestartTemp
    );

    while (!m_currentTempSeries->points().isEmpty() &&
           m_currentTempSeries->points().first().x() <
               (elapsedSeconds - DISPLAY_WINDOW_SEC))
    {
        m_currentTempSeries->remove(0);
    }

    while (!m_targetTempSeries->points().isEmpty() &&
           m_targetTempSeries->points().first().x() <
               (elapsedSeconds - DISPLAY_WINDOW_SEC))
    {
        m_targetTempSeries->remove(0);
    }

    while (!m_restartThresholdSeries->points().isEmpty() &&
           m_restartThresholdSeries->points().first().x() <
               (elapsedSeconds - DISPLAY_WINDOW_SEC))
    {
        m_restartThresholdSeries->remove(0);
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
}

void TemperatureWindow::onConnectionChanged(bool connected)
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

void TemperatureWindow::resetRecording()
{
    m_currentTempSeries->clear();
    m_targetTempSeries->clear();
    m_restartThresholdSeries->clear();

    m_elapsedTimer.restart();

    m_axisX->setRange(0.0, DISPLAY_WINDOW_SEC);
    m_axisY->setRange(TEMP_Y_MIN, TEMP_Y_MAX);

    m_currentTempLabel->setText("--.- °C");
    m_targetTempLabel->setText("--.- °C");
}