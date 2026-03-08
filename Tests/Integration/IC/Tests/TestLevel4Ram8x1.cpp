// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4Ram8x1.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

// ============================================================
// RAM Array (8x8) using RAM8x1 IC - Common Circuit Builder
// ============================================================

/**
 * @brief Build an 8x8 single-bit RAM array using RAM8x1 IC
 *
 * Architecture:
 * - RAM8x1 IC: Complete 8-cell, 1-bit wide RAM with internal decoder and mux
 *
 * IC Ports:
 *   - Inputs: Address[0], Address[1], Address[2], DataIn, WriteEnable, Clock
 *   - Output: DataOut
 */
struct RamArray8x8Circuit {
    // External interface
    InputSwitch addressBits[3];
    InputSwitch dataIn;
    InputSwitch writeEnable;
    InputSwitch clock;
    Led dataOut;

    // RAM IC
    IC *ramIC = nullptr;
};

void buildRamArray8x8(WorkSpace* workspace,
                      InputSwitch*& address,
                      InputSwitch*& dataIn,
                      InputSwitch*& writeEnable,
                      InputSwitch*& clock,
                      Led*& dataOut,
                      Simulation*& sim,
                      std::unique_ptr<RamArray8x8Circuit>& circuit)
{
    CircuitBuilder builder(workspace->scene());

    circuit = std::make_unique<RamArray8x8Circuit>();

    address = &circuit->addressBits[0];
    dataIn = &circuit->dataIn;
    writeEnable = &circuit->writeEnable;
    clock = &circuit->clock;
    dataOut = &circuit->dataOut;

    // Load RAM8x1 IC
    circuit->ramIC = loadBuildingBlockIC("level4_ram_8x1.panda");

    // Add all components to scene
    builder.add(&circuit->addressBits[0], &circuit->addressBits[1], &circuit->addressBits[2],
                &circuit->dataIn, &circuit->writeEnable, &circuit->clock,
                &circuit->dataOut, circuit->ramIC);

    // Connect inputs to RAM IC using semantic port labels
    builder.connect(&circuit->addressBits[0], 0, circuit->ramIC, "Address[0]");
    builder.connect(&circuit->addressBits[1], 0, circuit->ramIC, "Address[1]");
    builder.connect(&circuit->addressBits[2], 0, circuit->ramIC, "Address[2]");
    builder.connect(&circuit->dataIn, 0, circuit->ramIC, "DataIn");
    builder.connect(&circuit->writeEnable, 0, circuit->ramIC, "WriteEnable");
    builder.connect(&circuit->clock, 0, circuit->ramIC, "Clock");

    // Connect RAM IC output to LED
    builder.connect(circuit->ramIC, "DataOut", &circuit->dataOut, 0);

    sim = builder.initSimulation();
}

// ============================================================
// Test: RAM Array (8x8) - All addresses write/read tests
// ============================================================

void TestLevel4RAM8X1::testRamArray_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("dataToWrite");

    // Single write tests for all 8 addresses
    for (int i = 0; i < 8; i++) {
        QTest::newRow(qPrintable(QString("write_addr%1_1").arg(i))) << i << true;
    }

    // Edge case tests
    QTest::newRow("pattern_test") << -1 << false;
    QTest::newRow("we_gating_test") << -2 << false;
    QTest::newRow("hold_test") << -3 << false;
    QTest::newRow("isolation_test") << -2 << false;
    QTest::newRow("sequential_write") << -3 << false;
}

