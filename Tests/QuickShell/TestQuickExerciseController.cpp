// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickExerciseController.h"

#include <QSignalSpy>

#include "App/Core/Settings.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickExerciseController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Wiring/Connection.h"

namespace {

constexpr auto kAndGateExerciseId = "basic-and-gate";
constexpr auto kAndGateExercise = ":/Exercises/basic-and-gate.json";

/// Waits (spinning the event loop) until \a spy has at least one more signal than it did when
/// passed in, or the timeout elapses. ExerciseEngine::onCircuitChanged() runs off
/// CanvasItem::undoStack()'s indexChanged signal, a direct (same-thread) connection -- so in
/// practice this always resolves on the very first iteration, but waiting rather than asserting
/// synchronously keeps this robust against that connection type ever changing later.
bool waitForStepChange(QSignalSpy &spy, qsizetype previousCount)
{
    return QTest::qWaitFor([&] { return spy.size() > previousCount; }, 1000);
}

} // namespace

void TestQuickExerciseController::init()
{
    // ExerciseEngine::start() resumes from Settings-persisted progress (StepEngineCore::start()'s
    // m_persistence.getProgress(m_id)) -- real, intended "pick up where you left off" behavior,
    // backed by a real QSettings sandboxed to one temp dir for this whole test *process*
    // (QuickRunnerUtils.h), not per test function. Every test here shares the same bundled
    // "basic-and-gate" exercise id, so a prior test's advance()/completion would otherwise leak
    // into the next one's start() -- reset it fresh before each test.
    Settings::setExerciseProgress(kAndGateExerciseId, -1);
    QStringList completed = Settings::completedExercises();
    completed.removeAll(kAndGateExerciseId);
    Settings::setCompletedExercises(completed);
}

void TestQuickExerciseController::testStartRejectsEmptyResourcePath()
{
    QuickAppController controller;
    controller.newTab();
    QVERIFY(!controller.exercise()->start(""));
    QVERIFY(!controller.exercise()->isActive());
}

void TestQuickExerciseController::testStartRejectsMissingResource()
{
    QuickAppController controller;
    controller.newTab();
    QVERIFY(!controller.exercise()->start(":/Exercises/does_not_exist.json"));
    QVERIFY(!controller.exercise()->isActive());
}

void TestQuickExerciseController::testStartLoadsFirstStepAndActivates()
{
    QuickAppController controller;
    controller.newTab();
    auto *exercise = controller.exercise();

    QVERIFY(exercise->start(kAndGateExercise));
    QVERIFY(exercise->isActive());
    QCOMPARE(exercise->stepCounterText(), QStringLiteral("Step 1 of 5"));
    QCOMPARE(exercise->instruction(), QStringLiteral("Place an AND gate on the canvas"));
    QVERIFY(!exercise->hint().isEmpty());
    QVERIFY(!exercise->isHintVisible());
    QVERIFY(!exercise->isPrevEnabled()); // step 0
    QVERIFY(!exercise->isNextEnabled()); // requires a real And gate, not an observe step
    QCOMPARE(exercise->nextButtonText(), QStringLiteral("Next →"));
    QVERIFY(!exercise->isCompleted());
}

void TestQuickExerciseController::testStepAdvancesOnlyWhenRequirementsMet()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));

    QSignalSpy stepSpy(exercise, &QuickExerciseController::stepChanged);

    // Placing an unrelated element must not satisfy "place an And gate".
    canvas->receiveCommand(new CanvasAddItemsCommand({new InputSwitch()}, canvas));
    QVERIFY(!waitForStepChange(stepSpy, 0));
    QCOMPARE(exercise->stepCounterText(), QStringLiteral("Step 1 of 5"));

    // The real requirement: an And gate on the canvas.
    canvas->receiveCommand(new CanvasAddItemsCommand({new And()}, canvas));
    QVERIFY(waitForStepChange(stepSpy, 0));
    QCOMPARE(exercise->stepCounterText(), QStringLiteral("Step 2 of 5"));
    QCOMPARE(exercise->instruction(), QStringLiteral("Add two Input Switch elements to the canvas"));
}

