// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel8MemoryStage : public QObject
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

    // Per-bit lane isolation on the two data paths: Result -> DataOut passthrough,
    // and DataIn -> memory -> DataOut storage.
    void testResultPassthroughBitIsolation_data();
    void testResultPassthroughBitIsolation();
    void testStoredDataBitIsolation_data();
    void testStoredDataBitIsolation();
};
