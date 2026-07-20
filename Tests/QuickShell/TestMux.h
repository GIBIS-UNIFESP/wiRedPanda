// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// testMuxBigPivotsAtBoundingRectCenter/testMuxBigRotationDoesNotDriftInScene/
/// testMuxBigFlipDoesNotDriftInScene are dropped, not ported: same already-tracked "TestIc's 8
/// rotation/pivot tests" gap as TestDemux's own dropped trio -- see that class's doc comment.
class TestMux : public QObject
{
    Q_OBJECT

private slots:

    void testMuxInputSize();
    void testMuxSelection();
    void testMuxPainting();
    void testMuxOutOfRangeSelect_data();
    void testMuxOutOfRangeSelect();
};
