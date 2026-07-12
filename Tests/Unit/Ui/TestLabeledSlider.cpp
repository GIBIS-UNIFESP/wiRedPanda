// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLabeledSlider.h"

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
    //
    // grab() (not a hand-built QPaintEvent + sendEvent()) is required: a raw sendEvent()
    // dispatches paintEvent() against a widget with no backing store, so QPainter never
    // gets a valid paint engine -- this reliably SEGVs on macOS (Qt::AA_NativeWindows-style
    // paint engines are stricter than the Linux raster/offscreen one) even though it merely
    // warns-and-no-ops on Linux. grab() goes through QWidget::render()'s proper paint-device
    // setup, the same pattern DolphinExporter::renderToPixmap() already uses.
    LabeledSlider slider;
    slider.resize(200, 60);
    slider.setRange(-4, 4);
    slider.setTickPosition(QSlider::TicksBelow);
    slider.setTickInterval(1);

    const QPixmap pixmap = slider.grab();
    QVERIFY(!pixmap.isNull()); // reaching here means paintEvent() did not crash
}

void TestLabeledSlider::testPaintEventHandlesZeroTickIntervalWithoutCrashing()
{
    // Regression: paintEvent() divided (maximum() - minimum()) by tickInterval() with no
    // guard. tickInterval() defaults to 0 until explicitly set, so enabling tick marks
    // without also setting an interval was a division-by-zero crash waiting for any future
    // caller that didn't happen to pair the two calls the way ElementEditorUI.cpp does.
    // See testPaintEventDrawsTickLabelsWithoutCrashing() for why grab() (not sendEvent())
    // is required to invoke paintEvent() safely here.
    LabeledSlider slider;
    slider.resize(200, 60);
    slider.setRange(0, 100);
    slider.setTickPosition(QSlider::TicksBelow); // no setTickInterval() call

    const QPixmap pixmap = slider.grab();
    QVERIFY(!pixmap.isNull()); // reaching here means paintEvent() did not crash
}
