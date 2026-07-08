#include "module_a_page.h"

#include <algorithm>
#include <cmath>

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocalSocket>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

static const char *UART_SOCKET_PATH = "/tmp/pbv_uart.sock";

ModuleAPage::ModuleAPage(QWidget *parent)
    : QWidget(parent),
      m_socket(new QLocalSocket(this)),
      m_moduleLabel(createStatusChip("MODULE : A CONNECTED")),
      m_authLabel(createStatusChip("AUTH : ACCEPTED")),
      m_relayLabel(createStatusChip("RELAY : ON")),
      m_dockLabel(createStatusChip("DOCK : LOCKED")),
      m_systemLabel(createStatusChip("SYSTEM : OPERATIONAL")),
      m_cargoPercentLabel(new QLabel("0%", this)),
      m_cargoStateLabel(new QLabel("LOADED", this)),
      m_weightLabel(new QLabel("0 g", this)),
      m_driveStateLabel(new QLabel("STANDBY", this)),
      m_speedProfileLabel(new QLabel("MID", this)),
      m_targetRpmLabel(new QLabel("--.- RPM", this)),
      m_currentRpmLabel(new QLabel("--.- RPM", this)),
      m_dutyLabel(new QLabel("--.- %", this)),
      m_cargoBar(new QProgressBar(this)),
      m_lowButton(createNavButton("LOW")),
      m_midButton(createNavButton("MID")),
      m_highButton(createNavButton("HIGH")),
      m_startButton(createNavButton("DRIVE START")),
      m_stopButton(createNavButton("DRIVE STOP")),
      m_unmountButton(createNavButton("SAFE RELEASE MODULE"))
{
    setObjectName("ModuleAPageRoot");
    setAttribute(Qt::WA_StyledBackground, true);

    setStyleSheet(
        "#ModuleAPageRoot {"
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

    QLabel *mode = new QLabel("MODULE A", this);
    mode->setStyleSheet(
        "font-size: 16px;"
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

    QFrame *modulePanel = createPanel();
    QVBoxLayout *moduleLayout = new QVBoxLayout(modulePanel);
    moduleLayout->setContentsMargins(20, 20, 20, 20);
    moduleLayout->setSpacing(18);

    QLabel *moduleTitle = new QLabel("MODULE INFORMATION", this);
    moduleTitle->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 900;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    QLabel *cargoTitle = new QLabel("CARGO LEVEL MONITOR", this);
    cargoTitle->setStyleSheet(
        "font-size: 18px;"
        "font-weight: 900;"
        "color: #EAF6FF;"
        "background: transparent;"
        "border: none;"
    );

    m_cargoPercentLabel->setAlignment(Qt::AlignCenter);
    m_cargoPercentLabel->setStyleSheet(
        "font-size: 72px;"
        "font-weight: 900;"
        "color: #20C8FF;"
        "background: transparent;"
        "border: none;"
    );

    m_cargoStateLabel->setAlignment(Qt::AlignCenter);
    m_cargoStateLabel->setStyleSheet(
        "font-size: 18px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    m_cargoBar->setRange(0, 100);
    m_cargoBar->setValue(0);
    m_cargoBar->setTextVisible(false);
    m_cargoBar->setMinimumHeight(20);
    m_cargoBar->setStyleSheet(
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

    QFrame *cargoInfoCard = new QFrame(this);
    cargoInfoCard->setStyleSheet(
        "QFrame {"
        " background: #101F2E;"
        " border: 1px solid #2C465D;"
        " border-radius: 18px;"
        "}"
    );

    QVBoxLayout *cargoInfoLayout = new QVBoxLayout(cargoInfoCard);
    cargoInfoLayout->setContentsMargins(18, 18, 18, 18);
    cargoInfoLayout->setSpacing(8);

    QLabel *weightTitle = new QLabel("CURRENT CARGO WEIGHT", this);
    weightTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    m_weightLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: 900;"
        "color: #EAF6FF;"
        "background: transparent;"
        "border: none;"
    );

    cargoInfoLayout->addWidget(weightTitle);
    cargoInfoLayout->addWidget(m_weightLabel);

    QLabel *moduleDesc = new QLabel(
        "General cargo module is authenticated and powered.\n"
        "Cargo load is monitored in real time through the base controller.",
        this
    );

    moduleDesc->setStyleSheet(
        "font-size: 13px;"
        "line-height: 150%;"
        "color: #AAB8C7;"
        "background: transparent;"
        "border: none;"
    );

    moduleLayout->addWidget(moduleTitle);
    moduleLayout->addWidget(cargoTitle);
    moduleLayout->addStretch();
    moduleLayout->addWidget(m_cargoPercentLabel);
    moduleLayout->addWidget(m_cargoStateLabel);
    moduleLayout->addSpacing(12);
    moduleLayout->addWidget(m_cargoBar);
    moduleLayout->addSpacing(14);
    moduleLayout->addWidget(cargoInfoCard);
    moduleLayout->addWidget(moduleDesc);
    moduleLayout->addStretch();

    // ============================================================
    // Right side control area
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
        "font-size: 42px;"
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

    QLabel *targetTitle = new QLabel("TARGET", this);
    QLabel *currentTitle = new QLabel("CURRENT", this);
    QLabel *dutyTitle = new QLabel("PWM", this);

    const QString smallTitleStyle =
        "font-size: 11px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;";

    targetTitle->setStyleSheet(smallTitleStyle);
    currentTitle->setStyleSheet(smallTitleStyle);
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

    QVBoxLayout *targetBox = new QVBoxLayout();
    QVBoxLayout *currentBox = new QVBoxLayout();
    QVBoxLayout *dutyBox = new QVBoxLayout();

    targetBox->addWidget(targetTitle);
    targetBox->addWidget(m_targetRpmLabel);

    currentBox->addWidget(currentTitle);
    currentBox->addWidget(m_currentRpmLabel);

    dutyBox->addWidget(dutyTitle);
    dutyBox->addWidget(m_dutyLabel);

    rpmRow->addLayout(targetBox);
    rpmRow->addStretch();
    rpmRow->addLayout(currentBox);
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

    mainArea->addWidget(modulePanel, 3);
    mainArea->addLayout(rightColumn, 2);

    root->addLayout(topBar);
    root->addLayout(chipRow);
    root->addLayout(mainArea, 1);

    // Initial
    updateDriveProfile(2);
    setCargoStatus(0);

    // Socket
    m_socket->connectToServer(UART_SOCKET_PATH);

    connect(m_socket,
            &QLocalSocket::disconnected,
            this,
            [this]() {
                m_socket->abort();
                m_socket->connectToServer(UART_SOCKET_PATH);
            });

    // Buttons
    connect(m_startButton, &QPushButton::clicked,
            this, &ModuleAPage::driveStart);

    connect(m_stopButton, &QPushButton::clicked,
            this, &ModuleAPage::driveStop);

    connect(m_lowButton, &QPushButton::clicked,
            this, &ModuleAPage::selectLow);

    connect(m_midButton, &QPushButton::clicked,
            this, &ModuleAPage::selectMid);

    connect(m_highButton, &QPushButton::clicked,
            this, &ModuleAPage::selectHigh);

    connect(m_unmountButton, &QPushButton::clicked,
            this, &ModuleAPage::unmountModule);
}

QLabel *ModuleAPage::createStatusChip(const QString &text)
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

QPushButton *ModuleAPage::createNavButton(const QString &text)
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

QFrame *ModuleAPage::createPanel()
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

void ModuleAPage::setServiceConnected(bool connected)
{
    if (!connected)
    {
        m_systemLabel->setText("SYSTEM : COMM WAITING");
    }
}

void ModuleAPage::setSystemStatus(int moduleType,
                                  int authResult,
                                  int detectState,
                                  int relayState,
                                  int fsmState)
{
    if (moduleType == 1)
    {
        m_moduleLabel->setText("MODULE : A CONNECTED");
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

    if ((moduleType == 1) &&
        (authResult == 1) &&
        (detectState == 1) &&
        (relayState == 1) &&
        (fsmState == 3))
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
    }
}

void ModuleAPage::setCargoStatus(int weightG)
{
    int percent = static_cast<int>(
        std::round(static_cast<double>(weightG) * 100.0 / 1000.0)
    );

    percent = std::max(0, std::min(percent, 100));

    m_cargoPercentLabel->setText(QString("%1%").arg(percent));
    m_cargoBar->setValue(percent);
    m_weightLabel->setText(QString("%1 g").arg(weightG));

    if (percent >= 90)
    {
        m_cargoStateLabel->setText("NEAR LIMIT");
    }
    else if (percent >= 10)
    {
        m_cargoStateLabel->setText("LOADED");
    }
    else
    {
        m_cargoStateLabel->setText("EMPTY");
    }
}

void ModuleAPage::setMotorStatus(int motorRunning,
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

void ModuleAPage::updateDriveProfile(int level)
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

void ModuleAPage::updateSpeedButtonStyle(int selectedLevel)
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

void ModuleAPage::sendCommand(const QString &command)
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

void ModuleAPage::driveStart()
{
    sendCommand("1");
}

void ModuleAPage::driveStop()
{
    sendCommand("5");
}

void ModuleAPage::selectLow()
{
    updateDriveProfile(1);
    sendCommand("2");
}

void ModuleAPage::selectMid()
{
    updateDriveProfile(2);
    sendCommand("3");
}

void ModuleAPage::selectHigh()
{
    updateDriveProfile(3);
    sendCommand("4");
}

void ModuleAPage::unmountModule()
{
    m_systemLabel->setText("SYSTEM : RELEASE REQUESTED");
    sendCommand("d");
}