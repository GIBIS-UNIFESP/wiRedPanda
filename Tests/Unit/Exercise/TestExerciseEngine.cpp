// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Exercise/TestExerciseEngine.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Exercise/ExerciseEngine.h"
#include "App/Exercise/ExerciseOverlay.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

namespace {

const char *const kExerciseFixtureId = "test-retranslate-exercise";

const char *const kExerciseFixtureJson = R"({
    "id": "test-retranslate-exercise",
    "title": "Test Exercise",
    "steps": [
        { "key": "step0", "instruction": "Instruction 0", "hint": "Hint 0" },
        { "key": "step1", "instruction": "Instruction 1", "hint": "Hint 1" }
    ]
})";

QString writeExerciseFixture(const QTemporaryDir &dir)
{
    const QString path = dir.filePath("exercise.json");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    file.write(kExerciseFixtureJson);
    return path;
}

/// Writes a real file with \a contents at \a fileName inside \a dir, returning its path.
QString writeJsonFile(const QTemporaryDir &dir, const QString &fileName, const QByteArray &contents)
{
    const QString path = dir.filePath(fileName);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    file.write(contents);
    return path;
}

/// A 2-step exercise: step0 requires \a minCount elements of \a typeName, step1 is a plain
/// observe step. Two steps (not one) so satisfying step0 lands on a real "advanced" transition
/// (stepCompleted + stepChanged), not the reachedEnd/markCompleted path. The exercise "id" is
/// derived from \a fileName (not hardcoded) so each call site gets independent, non-persisted
/// progress -- Settings::exerciseProgress() is a real, process-wide persistent store shared
/// across every test in the run, not reset between test methods.
QString writeElementRequirementExercise(const QTemporaryDir &dir, const QString &typeName,
                                         int minCount, const QString &fileName)
{
    const QString id = QFileInfo(fileName).baseName();
    const QString json = QStringLiteral(R"({
        "id": "%1",
        "title": "Element Requirement",
        "steps": [
            { "key": "step0", "instruction": "Add elements",
              "requiredElements": [ { "type": "%2", "minCount": %3 } ] },
            { "key": "step1", "instruction": "Done" }
        ]
    })").arg(id, typeName).arg(minCount);
    return writeJsonFile(dir, fileName, json.toUtf8());
}

/// Same shape as writeElementRequirementExercise(), for a requiredConnections entry instead.
QString writeConnectionRequirementExercise(const QTemporaryDir &dir, const QString &fromType,
                                            const QString &toType, int minCount, const QString &fileName)
{
    const QString id = QFileInfo(fileName).baseName();
    const QString json = QStringLiteral(R"({
        "id": "%1",
        "title": "Connection Requirement",
        "steps": [
            { "key": "step0", "instruction": "Connect elements",
              "requiredConnections": [ { "fromType": "%2", "toType": "%3", "minCount": %4 } ] },
            { "key": "step1", "instruction": "Done" }
        ]
    })").arg(id, fromType, toType).arg(minCount);
    return writeJsonFile(dir, fileName, json.toUtf8());
}

} // namespace

void TestExerciseEngine::testRetranslateBeforeLoadIsNoOp()
{
    ExerciseEngine engine;
    QSignalSpy retranslatedSpy(&engine, &ExerciseEngine::retranslated);

    engine.retranslate();

    QVERIFY(!engine.isActive());
    QCOMPARE(retranslatedSpy.count(), 0);
}

void TestExerciseEngine::testRetranslateWhileInactiveIsNoOp()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    QVERIFY(!engine.isActive()); // loadFromResource() alone never activates the engine

    QSignalSpy retranslatedSpy(&engine, &ExerciseEngine::retranslated);
    engine.retranslate();

    QVERIFY(!engine.isActive());
    QCOMPARE(retranslatedSpy.count(), 0);
}

