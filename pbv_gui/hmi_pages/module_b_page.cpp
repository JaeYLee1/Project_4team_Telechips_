#include "module_b_page.h"

#include <cmath>

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocalSocket>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

static const char *UART_SOCKET_PATH = "/tmp/pbv_uart.sock";

ModuleBPage::ModuleBPage(QWidget *parent)
    : QWidget(parent),
      m_socket(new QLocalSocket(this)),
      m_moduleLabel(createStatusChip("MODULE : B CONNECTED")),
      m_authLabel(createStatusChip("AUTH : ACCEPTED")),
      m_relayLabel(createStatusChip("RELAY : ON")),
      m_dockLabel(createStatusChip("DOCK : LOCKED")),
      m_systemLabel(createStatusChip("SYSTEM : OPERATIONAL")),
      m_currentTempLabel(new QLabel("--.- °C", this)),
      m_targetTempLabel(new QLabel("--.- °C", this)),
      m_peltierLabel(new QLabel("0 %", this)),
      m_tempStateLabel(new QLabel("COOLING READY", this)),
      m_driveStateLabel(new QLabel("STANDBY", this)),
      m_speedProfileLabel(new QLabel("MID", this)),
      m_targetRpmLabel(new QLabel("--.- RPM", this)),
      m_currentRpmLabel(new QLabel("--.- RPM", this)),
      m_dutyLabel(new QLabel("--.- %", this)),
      m_peltierBar(new QProgressBar(this)),
      m_tempDownButton(createNavButton("- 1°C")),
      m_tempUpButton(createNavButton("+ 1°C")),
      m_lowButton(createNavButton("LOW")),
      m_midButton(createNavButton("MID")),
      m_highButton(createNavButton("HIGH")),
      m_startButton(createNavButton("DRIVE START")),
      m_stopButton(createNavButton("DRIVE STOP")),
      m_unmountButton(createNavButton("SAFE RELEASE MODULE")),
      m_moduleActive(false),
      m_targetInitialized(false),
      m_targetTempC(0)
{
    setObjectName("ModuleBPageRoot");
    setAttribute(Qt::WA_StyledBackground, true);

    setStyleSheet(
        "#ModuleBPageRoot {"
        " background: #050B12;"
        " color: #EAF6FF;"
        " font-family: Arial;"
        "}"
    );

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 18, 22, 18);
    root->setSpacing(14);

    // ============================================================
    // Top bar
    // ============================================================
    QHBoxLayout *topBar = new QHBoxLayout();

    QLabel *title = new QLabel("PBV DOCKING SYSTEM", this);
    title->setStyleSheet(
        "font-size: 24px;"
        "font-weight: 900;"
        "letter-spacing: 1px;"
        "color: #EAF6FF;"
        "background: transparent;"
    );

    QLabel *mode = new QLabel("MODULE B  |  TEMPERATURE CONTROL", this);
    mode->setStyleSheet(
        "font-size: 15px;"
        "font-weight: 900;"
        "color: #46D8FF;"
        "padding: 8px 18px;"
        "border: 1px solid #29506A;"
        "border-radius: 16px;"
        "background: #0B1A26;"
    );

    topBar->addWidget(title);
    topBar->addStretch();
    topBar->addWidget(mode);

    // ============================================================
    // Status chips
    // ============================================================
    QHBoxLayout *chipRow = new QHBoxLayout();
    chipRow->setSpacing(12);

    chipRow->addWidget(m_moduleLabel);
    chipRow->addWidget(m_authLabel);
    chipRow->addWidget(m_relayLabel);
    chipRow->addWidget(m_dockLabel);
    chipRow->addWidget(m_systemLabel);
    chipRow->addStretch();

    // ============================================================
    // Main area
    // ============================================================
    QHBoxLayout *mainArea = new QHBoxLayout();
    mainArea->setSpacing(16);

    // ============================================================
    // Temperature panel
    // ============================================================
    QFrame *tempPanel = createPanel();
    QVBoxLayout *tempLayout = new QVBoxLayout(tempPanel);
    tempLayout->setContentsMargins(20, 20, 20, 20);
    tempLayout->setSpacing(18);

    QLabel *tempTitle = new QLabel("REFRIGERATION MODULE", this);
    tempTitle->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 900;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    QLabel *mainTempTitle = new QLabel("TEMPERATURE CONTROL", this);
    mainTempTitle->setStyleSheet(
        "font-size: 20px;"
        "font-weight: 900;"
        "color: #EAF6FF;"
        "background: transparent;"
        "border: none;"
    );

    QHBoxLayout *tempValueRow = new QHBoxLayout();
    tempValueRow->setSpacing(16);

    QFrame *currentCard = new QFrame(this);
    currentCard->setStyleSheet(
        "QFrame {"
        " background: #101F2E;"
        " border: 1px solid #2C465D;"
        " border-radius: 18px;"
        "}"
    );

    QVBoxLayout *currentLayout = new QVBoxLayout(currentCard);
    currentLayout->setContentsMargins(18, 18, 18, 18);
    currentLayout->setSpacing(8);

    QLabel *currentTitle = new QLabel("CURRENT TEMP", this);
    currentTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    m_currentTempLabel->setStyleSheet(
        "font-size: 48px;"
        "font-weight: 900;"
        "color: #46D8FF;"
        "background: transparent;"
        "border: none;"
    );

    currentLayout->addWidget(currentTitle);
    currentLayout->addWidget(m_currentTempLabel);

    QFrame *targetCard = new QFrame(this);
    targetCard->setStyleSheet(
        "QFrame {"
        " background: #101F2E;"
        " border: 1px solid #2C465D;"
        " border-radius: 18px;"
        "}"
    );

    QVBoxLayout *targetLayout = new QVBoxLayout(targetCard);
    targetLayout->setContentsMargins(18, 18, 18, 18);
    targetLayout->setSpacing(8);

    QLabel *targetTitle = new QLabel("TARGET TEMP", this);
    targetTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    m_targetTempLabel->setStyleSheet(
        "font-size: 48px;"
        "font-weight: 900;"
        "color: #20E0B5;"
        "background: transparent;"
        "border: none;"
    );

    QHBoxLayout *targetButtonRow = new QHBoxLayout();
    targetButtonRow->addWidget(m_tempDownButton);
    targetButtonRow->addWidget(m_tempUpButton);

    targetLayout->addWidget(targetTitle);
    targetLayout->addWidget(m_targetTempLabel);
    targetLayout->addLayout(targetButtonRow);

    tempValueRow->addWidget(currentCard);
    tempValueRow->addWidget(targetCard);

    QFrame *coolingCard = new QFrame(this);
    coolingCard->setStyleSheet(
        "QFrame {"
        " background: #101F2E;"
        " border: 1px solid #2C465D;"
        " border-radius: 18px;"
        "}"
    );

    QVBoxLayout *coolingLayout = new QVBoxLayout(coolingCard);
    coolingLayout->setContentsMargins(18, 18, 18, 18);
    coolingLayout->setSpacing(10);

    QHBoxLayout *coolingHeaderRow = new QHBoxLayout();

    QLabel *coolingTitle = new QLabel("PELTIER OUTPUT", this);
    coolingTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    m_peltierLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_peltierLabel->setStyleSheet(
        "font-size: 22px;"
        "font-weight: 900;"
        "color: #EAF6FF;"
        "background: transparent;"
        "border: none;"
    );

    coolingHeaderRow->addWidget(coolingTitle);
    coolingHeaderRow->addStretch();
    coolingHeaderRow->addWidget(m_peltierLabel);

    m_peltierBar->setRange(0, 100);
    m_peltierBar->setValue(0);
    m_peltierBar->setTextVisible(false);
    m_peltierBar->setMinimumHeight(20);
    m_peltierBar->setStyleSheet(
        "QProgressBar {"
        " background: #0B1420;"
        " border: 1px solid #2C465D;"
        " border-radius: 10px;"
        "}"
        "QProgressBar::chunk {"
        " background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        " stop:0 #18E0B5,"
        " stop:1 #20C8FF);"
        " border-radius: 10px;"
        "}"
    );

    m_tempStateLabel->setStyleSheet(
        "font-size: 14px;"
        "font-weight: 900;"
        "color: #AAB8C7;"
        "background: transparent;"
        "border: none;"
    );

    QLabel *tempDesc = new QLabel(
        "Target temperature is initialized from the current temperature.\n"
        "Use + / - buttons to adjust the target by 1°C.",
        this
    );

    tempDesc->setStyleSheet(
        "font-size: 13px;"
        "line-height: 150%;"
        "color: #AAB8C7;"
        "background: transparent;"
        "border: none;"
    );

    coolingLayout->addLayout(coolingHeaderRow);
    coolingLayout->addWidget(m_peltierBar);
    coolingLayout->addWidget(m_tempStateLabel);

    tempLayout->addWidget(tempTitle);
    tempLayout->addWidget(mainTempTitle);
    tempLayout->addLayout(tempValueRow);
    tempLayout->addWidget(coolingCard);
    tempLayout->addWidget(tempDesc);
    tempLayout->addStretch();

    // ============================================================
    // Right column: drive + release
    // ============================================================
    QVBoxLayout *rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(16);

    QFrame *drivePanel = createPanel();
    QVBoxLayout *driveLayout = new QVBoxLayout(drivePanel);
    driveLayout->setContentsMargins(20, 20, 20, 20);
    driveLayout->setSpacing(14);

    QLabel *driveTitle = new QLabel("DRIVE CONTROL", this);
    driveTitle->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 900;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    m_driveStateLabel->setAlignment(Qt::AlignCenter);
    m_driveStateLabel->setStyleSheet(
        "font-size: 38px;"
        "font-weight: 900;"
        "color: #46D8FF;"
        "background: transparent;"
        "border: none;"
    );

    QHBoxLayout *modeRow = new QHBoxLayout();

    QLabel *speedTitle = new QLabel("DRIVE SPEED PROFILE", this);
    speedTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    m_speedProfileLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_speedProfileLabel->setStyleSheet(
        "font-size: 18px;"
        "font-weight: 900;"
        "color: #46D8FF;"
        "background: transparent;"
        "border: none;"
    );

    modeRow->addWidget(speedTitle);
    modeRow->addStretch();
    modeRow->addWidget(m_speedProfileLabel);

    QHBoxLayout *speedButtonRow = new QHBoxLayout();
    speedButtonRow->addWidget(m_lowButton);
    speedButtonRow->addWidget(m_midButton);
    speedButtonRow->addWidget(m_highButton);

    QHBoxLayout *rpmRow = new QHBoxLayout();

    QLabel *targetRpmTitle = new QLabel("TARGET", this);
    QLabel *currentRpmTitle = new QLabel("CURRENT", this);
    QLabel *dutyTitle = new QLabel("PWM", this);

    const QString smallTitleStyle =
        "font-size: 11px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;";

    targetRpmTitle->setStyleSheet(smallTitleStyle);
    currentRpmTitle->setStyleSheet(smallTitleStyle);
    dutyTitle->setStyleSheet(smallTitleStyle);

    const QString valueStyle =
        "font-size: 17px;"
        "font-weight: 900;"
        "color: #EAF6FF;"
        "background: transparent;"
        "border: none;";

    m_targetRpmLabel->setStyleSheet(valueStyle);
    m_currentRpmLabel->setStyleSheet(valueStyle);
    m_dutyLabel->setStyleSheet(valueStyle);

    QVBoxLayout *targetRpmBox = new QVBoxLayout();
    QVBoxLayout *currentRpmBox = new QVBoxLayout();
    QVBoxLayout *dutyBox = new QVBoxLayout();

    targetRpmBox->addWidget(targetRpmTitle);
    targetRpmBox->addWidget(m_targetRpmLabel);

    currentRpmBox->addWidget(currentRpmTitle);
    currentRpmBox->addWidget(m_currentRpmLabel);

    dutyBox->addWidget(dutyTitle);
    dutyBox->addWidget(m_dutyLabel);

    rpmRow->addLayout(targetRpmBox);
    rpmRow->addStretch();
    rpmRow->addLayout(currentRpmBox);
    rpmRow->addStretch();
    rpmRow->addLayout(dutyBox);

    QHBoxLayout *driveButtonRow = new QHBoxLayout();
    driveButtonRow->addWidget(m_startButton);
    driveButtonRow->addWidget(m_stopButton);

    driveLayout->addWidget(driveTitle);
    driveLayout->addWidget(m_driveStateLabel);
    driveLayout->addLayout(modeRow);
    driveLayout->addLayout(speedButtonRow);
    driveLayout->addSpacing(10);
    driveLayout->addLayout(rpmRow);
    driveLayout->addSpacing(14);
    driveLayout->addLayout(driveButtonRow);

    QFrame *releasePanel = createPanel();
    QVBoxLayout *releaseLayout = new QVBoxLayout(releasePanel);
    releaseLayout->setContentsMargins(20, 20, 20, 20);
    releaseLayout->setSpacing(14);

    QLabel *releaseTitle = new QLabel("MODULE RELEASE / UNDOCK", this);
    releaseTitle->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 900;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    QLabel *releaseDesc = new QLabel(
        "Before physically separating the connector,\n"
        "disconnect module power and communication safely.",
        this
    );

    releaseDesc->setStyleSheet(
        "font-size: 13px;"
        "line-height: 150%;"
        "color: #AAB8C7;"
        "background: transparent;"
        "border: none;"
    );

    m_unmountButton->setMinimumHeight(58);
    m_unmountButton->setStyleSheet(
        "QPushButton {"
        " background: #241018;"
        " color: #FF5A5A;"
        " border: 1px solid #FF3B3B;"
        " border-radius: 16px;"
        " padding: 14px 18px;"
        " font-size: 15px;"
        " font-weight: 900;"
        "}"
        "QPushButton:hover {"
        " background: #38151E;"
        " border: 1px solid #FF7777;"
        "}"
        "QPushButton:disabled {"
        " background: #151F29;"
        " color: #596977;"
        " border: 1px solid #263542;"
        "}"
    );

    releaseLayout->addWidget(releaseTitle);
    releaseLayout->addWidget(releaseDesc);
    releaseLayout->addStretch();
    releaseLayout->addWidget(m_unmountButton);

    rightColumn->addWidget(drivePanel, 2);
    rightColumn->addWidget(releasePanel, 1);

    mainArea->addWidget(tempPanel, 3);
    mainArea->addLayout(rightColumn, 2);

    root->addLayout(topBar);
    root->addLayout(chipRow);
    root->addLayout(mainArea, 1);

    // Initial
    updateDriveProfile(2);

    // Socket
    m_socket->connectToServer(UART_SOCKET_PATH);

    connect(m_socket,
            &QLocalSocket::disconnected,
            this,
            [this]() {
                m_socket->abort();
                m_socket->connectToServer(UART_SOCKET_PATH);
            });

    connect(m_tempDownButton, &QPushButton::clicked,
            this, &ModuleBPage::decreaseTargetTemp);

    connect(m_tempUpButton, &QPushButton::clicked,
            this, &ModuleBPage::increaseTargetTemp);

    connect(m_startButton, &QPushButton::clicked,
            this, &ModuleBPage::driveStart);

    connect(m_stopButton, &QPushButton::clicked,
            this, &ModuleBPage::driveStop);

    connect(m_lowButton, &QPushButton::clicked,
            this, &ModuleBPage::selectLow);

    connect(m_midButton, &QPushButton::clicked,
            this, &ModuleBPage::selectMid);

    connect(m_highButton, &QPushButton::clicked,
            this, &ModuleBPage::selectHigh);

    connect(m_unmountButton, &QPushButton::clicked,
            this, &ModuleBPage::unmountModule);
}

