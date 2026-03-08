// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCPUInstructionFetch : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testInstructionFetch();
    void testInstructionFetchBasic();
    void testInstructionFetchBasic_data();
    void testInstructionFetchEdgeCases();
    void testInstructionFetchEdgeCases_data();
    void testInstructionFetchSequences();
    void testInstructionFetchSequences_data();
    void testInstructionFetch_data();
};

