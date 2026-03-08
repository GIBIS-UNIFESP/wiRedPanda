// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCPUControlUnit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testControlUnit();
    void testControlUnitFlagWrite();
    void testControlUnit_data();
};
