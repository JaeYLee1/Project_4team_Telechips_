#ifndef MOTOR_CLIENT_H
#define MOTOR_CLIENT_H

#include <QObject>
#include <QString>

class QLocalSocket;
class QTimer;

class MotorClient : public QObject
{
    Q_OBJECT

public:
    explicit MotorClient(QObject *parent = nullptr);

    bool sendMotorCommand(const QString &command);

signals:
    void connectionChanged(bool connected);
    void lineReceived(const QString &line);
    void commandFailed(const QString &reason);

private slots:
    void connectToService();
    void onConnected();
    void onDisconnected();
    void onReadyRead();

private:
    QLocalSocket *m_socket;
    QTimer *m_reconnectTimer;
    QByteArray m_receiveBuffer;
};

#endif