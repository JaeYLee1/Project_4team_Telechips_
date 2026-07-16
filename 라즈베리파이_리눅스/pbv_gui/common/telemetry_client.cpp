#include "telemetry_client.h"

#include <QDebug>
#include <QLocalSocket>
#include <QStringList>
#include <QTimer>

static const char *PBV_UART_SOCKET_PATH = "/tmp/pbv_uart.sock";

TelemetryClient::TelemetryClient(QObject *parent)
    : QObject(parent),
      m_socket(new QLocalSocket(this)),
      m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setInterval(1000);

    connect(m_reconnectTimer,
            &QTimer::timeout,
            this,
            &TelemetryClient::connectToService);

    connect(m_socket,
            &QLocalSocket::connected,
            this,
            &TelemetryClient::onConnected);

    connect(m_socket,
            &QLocalSocket::disconnected,
            this,
            &TelemetryClient::onDisconnected);

    connect(m_socket,
            &QLocalSocket::readyRead,
            this,
            &TelemetryClient::onReadyRead);

    connect(m_socket,
            &QLocalSocket::errorOccurred,
            this,
            [this](QLocalSocket::LocalSocketError)
            {
                if (m_socket->state() ==
                    QLocalSocket::UnconnectedState)
                {
                    m_reconnectTimer->start();
                }
            });

    connectToService();
}

void TelemetryClient::connectToService()
{
    if (m_socket->state() ==
        QLocalSocket::ConnectedState)
    {
        return;
    }

    m_socket->abort();
    m_socket->connectToServer(PBV_UART_SOCKET_PATH);
}

void TelemetryClient::onConnected()
{
    m_reconnectTimer->stop();

    qDebug() << "[TelemetryClient] Connected to uart service";

    emit connectionChanged(true);
}

void TelemetryClient::onDisconnected()
{
    qDebug() << "[TelemetryClient] Disconnected from uart service";

    emit connectionChanged(false);

    if (!m_reconnectTimer->isActive())
    {
        m_reconnectTimer->start();
    }
}

void TelemetryClient::onReadyRead()
{
    m_receiveBuffer.append(m_socket->readAll());

    while (true)
    {
        const int newlineIndex =
            m_receiveBuffer.indexOf('\n');

        if (newlineIndex < 0)
        {
            break;
        }

        const QByteArray rawLine =
            m_receiveBuffer.left(newlineIndex).trimmed();

        m_receiveBuffer.remove(0, newlineIndex + 1);

        if (rawLine.isEmpty())
        {
            continue;
        }

        const QString line =
            QString::fromUtf8(rawLine);

        const QStringList parts =
            line.split(',');

        /*
         * AI Warning Packet
         *
         * WARN,AI,1,3
         * WARN,AI,2,4
         * WARN,AI,0,0
         */
        if ((parts.size() == 4) &&
            (parts[0] == "WARN") &&
            (parts[1] == "AI"))
        {
            bool ok = false;

            const int level = parts[2].toInt(&ok);
            if (!ok)
            {
                continue;
            }

            const int sleepFlag = parts[3].toInt(&ok);
            if (!ok)
            {
                continue;
            }

            qDebug() << "[TelemetryClient] AI WARN RX level =" << level
                    << "sleepFlag =" << sleepFlag;

            emit aiWarningReceived(level, sleepFlag);
            continue;
        }

        /*
         * STM32 -> UART Service -> GUI
         *
         * RTOS_Data,
         * module,auth,pressure,current_temp_x10,target_temp_x10,
         * motor_running,motor_level,target_rpm_x10,current_rpm_x10,motor_duty_x10,
         * detect_state,relay_state,fsm_state,
         * base_power_mw,module_b_power_mw,cooling_power_mw,motor_power_mw,total_power_mw
         *
         * parts.size() == 19
         */
        if ((!line.startsWith("TEL,")) &&
            (!line.startsWith("RTOS_Data,")))
        {
            continue;
        }

        if (parts.size() != 22)
        {
            continue;
        }

        bool ok = false;

        const int moduleType = parts[1].toInt(&ok);
        if (!ok) { continue; }

        const int authResult = parts[2].toInt(&ok);
        if (!ok) { continue; }

        const int pressure = parts[3].toInt(&ok);
        if (!ok) { continue; }

        const int currentTempX10 = parts[4].toInt(&ok);
        if (!ok) { continue; }

        const int targetTempX10 = parts[5].toInt(&ok);
        if (!ok) { continue; }

        const int motorRunning = parts[6].toInt(&ok);
        if (!ok) { continue; }

        const int motorSpeedLevel = parts[7].toInt(&ok);
        if (!ok) { continue; }

        const int targetRpmX10 = parts[8].toInt(&ok);
        if (!ok) { continue; }

        const int currentRpmX10 = parts[9].toInt(&ok);
        if (!ok) { continue; }

        const int motorDutyX10 = parts[10].toInt(&ok);
        if (!ok) { continue; }

        const int detectState = parts[11].toInt(&ok);
        if (!ok) { continue; }

        const int relayState = parts[12].toInt(&ok);
        if (!ok) { continue; }

        const int fsmState = parts[13].toInt(&ok);
        if (!ok) { continue; }

        const int basePowerMw = parts[14].toInt(&ok);
        if (!ok) { continue; }

        const int moduleBPowerMw = parts[15].toInt(&ok);
        if (!ok) { continue; }

        const int coolingPowerMw = parts[16].toInt(&ok);
        if (!ok) { continue; }

        const int motorPowerMw = parts[17].toInt(&ok);
        if (!ok) { continue; }

        const int totalPowerMw = parts[18].toInt(&ok);
        if (!ok) { continue; }

        const int powerWarningCount = parts[19].toInt(&ok);
        if (!ok) { continue; }

        const int powerFault = parts[20].toInt(&ok);
        if (!ok) { continue; }

        const int powerLimitMw = parts[21].toInt(&ok);
        if (!ok) { continue; }

        /*
        * 기존 telemetryReceived 시그널 호환을 위해 peltierDuty 자리는 0으로 유지.
        */
        emit telemetryReceived(moduleType,
                            authResult,
                            pressure,
                            currentTempX10,
                            targetTempX10,
                            0,
                            motorRunning,
                            motorSpeedLevel,
                            targetRpmX10,
                            currentRpmX10,
                            motorDutyX10,
                            detectState,
                            relayState,
                            fsmState);

        emit powerTelemetryReceived(basePowerMw,
                                    moduleBPowerMw,
                                    coolingPowerMw,
                                    motorPowerMw,
                                    totalPowerMw,
                                    powerWarningCount,
                                    powerFault,
                                    powerLimitMw);

        emit powerProtectionReceived(powerWarningCount,
                                    powerFault,
                                    totalPowerMw,
                                    powerLimitMw);
    }
}