// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLabeledSlider.h"

#include <QSlider>

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

void TestLabeledSlider::testSizeHintReservesLabelSpace()
{
    // Regression: sizeHint()/minimumSizeHint() must actually reserve room for the
    // tick-fraction labels paintEvent() draws — a plain contentsMargins() call
    // doesn't grow what the owning layout allocates, which let the labels overlap
    // the slider track (see LabeledSlider::sizeHint()).
    LabeledSlider slider;
    slider.setRange(-4, 4);
    slider.setOrientation(Qt::Horizontal);

    const QSize noTicksHint = slider.sizeHint();

    slider.setTickPosition(QSlider::TicksBelow);
    slider.setTickInterval(1);
    const QSize withTicksHint = slider.sizeHint();

    QVERIFY(withTicksHint.height() > noTicksHint.height());
    QCOMPARE(slider.minimumSizeHint(), slider.sizeHint());
}

void TestLabeledSlider::testPaintEventUsesFallbackFormatForNonCanonicalFraction()
{
    // periodFractionLabel() only special-cases the canonical eighths from -1/2 to 1/2
    // (0, +-1/8, +-1/4, +-3/8, +-1/2); anything else falls back to a 3-decimal float.
    // Range -8..8 with interval 2 puts the endpoint values at +-8, i.e. periodFraction
    // +-1.0 -- outside the canonical set. The endpoints are always drawn unconditionally
    // (see repositionCallout-style reservation comment in paintEvent()), so this
    // deterministically exercises the fallback branch rather than merely making it likely.
    LabeledSlider fallbackSlider;
    fallbackSlider.resize(200, 60);
    fallbackSlider.setRange(-8, 8);
    fallbackSlider.setTickPosition(QSlider::TicksBelow);
    fallbackSlider.setTickInterval(2);
    const QPixmap fallbackPixmap = fallbackSlider.grab();

    LabeledSlider noLabelsSlider;
    noLabelsSlider.resize(200, 60);
    noLabelsSlider.setRange(-8, 8);
    noLabelsSlider.setTickPosition(QSlider::NoTicks);
    const QPixmap baselinePixmap = noLabelsSlider.grab();

    QVERIFY(TestUtils::pixmapHasInk(fallbackPixmap));
    QVERIFY2(fallbackPixmap.toImage() != baselinePixmap.toImage(),
              "fallback fraction labels must actually render extra content vs. no ticks at all");
}

void TestLabeledSlider::testPaintEventSkipsLabelsWhenTickCountIsZero()
{
    // Regression seam: tickCount = (max - min) / interval can integer-divide down to 0
    // when the interval is larger than the whole range, even though interval itself is
    // non-zero (so the earlier interval<=0 guard doesn't fire). paintEvent() must bail
    // out before computing/drawing any labels in that case.
    LabeledSlider zeroCountSlider;
    zeroCountSlider.resize(200, 60);
    zeroCountSlider.setRange(0, 5);
    zeroCountSlider.setTickPosition(QSlider::TicksBelow);
    zeroCountSlider.setTickInterval(10); // (5 - 0) / 10 == 0
    const QPixmap zeroCountPixmap = zeroCountSlider.grab();

    LabeledSlider withLabelsSlider;
    withLabelsSlider.resize(200, 60);
    withLabelsSlider.setRange(0, 5);
    withLabelsSlider.setTickPosition(QSlider::TicksBelow);
    withLabelsSlider.setTickInterval(1); // (5 - 0) / 1 == 5, labels are drawn
    const QPixmap withLabelsPixmap = withLabelsSlider.grab();

    QVERIFY2(zeroCountPixmap.toImage() != withLabelsPixmap.toImage(),
              "zero tick count must skip the fraction labels drawn in the interval=1 case");
}
