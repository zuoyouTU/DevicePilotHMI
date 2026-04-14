#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QVariant>

#include "alarm/alarm_manager.h"
#include "backend/simulated_machine_backend.h"
#include "log/log_interface.h"
#include "log/log_model.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_apply_service.h"
#include "settings/settings_manager.h"
#include "settings/settings_session.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    SimulatedMachineBackend simulatedBackend(settingsManager);
    MachineRuntime machineRuntime(logInterface, simulatedBackend);
    AlarmManager alarmManager(logInterface, settingsManager, machineRuntime);
    SettingsApplyService settingsApplyService(logInterface, settingsManager, machineRuntime);
    SettingsSession settingsSession(logInterface, settingsManager, settingsApplyService);

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.setInitialProperties({{"runtime", QVariant::fromValue(&machineRuntime)},
                                 {"alarm", QVariant::fromValue(&alarmManager)},
                                 {"logModel", QVariant::fromValue(&logModel)},
                                 {"settingsSession", QVariant::fromValue(&settingsSession)}});

    engine.loadFromModule("DevicePilotHMI", "Main");

    return app.exec();
}
