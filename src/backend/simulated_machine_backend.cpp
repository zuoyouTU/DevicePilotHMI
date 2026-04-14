#include "backend/simulated_machine_backend.h"

#include <algorithm>

#include "settings/settings_manager.h"

SimulatedMachineBackend::SimulatedMachineBackend(SettingsManager &settings, QObject *parent)
    : MachineBackend(parent)
    , m_settings(&settings)
{
    Q_ASSERT(m_settings);

    m_updateTimer.setInterval(m_settings->updateIntervalMs());
    connect(&m_updateTimer, &QTimer::timeout, this, &SimulatedMachineBackend::updateSimulation);

    m_transitionTimer.setSingleShot(true);
    connect(&m_transitionTimer,
            &QTimer::timeout,
            this,
            &SimulatedMachineBackend::onTransitionTimeout);

    connect(m_settings, &SettingsManager::updateIntervalMsChanged, this, [this]() {
        m_updateTimer.setInterval(m_settings->updateIntervalMs());
    });
}

void SimulatedMachineBackend::requestStart()
{
    if (m_state != MachineState::Idle) {
        return;
    }

    m_pendingTransition = PendingTransition::FinishStart;
    setState(MachineState::Starting);
    m_transitionTimer.start(5000);
}

void SimulatedMachineBackend::requestStop()
{
    if (m_state != MachineState::Starting && m_state != MachineState::Running) {
        return;
    }

    m_pendingTransition = PendingTransition::FinishStop;
    setState(MachineState::Stopping);
    m_transitionTimer.start(1200);
}

void SimulatedMachineBackend::requestResetFault()
{
    if (m_state != MachineState::Fault) {
        return;
    }

    m_updateTimer.stop();
    m_transitionTimer.stop();
    m_pendingTransition = PendingTransition::None;
    resetTelemetryToIdle();
    publishTelemetry();
    setState(MachineState::Idle);
}

void SimulatedMachineBackend::requestSafeShutdown()
{
    if (m_state == MachineState::Fault) {
        return;
    }

    m_updateTimer.stop();
    m_transitionTimer.stop();
    m_pendingTransition = PendingTransition::None;

    if (m_telemetry.speed != 0) {
        m_telemetry.speed = 0;
        publishTelemetry();
    }

    setState(MachineState::Fault);
}

void SimulatedMachineBackend::updateSimulation()
{
    if (m_state != MachineState::Running) {
        return;
    }

    m_telemetry.temperature += 1.6;
    m_telemetry.pressure += 2.4;
    m_telemetry.speed = std::min(3600, m_telemetry.speed + 120);
    publishTelemetry();
}

void SimulatedMachineBackend::onTransitionTimeout()
{
    switch (m_pendingTransition) {
    case PendingTransition::FinishStart:
        m_pendingTransition = PendingTransition::None;
        m_telemetry.speed = 800;
        publishTelemetry();
        setState(MachineState::Running);
        m_updateTimer.start();
        break;

    case PendingTransition::FinishStop:
        m_pendingTransition = PendingTransition::None;
        m_updateTimer.stop();
        resetTelemetryToIdle();
        publishTelemetry();
        setState(MachineState::Idle);
        break;

    case PendingTransition::None:
        break;
    }
}

void SimulatedMachineBackend::setState(MachineState newState)
{
    if (m_state == newState) {
        return;
    }

    m_state = newState;
    emit stateReported(m_state);
}

void SimulatedMachineBackend::publishTelemetry()
{
    emit telemetryReceived(m_telemetry);
}

void SimulatedMachineBackend::resetTelemetryToIdle()
{
    m_telemetry.temperature = RuntimeInit::kTemperature;
    m_telemetry.pressure = RuntimeInit::kPressure;
    m_telemetry.speed = RuntimeInit::kSpeed;
}
