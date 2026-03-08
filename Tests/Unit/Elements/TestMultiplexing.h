// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestMultiplexing : public QObject
{
    Q_OBJECT

private slots:
    // Basic gate property tests
    void testMux();
    void testDemux();

    // Comprehensive Mux tests
    void testMuxVariableSizes();
    void testMuxPortNaming();
    void testMuxEdgeCases();
    void testMuxBoundaryValues();

    // Comprehensive Demux tests
    void testDemuxVariableSizes();
    void testDemuxPortNaming();
    void testDemuxEdgeCases();
    void testDemuxBoundaryValues();
};

