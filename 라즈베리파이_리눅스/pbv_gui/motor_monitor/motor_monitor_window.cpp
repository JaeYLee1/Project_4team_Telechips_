#include "motor_monitor_window.h"
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

MotorMonitorWindow::MotorMonitorWindow(QWidget *parent)
    : QMainWindow(parent),
      m_telemetryClient(new TelemetryClient(this)),
      m_connectionLabel(new QLabel("UART SERVICE : CONNECTING", this)),
      m_motorStateLabel(new QLabel("MOTOR : STOPPED", this)),
      m_speedProfileLabel(new QLabel("PROFILE : --", this)),
      m_targetRpmLabel(new QLabel("--.- RPM", this)),
      m_currentRpmLabel(new QLabel("--.- RPM", this)),
      m_pwmDutyLabel(new QLabel("--.- %", this)),
      m_rpmChart(new QChart()),
      m_rpmChartView(new QChartView(m_rpmChart, this)),
      m_targetRpmSeries(new QLineSeries()),
      m_currentRpmSeries(new QLineSeries()),
      m_rpmAxisX(new QValueAxis()),
      m_rpmAxisY(new QValueAxis()),
      m_dutyChart(new QChart()),
      m_dutyChartView(new QChartView(m_dutyChart, this)),
      m_dutySeries(new QLineSeries()),
      m_dutyAxisX(new QValueAxis()),
      m_dutyAxisY(new QValueAxis())
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
        new QLabel("MOTOR PI CONTROL MONITOR");

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
        "font-weight: 700;"
    );

    m_motorStateLabel->setStyleSheet(
        "padding: 8px 14px;"
        "border: 1px solid #384A5A;"
        "border-radius: 8px;"
        "color: #AAB8C7;"
        "background: #17212B;"
        "font-weight: 700;"
    );

    m_speedProfileLabel->setStyleSheet(
        "padding: 8px 14px;"
        "border: 1px solid #384A5A;"
        "border-radius: 8px;"
        "color: #AAB8C7;"
        "background: #17212B;"
        "font-weight: 700;"
    );

    statusLayout->addWidget(m_connectionLabel);
    statusLayout->addWidget(m_motorStateLabel);
    statusLayout->addWidget(m_speedProfileLabel);
    statusLayout->addStretch();

    // ============================================================
    // RPM / Duty cards
    // ============================================================
    QHBoxLayout *infoLayout = new QHBoxLayout();

    QFrame *targetRpmCard = new QFrame();
    QFrame *currentRpmCard = new QFrame();
    QFrame *dutyCard = new QFrame();

    const QString cardStyle =
        "QFrame {"
        " background: #17212B;"
        " border: 1px solid #314353;"
        " border-radius: 12px;"
        "}";

    targetRpmCard->setStyleSheet(cardStyle);
    currentRpmCard->setStyleSheet(cardStyle);
    dutyCard->setStyleSheet(cardStyle);

    QVBoxLayout *targetRpmLayout =
        new QVBoxLayout(targetRpmCard);

    QVBoxLayout *currentRpmLayout =
        new QVBoxLayout(currentRpmCard);

    QVBoxLayout *dutyLayout =
        new QVBoxLayout(dutyCard);

    QLabel *targetRpmTitle =
        new QLabel("TARGET RPM");

    QLabel *currentRpmTitle =
        new QLabel("CURRENT RPM");

    QLabel *dutyTitle =
        new QLabel("PWM DUTY");

    const QString titleStyle =
        "font-size: 13px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "border: none;"
        "background: transparent;";

    targetRpmTitle->setStyleSheet(titleStyle);
    currentRpmTitle->setStyleSheet(titleStyle);
    dutyTitle->setStyleSheet(titleStyle);

    const QString valueStyle =
        "font-size: 34px;"
        "font-weight: 700;"
        "color: #35CFFF;"
        "border: none;"
        "background: transparent;";

    m_targetRpmLabel->setAlignment(Qt::AlignCenter);
    m_currentRpmLabel->setAlignment(Qt::AlignCenter);
    m_pwmDutyLabel->setAlignment(Qt::AlignCenter);

    m_targetRpmLabel->setStyleSheet(valueStyle);
    m_currentRpmLabel->setStyleSheet(valueStyle);
    m_pwmDutyLabel->setStyleSheet(valueStyle);

    targetRpmLayout->addWidget(targetRpmTitle);
    targetRpmLayout->addStretch();
    targetRpmLayout->addWidget(m_targetRpmLabel);
    targetRpmLayout->addStretch();

    currentRpmLayout->addWidget(currentRpmTitle);
    currentRpmLayout->addStretch();
    currentRpmLayout->addWidget(m_currentRpmLabel);
    currentRpmLayout->addStretch();

    dutyLayout->addWidget(dutyTitle);
    dutyLayout->addStretch();
    dutyLayout->addWidget(m_pwmDutyLabel);
    dutyLayout->addStretch();

    infoLayout->addWidget(targetRpmCard);
    infoLayout->addWidget(currentRpmCard);
    infoLayout->addWidget(dutyCard);

    // ============================================================
    // RPM Graph
    // ============================================================
    m_targetRpmSeries->setName("Target RPM");
    m_currentRpmSeries->setName("Current RPM");

    QPen targetRpmPen(QColor("#E57C23"));
    targetRpmPen.setWidth(3);
    targetRpmPen.setStyle(Qt::DashLine);

    QPen currentRpmPen(QColor("#009FD5"));
    currentRpmPen.setWidth(3);

    m_targetRpmSeries->setPen(targetRpmPen);
    m_currentRpmSeries->setPen(currentRpmPen);

    m_rpmChart->addSeries(m_targetRpmSeries);
    m_rpmChart->addSeries(m_currentRpmSeries);

    m_rpmChart->setTitle("1. Target RPM vs Current RPM");

    QFont chartTitleFont;
    chartTitleFont.setPointSize(13);
    chartTitleFont.setBold(true);

    m_rpmChart->setTitleFont(chartTitleFont);
    m_rpmChart->setTitleBrush(QBrush(QColor("#1B2733")));

    m_rpmChart->setBackgroundVisible(true);
    m_rpmChart->setBackgroundBrush(QBrush(Qt::white));
    m_rpmChart->setBackgroundRoundness(0);

    m_rpmChart->setPlotAreaBackgroundVisible(true);
    m_rpmChart->setPlotAreaBackgroundBrush(QBrush(Qt::white));

    m_rpmChart->legend()->setVisible(true);
    m_rpmChart->legend()->setAlignment(Qt::AlignTop);
    m_rpmChart->legend()->setLabelColor(QColor("#263541"));

    m_rpmAxisX->setTitleText("Time (s)");
    m_rpmAxisX->setLabelFormat("%.0f");
    m_rpmAxisX->setTickCount(7);
    m_rpmAxisX->setRange(0.0, DISPLAY_WINDOW_SEC);

    m_rpmAxisX->setTitleBrush(QBrush(QColor("#1B2733")));
    m_rpmAxisX->setLabelsBrush(QBrush(QColor("#1B2733")));
    m_rpmAxisX->setLabelsColor(QColor("#1B2733"));
    m_rpmAxisX->setGridLineColor(QColor("#D5DDE4"));
    m_rpmAxisX->setLinePenColor(QColor("#4C5A65"));

    m_rpmAxisY->setTitleText("RPM");
    m_rpmAxisY->setLabelFormat("%.0f");
    m_rpmAxisY->setTickCount(6);
    m_rpmAxisY->setRange(0.0, 180.0);

    m_rpmAxisY->setTitleBrush(QBrush(QColor("#1B2733")));
    m_rpmAxisY->setLabelsBrush(QBrush(QColor("#1B2733")));
    m_rpmAxisY->setLabelsColor(QColor("#1B2733"));
    m_rpmAxisY->setGridLineColor(QColor("#D5DDE4"));
    m_rpmAxisY->setLinePenColor(QColor("#4C5A65"));

    m_rpmChart->addAxis(m_rpmAxisX, Qt::AlignBottom);
    m_rpmChart->addAxis(m_rpmAxisY, Qt::AlignLeft);

    m_targetRpmSeries->attachAxis(m_rpmAxisX);
    m_targetRpmSeries->attachAxis(m_rpmAxisY);

    m_currentRpmSeries->attachAxis(m_rpmAxisX);
    m_currentRpmSeries->attachAxis(m_rpmAxisY);

    m_rpmChartView->setRenderHint(QPainter::Antialiasing);
    m_rpmChartView->setMinimumHeight(330);

    m_rpmChartView->setStyleSheet(
        "background-color: white;"
        "border: 1px solid #314353;"
        "border-radius: 12px;"
    );

    // ============================================================
    // PWM Duty Graph
    // ============================================================
    m_dutySeries->setName("PWM Duty");

    QPen dutyPen(QColor("#8B5CF6"));
    dutyPen.setWidth(3);

    m_dutySeries->setPen(dutyPen);

    m_dutyChart->addSeries(m_dutySeries);

    m_dutyChart->setTitle("2. PWM Duty Output");

    m_dutyChart->setTitleFont(chartTitleFont);
    m_dutyChart->setTitleBrush(QBrush(QColor("#1B2733")));

    m_dutyChart->setBackgroundVisible(true);
    m_dutyChart->setBackgroundBrush(QBrush(Qt::white));
    m_dutyChart->setBackgroundRoundness(0);

    m_dutyChart->setPlotAreaBackgroundVisible(true);
    m_dutyChart->setPlotAreaBackgroundBrush(QBrush(Qt::white));

    m_dutyChart->legend()->setVisible(true);
    m_dutyChart->legend()->setAlignment(Qt::AlignTop);
    m_dutyChart->legend()->setLabelColor(QColor("#263541"));

    m_dutyAxisX->setTitleText("Time (s)");
    m_dutyAxisX->setLabelFormat("%.0f");
    m_dutyAxisX->setTickCount(7);
    m_dutyAxisX->setRange(0.0, DISPLAY_WINDOW_SEC);

    m_dutyAxisX->setTitleBrush(QBrush(QColor("#1B2733")));
    m_dutyAxisX->setLabelsBrush(QBrush(QColor("#1B2733")));
    m_dutyAxisX->setLabelsColor(QColor("#1B2733"));
    m_dutyAxisX->setGridLineColor(QColor("#D5DDE4"));
    m_dutyAxisX->setLinePenColor(QColor("#4C5A65"));

    m_dutyAxisY->setTitleText("PWM Duty (%)");
    m_dutyAxisY->setLabelFormat("%.0f");
    m_dutyAxisY->setTickCount(6);
    m_dutyAxisY->setRange(0.0, 100.0);

    m_dutyAxisY->setTitleBrush(QBrush(QColor("#1B2733")));
    m_dutyAxisY->setLabelsBrush(QBrush(QColor("#1B2733")));
    m_dutyAxisY->setLabelsColor(QColor("#1B2733"));
    m_dutyAxisY->setGridLineColor(QColor("#D5DDE4"));
    m_dutyAxisY->setLinePenColor(QColor("#4C5A65"));

    m_dutyChart->addAxis(m_dutyAxisX, Qt::AlignBottom);
    m_dutyChart->addAxis(m_dutyAxisY, Qt::AlignLeft);

    m_dutySeries->attachAxis(m_dutyAxisX);
    m_dutySeries->attachAxis(m_dutyAxisY);

    m_dutyChartView->setRenderHint(QPainter::Antialiasing);
    m_dutyChartView->setMinimumHeight(280);

    m_dutyChartView->setStyleSheet(
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

    /* ============================================================
     * RPM / PWM 그래프 좌우 배치
     * ============================================================ */
    QHBoxLayout *chartLayout = new QHBoxLayout();

    chartLayout->setSpacing(12);

    m_rpmChartView->setMinimumHeight(430);
    m_dutyChartView->setMinimumHeight(430);

    chartLayout->addWidget(m_rpmChartView, 1);
    chartLayout->addWidget(m_dutyChartView, 1);

    rootLayout->addLayout(headerLayout);
    rootLayout->addLayout(statusLayout);
    rootLayout->addLayout(infoLayout);
    rootLayout->addLayout(chartLayout);
    rootLayout->addLayout(buttonLayout);

    setCentralWidget(centralWidget);

    setWindowTitle("PBV Motor Monitor");
    resize(1440, 760);

    setStyleSheet(
        "QMainWindow { background: #0D141B; }"
        "QLabel { color: #EAF6FF; }"
    );

    m_elapsedTimer.start();

    connect(m_telemetryClient,
            &TelemetryClient::telemetryReceived,
            this,
            &MotorMonitorWindow::onTelemetryReceived);

    connect(m_telemetryClient,
            &TelemetryClient::connectionChanged,
            this,
            &MotorMonitorWindow::onConnectionChanged);

    connect(resetButton,
            &QPushButton::clicked,
            this,
            &MotorMonitorWindow::resetRecording);
}

