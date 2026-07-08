#ifndef TELEMETRY_CLIENT_H
#define TELEMETRY_CLIENT_H

#include <QObject>

class QLocalSocket;
class QTimer;

class TelemetryClient : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryClient(QObject *parent = nullptr);

signals:
    void connectionChanged(bool connected);

    void telemetryReceived(int moduleType,
                        int authResult,
                        int weight,
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
                        int fsmState);                           

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