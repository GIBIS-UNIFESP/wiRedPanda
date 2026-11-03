// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Checks the reference's combinational kernels (plan 5, §4.5): on levels they equal master's engine, and on
 * every vector of the four values they are monotone in the flat order, with the non-level readings the rule gives.
 */
class TestReferenceKernels : public QObject
{
    Q_OBJECT

private slots:
    void testLevelsMatchMaster();
    void testRuleIsMonotoneInTheFlatOrder();
    void testNonLevelsReadBothWays();
    void testMemoryIsMonotoneOffTheKeptClock();
    void testMemoryHoldsLoadsAndKeepsItsClock();
    void testMemoryMatchesMasterOnStableData();
};
