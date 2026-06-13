// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel8FetchStage : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testFetchStage_data();
    void testFetchStage();

    // Exercise the program-write path so the stage fetches real (non-zero)
    // instructions, driving the OpCode/RegisterAddr/RawInstr outputs; plus the
    // PCInc and Reset control paths the original test never asserted.
    void testProgramAndFetch();
    void testPCIncrement();
    void testReset();

    void testFetchStageStructure();
};
