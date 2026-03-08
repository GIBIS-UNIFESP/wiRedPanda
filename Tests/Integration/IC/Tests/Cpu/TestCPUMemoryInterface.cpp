// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUMemoryInterface.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;

void TestCPUMemoryInterface::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

void TestCPUMemoryInterface::cleanup()
{
}

// Helper: build a memory fixture, write data, read back, compare.
// Used by both tests to avoid duplicating setup code.
static void writeAndVerify(int address, int dataToWrite, bool doWrite, int expectedData)
{
    QVector<InputSwitch *> addr, dataIn;
    InputSwitch *memRead, *memWrite, *memEnable, *clock;
    QVector<Led *> dataOut;
    for (int i = 0; i < 8; i++) {
        addr.append(new InputSwitch());
        dataIn.append(new InputSwitch());
        dataOut.append(new Led());
    }
    memRead = new InputSwitch();
    memWrite = new InputSwitch();
    memEnable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildMemoryInterface(addr.data(), dataIn.data(), memRead, memWrite, memEnable, clock, dataOut.data()));
    auto *sim = workspace->simulation();
    // Set address and data inputs
    for (int i = 0; i < 8; i++) {
        addr[i]->setOn((address >> i) & 1);
        dataIn[i]->setOn((dataToWrite >> i) & 1);
    }
    sim->update();
    // Write cycle
    if (doWrite) {
        memWrite->setOn(true);
    }
    memEnable->setOn(true);
    sim->update();
    clockCycle(sim, clock);
    memWrite->setOn(false);
    memRead->setOn(true);
    sim->update();
    // Read memory output
    QVector<GraphicElement *> dataOutVec;
    for (int i = 0; i < 8; i++) {
        dataOutVec.append(dataOut[i]);
    }
    int result = readMultiBitOutput(dataOutVec);
    QCOMPARE(result, expectedData);
}

void TestCPUMemoryInterface::testMemoryWriteRead()
{
    // Each row writes to a unique address, so they are independent.
    // Build one fixture and run all write-then-read cases.
    struct Case { int address; int data; int expected; };
    const Case cases[] = {
        {0x10, 0x55, 0x55},
        {0x00, 0xAA, 0xAA},
        {0x3F, 0x00, 0x00},
        {0x20, 0x42, 0x42},
        {0x01, 0x11, 0x11},
        {0x02, 0x22, 0x22},
        {0x30, 0xCC, 0xCC},
    };

    QVector<InputSwitch *> addr, dataIn;
    InputSwitch *memRead, *memWrite, *memEnable, *clock;
    QVector<Led *> dataOut;
    for (int i = 0; i < 8; i++) {
        addr.append(new InputSwitch());
        dataIn.append(new InputSwitch());
        dataOut.append(new Led());
    }
    memRead = new InputSwitch();
    memWrite = new InputSwitch();
    memEnable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildMemoryInterface(addr.data(), dataIn.data(), memRead, memWrite, memEnable, clock, dataOut.data()));
    auto *sim = workspace->simulation();

    for (const auto &c : cases) {
        // Set address and data inputs
        for (int i = 0; i < 8; i++) {
            addr[i]->setOn((c.address >> i) & 1);
            dataIn[i]->setOn((c.data >> i) & 1);
        }
        sim->update();
        // Write cycle
        memWrite->setOn(true);
        memEnable->setOn(true);
        sim->update();
        clockCycle(sim, clock);
        memWrite->setOn(false);
        // Read back
        memRead->setOn(true);
        sim->update();

        QVector<GraphicElement *> dataOutVec;
        for (int i = 0; i < 8; i++) {
            dataOutVec.append(dataOut[i]);
        }
        int result = readMultiBitOutput(dataOutVec);
        QCOMPARE(result, c.expected);

        // Reset control signals for next iteration
        memRead->setOn(false);
        memEnable->setOn(false);
        sim->update();
    }
}

void TestCPUMemoryInterface::testMemoryNoWrite()
{
    // Reading without writing should return 0 (fresh memory)
    writeAndVerify(0x10, 0x55, false, 0x00);
}

