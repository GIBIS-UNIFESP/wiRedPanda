// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestLevel2Decoder5To32 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    // 5-to-32 Decoder Tests
    void test5to32Decoder_data();
    void test5to32Decoder();

    void testEnableGating();
};
