#include <QtTest>

#include <QFile>
#include <QStandardPaths>

#include "log/log_interface.h"
#include "log/log_model.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_apply_service.h"
#include "settings/settings_file_store.h"
#include "settings/settings_manager.h"
#include "test_machine_backend.h"

class SettingsApplyServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void idleAllowsThresholdAndIntervalChanges();
    void startingBlocksUpdateSettingsChanges();
    void runningBlocksUpdateIntervalChanges();
    void stoppingBlocksSettingsChanges();
    void faultBlocksSettingsChanges();
    void successfulApplyPersistsSnapshot();
    void rejectedApplyDoesNotPersistSnapshot();
};

void SettingsApplyServiceTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-SettingsApplyServiceTests");
}

void SettingsApplyServiceTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void SettingsApplyServiceTest::idleAllowsThresholdAndIntervalChanges()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature += 1;
    candidate.faultTemperature += 1;
    candidate.warningPressure += 1;
    candidate.faultPressure += 1;
    candidate.updateIntervalMs += 100;

    const auto analysis = service.analyzeSettingsApply(candidate);

    QVERIFY(analysis.allowed);
    QVERIFY(analysis.reason.isEmpty());
    QVERIFY(analysis.changesThresholds);
    QVERIFY(analysis.changesUpdateInterval);
}

void SettingsApplyServiceTest::startingBlocksUpdateSettingsChanges()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);

    runtime.start();
    QCOMPARE(runtime.state(), MachineRuntime::State::Starting);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.updateIntervalMs += 100;
    auto analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(!analysis.allowed);
    QVERIFY(analysis.reason.contains("starting", Qt::CaseInsensitive));
    QVERIFY(analysis.changesUpdateInterval);

    candidate = settingsManager.snapshot();
    candidate.warningTemperature += 1;
    analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(!analysis.allowed);
    QVERIFY(analysis.reason.contains("starting", Qt::CaseInsensitive));
    QVERIFY(analysis.changesThresholds);

    candidate = settingsManager.snapshot();
    candidate.faultTemperature += 1;
    analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(!analysis.allowed);
    QVERIFY(analysis.reason.contains("starting", Qt::CaseInsensitive));
    QVERIFY(analysis.changesThresholds);

    candidate = settingsManager.snapshot();
    candidate.warningPressure += 1;
    analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(!analysis.allowed);
    QVERIFY(analysis.reason.contains("starting", Qt::CaseInsensitive));
    QVERIFY(analysis.changesThresholds);

    candidate = settingsManager.snapshot();
    candidate.faultPressure += 1;
    analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(!analysis.allowed);
    QVERIFY(analysis.reason.contains("starting", Qt::CaseInsensitive));
    QVERIFY(analysis.changesThresholds);
}

void SettingsApplyServiceTest::runningBlocksUpdateIntervalChanges()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);

    runtime.start();
    QCOMPARE(runtime.state(), MachineRuntime::State::Starting);
    backend.publishState(MachineState::Running);
    QCOMPARE(runtime.state(), MachineRuntime::State::Running);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.updateIntervalMs += 100;
    auto analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(!analysis.allowed);
    QVERIFY(analysis.reason.contains("update interval", Qt::CaseInsensitive));
    QVERIFY(analysis.changesUpdateInterval);

    candidate = settingsManager.snapshot();
    candidate.warningTemperature += 1;
    analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(analysis.allowed);
    QVERIFY(analysis.reason.isEmpty());
    QVERIFY(analysis.changesThresholds);

    candidate = settingsManager.snapshot();
    candidate.faultTemperature += 1;
    analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(analysis.allowed);
    QVERIFY(analysis.reason.isEmpty());
    QVERIFY(analysis.changesThresholds);

    candidate = settingsManager.snapshot();
    candidate.warningPressure += 1;
    analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(analysis.allowed);
    QVERIFY(analysis.reason.isEmpty());
    QVERIFY(analysis.changesThresholds);

    candidate = settingsManager.snapshot();
    candidate.faultPressure += 1;
    analysis = service.analyzeSettingsApply(candidate);
    QVERIFY(analysis.allowed);
    QVERIFY(analysis.reason.isEmpty());
    QVERIFY(analysis.changesThresholds);
}

void SettingsApplyServiceTest::stoppingBlocksSettingsChanges()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);

    runtime.start();
    backend.publishState(MachineState::Running);
    QCOMPARE(runtime.state(), MachineRuntime::State::Running);

    runtime.stop();
    QCOMPARE(runtime.state(), MachineRuntime::State::Stopping);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature += 1;

    const auto analysis = service.analyzeSettingsApply(candidate);

    QVERIFY(!analysis.allowed);
    QVERIFY(analysis.reason.contains("stopping", Qt::CaseInsensitive));
    QVERIFY(analysis.changesThresholds);
}

void SettingsApplyServiceTest::faultBlocksSettingsChanges()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);

    runtime.enterFault();
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature += 1;

    const auto analysis = service.analyzeSettingsApply(candidate);

    QVERIFY(!analysis.allowed);
    QVERIFY(analysis.reason.contains("fault", Qt::CaseInsensitive));
    QVERIFY(analysis.changesThresholds);
}

void SettingsApplyServiceTest::successfulApplyPersistsSnapshot()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature += 1;
    candidate.faultTemperature += 1;
    candidate.warningPressure += 1;
    candidate.faultPressure += 1;
    candidate.updateIntervalMs += 100;

    QVERIFY(service.applySettings(candidate));
    QCOMPARE(settingsManager.snapshot(), candidate);

    const auto loaded = Settings::Store::loadSnapshot();
    QCOMPARE(loaded.snapshot, candidate);
    QVERIFY(!loaded.repaired);
    QVERIFY(loaded.reason.isEmpty());
}

void SettingsApplyServiceTest::rejectedApplyDoesNotPersistSnapshot()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);

    const Settings::Snapshot baseline = settingsManager.snapshot();

    runtime.start();
    backend.publishState(MachineState::Running);
    QCOMPARE(runtime.state(), MachineRuntime::State::Running);

    Settings::Snapshot candidate = baseline;
    candidate.updateIntervalMs += 100;

    QVERIFY(!service.applySettings(candidate));
    QCOMPARE(settingsManager.snapshot(), baseline);

    const auto loaded = Settings::Store::loadSnapshot();
    QCOMPARE(loaded.snapshot, baseline);
    QVERIFY(!loaded.repaired);
}

QTEST_APPLESS_MAIN(SettingsApplyServiceTest)
#include "settings_apply_service_test.moc"