void TestQuickExerciseController::testPreviousStepGoesBack()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));

    QSignalSpy stepSpy(exercise, &QuickExerciseController::stepChanged);
    canvas->receiveCommand(new CanvasAddItemsCommand({new And()}, canvas));
    QVERIFY(waitForStepChange(stepSpy, 0));
    QVERIFY(exercise->isPrevEnabled());

    exercise->goToPreviousStep();
    QCOMPARE(exercise->stepCounterText(), QStringLiteral("Step 1 of 5"));
    QVERIFY(!exercise->isPrevEnabled());
}

void TestQuickExerciseController::testToggleHintTogglesVisibilityAndButtonText()
{
    QuickAppController controller;
    controller.newTab();
    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));

    QVERIFY(!exercise->isHintVisible());
    QCOMPARE(exercise->hintButtonText(), QStringLiteral("Hint"));

    QSignalSpy hintSpy(exercise, &QuickExerciseController::hintVisibleChanged);
    exercise->toggleHint();
    QCOMPARE(hintSpy.size(), 1);
    QVERIFY(exercise->isHintVisible());
    QCOMPARE(exercise->hintButtonText(), QStringLiteral("Hide hint"));

    exercise->toggleHint();
    QCOMPARE(hintSpy.size(), 2);
    QVERIFY(!exercise->isHintVisible());
    QCOMPARE(exercise->hintButtonText(), QStringLiteral("Hint"));
}

void TestQuickExerciseController::testHintResetOnStepChange()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));

    exercise->toggleHint();
    QVERIFY(exercise->isHintVisible());

    QSignalSpy stepSpy(exercise, &QuickExerciseController::stepChanged);
    canvas->receiveCommand(new CanvasAddItemsCommand({new And()}, canvas));
    QVERIFY(waitForStepChange(stepSpy, 0));

    QVERIFY2(!exercise->isHintVisible(), "a new step must not carry over the previous step's hint visibility");
}

void TestQuickExerciseController::testObserveStepAllowsManualAdvance()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));

    // Build the full circuit the first 4 steps require, driving each real advance the same way
    // a live exercise does -- not synthetic step jumps.
    auto *andGate = new And();
    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *led = new Led();
    QSignalSpy stepSpy(exercise, &QuickExerciseController::stepChanged);

    qsizetype previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({andGate}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));

    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({sw1, sw2}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));

    // Connections must go through the undo stack (CanvasAddItemsCommand) like every other
    // circuit edit -- ExerciseEngine::onCircuitChanged() is wired to
    // CanvasItem::undoStack()->indexChanged (QuickExerciseEngineBinding.cpp), so a connection
    // added via a bare canvas->addItem() never triggers re-validation.
    auto *c1 = new Connection();
    c1->setStartPort(sw1->outputPort());
    c1->setEndPort(andGate->inputPort(0));
    auto *c2 = new Connection();
    c2->setStartPort(sw2->outputPort());
    c2->setEndPort(andGate->inputPort(1));
    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({c1, c2}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));

    // Step 4 ("add-output") requires the LED *and* its connection to the AND gate together --
    // adding just the element wouldn't satisfy it, so both go through in one command.
    auto *c3 = new Connection();
    c3->setStartPort(andGate->outputPort());
    c3->setEndPort(led->inputPort(0));
    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({led, c3}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));

    // Step 5 (index 4) is the observe step: no requirements, so it never auto-advances --
    // isNextEnabled() must be the only way forward.
    QCOMPARE(exercise->stepCounterText(), QStringLiteral("Step 5 of 5"));
    QVERIFY(exercise->isNextEnabled());
    QCOMPARE(exercise->nextButtonText(), QStringLiteral("Finish"));
}

void TestQuickExerciseController::testCompletingLastStepSetsCompletedState()
{
    QuickAppController controller;
    controller.newTab();
    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));

    // Skip straight past the requirement steps is not possible via the public API (by design --
    // advanceStep() only works on observe steps); reach the last step exactly the way
    // testObserveStepAllowsManualAdvance() does, then finish it.
    auto *canvas = controller.currentTab()->canvas();
    auto *andGate = new And();
    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *led = new Led();
    QSignalSpy stepSpy(exercise, &QuickExerciseController::stepChanged);
    qsizetype previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({andGate}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));
    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({sw1, sw2}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));
    auto *c1 = new Connection();
    c1->setStartPort(sw1->outputPort());
    c1->setEndPort(andGate->inputPort(0));
    auto *c2 = new Connection();
    c2->setStartPort(sw2->outputPort());
    c2->setEndPort(andGate->inputPort(1));
    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({c1, c2}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));
    auto *c3 = new Connection();
    c3->setStartPort(andGate->outputPort());
    c3->setEndPort(led->inputPort(0));
    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({led, c3}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));

    QSignalSpy completedSpy(exercise, &QuickExerciseController::completedChanged);
    exercise->advanceStep(); // the observe step's manual Finish
    QVERIFY(completedSpy.size() >= 1);
    QVERIFY(exercise->isCompleted());
    QCOMPARE(exercise->instruction(), QStringLiteral("Exercise complete! Well done."));
    QVERIFY(exercise->stepCounterText().isEmpty());
    QVERIFY(!exercise->isPrevEnabled());
    QCOMPARE(exercise->nextButtonText(), QStringLiteral("Close"));
}