void TestExerciseEngine::testRetranslatePreservesProgressAndData()
{
    const QString originalLang = Settings::language();
    Settings::setLanguage("en");
    Settings::setExerciseProgress(QLatin1String(kExerciseFixtureId), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    engine.advanceStep();

    QCOMPARE(engine.currentStep(), 1);
    QVERIFY(engine.isActive());

    engine.retranslate();

    QCOMPARE(engine.currentStep(), 1);
    QVERIFY(engine.isActive());
    QCOMPARE(engine.exerciseTitle(), QStringLiteral("Test Exercise"));
    QCOMPARE(engine.currentStepData().instruction, QStringLiteral("Instruction 1"));
    QCOMPARE(engine.currentStepData().hint, QStringLiteral("Hint 1"));

    Settings::setLanguage(originalLang);
}

void TestExerciseEngine::testRetranslateEmitsRetranslatedOnly()
{
    const QString originalLang = Settings::language();
    Settings::setLanguage("en");
    Settings::setExerciseProgress(QLatin1String(kExerciseFixtureId), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    QSignalSpy retranslatedSpy(&engine, &ExerciseEngine::retranslated);
    QSignalSpy stepChangedSpy(&engine, &ExerciseEngine::stepChanged);
    QSignalSpy stepCompletedSpy(&engine, &ExerciseEngine::stepCompleted);

    engine.retranslate();

    QCOMPARE(retranslatedSpy.count(), 1);
    QCOMPARE(stepChangedSpy.count(), 0);
    QCOMPARE(stepCompletedSpy.count(), 0);

    Settings::setLanguage(originalLang);
}

void TestExerciseEngine::testNegativeMinCountClampsToZero()
{
    // Regression: a negative minCount (typo'd or malicious custom exercise JSON — Exercise/Tour
    // content is end-user-writable) silently auto-satisfied "count < minCount" since any
    // non-negative count is always >= a negative number, permanently skipping the check.
    const char *const json = R"({
        "id": "test-negative-mincount",
        "title": "Test",
        "steps": [
            {
                "key": "step0",
                "instruction": "Instruction",
                "requiredElements": [ { "type": "And", "minCount": -5 } ],
                "requiredConnections": [ { "fromType": "And", "toType": "Or", "minCount": -3 } ]
            }
        ]
    })";

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("negative_mincount.json");
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(json);
    }

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));

    const ExerciseStep &step = engine.currentStepData();
    QCOMPARE(step.requiredElements.size(), 1);
    QCOMPARE(step.requiredElements[0].minCount, 0);
    QCOMPARE(step.requiredConnections.size(), 1);
    QCOMPARE(step.requiredConnections[0].minCount, 0);
}

void TestExerciseEngine::testUnknownElementTypeWarnsAndNeverAdvances()
{
    // Regression: a typo'd/unknown "type" in a step's requiredElements (Exercise content is
    // end-user-writable) made validateElements() return false unconditionally, permanently
    // stuck-ing the step with no diagnostic anywhere — unlike every other malformed-content
    // path in this feature (e.g. ExerciseTourResources::scan()), which logs a qWarning().
    const char *const json = R"({
        "id": "test-unknown-type",
        "title": "Test",
        "steps": [
            {
                "key": "step0",
                "instruction": "Instruction",
                "requiredElements": [ { "type": "NotARealElementType", "minCount": 1 } ]
            }
        ]
    })";

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("unknown_type.json");
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(json);
    }

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();

    QTest::ignoreMessage(QtWarningMsg,
        QRegularExpression(".*ExerciseEngine::validateElements.*NotARealElementType.*"));

    auto *elm = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->receiveCommand(new AddItemsCommand({elm}, workspace.scene()));

    // The step's only requirement references an unknown type, so it can never be satisfied —
    // the engine must stay on step 0 no matter what the user builds.
    QCOMPARE(engine.currentStep(), 0);
    QVERIFY(engine.isActive());
}

void TestExerciseEngine::testOverlayFontScalesWithApplicationFont()
{
    const QFont originalFont = QApplication::font();

    QFont smallFont = originalFont;
    smallFont.setPointSize(8);
    QApplication::setFont(smallFont);
    const int smallPx = ExerciseOverlay::scaledFontPx(13);

    QFont largeFont = originalFont;
    largeFont.setPointSize(24);
    QApplication::setFont(largeFont);
    const int largePx = ExerciseOverlay::scaledFontPx(13);

    QApplication::setFont(originalFont);

    QVERIFY2(largePx > smallPx, "scaledFontPx must grow with the application's font size, "
                                "so exercise overlay text respects an OS/Qt font-scale setting");
}

void TestExerciseEngine::testLoadFromResourceMissingFileFails()
{
    ExerciseEngine engine;
    QVERIFY(!engine.loadFromResource("/nonexistent/path/to/exercise.json"));
    QVERIFY(!engine.isActive());
}

