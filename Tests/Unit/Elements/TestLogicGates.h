// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLogicGates : public QObject
{
    Q_OBJECT

private slots:
    // Basic gate property tests
    void testAnd();
    void testOr();
    void testNot();
    void testNand();
    void testNor();
    void testXor();
    void testXnor();

    // Behavioral tests - verify actual logic computation in circuits
    void testAndGateBehavior();
    void testOrGateBehavior();
    void testNotGateBehavior();
    void testNandGateBehavior();
    void testNorGateBehavior();
    void testXorGateBehavior();
    void testXnorGateBehavior();
};

