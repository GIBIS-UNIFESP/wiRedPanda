// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestCPUBranch : public QObject
{
    Q_OBJECT

private slots:
    void testBranchCondition();
    void testBranchIntegration();
    void testBranchIntegration_data();
};
