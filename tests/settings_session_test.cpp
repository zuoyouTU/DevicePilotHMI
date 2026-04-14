#include <QtTest>

#include <QFile>
#include <QStandardPaths>

#include "log/log_interface.h"
#include "log/log_model.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_apply_service.h"
#include "settings/settings_file_store.h"
#include "settings/settings_manager.h"
#include "settings/settings_session.h"
#include "test_machine_backend.h"

class SettingsSessionTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void sessionStartsLoadedAndClean();
    void invalidDraftDisablesApplyWithoutRestrictionReason();
    void idleDirtyValidDraftEnablesApply();
    void startingDisablesApplyWithReason();
    void runtimeTransitionUpdatesApplyState();
    void runningThresholdChangeEnablesApply();
    void runningIntervalChangeDisablesApplyWithReason();
    void stoppingDisablesApplyWithReason();
    void faultDisablesApplyWithReason();
    void reloadRestoresCleanState();
    void applyRestoresCleanState();
};

void SettingsSessionTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-SettingsSessionTests");
}

void SettingsSessionTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void SettingsSessionTest::sessionStartsLoadedAndClean()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    QCOMPARE(session.draft()->warningTemperature(), settingsManager.snapshot().warningTemperature);
    QCOMPARE(session.draft()->faultTemperature(), settingsManager.snapshot().faultTemperature);
    QCOMPARE(session.draft()->warningPressure(), settingsManager.snapshot().warningPressure);
    QCOMPARE(session.draft()->faultPressure(), settingsManager.snapshot().faultPressure);
    QCOMPARE(session.draft()->updateIntervalMs(), settingsManager.snapshot().updateIntervalMs);
    QVERIFY(!session.draft()->dirty());
    QVERIFY(!session.applyEnabled());
}

void SettingsSessionTest::invalidDraftDisablesApplyWithoutRestrictionReason()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    session.draft()->setUpdateIntervalMs(50);

    QVERIFY(!session.draft()->valid());
    QVERIFY(session.draft()->dirty());
    QVERIFY(!session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().isEmpty());
}

void SettingsSessionTest::idleDirtyValidDraftEnablesApply()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    session.draft()->setWarningTemperature(settingsManager.snapshot().warningTemperature + 1);

    QVERIFY(session.draft()->dirty());
    QVERIFY(session.draft()->valid());
    QVERIFY(session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().isEmpty());
}

void SettingsSessionTest::startingDisablesApplyWithReason()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    runtime.start();
    QCOMPARE(runtime.state(), MachineRuntime::State::Starting);
    session.draft()->setWarningTemperature(settingsManager.snapshot().warningTemperature + 1);

    QVERIFY(!session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().contains("starting", Qt::CaseInsensitive));
}

void SettingsSessionTest::runtimeTransitionUpdatesApplyState()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    session.draft()->setWarningTemperature(settingsManager.snapshot().warningTemperature + 1);
    QVERIFY(session.applyEnabled());

    runtime.start();
    QCOMPARE(runtime.state(), MachineRuntime::State::Starting);

    QVERIFY(!session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().contains("starting", Qt::CaseInsensitive));
}

void SettingsSessionTest::runningThresholdChangeEnablesApply()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    runtime.start();
    backend.publishState(MachineState::Running);
    QCOMPARE(runtime.state(), MachineRuntime::State::Running);

    session.draft()->setWarningTemperature(settingsManager.snapshot().warningTemperature + 1);

    QVERIFY(session.draft()->valid());
    QVERIFY(session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().isEmpty());
}

void SettingsSessionTest::runningIntervalChangeDisablesApplyWithReason()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    runtime.start();
    backend.publishState(MachineState::Running);
    QCOMPARE(runtime.state(), MachineRuntime::State::Running);

    session.draft()->setUpdateIntervalMs(settingsManager.snapshot().updateIntervalMs + 100);

    QVERIFY(session.draft()->valid());
    QVERIFY(!session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().contains("update interval", Qt::CaseInsensitive));
}

void SettingsSessionTest::stoppingDisablesApplyWithReason()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    runtime.start();
    backend.publishState(MachineState::Running);
    runtime.stop();
    QCOMPARE(runtime.state(), MachineRuntime::State::Stopping);

    session.draft()->setWarningTemperature(settingsManager.snapshot().warningTemperature + 1);

    QVERIFY(!session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().contains("stopping", Qt::CaseInsensitive));
}

void SettingsSessionTest::faultDisablesApplyWithReason()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    runtime.enterFault();
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    session.draft()->setWarningTemperature(settingsManager.snapshot().warningTemperature + 1);

    QVERIFY(!session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().contains("fault", Qt::CaseInsensitive));
}

void SettingsSessionTest::reloadRestoresCleanState()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    session.draft()->setWarningTemperature(settingsManager.snapshot().warningTemperature + 1);
    QVERIFY(session.applyEnabled());

    session.reload();

    QVERIFY(!session.draft()->dirty());
    QVERIFY(!session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().isEmpty());
}

void SettingsSessionTest::applyRestoresCleanState()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    SettingsSession session(logInterface, settingsManager, service);

    const int expectedWarningTemperature = settingsManager.snapshot().warningTemperature + 1;
    session.draft()->setWarningTemperature(expectedWarningTemperature);

    QVERIFY(session.applyEnabled());
    QVERIFY(session.apply());

    QCOMPARE(settingsManager.snapshot().warningTemperature, expectedWarningTemperature);
    QVERIFY(!session.draft()->dirty());
    QVERIFY(!session.applyEnabled());
    QVERIFY(session.applyRestrictionReason().isEmpty());
}

QTEST_APPLESS_MAIN(SettingsSessionTest)
#include "settings_session_test.moc"