void TestQuickExerciseController::testAdvanceStepAfterCompletionCloses()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));

    auto *andGate = new And();
    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *led = new Led();
    QSignalSpy stepSpy(exercise, &QuickExerciseController::stepChanged);
    qsizetype previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({andGate}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));
    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({sw1, sw2}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));
    auto *c1 = new Connection();
    c1->setStartPort(sw1->outputPort());
    c1->setEndPort(andGate->inputPort(0));
    auto *c2 = new Connection();
    c2->setStartPort(sw2->outputPort());
    c2->setEndPort(andGate->inputPort(1));
    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({c1, c2}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));
    auto *c3 = new Connection();
    c3->setStartPort(andGate->outputPort());
    c3->setEndPort(led->inputPort(0));
    previousCount = stepSpy.size();
    canvas->receiveCommand(new CanvasAddItemsCommand({led, c3}, canvas));
    QVERIFY(waitForStepChange(stepSpy, previousCount));

    QSignalSpy completedSpy(exercise, &QuickExerciseController::completedChanged);
    exercise->advanceStep(); // finishes the exercise
    QVERIFY(exercise->isCompleted());
    // StepEngineCore::markCompleted() deactivates the engine immediately, as part of finishing
    // (not a separate, later step()) -- isCompleted() and !isActive() are both true at once here.
    QVERIFY(!exercise->isActive());

    exercise->advanceStep(); // Next is now relabeled Close
    QVERIFY2(!exercise->isActive(), "advanceStep() once completed must close the exercise, not advance a non-existent next step");
    QVERIFY2(!exercise->isCompleted(), "closing a completed exercise must actually reset completed state, not leave the overlay stuck");
    QVERIFY(completedSpy.size() >= 2); // once for completion, once for the close-driven reset
}

void TestQuickExerciseController::testCloseStopsExercise()
{
    QuickAppController controller;
    controller.newTab();
    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));
    QVERIFY(exercise->isActive());

    QSignalSpy activeSpy(exercise, &QuickExerciseController::activeChanged);
    exercise->close();
    QVERIFY(!exercise->isActive());
    QVERIFY(activeSpy.size() >= 1);
}

void TestQuickExerciseController::testSetCanvasBeforeActiveIsRememberedForNextStart()
{
    // Two tabs; the exercise is only ever started against whichever canvas is current at that
    // moment -- setCanvas() calls that happen while inactive (every tab switch before start())
    // are just remembered, not bound to the engine until start() runs.
    QuickAppController controller;
    controller.newTab();
    auto *firstCanvas = controller.currentTab()->canvas();
    controller.newTab();
    auto *secondCanvas = controller.currentTab()->canvas();
    QVERIFY(firstCanvas != secondCanvas);

    auto *exercise = controller.exercise();
    QVERIFY(exercise->start(kAndGateExercise));

    QSignalSpy stepSpy(exercise, &QuickExerciseController::stepChanged);

    // An And gate on the *first* (non-current) tab must not satisfy the step.
    firstCanvas->receiveCommand(new CanvasAddItemsCommand({new And()}, firstCanvas));
    QVERIFY(!waitForStepChange(stepSpy, 0));
    QCOMPARE(exercise->stepCounterText(), QStringLiteral("Step 1 of 5"));

    // The same element on the second (current, bound-at-start) tab does.
    secondCanvas->receiveCommand(new CanvasAddItemsCommand({new And()}, secondCanvas));
    QVERIFY(waitForStepChange(stepSpy, 0));
    QCOMPARE(exercise->stepCounterText(), QStringLiteral("Step 2 of 5"));
}
