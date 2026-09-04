// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Drives the real bundled ":/Exercises/basic-and-gate.json" fixture (5 real steps:
/// place-and-gate, add-inputs, connect-inputs, add-output, toggle-observe) end to end against a
/// real CanvasItem and real elements added via CanvasAddItemsCommand/Connection, the same way a
/// live exercise actually completes -- not synthetic step data.
class TestQuickExerciseController : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void testStartRejectsEmptyResourcePath();
    void testStartRejectsMissingResource();
    void testStartLoadsFirstStepAndActivates();
    void testStepAdvancesOnlyWhenRequirementsMet();
    void testPreviousStepGoesBack();
    void testToggleHintTogglesVisibilityAndButtonText();
    void testHintResetOnStepChange();
    void testObserveStepAllowsManualAdvance();
    void testCompletingLastStepSetsCompletedState();
    void testAdvanceStepAfterCompletionCloses();
    void testCloseStopsExercise();
    void testSetCanvasBeforeActiveIsRememberedForNextStart();
};
