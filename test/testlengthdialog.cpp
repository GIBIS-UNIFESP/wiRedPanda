// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testlengthdialog.h"

#include "lengthdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTest>

void TestLengthDialog::initTestCase()
{
    // Initialize test environment
}

void TestLengthDialog::cleanupTestCase()
{
    // Clean up test environment
}

void TestLengthDialog::testConstruction()
{
    // Test default construction
    LengthDialog dialog(10);
    
    // Should construct without crashing
    QVERIFY(true);
    
    // Dialog should not be visible by default
    QVERIFY(!dialog.isVisible());
}

void TestLengthDialog::testMinMaxValues()
{
    // Test with different values
    LengthDialog dialog1(5);
    dialog1.show();
    
    // Get the spinbox (assuming it's accessible)
    auto* spinBox = dialog1.findChild<QSpinBox*>();
    if (spinBox) {
        // Check that min/max are set appropriately
        QVERIFY(spinBox->minimum() <= 5);
        QVERIFY(spinBox->maximum() >= 5);
        QCOMPARE(spinBox->value(), 5); // Should show current value
    }
    
    // Test with larger value
    LengthDialog dialog2(100);
    dialog2.show();
    
    spinBox = dialog2.findChild<QSpinBox*>();
    if (spinBox) {
        QVERIFY(spinBox->minimum() <= 100);
        QVERIFY(spinBox->maximum() >= 100);
        QCOMPARE(spinBox->value(), 100);
    }
}

void TestLengthDialog::testInitialValue()
{
    // Test setting initial value
    LengthDialog dialog(10);
    dialog.show();
    
    auto* spinBox = dialog.findChild<QSpinBox*>();
    if (spinBox) {
        QCOMPARE(spinBox->value(), 10);
    }
    
    // Test with different initial value
    LengthDialog dialog2(15);
    dialog2.show();
    
    spinBox = dialog2.findChild<QSpinBox*>();
    if (spinBox) {
        QCOMPARE(spinBox->value(), 15);
    }
}

void TestLengthDialog::testInputValidation()
{
    LengthDialog dialog(5);
    dialog.show();
    
    auto* spinBox = dialog.findChild<QSpinBox*>();
    if (spinBox) {
        // Test setting valid value
        spinBox->setValue(7);
        QCOMPARE(spinBox->value(), 7);
        
        // Test that spinbox enforces min/max
        int maxValue = spinBox->maximum();
        spinBox->setValue(maxValue + 10); // Above max
        QCOMPARE(spinBox->value(), maxValue); // Should be clamped
        
        int minValue = spinBox->minimum();
        spinBox->setValue(minValue - 10); // Below min
        QCOMPARE(spinBox->value(), minValue); // Should be clamped
    }
}

void TestLengthDialog::testAcceptReject()
{
    LengthDialog dialog(5);
    
    // Find button box
    auto* buttonBox = dialog.findChild<QDialogButtonBox*>();
    if (buttonBox) {
        // Test that OK and Cancel buttons exist
        auto* okButton = buttonBox->button(QDialogButtonBox::Ok);
        auto* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
        
        QVERIFY(okButton != nullptr);
        QVERIFY(cancelButton != nullptr);
        
        // Test accept signal
        QSignalSpy acceptSpy(&dialog, &QDialog::accepted);
        okButton->click();
        QCOMPARE(acceptSpy.count(), 1);
        
        // Test reject signal
        QSignalSpy rejectSpy(&dialog, &QDialog::rejected);
        cancelButton->click();
        QCOMPARE(rejectSpy.count(), 1);
    }
}

void TestLengthDialog::testExecValue()
{
    // Test the static exec method behavior
    // Note: We can't actually test modal dialogs in unit tests,
    // but we can test the setup
    
    LengthDialog dialog(4);
    
    auto* spinBox = dialog.findChild<QSpinBox*>();
    if (spinBox) {
        // Set a value
        spinBox->setValue(6);
        
        // Simulate accepting the dialog
        dialog.accept();
        
        // The value should be retrievable
        QCOMPARE(spinBox->value(), 6);
    }
}

void TestLengthDialog::testEdgeValues()
{
    // Test edge cases
    
    // Test with specific value
    LengthDialog dialog1(5);
    dialog1.show();
    auto* spinBox = dialog1.findChild<QSpinBox*>();
    if (spinBox) {
        QCOMPARE(spinBox->value(), 5);
        // Test that range is sensible
        QVERIFY(spinBox->minimum() <= 5);
        QVERIFY(spinBox->maximum() >= 5);
    }
    
    // Test with negative values
    LengthDialog dialog2(-5);
    dialog2.show();
    spinBox = dialog2.findChild<QSpinBox*>();
    if (spinBox) {
        QCOMPARE(spinBox->value(), -5);
        QVERIFY(spinBox->minimum() <= -5);
    }
    
    // Test with zero
    LengthDialog dialog3(0);
    dialog3.show();
    spinBox = dialog3.findChild<QSpinBox*>();
    if (spinBox) {
        QCOMPARE(spinBox->value(), 0);
    }
}

void TestLengthDialog::testKeyboardInput()
{
    LengthDialog dialog(50);
    dialog.show();
    
    auto* spinBox = dialog.findChild<QSpinBox*>();
    if (spinBox) {
        // Give focus to spinbox
        spinBox->setFocus();
        
        // Clear and type new value
        spinBox->clear();
        QTest::keyClicks(spinBox, "75");
        
        // Focus out to trigger validation
        spinBox->clearFocus();
        
        QCOMPARE(spinBox->value(), 75);
        
        // Test invalid input (letters)
        spinBox->clear();
        QTest::keyClicks(spinBox, "abc");
        spinBox->clearFocus();
        
        // Should retain last valid value or within range
        QVERIFY(spinBox->value() >= spinBox->minimum() && 
                spinBox->value() <= spinBox->maximum());
    }
}

void TestLengthDialog::testStepIncrement()
{
    LengthDialog dialog(50);
    dialog.show();
    
    auto* spinBox = dialog.findChild<QSpinBox*>();
    if (spinBox) {
        // Test up/down arrows
        spinBox->setValue(50);
        
        // Simulate up arrow key
        QTest::keyClick(spinBox, Qt::Key_Up);
        QCOMPARE(spinBox->value(), 51);
        
        // Simulate down arrow key
        QTest::keyClick(spinBox, Qt::Key_Down);
        QTest::keyClick(spinBox, Qt::Key_Down);
        QCOMPARE(spinBox->value(), 49);
        
        // Test Page Up/Down for larger increments
        QTest::keyClick(spinBox, Qt::Key_PageUp);
        QVERIFY(spinBox->value() > 49); // Should increment by more than 1
        
        QTest::keyClick(spinBox, Qt::Key_PageDown);
        // Should decrement by more than 1
    }
}

void TestLengthDialog::testSignalsAndSlots()
{
    LengthDialog dialog(5);
    dialog.show();
    
    auto* spinBox = dialog.findChild<QSpinBox*>();
    if (spinBox) {
        // Connect to value changed signal
        QSignalSpy valueSpy(spinBox, QOverload<int>::of(&QSpinBox::valueChanged));
        
        // Change value
        spinBox->setValue(7);
        
        // Should emit signal
        QCOMPARE(valueSpy.count(), 1);
        QList<QVariant> arguments = valueSpy.takeFirst();
        QCOMPARE(arguments.at(0).toInt(), 7);
        
        // Test multiple changes
        spinBox->setValue(8);
        spinBox->setValue(9);
        spinBox->setValue(10);
        
        QCOMPARE(valueSpy.count(), 3);
    }
}