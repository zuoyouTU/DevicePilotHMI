#pragma once

#include <QObject>

#include "runtime/machine_types.h"

class MachineBackend : public QObject
{
    Q_OBJECT

public:
    explicit MachineBackend(QObject *parent = nullptr);

    virtual void requestStart() = 0;
    virtual void requestStop() = 0;
    virtual void requestResetFault() = 0;
    // Request a transport/backend-level safe stop without making the backend
    // the authority for when the application enters Fault.
    virtual void requestSafeShutdown() = 0;

signals:
    void telemetryReceived(TelemetryFrame frame);
    void stateReported(MachineState state);
};