void TestExerciseEngine::testLoadFromResourceInvalidJsonFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "garbage.json", "this is not valid JSON {{{");

    ExerciseEngine engine;
    QVERIFY(!engine.loadFromResource(path));
}

void TestExerciseEngine::testLoadFromResourceMissingIdOrTitleFails()
{
    const char *const json = R"({
        "title": "No Id Here",
        "steps": [ { "instruction": "x" } ]
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "no_id.json", json);

    ExerciseEngine engine;
    QVERIFY(!engine.loadFromResource(path));
}

void TestExerciseEngine::testLoadFromResourceEmptyStepsArrayFails()
{
    const char *const json = R"({
        "id": "empty-steps",
        "title": "Empty",
        "steps": []
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "empty_steps.json", json);

    ExerciseEngine engine;
    QVERIFY(!engine.loadFromResource(path));
}

void TestExerciseEngine::testStepWithoutKeyUsesRawInstructionAndHintDirectly()
{
    // Without a "key", fillExerciseStepFields() must use the raw JSON text directly rather
    // than looking it up in the (empty, for this made-up id) translation catalog.
    const char *const json = R"({
        "id": "no-key-exercise",
        "title": "No Key",
        "steps": [ { "instruction": "Raw instruction text", "hint": "Raw hint text" } ]
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "no_key.json", json);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    QCOMPARE(engine.totalSteps(), 1);
    QCOMPARE(engine.currentStepData().instruction, QStringLiteral("Raw instruction text"));
    QCOMPARE(engine.currentStepData().hint, QStringLiteral("Raw hint text"));
}

void TestExerciseEngine::testSetSceneReconnectsWhenCalledTwice()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeElementRequirementExercise(dir, "And", 1, "and_req_reconnect.json");

    WorkSpace workspaceA;
    WorkSpace workspaceB;

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspaceA.scene());
    engine.start();

    engine.setScene(workspaceB.scene()); // must disconnect workspaceA's signal

    auto *elmA = ElementFactory::buildElement(ElementType::And);
    workspaceA.scene()->receiveCommand(new AddItemsCommand({elmA}, workspaceA.scene()));
    QCOMPARE(engine.currentStep(), 0); // workspaceA changes must no longer be observed

    auto *elmB = ElementFactory::buildElement(ElementType::And);
    workspaceB.scene()->receiveCommand(new AddItemsCommand({elmB}, workspaceB.scene()));
    QCOMPARE(engine.currentStep(), 1); // workspaceB changes ARE observed
}

void TestExerciseEngine::testSetSceneWhileActiveConnectsImmediately()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeElementRequirementExercise(dir, "And", 1, "and_req_late_scene.json");

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start(); // active, but no scene attached yet

    engine.setScene(workspace.scene()); // must connect immediately since isActive() is true

    auto *elm = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->receiveCommand(new AddItemsCommand({elm}, workspace.scene()));

    QCOMPARE(engine.currentStep(), 1);
}

void TestExerciseEngine::testStartWithoutLoadIsNoOp()
{
    ExerciseEngine engine;
    QSignalSpy stepChangedSpy(&engine, &ExerciseEngine::stepChanged);

    engine.start();

    QVERIFY(!engine.isActive());
    QCOMPARE(stepChangedSpy.count(), 0);
}

void TestExerciseEngine::testAdvanceStepWhileInactiveIsNoOp()
{
    // StepEngineCore::advance()'s own "!active || steps empty" guard, reached via
    // ExerciseEngine::advanceStep() on an engine that was never started.
    ExerciseEngine engine;
    QSignalSpy stepChangedSpy(&engine, &ExerciseEngine::stepChanged);
    QSignalSpy stepCompletedSpy(&engine, &ExerciseEngine::stepCompleted);
    QSignalSpy completedSpy(&engine, &ExerciseEngine::exerciseCompleted);

    engine.advanceStep();

    QCOMPARE(engine.currentStep(), 0);
    QCOMPARE(stepChangedSpy.count(), 0);
    QCOMPARE(stepCompletedSpy.count(), 0);
    QCOMPARE(completedSpy.count(), 0);
}

