// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCPUBranch : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testBranchCondition();
    void testBranchIntegration();
    void testBranchIntegration_data();
    void testBranchSignFlag();
    void testBranchSignFlag_data();
    void testBranchUnconditional();
    void testBranchUnconditional_data();
    void testBranchZeroFlag();
    void testBranchZeroFlag_data();
};
