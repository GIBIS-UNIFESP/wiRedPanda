// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLabeledSlider.h"

#include <QPaintEvent>

#include "App/UI/LabeledSlider.h"
#include "Tests/Common/TestUtils.h"

void TestLabeledSlider::testSliderValueChange()
{
    // Plain QSlider getter/setter round-trip; LabeledSlider adds no custom value logic.
    LabeledSlider slider;
    slider.setRange(0, 100);
    slider.setValue(50);

    QCOMPARE(slider.value(), 50);
}

void TestLabeledSlider::testPaintEventDrawsTickLabelsWithoutCrashing()
{
    // paintEvent() is LabeledSlider's only custom behavior (tick-fraction labels drawn
    // beneath the track). Smoke-test it end to end with ticks enabled and a real interval,
    // the same pairing App/UI/ElementEditorUI.cpp always uses for sliderDelay/sliderVolume.
    LabeledSlider slider;
    slider.resize(200, 60);
    slider.setRange(-4, 4);
    slider.setTickPosition(QSlider::TicksBelow);
    slider.setTickInterval(1);

    QPaintEvent event(slider.rect());
    QCoreApplication::sendEvent(&slider, &event);
    QVERIFY(true); // reaching here means paintEvent() did not crash
}

void TestLabeledSlider::testPaintEventHandlesZeroTickIntervalWithoutCrashing()
{
    // Regression: paintEvent() divided (maximum() - minimum()) by tickInterval() with no
    // guard. tickInterval() defaults to 0 until explicitly set, so enabling tick marks
    // without also setting an interval was a division-by-zero crash waiting for any future
    // caller that didn't happen to pair the two calls the way ElementEditorUI.cpp does.
    LabeledSlider slider;
    slider.resize(200, 60);
    slider.setRange(0, 100);
    slider.setTickPosition(QSlider::TicksBelow); // no setTickInterval() call

    QPaintEvent event(slider.rect());
    QCoreApplication::sendEvent(&slider, &event);
    QVERIFY(true); // reaching here means paintEvent() did not crash
}
