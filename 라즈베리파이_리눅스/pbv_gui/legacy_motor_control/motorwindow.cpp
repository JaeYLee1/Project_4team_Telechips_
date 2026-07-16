#include "motorwindow.h"
#include "motor_client.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

MotorWindow::MotorWindow(QWidget *parent)
    : QMainWindow(parent),
      m_motorClient(new MotorClient(this)),
      m_connectionLabel(new QLabel("UART SERVICE : CONNECTING", this)),
      m_driveStateLabel(new QLabel("VEHICLE STANDBY", this)),
      m_speedLabel(new QLabel("NORMAL", this)),
      m_responseLabel(new QLabel("Waiting for motor command...", this)),
      m_slowButton(new QPushButton("LOW", this)),
      m_normalButton(new QPushButton("NORMAL", this)),
      m_fastButton(new QPushButton("HIGH", this)),
      m_startButton(new QPushButton("START", this)),
      m_stopButton(new QPushButton("STOP", this)),
      m_selectedSpeed(SpeedMode::Normal),
      m_motorRunning(false)
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);

    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(16);

    /* ============================================================
     * Header
     * ============================================================ */
    QLabel *systemTitle =
        new QLabel("PBV DOCKING MODULE CONTROL SYSTEM");

    systemTitle->setStyleSheet(
        "font-size: 22px;"
        "font-weight: 700;"
        "color: #EAF6FF;"
    );

    QLabel *pageTitle =
        new QLabel("DRIVE NAVIGATION / MOTOR CONTROL");

    pageTitle->setStyleSheet(
        "font-size: 14px;"
        "font-weight: 700;"
        "color: #46D8FF;"
    );

    QHBoxLayout *headerLayout = new QHBoxLayout();

    headerLayout->addWidget(systemTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(pageTitle);

    /* ============================================================
     * UART Service Connection
     * ============================================================ */
    m_connectionLabel->setStyleSheet(
        "padding: 9px 15px;"
        "border: 1px solid #384A5A;"
        "border-radius: 8px;"
        "background: #17212B;"
        "color: #AAB8C7;"
        "font-weight: 700;"
    );

    QHBoxLayout *connectionLayout = new QHBoxLayout();

    connectionLayout->addWidget(m_connectionLabel);
    connectionLayout->addStretch();

    /* ============================================================
     * Drive Status Card
     * ============================================================ */
    QFrame *driveCard = new QFrame();

    driveCard->setStyleSheet(
        "QFrame {"
        " background: #17212B;"
        " border: 1px solid #314353;"
        " border-radius: 18px;"
        "}"
    );

    QVBoxLayout *driveLayout = new QVBoxLayout(driveCard);

    driveLayout->setContentsMargins(28, 24, 28, 24);
    driveLayout->setSpacing(10);

    QLabel *directionLabel =
        new QLabel("FORWARD DRIVE MODE");

    directionLabel->setAlignment(Qt::AlignCenter);

    directionLabel->setStyleSheet(
        "font-size: 15px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "border: none;"
        "background: transparent;"
    );

    QLabel *driveIcon = new QLabel("D");

    driveIcon->setAlignment(Qt::AlignCenter);

    driveIcon->setStyleSheet(
        "font-size: 94px;"
        "font-weight: 700;"
        "color: #35CFFF;"
        "border: none;"
        "background: transparent;"
    );

    m_driveStateLabel->setAlignment(Qt::AlignCenter);

    m_driveStateLabel->setStyleSheet(
        "font-size: 24px;"
        "font-weight: 700;"
        "color: #EAF6FF;"
        "border: none;"
        "background: transparent;"
    );

    QLabel *speedTitle =
        new QLabel("SELECTED SPEED PROFILE");

    speedTitle->setAlignment(Qt::AlignCenter);

    speedTitle->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "border: none;"
        "background: transparent;"
    );

    m_speedLabel->setAlignment(Qt::AlignCenter);

    m_speedLabel->setStyleSheet(
        "font-size: 36px;"
        "font-weight: 700;"
        "color: #35CFFF;"
        "border: none;"
        "background: transparent;"
    );

    driveLayout->addWidget(directionLabel);
    driveLayout->addWidget(driveIcon);
    driveLayout->addWidget(m_driveStateLabel);
    driveLayout->addWidget(speedTitle);
    driveLayout->addWidget(m_speedLabel);

    /* ============================================================
     * Speed Select Buttons
     * ============================================================ */
    QLabel *speedSelectTitle =
        new QLabel("SPEED CONTROL");

    speedSelectTitle->setStyleSheet(
        "font-size: 15px;"
        "font-weight: 700;"
        "color: #EAF6FF;"
    );

    QHBoxLayout *speedLayout = new QHBoxLayout();

    speedLayout->setSpacing(14);

    speedLayout->addWidget(m_slowButton);
    speedLayout->addWidget(m_normalButton);
    speedLayout->addWidget(m_fastButton);

    /* ============================================================
     * Start / Stop Buttons
     * ============================================================ */
    QHBoxLayout *commandLayout = new QHBoxLayout();

    commandLayout->setSpacing(16);

    m_startButton->setMinimumHeight(78);
    m_stopButton->setMinimumHeight(78);

    m_startButton->setStyleSheet(
        "QPushButton {"
        " background: #156B45;"
        " color: white;"
        " border: 1px solid #37D996;"
        " border-radius: 12px;"
        " font-size: 24px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover {"
        " background: #1D885B;"
        "}"
    );

    m_stopButton->setStyleSheet(
        "QPushButton {"
        " background: #702A2A;"
        " color: white;"
        " border: 1px solid #E85D5D;"
        " border-radius: 12px;"
        " font-size: 24px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover {"
        " background: #8A3535;"
        "}"
    );

    commandLayout->addWidget(m_startButton);
    commandLayout->addWidget(m_stopButton);

    /* ============================================================
     * STM32 Response Card
     * ============================================================ */
    QFrame *responseCard = new QFrame();

    responseCard->setStyleSheet(
        "QFrame {"
        " background: #111B23;"
        " border: 1px solid #314353;"
        " border-radius: 10px;"
        "}"
    );

    QVBoxLayout *responseLayout =
        new QVBoxLayout(responseCard);

    QLabel *responseTitle =
        new QLabel("STM32 COMMAND RESPONSE");

    responseTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "border: none;"
        "background: transparent;"
    );

    m_responseLabel->setStyleSheet(
        "font-size: 15px;"
        "color: #EAF6FF;"
        "border: none;"
        "background: transparent;"
    );

    responseLayout->addWidget(responseTitle);
    responseLayout->addWidget(m_responseLabel);

    rootLayout->addLayout(headerLayout);
    rootLayout->addLayout(connectionLayout);
    rootLayout->addWidget(driveCard, 1);
    rootLayout->addWidget(speedSelectTitle);
    rootLayout->addLayout(speedLayout);
    rootLayout->addLayout(commandLayout);
    rootLayout->addWidget(responseCard);

    setCentralWidget(centralWidget);

    setWindowTitle("PBV Drive Navigation");
    resize(1280, 760);

    setStyleSheet(
        "QMainWindow { background: #0D141B; }"
        "QLabel { color: #EAF6FF; }"
    );

    connect(m_startButton,
            &QPushButton::clicked,
            this,
            &MotorWindow::startMotor);

    connect(m_stopButton,
            &QPushButton::clicked,
            this,
            &MotorWindow::stopMotor);

    connect(m_slowButton,
            &QPushButton::clicked,
            this,
            &MotorWindow::selectSlow);

    connect(m_normalButton,
            &QPushButton::clicked,
            this,
            &MotorWindow::selectNormal);

    connect(m_fastButton,
            &QPushButton::clicked,
            this,
            &MotorWindow::selectFast);

    connect(m_motorClient,
            &MotorClient::connectionChanged,
            this,
            &MotorWindow::onConnectionChanged);

    connect(m_motorClient,
            &MotorClient::lineReceived,
            this,
            &MotorWindow::onServiceLineReceived);

    connect(m_motorClient,
            &MotorClient::commandFailed,
            this,
            &MotorWindow::onCommandFailed);

    updateSpeedButtons();
}

