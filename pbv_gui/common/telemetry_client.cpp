#include "telemetry_client.h"

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

    emit connectionChanged(true);
}

void TelemetryClient::onDisconnected()
{
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

        if (!line.startsWith("TEL,"))
        {
            continue;
        }

        const QStringList parts =
            line.split(',');

        /*
         * TEL + 14 fields
         *
         * TEL,module,auth,weight,temp_x10,target_temp_x10,peltier_pwm,
         * motor_running,motor_level,target_rpm_x10,current_rpm_x10,motor_duty_x10,
         * detect_state,relay_state,fsm_state
         */
        if (parts.size() != 15)
        {
            continue;
        }

        bool ok = false;

        const int moduleType = parts[1].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int authResult = parts[2].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int pressure = parts[3].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int currentTempX10 = parts[4].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int targetTempX10 = parts[5].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int peltierDuty = parts[6].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int motorRunning = parts[7].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int motorSpeedLevel = parts[8].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int targetRpmX10 = parts[9].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int currentRpmX10 = parts[10].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int motorDutyX10 = parts[11].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int detectState = parts[12].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int relayState = parts[13].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        const int fsmState = parts[14].toInt(&ok);
        if (!ok)
        {
            continue;
        }

        emit telemetryReceived(moduleType,
                               authResult,
                               pressure,
                               currentTempX10,
                               targetTempX10,
                               peltierDuty,
                               motorRunning,
                               motorSpeedLevel,
                               targetRpmX10,
                               currentRpmX10,
                               motorDutyX10,
                               detectState,
                               relayState,
                               fsmState);
    }
}