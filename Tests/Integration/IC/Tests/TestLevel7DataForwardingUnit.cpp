// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7DataForwardingUnit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel7DataForwardingUnit::initTestCase()
{
}

void TestLevel7DataForwardingUnit::cleanup()
{
}

void TestLevel7DataForwardingUnit::testDataForwardingUnit_data()
{
    QTest::addColumn<int>("dataA");
    QTest::addColumn<int>("dataB");
    QTest::addColumn<int>("dataC");
    QTest::addColumn<int>("dataD");
    QTest::addColumn<int>("select");
    QTest::addColumn<int>("expectedOutput");

    // Test all 4 forwarding paths
    // Select: 00=DataA, 01=DataB, 10=DataC, 11=DataD
    QTest::newRow("forward DataA (select=00)") << 0x11 << 0x22 << 0x33 << 0x44 << 0 << 0x11;
    QTest::newRow("forward DataB (select=01)") << 0x11 << 0x22 << 0x33 << 0x44 << 1 << 0x22;
    QTest::newRow("forward DataC (select=10)") << 0x11 << 0x22 << 0x33 << 0x44 << 2 << 0x33;
    QTest::newRow("forward DataD (select=11)") << 0x11 << 0x22 << 0x33 << 0x44 << 3 << 0x44;
    QTest::newRow("forward 0xFF from DataA")   << 0xFF << 0x00 << 0x00 << 0x00 << 0 << 0xFF;
    QTest::newRow("forward 0xAA from DataD")   << 0x00 << 0x00 << 0x00 << 0xAA << 3 << 0xAA;
}

void TestLevel7DataForwardingUnit::testDataForwardingUnit()
{
    QFETCH(int, dataA);
    QFETCH(int, dataB);
    QFETCH(int, dataC);
    QFETCH(int, dataD);
    QFETCH(int, select);
    QFETCH(int, expectedOutput);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create DataA input (8-bit)
    QVector<InputSwitch *> dataA_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("DataA[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        dataA_inputs.append(sw);
    }

    // Create DataB input (8-bit)
    QVector<InputSwitch *> dataB_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("DataB[%1]").arg(i));
        sw->setPos(50 + i * 60, 140);
        dataB_inputs.append(sw);
    }

    // Create DataC input (8-bit)
    QVector<InputSwitch *> dataC_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("DataC[%1]").arg(i));
        sw->setPos(50 + i * 60, 180);
        dataC_inputs.append(sw);
    }

    // Create DataD input (8-bit)
    QVector<InputSwitch *> dataD_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("DataD[%1]").arg(i));
        sw->setPos(50 + i * 60, 220);
        dataD_inputs.append(sw);
    }

    // Create Select input (2-bit)
    QVector<InputSwitch *> select_inputs;
    for (int i = 0; i < 2; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("Select[%1]").arg(i));
        sw->setPos(600 + i * 60, 140);
        select_inputs.append(sw);
    }

    // Load the forwarding unit IC
    IC *fwd_unit = loadBuildingBlockIC("level7_data_forwarding_unit.panda");
    builder.add(fwd_unit);

    // Create output LEDs for ForwardedData
    QVector<Led *> output_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("ForwardedData[%1]").arg(i));
        led->setPos(50 + i * 60, 300);
        output_leds.append(led);
    }

    // Connect all inputs to forwarding unit
    for (int i = 0; i < 8; i++) {
        builder.connect(dataA_inputs[i], 0, fwd_unit, QString("DataA[%1]").arg(i));
        builder.connect(dataB_inputs[i], 0, fwd_unit, QString("DataB[%1]").arg(i));
        builder.connect(dataC_inputs[i], 0, fwd_unit, QString("DataC[%1]").arg(i));
        builder.connect(dataD_inputs[i], 0, fwd_unit, QString("DataD[%1]").arg(i));
    }

    for (int i = 0; i < 2; i++) {
        builder.connect(select_inputs[i], 0, fwd_unit, QString("Select[%1]").arg(i));
    }

    // Connect outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(fwd_unit, QString("ForwardedData[%1]").arg(i), output_leds[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set input data
    setMultiBitInput(dataA_inputs, dataA);
    setMultiBitInput(dataB_inputs, dataB);
    setMultiBitInput(dataC_inputs, dataC);
    setMultiBitInput(dataD_inputs, dataD);
    setMultiBitInput(select_inputs, select);
    sim->update();

    // Read output
    int output = readMultiBitOutput(QVector<GraphicElement *>(output_leds.begin(), output_leds.end()), 0);

    // Verify forwarded data matches expected
    QCOMPARE(output, expectedOutput);
}

void TestLevel7DataForwardingUnit::testDataForwardingUnitStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *fwd_unit = loadBuildingBlockIC("level7_data_forwarding_unit.panda");
    builder.add(fwd_unit);

    // Verify IC has been loaded
    QVERIFY(fwd_unit != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 DataA + 8 DataB + 8 DataC + 8 DataD + 2 Select = 34 inputs
    // Outputs: 8 ForwardedData = 8 outputs
    QCOMPARE(fwd_unit->inputSize(), 34);  // 32 data + 2 select
    QCOMPARE(fwd_unit->outputSize(), 8);  // 8 forwarded data

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, fwd_unit, "DataA[0]");
        builder.connect(data, 0, fwd_unit, "DataB[0]");
        builder.connect(data, 0, fwd_unit, "DataC[0]");
        builder.connect(data, 0, fwd_unit, "DataD[0]");
        builder.connect(data, 0, fwd_unit, "Select[0]");
        builder.connect(fwd_unit, "ForwardedData[0]", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}