void MotorMonitorWindow::onTelemetryReceived(int moduleType,
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
                                             int fsmState)
{
    Q_UNUSED(moduleType)
    Q_UNUSED(authResult)
    Q_UNUSED(pressure)
    Q_UNUSED(currentTempX10)
    Q_UNUSED(targetTempX10)
    Q_UNUSED(peltierDuty)
    Q_UNUSED(detectState)
    Q_UNUSED(relayState)
    Q_UNUSED(fsmState)

    const double targetRpm =
        static_cast<double>(targetRpmX10) / 10.0;

    const double currentRpm =
        static_cast<double>(currentRpmX10) / 10.0;

    const double pwmDuty =
        static_cast<double>(motorDutyX10) / 10.0;

    const double elapsedSeconds =
        static_cast<double>(m_elapsedTimer.elapsed()) / 1000.0;

    m_targetRpmLabel->setText(
        QString("%1 RPM").arg(targetRpm, 0, 'f', 1)
    );

    m_currentRpmLabel->setText(
        QString("%1 RPM").arg(currentRpm, 0, 'f', 1)
    );

    m_pwmDutyLabel->setText(
        QString("%1 %").arg(pwmDuty, 0, 'f', 1)
    );

    updateMotorState(motorRunning, motorSpeedLevel);

    m_targetRpmSeries->append(elapsedSeconds, targetRpm);
    m_currentRpmSeries->append(elapsedSeconds, currentRpm);
    m_dutySeries->append(elapsedSeconds, pwmDuty);

    removeOldPoints(m_targetRpmSeries, elapsedSeconds);
    removeOldPoints(m_currentRpmSeries, elapsedSeconds);
    removeOldPoints(m_dutySeries, elapsedSeconds);

    if (elapsedSeconds <= DISPLAY_WINDOW_SEC)
    {
        m_rpmAxisX->setRange(0.0, DISPLAY_WINDOW_SEC);
        m_dutyAxisX->setRange(0.0, DISPLAY_WINDOW_SEC);
    }
    else
    {
        m_rpmAxisX->setRange(elapsedSeconds - DISPLAY_WINDOW_SEC,
                             elapsedSeconds);

        m_dutyAxisX->setRange(elapsedSeconds - DISPLAY_WINDOW_SEC,
                              elapsedSeconds);
    }

    updateRpmAxis(targetRpm, currentRpm);
}

