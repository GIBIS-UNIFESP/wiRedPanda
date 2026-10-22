// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Pins the order in which master stacks a scene's items, and that a paint in that order is what Qt renders.
 * \details The order goldens name every item, bottom first (elements, their ports, labels and other children, wires,
 * the selection rectangle), of every example and fixture circuit, of three overlapping elements after undoing the
 * delete and the morph of the bottom one, and of those elements under a rubber band and under a wire being routed
 * across them. Pixels cannot show a change of order where items do not overlap opaquely, so the order itself is
 * pinned; and each of the overlapping scenes, painted item by item in that order, must equal Qt's own render of the
 * scene, grid dots aside.
 */
class TestStackingSnapshot : public QObject
{
    Q_OBJECT

private slots:
    void testCircuitOrderMatchesGolden_data();
    void testCircuitOrderMatchesGolden();
    void testEveryGoldenHasACircuit();
    void testOverlapPaintMatchesSceneRender();
    void testOrderAfterUndoMatchesGolden();
    void testRubberBandOverOverlapMatchesGolden();
    void testWireRoutedAcrossOverlapMatchesGolden();
};