QLabel *ModuleBPage::createStatusChip(const QString &text)
{
    QLabel *label = new QLabel(text, this);

    label->setStyleSheet(
        "padding: 8px 14px;"
        "border: 1px solid #263D52;"
        "border-radius: 14px;"
        "background: #0B1A26;"
        "color: #BFF7DD;"
        "font-size: 12px;"
        "font-weight: 900;"
    );

    return label;
}

QPushButton *ModuleBPage::createNavButton(const QString &text)
{
    QPushButton *button = new QPushButton(text, this);

    button->setCursor(Qt::PointingHandCursor);

    button->setStyleSheet(
        "QPushButton {"
        " background: #173047;"
        " color: #EAF6FF;"
        " border: 1px solid #2F5F7A;"
        " border-radius: 14px;"
        " padding: 12px 16px;"
        " font-size: 13px;"
        " font-weight: 900;"
        "}"
        "QPushButton:hover {"
        " background: #21506D;"
        " border: 1px solid #46D8FF;"
        "}"
        "QPushButton:disabled {"
        " background: #151F29;"
        " color: #596977;"
        " border: 1px solid #263542;"
        "}"
    );

    return button;
}

QFrame *ModuleBPage::createPanel()
{
    QFrame *panel = new QFrame(this);

    panel->setStyleSheet(
        "QFrame {"
        " background: #0B1622;"
        " border: 1px solid #1D3346;"
        " border-radius: 22px;"
        "}"
    );

    return panel;
}

