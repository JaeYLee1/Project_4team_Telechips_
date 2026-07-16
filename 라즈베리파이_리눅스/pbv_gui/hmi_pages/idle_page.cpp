#include "idle_page.h"

#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocalSocket>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

static const char *UART_SOCKET_PATH = "/tmp/pbv_uart.sock";

VehicleView::VehicleView(QWidget *parent)
    : QWidget(parent),
      m_dockDetected(false),
      m_relayOn(false),
      m_motorRunning(false)
{
    setMinimumSize(520, 300);
}

void VehicleView::setDockDetected(bool detected)
{
    m_dockDetected = detected;
    update();
}

void VehicleView::setRelayOn(bool relayOn)
{
    m_relayOn = relayOn;
    update();
}

void VehicleView::setMotorRunning(bool running)
{
    m_motorRunning = running;
    update();
}

void VehicleView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF area = rect().adjusted(20, 20, -20, -20);

    QLinearGradient bg(area.topLeft(), area.bottomRight());
    bg.setColorAt(0.0, QColor("#101C2B"));
    bg.setColorAt(1.0, QColor("#07111D"));

    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(area, 24, 24);

    // Road glow
    QPainterPath road;
    road.moveTo(area.left() + 70, area.bottom() - 50);
    road.lineTo(area.right() - 70, area.bottom() - 50);
    road.lineTo(area.right() - 130, area.bottom() - 100);
    road.lineTo(area.left() + 130, area.bottom() - 100);
    road.closeSubpath();

    p.setBrush(QColor(30, 210, 255, 28));
    p.drawPath(road);

    // Vehicle body
    QRectF body(area.center().x() - 155,
                area.center().y() - 35,
                310,
                85);

    p.setBrush(QColor("#162A3D"));
    p.setPen(QPen(QColor("#3DDCFF"), 2));
    p.drawRoundedRect(body, 18, 18);

    // Cabin
    QRectF cabin(body.left() + 55,
                 body.top() - 45,
                 120,
                 55);

    p.setBrush(QColor("#203A54"));
    p.setPen(QPen(QColor("#3DDCFF"), 2));
    p.drawRoundedRect(cabin, 14, 14);

    // Docking port
    QRectF dock(body.right() - 45,
                body.top() + 20,
                26,
                45);

    QColor dockColor =
        m_dockDetected ? QColor("#37D996") : QColor("#FF4D4D");

    p.setBrush(dockColor);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(dock, 8, 8);

    // Module ghost
    QRectF module(body.right() + 25,
                  body.top() - 5,
                  100,
                  90);

    QColor moduleColor =
        m_dockDetected ? QColor(55, 217, 150, 70)
                       : QColor(255, 255, 255, 22);

    p.setBrush(moduleColor);
    p.setPen(QPen(m_dockDetected ? QColor("#37D996")
                                 : QColor("#516273"),
                  2,
                  Qt::DashLine));
    p.drawRoundedRect(module, 18, 18);

    // Connection line
    p.setPen(QPen(m_dockDetected ? QColor("#37D996")
                                 : QColor("#516273"),
                  3,
                  Qt::DashLine));

    p.drawLine(QPointF(dock.right(), dock.center().y()),
               QPointF(module.left(), module.center().y()));

    // Wheels
    QColor wheelGlow =
        m_motorRunning ? QColor("#3DDCFF") : QColor("#52616F");

    p.setPen(QPen(wheelGlow, 4));
    p.setBrush(QColor("#080D14"));

    QPointF w1(body.left() + 70, body.bottom());
    QPointF w2(body.right() - 70, body.bottom());

    p.drawEllipse(w1, 30, 30);
    p.drawEllipse(w2, 30, 30);

    p.setPen(QPen(wheelGlow, 2));
    p.drawLine(w1 + QPointF(-18, 0), w1 + QPointF(18, 0));
    p.drawLine(w1 + QPointF(0, -18), w1 + QPointF(0, 18));
    p.drawLine(w2 + QPointF(-18, 0), w2 + QPointF(18, 0));
    p.drawLine(w2 + QPointF(0, -18), w2 + QPointF(0, 18));

    // Title
    p.setPen(QColor("#EAF6FF"));
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    p.setFont(titleFont);

    QString title = m_dockDetected ? "MODULE DETECTED"
                                   : "NO MODULE CONNECTED";

    p.drawText(area.adjusted(0, 18, 0, 0),
               Qt::AlignHCenter | Qt::AlignTop,
               title);

    QFont subFont;
    subFont.setPointSize(10);
    subFont.setBold(false);
    p.setFont(subFont);
    p.setPen(QColor("#8EA5B8"));

    QString subText =
        m_dockDetected
            ? "Ready to supply power and start authentication"
            : "Attach a certified module to the docking port";

    p.drawText(area.adjusted(0, 54, 0, 0),
               Qt::AlignHCenter | Qt::AlignTop,
               subText);

    // Relay badge
    QRectF relayBadge(area.right() - 170,
                      area.top() + 18,
                      130,
                      34);

    p.setPen(Qt::NoPen);
    p.setBrush(m_relayOn ? QColor("#163126") : QColor("#332126"));
    p.drawRoundedRect(relayBadge, 12, 12);

    p.setPen(m_relayOn ? QColor("#BFF7DD") : QColor("#FFB5B5"));
    QFont badgeFont;
    badgeFont.setPointSize(9);
    badgeFont.setBold(true);
    p.setFont(badgeFont);

    p.drawText(relayBadge,
               Qt::AlignCenter,
               m_relayOn ? "RELAY ON" : "RELAY OFF");
}

