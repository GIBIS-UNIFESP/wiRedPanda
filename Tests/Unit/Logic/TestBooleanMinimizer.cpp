// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestBooleanMinimizer.h"

#include <QTest>

#include "App/CodeGen/BooleanMinimizer.h"

void TestBooleanMinimizer::testMinimizesXor()
{
    QVector<int> inputs{0, 1};
    QVector<int> outputs{1, 1};
    auto result = BooleanMinimizer::minimize(inputs, outputs, 2, 1);
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0], QStringLiteral("A ^ B"));
}

void TestBooleanMinimizer::testMinimizesMajority()
{
    QVector<int> inputs{0, 1, 2};
    QVector<int> outputs{0, 1, 1, 1, 1, 1, 1, 1};
    auto result = BooleanMinimizer::minimize(inputs, outputs, 3, 1);
    QVERIFY(!result.isEmpty());
    QCOMPARE(result[0], QStringLiteral("(A & B) | (A & C) | (B & C)"));
}

void TestBooleanMinimizer::testMinimizesSingleMinterm()
{
    QVector<int> inputs{0, 1, 2};
    QVector<int> outputs{0, 0, 1, 0, 0, 0, 0, 0};
    auto result = BooleanMinimizer::minimize(inputs, outputs, 3, 1);
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0], QStringLiteral("!A & !B & C"));
}
