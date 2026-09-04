// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTest>

class TestMUXDEMUXComprehensive : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // ============================================================
    // MUX TESTS
    // ============================================================

    // 2-to-1 Mux (2 data inputs + 1 select = 3 total)
    void testMux2to1_data();
    void testMux2to1();

    // 4-to-1 Mux (4 data inputs + 2 selects = 6 total)
    void testMux4to1_data();
    void testMux4to1();

    // 8-to-1 Mux (8 data inputs + 3 selects = 11 total)
    void testMux8to1_data();
    void testMux8to1();

    // Select line transitions (rapid changes)
    void testMuxSelectLineTransitions_data();
    void testMuxSelectLineTransitions();

    // Cascaded muxes
    void testMuxCascaded_data();
    void testMuxCascaded();

    // Data input patterns
    void testMuxDataInputPatterns_data();
    void testMuxDataInputPatterns();

    // ============================================================
    // DEMUX TESTS
    // ============================================================

    // 1-to-2 Demux (1 data + 1 select = 2 inputs)
    void testDemux1to2_data();
    void testDemux1to2();

    // 1-to-4 Demux (1 data + 2 selects = 3 inputs)
    void testDemux1to4_data();
    void testDemux1to4();

    // 1-to-8 Demux (1 data + 3 selects = 4 inputs)
    void testDemux1to8_data();
    void testDemux1to8();

    // Data input variations
    void testDemuxDataInputVariations_data();
    void testDemuxDataInputVariations();

    // Select line transitions
    void testDemuxSelectTransitions_data();
    void testDemuxSelectTransitions();

    // ============================================================
    // COMBINED TESTS
    // ============================================================

    // Chained Mux -> Demux
    void testMuxDemuxChained_data();
    void testMuxDemuxChained();
};
