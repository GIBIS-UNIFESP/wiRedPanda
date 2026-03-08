// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestMemorySettlingTime : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // ============================================================
    // Settling Time Benchmarks
    // ============================================================

    // T6.1: Single-stage 6-bit decoder settling time
    void testDecoderSettlingTime_singlestage_data();
    void testDecoderSettlingTime_singlestage();

    // T6.2: Cascaded 6-bit decoder settling time
    void testDecoderSettlingTime_cascaded_data();
    void testDecoderSettlingTime_cascaded();

    // T6.3: Address decoder vs mux settling comparison
    void testReadMuxSettlingTime_data();
    void testReadMuxSettlingTime();

    // T6.4: Performance benchmark summary
    void testSettlingTimeBenchmarkSummary();
};
