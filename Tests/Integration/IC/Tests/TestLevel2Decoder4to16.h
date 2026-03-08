// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel2Decoder4To16 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    // 4-to-16 Decoder Tests
    void test4to16Decoder_data();
    void test4to16Decoder();

    // Edge Cases
    void test4to16DecoderMaxAddress_data();
    void test4to16DecoderMaxAddress();
};