void ModuleBPage::setServiceConnected(bool connected)
{
    if (!connected)
    {
        m_systemLabel->setText("SYSTEM : COMM WAITING");
    }
}

void ModuleBPage::setSystemStatus(int moduleType,
                                  int authResult,
                                  int detectState,
                                  int relayState,
                                  int fsmState)
{
    if (moduleType == 2)
    {
        m_moduleLabel->setText("MODULE : B CONNECTED");
    }
    else
    {
        m_moduleLabel->setText("MODULE : UNKNOWN");
    }

    if (authResult == 1)
    {
        m_authLabel->setText("AUTH : ACCEPTED");
    }
    else if (authResult == 2)
    {
        m_authLabel->setText("AUTH : REJECTED");
    }
    else
    {
        m_authLabel->setText("AUTH : WAITING");
    }

    m_relayLabel->setText(
        relayState == 1 ? "RELAY : ON" : "RELAY : OFF"
    );

    m_dockLabel->setText(
        detectState == 1 ? "DOCK : LOCKED" : "DOCK : EMPTY"
    );

    const bool active =
        (moduleType == 2) &&
        (authResult == 1) &&
        (detectState == 1) &&
        (relayState == 1) &&
        (fsmState == 3);

    m_moduleActive = active;

    if (active)
    {
        m_systemLabel->setText("SYSTEM : OPERATIONAL");
    }
    else if (fsmState == 4)
    {
        m_systemLabel->setText("SYSTEM : FAULT");
    }
    else
    {
        m_systemLabel->setText("SYSTEM : STANDBY");

        /*
        * 모듈 B가 해제되거나 IDLE로 돌아가면
        * 다음 장착 시 현재온도 기준으로 목표온도를 다시 초기화하기 위함
        */
        m_targetInitialized = false;
    }
}

