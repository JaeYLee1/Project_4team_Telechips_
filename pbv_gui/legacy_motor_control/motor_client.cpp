#include "motor_client.h"

#include <QLocalSocket>
#include <QTimer>

static const char *PBV_UART_SOCKET_PATH = "/tmp/pbv_uart.sock";

MotorClient::MotorClient(QObject *parent)
    : QObject(parent),
      m_socket(new QLocalSocket(this)),
      m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setInterval(1000);

    connect(m_reconnectTimer,
            &QTimer::timeout,
            this,
            &MotorClient::connectToService);

    connect(m_socket,
            &QLocalSocket::connected,
            this,
            &MotorClient::onConnected);

    connect(m_socket,
            &QLocalSocket::disconnected,
            this,
            &MotorClient::onDisconnected);

    connect(m_socket,
            &QLocalSocket::readyRead,
            this,
            &MotorClient::onReadyRead);

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

void MotorClient::connectToService()
{
    if (m_socket->state() ==
        QLocalSocket::ConnectedState)
    {
        return;
    }

    m_socket->abort();
    m_socket->connectToServer(PBV_UART_SOCKET_PATH);
}

void MotorClient::onConnected()
{
    m_reconnectTimer->stop();
    emit connectionChanged(true);
}

void MotorClient::onDisconnected()
{
    emit connectionChanged(false);

    if (!m_reconnectTimer->isActive())
    {
        m_reconnectTimer->start();
    }
}

void MotorClient::onReadyRead()
{
    m_receiveBuffer.append(m_socket->readAll());

    while (true)
    {
        const int newline_index =
            m_receiveBuffer.indexOf('\n');

        if (newline_index < 0)
        {
            break;
        }

        QByteArray line =
            m_receiveBuffer.left(newline_index).trimmed();

        m_receiveBuffer.remove(0, newline_index + 1);

        if (!line.isEmpty())
        {
            emit lineReceived(QString::fromUtf8(line));
        }
    }
}

bool MotorClient::sendMotorCommand(const QString &command)
{
    if (m_socket->state() !=
        QLocalSocket::ConnectedState)
    {
        emit commandFailed("UART service is not connected.");
        return false;
    }

    const QByteArray packet =
        QString("CMD,%1\n").arg(command).toUtf8();

    if (m_socket->write(packet) != packet.size())
    {
        emit commandFailed("Command write failed.");
        return false;
    }

    m_socket->flush();

    return true;
}