// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestLabeledSlider : public QObject
{
    Q_OBJECT

private slots:

    void testSliderValueChange();
    void testPaintEventDrawsTickLabelsWithoutCrashing();

    // Regression: paintEvent() divided by tickInterval() with no zero guard.
    void testPaintEventHandlesZeroTickIntervalWithoutCrashing();

    void testSizeHintReservesLabelSpace();
};
