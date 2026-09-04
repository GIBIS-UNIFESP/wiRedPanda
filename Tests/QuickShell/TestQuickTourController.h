// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Drives the real bundled ":/Tours/ui-overview.json" fixture (10 real steps). TourEngine has
/// no Scene/CanvasItem coupling, so these tests need no canvas/tab setup.
class TestQuickTourController : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void testStartRejectsMissingResource();
    void testStartLoadsFirstStepAndActivates();
    void testAdvanceStepMovesForwardAndBack();
    void testCurrentTargetTracksStepTarget();
    void testClicksRequestedEmittedBeforeStepChanged();
    void testAdvancingPastLastStepFinishesTour();
    void testCloseStopsTour();
};
