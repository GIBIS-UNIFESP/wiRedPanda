/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>
#include <QVector>

class LogicInput;

class TestLogicElements : public QObject
{
    Q_OBJECT

private:
    void cleanup();
    void init();
    void testLogicAnd();
    void testLogicDFlipFlop();
    void testLogicDLatch();
    void testLogicDemux();
    void testLogicInput();
    void testLogicJKFlipFlop();
    void testLogicMux();
    void testLogicNode();
    void testLogicOr();
    void testLogicSRFlipFlop();
    void testLogicTFlipFlop();

    QVector<LogicInput *> switches{5};
};
