// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUDecoders.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::getInputStatus;
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

void TestCPUDecoders::testInstructionDecoder4to16()
{
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
    // Test all 16 opcodes - each should produce one-hot output
    struct TestCase {
        int opcode;
        int expectedOneHot;
        const char *name;
    };
    TestCase tests[] = {
        // Arithmetic & Logic
        {ISA_NOP, 1 << 0, "NOP"},
        {ISA_LOAD, 1 << 1, "LOAD"},
        {ISA_STORE, 1 << 2, "STORE"},
        {ISA_ADD, 1 << 3, "ADD"},
        {ISA_SUB, 1 << 4, "SUB"},
        {ISA_AND, 1 << 5, "AND"},
        {ISA_OR, 1 << 6, "OR"},
        {ISA_XOR, 1 << 7, "XOR"},
        // Control Flow
        {ISA_JMP, 1 << 8, "JMP"},
        {ISA_BEQ, 1 << 9, "BEQ"},
        {ISA_BNE, 1 << 10, "BNE"},
        {ISA_BLT, 1 << 11, "BLT"},
        {ISA_BGE, 1 << 12, "BGE"},
        // Reserved
        {ISA_RESERVED_13, 1 << 13, "RESERVED_13"},
        {ISA_RESERVED_14, 1 << 14, "RESERVED_14"},
        {ISA_RESERVED_15, 1 << 15, "RESERVED_15"},
    };
    for (int testIdx = 0; testIdx < 16; ++testIdx) {
        const auto &test = tests[testIdx];
        // Set opcode bits (4 bits)
        for (int i = 0; i < 4; i++) {
            opcodeBits[i]->setOn((test.opcode >> i) & 1);
        }
        sim->update();
        // Read 16-bit output
        int result = 0;
        for (int i = 0; i < 16; i++) {
            if (TestUtils::getInputStatus(decodedOutput[i], 0)) {
                result |= (1 << i);
            }
        }
        QCOMPARE(result, test.expectedOneHot);
    }
}
