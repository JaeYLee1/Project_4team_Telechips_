#include "hmi_window.h"

#include "idle_page.h"
#include "module_a_page.h"
#include "telemetry_client.h"

#include <QStackedWidget>

HmiWindow::HmiWindow(QWidget *parent)
    : QMainWindow(parent),
      m_stack(new QStackedWidget(this)),
      m_telemetryClient(new TelemetryClient(this)),
      m_idlePage(new IdlePage(this)),
      m_moduleAPage(new ModuleAPage(this))
{
    setWindowTitle("PBV Docking HMI");
    resize(1280, 720);

    m_stack->addWidget(m_idlePage);
    m_stack->addWidget(m_moduleAPage);

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
    Q_UNUSED(currentTempX10)
    Q_UNUSED(targetTempX10)
    Q_UNUSED(peltierDuty)

    const double targetRpm = targetRpmX10 / 10.0;
    const double currentRpm = currentRpmX10 / 10.0;
    const double duty = motorDutyX10 / 10.0;

    // IDLE 화면도 계속 최신 상태 유지
    m_idlePage->setDockDetected(detectState == 1);
    m_idlePage->setRelayState(relayState == 1);
    m_idlePage->setFsmState(fsmState, authResult);
    m_idlePage->setMotorStatus(motorRunning,
                               motorSpeedLevel,
                               targetRpm,
                               currentRpm,
                               duty);

    // Module A 화면 데이터 갱신
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

    /*
     * 화면 전환 조건
     *
     * moduleType == 1 : Module A
     * authResult == 1 : 인증 성공
     * detectState == 1: 물리 장착 감지
     * relayState == 1 : 릴레이 ON
     * fsmState == 3   : ACTIVE
     */
    if ((moduleType == 1) &&
        (authResult == 1) &&
        (detectState == 1) &&
        (relayState == 1) &&
        (fsmState == 3))
    {
        if (m_stack->currentWidget() != m_moduleAPage)
        {
            m_stack->setCurrentWidget(m_moduleAPage);
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
}