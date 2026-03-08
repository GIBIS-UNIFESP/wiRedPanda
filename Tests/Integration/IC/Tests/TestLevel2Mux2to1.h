// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel2MUX2To1 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // 2-to-1 Multiplexer Tests
    void testMux2to1_data();
    void testMux2to1();
};

