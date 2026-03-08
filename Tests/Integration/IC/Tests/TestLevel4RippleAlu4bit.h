// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel4RippleALU4Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // ADD operation tests
    void testAluAdd_data();
    void testAluAdd();

    // AND operation tests
    void testAluAnd_data();
    void testAluAnd();

    // OR operation tests
    void testAluOr_data();
    void testAluOr();

    // SUB operation tests
    void testAluSub_data();
    void testAluSub();

    // Flag verification tests (all operations)
    void testAluFlags_data();
    void testAluFlags();
};

