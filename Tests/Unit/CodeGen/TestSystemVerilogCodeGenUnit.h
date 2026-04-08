// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestSystemVerilogCodeGenUnit : public QObject
{
    Q_OBJECT

private slots:

    void testAndGateCircuit();
    void testOrNotCircuit();
    void testMultiGateChain();
    void testMuxCircuit();
    void testDemuxCircuit();
    void testEmptyScene();
};

