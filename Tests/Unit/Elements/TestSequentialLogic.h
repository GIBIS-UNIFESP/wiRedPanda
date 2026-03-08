// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSequentialLogic : public QObject
{
    Q_OBJECT

private slots:
    // Port count and type verification
    void testDFlipFlop();
    void testDLatch();
    void testJKFlipFlop();
    void testSRFlipFlop();
    void testSRLatch();
    void testTFlipFlop();

    // Behavioral tests
    void testDFlipFlopBehavior();
    void testDLatchBehavior();
    void testJKFlipFlopBehavior();
    void testSRFlipFlopBehavior();
    void testSRLatchBehavior();
    void testTFlipFlopBehavior();
};

