// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/**
 * \class TestTourEngine
 * \brief Domain-layer (TourEngine-only) test coverage.
 *
 * \details TourOverlay (the spotlight/callout widget) is listed under CMakeSources.cmake's
 * WIDGETS_ONLY_SOURCES, so it isn't compiled into wiredpanda_lib and has no coverage here.
 * TourEngine.cpp -- the pure state-machine/JSON-loading logic every test below exercises -- is
 * part of the portable library.
 */
class TestTourEngine : public QObject
{
    Q_OBJECT

private slots:
    void testRetranslateBeforeLoadIsNoOp();
    void testRetranslateWhileInactiveIsNoOp();
    void testRetranslatePreservesProgressAndData();
    void testRetranslateEmitsRetranslatedOnly();

    void testStartWithoutLoadIsNoOp();
    void testAdvanceStepWhileInactiveIsNoOp();
    void testStopWhileActiveEmitsAndDeactivates();
    void testStopWhileInactiveIsNoOp();
    void testGoToPreviousStepMovesBackAndPersists();
    void testGoToPreviousStepAtFirstStepIsNoOp();
    void testGoToPreviousStepWhileInactiveIsNoOp();
    void testAdvanceStepReachesEndMarksCompletedAndEmits();
    void testLoadFromResourceMissingFileFails();
    void testLoadFromResourceInvalidJsonFails();
    void testLoadFromResourceMissingIdOrTitleFails_data();
    void testLoadFromResourceMissingIdOrTitleFails();
    void testLoadFromResourceEmptyStepsArrayFails();
    void testStepWithoutKeyUsesRawTitleAndBodyDirectly();
    void testStepTargetAndClickFieldsParsedFromJson();
    void testCurrentStepDataBeforeLoadReturnsEmptyStep();
    void testRetranslateReloadFailureDeactivatesEngine();
};
