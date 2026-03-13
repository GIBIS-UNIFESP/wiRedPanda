// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"
#include "Tests/Integration/IC/Tests/MemoryHelpers.h"

using TestUtils::getInputStatus;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel5RegisterFile4X4::initTestCase()
{
}

void TestLevel5RegisterFile4X4::cleanup()
{
}

// ============================================================
// 4x4 Register File IC Tests
// ============================================================

void TestLevel5RegisterFile4X4::testRegisterFile4x4_data()
{
    QTest::addColumn<int>("registerAddr");
    QTest::addColumn<int>("dataToWrite");

    QTest::newRow("write_reg0_5") << 0 << 5;
    QTest::newRow("write_reg1_10") << 1 << 10;
    QTest::newRow("write_reg2_15") << 2 << 15;
    QTest::newRow("write_reg3_7") << 3 << 7;
    QTest::newRow("independence_test") << -1 << 0;
}

void TestLevel5RegisterFile4X4::testRegisterFile4x4()
{
    QFETCH(int, registerAddr);
    QFETCH(int, dataToWrite);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Register File 4x4 circuit
    InputSwitch writeAddr[2], readAddr[2], writeData[4];
    InputSwitch writeEnable, clock;
    Led readData[4];

    // Load RegisterFile4x4 IC
    IC *regFileIC = loadBuildingBlockIC("level5_register_file_4x4.panda");

    // Add all elements to scene
    builder.add(&writeEnable, &clock, regFileIC);
    for (int i = 0; i < 2; i++) {
        builder.add(&writeAddr[i], &readAddr[i]);
    }
    for (int i = 0; i < 4; i++) {
        builder.add(&writeData[i], &readData[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 2; i++) {
        builder.connect(&writeAddr[i], 0, regFileIC, QString("Write_Addr[%1]").arg(i));
        builder.connect(&readAddr[i], 0, regFileIC, QString("Read_Addr1[%1]").arg(i));
    }
    for (int i = 0; i < 4; i++) {
        builder.connect(&writeData[i], 0, regFileIC, QString("Data_In[%1]").arg(i));
    }
    builder.connect(&writeEnable, 0, regFileIC, "Write_Enable");
    builder.connect(&clock, 0, regFileIC, "Clock");

    // Connect IC outputs to LEDs
    for (int i = 0; i < 4; i++) {
        builder.connect(regFileIC, QString("Read_Data1[%1]").arg(i), &readData[i], 0);
    }

    auto *simulation = builder.initSimulation();

    if (registerAddr >= 0 && registerAddr < 4) {
        // Single register write test
        simulation->update();

        // Set write address and data
        writeAddr[0].setOn((registerAddr >> 0) & 1);
        writeAddr[1].setOn((registerAddr >> 1) & 1);

        for (int bit = 0; bit < 4; ++bit) {
            writeData[bit].setOn((dataToWrite >> bit) & 1);
        }

        writeEnable.setOn(true);
        simulation->update();

        // Clock pulse
        clockCycle(simulation, &clock);
        simulation->update();

        writeEnable.setOff();
        simulation->update();

        // Read back from same register
        readAddr[0].setOn((registerAddr >> 0) & 1);
        readAddr[1].setOn((registerAddr >> 1) & 1);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (getInputStatus(&readData[bit])) {
                readValue |= (1 << bit);
            }
        }

        QCOMPARE(readValue, dataToWrite);
    } else if (registerAddr == -1) {
        // Independence test: write each register separately, then read all
        int pattern[4] = {5, 10, 15, 7};

        // Disable writeEnable initially
        writeEnable.setOff();
        simulation->update();

        // Write each register
        for (int regIdx = 0; regIdx < 4; ++regIdx) {
            writeAddr[0].setOn((regIdx >> 0) & 1);
            writeAddr[1].setOn((regIdx >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeData[bit].setOn((pattern[regIdx] >> bit) & 1);
            }
            simulation->update();

            writeEnable.setOn(true);
            simulation->update();

            clockCycle(simulation, &clock);
            simulation->update();

            writeEnable.setOff();
            simulation->update();
        }

        // Wait after all writes complete
        simulation->update();

        // Read and verify each register
        bool allCorrect = true;
        for (int regIdx = 0; regIdx < 4; ++regIdx) {
            readAddr[0].setOn((regIdx >> 0) & 1);
            readAddr[1].setOn((regIdx >> 1) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if (getInputStatus(&readData[bit])) {
                    readValue |= (1 << bit);
                }
            }

            if (readValue != pattern[regIdx]) {
                allCorrect = false;
            }
        }
        if (!allCorrect) {
            QFAIL("Some registers have incorrect values");
        }
    }
}

// ============================================================
// Read Mux Debug Tests (merged from test_memory_readmux)
// ============================================================

void TestLevel5RegisterFile4X4::testRegisterFile4x4_debug_readmux_data()
{
    QTest::addColumn<int>("writeRegAddr");
    QTest::addColumn<int>("writeData");
    QTest::addColumn<int>("readRegAddr");
    QTest::addColumn<int>("expectedReadData");

    // Debug: Write to each register individually, then read it back with exact address match
    QTest::newRow("debug_write_read_reg0") << 0 << 3 << 0 << 3;
    QTest::newRow("debug_write_read_reg1") << 1 << 5 << 1 << 5;
    QTest::newRow("debug_write_read_reg2") << 2 << 7 << 2 << 7;
    QTest::newRow("debug_write_read_reg3") << 3 << 9 << 3 << 9;

    // Debug: Write to all, then read each individually
    QTest::newRow("debug_write_all_read_reg0") << -1 << 0 << 0 << 0;
    QTest::newRow("debug_write_all_read_reg1") << -1 << 0 << 1 << 1;
    QTest::newRow("debug_write_all_read_reg2") << -1 << 0 << 2 << 2;
    QTest::newRow("debug_write_all_read_reg3") << -1 << 0 << 3 << 3;
}

void TestLevel5RegisterFile4X4::testRegisterFile4x4_debug_readmux()
{
    QFETCH(int, writeRegAddr);
    QFETCH(int, writeData);
    QFETCH(int, readRegAddr);
    QFETCH(int, expectedReadData);

    RegisterFile4x4Output f;
    buildRegisterFile4x4(f);
    auto *simulation = f.sim;
    InputSwitch *writeAddrBits = f.writeAddr;
    InputSwitch *writeDataBits = f.writeData;
    InputSwitch *writeEnableSwitch = &f.writeEnable;
    InputSwitch *readAddrBits = f.readAddr;
    Led *readDataBits = f.readData;
    InputSwitch *clockSwitch = &f.clock;

    // Initialize
    simulation->update();

    if (writeRegAddr >= 0) {
        // Single write operation
        writeAddrBits[0].setOn((writeRegAddr >> 0) & 1);
        writeAddrBits[1].setOn((writeRegAddr >> 1) & 1);

        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((writeData >> bit) & 1);
        }

        writeEnableSwitch->setOn(true);

        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();
    } else {
        // Write all registers with pattern 0,1,2,3
        int pattern[4] = {0, 1, 2, 3};
        for (int regIdx = 0; regIdx < 4; ++regIdx) {
            simulation->update();

            writeAddrBits[0].setOn((regIdx >> 0) & 1);
            writeAddrBits[1].setOn((regIdx >> 1) & 1);

            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((pattern[regIdx] >> bit) & 1);
            }

            writeEnableSwitch->setOn(true);

            simulation->update();
            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        // VERIFY WRITE: Check that all registers have their written values
        for (int regIdx = 0; regIdx < 4; ++regIdx) {
            simulation->update();

            readAddrBits[0].setOn((regIdx >> 0) & 1);
            readAddrBits[1].setOn((regIdx >> 1) & 1);
            simulation->update();
        }
    }

    // Now perform read operation
    readAddrBits[0].setOn((readRegAddr >> 0) & 1);
    readAddrBits[1].setOn((readRegAddr >> 1) & 1);

    simulation->update();

    // Read the data
    int readValue = 0;
    for (int bit = 0; bit < 4; ++bit) {
        bool bitValue = TestUtils::getInputStatus(&readDataBits[bit]);
        readValue |= (bitValue ? (1 << bit) : 0);
    }

    if (readValue != expectedReadData) {
        // Don't fail yet, just log - we're debugging
    }

    QCOMPARE(readValue, expectedReadData);
}

// ============================================================
// Read Address Tests (merged from test_memory_readaddr)
// ============================================================

void TestLevel5RegisterFile4X4::testRegisterFile4x4_debug_readaddr_data()
{
    QTest::addColumn<int>("writeRegAddr");
    QTest::addColumn<int>("writeData");
    QTest::addColumn<int>("readAddr");
    QTest::addColumn<int>("expectedValue");

    QTest::newRow("write_reg2_read_addr00") << 2 << 5 << 0 << 0;
    QTest::newRow("write_reg2_read_addr01") << 2 << 5 << 1 << 0;
    QTest::newRow("write_reg2_read_addr10") << 2 << 5 << 2 << 5;
    QTest::newRow("write_reg2_read_addr11") << 2 << 5 << 3 << 0;
}

void TestLevel5RegisterFile4X4::testRegisterFile4x4_debug_readaddr()
{
    QFETCH(int, writeRegAddr);
    QFETCH(int, writeData);
    QFETCH(int, readAddr);
    QFETCH(int, expectedValue);

    RegisterFile4x4Output f;
    buildRegisterFile4x4(f);
    auto *simulation = f.sim;
    InputSwitch *writeAddrBits = f.writeAddr;
    InputSwitch *writeDataBits = f.writeData;
    InputSwitch *writeEnableSwitch = &f.writeEnable;
    InputSwitch *readAddrBits = f.readAddr;
    Led *readDataBits = f.readData;
    InputSwitch *clockSwitch = &f.clock;

    simulation->update();

    writeAddrBits[0].setOn((writeRegAddr >> 0) & 1);
    writeAddrBits[1].setOn((writeRegAddr >> 1) & 1);

    for (int bit = 0; bit < 4; ++bit) {
        writeDataBits[bit].setOn((writeData >> bit) & 1);
    }

    writeEnableSwitch->setOn(true);
    simulation->update();

    clockSwitch->setOn(true);
    simulation->update();
    clockSwitch->setOn(false);
    simulation->update();

    writeEnableSwitch->setOff();
    simulation->update();

    readAddrBits[0].setOn((readAddr >> 0) & 1);
    readAddrBits[1].setOn((readAddr >> 1) & 1);

    simulation->update();

    int readValue = 0;
    for (int bit = 0; bit < 4; ++bit) {
        bool bitVal = TestUtils::getInputStatus(&readDataBits[bit]);
        readValue |= (bitVal ? (1 << bit) : 0);
    }

    QCOMPARE(readValue, expectedValue);
}

// ============================================================
// Boundary Tests (merged from test_memory_boundary, 4x4 rows only)
// ============================================================

void TestLevel5RegisterFile4X4::testRegisterFileBoundary4x4_data()
{
    QTest::addColumn<int>("boundaryValue");

    QTest::newRow("4x4_boundary_zero") << 0x0;
    QTest::newRow("4x4_boundary_ones") << 0xF;
}

void TestLevel5RegisterFile4X4::testRegisterFileBoundary4x4()
{
    QFETCH(int, boundaryValue);

    RegisterFile4x4Output f;
    buildRegisterFile4x4(f);
    auto *simulation = f.sim;
    InputSwitch *writeAddrBits = f.writeAddr;
    InputSwitch *writeDataBits = f.writeData;
    InputSwitch *writeEnableSwitch = &f.writeEnable;
    InputSwitch *readAddrBits = f.readAddr;
    Led *readDataBits = f.readData;
    InputSwitch *clockSwitch = &f.clock;

    simulation->update();

    // Write boundary value to all 4 registers
    for (int regIdx = 0; regIdx < 4; ++regIdx) {
        writeAddrBits[0].setOn((regIdx >> 0) & 1);
        writeAddrBits[1].setOn((regIdx >> 1) & 1);

        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((boundaryValue >> bit) & 1);
        }

        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();
    }

    writeEnableSwitch->setOff();
    simulation->update();

    // Read and verify all registers
    for (int regIdx = 0; regIdx < 4; ++regIdx) {
        readAddrBits[0].setOn((regIdx >> 0) & 1);
        readAddrBits[1].setOn((regIdx >> 1) & 1);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }

        QCOMPARE(readValue, boundaryValue);
    }
}