void ModuleBPage::setTemperatureStatus(int currentTempX10,
                                       int targetTempX10,
                                       int peltierDuty)
{
    Q_UNUSED(targetTempX10)

    const double currentTemp = currentTempX10 / 10.0;

    m_currentTempLabel->setText(
        QString("%1 °C").arg(currentTemp, 0, 'f', 1)
    );

    /*
     * Module B ACTIVE 상태에서 최초 1회만 목표온도 초기화
     *
     * 예)
     * currentTemp = 25.8
     * floor(25.8) = 25
     * target = 25.0°C
     *
     * 그리고 즉시 Base STM32로 목표온도 전송
     */
    if (m_moduleActive && !m_targetInitialized)
    {
        m_targetTempC = static_cast<int>(std::floor(currentTemp));
        m_targetInitialized = true;

        updateTargetTempView();
        sendTargetTempToBase();
    }

    int duty = peltierDuty;

    if (duty < 0)
    {
        duty = 0;
    }

    if (duty > 100)
    {
        duty = 100;
    }

    m_peltierLabel->setText(QString("%1 %").arg(duty));
    m_peltierBar->setValue(duty);

    if (m_targetInitialized &&
        currentTemp > static_cast<double>(m_targetTempC) + 3.0)
    {
        m_tempStateLabel->setText("COOLING ACTIVE");
    }
    else
    {
        m_tempStateLabel->setText("TEMPERATURE STABLE");
    }
}

