// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel6Register8Bit : public QObject {
    Q_OBJECT

private slots:
    // Functional Tests
    void test8BitRegister();
    void test8BitRegisterSequential();
    void test8BitRegisterEdgeCases();

    // Structure Tests
    void test8BitRegisterStructure();
};
