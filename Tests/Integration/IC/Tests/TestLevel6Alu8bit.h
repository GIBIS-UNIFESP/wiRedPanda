// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel6ALU8Bit : public QObject {
    Q_OBJECT

private slots:
    // Functional Tests
    void testALU8Bit();

    // Structure Tests
    void testALU8BitStructure();

    // Port Isolation Tests
    void testALU8BitInputPortIsolation_data();
    void testALU8BitInputPortIsolation();

    void testALU8BitOutputPortIsolation_data();
    void testALU8BitOutputPortIsolation();
};

