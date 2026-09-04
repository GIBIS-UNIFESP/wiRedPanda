// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestMux : public QObject
{
    Q_OBJECT

private slots:

    void testMuxInputSize();
    void testMuxSelection();
    void testMuxPainting();
    void testMuxPaintingSelected();
    void testMuxOutOfRangeSelect_data();
    void testMuxOutOfRangeSelect();

    // Rotation/flip pivot on a "big" Mux (ports extend past the 64x64 body)
    void testMuxBigPivotsAtBoundingRectCenter();
    void testMuxBigRotationDoesNotDriftInScene();
    void testMuxBigFlipDoesNotDriftInScene();
};
