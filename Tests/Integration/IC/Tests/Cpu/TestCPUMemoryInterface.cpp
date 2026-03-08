// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUMemoryInterface.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: MemoryInterface
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU27MemoryInterfaceWrite
// - TestCPU28MemoryInterfaceControl
// - TestCPU29MemoryInterfaceBoundary
// - TestCPU30MemoryInterfacePatterns
// - TestCPU31MemoryInterface
//

void TestCPUMemoryInterface::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

void TestCPUMemoryInterface::cleanup()
{
}

void TestCPUMemoryInterface::testMemoryInterface()
{
    QFETCH(int, address);
    QFETCH(int, dataToWrite);
    QFETCH(bool, doWrite);
    QFETCH(int, expectedData);
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

void TestCPUMemoryInterface::testMemoryInterfaceBoundary()
{
    QFETCH(int, address);
    QFETCH(int, dataToWrite);
    QFETCH(bool, doWrite);
    QFETCH(int, expectedData);
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

void TestCPUMemoryInterface::testMemoryInterfaceBoundary_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataToWrite");
    QTest::addColumn<bool>("doWrite");
    QTest::addColumn<int>("expectedData");
    // Boundary value tests (max address, zero data)
    QTest::newRow("mem_write_0x00_addr_0x3F") << 0x3F << 0x00 << true << 0x00;
    QTest::newRow("mem_write_0x42_addr_0x20") << 0x20 << 0x42 << true << 0x42;
}

void TestCPUMemoryInterface::testMemoryInterfaceControl()
{
    QFETCH(int, address);
    QFETCH(int, dataToWrite);
    QFETCH(bool, doWrite);
    QFETCH(int, expectedData);
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

void TestCPUMemoryInterface::testMemoryInterfaceControl_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataToWrite");
    QTest::addColumn<bool>("doWrite");
    QTest::addColumn<int>("expectedData");
    // Control signal tests (write enable, no write)
    QTest::newRow("mem_no_write") << 0x10 << 0x55 << false << 0x00;
    QTest::newRow("mem_multiple_addr_1") << 0x01 << 0x11 << true << 0x11;
}

void TestCPUMemoryInterface::testMemoryInterfacePatterns()
{
    QFETCH(int, address);
    QFETCH(int, dataToWrite);
    QFETCH(bool, doWrite);
    QFETCH(int, expectedData);
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

void TestCPUMemoryInterface::testMemoryInterfacePatterns_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataToWrite");
    QTest::addColumn<bool>("doWrite");
    QTest::addColumn<int>("expectedData");
    // Pattern tests (multiple addresses, bit patterns)
    QTest::newRow("mem_multiple_addr_2") << 0x02 << 0x22 << true << 0x22;
    QTest::newRow("mem_alternating") << 0x30 << 0xCC << true << 0xCC;
}

void TestCPUMemoryInterface::testMemoryInterfaceWrite()
{
    QFETCH(int, address);
    QFETCH(int, dataToWrite);
    QFETCH(bool, doWrite);
    QFETCH(int, expectedData);
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

void TestCPUMemoryInterface::testMemoryInterfaceWrite_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataToWrite");
    QTest::addColumn<bool>("doWrite");
    QTest::addColumn<int>("expectedData");
    // Basic write tests to different addresses
    QTest::newRow("mem_write_0x55_addr_0x10") << 0x10 << 0x55 << true << 0x55;
    QTest::newRow("mem_write_0xAA_addr_0x00") << 0x00 << 0xAA << true << 0xAA;
}

void TestCPUMemoryInterface::testMemoryInterface_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataToWrite");
    QTest::addColumn<bool>("doWrite");
    QTest::addColumn<int>("expectedData");
    // Basic write/read tests (all addresses within 0x00-0x3F range)
    QTest::newRow("mem_write_0x55_addr_0x10") << 0x10 << 0x55 << true << 0x55;
    QTest::newRow("mem_write_0xAA_addr_0x00") << 0x00 << 0xAA << true << 0xAA;
    QTest::newRow("mem_write_0x00_addr_0x3F") << 0x3F << 0x00 << true << 0x00;
    QTest::newRow("mem_write_0x42_addr_0x20") << 0x20 << 0x42 << true << 0x42;
    QTest::newRow("mem_no_write") << 0x10 << 0x55 << false << 0x00;
    QTest::newRow("mem_multiple_addr_1") << 0x01 << 0x11 << true << 0x11;
    QTest::newRow("mem_multiple_addr_2") << 0x02 << 0x22 << true << 0x22;
    QTest::newRow("mem_alternating") << 0x30 << 0xCC << true << 0xCC;
}

