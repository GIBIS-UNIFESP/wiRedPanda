/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>
#include <QVector>

class IC;
class InputSwitch;
class QNEConnection;

class TestElements : public QObject
{
    Q_OBJECT

    QVector<QNEConnection *> connections{5};
    QVector<InputSwitch *> switches{5};
    void testICData(const IC *ic);
    QString testFile(const QString &fileName);

private slots:
    void init();
    void cleanup();

    void testNode();
    void testAnd();
    void testOr();
    void testVCC();
    void testGND();

    void testMux();
    void testDemux();

    void testDFlipFlop();
    void testDLatch();
    void testJKFlipFlop();
    void testSRFlipFlop();
    void testTFlipFlop();

    void testIC();
    void testICs();
};
