// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel4RAM4X1 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testRamArray4x4_data();
    void testRamArray4x4();
    void testPatternTest();
    void testWeGating();
    void testHoldBehavior();
    void testIsolation();
    void testSequentialWrite();
};

