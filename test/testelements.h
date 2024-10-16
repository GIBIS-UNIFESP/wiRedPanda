// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

class IC;
class InputSwitch;
class QNEConnection;

class TestElements : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();
    void testAnd();
    void testDFlipFlop();
    void testDLatch();
    void testDemux();
    void testGND();
    void testIC();
    void testICs();
    void testJKFlipFlop();
    void testMux();
    void testNode();
    void testOr();
    void testSRFlipFlop();
    void testTFlipFlop();
    void testVCC();

private:
    void testICData(IC *ic);

    QVector<InputSwitch *> switches{5};
    QVector<QNEConnection *> connections{5};
};
