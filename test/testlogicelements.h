// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

class LogicInput;

class TestLogicElements : public QObject
{
    Q_OBJECT

private slots:
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
    void testLogicNAND();
    void testLogicNOR();
    void testLogicNOT();
    void testLogicXOR();
    void testLogicXNOR();
    void testLogicOutput();

private:
    QVector<LogicInput *> switches{5};
};
