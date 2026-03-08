// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestDecoder8To256 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // 8-to-256 Decoder Tests (Memory Address Decoder)
    void test8to256Decoder_data();
    void test8to256Decoder();

    // Comprehensive address space test
    void test8to256DecoderAddressSpace_data();
    void test8to256DecoderAddressSpace();

    // One-hot property verification
    void test8to256DecoderOneHot_data();
    void test8to256DecoderOneHot();
};