void MotorMonitorWindow::onConnectionChanged(bool connected)
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
            "font-weight: 700;"
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
            "font-weight: 700;"
        );
    }
}

void MotorMonitorWindow::resetRecording()
{
    m_targetRpmSeries->clear();
    m_currentRpmSeries->clear();
    m_dutySeries->clear();

    m_elapsedTimer.restart();

    m_rpmAxisX->setRange(0.0, DISPLAY_WINDOW_SEC);
    m_rpmAxisY->setRange(0.0, 180.0);

    m_dutyAxisX->setRange(0.0, DISPLAY_WINDOW_SEC);
    m_dutyAxisY->setRange(0.0, 100.0);

    m_targetRpmLabel->setText("--.- RPM");
    m_currentRpmLabel->setText("--.- RPM");
    m_pwmDutyLabel->setText("--.- %");

    m_motorStateLabel->setText("MOTOR : STOPPED");
    m_speedProfileLabel->setText("PROFILE : --");
}

void MotorMonitorWindow::updateMotorState(int motorRunning,
                                          int motorSpeedLevel)
{
    QString speedProfile;

    switch (motorSpeedLevel)
    {
        case 1:
            speedProfile = "LOW";
            break;

        case 2:
            speedProfile = "NORMAL";
            break;

        case 3:
            speedProfile = "HIGH";
            break;

        default:
            speedProfile = "--";
            break;
    }

    if (motorRunning == 1)
    {
        m_motorStateLabel->setText("MOTOR : RUNNING");

        m_motorStateLabel->setStyleSheet(
            "padding: 8px 14px;"
            "border: 1px solid #37D996;"
            "border-radius: 8px;"
            "color: #BFF7DD;"
            "background: #163126;"
            "font-weight: 700;"
        );
    }
    else
    {
        m_motorStateLabel->setText("MOTOR : STOPPED");

        m_motorStateLabel->setStyleSheet(
            "padding: 8px 14px;"
            "border: 1px solid #E85D5D;"
            "border-radius: 8px;"
            "color: #FFD0D0;"
            "background: #3A1E1E;"
            "font-weight: 700;"
        );
    }

    m_speedProfileLabel->setText(
        QString("PROFILE : %1").arg(speedProfile)
    );
}

void MotorMonitorWindow::updateRpmAxis(double targetRpm,
                                       double currentRpm)
{
    const double maxRpm =
        qMax(targetRpm, currentRpm);

    const double upperBound =
        qMax(180.0, maxRpm + 10.0);

    m_rpmAxisY->setRange(0.0, upperBound);
}

void MotorMonitorWindow::removeOldPoints(QLineSeries *series,
                                         double elapsedSeconds)
{
    while (!series->points().isEmpty() &&
           series->points().first().x() <
               (elapsedSeconds - DISPLAY_WINDOW_SEC))
    {
        series->remove(0);
    }
}