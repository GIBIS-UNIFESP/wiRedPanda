// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Pins the pixels master paints its wires with, in Light theme with antialiasing on.
 * \details Each visible wire is painted with its scene transform, bottom first in the order Qt stacks them, on a white
 * 320x200 image, for a synthetic circuit (a wire per length class and direction, every status, one selected and one
 * highlighted) and for the 2,000-element clocked fixture, at several scales and from two window positions. The scene's
 * own render would add the grid's dots, so the wires are painted alone; outside a view nothing measures a paint pass,
 * so the adaptive antialiasing stays on, which each test asserts. Each channel may differ from the golden by
 * TestUtils::kPixelChannelTolerance.
 */
class TestWirePixelSnapshot : public QObject
{
    Q_OBJECT

private slots:
    void testSyntheticWiresMatchGoldens();
    void testClockedFixtureWiresMatchGoldens();
    void testEveryGoldenIsCompared();
};
