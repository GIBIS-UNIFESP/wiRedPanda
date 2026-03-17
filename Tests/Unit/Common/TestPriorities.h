// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestPriorities : public QObject
{
    Q_OBJECT

private slots:

    // Priority Calculation Tests (2 tests) — used by Scene::sortByTopology for codegen
    void testPriorityBasic();
    void testPriorityMemoization();

    // Feedback Detection Tests (7 tests)
    void testFeedbackNoCycle();
    void testFeedbackSimpleCycle();
    void testFeedbackSelfLoop();
    void testFeedbackCycleWithTail();
    void testFeedbackMultipleCycles();
    void testFeedbackTriangleCycle();
    void testFeedbackDiamond();
};
