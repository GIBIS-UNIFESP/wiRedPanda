// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDisplays.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

// Display_7 Tests

void TestDisplays::testDisplay7Configuration()
{
    Display7 display;

    // Verify input/output counts
    QCOMPARE(display.inputSize(), 8);
    QCOMPARE(display.minInputSize(), 8);
    QCOMPARE(display.maxInputSize(), 8);  // Fixed size
    QCOMPARE(display.outputSize(), 0);
    QCOMPARE(display.minOutputSize(), 0);
    QCOMPARE(display.maxOutputSize(), 0);

    // Verify element type
    QCOMPARE(display.elementType(), ElementType::Display7);
    QCOMPARE(display.elementGroup(), ElementGroup::Output);

    // Verify color support
    QVERIFY(display.hasColors());
    QCOMPARE(display.color(), QString("Red"));  // Default color

    // Verify all ports exist and are optional
    for (int i = 0; i < display.inputSize(); ++i) {
        auto *port = display.inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Port %1 should exist").arg(i)));
        QVERIFY2(!port->isRequired(), qPrintable(QString("Port %1 should be optional").arg(i)));
        QCOMPARE(port->defaultValue(), Status::Inactive);
    }
}

void TestDisplays::testDisplay7SegmentControl()
{
    Display7 display;

    // Verify segment count (7 segments + decimal point = 8 inputs)
    QCOMPARE(display.inputSize(), 8);

    // All inputs start inactive
    for (int i = 0; i < display.inputSize(); ++i) {
        QCOMPARE(display.inputPort(i)->status(), Status::Inactive);
    }
}

void TestDisplays::testDisplay7PortNames()
{
    Display7 display;

    // Verify all ports exist and have proper names (not relying on translation)
    // Port order: 0=G, 1=F, 2=E, 3=D, 4=A, 5=B, 6=DP, 7=C (top, top-left, mid-left, bottom-left, top, top-right, decimal, bottom-right)
    // This verifies the segment pin configuration without depending on translations

    const int expectedCount = 8;  // 7 segments + 1 decimal point
    QCOMPARE(display.inputSize(), expectedCount);

    // Verify each port exists and is named (non-empty name)
    for (int i = 0; i < display.inputSize(); ++i) {
        auto *port = display.inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Port %1 should exist").arg(i)));
        QVERIFY2(!port->name().isEmpty(), qPrintable(QString("Port %1 should have a name").arg(i)));

        // Port name should contain segment identifier (may be translated)
        // Common identifiers: A-G for segments, DP for decimal point
        const QString name = port->name().toUpper();
        bool hasSegmentIdentifier = name.contains("A") || name.contains("B") || name.contains("C") ||
                                   name.contains("D") || name.contains("E") || name.contains("F") ||
                                   name.contains("G") || name.contains("DP");
        QVERIFY2(hasSegmentIdentifier,
                 qPrintable(QString("Port %1 name '%2' should identify a segment (A-G or DP)")
                           .arg(i).arg(port->name())));
    }
}

// Display_14 Tests

void TestDisplays::testDisplay14Configuration()
{
    Display14 display;

    // Verify input/output counts
    QCOMPARE(display.inputSize(), 15);
    QCOMPARE(display.minInputSize(), 15);
    QCOMPARE(display.maxInputSize(), 15);  // Fixed size
    QCOMPARE(display.outputSize(), 0);
    QCOMPARE(display.minOutputSize(), 0);
    QCOMPARE(display.maxOutputSize(), 0);

    // Verify element type
    QCOMPARE(display.elementType(), ElementType::Display14);
    QCOMPARE(display.elementGroup(), ElementGroup::Output);

    // Verify color support
    QVERIFY(display.hasColors());
    QCOMPARE(display.color(), QString("Red"));  // Default color

    // Verify all ports are optional
    for (int i = 0; i < display.inputSize(); ++i) {
        auto *port = display.inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Port %1 should exist").arg(i)));
        QVERIFY2(!port->isRequired(), qPrintable(QString("Port %1 should be optional").arg(i)));
        QCOMPARE(port->defaultValue(), Status::Inactive);
    }
}

