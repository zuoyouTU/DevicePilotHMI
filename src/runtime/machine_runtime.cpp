#include "runtime/machine_runtime.h"

#include <QMetaType>

#include "backend/machine_backend.h"
#include "log/log_interface.h"

MachineRuntime::MachineRuntime(LogInterface &logInterface,
                               MachineBackend &backend,
                               QObject *parent)
    : QObject(parent)
    , m_backend(&backend)
    , m_logInterface(&logInterface)
{
    qRegisterMetaType<MachineState>("MachineState");
    qRegisterMetaType<TelemetryFrame>("TelemetryFrame");

    Q_ASSERT(m_backend);
    Q_ASSERT(m_logInterface);

    connect(m_backend,
            &MachineBackend::telemetryReceived,
            this,
            &MachineRuntime::onTelemetryReceived);
    connect(m_backend, &MachineBackend::stateReported, this, &MachineRuntime::onStateReported);
}

QString MachineRuntime::status() const
{
    return stateToString(m_state);
}

double MachineRuntime::temperature() const
{
    return m_temperature;
}

double MachineRuntime::pressure() const
{
    return m_pressure;
}

int MachineRuntime::speed() const
{
    return m_speed;
}

bool MachineRuntime::canStart() const
{
    return m_state == State::Idle;
}

bool MachineRuntime::canStop() const
{
    return m_state == State::Starting || m_state == State::Running;
}

bool MachineRuntime::canResetFault() const
{
    return m_state == State::Fault;
}

MachineRuntime::State MachineRuntime::state() const
{
    return m_state;
}

void MachineRuntime::start()
{
    if (!canStart()) {
        return;
    }

    appendLog("INFO", "Start requested");
    m_backend->requestStart();
}

void MachineRuntime::stop()
{
    if (!canStop()) {
        return;
    }

    appendLog("INFO", "Stop requested");
    m_backend->requestStop();
}

void MachineRuntime::resetFault()
{
    if (!canResetFault()) {
        return;
    }

    appendLog("INFO", "Fault reset requested");
    m_faultResetPending = true;
    m_backend->requestResetFault();
}

void MachineRuntime::onTelemetryReceived(TelemetryFrame frame)
{
    bool telemetryChanged = false;

    if (m_temperature != frame.temperature) {
        m_temperature = frame.temperature;
        emit temperatureChanged();
        telemetryChanged = true;
    }

    if (m_pressure != frame.pressure) {
        m_pressure = frame.pressure;
        emit pressureChanged();
        telemetryChanged = true;
    }

    if (m_speed != frame.speed) {
        m_speed = frame.speed;
        emit speedChanged();
        telemetryChanged = true;
    }

    if (telemetryChanged && m_state == State::Running) {
        emit evaluateAlarm();
    }
}

void MachineRuntime::onStateReported(MachineState state)
{
    if (m_state == State::Fault) {
        if (state == State::Idle && !m_faultResetPending) {
            return;
        }

        if (state != State::Fault && state != State::Idle) {
            return;
        }
    }

    if (m_state == state) {
        return;
    }

    const State previousState = m_state;
    m_state = state;
    emit statusChanged();
    emit stateChanged();

    if (m_state == State::Fault) {
        m_faultResetPending = false;
    }

    if (previousState == State::Starting && m_state == State::Running) {
        appendLog("INFO", "Transition to Running completed");
        emit evaluateAlarm();
        return;
    }

    if (previousState == State::Stopping && m_state == State::Idle) {
        appendLog("INFO", "Transition to Idle completed");
        emit resetAlarmState();
        return;
    }

    if (previousState == State::Fault && m_state == State::Idle) {
        m_faultResetPending = false;
        appendLog("INFO", "Fault reset completed");
        emit resetAlarmState();
    }
}

void MachineRuntime::enterFault()
{
    if (m_state == State::Fault) {
        return;
    }

    m_state = State::Fault;
    m_faultResetPending = false;
    emit statusChanged();
    emit stateChanged();

    if (m_speed != RuntimeInit::kSpeed) {
        m_speed = RuntimeInit::kSpeed;
        emit speedChanged();
    }

    m_backend->requestSafeShutdown();
}

QString MachineRuntime::stateToString(State state) const
{
    switch (state) {
    case State::Idle:
        return "Idle";
    case State::Starting:
        return "Starting";
    case State::Running:
        return "Running";
    case State::Stopping:
        return "Stopping";
    case State::Fault:
        return "Fault";
    }
    return "Unknown";
}

void MachineRuntime::appendLog(const QString &level, const QString &message)
{
    m_logInterface->appendLog(level, message);
}
