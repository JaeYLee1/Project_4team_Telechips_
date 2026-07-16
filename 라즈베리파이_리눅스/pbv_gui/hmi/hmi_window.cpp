#include "hmi_window.h"

#include "idle_page.h"
#include "module_a_page.h"
#include "module_b_page.h"
#include "telemetry_client.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QLocalSocket>
#include <QPushButton>
#include <QStackedWidget>

#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QLocalSocket>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <QDebug>
#include <QTimer>

static const char *UART_SOCKET_PATH = "/tmp/pbv_uart.sock";

HmiWindow::HmiWindow(QWidget *parent)
    : QMainWindow(parent),
      m_stack(new QStackedWidget(this)),
      m_telemetryClient(new TelemetryClient(this)),
      m_idlePage(new IdlePage(this)),
      m_moduleAPage(new ModuleAPage(this)),
      m_moduleBPage(new ModuleBPage(this)),
      m_commandSocket(new QLocalSocket(this)),
      m_aiWarningBox(nullptr),
      m_powerWarningBox(nullptr),
      m_currentAiWarningLevel(0),
      m_aiWarningEnabled(false),
      m_powerCutoffDialogShown(false),
      m_coolingFaultDialogShown(false),
      m_aiDialogSuppressed(false)
{
    setWindowTitle("PBV Docking HMI");
    resize(1280, 720);

    m_stack->addWidget(m_idlePage);
    m_stack->addWidget(m_moduleAPage);
    m_stack->addWidget(m_moduleBPage);

    setCentralWidget(m_stack);
    m_stack->setCurrentWidget(m_idlePage);

    connect(m_telemetryClient,
            &TelemetryClient::telemetryReceived,
            this,
            &HmiWindow::onTelemetryReceived);

    connect(m_telemetryClient,
            &TelemetryClient::connectionChanged,
            this,
            &HmiWindow::onConnectionChanged);

    connect(m_telemetryClient,
            &TelemetryClient::aiWarningReceived,
            this,
            &HmiWindow::onAiWarningReceived);

    connect(m_telemetryClient,
        &TelemetryClient::powerProtectionReceived,
        this,
        &HmiWindow::onPowerProtectionReceived);

    m_commandSocket->connectToServer(UART_SOCKET_PATH);

    connect(m_commandSocket,
            &QLocalSocket::disconnected,
            this,
            [this]() {
                m_commandSocket->abort();
                m_commandSocket->connectToServer(UART_SOCKET_PATH);
            });
}

void HmiWindow::onTelemetryReceived(int moduleType,
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
    Q_UNUSED(peltierDuty)

    const double targetRpm = targetRpmX10 / 10.0;
    const double currentRpm = currentRpmX10 / 10.0;
    const double duty = motorDutyX10 / 10.0;

    const bool moduleAActive =
        (moduleType == 1) &&
        (authResult == 1) &&
        (detectState == 1) &&
        (relayState == 1) &&
        (fsmState == 3);

    const bool moduleBActive =
        (moduleType == 2) &&
        (authResult == 1) &&
        (detectState == 1) &&
        (relayState == 1) &&
        (fsmState == 3);

    m_aiWarningEnabled = moduleAActive || moduleBActive;

    if (!m_aiWarningEnabled)
    {
        m_currentAiWarningLevel = 0;
        closeAiWarningDialog();
    }

    m_idlePage->setDockDetected(detectState == 1);
    m_idlePage->setRelayState(relayState == 1);
    m_idlePage->setFsmState(fsmState, authResult);
    m_idlePage->setMotorStatus(motorRunning,
                               motorSpeedLevel,
                               targetRpm,
                               currentRpm,
                               duty);

    m_moduleAPage->setSystemStatus(moduleType,
                                   authResult,
                                   detectState,
                                   relayState,
                                   fsmState);

    m_moduleAPage->setCargoStatus(pressure);

    m_moduleAPage->setMotorStatus(motorRunning,
                                  motorSpeedLevel,
                                  targetRpm,
                                  currentRpm,
                                  duty);

    m_moduleBPage->setSystemStatus(moduleType,
                                   authResult,
                                   detectState,
                                   relayState,
                                   fsmState);

    m_moduleBPage->setTemperatureStatus(currentTempX10,
                                        targetTempX10);

    m_moduleBPage->setMotorStatus(motorRunning,
                                  motorSpeedLevel,
                                  targetRpm,
                                  currentRpm,
                                  duty);

    if (moduleAActive)
    {
        if (m_stack->currentWidget() != m_moduleAPage)
        {
            m_stack->setCurrentWidget(m_moduleAPage);
        }
    }
    else if (moduleBActive)
    {
        if (m_stack->currentWidget() != m_moduleBPage)
        {
            m_stack->setCurrentWidget(m_moduleBPage);
        }
    }
    else
    {
        if (m_stack->currentWidget() != m_idlePage)
        {
            m_stack->setCurrentWidget(m_idlePage);
        }
    }
}