void TestExerciseEngine::testStopWhileActiveDisconnectsAndEmits()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeElementRequirementExercise(dir, "And", 1, "and_req_stop.json");

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();
    QVERIFY(engine.isActive());

    QSignalSpy stoppedSpy(&engine, &ExerciseEngine::exerciseStopped);
    engine.stop();

    QVERIFY(!engine.isActive());
    QCOMPARE(stoppedSpy.count(), 1);

    // The scene must be disconnected: adding a satisfying element now must not advance anything.
    auto *elm = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->receiveCommand(new AddItemsCommand({elm}, workspace.scene()));
    QCOMPARE(engine.currentStep(), 0);
    QVERIFY(!engine.isActive());
}

void TestExerciseEngine::testStopWhileInactiveIsNoOp()
{
    ExerciseEngine engine;
    QSignalSpy stoppedSpy(&engine, &ExerciseEngine::exerciseStopped);

    engine.stop();

    QCOMPARE(stoppedSpy.count(), 0);
}

void TestExerciseEngine::testGoToPreviousStepMovesBackAndPersists()
{
    Settings::setExerciseProgress(QLatin1String(kExerciseFixtureId), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    engine.advanceStep();
    QCOMPARE(engine.currentStep(), 1);

    QSignalSpy stepChangedSpy(&engine, &ExerciseEngine::stepChanged);
    engine.goToPreviousStep();

    QCOMPARE(engine.currentStep(), 0);
    QCOMPARE(stepChangedSpy.count(), 1);
    QCOMPARE(Settings::exerciseProgress(QLatin1String(kExerciseFixtureId)), 0);
}

void TestExerciseEngine::testGoToPreviousStepAtFirstStepIsNoOp()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    QSignalSpy stepChangedSpy(&engine, &ExerciseEngine::stepChanged);
    engine.goToPreviousStep();

    QCOMPARE(engine.currentStep(), 0);
    QCOMPARE(stepChangedSpy.count(), 0);
}

void TestExerciseEngine::testGoToPreviousStepWhileInactiveIsNoOp()
{
    ExerciseEngine engine;
    QSignalSpy stepChangedSpy(&engine, &ExerciseEngine::stepChanged);

    engine.goToPreviousStep();

    QCOMPARE(stepChangedSpy.count(), 0);
}

void TestExerciseEngine::testAdvanceStepReachesEndMarksCompletedAndEmits()
{
    // Reaching the last step's advance() call must mark the exercise completed -- never
    // exercised before (existing tests only ever advance partway through a multi-step
    // exercise).
    const char *const json = R"({
        "id": "single-step-exercise",
        "title": "Single Step",
        "steps": [ { "key": "only", "instruction": "Just look" } ]
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "single_step.json", json);

    const QString exerciseId = QStringLiteral("single-step-exercise");
    Settings::setExerciseProgress(exerciseId, 0);
    QStringList completed = Settings::completedExercises();
    completed.removeAll(exerciseId);
    Settings::setCompletedExercises(completed);

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();

    QSignalSpy completedSpy(&engine, &ExerciseEngine::exerciseCompleted);
    QSignalSpy stepCompletedSpy(&engine, &ExerciseEngine::stepCompleted);
    engine.advanceStep();

    QCOMPARE(completedSpy.count(), 1);
    QCOMPARE(stepCompletedSpy.count(), 0); // reachedEnd path, not the advanced path
    QVERIFY(!engine.isActive());
    QVERIFY(Settings::completedExercises().contains(exerciseId));

    // markCompleted() must also disconnect the scene, mirroring stop()'s own disconnect.
    auto *elm = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->receiveCommand(new AddItemsCommand({elm}, workspace.scene()));
    QVERIFY(!engine.isActive());
    QCOMPARE(Settings::exerciseProgress(exerciseId), 0);
}

void TestExerciseEngine::testRetranslateReloadFailureDeactivatesEngine()
{
    // If the underlying resource becomes unreadable between load and a later retranslate()
    // (e.g. a user-writable custom exercise file deleted mid-session), loadFromResource()'s own
    // unconditional state reset at entry leaves the engine deactivated even though retranslate()
    // itself just returns false -- a real (if narrow) consequence of the reset-before-parse
    // ordering, documented here rather than assumed.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    QVERIFY(engine.isActive());

    QVERIFY(QFile::remove(path));

    QSignalSpy retranslatedSpy(&engine, &ExerciseEngine::retranslated);
    engine.retranslate();

    QCOMPARE(retranslatedSpy.count(), 0);
    QVERIFY(!engine.isActive());
}

