// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// testDemuxBigPivotsAtBoundingRectCenter/testDemuxBigRotationDoesNotDriftInScene/
/// testDemuxBigFlipDoesNotDriftInScene are dropped, not ported: they duplicate the same
/// already-tracked "TestIc's 8 rotation/pivot tests" gap (.claude/WIDGETS_TEST_PARITY_AUDIT.md),
/// which needs a real elementTransform()-based testing technique this branch hasn't built yet
/// (GraphicElement dropped QGraphicsItem's mapToScene()/transformOriginPoint()) -- a Phase F
/// item, not specific to Demux.
class TestDemux : public QObject
{
    Q_OBJECT

private slots:

    void testDemuxOutputSize();
    void testDemuxRouting();
    void testDemuxPainting();
    void testDemuxOutOfRangeSelect();
};