void HmiWindow::onConnectionChanged(bool connected)
{
    m_idlePage->setServiceConnected(connected);
    m_moduleAPage->setServiceConnected(connected);
    m_moduleBPage->setServiceConnected(connected);
}

void HmiWindow::onAiWarningReceived(int level, int sleepFlag)
{
    qDebug() << "[HMI AI] RX level =" << level
             << "sleepFlag =" << sleepFlag
             << "enabled =" << m_aiWarningEnabled
             << "suppressed =" << m_aiDialogSuppressed
             << "box =" << m_aiWarningBox;

    /*
     * WARN,AI,0은 항상 닫기 명령으로 처리.
     */
    if (level <= 0)
    {
        qDebug() << "[HMI AI] close request by WARN,AI,0";
        m_currentAiWarningLevel = 0;
        closeAiWarningDialog();
        return;
    }

    /*
     * ACK 직후 일정 시간 동안은 이전에 큐에 남아 있던
     * WARN,AI,1/2 재수신을 무시한다.
     */
    if (m_aiDialogSuppressed)
    {
        qDebug() << "[HMI AI] warning ignored by ACK suppression";
        return;
    }

    if (!m_aiWarningEnabled)
    {
        m_currentAiWarningLevel = 0;
        closeAiWarningDialog();
        return;
    }

    if ((m_currentAiWarningLevel == 2) &&
        (level == 1) &&
        (m_aiWarningBox != nullptr) &&
        m_aiWarningBox->isVisible())
    {
        m_aiWarningBox->raise();
        m_aiWarningBox->activateWindow();
        return;
    }

    if ((m_aiWarningBox != nullptr) &&
        (m_currentAiWarningLevel == level) &&
        m_aiWarningBox->isVisible())
    {
        m_aiWarningBox->raise();
        m_aiWarningBox->activateWindow();
        return;
    }

    m_currentAiWarningLevel = level;
    showAiWarningDialog(level, sleepFlag);
}

