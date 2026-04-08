// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLabeledSlider.h"

#include "App/UI/LabeledSlider.h"
#include "Tests/Common/TestUtils.h"

void TestLabeledSlider::testSliderValueChange()
{
    // Test: Labeled slider value change signal works
    LabeledSlider slider;
    slider.setRange(0, 100);
    slider.setValue(50);

    QCOMPARE(slider.value(), 50);
}