void TestDisplays::testDisplay14SegmentControl()
{
    Display14 display;

    // Verify 14 segments + 1 decimal point = 15 inputs
    QCOMPARE(display.inputSize(), 15);

    // All inputs start inactive
    for (int i = 0; i < display.inputSize(); ++i) {
        QCOMPARE(display.inputPort(i)->status(), Status::Inactive);
    }
}

// Display_16 Tests

void TestDisplays::testDisplay16Configuration()
{
    Display16 display;

    // Verify input/output counts
    QCOMPARE(display.inputSize(), 17);
    QCOMPARE(display.minInputSize(), 17);
    QCOMPARE(display.maxInputSize(), 17);  // Fixed size
    QCOMPARE(display.outputSize(), 0);
    QCOMPARE(display.minOutputSize(), 0);
    QCOMPARE(display.maxOutputSize(), 0);

    // Verify element type
    QCOMPARE(display.elementType(), ElementType::Display16);
    QCOMPARE(display.elementGroup(), ElementGroup::Output);

    // Verify color support
    QVERIFY(display.hasColors());
    QCOMPARE(display.color(), QString("Red"));  // Default color

    // Verify all ports are optional
    for (int i = 0; i < display.inputSize(); ++i) {
        auto *port = display.inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Port %1 should exist").arg(i)));
        QVERIFY2(!port->isRequired(), qPrintable(QString("Port %1 should be optional").arg(i)));
        QCOMPARE(port->defaultValue(), Status::Inactive);
    }
}

void TestDisplays::testDisplay16SegmentControl()
{
    Display16 display;

    // Verify 16 segments + 1 decimal point = 17 inputs
    QCOMPARE(display.inputSize(), 17);

    // All inputs start inactive
    for (int i = 0; i < display.inputSize(); ++i) {
        QCOMPARE(display.inputPort(i)->status(), Status::Inactive);
    }
}

// Color Tests

void TestDisplays::testDisplayColors()
{
    // Test Display_7
    Display7 display7;
    QCOMPARE(display7.color(), QString("Red"));
    display7.setColor("Green");
    QCOMPARE(display7.color(), QString("Green"));
    display7.setColor("Blue");
    QCOMPARE(display7.color(), QString("Blue"));
    display7.setColor("Purple");
    QCOMPARE(display7.color(), QString("Purple"));
    display7.setColor("White");
    QCOMPARE(display7.color(), QString("White"));
    display7.setColor("Red");  // Back to default
    QCOMPARE(display7.color(), QString("Red"));

    // Test Display_14
    Display14 display14;
    QCOMPARE(display14.color(), QString("Red"));
    display14.setColor("Green");
    QCOMPARE(display14.color(), QString("Green"));
    display14.setColor("Blue");
    QCOMPARE(display14.color(), QString("Blue"));

    // Test Display_16
    Display16 display16;
    QCOMPARE(display16.color(), QString("Red"));
    display16.setColor("Purple");
    QCOMPARE(display16.color(), QString("Purple"));
    display16.setColor("White");
    QCOMPARE(display16.color(), QString("White"));
}

// Integration Tests