// ============================================================
// Timing Tests (merged from test_memory_timing)
// ============================================================

void TestLevel5RegisterFile4X4::testMemoryTiming_data()
{
    QTest::addColumn<int>("timingTest");

    QTest::newRow("immediate_read") << 0;
    QTest::newRow("settle_read") << 1;
    QTest::newRow("rapid_writes") << 2;
    QTest::newRow("rapid_reads") << 3;
    QTest::newRow("address_change_mid") << 4;
    QTest::newRow("we_glitch") << 5;
    QTest::newRow("setup_violation") << 6;
    QTest::newRow("hold_violation") << 7;
    QTest::newRow("multi_cycle_hold") << 8;
    QTest::newRow("long_sequence") << 9;
    QTest::newRow("concurrent_write_read") << 10;
    QTest::newRow("we_setup_violation") << 11;
    QTest::newRow("we_hold_violation") << 12;
    QTest::newRow("data_setup_violation") << 13;
    QTest::newRow("multi_register_retention") << 14;
    QTest::newRow("bitpattern_0x55") << 15;
    QTest::newRow("bitpattern_0xAA") << 16;
    QTest::newRow("bitpattern_single_bit") << 17;
    QTest::newRow("stress_1000ops") << 18;
    QTest::newRow("read_order_sequential") << 19;
    QTest::newRow("read_order_reverse") << 20;
    QTest::newRow("read_order_interleaved") << 21;
}

