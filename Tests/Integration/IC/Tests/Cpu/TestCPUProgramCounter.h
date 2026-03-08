// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCPUProgramCounter : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testProgramCounter();
    void testProgramCounter_data();
    void testProgramCounterPriority();
    void testProgramCounterPriority_data();
};

