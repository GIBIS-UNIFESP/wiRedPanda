// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCommon : public QObject
{
    Q_OBJECT

private slots:

    // Topological Sort Tests (4 tests)
    void testSortSimpleChain();
    void testSortMultipleChains();
    void testSortCycleDetection();
    void testSortDisconnectedComponents();

    // Priority Calculation Tests (2 tests)
    void testPriorityBasic();
    void testPriorityMemoization();

    // Exception Tests (2 tests)
    void testPandaceptionMessage();
    void testPandaceptionEnglishMessage();
};
