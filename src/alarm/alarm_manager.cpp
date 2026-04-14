#include "alarm/alarm_manager.h"

#include "log/log_interface.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_manager.h"

AlarmManager::AlarmManager(LogInterface &logInterface,
                           SettingsManager &settings,
                           MachineRuntime &runtime,
                           QObject *parent)
    : QObject(parent)
    , m_logInterface(&logInterface)
    , m_settings(&settings)
    , m_runtime(&runtime)
{
    Q_ASSERT(m_logInterface);
    Q_ASSERT(m_settings);
    Q_ASSERT(m_runtime);

    connect(m_settings, &SettingsManager::thresholdsChanged, this, [this]() {
        if (m_alarmLevel != AlarmLevel::Fault) {
            evaluateAlarm();
        }
    });

    connect(m_runtime, &MachineRuntime::resetAlarmState, this, [this]() {
        m_alarmLevel = AlarmLevel::Normal;
        m_alarmText = "System normal";
        emit alarmChanged();
    });

    connect(m_runtime, &MachineRuntime::evaluateAlarm, this, &AlarmManager::evaluateAlarm);
}

QString AlarmManager::alarmText() const
{
    return m_alarmText;
}

bool AlarmManager::hasWarning() const
{
    return m_alarmLevel == AlarmLevel::Warning;
}

bool AlarmManager::isFault() const
{
    return m_alarmLevel == AlarmLevel::Fault;
}

void AlarmManager::evaluateAlarm()
{
    if (isFault()) {
        return;
    }

    const Snapshot &snapShot = m_settings->snapshot();
    const auto temperature = m_runtime->temperature();
    if (temperature >= snapShot.faultTemperature) {
        enterFault("Temperature exceeded fault threshold");
        return;
    }

    const auto pressure = m_runtime->pressure();
    if (pressure >= snapShot.faultPressure) {
        enterFault("Pressure exceeded fault threshold");
        return;
    }

    AlarmLevel newLevel = AlarmLevel::Normal;
    QString newAlarmText = "System normal";
    if (temperature >= snapShot.warningTemperature) {
        newLevel = AlarmLevel::Warning;
        newAlarmText = "Temperature exceeded warning threshold";
    } else if (pressure >= snapShot.warningPressure) {
        newLevel = AlarmLevel::Warning;
        newAlarmText = "Pressure exceeded warning threshold";
    }

    if (m_alarmLevel != newLevel || m_alarmText != newAlarmText) {
        if (newLevel == AlarmLevel::Warning && m_alarmLevel != AlarmLevel::Warning) {
            appendLog("WARNING", newAlarmText);
        }

        m_alarmLevel = newLevel;
        m_alarmText = newAlarmText;
        emit alarmChanged();
    }
}

void AlarmManager::enterFault(const QString &reason)
{
    if (isFault()) {
        return;
    }

    m_alarmLevel = AlarmLevel::Fault;
    m_alarmText = reason;
    m_runtime->enterFault();
    emit alarmChanged();

    appendLog("FAULT", reason);
}

void AlarmManager::appendLog(const QString &level, const QString &message)
{
    m_logInterface->appendLog(level, message);
}
