// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPURegisterBank.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::getInputStatus;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: RegisterBank
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU18RegisterBankBasic
// - TestCPU19RegisterBankIndependence
// - TestCPU20RegisterBank
//

void TestCPURegisterBank::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

void TestCPURegisterBank::cleanup()
{
}

void TestCPURegisterBank::testRegisterBank()
{
    QFETCH(int, writeAddr);
    QFETCH(int, writeData);
    QFETCH(int, readAddrA);
    QFETCH(int, readAddrB);
    QFETCH(int, expectedReadA);
    QFETCH(int, expectedReadB);
    QVector<InputSwitch *> wAddr, wData, rAddrA, rAddrB;
    InputSwitch *writeEnable, *clock;
    QVector<Led *> readDataA, readDataB;
    for (int i = 0; i < 3; i++) {
        wAddr.append(new InputSwitch());
        rAddrA.append(new InputSwitch());
        rAddrB.append(new InputSwitch());
    }
    for (int i = 0; i < 8; i++) {
        wData.append(new InputSwitch());
        readDataA.append(new Led());
        readDataB.append(new Led());
    }
    writeEnable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildRegisterBank8x8(wAddr.data(), wData.data(), writeEnable, clock, rAddrA.data(), rAddrB.data(), readDataA.data(), readDataB.data()));
    auto *sim = workspace->simulation();
    // Set write address and data
    for (int i = 0; i < 3; i++) {
        wAddr[i]->setOn((writeAddr >> i) & 1);
    }
    for (int i = 0; i < 8; i++) {
        wData[i]->setOn((writeData >> i) & 1);
    }
    sim->update();
    // Write cycle
    writeEnable->setOn(true);
    sim->update();
    clockCycle(sim, clock);
    sim->update();
    writeEnable->setOn(false);
    sim->update();
    // Set read addresses
    for (int i = 0; i < 3; i++) {
        rAddrA[i]->setOn((readAddrA >> i) & 1);
        rAddrB[i]->setOn((readAddrB >> i) & 1);
    }
    sim->update();
    // Read outputs
    QVector<GraphicElement *> readAVec, readBVec;
    for (int i = 0; i < 8; i++) {
        readAVec.append(readDataA[i]);
        readBVec.append(readDataB[i]);
    }
    int readA = readMultiBitOutput(readAVec);
    int readB = readMultiBitOutput(readBVec);
    QCOMPARE(readA, expectedReadA);
    QCOMPARE(readB, expectedReadB);
}

void TestCPURegisterBank::testRegisterBankBasic()
{
    QFETCH(int, writeAddr);
    QFETCH(int, writeData);
    QFETCH(int, readAddrA);
    QFETCH(int, readAddrB);
    QFETCH(int, expectedReadA);
    QFETCH(int, expectedReadB);
    QVector<InputSwitch *> wAddr, wData, rAddrA, rAddrB;
    InputSwitch *writeEnable, *clock;
    QVector<Led *> readDataA, readDataB;
    for (int i = 0; i < 3; i++) {
        wAddr.append(new InputSwitch());
        rAddrA.append(new InputSwitch());
        rAddrB.append(new InputSwitch());
    }
    for (int i = 0; i < 8; i++) {
        wData.append(new InputSwitch());
        readDataA.append(new Led());
        readDataB.append(new Led());
    }
    writeEnable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildRegisterBank8x8(wAddr.data(), wData.data(), writeEnable, clock, rAddrA.data(), rAddrB.data(), readDataA.data(), readDataB.data()));
    auto *sim = workspace->simulation();
    // Set write address and data
    for (int i = 0; i < 3; i++) {
        wAddr[i]->setOn((writeAddr >> i) & 1);
    }
    for (int i = 0; i < 8; i++) {
        wData[i]->setOn((writeData >> i) & 1);
    }
    sim->update();
    // Write cycle
    writeEnable->setOn(true);
    sim->update();
    clockCycle(sim, clock);
    sim->update();
    writeEnable->setOn(false);
    sim->update();
    // Set read addresses
    for (int i = 0; i < 3; i++) {
        rAddrA[i]->setOn((readAddrA >> i) & 1);
        rAddrB[i]->setOn((readAddrB >> i) & 1);
    }
    sim->update();
    // Read outputs
    QVector<GraphicElement *> readAVec, readBVec;
    for (int i = 0; i < 8; i++) {
        readAVec.append(readDataA[i]);
        readBVec.append(readDataB[i]);
    }
    int readA = readMultiBitOutput(readAVec);
    int readB = readMultiBitOutput(readBVec);
    QCOMPARE(readA, expectedReadA);
    QCOMPARE(readB, expectedReadB);
}