void MotorWindow::startMotor()
{
    if (!m_motorClient->sendMotorCommand("1"))
    {
        return;
    }

    updateDriveState("STARTING...");
    m_responseLabel->setText("START command transmitted.");

    /*
     * STM32 Motor_RequestStart()는 기본 MID로 시작함.
     * 선택한 속도를 이어서 전송해 실제 선택값을 적용함.
     */
    QTimer::singleShot(
        120,
        this,
        [this]()
        {
            sendSelectedSpeed();
        });
}

void MotorWindow::stopMotor()
{
    if (!m_motorClient->sendMotorCommand("5"))
    {
        return;
    }

    m_motorRunning = false;

    updateDriveState("STOPPING...");
    m_responseLabel->setText("STOP command transmitted.");
}

void MotorWindow::selectSlow()
{
    setSpeedMode(SpeedMode::Slow);
}

void MotorWindow::selectNormal()
{
    setSpeedMode(SpeedMode::Normal);
}

void MotorWindow::selectFast()
{
    setSpeedMode(SpeedMode::Fast);
}

void MotorWindow::setSpeedMode(SpeedMode mode)
{
    m_selectedSpeed = mode;

    m_speedLabel->setText(selectedSpeedText());

    updateSpeedButtons();

    if (m_motorRunning)
    {
        sendSelectedSpeed();
    }
    else
    {
        m_responseLabel->setText(
            QString("%1 selected. Applied after START.")
                .arg(selectedSpeedText())
        );
    }
}

