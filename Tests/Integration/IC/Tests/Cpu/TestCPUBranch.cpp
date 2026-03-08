// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUBranch.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::getInputStatus;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: Branch
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU13BranchCondition
// - TestCPU14BranchUnconditional
// - TestCPU15BranchZeroFlag
// - TestCPU16BranchSignFlag
// - TestCPU17BranchIntegration
//

void TestCPUBranch::initTestCase()
{
    // Initialize test case resources if needed
}

void TestCPUBranch::cleanup()
{
    // Clean up test resources if needed
}

void TestCPUBranch::testBranchCondition()
{
    InputSwitch *decoderJMP = new InputSwitch();
    InputSwitch *decoderBEQ = new InputSwitch();
    InputSwitch *decoderBNE = new InputSwitch();
    InputSwitch *decoderBLT = new InputSwitch();
    InputSwitch *decoderBGE = new InputSwitch();
    InputSwitch *zeroFlag = new InputSwitch();
    InputSwitch *negativeFlag = new InputSwitch();
    Led *pcLoad = new Led();
    std::unique_ptr<WorkSpace> workspace(buildBranchCondition(
        decoderJMP, decoderBEQ, decoderBNE, decoderBLT, decoderBGE,
        zeroFlag, negativeFlag, pcLoad));
    auto *sim = workspace->simulation();
    // Test cases: {decoder_signals, zero, negative, expected_pcLoad}
    struct TestCase {
        bool jmp, beq, bne, blt, bge;  // Decoder signals
        bool zero, negative;            // Flag values
        bool expected;                  // Expected pcLoad
        const char *name;
    };
    TestCase tests[] = {
        // JMP always branches (regardless of flags)
        {true, false, false, false, false, false, false, true, "JMP (flags=00)"},
        {true, false, false, false, false, true, true, true, "JMP (flags=11)"},
        // BEQ branches when zero=1
        {false, true, false, false, false, true, false, true, "BEQ taken (zero=1)"},
        {false, true, false, false, false, false, false, false, "BEQ not taken (zero=0)"},
        // BNE branches when zero=0
        {false, false, true, false, false, false, false, true, "BNE taken (zero=0)"},
        {false, false, true, false, false, true, false, false, "BNE not taken (zero=1)"},
        // BLT branches when negative=1
        {false, false, false, true, false, false, true, true, "BLT taken (neg=1)"},
        {false, false, false, true, false, false, false, false, "BLT not taken (neg=0)"},
        // BGE branches when negative=0
        {false, false, false, false, true, false, false, true, "BGE taken (neg=0)"},
        {false, false, false, false, true, false, true, false, "BGE not taken (neg=1)"},
    };
    for (int testIdx = 0; testIdx < 10; ++testIdx) {
        const auto &test = tests[testIdx];
        // Set decoder signals
        decoderJMP->setOn(test.jmp);
        decoderBEQ->setOn(test.beq);
        decoderBNE->setOn(test.bne);
        decoderBLT->setOn(test.blt);
        decoderBGE->setOn(test.bge);
        // Set flags
        zeroFlag->setOn(test.zero);
        negativeFlag->setOn(test.negative);
        sim->update();
        bool result = TestUtils::getInputStatus(pcLoad, 0);
        QCOMPARE(result, test.expected);
    }
}