void TestCPURegisterBank::testRegisterBankBasic_data()
{
    QTest::addColumn<int>("writeAddr");
    QTest::addColumn<int>("writeData");
    QTest::addColumn<int>("readAddrA");
    QTest::addColumn<int>("readAddrB");
    QTest::addColumn<int>("expectedReadA");
    QTest::addColumn<int>("expectedReadB");
    QTest::newRow("regbank_write_read_r0") << 0 << 0x42 << 0 << 0 << 0x42 << 0x42;
    QTest::newRow("regbank_write_read_r5") << 5 << 0x55 << 5 << 5 << 0x55 << 0x55;
    QTest::newRow("regbank_dual_read") << 3 << 0x33 << 3 << 3 << 0x33 << 0x33;
    QTest::newRow("regbank_pattern") << 4 << 0xAA << 4 << 4 << 0xAA << 0xAA;
}

void TestCPURegisterBank::testRegisterBankIndependence()
{
    QFETCH(int, writeAddr);
    QFETCH(int, writeData);
    QFETCH(int, readAddrA);
    QFETCH(int, readAddrB);
    QFETCH(int, expectedReadA);
    QFETCH(int, expectedReadB);
    QVector<InputSwitch *> wAddr, wData, rAddrA, rAddrB;
    InputSwitch *writeEnable, *clock;
    QVector<Led *> readDataA, readDataB;
    for (int i = 0; i < 3; i++) {
        wAddr.append(new InputSwitch());
        rAddrA.append(new InputSwitch());
        rAddrB.append(new InputSwitch());
    }
    for (int i = 0; i < 8; i++) {
        wData.append(new InputSwitch());
        readDataA.append(new Led());
        readDataB.append(new Led());
    }
    writeEnable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildRegisterBank8x8(wAddr.data(), wData.data(), writeEnable, clock, rAddrA.data(), rAddrB.data(), readDataA.data(), readDataB.data()));
    auto *sim = workspace->simulation();
    // Set write address and data
    for (int i = 0; i < 3; i++) {
        wAddr[i]->setOn((writeAddr >> i) & 1);
    }
    for (int i = 0; i < 8; i++) {
        wData[i]->setOn((writeData >> i) & 1);
    }
    sim->update();
    // Write cycle
    writeEnable->setOn(true);
    sim->update();
    clockCycle(sim, clock);
    sim->update();
    writeEnable->setOn(false);
    sim->update();
    // Set read addresses
    for (int i = 0; i < 3; i++) {
        rAddrA[i]->setOn((readAddrA >> i) & 1);
        rAddrB[i]->setOn((readAddrB >> i) & 1);
    }
    sim->update();
    // Read outputs
    QVector<GraphicElement *> readAVec, readBVec;
    for (int i = 0; i < 8; i++) {
        readAVec.append(readDataA[i]);
        readBVec.append(readDataB[i]);
    }
    int readA = readMultiBitOutput(readAVec);
    int readB = readMultiBitOutput(readBVec);
    QCOMPARE(readA, expectedReadA);
    QCOMPARE(readB, expectedReadB);
}

void TestCPURegisterBank::testRegisterBankIndependence_data()
{
    QTest::addColumn<int>("writeAddr");
    QTest::addColumn<int>("writeData");
    QTest::addColumn<int>("readAddrA");
    QTest::addColumn<int>("readAddrB");
    QTest::addColumn<int>("expectedReadA");
    QTest::addColumn<int>("expectedReadB");
    QTest::newRow("regbank_independence_1") << 1 << 0x11 << 2 << 2 << 0x00 << 0x00;
    QTest::newRow("regbank_independence_2") << 7 << 0x77 << 0 << 0 << 0x00 << 0x00;
    QTest::newRow("regbank_zero_write") << 2 << 0x00 << 2 << 2 << 0x00 << 0x00;
    QTest::newRow("regbank_all_ones") << 6 << 0xFF << 6 << 6 << 0xFF << 0xFF;
}

void TestCPURegisterBank::testRegisterBank_data()
{
    QTest::addColumn<int>("writeAddr");
    QTest::addColumn<int>("writeData");
    QTest::addColumn<int>("readAddrA");
    QTest::addColumn<int>("readAddrB");
    QTest::addColumn<int>("expectedReadA");
    QTest::addColumn<int>("expectedReadB");
    QTest::newRow("regbank_write_read_r0") << 0 << 0x42 << 0 << 0 << 0x42 << 0x42;
    QTest::newRow("regbank_write_read_r5") << 5 << 0x55 << 5 << 5 << 0x55 << 0x55;
    QTest::newRow("regbank_dual_read") << 3 << 0x33 << 3 << 3 << 0x33 << 0x33;
    QTest::newRow("regbank_independence_1") << 1 << 0x11 << 2 << 2 << 0x00 << 0x00;
    QTest::newRow("regbank_independence_2") << 7 << 0x77 << 0 << 0 << 0x00 << 0x00;
    QTest::newRow("regbank_pattern") << 4 << 0xAA << 4 << 4 << 0xAA << 0xAA;
    QTest::newRow("regbank_zero_write") << 2 << 0x00 << 2 << 2 << 0x00 << 0x00;
    QTest::newRow("regbank_all_ones") << 6 << 0xFF << 6 << 6 << 0xFF << 0xFF;
}

