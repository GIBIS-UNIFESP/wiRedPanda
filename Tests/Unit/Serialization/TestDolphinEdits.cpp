// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinEdits.h"

#include "App/BeWavedDolphin/DolphinEdits.h"
#include "App/BeWavedDolphin/SignalModel.h"

void TestDolphinEdits::testApplyToCellsInvertsSelectedCells()
{
    SignalModel model(1, 4);
    model.setInputRows(1);
    model.setValue(0, 0, 0);
    model.setValue(0, 1, 1);
    model.setValue(0, 2, 0);
    model.setValue(0, 3, 1);

    const QModelIndexList cells{model.index(0, 0), model.index(0, 1), model.index(0, 2), model.index(0, 3)};
    DolphinEdits::applyToCells(model, cells, [](int v) { return (v + 1) % 2; });

    QCOMPARE(model.value(0, 0), 1);
    QCOMPARE(model.value(0, 1), 0);
    QCOMPARE(model.value(0, 2), 1);
    QCOMPARE(model.value(0, 3), 0);
}

void TestDolphinEdits::testLastNonZeroColumnFindsRightmostNonZero()
{
    SignalModel model(1, 8);
    model.setInputRows(1);
    model.setValue(0, 3, 1);

    QCOMPARE(DolphinEdits::lastNonZeroColumn(model, 1), 3);
}

void TestDolphinEdits::testLastNonZeroColumnAllZeroReturnsZero()
{
    SignalModel model(1, 8);
    model.setInputRows(1);
    // Every cell defaults to 0 — no non-zero value exists anywhere in the grid.

    QCOMPARE(DolphinEdits::lastNonZeroColumn(model, 1), 0);
}