IdlePage::IdlePage(QWidget *parent)
    : QWidget(parent),
      m_socket(new QLocalSocket(this)),
      m_vehicleView(new VehicleView(this)),
      m_timeLabel(new QLabel("--:--", this)),
      m_serviceLabel(createStatusChip("UART : WAITING")),
      m_moduleLabel(createStatusChip("MODULE : DISCONNECTED")),
      m_dockingLabel(createStatusChip("DOCK : EMPTY")),
      m_relayLabel(createStatusChip("RELAY : OFF")),
      m_authLabel(createStatusChip("AUTH : WAITING")),
      m_driveStateLabel(new QLabel("STANDBY", this)),
      m_speedProfileLabel(new QLabel("MID", this)),
      m_targetRpmLabel(new QLabel("--.- RPM", this)),
      m_currentRpmLabel(new QLabel("--.- RPM", this)),
      m_dutyLabel(new QLabel("--.- %", this)),
      m_connectButton(createNavButton("CONNECT MODULE")),
      m_lowButton(createNavButton("LOW")),
      m_midButton(createNavButton("MID")),
      m_highButton(createNavButton("HIGH")),
      m_startButton(createNavButton("DRIVE START")),
      m_stopButton(createNavButton("DRIVE STOP")),
      m_dockDetected(false),
      m_relayOn(false)
{
    setObjectName("IdlePageRoot");
    setAttribute(Qt::WA_StyledBackground, true);

    setStyleSheet(
        "#IdlePageRoot {"
        " background: #050B12;"
        " color: #EAF6FF;"
        " font-family: Arial;"
        "}"
    );

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 18, 22, 18);
    root->setSpacing(14);

    m_serviceLabel->hide();

    // ============================================================
    // Top navigation bar
    // ============================================================
    QHBoxLayout *topBar = new QHBoxLayout();

    QLabel *title = new QLabel("PBV DOCKING SYSTEM", this);
    title->setStyleSheet(
        "font-size: 24px;"
        "font-weight: 800;"
        "letter-spacing: 1px;"
        "color: #EAF6FF;"
    );

    QLabel *mode = new QLabel("IDLE MODE", this);
    mode->setStyleSheet(
        "font-size: 15px;"
        "font-weight: 800;"
        "color: #46D8FF;"
        "padding: 8px 16px;"
        "border: 1px solid #29506A;"
        "border-radius: 16px;"
        "background: #0B1A26;"
    );

    m_timeLabel->setStyleSheet(
        "font-size: 18px;"
        "font-weight: 700;"
        "color: #AAB8C7;"
    );

    topBar->addWidget(title);
    topBar->addStretch();
    topBar->addWidget(mode);
    topBar->addSpacing(18);
    topBar->addWidget(m_timeLabel);

    // ============================================================
    // Status chip row
    // ============================================================
    QHBoxLayout *chipRow = new QHBoxLayout();

    chipRow->addWidget(m_moduleLabel);
    chipRow->addWidget(m_dockingLabel);
    chipRow->addWidget(m_relayLabel);
    chipRow->addWidget(m_authLabel);
    chipRow->addStretch();

    // ============================================================
    // Main area
    // ============================================================
    QHBoxLayout *mainArea = new QHBoxLayout();
    mainArea->setSpacing(16);

    QFrame *leftPanel = new QFrame(this);
    leftPanel->setStyleSheet(
        "QFrame {"
        " background: #0B1622;"
        " border: 1px solid #1D3346;"
        " border-radius: 22px;"
        "}"
    );

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(18, 18, 18, 18);
    leftLayout->setSpacing(14);

    QLabel *vehicleTitle = new QLabel("DOCKING PORT VISUALIZER", this);
    vehicleTitle->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    leftLayout->addWidget(vehicleTitle);
    leftLayout->addWidget(m_vehicleView, 1);

    QFrame *rightPanel = new QFrame(this);
    rightPanel->setStyleSheet(
        "QFrame {"
        " background: #0B1622;"
        " border: 1px solid #1D3346;"
        " border-radius: 22px;"
        "}"
    );

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(18, 18, 18, 18);
    rightLayout->setSpacing(14);

    QLabel *driveTitle = new QLabel("VEHICLE DRIVE CONTROL", this);
    driveTitle->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    QFrame *driveCard = new QFrame(this);
    driveCard->setStyleSheet(
        "QFrame {"
        " background: #101F2E;"
        " border: 1px solid #2C465D;"
        " border-radius: 18px;"
        "}"
    );

    QVBoxLayout *driveCardLayout = new QVBoxLayout(driveCard);
    driveCardLayout->setContentsMargins(18, 18, 18, 18);
    driveCardLayout->setSpacing(14);

    QLabel *driveLabel = new QLabel("DRIVE STATE", this);

    m_driveStateLabel->setAlignment(Qt::AlignCenter);
    m_driveStateLabel->setStyleSheet(
        "font-size: 38px;"
        "font-weight: 900;"
        "color: #46D8FF;"
        "background: transparent;"
        "border: none;"
    );

    driveLabel->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    QHBoxLayout *rpmRow = new QHBoxLayout();

    QLabel *targetTitle = new QLabel("TARGET", this);
    QLabel *currentTitle = new QLabel("CURRENT", this);
    QLabel *dutyTitle = new QLabel("PWM", this);

    const QString smallTitleStyle =
        "font-size: 11px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;";

    targetTitle->setStyleSheet(smallTitleStyle);
    currentTitle->setStyleSheet(smallTitleStyle);
    dutyTitle->setStyleSheet(smallTitleStyle);

    const QString valueStyle =
        "font-size: 18px;"
        "font-weight: 800;"
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

    QLabel *profileTitle = new QLabel("DRIVE MOTOR SPEED", this);
    profileTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 700;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    QHBoxLayout *speedRow = new QHBoxLayout();
    speedRow->addWidget(m_lowButton);
    speedRow->addWidget(m_midButton);
    speedRow->addWidget(m_highButton);

    QHBoxLayout *driveButtonRow = new QHBoxLayout();
    driveButtonRow->addWidget(m_startButton);
    driveButtonRow->addWidget(m_stopButton);

    QHBoxLayout *modeRow = new QHBoxLayout();

    QLabel *modeTitle = new QLabel("DRIVE MODE", this);

    modeTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 700;"
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

    modeRow->addWidget(modeTitle);
    modeRow->addStretch();
    modeRow->addWidget(m_speedProfileLabel);

    driveCardLayout->addWidget(driveLabel);
    driveCardLayout->addWidget(m_driveStateLabel);
    driveCardLayout->addLayout(modeRow);
    driveCardLayout->addLayout(rpmRow);
    driveCardLayout->addSpacing(10);
    driveCardLayout->addWidget(profileTitle);
    driveCardLayout->addLayout(speedRow);
    driveCardLayout->addLayout(driveButtonRow);

    QFrame *connectCard = new QFrame(this);
    connectCard->setStyleSheet(
        "QFrame {"
        " background: #101F2E;"
        " border: 1px solid #2C465D;"
        " border-radius: 18px;"
        "}"
    );

    QVBoxLayout *connectLayout = new QVBoxLayout(connectCard);
    connectLayout->setContentsMargins(18, 18, 18, 18);
    connectLayout->setSpacing(12);

    QLabel *connectTitle = new QLabel("MODULE POWER AUTHORIZATION", this);
    connectTitle->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 800;"
        "color: #8EA5B8;"
        "background: transparent;"
        "border: none;"
    );

    QLabel *connectDesc = new QLabel(
        "Physical docking detection enables relay power supply.\n"
        "Press CONNECT MODULE to start identification and authentication.",
        this
    );

    connectDesc->setStyleSheet(
        "font-size: 12px;"
        "line-height: 150%;"
        "color: #AAB8C7;"
        "background: transparent;"
        "border: none;"
    );

    m_connectButton->setMinimumHeight(48);

    connectLayout->addWidget(connectTitle);
    connectLayout->addWidget(connectDesc);
    connectLayout->addWidget(m_connectButton);

    rightLayout->addWidget(driveTitle);
    rightLayout->addWidget(driveCard, 2);
    rightLayout->addWidget(connectCard, 1);

    mainArea->addWidget(leftPanel, 3);
    mainArea->addWidget(rightPanel, 2);

    root->addLayout(topBar);
    root->addLayout(chipRow);
    root->addLayout(mainArea, 1);

    // Initial state
    setServiceConnected(false);
    setDockDetected(false);
    updateDriveProfile(2);

    // Timer
    QTimer *clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, [this]() {
        m_timeLabel->setText(
            QDateTime::currentDateTime().toString("HH:mm")
        );
    });
    clockTimer->start(1000);

    m_timeLabel->setText(
        QDateTime::currentDateTime().toString("HH:mm")
    );

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
    connect(m_connectButton, &QPushButton::clicked,
            this, &IdlePage::connectModule);

    connect(m_startButton, &QPushButton::clicked,
            this, &IdlePage::driveStart);

    connect(m_stopButton, &QPushButton::clicked,
            this, &IdlePage::driveStop);

    connect(m_lowButton, &QPushButton::clicked,
            this, &IdlePage::selectLow);

    connect(m_midButton, &QPushButton::clicked,
            this, &IdlePage::selectMid);

    connect(m_highButton, &QPushButton::clicked,
            this, &IdlePage::selectHigh);
}

