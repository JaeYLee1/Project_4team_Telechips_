#include "hmi_window.h"

#include "idle_page.h"
#include "module_a_page.h"
#include "module_b_page.h"
#include "telemetry_client.h"

#include <QStackedWidget>

HmiWindow::HmiWindow(QWidget *parent)
    : QMainWindow(parent),
      m_stack(new QStackedWidget(this)),
      m_telemetryClient(new TelemetryClient(this)),
      m_idlePage(new IdlePage(this)),
      m_moduleAPage(new ModuleAPage(this)),
      m_moduleBPage(new ModuleBPage(this))
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
    Q_UNUSED(pressure)

    const double targetRpm = targetRpmX10 / 10.0;
    const double currentRpm = currentRpmX10 / 10.0;
    const double duty = motorDutyX10 / 10.0;

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
                                        targetTempX10,
                                        peltierDuty);

    m_moduleBPage->setMotorStatus(motorRunning,
                                  motorSpeedLevel,
                                  targetRpm,
                                  currentRpm,
                                  duty);

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
    else if ((moduleType == 2) &&
             (authResult == 1) &&
             (detectState == 1) &&
             (relayState == 1) &&
             (fsmState == 3))
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