void TestExerciseEngine::testCurrentStepDataBeforeLoadReturnsEmptyStep()
{
    ExerciseEngine engine;
    const ExerciseStep &step = engine.currentStepData();

    QVERIFY(step.instruction.isEmpty());
    QVERIFY(step.hint.isEmpty());
    QVERIFY(step.requiredElements.isEmpty());
    QVERIFY(step.requiredConnections.isEmpty());
}

void TestExerciseEngine::testOnCircuitChangedGuardsAgainstInactiveOrSceneless()
{
    // validateCurrentStep()'s "!isActive() || totalSteps()==0 || !scene" guard is normally
    // unreachable through the real signal path: onCircuitChanged() is only ever connected while
    // active and scene-bound (setScene()/start()), and Qt auto-disconnects the connection if the
    // scene is destroyed. Invoke the private slot directly (a real, meta-object-registered call,
    // not a hand-rolled substitute) to exercise the guard itself.
    ExerciseEngine engine; // never loaded, never started, no scene
    QMetaObject::invokeMethod(&engine, "onCircuitChanged");

    QCOMPARE(engine.currentStep(), 0);
    QVERIFY(!engine.isActive());
}

void TestExerciseEngine::testOnCircuitChangedDoesNotAdvanceOnObserveStep()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir); // both steps are observe steps

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();

    auto *elm = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->receiveCommand(new AddItemsCommand({elm}, workspace.scene()));

    QCOMPARE(engine.currentStep(), 0);
    QVERIFY(engine.isActive());
}

void TestExerciseEngine::testOnCircuitChangedAutoAdvancesWhenElementRequirementSatisfied()
{
    // The exercise "happy path": a real circuit change that actually satisfies a step's
    // requirement must auto-advance -- previously only the always-fails unknown-type path
    // exercised any part of this validation.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeElementRequirementExercise(dir, "And", 1, "and_req_satisfied.json");

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();
    QCOMPARE(engine.currentStep(), 0);

    QSignalSpy stepCompletedSpy(&engine, &ExerciseEngine::stepCompleted);

    auto *elm = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->receiveCommand(new AddItemsCommand({elm}, workspace.scene()));

    QCOMPARE(stepCompletedSpy.count(), 1);
    QCOMPARE(engine.currentStep(), 1);
}

void TestExerciseEngine::testOnCircuitChangedDoesNotAdvanceWhenElementCountInsufficient()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeElementRequirementExercise(dir, "And", 2, "and_req_insufficient.json");

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();

    auto *elm = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->receiveCommand(new AddItemsCommand({elm}, workspace.scene()));

    QCOMPARE(engine.currentStep(), 0); // only 1 And, minCount is 2
}

void TestExerciseEngine::testValidateConnectionsUnknownTypeWarnsAndNeverAdvances()
{
    const char *const json = R"({
        "id": "unknown-connection-type",
        "title": "Test",
        "steps": [
            { "key": "step0", "instruction": "Connect",
              "requiredConnections": [ { "fromType": "NotAType", "toType": "Or", "minCount": 1 } ] }
        ]
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "unknown_conn.json", json);

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();

    QTest::ignoreMessage(QtWarningMsg,
        QRegularExpression(".*ExerciseEngine::validateConnections.*NotAType.*"));

    auto *elm = ElementFactory::buildElement(ElementType::Or);
    workspace.scene()->receiveCommand(new AddItemsCommand({elm}, workspace.scene()));

    QCOMPARE(engine.currentStep(), 0);
    QVERIFY(engine.isActive());
}

void TestExerciseEngine::testOnCircuitChangedAutoAdvancesWhenConnectionRequirementSatisfied()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeConnectionRequirementExercise(dir, "And", "Or", 1, "and_or_conn.json");

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *orGate  = ElementFactory::buildElement(ElementType::Or);
    workspace.scene()->receiveCommand(new AddItemsCommand({andGate, orGate}, workspace.scene()));
    QCOMPARE(engine.currentStep(), 0); // elements alone don't satisfy a connection requirement

    auto *conn = new Connection();
    conn->setStartPort(andGate->outputPort());
    conn->setEndPort(orGate->inputPort(0));
    workspace.scene()->receiveCommand(new AddItemsCommand({conn}, workspace.scene()));

    QCOMPARE(engine.currentStep(), 1);
}

