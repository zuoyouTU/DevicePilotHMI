#pragma once

#include "backend/machine_backend.h"

class FakeMachineBackend : public MachineBackend
{
public:
    using MachineBackend::MachineBackend;

    void requestStart() override
    {
        if (m_state != MachineState::Idle) {
            return;
        }

        m_state = MachineState::Starting;
        emit stateReported(m_state);
    }

    void requestStop() override
    {
        if (m_state != MachineState::Starting && m_state != MachineState::Running) {
            return;
        }

        m_state = MachineState::Stopping;
        emit stateReported(m_state);
    }

    void requestResetFault() override
    {
        if (m_state != MachineState::Fault) {
            return;
        }

        m_telemetry = {
            RuntimeInit::kTemperature,
            RuntimeInit::kPressure,
            RuntimeInit::kSpeed,
        };
        emit telemetryReceived(m_telemetry);
        m_state = MachineState::Idle;
        emit stateReported(m_state);
    }

    void requestSafeShutdown() override
    {
        if (m_state == MachineState::Fault) {
            return;
        }

        m_telemetry.speed = 0;
        emit telemetryReceived(m_telemetry);

        m_state = MachineState::Fault;
        emit stateReported(m_state);
    }

    void publishState(MachineState state)
    {
        m_state = state;
        emit stateReported(m_state);
    }

    void publishTelemetry(double temperature, double pressure, int speed)
    {
        m_telemetry = TelemetryFrame{temperature, pressure, speed};
        emit telemetryReceived(m_telemetry);
    }

private:
    MachineState m_state{MachineState::Idle};
    TelemetryFrame m_telemetry{
        RuntimeInit::kTemperature,
        RuntimeInit::kPressure,
        RuntimeInit::kSpeed,
    };
};