void ModuleBPage::updateTargetTempView()
{
    m_targetTempLabel->setText(
        QString("%1.0 °C").arg(m_targetTempC)
    );
}

void ModuleBPage::increaseTargetTemp()
{
    if (!m_targetInitialized)
    {
        return;
    }

    m_targetTempC += 1;

    updateTargetTempView();
    sendTargetTempToBase();
}

void ModuleBPage::decreaseTargetTemp()
{
    if (!m_targetInitialized)
    {
        return;
    }

    m_targetTempC -= 1;

    updateTargetTempView();
    sendTargetTempToBase();
}

void ModuleBPage::setMotorStatus(int motorRunning,
                                 int motorSpeedLevel,
                                 double targetRpm,
                                 double currentRpm,
                                 double duty)
{
    const bool running = (motorRunning == 1);

    m_driveStateLabel->setText(running ? "DRIVING" : "STANDBY");

    m_targetRpmLabel->setText(
        QString("%1 RPM").arg(targetRpm, 0, 'f', 1)
    );

    m_currentRpmLabel->setText(
        QString("%1 RPM").arg(currentRpm, 0, 'f', 1)
    );

    m_dutyLabel->setText(
        QString("%1 %").arg(duty, 0, 'f', 1)
    );

    updateDriveProfile(motorSpeedLevel);
}