void TestExerciseEngine::testOnCircuitChangedDoesNotAdvanceWhenConnectionCountInsufficient()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeConnectionRequirementExercise(dir, "And", "Or", 2, "and_or_conn_2.json");

    WorkSpace workspace;
    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.setScene(workspace.scene());
    engine.start();

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *orGate  = ElementFactory::buildElement(ElementType::Or);
    workspace.scene()->receiveCommand(new AddItemsCommand({andGate, orGate}, workspace.scene()));

    auto *conn = new Connection();
    conn->setStartPort(andGate->outputPort());
    conn->setEndPort(orGate->inputPort(0));
    workspace.scene()->receiveCommand(new AddItemsCommand({conn}, workspace.scene()));

    QCOMPARE(engine.currentStep(), 0); // only 1 connection, minCount is 2
}

void TestExerciseEngine::testOverlayConstructionBuildsUiWidgets()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    QVERIFY(overlay.m_stepCounter != nullptr);
    QVERIFY(overlay.m_instructionLabel != nullptr);
    QVERIFY(overlay.m_hintLabel != nullptr);
    QVERIFY(overlay.m_hintButton != nullptr);
    QVERIFY(overlay.m_prevButton != nullptr);
    QVERIFY(overlay.m_nextButton != nullptr);
    QVERIFY(overlay.m_closeButton != nullptr);

    QVERIFY(overlay.m_hintLabel->isHidden());
    QVERIFY(!overlay.m_nextButton->isEnabled()); // disabled until a real step is applied
    QCOMPARE(overlay.width(), 480);
}

void TestExerciseEngine::testOverlayAppliesThemeReactsToThemeChange()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    ThemeManager::setTheme(Theme::Light);
    const QColor lightBg = overlay.m_bgColor;

    ThemeManager::setTheme(Theme::Dark);
    const QColor darkBg = overlay.m_bgColor;

    QVERIFY2(lightBg != darkBg, "applyTheme() must pick different colors for light vs dark");

    ThemeManager::setTheme(Theme::Light);
}

void TestExerciseEngine::testOverlayOnStepChangedUpdatesLabelsAndButtons()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    ExerciseStep step;
    step.instruction = QStringLiteral("Do the thing");
    step.hint = QStringLiteral("Here's a hint");
    step.requiredElements.append({QStringLiteral("And"), 1});

    overlay.onStepChanged(0, 2, step);

    QCOMPARE(overlay.m_instructionLabel->text(), QStringLiteral("Do the thing"));
    QCOMPARE(overlay.m_hintLabel->text(), QStringLiteral("Here's a hint"));
    QVERIFY(!overlay.m_nextButton->isEnabled()); // not an observe step
    QVERIFY(!overlay.m_prevButton->isEnabled()); // step 0
}

void TestExerciseEngine::testOverlayOnStepChangedObserveStepEnablesNext()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    ExerciseStep observeStep; // both requirement vectors empty
    overlay.onStepChanged(1, 3, observeStep);

    QVERIFY(overlay.m_nextButton->isEnabled());
    QVERIFY(overlay.m_prevButton->isEnabled()); // step > 0
}

void TestExerciseEngine::testOverlayOnStepChangedLastStepShowsFinish()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    ExerciseStep step;
    overlay.onStepChanged(2, 3, step); // step == total - 1

    QCOMPARE(overlay.m_nextButton->text(), QStringLiteral("Finish"));
}

void TestExerciseEngine::testOverlayOnStepChangedResetsHintVisibility()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    overlay.m_hintVisible = true;
    overlay.m_hintLabel->show();
    overlay.m_hintButton->setText(QStringLiteral("Hide hint"));

    ExerciseStep step;
    overlay.onStepChanged(0, 2, step);

    QVERIFY(!overlay.m_hintVisible);
    QVERIFY(overlay.m_hintLabel->isHidden());
    QCOMPARE(overlay.m_hintButton->text(), QStringLiteral("Hint"));
}

