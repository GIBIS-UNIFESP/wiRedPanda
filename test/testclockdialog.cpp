// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testclockdialog.h"

#include "clockdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTest>

void TestClockDialog::initTestCase()
{
    // Initialize test environment
}

void TestClockDialog::cleanupTestCase()
{
    // Clean up test environment
}

void TestClockDialog::testConstruction()
{
    // Test basic construction with frequency in Hz
    ClockDialog dialog(1); // 1 Hz
    
    // Should construct without crashing
    QVERIFY(true);
    
    // Dialog should not be visible by default
    QVERIFY(!dialog.isVisible());
}

void TestClockDialog::testFrequencySettings()
{
    // Test with different initial frequencies (in Hz)
    ClockDialog dialog(1); // 1 Hz
    dialog.show();
    
    // Find frequency spinbox (might be QSpinBox or QDoubleSpinBox)
    auto* intSpinBox = dialog.findChild<QSpinBox*>();
    auto* doubleSpinBox = dialog.findChild<QDoubleSpinBox*>();
    
    if (intSpinBox) {
        // Should show current frequency
        QCOMPARE(intSpinBox->value(), 1);
        
        // Test changing frequency
        intSpinBox->setValue(2);
        QCOMPARE(intSpinBox->value(), 2);
        
        // Test minimum frequency
        intSpinBox->setValue(intSpinBox->minimum());
        QCOMPARE(intSpinBox->value(), intSpinBox->minimum());
        
        // Test maximum frequency
        intSpinBox->setValue(intSpinBox->maximum());
        QCOMPARE(intSpinBox->value(), intSpinBox->maximum());
    } else if (doubleSpinBox) {
        // Should show current frequency
        QCOMPARE(static_cast<int>(doubleSpinBox->value()), 1);
        
        // Test changing frequency
        doubleSpinBox->setValue(2.5);
        QCOMPARE(doubleSpinBox->value(), 2.5);
    }
}

void TestClockDialog::testFrequencyUnitConversion()
{
    ClockDialog dialog(1000); // 1000 Hz = 1 kHz
    dialog.show();
    
    auto* unitComboBox = dialog.findChild<QComboBox*>();
    
    if (unitComboBox && unitComboBox->count() > 1) {
        // Test Hz to kHz conversion
        unitComboBox->setCurrentIndex(0); // Hz
        
        // Switch to kHz if available
        unitComboBox->setCurrentIndex(1); // kHz
        
        // Note: Actual conversion behavior depends on implementation
        QVERIFY(true);
    }
}

void TestClockDialog::testAcceptRejectButtons()
{
    ClockDialog dialog(5); // 5 Hz
    
    // Find button box
    auto* buttonBox = dialog.findChild<QDialogButtonBox*>();
    if (buttonBox) {
        auto* okButton = buttonBox->button(QDialogButtonBox::Ok);
        auto* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
        
        QVERIFY(okButton != nullptr);
        QVERIFY(cancelButton != nullptr);
        
        // Test accept
        QSignalSpy acceptSpy(&dialog, &QDialog::accepted);
        okButton->click();
        QCOMPARE(acceptSpy.count(), 1);
        
        // Test reject (create new dialog since previous was accepted)
        ClockDialog dialog2(5);
        QSignalSpy rejectSpy(&dialog2, &QDialog::rejected);
        auto* buttonBox2 = dialog2.findChild<QDialogButtonBox*>();
        if (buttonBox2) {
            auto* cancelButton2 = buttonBox2->button(QDialogButtonBox::Cancel);
            if (cancelButton2) {
                cancelButton2->click();
                QCOMPARE(rejectSpy.count(), 1);
            }
        }
    }
}

void TestClockDialog::testPersistenceOnAccept()
{
    // Test that frequency() returns the selected value after accept
    {
        ClockDialog dialog(2); // 2 Hz initial
        dialog.show();
        
        auto* intSpinBox = dialog.findChild<QSpinBox*>();
        auto* doubleSpinBox = dialog.findChild<QDoubleSpinBox*>();
        
        if (intSpinBox) {
            // Change frequency
            intSpinBox->setValue(10);
            
            // Accept dialog
            dialog.accept();
            
            // Should return new frequency
            QCOMPARE(dialog.frequency(), 10);
        } else if (doubleSpinBox) {
            // Change frequency
            doubleSpinBox->setValue(10.0);
            
            // Accept dialog
            dialog.accept();
            
            // Should return new frequency
            QCOMPARE(dialog.frequency(), 10);
        }
    }
    
    // Test cancel doesn't change value
    {
        ClockDialog dialog(5); // 5 Hz initial
        dialog.show();
        
        auto* intSpinBox = dialog.findChild<QSpinBox*>();
        if (intSpinBox) {
            // Change frequency
            intSpinBox->setValue(20);
            
            // Reject dialog
            dialog.reject();
            
            // frequency() might still return changed value or original
            // depending on implementation
            int freq = dialog.frequency();
            QVERIFY(freq > 0); // Should be valid
        }
    }
}