void TestDisplays::testDisplay7InCircuit()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create circuit: 8 InputSwitches → Display_7
    InputSwitch sw0, sw1, sw2, sw3, sw4, sw5, sw6, sw7;
    Display7 display;

    builder.add(&sw0, &sw1, &sw2, &sw3, &sw4, &sw5, &sw6, &sw7, &display);

    // Connect switches to display segments
    builder.connect(&sw0, 0, &display, 0);  // G
    builder.connect(&sw1, 0, &display, 1);  // F
    builder.connect(&sw2, 0, &display, 2);  // E
    builder.connect(&sw3, 0, &display, 3);  // D
    builder.connect(&sw4, 0, &display, 4);  // A
    builder.connect(&sw5, 0, &display, 5);  // B
    builder.connect(&sw6, 0, &display, 6);  // DP
    builder.connect(&sw7, 0, &display, 7);  // C

    // Initialize simulation
    auto *sim = builder.initSimulation();
    QVERIFY(sim != nullptr);

    // Verify all connections established
    for (int i = 0; i < display.inputSize(); ++i) {
        QVERIFY2(display.inputPort(i)->connections().size() > 0,
                 qPrintable(QString("Port %1 should be connected").arg(i)));
    }

    // Test: All switches OFF → all segments inactive
    sw0.setOn(false); sw1.setOn(false); sw2.setOn(false); sw3.setOn(false);
    sw4.setOn(false); sw5.setOn(false); sw6.setOn(false); sw7.setOn(false);
    sim->update();

    for (int i = 0; i < display.inputSize(); ++i) {
        QCOMPARE(display.inputPort(i)->status(), Status::Inactive);
    }

    // Test: Turn on segment A (top horizontal)
    sw4.setOn(true);
    sim->update();
    QCOMPARE(display.inputPort(4)->status(), Status::Active);

    // Test: Turn on multiple segments (display "7" segments: A, B, C)
    sw4.setOn(true);   // A (top)
    sw5.setOn(true);   // B (upper right)
    sw7.setOn(true);   // C (lower right)
    sim->update();
    QCOMPARE(display.inputPort(4)->status(), Status::Active);
    QCOMPARE(display.inputPort(5)->status(), Status::Active);
    QCOMPARE(display.inputPort(7)->status(), Status::Active);
    QCOMPARE(display.inputPort(0)->status(), Status::Inactive);  // G still off
}

void TestDisplays::testDisplayWithDecoder()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create simple 2-bit decoder → Display_7
    // This simulates a partial BCD decoder for digit display
    InputSwitch bit0, bit1;
    Display7 display;
    And andGate;

    builder.add(&bit0, &bit1, &andGate, &display);

    // Simple logic: bit0 AND bit1 → segment A
    builder.connect(&bit0, 0, &andGate, 0);
    builder.connect(&bit1, 0, &andGate, 1);
    builder.connect(&andGate, 0, &display, 4);  // Connect AND output to segment A

    auto *sim = builder.initSimulation();
    QVERIFY(sim != nullptr);

    // Verify connection established
    QVERIFY(display.inputPort(4)->connections().size() > 0);

    // Test: 00 → segment A off
    bit0.setOn(false); bit1.setOn(false);
    sim->update();
    QCOMPARE(display.inputPort(4)->status(), Status::Inactive);

    // Test: 11 → segment A on
    bit0.setOn(true); bit1.setOn(true);
    sim->update();
    QCOMPARE(display.inputPort(4)->status(), Status::Active);

    // Test: 01 → segment A off (AND result is false)
    bit0.setOn(true); bit1.setOn(false);
    sim->update();
    QCOMPARE(display.inputPort(4)->status(), Status::Inactive);

    // Test: 10 → segment A off (AND result is false)
    bit0.setOn(false); bit1.setOn(true);
    sim->update();
    QCOMPARE(display.inputPort(4)->status(), Status::Inactive);
}

// ============================================================================
// Basic Display7 Test (migrated from testelements.cpp)
// ============================================================================

void TestDisplays::testDisplay7()
{
    Display7 elm;
    // 7-segment display has 8 inputs: a, b, c, d, e, f, g, dp (decimal point)
    QCOMPARE(elm.inputSize(), 8);
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.elementType(), ElementType::Display7);

    // Test color property
    QCOMPARE(elm.color(), QString("Red"));  // Default color
    elm.setColor("Green");
    QCOMPARE(elm.color(), QString("Green"));
    elm.setColor("Blue");
    QCOMPARE(elm.color(), QString("Blue"));
}

