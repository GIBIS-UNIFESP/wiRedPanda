// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCPUAlu : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testALU8bit();
    void testALU8bit_data();
    void testALU8bitFlags();
    void testALU8bitFlags_data();
};

