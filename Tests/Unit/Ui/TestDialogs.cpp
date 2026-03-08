// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestDialogs.h"

#include <QSlider>
#include <QSpinBox>
#include <QTest>
#include <QTimer>

#include "App/UI/ClockDialog.h"
#include "App/UI/LengthDialog.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// ClockDialog Tests (5 tests)
// ============================================================================

void TestDialogs::testClockDialogConstructor()
{
    // Test that constructor sets initial frequency value and syncs spinbox/slider
    ClockDialog dialog(100);

    auto *spinBox = dialog.findChild<QSpinBox *>("frequencySpinBox");
    QVERIFY(spinBox != nullptr);
    QCOMPARE(spinBox->value(), 100);

    auto *slider = dialog.findChild<QSlider *>("frequencySlider");
    QVERIFY(slider != nullptr);
    QCOMPARE(slider->value(), 100);
}

void TestDialogs::testClockDialogMinFrequency()
{
    // Test that minimum frequency (2 Hz) is accepted and stored correctly
    ClockDialog dialog(2);

    auto *spinBox = dialog.findChild<QSpinBox *>("frequencySpinBox");
    QCOMPARE(spinBox->minimum(), 2);
    QCOMPARE(spinBox->value(), 2);
}

void TestDialogs::testClockDialogMaxFrequency()
{
    // Test that maximum frequency (1024 Hz) is accepted and stored correctly
    ClockDialog dialog(1024);

    auto *spinBox = dialog.findChild<QSpinBox *>("frequencySpinBox");
    QCOMPARE(spinBox->maximum(), 1024);
    QCOMPARE(spinBox->value(), 1024);
}

void TestDialogs::testClockDialogRange()
{
    // Test various valid frequencies within range (2-1024 Hz)
    QVector<int> validFrequencies = {2, 10, 100, 256, 512, 1000, 1024};

    for (int freq : validFrequencies) {
        ClockDialog dialog(freq);
        auto *spinBox = dialog.findChild<QSpinBox *>("frequencySpinBox");
        QCOMPARE(spinBox->value(), freq);
    }

    // Test out-of-range clamping: value above max clamps to 1024
    ClockDialog dialogHigh(5000);
    auto *spinBox = dialogHigh.findChild<QSpinBox *>("frequencySpinBox");
    QCOMPARE(spinBox->value(), 1024);
}

void TestDialogs::testClockDialogFrequencyReturnValue()
{
    // Test cancel and accept return values for frequency()
    // Cancel path → -1
    ClockDialog dialog1(100);
    QTimer::singleShot(0, &dialog1, &QDialog::reject);
    QCOMPARE(dialog1.frequency(), -1);

    // Accept path → returns spinbox value
    ClockDialog dialog2(256);
    QTimer::singleShot(0, &dialog2, &QDialog::accept);
    QCOMPARE(dialog2.frequency(), 256);
}

// ============================================================================
// LengthDialog Tests (4 tests)
// ============================================================================

void TestDialogs::testLengthDialogConstructor()
{
    // Test that constructor sets initial length value and syncs spinbox/slider
    LengthDialog dialog(64);

    auto *spinBox = dialog.findChild<QSpinBox *>("lengthSpinBox");
    QVERIFY(spinBox != nullptr);
    QCOMPARE(spinBox->value(), 64);

    auto *slider = dialog.findChild<QSlider *>("lengthSlider");
    QVERIFY(slider != nullptr);
    QCOMPARE(slider->value(), 64);

    // Verify constructor actually overrides: use non-default value
    LengthDialog dialog2(100);
    QCOMPARE(dialog2.findChild<QSpinBox *>("lengthSpinBox")->value(), 100);
}

void TestDialogs::testLengthDialogMinLength()
{
    // Test that minimum length (2) is accepted and stored correctly
    LengthDialog dialog(2);

    auto *spinBox = dialog.findChild<QSpinBox *>("lengthSpinBox");
    QCOMPARE(spinBox->minimum(), 2);
    QCOMPARE(spinBox->value(), 2);
}

void TestDialogs::testLengthDialogMaxLength()
{
    // Test that maximum length (2048) is accepted and stored correctly
    LengthDialog dialog(2048);

    auto *spinBox = dialog.findChild<QSpinBox *>("lengthSpinBox");
    QCOMPARE(spinBox->maximum(), 2048);
    QCOMPARE(spinBox->value(), 2048);
}

void TestDialogs::testLengthDialogRange()
{
    // Test various valid lengths within range (2-2048)
    QVector<int> validLengths = {2, 10, 32, 64, 128, 512, 1024, 2048};

    for (int length : validLengths) {
        LengthDialog dialog(length);
        auto *spinBox = dialog.findChild<QSpinBox *>("lengthSpinBox");
        QCOMPARE(spinBox->value(), length);
    }

    // Test out-of-range clamping: value above max clamps to 2048
    LengthDialog dialogHigh(9999);
    auto *spinBox = dialogHigh.findChild<QSpinBox *>("lengthSpinBox");
    QCOMPARE(spinBox->value(), 2048);
}