void ModuleBPage::updateDriveProfile(int level)
{
    QString profile;

    switch (level)
    {
        case 1:
            profile = "LOW";
            break;

        case 2:
            profile = "MID";
            break;

        case 3:
            profile = "HIGH";
            break;

        default:
            profile = "--";
            break;
    }

    m_speedProfileLabel->setText(profile);
    updateSpeedButtonStyle(level);
}

void ModuleBPage::updateSpeedButtonStyle(int selectedLevel)
{
    const QString normalStyle =
        "QPushButton {"
        " background: #173047;"
        " color: #EAF6FF;"
        " border: 1px solid #2F5F7A;"
        " border-radius: 14px;"
        " padding: 12px 16px;"
        " font-size: 13px;"
        " font-weight: 900;"
        "}"
        "QPushButton:hover {"
        " background: #21506D;"
        " border: 1px solid #46D8FF;"
        "}";

    const QString selectedStyle =
        "QPushButton {"
        " background: #098EE8;"
        " color: #FFFFFF;"
        " border: 1px solid #46D8FF;"
        " border-radius: 14px;"
        " padding: 12px 16px;"
        " font-size: 13px;"
        " font-weight: 900;"
        "}";

    m_lowButton->setStyleSheet(selectedLevel == 1 ? selectedStyle : normalStyle);
    m_midButton->setStyleSheet(selectedLevel == 2 ? selectedStyle : normalStyle);
    m_highButton->setStyleSheet(selectedLevel == 3 ? selectedStyle : normalStyle);
}

void ModuleBPage::sendCommand(const QString &command)
{
    if (command.length() != 1)
    {
        return;
    }

    if (m_socket->state() != QLocalSocket::ConnectedState)
    {
        m_socket->abort();
        m_socket->connectToServer(UART_SOCKET_PATH);
        m_socket->waitForConnected(100);
    }

    if (m_socket->state() == QLocalSocket::ConnectedState)
    {
        const QByteArray packet =
            QString("CMD,%1\n").arg(command).toUtf8();

        m_socket->write(packet);
        m_socket->flush();
    }
}

void ModuleBPage::sendTargetTempToBase()
{
    if (!m_targetInitialized)
    {
        return;
    }

    if (m_socket->state() != QLocalSocket::ConnectedState)
    {
        m_socket->abort();
        m_socket->connectToServer(UART_SOCKET_PATH);
        m_socket->waitForConnected(100);
    }

    if (m_socket->state() == QLocalSocket::ConnectedState)
    {
        /*
         * GUI -> uart_service
         *
         * 예)
         * 목표온도 25°C
         * CMD,T,25
         */
        const QByteArray packet =
            QString("CMD,T,%1\n").arg(m_targetTempC).toUtf8();

        m_socket->write(packet);
        m_socket->flush();
    }
}

void ModuleBPage::driveStart()
{
    sendCommand("1");
}

void ModuleBPage::driveStop()
{
    sendCommand("5");
}

void ModuleBPage::selectLow()
{
    updateDriveProfile(1);
    sendCommand("2");
}

void ModuleBPage::selectMid()
{
    updateDriveProfile(2);
    sendCommand("3");
}

void ModuleBPage::selectHigh()
{
    updateDriveProfile(3);
    sendCommand("4");
}

void ModuleBPage::unmountModule()
{
    m_systemLabel->setText("SYSTEM : RELEASE REQUESTED");
    sendCommand("d");
}