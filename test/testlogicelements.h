/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTLOGICELEMENTS_H
#define TESTLOGICELEMENTS_H

#include "logicelement.h"

#include <QObject>
#include <QTest>

#include "logicelement/logicinput.h"

class TestLogicElements : public QObject
{
    Q_OBJECT

    QVector<LogicInput *> sw{5};

public:
    explicit TestLogicElements(QObject *parent = nullptr);

private slots:
    void init();
    void cleanup();
    void testLogicNode();
    void testLogicAnd();
    void testLogicOr();
    void testLogicInput();
    void testLogicMux();
    void testLogicDemux();
    void testLogicDFlipFlop();
    void testLogicDLatch();
    void testLogicJKFlipFlop();
    void testLogicSRFlipFlop();
    void testLogicTFlipFlop();
};

#endif // TESTLOGICELEMENTS_H