QPushButton *IdlePage::createNavButton(const QString &text)
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
        " font-weight: 800;"
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

QLabel *IdlePage::createStatusChip(const QString &text)
{
    QLabel *label = new QLabel(text, this);

    label->setStyleSheet(
        "padding: 8px 14px;"
        "border: 1px solid #263D52;"
        "border-radius: 14px;"
        "background: #0B1A26;"
        "color: #AAB8C7;"
        "font-size: 12px;"
        "font-weight: 800;"
    );

    return label;
}

void IdlePage::setServiceConnected(bool connected)
{
    if (connected)
    {
        m_serviceLabel->setText("UART : CONNECTED");
        m_serviceLabel->setStyleSheet(
            "padding: 8px 14px;"
            "border: 1px solid #37D996;"
            "border-radius: 14px;"
            "background: #123426;"
            "color: #BFF7DD;"
            "font-size: 12px;"
            "font-weight: 800;"
        );
    }
    else
    {
        m_serviceLabel->setText("UART : WAITING");
        m_serviceLabel->setStyleSheet(
            "padding: 8px 14px;"
            "border: 1px solid #263D52;"
            "border-radius: 14px;"
            "background: #0B1A26;"
            "color: #AAB8C7;"
            "font-size: 12px;"
            "font-weight: 800;"
        );
    }
}