void TestCPUBranch::testBranchIntegration()
{
    QFETCH(int, opcode);
    QFETCH(bool, zeroFlag);
    QFETCH(bool, negativeFlag);
    QFETCH(int, targetAddr);
    QFETCH(bool, shouldBranch);
    // Build integrated branch system:
    // 1. Program Counter
    // 2. 4-to-16 Instruction Decoder
    // 3. Flag Register
    // 4. Branch Condition Evaluator
    // Each build function creates its own workspace/simulation
    // === Program Counter ===
    InputSwitch *pc_clock = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    QVector<InputSwitch *> pc_load_val;
    QVector<Led *> pc_out;
    for (int i = 0; i < 8; i++) {
        pc_load_val.append(new InputSwitch());
        pc_out.append(new Led());
    }
    InputSwitch *pc_load = new InputSwitch();
    InputSwitch *pc_inc = new InputSwitch();
    InputSwitch *pc_enable = new InputSwitch();
    // Build PC using existing function (creates its own workspace/simulation)
    std::unique_ptr<WorkSpace> workspace_pc(buildProgramCounter8bit(
        pc_load_val.data(), pc_load, pc_inc, reset, pc_enable, pc_clock, pc_out.data()));
    auto *sim_pc = workspace_pc->simulation();
    // === Instruction Decoder (4-to-16) ===
    QVector<InputSwitch *> opcode_bits;
    QVector<Led *> decoder_out;
    for (int i = 0; i < 4; i++) {
        opcode_bits.append(new InputSwitch());
    }
    for (int i = 0; i < 16; i++) {
        decoder_out.append(new Led());
    }
    std::unique_ptr<WorkSpace> workspace_decoder(buildInstructionDecoder4to16(
        opcode_bits.data(), decoder_out.data()));
    auto *sim_decoder = workspace_decoder->simulation();
    // === Flag Register ===
    InputSwitch *flag_zero_in = new InputSwitch();
    InputSwitch *flag_negative_in = new InputSwitch();
    InputSwitch *flag_write = new InputSwitch();
    InputSwitch *flag_clock = new InputSwitch();
    Led *flag_zero_out = new Led();
    Led *flag_negative_out = new Led();
    std::unique_ptr<WorkSpace> workspace_flags(buildFlagRegister(
        flag_zero_in, flag_negative_in, flag_write, flag_clock, flag_zero_out, flag_negative_out));
    auto *sim_flags = workspace_flags->simulation();
    // === Branch Condition Evaluator ===
    InputSwitch *branch_decoder_jmp = new InputSwitch();
    InputSwitch *branch_decoder_beq = new InputSwitch();
    InputSwitch *branch_decoder_bne = new InputSwitch();
    InputSwitch *branch_decoder_blt = new InputSwitch();
    InputSwitch *branch_decoder_bge = new InputSwitch();
    InputSwitch *branch_zero_flag = new InputSwitch();
    InputSwitch *branch_negative_flag = new InputSwitch();
    Led *branch_pc_load = new Led();
    std::unique_ptr<WorkSpace> workspace_branch(buildBranchCondition(
        branch_decoder_jmp, branch_decoder_beq, branch_decoder_bne,
        branch_decoder_blt, branch_decoder_bge,
        branch_zero_flag, branch_negative_flag, branch_pc_load));
    auto *sim_branch = workspace_branch->simulation();
    // === Initialize System ===
    // Reset PC
    reset->setOn(true);
    pc_enable->setOn(false);
    pc_inc->setOn(false);
    pc_load->setOn(false);
    sim_pc->update();
    clockCycle(sim_pc, pc_clock);
    sim_pc->update();
    reset->setOn(false);
    // Set up flags in flag register
    flag_zero_in->setOn(zeroFlag);
    flag_negative_in->setOn(negativeFlag);
    flag_write->setOn(true);
    sim_flags->update();
    clockCycle(sim_flags, flag_clock);
    sim_flags->update();
    flag_write->setOn(false);
    // Verify flags were stored correctly
    sim_flags->update();
    QCOMPARE(getInputStatus(flag_zero_out, 0), zeroFlag);
    QCOMPARE(getInputStatus(flag_negative_out, 0), negativeFlag);
    // === Set Opcode on Decoder ===
    for (int i = 0; i < 4; i++) {
        opcode_bits[i]->setOn((opcode >> i) & 1);
    }
    sim_decoder->update();
    // Verify decoder output (one-hot encoding)
    int decoder_value = 0;
    for (int i = 0; i < 16; i++) {
        if (getInputStatus(decoder_out[i], 0)) {
            decoder_value |= (1 << i);
        }
    }
    QCOMPARE(decoder_value, 1 << opcode);
    // === Connect Decoder Output and Flags to Branch Evaluator ===
    // (Manual wiring since separate simulations)
    branch_decoder_jmp->setOn(getInputStatus(decoder_out[ISA_JMP], 0));
    branch_decoder_beq->setOn(getInputStatus(decoder_out[ISA_BEQ], 0));
    branch_decoder_bne->setOn(getInputStatus(decoder_out[ISA_BNE], 0));
    branch_decoder_blt->setOn(getInputStatus(decoder_out[ISA_BLT], 0));
    branch_decoder_bge->setOn(getInputStatus(decoder_out[ISA_BGE], 0));
    branch_zero_flag->setOn(getInputStatus(flag_zero_out, 0));
    branch_negative_flag->setOn(getInputStatus(flag_negative_out, 0));
    sim_branch->update();
    // === Verify Branch Decision ===
    bool branchTaken = getInputStatus(branch_pc_load, 0);
    QCOMPARE(branchTaken, shouldBranch);
    // === Execute Branch if Taken ===
    if (shouldBranch) {
        // Load target address into PC
        for (int i = 0; i < 8; i++) {
            pc_load_val[i]->setOn((targetAddr >> i) & 1);
        }
        pc_load->setOn(true);
        pc_enable->setOn(true);
        sim_pc->update();
        clockCycle(sim_pc, pc_clock);  // Complete clock cycle (HIGH->settle->LOW->settle)
        sim_pc->update();
        pc_load->setOn(false);
        // Verify PC loaded branch target
        int pc_value = readMultiBitOutput(pc_out);
        QCOMPARE(pc_value, targetAddr);
    } else {
        // No branch - just increment PC
        pc_inc->setOn(true);
        pc_enable->setOn(true);
        sim_pc->update();
        clockCycle(sim_pc, pc_clock);  // Complete clock cycle (HIGH->settle->LOW->settle)
        sim_pc->update();
        // Verify PC incremented (from 0 to 1)
        int pc_value = readMultiBitOutput(pc_out);
        QCOMPARE(pc_value, 1);
    }
}

