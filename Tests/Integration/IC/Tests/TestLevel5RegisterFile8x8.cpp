// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5RegisterFile8x8.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"
#include "Tests/Integration/IC/Tests/MemoryHelpers.h"

using TestUtils::getInputStatus;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel5RegisterFile8X8::initTestCase()
{
}

void TestLevel5RegisterFile8X8::cleanup()
{
}

void TestLevel5RegisterFile8X8::testRegisterFile8x8_data()
{
    QTest::addColumn<int>("registerAddr");
    QTest::addColumn<int>("dataToWrite");

    // Single write tests for all 8 registers
    for (int i = 0; i < 8; i++) {
        QTest::newRow(qPrintable(QString("write_reg%1").arg(i)))
            << i << (i * 2);
    }

    // Edge case tests
    QTest::newRow("independence_test") << -1 << 0;
}

void TestLevel5RegisterFile8X8::testRegisterFile8x8()
{
    QFETCH(int, registerAddr);
    QFETCH(int, dataToWrite);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Register File 8x8 circuit
    InputSwitch writeAddr[3], readAddr[3], writeData[8];
    InputSwitch writeEnable, clock;
    Led readData[8];

    // Load RegisterFile8x8 IC
    IC *regFileIC = loadBuildingBlockIC("level5_register_file_8x8.panda");

    // Add all elements to scene
    builder.add(&writeEnable, &clock, regFileIC);
    for (int i = 0; i < 3; i++) {
        builder.add(&writeAddr[i], &readAddr[i]);
    }
    for (int i = 0; i < 8; i++) {
        builder.add(&writeData[i], &readData[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 3; i++) {
        builder.connect(&writeAddr[i], 0, regFileIC, QString("Write_Addr[%1]").arg(i));
        builder.connect(&readAddr[i], 0, regFileIC, QString("Read_Addr1[%1]").arg(i));
    }
    for (int i = 0; i < 8; i++) {
        builder.connect(&writeData[i], 0, regFileIC, QString("Data_In[%1]").arg(i));
    }
    builder.connect(&writeEnable, 0, regFileIC, "Write_Enable");
    builder.connect(&clock, 0, regFileIC, "Clock");

    // Connect IC outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(regFileIC, QString("Read_Data1[%1]").arg(i), &readData[i], 0);
    }

    auto *simulation = builder.initSimulation();

    if (registerAddr >= 0 && registerAddr < 8) {
        // Single register write test
        int dataValue = dataToWrite;

        simulation->update();

        // Set write address and data
        writeAddr[0].setOn((registerAddr >> 0) & 1);
        writeAddr[1].setOn((registerAddr >> 1) & 1);
        writeAddr[2].setOn((registerAddr >> 2) & 1);

        for (int bit = 0; bit < 8; ++bit) {
            writeData[bit].setOn((dataValue >> bit) & 1);
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
        readAddr[2].setOn((registerAddr >> 2) & 1);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 8; ++bit) {
            if (getInputStatus(&readData[bit])) {
                readValue |= (1 << bit);
            }
        }

        QCOMPARE(readValue, dataValue);
    } else if (registerAddr == -1) {
        // Independence test
        int pattern[8] = {0, 2, 4, 6, 8, 10, 12, 14};

        // Disable writeEnable initially
        writeEnable.setOff();
        simulation->update();

        for (int regIdx = 0; regIdx < 8; ++regIdx) {
            writeAddr[0].setOn((regIdx >> 0) & 1);
            writeAddr[1].setOn((regIdx >> 1) & 1);
            writeAddr[2].setOn((regIdx >> 2) & 1);

            for (int bit = 0; bit < 8; ++bit) {
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

        // Read all and verify independence
        for (int regIdx = 0; regIdx < 8; ++regIdx) {
            readAddr[0].setOn((regIdx >> 0) & 1);
            readAddr[1].setOn((regIdx >> 1) & 1);
            readAddr[2].setOn((regIdx >> 2) & 1);
            simulation->update();

            int readValue = 0;
            for (int bit = 0; bit < 8; ++bit) {
                if (getInputStatus(&readData[bit])) {
                    readValue |= (1 << bit);
                }
            }

            QCOMPARE(readValue, pattern[regIdx]);
        }
    }
}

// ============================================================
// Boundary Tests (merged from test_memory_boundary, 8x8 rows only)
// ============================================================

void TestLevel5RegisterFile8X8::testBoundary8x8_data()
{
    QTest::addColumn<int>("boundaryValue");
    QTest::addColumn<QString>("testDescription");

    QTest::newRow("8x8_boundary_zero") << 0x00 << "8x8: Write boundary value 0x00 to all registers";
    QTest::newRow("8x8_boundary_ones") << 0xFF << "8x8: Write boundary value 0xFF to all registers";
    QTest::newRow("8x8_boundary_signbit") << 0x80 << "8x8: Write boundary value 0x80 (sign bit) to all registers";
}

void TestLevel5RegisterFile8X8::testBoundary8x8()
{
    QFETCH(int, boundaryValue);

    RegisterFile8x8Output f;
    buildRegisterFile8x8(f);
    auto *simulation = f.sim;
    InputSwitch *writeAddrBits = f.writeAddr;
    InputSwitch *writeDataBits = f.writeData;
    InputSwitch *writeEnableSwitch = &f.writeEnable;
    InputSwitch *readAddrBits = f.readAddr;
    Led *readDataBits = f.readData;
    InputSwitch *clockSwitch = &f.clock;

    simulation->update();

    // Write boundary value to all 8 registers
    for (int regIdx = 0; regIdx < 8; ++regIdx) {
        writeAddrBits[0].setOn((regIdx >> 0) & 1);
        writeAddrBits[1].setOn((regIdx >> 1) & 1);
        writeAddrBits[2].setOn((regIdx >> 2) & 1);

        for (int bit = 0; bit < 8; ++bit) {
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
    for (int regIdx = 0; regIdx < 8; ++regIdx) {
        readAddrBits[0].setOn((regIdx >> 0) & 1);
        readAddrBits[1].setOn((regIdx >> 1) & 1);
        readAddrBits[2].setOn((regIdx >> 2) & 1);
        simulation->update();

        int readValue = 0;
        for (int bit = 0; bit < 8; ++bit) {
            if (TestUtils::getInputStatus(&readDataBits[bit])) {
                readValue |= (1 << bit);
            }
        }

        QCOMPARE(readValue, boundaryValue);
    }
}