void TestExerciseEngine::testOverlayHintButtonTogglesVisibility()
{
    // The overlay is never shown (no top-level ancestor), so isVisible() would always read
    // false regardless of internal show()/hide() calls -- isHidden() tracks the widget's own
    // explicit hidden flag instead, independent of ancestor visibility.
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    QCOMPARE(overlay.m_hintButton->text(), QStringLiteral("Hint"));
    QVERIFY(overlay.m_hintLabel->isHidden());

    QTest::mouseClick(overlay.m_hintButton, Qt::LeftButton);
    QVERIFY(!overlay.m_hintLabel->isHidden());
    QCOMPARE(overlay.m_hintButton->text(), QStringLiteral("Hide hint"));

    QTest::mouseClick(overlay.m_hintButton, Qt::LeftButton);
    QVERIFY(overlay.m_hintLabel->isHidden());
    QCOMPARE(overlay.m_hintButton->text(), QStringLiteral("Hint"));
}

void TestExerciseEngine::testOverlayPrevButtonCallsGoToPreviousStep()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    engine.advanceStep();
    QCOMPARE(engine.currentStep(), 1);

    ExerciseOverlay overlay(&engine);
    overlay.m_prevButton->click();

    QCOMPARE(engine.currentStep(), 0);
}

void TestExerciseEngine::testOverlayNextButtonCallsAdvanceStep()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir); // observe steps: Next is enabled

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    ExerciseOverlay overlay(&engine);
    overlay.onStepChanged(engine.currentStep(), engine.totalSteps(), engine.currentStepData());
    QVERIFY(overlay.m_nextButton->isEnabled());

    overlay.m_nextButton->click();

    QCOMPARE(engine.currentStep(), 1);
}

void TestExerciseEngine::testOverlayCloseButtonEmitsCloseRequested()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    QSignalSpy closeSpy(&overlay, &ExerciseOverlay::closeRequested);
    overlay.m_closeButton->click();

    QCOMPARE(closeSpy.count(), 1);
}

void TestExerciseEngine::testOverlayOnRetranslatedUpdatesTextsFromCurrentEngineState()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    engine.advanceStep();
    QCOMPARE(engine.currentStep(), 1);

    ExerciseOverlay overlay(&engine);
    overlay.onRetranslated();

    QCOMPARE(overlay.m_instructionLabel->text(), engine.currentStepData().instruction);
    QCOMPARE(overlay.m_hintLabel->text(), engine.currentStepData().hint);
    QCOMPARE(overlay.m_closeButton->text(), QStringLiteral("Exit"));
    QCOMPARE(overlay.m_prevButton->text(), QStringLiteral("← Back"));
}

void TestExerciseEngine::testOverlayOnExerciseCompletedShowsCompletionUiAndClosesOnNextClick()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    QVERIFY(!overlay.m_hintButton->isHidden());
    QVERIFY(!overlay.m_prevButton->isHidden());

    overlay.onExerciseCompleted();

    QCOMPARE(overlay.m_instructionLabel->text(), QStringLiteral("Exercise complete! Well done."));
    QVERIFY(overlay.m_hintLabel->isHidden());
    QVERIFY(overlay.m_hintButton->isHidden());
    QVERIFY(overlay.m_prevButton->isHidden());
    QCOMPARE(overlay.m_nextButton->text(), QStringLiteral("Close"));
    QVERIFY(overlay.m_nextButton->isEnabled());

    QSignalSpy closeSpy(&overlay, &ExerciseOverlay::closeRequested);
    QTest::mouseClick(overlay.m_nextButton, Qt::LeftButton);
    QCOMPARE(closeSpy.count(), 1);
}

void TestExerciseEngine::testOverlayRepositionToParentMovesToBottomCenterOfParent()
{
    QWidget parent;
    parent.resize(800, 600);

    ExerciseEngine engine;
    auto *overlay = new ExerciseOverlay(&engine, &parent);
    overlay->repositionToParent();

    const int expectedX = (parent.width() - overlay->width()) / 2;
    const int expectedY = parent.height() - overlay->height() - 12;
    QCOMPARE(overlay->pos(), QPoint(expectedX, expectedY));
}

void TestExerciseEngine::testOverlayRepositionToParentNoParentIsNoOp()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);

    const QPoint posBefore = overlay.pos();
    overlay.repositionToParent();

    QCOMPARE(overlay.pos(), posBefore);
}

void TestExerciseEngine::testOverlayPaintEventDrawsRoundedBackground()
{
    ExerciseEngine engine;
    ExerciseOverlay overlay(&engine);
    overlay.resize(480, 200);

    const QPixmap pixmap = overlay.grab();
    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "paintEvent() must draw a visible rounded background");
}