void HmiWindow::showAiWarningDialog(int level, int sleepFlag)
{
    Q_UNUSED(sleepFlag)

    closeAiWarningDialog();

    QString titleText;
    QString messageText;
    QString borderColor;
    QString accentColor;
    QString iconText;

    if (level == 1)
    {
        titleText = "AI DROWSINESS WARNING";
        messageText =
            "Drowsiness has been detected.\n\n"
            "The buzzer warning is active.\n"
            "Please check driver status.";
        borderColor = "#FFB020";
        accentColor = "#FFB020";
        iconText = "!";
    }
    else
    {
        titleText = "CRITICAL DROWSINESS WARNING";
        messageText =
            "Continuous drowsiness has been detected.\n\n"
            "Buzzer, LED warning and speed reduction are active.\n"
            "Please check safety status before clearing the warning.";
        borderColor = "#FF4D5A";
        accentColor = "#FF4D5A";
        iconText = "!";
    }

    QDialog *dialog = new QDialog(this);
    m_aiWarningBox = dialog;

    dialog->setAttribute(Qt::WA_DeleteOnClose, false);
    dialog->setModal(false);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setFixedSize(560, 250);

    QVBoxLayout *rootLayout = new QVBoxLayout(dialog);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QFrame *panel = new QFrame(dialog);
    panel->setStyleSheet(QString(
        "QFrame {"
        " background-color: #08131F;"
        " border: 2px solid %1;"
        " border-radius: 16px;"
        "}").arg(borderColor));

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(22, 18, 22, 18);
    panelLayout->setSpacing(14);

    QLabel *titleLabel = new QLabel(titleText, panel);
    titleLabel->setStyleSheet(QString(
        "QLabel {"
        " color: %1;"
        " font-size: 22px;"
        " font-weight: 800;"
        " border: none;"
        " background: transparent;"
        "}").arg(accentColor));
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(18);

    QLabel *iconLabel = new QLabel(iconText, panel);
    iconLabel->setFixedSize(58, 58);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(QString(
        "QLabel {"
        " background-color: %1;"
        " color: white;"
        " border-radius: 29px;"
        " font-size: 30px;"
        " font-weight: 900;"
        " border: none;"
        "}").arg(accentColor));

    QLabel *messageLabel = new QLabel(messageText, panel);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet(
        "QLabel {"
        " color: #EAF6FF;"
        " font-size: 17px;"
        " line-height: 1.4;"
        " border: none;"
        " background: transparent;"
        "}"
    );

    contentLayout->addWidget(iconLabel, 0, Qt::AlignTop);
    contentLayout->addWidget(messageLabel, 1);

    QPushButton *ackButton = new QPushButton("ACK / CLEAR", panel);
    ackButton->setFixedHeight(46);
    ackButton->setStyleSheet(QString(
        "QPushButton {"
        " background-color: #173047;"
        " color: #EAF6FF;"
        " border: 1px solid %1;"
        " border-radius: 10px;"
        " font-size: 16px;"
        " font-weight: 800;"
        " padding: 8px 20px;"
        "}"
        "QPushButton:hover {"
        " background-color: #21506D;"
        "}"
        "QPushButton:pressed {"
        " background-color: #123246;"
        "}").arg(accentColor));

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(ackButton);

    panelLayout->addWidget(titleLabel);
    panelLayout->addLayout(contentLayout);
    panelLayout->addStretch();
    panelLayout->addLayout(buttonLayout);

    rootLayout->addWidget(panel);

    connect(ackButton,
            &QPushButton::clicked,
            this,
            [this, dialog]() {
                qDebug() << "[HMI AI] ACK clicked";

                m_aiDialogSuppressed = true;
                m_currentAiWarningLevel = 0;

                sendCommand("K");

                if (m_aiWarningBox == dialog)
                {
                    m_aiWarningBox = nullptr;
                }

                dialog->setEnabled(false);
                dialog->hide();
                dialog->done(0);
                dialog->close();
                dialog->deleteLater();

                QTimer::singleShot(2000,
                                this,
                                [this]() {
                                    m_aiDialogSuppressed = false;
                                    qDebug() << "[HMI AI] ACK suppression released";
                                });
            });

    connect(dialog,
            &QObject::destroyed,
            this,
            [this, dialog](QObject *) {
                if (m_aiWarningBox == dialog)
                {
                    m_aiWarningBox = nullptr;
                }
            });

    QPoint centerPos = this->geometry().center()
                    - QPoint(dialog->width() / 2,
                                dialog->height() / 2);

    dialog->move(centerPos);

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void HmiWindow::closeAiWarningDialog()
{
    QDialog *dialog = m_aiWarningBox;

    qDebug() << "[HMI AI] closeAiWarningDialog called. dialog =" << dialog;

    if (dialog == nullptr)
    {
        return;
    }

    if (m_aiWarningBox == dialog)
    {
        m_aiWarningBox = nullptr;
    }

    dialog->setEnabled(false);
    dialog->hide();
    dialog->done(0);
    dialog->close();
    dialog->deleteLater();
}

void HmiWindow::sendCommand(const QString &command)
{
    if (command.length() != 1)
    {
        return;
    }

    if (m_commandSocket->state() != QLocalSocket::ConnectedState)
    {
        m_commandSocket->abort();
        m_commandSocket->connectToServer(UART_SOCKET_PATH);
        m_commandSocket->waitForConnected(100);
    }

    if (m_commandSocket->state() == QLocalSocket::ConnectedState)
    {
        const QByteArray packet =
            QString("CMD,%1\n").arg(command).toUtf8();

        m_commandSocket->write(packet);
        m_commandSocket->flush();
    }
}

void HmiWindow::onPowerProtectionReceived(int powerWarningCount,
                                          int powerFault,
                                          int totalPowerMw,
                                          int powerLimitMw)
{
    m_moduleBPage->setPowerProtectionStatus(powerWarningCount,
                                            powerFault,
                                            totalPowerMw,
                                            powerLimitMw);

    /*
     * 현재 STM32 기준:
     * - 과전력 차단: powerFault=1, powerWarningCount >= 4
     * - 펠티어 이상: powerFault=1, powerWarningCount < 4
     *
     * 나중에 STM32에서 펠티어 이상을 powerFault=2로 보내도
     * 그대로 대응 가능하게 작성.
     */
    const bool coolingFault =
        (powerFault == 2) ||
        ((powerFault == 1) && (powerWarningCount < 4));

    const bool powerCutoff =
        (powerFault == 1) && (powerWarningCount >= 4);

    if (coolingFault)
    {
        if (!m_coolingFaultDialogShown)
        {
            m_coolingFaultDialogShown = true;
            showCoolingFaultDialog();
        }

        return;
    }

    if (powerCutoff)
    {
        if (!m_powerCutoffDialogShown)
        {
            m_powerCutoffDialogShown = true;
            showPowerCutoffDialog(totalPowerMw, powerLimitMw);
        }

        return;
    }

    if ((powerFault == 0) && (powerWarningCount == 0))
    {
        m_powerCutoffDialogShown = false;
        m_coolingFaultDialogShown = false;
    }
}

void HmiWindow::showPowerCutoffDialog(int totalPowerMw,
                                      int powerLimitMw)
{
    const double totalPowerW = totalPowerMw / 1000.0;
    const double limitPowerW = powerLimitMw / 1000.0;

    if (m_powerWarningBox != nullptr)
    {
        m_powerWarningBox->raise();
        m_powerWarningBox->activateWindow();
        return;
    }

    /*
     * Power cutoff 발생 시 STM32 부저 경고 ON
     * 기존 Warning Level 1 명령을 시연용 전력 차단 경고음으로 재사용.
     */
    sendCommand("A");

    QDialog *dialog = new QDialog(this);
    m_powerWarningBox = dialog;

    dialog->setModal(false);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setFixedSize(520, 240);

    QVBoxLayout *rootLayout = new QVBoxLayout(dialog);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QFrame *panel = new QFrame(dialog);
    panel->setStyleSheet(
        "QFrame {"
        " background-color: #170B0B;"
        " border: 2px solid #FF4D5A;"
        " border-radius: 16px;"
        "}"
    );

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(22, 18, 22, 18);
    panelLayout->setSpacing(12);

    QLabel *titleLabel = new QLabel("POWER CUTOFF", panel);
    titleLabel->setStyleSheet(
        "QLabel {"
        " color: #FF4D5A;"
        " font-size: 22px;"
        " font-weight: 900;"
        " border: none;"
        " background: transparent;"
        "}"
    );

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    QLabel *iconLabel = new QLabel("!", panel);
    iconLabel->setFixedSize(56, 56);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(
        "QLabel {"
        " background-color: #FF4D5A;"
        " color: white;"
        " border-radius: 28px;"
        " font-size: 30px;"
        " font-weight: 900;"
        " border: none;"
        "}"
    );

    QLabel *messageLabel = new QLabel(panel);
    messageLabel->setWordWrap(true);
    messageLabel->setText(
        QString(
            "Module B power limit exceeded 4 times.\n"
            "Relay has been turned OFF for safety.\n\n"
            "Total : %1 W    Limit : %2 W"
        ).arg(totalPowerW, 0, 'f', 2)
         .arg(limitPowerW, 0, 'f', 2)
    );
    messageLabel->setStyleSheet(
        "QLabel {"
        " color: #EAF6FF;"
        " font-size: 16px;"
        " border: none;"
        " background: transparent;"
        "}"
    );

    contentLayout->addWidget(iconLabel, 0, Qt::AlignTop);
    contentLayout->addWidget(messageLabel, 1);

    QPushButton *ackButton = new QPushButton("ACK", panel);
    ackButton->setFixedSize(120, 42);
    ackButton->setStyleSheet(
        "QPushButton {"
        " background-color: #3A1111;"
        " color: #FFFFFF;"
        " border: 1px solid #FF4D5A;"
        " border-radius: 10px;"
        " font-size: 15px;"
        " font-weight: 900;"
        "}"
        "QPushButton:hover {"
        " background-color: #5A1717;"
        "}"
    );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(ackButton);

    panelLayout->addWidget(titleLabel);
    panelLayout->addLayout(contentLayout);
    panelLayout->addLayout(buttonLayout);

    rootLayout->addWidget(panel);

    connect(ackButton,
            &QPushButton::clicked,
            this,
            [this, dialog]() {
                /*
                * Power cutoff 팝업 ACK 시 STM32 경고 출력 OFF
                */
                sendCommand("K");

                if (m_powerWarningBox == dialog)
                {
                    m_powerWarningBox = nullptr;
                }

                dialog->hide();
                dialog->close();
                dialog->deleteLater();
            });

    connect(dialog,
            &QObject::destroyed,
            this,
            [this, dialog](QObject *) {
                if (m_powerWarningBox == dialog)
                {
                    m_powerWarningBox = nullptr;
                }
            });

    QPoint centerPos = this->geometry().center()
                       - QPoint(dialog->width() / 2,
                                dialog->height() / 2);

    dialog->move(centerPos);

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void HmiWindow::showCoolingFaultDialog(void)
{
    if (m_powerWarningBox != nullptr)
    {
        m_powerWarningBox->raise();
        m_powerWarningBox->activateWindow();
        return;
    }

    QDialog *dialog = new QDialog(this);
    m_powerWarningBox = dialog;

    dialog->setModal(false);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setFixedSize(560, 250);

    QVBoxLayout *rootLayout = new QVBoxLayout(dialog);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QFrame *panel = new QFrame(dialog);
    panel->setStyleSheet(
        "QFrame {"
        " background-color: #170B0B;"
        " border: 2px solid #FF4D5A;"
        " border-radius: 16px;"
        "}"
    );

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(22, 18, 22, 18);
    panelLayout->setSpacing(12);

    QLabel *titleLabel = new QLabel("COOLING MODULE FAULT", panel);
    titleLabel->setStyleSheet(
        "QLabel {"
        " color: #FF4D5A;"
        " font-size: 22px;"
        " font-weight: 900;"
        " border: none;"
        " background: transparent;"
        "}"
    );

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    QLabel *iconLabel = new QLabel("!", panel);
    iconLabel->setFixedSize(56, 56);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(
        "QLabel {"
        " background-color: #FF4D5A;"
        " color: white;"
        " border-radius: 28px;"
        " font-size: 30px;"
        " font-weight: 900;"
        " border: none;"
        "}"
    );

    QLabel *messageLabel = new QLabel(panel);
    messageLabel->setWordWrap(true);
    messageLabel->setText(
        "Relay is ON, but cooling current is too low.\n"
        "Peltier disconnection or driver fault is detected.\n\n"
        "Relay has been turned OFF for safety."
    );
    messageLabel->setStyleSheet(
        "QLabel {"
        " color: #EAF6FF;"
        " font-size: 16px;"
        " border: none;"
        " background: transparent;"
        "}"
    );

    contentLayout->addWidget(iconLabel, 0, Qt::AlignTop);
    contentLayout->addWidget(messageLabel, 1);

    QPushButton *ackButton = new QPushButton("ACK", panel);
    ackButton->setFixedSize(120, 42);
    ackButton->setStyleSheet(
        "QPushButton {"
        " background-color: #3A1111;"
        " color: #FFFFFF;"
        " border: 1px solid #FF4D5A;"
        " border-radius: 10px;"
        " font-size: 15px;"
        " font-weight: 900;"
        "}"
        "QPushButton:hover {"
        " background-color: #5A1717;"
        "}"
    );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(ackButton);

    panelLayout->addWidget(titleLabel);
    panelLayout->addLayout(contentLayout);
    panelLayout->addLayout(buttonLayout);

    rootLayout->addWidget(panel);

    connect(ackButton,
            &QPushButton::clicked,
            this,
            [this, dialog]() {
                /*
                 * STM32 부저/LED OFF.
                 * STM32는 이미 fault 발생 시 부저/LED를 켰으므로
                 * GUI는 ACK 시 K만 보낸다.
                 */
                sendCommand("K");

                if (m_powerWarningBox == dialog)
                {
                    m_powerWarningBox = nullptr;
                }

                dialog->hide();
                dialog->close();
                dialog->deleteLater();
            });

    connect(dialog,
            &QObject::destroyed,
            this,
            [this, dialog](QObject *) {
                if (m_powerWarningBox == dialog)
                {
                    m_powerWarningBox = nullptr;
                }
            });

    QPoint centerPos = this->geometry().center()
                       - QPoint(dialog->width() / 2,
                                dialog->height() / 2);

    dialog->move(centerPos);

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}