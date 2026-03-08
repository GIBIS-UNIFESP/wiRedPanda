// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4Ram4x1.h"

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

void TestLevel4RAM4X1::initTestCase()
{
    // Initialize test environment
}

void TestLevel4RAM4X1::cleanup()
{
    // Clean up after each test
}

void TestLevel4RAM4X1::testRamArray4x4_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("dataToWrite");

    QTest::newRow("write_addr0_1") << 0 << true;
    QTest::newRow("write_addr1_0") << 1 << false;
    QTest::newRow("write_addr2_1") << 2 << true;
    QTest::newRow("write_addr3_0") << 3 << false;
}

/**
 * @brief Build a 4x4 single-bit RAM array using RAM4x1 IC
 *
 * Architecture:
 * - RAM4x1 IC: Complete 4-cell, 1-bit wide RAM with internal decoder and mux
 *
 * IC Ports:
 *   - Inputs: Address[0], Address[1], DataIn, WriteEnable, Clock
 *   - Output: DataOut
 */
struct RamArray4x4Circuit {
    // External interface (same as before for test compatibility)
    InputSwitch addressBits[2];
    InputSwitch dataIn;
    InputSwitch writeEnable;
    InputSwitch clock;
    Led dataOut;

    // RAM IC
    IC *ramIC = nullptr;
};

void buildRamArray4x4(WorkSpace *workspace,
                      InputSwitch*& address,
                      InputSwitch*& dataIn,
                      InputSwitch*& writeEnable,
                      InputSwitch*& clock,
                      Led*& dataOut,
                      Simulation*& sim,
                      std::unique_ptr<RamArray4x4Circuit>& circuit)
{
    CircuitBuilder builder(workspace->scene());

    circuit = std::make_unique<RamArray4x4Circuit>();

    address = &circuit->addressBits[0];
    dataIn = &circuit->dataIn;
    writeEnable = &circuit->writeEnable;
    clock = &circuit->clock;
    dataOut = &circuit->dataOut;

    // Load RAM4x1 IC
    circuit->ramIC = loadBuildingBlockIC("level4_ram_4x1.panda");

    // Add all components to scene
    builder.add(&circuit->addressBits[0], &circuit->addressBits[1],
                &circuit->dataIn, &circuit->writeEnable, &circuit->clock,
                &circuit->dataOut, circuit->ramIC);

    // Connect inputs to RAM IC using semantic port labels
    builder.connect(&circuit->addressBits[0], 0, circuit->ramIC, "Address[0]");
    builder.connect(&circuit->addressBits[1], 0, circuit->ramIC, "Address[1]");
    builder.connect(&circuit->dataIn, 0, circuit->ramIC, "DataIn");
    builder.connect(&circuit->writeEnable, 0, circuit->ramIC, "WriteEnable");
    builder.connect(&circuit->clock, 0, circuit->ramIC, "Clock");

    // Connect RAM IC output to LED
    builder.connect(circuit->ramIC, "DataOut", &circuit->dataOut, 0);

    sim = builder.initSimulation();
}