void IdlePage::setDockDetected(bool detected)
{
    m_dockDetected = detected;

    m_vehicleView->setDockDetected(detected);

    if (detected)
    {
        m_moduleLabel->setText("MODULE : DETECTED");
        m_dockingLabel->setText("DOCK : PHYSICAL LOCK");
        m_connectButton->setEnabled(m_dockDetected && !m_relayOn);
    }
    else
    {
        m_moduleLabel->setText("MODULE : DISCONNECTED");
        m_dockingLabel->setText("DOCK : EMPTY");
        m_connectButton->setEnabled(false);
    }
}

void IdlePage::setAuthStatus(const QString &status)
{
    m_authLabel->setText(QString("AUTH : %1").arg(status));
}

void IdlePage::setMotorStatus(int motorRunning,
                              int motorSpeedLevel,
                              double targetRpm,
                              double currentRpm,
                              double duty)
{
    const bool running = (motorRunning == 1);

    m_vehicleView->setMotorRunning(running);

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

void IdlePage::updateDriveProfile(int level)
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
}

void IdlePage::sendCommand(const QString &command)
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

void IdlePage::connectModule()
{
    m_relayOn = true;
    m_vehicleView->setRelayOn(true);
    m_relayLabel->setText("RELAY : ON");
    m_authLabel->setText("AUTH : REQUESTED");

    sendCommand("c");
}

