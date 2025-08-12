// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testdialogs.h"

#include "clockdialog.h"
#include "lengthdialog.h"
#include "clock.h"

#include <QTest>
#include <QApplication>

void TestDialogs::testClockDialogConstruction()
{
    // Skip if no QApplication available (shouldn't happen in test context)
    if (!QApplication::instance()) {
        QSKIP("QApplication required for widget tests");
    }
    
    ClockDialog dialog(1000); // 1000 Hz frequency
    
    // Test basic construction doesn't crash
    QVERIFY(true);
    
    // Test that dialog has basic properties
    QVERIFY(!dialog.windowTitle().isEmpty());
    QCOMPARE(dialog.frequency(), 1000);
}

void TestDialogs::testLengthDialogConstruction() 
{
    // Skip if no QApplication available
    if (!QApplication::instance()) {
        QSKIP("QApplication required for widget tests");
    }
    
    LengthDialog dialog(100); // Length of 100
    
    // Test basic construction doesn't crash
    QVERIFY(true);
    
    // Test dialog initialization
    QCOMPARE(dialog.length(), 100);
}

void TestDialogs::testClockDialogBasicFunctionality()
{
    // Skip if no QApplication available
    if (!QApplication::instance()) {
        QSKIP("QApplication required for widget tests");
    }
    
    ClockDialog dialog(500); // 500 Hz frequency
    
    // Test that we can access dialog without crashes
    // This tests the basic UI setup
    QVERIFY(dialog.frequency() >= 0);
    
    // Test dialog basic functionality
    QCOMPARE(dialog.frequency(), 500);
}