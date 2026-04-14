#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <qqmlintegration.h>

class MachineBackend;
class LogInterface;

#include "runtime/machine_types.h"

class MachineRuntime : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("MachineRuntime is created in C++ and injected into the root QML object.")

    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(double pressure READ pressure NOTIFY pressureChanged)
    Q_PROPERTY(int speed READ speed NOTIFY speedChanged)

    Q_PROPERTY(bool canStart READ canStart NOTIFY stateChanged)
    Q_PROPERTY(bool canStop READ canStop NOTIFY stateChanged)
    Q_PROPERTY(bool canResetFault READ canResetFault NOTIFY stateChanged)

public:
    using State = MachineState;

    explicit MachineRuntime(LogInterface &logInterface,
                            MachineBackend &backend,
                            QObject *parent = nullptr);

    QString status() const;
    double temperature() const;
    double pressure() const;
    int speed() const;

    bool canStart() const;
    bool canStop() const;
    bool canResetFault() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void resetFault();

signals:
    void statusChanged();
    void temperatureChanged();
    void pressureChanged();
    void speedChanged();
    void stateChanged();
    void evaluateAlarm();
    void resetAlarmState();

private slots:
    void onTelemetryReceived(TelemetryFrame frame);
    void onStateReported(MachineState state);

public:
    State state() const;
    void enterFault();

private:
    QString stateToString(State state) const;
    void appendLog(const QString &level, const QString &message);

private:
    State m_state{State::Idle};
    bool m_faultResetPending{false};

    double m_temperature{RuntimeInit::kTemperature};
    double m_pressure{RuntimeInit::kPressure};
    int m_speed{RuntimeInit::kSpeed};

    QPointer<MachineBackend> m_backend{nullptr};
    QPointer<LogInterface> m_logInterface{nullptr};
};
