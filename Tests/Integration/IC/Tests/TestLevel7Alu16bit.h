// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel7ALU16Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testALU16Bit_data();
    void testALU16Bit();

    // Zero/Sign/Carry flags — wired in the fixture but never asserted by the
    // result-only test above. The 16-bit Zero flag is a distinct AND-of-both-
    // half-Zero-flags circuit (F26).
    void testFlags16Bit_data();
    void testFlags16Bit();
};
