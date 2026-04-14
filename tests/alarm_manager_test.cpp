#include <QtTest>

#include <QFile>
#include <QStandardPaths>

#include "alarm/alarm_manager.h"
#include "log/log_interface.h"
#include "log/log_model.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_file_store.h"
#include "settings/settings_manager.h"
#include "test_machine_backend.h"

class AlarmManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void initialStateIsNormal();
    void faultStateRemainsLatchedUntilReset();
    void warningTemperatureThresholdCrossingRaisesWarning();
    void faultTemperatureThresholdCrossingEntersFault();
    void warningPressureThresholdCrossingRaisesWarning();
    void faultPressureThresholdCrossingEntersFault();
};

void AlarmManagerTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-AlarmManagerTests");
}

void AlarmManagerTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void AlarmManagerTest::initialStateIsNormal()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    QVERIFY(!alarm.hasWarning());
    QVERIFY(!alarm.isFault());
    QCOMPARE(alarm.alarmText(), QString("System normal"));
}

void AlarmManagerTest::faultStateRemainsLatchedUntilReset()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature - 2;
    candidate.faultTemperature = RuntimeInit::kTemperature - 1;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    QVERIFY(alarm.isFault());
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    backend.publishState(MachineState::Starting);
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    backend.publishState(MachineState::Running);
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    backend.publishState(MachineState::Stopping);
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    backend.publishState(MachineState::Idle);
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);
}

void AlarmManagerTest::warningTemperatureThresholdCrossingRaisesWarning()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature - 2;
    candidate.faultTemperature = RuntimeInit::kTemperature + 40;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 1.6, RuntimeInit::kPressure, 800);

    QVERIFY(alarm.hasWarning());
    QVERIFY(!alarm.isFault());
}

void AlarmManagerTest::faultTemperatureThresholdCrossingEntersFault()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature - 5;
    candidate.faultTemperature = RuntimeInit::kTemperature - 2;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 1.6, RuntimeInit::kPressure, 800);

    QVERIFY(alarm.isFault());
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);
    QCOMPARE(alarm.alarmText(), QString("Temperature exceeded fault threshold"));
}

void AlarmManagerTest::warningPressureThresholdCrossingRaisesWarning()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningPressure = RuntimeInit::kPressure - 2;
    candidate.faultPressure = RuntimeInit::kPressure + 40;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature, RuntimeInit::kPressure + 2.4, 800);

    QVERIFY(alarm.hasWarning());
    QVERIFY(!alarm.isFault());
}

void AlarmManagerTest::faultPressureThresholdCrossingEntersFault()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningPressure = RuntimeInit::kPressure - 5;
    candidate.faultPressure = RuntimeInit::kPressure - 2;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature, RuntimeInit::kPressure + 2.4, 800);

    QVERIFY(alarm.isFault());
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);
    QCOMPARE(alarm.alarmText(), QString("Pressure exceeded fault threshold"));
}

QTEST_APPLESS_MAIN(AlarmManagerTest)
#include "alarm_manager_test.moc"