void IdlePage::driveStart()
{
    sendCommand("1");
}

void IdlePage::driveStop()
{
    sendCommand("5");
}

void IdlePage::selectLow()
{
    updateDriveProfile(1);
    sendCommand("2");
}

void IdlePage::selectMid()
{
    updateDriveProfile(2);
    sendCommand("3");
}

void IdlePage::selectHigh()
{
    updateDriveProfile(3);
    sendCommand("4");
}

void IdlePage::setRelayState(bool relayOn)
{
    m_relayOn = relayOn;

    m_vehicleView->setRelayOn(relayOn);

    if (relayOn)
    {
        m_relayLabel->setText("RELAY : ON");
    }
    else
    {
        m_relayLabel->setText("RELAY : OFF");
    }

    m_connectButton->setEnabled(
        m_dockDetected &&
        !m_relayOn
    );
}

void IdlePage::setFsmState(int fsmState, int authResult)
{
    switch (fsmState)
    {
        case 0:
            m_authLabel->setText("AUTH : WAITING");
            break;

        case 1:
            m_authLabel->setText("AUTH : READY");
            break;

        case 2:
            m_authLabel->setText("AUTH : CHECKING");
            break;

        case 3:
            m_authLabel->setText("AUTH : ACCEPTED");
            break;

        case 4:
            m_authLabel->setText("AUTH : REJECTED");
            break;

        default:
            m_authLabel->setText("AUTH : UNKNOWN");
            break;
    }

    if (authResult == 2)
    {
        m_authLabel->setText("AUTH : REJECTED");
    }

    m_connectButton->setEnabled(
        m_dockDetected &&
        !m_relayOn &&
        fsmState == 1
    );
}