void MotorWindow::sendSelectedSpeed()
{
    const QString command = selectedCommand();

    if (m_motorClient->sendMotorCommand(command))
    {
        m_responseLabel->setText(
            QString("%1 speed command transmitted.")
                .arg(selectedSpeedText())
        );
    }
}

QString MotorWindow::selectedCommand() const
{
    switch (m_selectedSpeed)
    {
        case SpeedMode::Slow:
            return "2";

        case SpeedMode::Normal:
            return "3";

        case SpeedMode::Fast:
            return "4";
    }

    return "3";
}

QString MotorWindow::selectedSpeedText() const
{
    switch (m_selectedSpeed)
    {
        case SpeedMode::Slow:
            return "LOW";

        case SpeedMode::Normal:
            return "NORMAL";

        case SpeedMode::Fast:
            return "HIGH";
    }

    return "NORMAL";
}

void MotorWindow::updateSpeedButtons()
{
    const QString normalStyle =
        "QPushButton {"
        " background: #17212B;"
        " color: #AAB8C7;"
        " border: 1px solid #384A5A;"
        " border-radius: 10px;"
        " min-height: 58px;"
        " font-size: 18px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover {"
        " background: #233442;"
        "}";

    const QString selectedStyle =
        "QPushButton {"
        " background: #1D526B;"
        " color: #EAF6FF;"
        " border: 2px solid #46D8FF;"
        " border-radius: 10px;"
        " min-height: 58px;"
        " font-size: 18px;"
        " font-weight: 700;"
        "}";

    m_slowButton->setStyleSheet(
        m_selectedSpeed == SpeedMode::Slow
            ? selectedStyle
            : normalStyle
    );

    m_normalButton->setStyleSheet(
        m_selectedSpeed == SpeedMode::Normal
            ? selectedStyle
            : normalStyle
    );

    m_fastButton->setStyleSheet(
        m_selectedSpeed == SpeedMode::Fast
            ? selectedStyle
            : normalStyle
    );
}

void MotorWindow::updateDriveState(const QString &state)
{
    m_driveStateLabel->setText(state);
}

void MotorWindow::onConnectionChanged(bool connected)
{
    if (connected)
    {
        m_connectionLabel->setText(
            "UART SERVICE : CONNECTED"
        );

        m_connectionLabel->setStyleSheet(
            "padding: 9px 15px;"
            "border: 1px solid #37D996;"
            "border-radius: 8px;"
            "background: #163126;"
            "color: #BFF7DD;"
            "font-weight: 700;"
        );

        m_responseLabel->setText(
            "UART service connected. Ready for command."
        );
    }
    else
    {
        m_connectionLabel->setText(
            "UART SERVICE : WAITING"
        );

        m_connectionLabel->setStyleSheet(
            "padding: 9px 15px;"
            "border: 1px solid #384A5A;"
            "border-radius: 8px;"
            "background: #17212B;"
            "color: #AAB8C7;"
            "font-weight: 700;"
        );

        m_motorRunning = false;

        updateDriveState("VEHICLE STANDBY");
    }
}

void MotorWindow::onServiceLineReceived(const QString &line)
{
    if (line.startsWith("SVC,CMD_SENT,"))
    {
        return;
    }

    if (line == "OK,START")
    {
        m_motorRunning = true;

        updateDriveState("FORWARD DRIVE ACTIVE");

        m_responseLabel->setText(
            "STM32 ACK : START accepted."
        );

        return;
    }

    if (line == "OK,SLOW")
    {
        m_responseLabel->setText(
            "STM32 ACK : LOW speed selected."
        );

        return;
    }

    if (line == "OK,MID")
    {
        m_responseLabel->setText(
            "STM32 ACK : NORMAL speed selected."
        );

        return;
    }

    if (line == "OK,FAST")
    {
        m_responseLabel->setText(
            "STM32 ACK : HIGH speed selected."
        );

        return;
    }

    if (line == "OK,STOP")
    {
        m_motorRunning = false;

        updateDriveState("VEHICLE STANDBY");

        m_responseLabel->setText(
            "STM32 ACK : Motor stopped."
        );

        return;
    }

    if (line.startsWith("ERR,"))
    {
        m_responseLabel->setText(
            QString("STM32 ERROR : %1").arg(line)
        );
    }
}

void MotorWindow::onCommandFailed(const QString &reason)
{
    m_responseLabel->setText(
        QString("COMMAND FAILED : %1").arg(reason)
    );
}