void TestCPUBranch::testBranchIntegration_data()
{
    QTest::addColumn<int>("opcode");         // Instruction opcode
    QTest::addColumn<bool>("zeroFlag");      // Zero flag state
    QTest::addColumn<bool>("negativeFlag");  // Negative flag state
    QTest::addColumn<int>("targetAddr");     // Branch target address
    QTest::addColumn<bool>("shouldBranch");  // Expected: branch taken or not
    // JMP (opcode 8): Unconditional - always branches
    QTest::newRow("jmp_unconditional") << static_cast<int>(ISA_JMP) << false << false << 0x20 << true;
    // BEQ (opcode 9): Branch if zero flag = 1
    QTest::newRow("beq_taken") << static_cast<int>(ISA_BEQ) << true << false << 0x30 << true;
    QTest::newRow("beq_not_taken") << static_cast<int>(ISA_BEQ) << false << false << 0x30 << false;
    // BNE (opcode 10): Branch if zero flag = 0
    QTest::newRow("bne_taken") << static_cast<int>(ISA_BNE) << false << false << 0x40 << true;
    QTest::newRow("bne_not_taken") << static_cast<int>(ISA_BNE) << true << false << 0x40 << false;
    // BLT (opcode 11): Branch if negative flag = 1
    QTest::newRow("blt_taken") << static_cast<int>(ISA_BLT) << false << true << 0x50 << true;
    QTest::newRow("blt_not_taken") << static_cast<int>(ISA_BLT) << false << false << 0x50 << false;
    // BGE (opcode 12): Branch if negative flag = 0
    QTest::newRow("bge_taken") << static_cast<int>(ISA_BGE) << false << false << 0x60 << true;
    QTest::newRow("bge_not_taken") << static_cast<int>(ISA_BGE) << false << true << 0x60 << false;
}

