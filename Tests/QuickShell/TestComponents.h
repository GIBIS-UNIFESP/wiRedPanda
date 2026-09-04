// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class IC;

/// testIC() builds its Simulation against a CanvasItem (`CanvasItem canvas; canvas.addItem(x);
/// Simulation simulation(&canvas);`) -- CanvasItem implements SimulationHost directly, same as
/// QuickCircuitBuilder, so a Simulation can be constructed against it the same way.
class TestComponents : public QObject
{
    Q_OBJECT

private slots:
    void testNode();
    void testVCC();
    void testGND();
    void testIC();
    void testICs();

private:
    void testICData(IC *ic);
};
