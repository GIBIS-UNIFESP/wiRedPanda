// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testdialogs.h"

#include "clockdialog.h"
#include "lengthdialog.h"
#include "clock.h"

#include <QTest>
#include <QApplication>
#include <QSpinBox>
#include <QDialogButtonBox>

void TestDialogs::testClockDialogConstruction()
{
    // Test dialog construction without showing it
    ClockDialog dialog(1000); // 1000 Hz frequency
    
    // Test basic construction doesn't crash
    QVERIFY(true);
    
    // Test that dialog has basic properties
    QVERIFY(!dialog.windowTitle().isEmpty());
    QVERIFY(dialog.windowTitle().contains("Clock", Qt::CaseInsensitive));
    
    // Test that dialog is properly initialized but not shown
    QVERIFY(!dialog.isVisible());
    QCOMPARE(dialog.result(), QDialog::Rejected); // Default state
}

void TestDialogs::testLengthDialogConstruction() 
{
    // Test dialog construction without showing it
    LengthDialog dialog(100); // Length of 100
    
    // Test basic construction doesn't crash
    QVERIFY(true);
    
    // Test that dialog has basic properties  
    QVERIFY(!dialog.windowTitle().isEmpty());
    QVERIFY(dialog.windowTitle().contains("Length", Qt::CaseInsensitive));
    
    // Test that dialog is properly initialized but not shown
    QVERIFY(!dialog.isVisible());
    QCOMPARE(dialog.result(), QDialog::Rejected); // Default state
}

void TestDialogs::testClockDialogBasicFunctionality()
{
    // Test dialog UI components without showing the dialog
    ClockDialog dialog(750); // 750 Hz frequency
    
    // Test that dialog was created successfully
    QVERIFY(!dialog.isVisible()); // Should not be visible
    
    // Test window properties
    QVERIFY(!dialog.windowTitle().isEmpty());
    
    // Test that the dialog has the expected child widgets
    // (This tests the UI setup without calling exec())
    auto spinBoxes = dialog.findChildren<QSpinBox*>();
    QVERIFY(spinBoxes.size() > 0); // Should have at least one spinbox
    
    auto buttonBoxes = dialog.findChildren<QDialogButtonBox*>();
    QVERIFY(buttonBoxes.size() > 0); // Should have button box
    
    // Test that we can access the spinbox value without showing dialog
    if (!spinBoxes.isEmpty()) {
        QCOMPARE(spinBoxes.first()->value(), 750);
    }
}