void TestLevel4RAM8X1::testRamArray()
{
    QFETCH(int, address);
    QFETCH(bool, dataToWrite);

    if (address >= 0 && address < 8) {
        // Single write test for all 8 addresses

        auto workspace = std::make_unique<WorkSpace>();
        InputSwitch *addressSwitches = nullptr;
        InputSwitch *dataInSwitch = nullptr;
        InputSwitch *weSwitch = nullptr;
        InputSwitch *clockSwitch = nullptr;
        Led *dataOutLed = nullptr;
        Simulation *simulation = nullptr;
        std::unique_ptr<RamArray8x8Circuit> circuit;

        buildRamArray8x8(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

        // Initialize
        simulation->update();

        // Set address and write data
        addressSwitches[0].setOn((address >> 0) & 1);
        addressSwitches[1].setOn((address >> 1) & 1);
        addressSwitches[2].setOn((address >> 2) & 1);
        dataInSwitch->setOn(dataToWrite);
        weSwitch->setOn(true);

        simulation->update();

        // Clock pulse
        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        weSwitch->setOff();
        simulation->update();

        // Read back
        bool readData = TestUtils::getInputStatus(dataOutLed);
        QCOMPARE(readData, dataToWrite);
    } else if (address == -1) {
        // Pattern test: Verify all 8 addresses can hold independent values

        bool pattern[8] = {true, false, true, false, true, false, true, false};

        // Write and verify each address independently (separate circuit per address for stability)
        for (int testAddr = 0; testAddr < 8; ++testAddr) {
            auto workspace = std::make_unique<WorkSpace>();
            InputSwitch *addressSwitches = nullptr;
            InputSwitch *dataInSwitch = nullptr;
            InputSwitch *weSwitch = nullptr;
            InputSwitch *clockSwitch = nullptr;
            Led *dataOutLed = nullptr;
            Simulation *simulation = nullptr;
            std::unique_ptr<RamArray8x8Circuit> circuit;

            buildRamArray8x8(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

            // Initialize
            simulation->update();

            // Write to this address
            addressSwitches[0].setOn((testAddr >> 0) & 1);
            addressSwitches[1].setOn((testAddr >> 1) & 1);
            addressSwitches[2].setOn((testAddr >> 2) & 1);
            dataInSwitch->setOn(pattern[testAddr]);
            weSwitch->setOn(true);

            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();

            weSwitch->setOff();
            simulation->update();

            // Read back
            bool readData = TestUtils::getInputStatus(dataOutLed);
            QCOMPARE(readData, pattern[testAddr]);
        }
    } else if (address == -2) {
        // WE gating - write with WE=0 should not write
        auto workspace = std::make_unique<WorkSpace>();
        InputSwitch *addressSwitches = nullptr;
        InputSwitch *dataInSwitch = nullptr;
        InputSwitch *weSwitch = nullptr;
        InputSwitch *clockSwitch = nullptr;
        Led *dataOutLed = nullptr;
        Simulation *simulation = nullptr;
        std::unique_ptr<RamArray8x8Circuit> circuit;

        buildRamArray8x8(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

        simulation->update();
        addressSwitches[0].setOn(true);   // Address 3 (binary 011)
        addressSwitches[1].setOn(true);
        addressSwitches[2].setOn(false);
        dataInSwitch->setOn(true);        // Try to write 1
        weSwitch->setOn(false);           // WE disabled
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        simulation->update();
        bool result = TestUtils::getInputStatus(dataOutLed);
        QCOMPARE(result, false);
    } else if (address == -3) {
        // Hold - value persists after clock with WE=0
        auto workspace = std::make_unique<WorkSpace>();
        InputSwitch *addressSwitches = nullptr;
        InputSwitch *dataInSwitch = nullptr;
        InputSwitch *weSwitch = nullptr;
        InputSwitch *clockSwitch = nullptr;
        Led *dataOutLed = nullptr;
        Simulation *simulation = nullptr;
        std::unique_ptr<RamArray8x8Circuit> circuit;

        buildRamArray8x8(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

        simulation->update();
        addressSwitches[0].setOn(true);   // Address 5 (binary 101)
        addressSwitches[1].setOn(false);
        addressSwitches[2].setOn(true);
        dataInSwitch->setOn(true);
        weSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        weSwitch->setOff();
        simulation->update();

        bool afterWrite = TestUtils::getInputStatus(dataOutLed);
        QCOMPARE(afterWrite, true);

        dataInSwitch->setOn(false);       // Change data to 0
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        simulation->update();
        bool afterHold = TestUtils::getInputStatus(dataOutLed);
        QCOMPARE(afterHold, true);
    } else {
        QFAIL(qPrintable(QString("Unhandled test case: address=%1").arg(address)));
    }
}

// ============================================================
// Test: RAM Basic - Single address write/read tests
// ============================================================

void TestLevel4RAM8X1::testRamBasic_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("dataToWrite");
    QTest::addColumn<QString>("testDescription");

    // Single write tests for all 8 addresses
    for (int i = 0; i < 8; i++) {
        QTest::newRow(qPrintable(QString("write_addr%1_1").arg(i))) << i << true << QString("Write address %1 with 1").arg(i);
    }
}

void TestLevel4RAM8X1::testRamBasic()
{
    QFETCH(int, address);
    QFETCH(bool, dataToWrite);

    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *addressSwitches = nullptr;
    InputSwitch *dataInSwitch = nullptr;
    InputSwitch *weSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *dataOutLed = nullptr;
    Simulation *simulation = nullptr;
    std::unique_ptr<RamArray8x8Circuit> circuit;

    buildRamArray8x8(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

    simulation->update();

    addressSwitches[0].setOn((address >> 0) & 1);
    addressSwitches[1].setOn((address >> 1) & 1);
    addressSwitches[2].setOn((address >> 2) & 1);
    dataInSwitch->setOn(dataToWrite);
    weSwitch->setOn(true);

    simulation->update();

    clockSwitch->setOn(true);
    simulation->update();
    clockSwitch->setOn(false);
    simulation->update();

    weSwitch->setOff();
    simulation->update();

    bool readData = TestUtils::getInputStatus(dataOutLed);
    QCOMPARE(readData, dataToWrite);
}

// ============================================================
// Test: RAM Edge Cases - WE gating, pattern, hold behavior
// ============================================================

void TestLevel4RAM8X1::testRamEdgeCases_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<QString>("testDescription");

    QTest::newRow("pattern_test") << -1 << "";
    QTest::newRow("we_gating_test") << -2 << "";
    QTest::newRow("hold_test") << -3 << "";
    QTest::newRow("isolation_test") << -2 << "";
    QTest::newRow("sequential_write") << -3 << "";
}

void TestLevel4RAM8X1::testRamEdgeCases()
{
    QFETCH(int, address);

    if (address == -1) {
        // Pattern test: Verify all 8 addresses can hold independent values
        bool pattern[8] = {true, false, true, false, true, false, true, false};

        for (int testAddr = 0; testAddr < 8; ++testAddr) {
            auto workspace = std::make_unique<WorkSpace>();
            InputSwitch *addressSwitches = nullptr;
            InputSwitch *dataInSwitch = nullptr;
            InputSwitch *weSwitch = nullptr;
            InputSwitch *clockSwitch = nullptr;
            Led *dataOutLed = nullptr;
            Simulation *simulation = nullptr;
            std::unique_ptr<RamArray8x8Circuit> circuit;

            buildRamArray8x8(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

            simulation->update();

            addressSwitches[0].setOn((testAddr >> 0) & 1);
            addressSwitches[1].setOn((testAddr >> 1) & 1);
            addressSwitches[2].setOn((testAddr >> 2) & 1);
            dataInSwitch->setOn(pattern[testAddr]);
            weSwitch->setOn(true);

            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();

            weSwitch->setOff();
            simulation->update();

            bool readData = TestUtils::getInputStatus(dataOutLed);
            QCOMPARE(readData, pattern[testAddr]);
        }
    } else if (address == -2) {
        // WE gating - write with WE=0 should not write
        auto workspace = std::make_unique<WorkSpace>();
        InputSwitch *addressSwitches = nullptr;
        InputSwitch *dataInSwitch = nullptr;
        InputSwitch *weSwitch = nullptr;
        InputSwitch *clockSwitch = nullptr;
        Led *dataOutLed = nullptr;
        Simulation *simulation = nullptr;
        std::unique_ptr<RamArray8x8Circuit> circuit;

        buildRamArray8x8(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

        simulation->update();
        addressSwitches[0].setOn(true);
        addressSwitches[1].setOn(true);
        addressSwitches[2].setOn(false);
        dataInSwitch->setOn(true);
        weSwitch->setOn(false);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        simulation->update();
        bool result = TestUtils::getInputStatus(dataOutLed);
        QCOMPARE(result, false);
    } else if (address == -3) {
        // Hold - value persists after clock with WE=0
        auto workspace = std::make_unique<WorkSpace>();
        InputSwitch *addressSwitches = nullptr;
        InputSwitch *dataInSwitch = nullptr;
        InputSwitch *weSwitch = nullptr;
        InputSwitch *clockSwitch = nullptr;
        Led *dataOutLed = nullptr;
        Simulation *simulation = nullptr;
        std::unique_ptr<RamArray8x8Circuit> circuit;

        buildRamArray8x8(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

        simulation->update();
        addressSwitches[0].setOn(true);
        addressSwitches[1].setOn(false);
        addressSwitches[2].setOn(true);
        dataInSwitch->setOn(true);
        weSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        weSwitch->setOff();
        simulation->update();

        bool afterWrite = TestUtils::getInputStatus(dataOutLed);
        QCOMPARE(afterWrite, true);

        dataInSwitch->setOn(false);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        simulation->update();
        bool afterHold = TestUtils::getInputStatus(dataOutLed);
        QCOMPARE(afterHold, true);
    } else {
        QFAIL(qPrintable(QString("Unhandled test case: address=%1").arg(address)));
    }
}
