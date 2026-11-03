// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Checks the random circuits of plan 5: one seed builds one circuit, every feature the generator offers appears
 * across seeds, and a walk over one runs on master's engine and repeats exactly.
 */
class TestRandomCircuit : public QObject
{
    Q_OBJECT

private slots:
    void testSameSeedBuildsSameCircuit();
    void testBuildingBlocksLoad();
    void testEveryFeatureIsGenerated();
    void testWalksRunOnMasterEngine();
};
