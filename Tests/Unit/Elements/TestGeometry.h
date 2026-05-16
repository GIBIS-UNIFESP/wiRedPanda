// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestGeometry : public QObject
{
    Q_OBJECT

private slots:
    // Pixmap and bounding rect tests (3 tests)
    void testPixmapCenter();
    void testBoundingRectNoPortsCalculation();
    void testBoundingRectWithPorts();

    // Port positioning tests (4 tests)
    void testInputPortPositionSinglePort();
    void testInputPortPositionMultiplePorts();
    void testOutputPortPositionSinglePort();
    void testOutputPortPositionMultiplePorts();

    // Port spacing and centering tests (2 tests)
    void testPortCenteringAroundY32();
    void testPortSpacingCalculation();
};
