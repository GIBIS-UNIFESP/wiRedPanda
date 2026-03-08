// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel6RippleAdder8Bit : public QObject {
    Q_OBJECT

private slots:
    // Functional Tests
    void test8BitAdder();

    // Structure Tests
    void test8BitAdderStructure();
};