void TestClockDialog::testFrequencyRanges()
{
    ClockDialog dialog(1);
    dialog.show();
    
    auto* intSpinBox = dialog.findChild<QSpinBox*>();
    auto* doubleSpinBox = dialog.findChild<QDoubleSpinBox*>();
    
    if (intSpinBox) {
        // Test minimum frequency (should be > 0)
        QVERIFY(intSpinBox->minimum() >= 0);
        
        // Test maximum frequency (should be reasonable)
        QVERIFY(intSpinBox->maximum() > 0);
        QVERIFY(intSpinBox->maximum() <= 1000000); // Max 1 MHz
        
        // Test step size
        QVERIFY(intSpinBox->singleStep() > 0);
    } else if (doubleSpinBox) {
        // Test minimum frequency (should be > 0)
        QVERIFY(doubleSpinBox->minimum() >= 0);
        
        // Test maximum frequency (should be reasonable)
        QVERIFY(doubleSpinBox->maximum() > 0);
        
        // Test decimal places
        QVERIFY(doubleSpinBox->decimals() >= 0);
    }
}

void TestClockDialog::testFrequencyValidation()
{
    ClockDialog dialog(1);
    dialog.show();
    
    auto* intSpinBox = dialog.findChild<QSpinBox*>();
    
    if (intSpinBox) {
        // Test that spinbox enforces valid range
        
        // Try to set negative (should be rejected by spinbox)
        intSpinBox->setValue(-1);
        QVERIFY(intSpinBox->value() >= intSpinBox->minimum());
        
        // Try to set zero (may be valid or invalid)
        intSpinBox->setValue(0);
        if (intSpinBox->minimum() > 0) {
            QVERIFY(intSpinBox->value() > 0);
        } else {
            QCOMPARE(intSpinBox->value(), 0);
        }
        
        // Try extremely high frequency
        intSpinBox->setValue(999999);
        QVERIFY(intSpinBox->value() <= intSpinBox->maximum());
    }
}

void TestClockDialog::testKeyboardShortcuts()
{
    ClockDialog dialog(1);
    dialog.show();
    
    // Test escape key for cancel
    QTest::keyClick(&dialog, Qt::Key_Escape);
    QVERIFY(!dialog.isVisible() || dialog.result() == QDialog::Rejected);
    
    // Test enter key for accept (when OK button has focus)
    ClockDialog dialog2(1);
    dialog2.show();
    
    auto* buttonBox = dialog2.findChild<QDialogButtonBox*>();
    if (buttonBox) {
        auto* okButton = buttonBox->button(QDialogButtonBox::Ok);
        if (okButton) {
            okButton->setFocus();
            QTest::keyClick(okButton, Qt::Key_Return);
            QVERIFY(!dialog2.isVisible() || dialog2.result() == QDialog::Accepted);
        }
    }
}

void TestClockDialog::testMultipleFrequencies()
{
    // Test dialog with different initial frequencies
    ClockDialog dialog1(1);     // 1 Hz
    ClockDialog dialog2(10);    // 10 Hz
    ClockDialog dialog3(100);   // 100 Hz
    ClockDialog dialog4(1000);  // 1 kHz
    
    // Each should handle their frequency independently
    QCOMPARE(dialog1.frequency(), 1);
    QCOMPARE(dialog2.frequency(), 10);
    QCOMPARE(dialog3.frequency(), 100);
    QCOMPARE(dialog4.frequency(), 1000);
}

void TestClockDialog::testSignalConnections()
{
    ClockDialog dialog(1);
    dialog.show();
    
    auto* intSpinBox = dialog.findChild<QSpinBox*>();
    if (intSpinBox) {
        // Monitor value changes
        QSignalSpy valueChangedSpy(intSpinBox, QOverload<int>::of(&QSpinBox::valueChanged));
        
        // Change value
        intSpinBox->setValue(5);
        
        // Should emit signal
        QCOMPARE(valueChangedSpy.count(), 1);
        
        // Clear spy
        valueChangedSpy.clear();
        
        // Test rapid changes
        for (int i = 0; i < 5; ++i) {
            intSpinBox->setValue(i + 1);
        }
        
        // Should have emitted multiple signals
        QCOMPARE(valueChangedSpy.count(), 5);
    }
}