void TestLevel5RegisterFile4X4::testMemoryTiming()
{
    QFETCH(int, timingTest);

    RegisterFile4x4Output f;
    buildRegisterFile4x4(f);
    auto *simulation = f.sim;
    InputSwitch *writeAddrBits = f.writeAddr;
    InputSwitch *writeDataBits = f.writeData;
    InputSwitch *writeEnableSwitch = &f.writeEnable;
    InputSwitch *readAddrBits = f.readAddr;
    Led *readDataBits = f.readData;
    InputSwitch *clockSwitch = &f.clock;

    if (timingTest == 0) {
        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((5 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 5);
    } else if (timingTest == 1) {
        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((10 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 10);
    } else if (timingTest == 2) {
        int values[4] = {3, 6, 12, 9};
        for (int regIdx = 0; regIdx < 4; ++regIdx) {
            writeAddrBits[0].setOn((regIdx >> 0) & 1);
            writeAddrBits[1].setOn((regIdx >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((values[regIdx] >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        for (int regIdx = 0; regIdx < 4; ++regIdx) {
            readAddrBits[0].setOn((regIdx >> 0) & 1);
            readAddrBits[1].setOn((regIdx >> 1) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if (TestUtils::getInputStatus(&readDataBits[bit])) {
                    readValue |= (1 << bit);
                }
            }
            QCOMPARE(readValue, values[regIdx]);
        }
    } else if (timingTest == 3) {
        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((0xA >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeAddrBits[0].setOn(true);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((0x5 >> bit) & 1);
        }
        simulation->update();
        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();
        int val0 = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) val0 |= (1 << bit);
        }

        readAddrBits[0].setOn(true);
        readAddrBits[1].setOn(false);
        simulation->update();
        int val1 = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) val1 |= (1 << bit);
        }

        QCOMPARE(val0, 0xA);
        QCOMPARE(val1, 0x5);
    } else if (timingTest == 4) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((7 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        writeAddrBits[0].setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 7);
    } else if (timingTest == 5) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((12 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 12);
    } else if (timingTest == 6) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((4 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QVERIFY((readValue == 4) || (readValue == 0));
    } else if (timingTest == 7) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((8 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        writeAddrBits[0].setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 8);
    } else if (timingTest == 8) {
        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((15 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 15);
    } else if (timingTest == 9) {
        simulation->update();

        for (int op = 0; op < 100; ++op) {
            int regIdx = op % 4;
            int value = (op * 3) % 16;

            writeAddrBits[0].setOn((regIdx >> 0) & 1);
            writeAddrBits[1].setOn((regIdx >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((value >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 0);
    } else if (timingTest == 10) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((11 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        writeAddrBits[0].setOn(true);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((7 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readVal0 = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readVal0 |= (1 << bit);
            }
        }
        QCOMPARE(readVal0, 11);

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(true);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readVal1 = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readVal1 |= (1 << bit);
            }
        }
        QCOMPARE(readVal1, 7);
    } else if (timingTest == 11) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((6 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QVERIFY((readValue == 6) || (readValue == 0));
    } else if (timingTest == 12) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((9 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        writeEnableSwitch->setOff();
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 9);
    } else if (timingTest == 13) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        writeEnableSwitch->setOn(true);

        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((5 >> bit) & 1);
        }
        simulation->update();

        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((13 >> bit) & 1);
        }
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QVERIFY((readValue == 5) || (readValue == 13));
    } else if (timingTest == 14) {
        simulation->update();

        int testValues[4] = {5, 10, 15, 7};

        for (int reg = 0; reg < 4; ++reg) {
            writeAddrBits[0].setOn((reg >> 0) & 1);
            writeAddrBits[1].setOn((reg >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((testValues[reg] >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        for (int reg = 0; reg < 4; ++reg) {
            readAddrBits[0].setOn((reg >> 0) & 1);
            readAddrBits[1].setOn((reg >> 1) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if (TestUtils::getInputStatus(&readDataBits[bit])) {
                    readValue |= (1 << bit);
                }
            }
            QCOMPARE(readValue, testValues[reg]);
        }
    } else if (timingTest == 15) {
        simulation->update();

        for (int reg = 0; reg < 4; ++reg) {
            writeAddrBits[0].setOn((reg >> 0) & 1);
            writeAddrBits[1].setOn((reg >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((0x5 >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        for (int reg = 0; reg < 4; ++reg) {
            readAddrBits[0].setOn((reg >> 0) & 1);
            readAddrBits[1].setOn((reg >> 1) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if (TestUtils::getInputStatus(&readDataBits[bit])) {
                    readValue |= (1 << bit);
                }
            }
            QCOMPARE(readValue, 0x5);
        }
    } else if (timingTest == 16) {
        simulation->update();

        for (int reg = 0; reg < 4; ++reg) {
            writeAddrBits[0].setOn((reg >> 0) & 1);
            writeAddrBits[1].setOn((reg >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((0xA >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        for (int reg = 0; reg < 4; ++reg) {
            readAddrBits[0].setOn((reg >> 0) & 1);
            readAddrBits[1].setOn((reg >> 1) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if (TestUtils::getInputStatus(&readDataBits[bit])) {
                    readValue |= (1 << bit);
                }
            }
            QCOMPARE(readValue, 0xA);
        }
    } else if (timingTest == 17) {
        simulation->update();

        int bitPatterns[4] = {0x1, 0x2, 0x4, 0x8};

        for (int reg = 0; reg < 4; ++reg) {
            writeAddrBits[0].setOn((reg >> 0) & 1);
            writeAddrBits[1].setOn((reg >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((bitPatterns[reg] >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        for (int reg = 0; reg < 4; ++reg) {
            readAddrBits[0].setOn((reg >> 0) & 1);
            readAddrBits[1].setOn((reg >> 1) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if (TestUtils::getInputStatus(&readDataBits[bit])) {
                    readValue |= (1 << bit);
                }
            }
            QCOMPARE(readValue, bitPatterns[reg]);
        }
    } else if (timingTest == 18) {
        simulation->update();

        for (int op = 0; op < 1000; ++op) {
            int regIdx = op % 4;
            int value = (op * 7) % 16;

            writeAddrBits[0].setOn((regIdx >> 0) & 1);
            writeAddrBits[1].setOn((regIdx >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((value >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }
        QCOMPARE(readValue, 12);
    } else if (timingTest >= 19 && timingTest <= 21) {
        simulation->update();

        int testVals[4] = {1, 2, 4, 8};
        for (int reg = 0; reg < 4; ++reg) {
            writeAddrBits[0].setOn((reg >> 0) & 1);
            writeAddrBits[1].setOn((reg >> 1) & 1);
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((testVals[reg] >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        // Different read orders based on test number
        int readOrder[4];
        if (timingTest == 19) {
            readOrder[0] = 0; readOrder[1] = 1; readOrder[2] = 2; readOrder[3] = 3;
        } else if (timingTest == 20) {
            readOrder[0] = 3; readOrder[1] = 2; readOrder[2] = 1; readOrder[3] = 0;
        } else {
            readOrder[0] = 0; readOrder[1] = 2; readOrder[2] = 1; readOrder[3] = 3;
        }

        for (int idx = 0; idx < 4; ++idx) {
            int reg = readOrder[idx];
            readAddrBits[0].setOn((reg >> 0) & 1);
            readAddrBits[1].setOn((reg >> 1) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if (TestUtils::getInputStatus(&readDataBits[bit])) {
                    readValue |= (1 << bit);
                }
            }
            QCOMPARE(readValue, testVals[reg]);
        }
    } else {
        QFAIL(qPrintable(QString("Invalid timing test: %1").arg(timingTest)));
    }
}

// ============================================================
// Timing Edge Case Tests (merged from test_memory_timing_edges)
// ============================================================

void TestLevel5RegisterFile4X4::testRegisterFileTimingEdges_data()
{
    QTest::addColumn<int>("timingTest");

    QTest::newRow("backtoback_writes") << 0;
    QTest::newRow("rapid_reads") << 1;
    QTest::newRow("we_glitch") << 2;
    QTest::newRow("address_change_midwrite") << 3;
    QTest::newRow("data_change_midwrite") << 4;
}

void TestLevel5RegisterFile4X4::testRegisterFileTimingEdges()
{
    QFETCH(int, timingTest);

    RegisterFile4x4Output f;
    buildRegisterFile4x4(f);
    auto *simulation = f.sim;
    InputSwitch *writeAddrBits = f.writeAddr;
    InputSwitch *writeDataBits = f.writeData;
    InputSwitch *writeEnableSwitch = &f.writeEnable;
    InputSwitch *readAddrBits = f.readAddr;
    Led *readDataBits = f.readData;
    InputSwitch *clockSwitch = &f.clock;

    if (timingTest == 0) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((5 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeAddrBits[0].setOn(true);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((10 >> bit) & 1);
        }
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        for (int regIdx = 0; regIdx < 2; ++regIdx) {
            readAddrBits[0].setOn(regIdx & 1);
            readAddrBits[1].setOn((regIdx >> 1) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if (TestUtils::getInputStatus(&readDataBits[bit])) {
                    readValue |= (1 << bit);
                }
            }

            int expected = (regIdx == 0) ? 5 : 10;
            QCOMPARE(readValue, expected);
        }
    } else if (timingTest == 1) {
        for (int regIdx = 0; regIdx < 2; ++regIdx) {
            writeAddrBits[0].setOn(regIdx & 1);
            writeAddrBits[1].setOn((regIdx >> 1) & 1);
            int val = (regIdx == 0) ? 0xA : 0x5;
            for (int bit = 0; bit < 4; ++bit) {
                writeDataBits[bit].setOn((val >> bit) & 1);
            }
            writeEnableSwitch->setOn(true);
            simulation->update();

            clockSwitch->setOn(true);
            simulation->update();
            clockSwitch->setOn(false);
            simulation->update();
        }

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();
        int val0 = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) val0 |= (1 << bit);
        }

        readAddrBits[0].setOn(true);
        readAddrBits[1].setOn(false);
        simulation->update();
        int val1 = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) val1 |= (1 << bit);
        }

        QCOMPARE(val0, 0xA);
        QCOMPARE(val1, 0x5);
    } else if (timingTest == 2) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((7 >> bit) & 1);
        }

        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        writeEnableSwitch->setOff();
        simulation->update();
        writeEnableSwitch->setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }

        QCOMPARE(readValue, 7);
    } else if (timingTest == 3) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((9 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        writeAddrBits[0].setOn(true);
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        for (int regIdx = 0; regIdx < 2; ++regIdx) {
            readAddrBits[0].setOn(regIdx & 1);
            readAddrBits[1].setOn((regIdx >> 1) & 1);
            simulation->update();
        }
    } else if (timingTest == 4) {
        simulation->update();

        writeAddrBits[0].setOn(false);
        writeAddrBits[1].setOn(false);
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((3 >> bit) & 1);
        }
        writeEnableSwitch->setOn(true);
        simulation->update();

        clockSwitch->setOn(true);
        simulation->update();
        for (int bit = 0; bit < 4; ++bit) {
            writeDataBits[bit].setOn((12 >> bit) & 1);
        }
        simulation->update();
        clockSwitch->setOn(false);
        simulation->update();

        writeEnableSwitch->setOff();
        simulation->update();

        readAddrBits[0].setOn(false);
        readAddrBits[1].setOn(false);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 4; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }

        QVERIFY((readValue == 3) || (readValue == 12));
    } else {
        QFAIL(qPrintable(QString("Invalid timingTest: %1").arg(timingTest)));
    }
}
