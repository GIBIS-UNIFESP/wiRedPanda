// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel3Comparator4BitEquality : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // 4-bit Equality Comparator Tests
    void testComparator4BitEquality_data();
    void testComparator4BitEquality();
};

