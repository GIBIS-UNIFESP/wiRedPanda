// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUDecoders.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::inputStatus;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: Decoders
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU11Decoder3To8
// - TestCPU12InstructionDecoder4To16
//

void TestCPUDecoders::initTestCase()
{
    // Initialize test case resources if needed
}

void TestCPUDecoders::cleanup()
{
    // Clean up test resources if needed
}

void TestCPUDecoders::testDecoder3to8()
{
    InputSwitch *addr[3];
    Led *outputs[8];
    for (int i = 0; i < 3; i++) {
        addr[i] = new InputSwitch();
    }
    for (int i = 0; i < 8; i++) {
        outputs[i] = new Led();
    }
    std::unique_ptr<WorkSpace> workspace(buildDecoder3to8Debug(addr, outputs));
    auto *sim = workspace->simulation();
    // Test each address from 0-7
    for (int address = 0; address < 8; address++) {
        // Set address bits
        for (int i = 0; i < 3; i++) {
            addr[i]->setOn((address >> i) & 1);
        }
        // Settle and read outputs
        sim->update();
        // Read decoder outputs as multi-bit value by converting to QVector
        QVector<GraphicElement *> outputsVec;
        for (int i = 0; i < 8; i++) {
            outputsVec.append(outputs[i]);
        }
        int activeBits = readMultiBitOutput(outputsVec);
        // Count active bits
        int activeCount = 0;
        for (int i = 0; i < 8; i++) {
            if ((activeBits >> i) & 1) {
                activeCount++;
            }
        }
        // Verify one-hot output
        QCOMPARE(activeCount, 1);  // Exactly one output should be high
        QCOMPARE(activeBits, 1 << address);  // The correct output should be high
    }
}

void TestCPUDecoders::testInstructionDecoder4to16_data()
{
    QTest::addColumn<int>("opcode");
    QTest::addColumn<int>("expectedOneHot");

    // Test all 16 opcodes - each should produce one-hot output. ISA_* are InstructionOpcode
    // values, not int -- cast explicitly since the "opcode" column is declared as int (matching
    // how the rest of this test treats opcodes as plain bit patterns).
    // Arithmetic & Logic
    QTest::newRow("NOP") << static_cast<int>(ISA_NOP) << (1 << 0);
    QTest::newRow("LOAD") << static_cast<int>(ISA_LOAD) << (1 << 1);
    QTest::newRow("STORE") << static_cast<int>(ISA_STORE) << (1 << 2);
    QTest::newRow("ADD") << static_cast<int>(ISA_ADD) << (1 << 3);
    QTest::newRow("SUB") << static_cast<int>(ISA_SUB) << (1 << 4);
    QTest::newRow("AND") << static_cast<int>(ISA_AND) << (1 << 5);
    QTest::newRow("OR") << static_cast<int>(ISA_OR) << (1 << 6);
    QTest::newRow("XOR") << static_cast<int>(ISA_XOR) << (1 << 7);
    // Control Flow
    QTest::newRow("JMP") << static_cast<int>(ISA_JMP) << (1 << 8);
    QTest::newRow("BEQ") << static_cast<int>(ISA_BEQ) << (1 << 9);
    QTest::newRow("BNE") << static_cast<int>(ISA_BNE) << (1 << 10);
    QTest::newRow("BLT") << static_cast<int>(ISA_BLT) << (1 << 11);
    QTest::newRow("BGE") << static_cast<int>(ISA_BGE) << (1 << 12);
    // Reserved
    QTest::newRow("RESERVED_13") << static_cast<int>(ISA_RESERVED_13) << (1 << 13);
    QTest::newRow("RESERVED_14") << static_cast<int>(ISA_RESERVED_14) << (1 << 14);
    QTest::newRow("RESERVED_15") << static_cast<int>(ISA_RESERVED_15) << (1 << 15);
}

void TestCPUDecoders::testInstructionDecoder4to16()
{
    QFETCH(int, opcode);
    QFETCH(int, expectedOneHot);

    QVector<InputSwitch *> opcodeBits;
    QVector<Led *> decodedOutput;
    for (int i = 0; i < 4; i++) {
        opcodeBits.append(new InputSwitch());
    }
    for (int i = 0; i < 16; i++) {
        decodedOutput.append(new Led());
    }
    std::unique_ptr<WorkSpace> workspace(buildInstructionDecoder4to16(opcodeBits.data(), decodedOutput.data()));
    auto *sim = workspace->simulation();

    // Set opcode bits (4 bits)
    for (int i = 0; i < 4; i++) {
        opcodeBits[i]->setOn((opcode >> i) & 1);
    }
    sim->update();
    // Read 16-bit output
    int result = 0;
    for (int i = 0; i < 16; i++) {
        if (TestUtils::inputStatus(decodedOutput[i], 0)) {
            result |= (1 << i);
        }
    }
    QCOMPARE(result, expectedOneHot);
}
