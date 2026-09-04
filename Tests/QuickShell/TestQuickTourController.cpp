// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickTourController.h"

#include <QSignalSpy>
#include <QStringList>

#include "App/Core/Settings.h"
#include "App/QuickShell/Chrome/QuickTourController.h"

namespace {
constexpr auto kUiOverviewTourId = "ui-overview";
constexpr auto kUiOverviewTour = ":/Tours/ui-overview.json";
} // namespace

void TestQuickTourController::init()
{
    // TourEngine::start() resumes from Settings-persisted progress (StepEngineCore::start()'s
    // m_persistence.getProgress(m_id)), same real, intended behavior as ExerciseEngine's -- see
    // TestQuickExerciseController::init()'s identical note. Reset before each test so tests
    // sharing this one bundled tour id don't see each other's persisted progress.
    Settings::setTourProgress(kUiOverviewTourId, -1);
    QStringList completed = Settings::completedTours();
    completed.removeAll(kUiOverviewTourId);
    Settings::setCompletedTours(completed);
}

void TestQuickTourController::testStartRejectsMissingResource()
{
    QuickTourController tour;
    QVERIFY(!tour.start(":/Tours/does_not_exist.json"));
    QVERIFY(!tour.isActive());
}

void TestQuickTourController::testStartLoadsFirstStepAndActivates()
{
    QuickTourController tour;
    QVERIFY(tour.start(kUiOverviewTour));
    QVERIFY(tour.isActive());
    QCOMPARE(tour.stepCounterText(), QStringLiteral("Step 1 of 10"));
    QCOMPARE(tour.title(), QStringLiteral("Welcome to wiRedPanda!"));
    QVERIFY(!tour.body().isEmpty());
    QVERIFY(tour.currentTarget().isEmpty()); // "welcome" step's target is ""
    QVERIFY(!tour.isPrevEnabled());
    QCOMPARE(tour.nextButtonText(), QStringLiteral("Next →"));
}

void TestQuickTourController::testAdvanceStepMovesForwardAndBack()
{
    QuickTourController tour;
    QVERIFY(tour.start(kUiOverviewTour));

    tour.advanceStep();
    QCOMPARE(tour.stepCounterText(), QStringLiteral("Step 2 of 10"));
    QCOMPARE(tour.title(), QStringLiteral("Component Palette"));
    QVERIFY(tour.isPrevEnabled());

    tour.goToPreviousStep();
    QCOMPARE(tour.stepCounterText(), QStringLiteral("Step 1 of 10"));
    QVERIFY(!tour.isPrevEnabled());
}

void TestQuickTourController::testCurrentTargetTracksStepTarget()
{
    QuickTourController tour;
    QVERIFY(tour.start(kUiOverviewTour));
    QVERIFY(tour.currentTarget().isEmpty());

    tour.advanceStep(); // "component-palette", target "elementPalette"
    QCOMPARE(tour.currentTarget(), QStringLiteral("elementPalette"));

    tour.advanceStep(); // "logic-gates", target "gatesTab"
    QCOMPARE(tour.currentTarget(), QStringLiteral("gatesTab"));
}

void TestQuickTourController::testClicksRequestedEmittedBeforeStepChanged()
{
    QuickTourController tour;
    QVERIFY(tour.start(kUiOverviewTour));
    tour.advanceStep(); // "component-palette" -- no "click" field

    QStringList emissionOrder;
    QStringList lastClickIds;
    connect(&tour, &QuickTourController::clicksRequested, [&](const QStringList &ids) {
        emissionOrder << "clicks";
        lastClickIds = ids;
    });
    connect(&tour, &QuickTourController::stepChanged, [&] { emissionOrder << "step"; });

    tour.advanceStep(); // "logic-gates": click: ["gatesTab"]
    QCOMPARE(emissionOrder, QStringList({"clicks", "step"}));
    QCOMPARE(lastClickIds, QStringList({"gatesTab"}));
}

void TestQuickTourController::testAdvancingPastLastStepFinishesTour()
{
    QuickTourController tour;
    QVERIFY(tour.start(kUiOverviewTour));
    QVERIFY(tour.isActive());

    for (int i = 0; i < 9; ++i) {
        tour.advanceStep();
    }
    QCOMPARE(tour.stepCounterText(), QStringLiteral("Step 10 of 10"));
    QCOMPARE(tour.nextButtonText(), QStringLiteral("Finish"));
    QVERIFY(tour.isActive());

    QSignalSpy activeSpy(&tour, &QuickTourController::activeChanged);
    tour.advanceStep(); // finishes the tour
    QVERIFY2(!tour.isActive(), "advancing past the last step must finish (deactivate) the tour");
    QVERIFY(activeSpy.size() >= 1);
}

void TestQuickTourController::testCloseStopsTour()
{
    QuickTourController tour;
    QVERIFY(tour.start(kUiOverviewTour));
    QVERIFY(tour.isActive());

    QSignalSpy activeSpy(&tour, &QuickTourController::activeChanged);
    tour.close();
    QVERIFY(!tour.isActive());
    QVERIFY(activeSpy.size() >= 1);
}
