// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel5UpDownCounter4Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testUpDownCounter_data();
    void testUpDownCounter();

    // Tests counting from a non-zero initial value
    void testUpDownCounterFromValue_data();
    void testUpDownCounterFromValue();
};

