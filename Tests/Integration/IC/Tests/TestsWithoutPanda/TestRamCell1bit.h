// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestRAMCell1Bit : public QObject
{
    Q_OBJECT

private slots:

    void testWriteZeroReadZero();
    void testWriteOneReadOne();
    void testWriteOneHoldWeDisabled();
    void testWriteZeroHoldWeDisabled();
    void testWriteOneMultipleHolds();
    void testWeDisabledFromStart();
    void testRapidWeToggle();
    void testDataChangeWithoutWe();
    void testHoldMultipleCycles();
    void testWriteZeroThenOneWithHold();
};

