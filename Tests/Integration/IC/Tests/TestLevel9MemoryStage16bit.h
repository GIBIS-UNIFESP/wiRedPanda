// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel9MemoryStage16Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testMemoryStage_data();
    void testMemoryStage();

    void testMemoryStageStructure();
    void testMemoryStageReset();
    void testMultiAddressStorage();

    // Per-bit lane isolation on the two data paths (Result -> DataOut
    // passthrough, and DataIn -> memory -> DataOut storage), across all 16
    // bits so both byte lanes (level6_ram_8x8 low/high) are each exercised --
    // a lane-swap bug would only show up testing bits on both sides of the
    // byte boundary, not just one lane.
    void testResultPassthroughBitIsolation_data();
    void testResultPassthroughBitIsolation();
    void testStoredDataBitIsolation_data();
    void testStoredDataBitIsolation();
};
