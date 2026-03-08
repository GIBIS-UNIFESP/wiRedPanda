// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel5ProgramCounter4Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testProgramCounter4Bit_data();
    void testProgramCounter4Bit();

    void testProgramCounter4BitStructure();
};