void TestLevel4RAM4X1::testRamArray4x4()
{
    QFETCH(int, address);
    QFETCH(bool, dataToWrite);

    // Single write test
    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *addressSwitches = nullptr;
    InputSwitch *dataInSwitch = nullptr;
    InputSwitch *weSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *dataOutLed = nullptr;
    Simulation *simulation = nullptr;
    std::unique_ptr<RamArray4x4Circuit> circuit;

    buildRamArray4x4(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

    simulation->update();

    addressSwitches[0].setOn((address >> 0) & 1);
    addressSwitches[1].setOn((address >> 1) & 1);
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

// Helper to build circuit and capture pointers
static void makeRamCircuit(std::unique_ptr<WorkSpace> &workspace,
                            InputSwitch *&addressSwitches,
                            InputSwitch *&dataInSwitch,
                            InputSwitch *&weSwitch,
                            InputSwitch *&clockSwitch,
                            Led *&dataOutLed,
                            Simulation *&simulation,
                            std::unique_ptr<RamArray4x4Circuit> &circuit)
{
    workspace = std::make_unique<WorkSpace>();
    buildRamArray4x4(workspace.get(), addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);
    simulation->update();
}

void TestLevel4RAM4X1::testPatternTest()
{
    // Verify all 4 addresses can hold independent values
    const bool pattern[4] = {true, false, true, false};

    for (int testAddr = 0; testAddr < 4; ++testAddr) {
        std::unique_ptr<WorkSpace> workspace;
        InputSwitch *addressSwitches = nullptr;
        InputSwitch *dataInSwitch = nullptr;
        InputSwitch *weSwitch = nullptr;
        InputSwitch *clockSwitch = nullptr;
        Led *dataOutLed = nullptr;
        Simulation *simulation = nullptr;
        std::unique_ptr<RamArray4x4Circuit> circuit;
        makeRamCircuit(workspace, addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

        addressSwitches[0].setOn((testAddr >> 0) & 1);
        addressSwitches[1].setOn((testAddr >> 1) & 1);
        dataInSwitch->setOn(pattern[testAddr]);
        weSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        weSwitch->setOff();
        simulation->update();

        QCOMPARE(TestUtils::getInputStatus(dataOutLed), pattern[testAddr]);
    }
}

void TestLevel4RAM4X1::testWeGating()
{
    // Write with WE=0 should not write — output should remain 0
    std::unique_ptr<WorkSpace> workspace;
    InputSwitch *addressSwitches = nullptr;
    InputSwitch *dataInSwitch = nullptr;
    InputSwitch *weSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *dataOutLed = nullptr;
    Simulation *simulation = nullptr;
    std::unique_ptr<RamArray4x4Circuit> circuit;
    makeRamCircuit(workspace, addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

    addressSwitches[0].setOn(true);   // Address 1
    addressSwitches[1].setOn(false);
    dataInSwitch->setOn(true);        // Try to write 1
    weSwitch->setOn(false);           // WE disabled
    simulation->update();

    clockSwitch->setOn(true);
    simulation->update();
    clockSwitch->setOn(false);
    simulation->update();

    simulation->update();
    QCOMPARE(TestUtils::getInputStatus(dataOutLed), false);
}

void TestLevel4RAM4X1::testHoldBehavior()
{
    // Value persists after clock when WE=0
    std::unique_ptr<WorkSpace> workspace;
    InputSwitch *addressSwitches = nullptr;
    InputSwitch *dataInSwitch = nullptr;
    InputSwitch *weSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *dataOutLed = nullptr;
    Simulation *simulation = nullptr;
    std::unique_ptr<RamArray4x4Circuit> circuit;
    makeRamCircuit(workspace, addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

    addressSwitches[0].setOn(false);  // Address 2
    addressSwitches[1].setOn(true);
    dataInSwitch->setOn(true);
    weSwitch->setOn(true);
    simulation->update();

    clockSwitch->setOn(true);
    simulation->update();
    clockSwitch->setOn(false);
    simulation->update();

    weSwitch->setOff();
    simulation->update();

    QCOMPARE(TestUtils::getInputStatus(dataOutLed), true);

    dataInSwitch->setOn(false);  // Change data to 0
    simulation->update();

    clockSwitch->setOn(true);
    simulation->update();
    clockSwitch->setOn(false);
    simulation->update();

    simulation->update();
    QCOMPARE(TestUtils::getInputStatus(dataOutLed), true);  // Should still hold 1
}

void TestLevel4RAM4X1::testIsolation()
{
    // Writes to one address should not affect other addresses
    std::unique_ptr<WorkSpace> workspace;
    InputSwitch *addressSwitches = nullptr;
    InputSwitch *dataInSwitch = nullptr;
    InputSwitch *weSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *dataOutLed = nullptr;
    Simulation *simulation = nullptr;
    std::unique_ptr<RamArray4x4Circuit> circuit;
    makeRamCircuit(workspace, addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

    // Write 1 to address 0
    addressSwitches[0].setOn(false);
    addressSwitches[1].setOn(false);
    dataInSwitch->setOn(true);
    weSwitch->setOn(true);
    simulation->update();

    clockSwitch->setOn(true);
    simulation->update();
    clockSwitch->setOn(false);
    simulation->update();
    weSwitch->setOff();
    simulation->update();

    // Write 0 to address 1
    addressSwitches[0].setOn(true);
    addressSwitches[1].setOn(false);
    dataInSwitch->setOn(false);
    weSwitch->setOn(true);
    simulation->update();

    clockSwitch->setOn(true);
    simulation->update();
    clockSwitch->setOn(false);
    simulation->update();
    weSwitch->setOff();
    simulation->update();

    // Read address 0 — should still be 1
    addressSwitches[0].setOn(false);
    addressSwitches[1].setOn(false);
    simulation->update();
    QCOMPARE(TestUtils::getInputStatus(dataOutLed), true);

    // Read address 1 — should be 0
    addressSwitches[0].setOn(true);
    addressSwitches[1].setOn(false);
    simulation->update();
    QCOMPARE(TestUtils::getInputStatus(dataOutLed), false);
}

void TestLevel4RAM4X1::testSequentialWrite()
{
    // Multiple writes to same address should work correctly
    std::unique_ptr<WorkSpace> workspace;
    InputSwitch *addressSwitches = nullptr;
    InputSwitch *dataInSwitch = nullptr;
    InputSwitch *weSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *dataOutLed = nullptr;
    Simulation *simulation = nullptr;
    std::unique_ptr<RamArray4x4Circuit> circuit;
    makeRamCircuit(workspace, addressSwitches, dataInSwitch, weSwitch, clockSwitch, dataOutLed, simulation, circuit);

    const bool writeSequence[] = {true, false, true, false};

    for (int writeIdx = 0; writeIdx < 4; ++writeIdx) {
        simulation->update();
        addressSwitches[0].setOn(false);
        addressSwitches[1].setOn(false);
        dataInSwitch->setOn(writeSequence[writeIdx]);
        weSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();
        weSwitch->setOff();
        simulation->update();

        QCOMPARE(TestUtils::getInputStatus(dataOutLed), writeSequence[writeIdx]);
    }
}
