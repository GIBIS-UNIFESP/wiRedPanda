// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputGND.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/Xor.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

// Standard library
#include <memory>

// ============================================================
// Helper Function: buildProgramCounter8bit
// ============================================================

/**
 * @brief Build an 8-bit Program Counter with increment, load, and reset
 *
 * Optimized architecture:
 * - 8x D Flip-Flops (PC value storage)
 * - RippleCarryAdder8bit IC (PC + 1 increment logic)
 * - 8x Mux gates (select PC+1 or hold)
 * - 8x PriorityMux3to1 ICs (priority: reset > load > inc/hold)
 *
 * Port mapping:
 * - Inputs: loadValue[7:0], load, inc, reset, enable, clock
 * - Outputs: pc[7:0]
 *
 * Control flow (per clock cycle):
 * 1. Reset has priority: PC = 0x00
 * 2. Load has next priority: PC = loadValue
 * 3. Increment has lower priority: PC = PC + 1
 * 4. Hold (no change): PC unchanged
 */
inline std::unique_ptr<WorkSpace> buildProgramCounter8bit(InputSwitch* loadValue[8],
                                                           InputSwitch* load,
                                                           InputSwitch* inc,
                                                           InputSwitch* reset,
                                                           InputSwitch* enable,
                                                           InputSwitch* clock,
                                                           Led* pc[8])
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load ProgramCounter8bit IC
    IC *pcIC = new IC();
    QString icFilePath = TestUtils::cpuComponentsDir() + "level6_program_counter_8bit_arithmetic.panda";
    QFileInfo icFileInfo(icFilePath);

    if (!icFileInfo.exists()) {
        throw std::runtime_error(QString("ProgramCounter8bit IC file not found at: %1").arg(icFilePath).toStdString());
    }

    // Get absolute path for IC loading and nested IC resolution
    QString absoluteIcPath = icFileInfo.absoluteFilePath();
    QString icComponentsDir = icFileInfo.absolutePath();

    // Set GlobalProperties::currentDir to IC components directory for nested IC resolution
    GlobalProperties::currentDir = icComponentsDir;
    try {
        pcIC->loadFile(absoluteIcPath);
    } catch (const Pandaception &e) {
        throw std::runtime_error(QString("Failed to load ProgramCounter8bit IC: %1").arg(e.what()).toStdString());
    }

    // Add all elements to scene (enable is unused but kept for API compatibility)
    builder.add(load, inc, reset, enable, clock, pcIC);
    for (int i = 0; i < 8; i++) {
        builder.add(loadValue[i], pc[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(loadValue[i], 0, pcIC, QString("loadValue[%1]").arg(i));
    }
    builder.connect(load, 0, pcIC, "load");
    builder.connect(inc, 0, pcIC, "inc");
    builder.connect(reset, 0, pcIC, "reset");
    builder.connect(clock, 0, pcIC, "clock");

    // Connect IC outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(pcIC, QString("pc[%1]").arg(i), pc[i], 0);
    }

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildInstructionRegister8bit
// ============================================================

/**
 * @brief Build an 8-bit Instruction Register with load control
 *
 * Architecture:
 * - 8x D Flip-Flops (IR value storage)
 * - 8x Mux gates (select between hold and load)
 *
 * Port mapping:
 * - Inputs: instruction[7:0], load, enable, clock
 * - Outputs: ir[7:0]
 *
 * Control flow (per clock cycle):
 * - load = 1: IR = instruction
 * - load = 0: IR = hold (no change)
 */
inline std::unique_ptr<WorkSpace> buildInstructionRegister8bit(InputSwitch* instruction[8],
                                                InputSwitch* load,
                                                InputSwitch* enable,
                                                InputSwitch* clock,
                                                Led* ir[8])
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 8 D flip-flops for IR storage
    QVector<DFlipFlop *> irStorage;
    QVector<Mux *> irMuxes;
    for (int i = 0; i < 8; i++) {
        irStorage.append(new DFlipFlop());
        irMuxes.append(new Mux());
    }

    // Add all elements
    for (int i = 0; i < 8; i++) {
        builder.add(instruction[i], ir[i], irStorage[i], irMuxes[i]);
    }
    builder.add(load, enable, clock);

    // Connect clock to all flip-flops
    for (int i = 0; i < 8; i++) {
        builder.connect(clock, 0, irStorage[i], 1);
    }

    // Mux: select between hold (Q) and load (instruction[i])
    for (int i = 0; i < 8; i++) {
        builder.connect(irStorage[i], 0, irMuxes[i], 0);  // Hold: Q
        builder.connect(instruction[i], 0, irMuxes[i], 1); // Load: instruction[i]
        builder.connect(load, 0, irMuxes[i], 2);          // Select: load

        // Connect mux to flip-flop D input
        builder.connect(irMuxes[i], 0, irStorage[i], 0);

        // Connect flip-flop Q to LED
        builder.connect(irStorage[i], 0, ir[i], 0);
    }

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildInstructionDecoder4to16
// ============================================================

/**
 * @brief Build a 4-to-16 instruction decoder using Decoder4to16 IC
 *
 * Supports expanded ISA with 16 opcodes:
 * 0-7: Arithmetic/Logic (NOP, LOAD, STORE, ADD, SUB, AND, OR, XOR)
 * 8-12: Control Flow (JMP, BEQ, BNE, BLT, BGE)
 * 13-15: Reserved
 */
inline std::unique_ptr<WorkSpace> buildInstructionDecoder4to16(InputSwitch* opcode[4],
                                                Led* decodedOpcode[16])
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load Decoder4to16 IC
    IC *decoder = new IC();
    builder.add(decoder);

    QString icFilePath = TestUtils::cpuComponentsDir() + "level2_decoder_4to16.panda";
    QFileInfo fileInfo(icFilePath);
    if (!fileInfo.exists()) {
        throw std::runtime_error(QString("Decoder4to16 IC not found at: %1").arg(icFilePath).toStdString());
    }

    // Set GlobalProperties::currentDir for nested IC resolution
    QString absoluteIcPath = fileInfo.absoluteFilePath();
    QString icComponentsDir = fileInfo.absolutePath();

    GlobalProperties::currentDir = icComponentsDir;
    try {
        decoder->loadFile(absoluteIcPath);
    } catch (const Pandaception &e) {
        throw std::runtime_error(QString("Failed to load Decoder4to16 IC: %1").arg(e.what()).toStdString());
    }

    // Add input/output elements
    for (int i = 0; i < 4; i++) {
        builder.add(opcode[i]);
    }
    for (int i = 0; i < 16; i++) {
        builder.add(decodedOpcode[i]);
    }

    // Connect opcode inputs to decoder (semantic port labels)
    for (int i = 0; i < 4; i++) {
        builder.connect(opcode[i], 0, decoder, QString("addr[%1]").arg(i));
    }

    // Connect decoder outputs to LEDs (semantic port labels)
    for (int i = 0; i < 16; i++) {
        builder.connect(decoder, QString("out[%1]").arg(i), decodedOpcode[i], 0);
    }

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildBranchCondition
// ============================================================

/**
 * @brief Build Branch Condition Evaluation Logic
 *
 * Evaluates branch conditions based on decoder outputs and stored flags.
 * Produces pc_load signal when a branch should be taken.
 *
 * Branch Conditions:
 * - JMP (decoder[8]): Always branch
 * - BEQ (decoder[9]): Branch if zeroFlag = 1
 * - BNE (decoder[10]): Branch if zeroFlag = 0
 * - BLT (decoder[11]): Branch if negativeFlag = 1
 * - BGE (decoder[12]): Branch if negativeFlag = 0
 */
inline std::unique_ptr<WorkSpace> buildBranchCondition(InputSwitch* decoderJMP,
                                        InputSwitch* decoderBEQ,
                                        InputSwitch* decoderBNE,
                                        InputSwitch* decoderBLT,
                                        InputSwitch* decoderBGE,
                                        InputSwitch* zeroFlag,
                                        InputSwitch* negativeFlag,
                                        Led* pcLoad)
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Flag inversion
    Not *notZero = new Not();
    Not *notNegative = new Not();

    // Branch enable ANDs (condition evaluation)
    And *beqEnable = new And();   // BEQ: decoder[9] AND zeroFlag
    And *bneEnable = new And();   // BNE: decoder[10] AND NOT zeroFlag
    And *bltEnable = new And();   // BLT: decoder[11] AND negativeFlag
    And *bgeEnable = new And();   // BGE: decoder[12] AND NOT negativeFlag

    // Final OR to combine all branch enables
    Or *branchOR = new Or();
    branchOR->setInputSize(5);    // JMP + BEQ + BNE + BLT + BGE

    // Add all elements
    builder.add(decoderJMP, decoderBEQ, decoderBNE, decoderBLT, decoderBGE);
    builder.add(zeroFlag, negativeFlag);
    builder.add(notZero, notNegative);
    builder.add(beqEnable, bneEnable, bltEnable, bgeEnable);
    builder.add(branchOR, pcLoad);

    // Flag inversion
    builder.connect(zeroFlag, 0, notZero, 0);
    builder.connect(negativeFlag, 0, notNegative, 0);

    // BEQ: decoder[9] AND zeroFlag
    builder.connect(decoderBEQ, 0, beqEnable, 0);
    builder.connect(zeroFlag, 0, beqEnable, 1);

    // BNE: decoder[10] AND NOT zeroFlag
    builder.connect(decoderBNE, 0, bneEnable, 0);
    builder.connect(notZero, 0, bneEnable, 1);

    // BLT: decoder[11] AND negativeFlag
    builder.connect(decoderBLT, 0, bltEnable, 0);
    builder.connect(negativeFlag, 0, bltEnable, 1);

    // BGE: decoder[12] AND NOT negativeFlag
    builder.connect(decoderBGE, 0, bgeEnable, 0);
    builder.connect(notNegative, 0, bgeEnable, 1);

    // Combine all branch enables with OR
    builder.connect(decoderJMP, 0, branchOR, 0);   // JMP always branches
    builder.connect(beqEnable, 0, branchOR, 1);   // BEQ
    builder.connect(bneEnable, 0, branchOR, 2);   // BNE
    builder.connect(bltEnable, 0, branchOR, 3);   // BLT
    builder.connect(bgeEnable, 0, branchOR, 4);   // BGE

    // Output
    builder.connect(branchOR, 0, pcLoad, 0);

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildALU8bit
// ============================================================

/**
 * @brief Build an 8-bit ALU with multiple operations
 */
inline std::unique_ptr<WorkSpace> buildALU8bit(InputSwitch* a[8],
                                InputSwitch* b[8],
                                InputSwitch* aluOp[3],
                                Led* result[8],
                                Led* zeroFlag,
                                Led* signFlag,
                                Led* carryFlag,
                                Led* overflowFlag)
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create operation circuits for each bit
    QVector<And *> andGates;
    QVector<Or *> orGates;
    QVector<Xor *> xorGates;
    QVector<Mux *> resultMuxes;
    for (int i = 0; i < 8; i++) {
        andGates.append(new And());
        orGates.append(new Or());
        xorGates.append(new Xor());
        resultMuxes.append(new Mux());
    }

    // Ripple-carry adder logic:
    // For each bit: sum = A XOR B XOR carry_in
    //              carry_out = (A AND B) OR ((A XOR B) AND carry_in)
    // Support for subtraction: SUB = ADD with inverted B (2's complement)

    QVector<Not *> notB;          // NOT(B) for subtraction
    QVector<Mux *> bMux;          // Select between B and NOT(B)
    QVector<Xor *> adderXor;      // A XOR (B or NOT B) for sum generation
    QVector<And *> adderAnd1;     // A AND (B or NOT B) for carry generation
    QVector<Xor *> adderXor2;     // (A XOR B) for carry with cin
    QVector<And *> adderAnd2;     // (A XOR B) AND carry_in
    QVector<Or *> adderCarryOr;   // Carry output: (A AND B) OR ((A XOR B) AND carry_in)
    QVector<Xor *> adderSum;      // Final sum: (A XOR B) XOR carry_in
    QVector<Or *> carryChain;     // Carry propagation

    for (int i = 0; i < 8; i++) {
        notB.append(new Not());
        bMux.append(new Mux());
        adderXor.append(new Xor());
        adderAnd1.append(new And());
        adderXor2.append(new Xor());
        adderAnd2.append(new And());
        adderCarryOr.append(new Or());
        adderSum.append(new Xor());
        if (i < 8) carryChain.append(new Or());  // For multi-bit carries
    }

    // Add all elements to workspace
    for (int i = 0; i < 8; i++) {
        builder.add(a[i], b[i], result[i], andGates[i], orGates[i], xorGates[i],
                    resultMuxes[i], notB[i], bMux[i], adderXor[i], adderAnd1[i], adderXor2[i],
                    adderAnd2[i], adderCarryOr[i], adderSum[i]);
        if (i < 8) builder.add(carryChain[i]);
    }
    for (int i = 0; i < 3; i++) {
        builder.add(aluOp[i]);
    }
    builder.add(zeroFlag, signFlag, carryFlag, overflowFlag);

    // 8-bit ALU with operation selection
    // aluOp encoding:
    //   000 = ADD
    //   001 = SUB
    //   010 = AND
    //   011 = OR
    //   100 = XOR

    // Use ALUSelector5way IC for 5-to-1 operation selection per bit
    // aluOp encoding:
    //   000 = ADD
    //   001 = SUB
    //   010 = AND
    //   011 = OR
    //   100 = XOR
    QVector<IC *> aluSelectors;

    // Load ALUSelector5way IC template for each bit
    for (int i = 0; i < 8; i++) {
        IC *selector = new IC();
        builder.add(selector);

        // Determine IC file path
        QString icFilePath = TestUtils::cpuComponentsDir() + "level3_alu_selector_5way.panda";
        QFileInfo fileInfo(icFilePath);

        if (!fileInfo.exists()) {
            throw std::runtime_error(QString("ALUSelector5way IC file not found at: %1").arg(icFilePath).toStdString());
        }

        // Set GlobalProperties::currentDir for nested IC resolution
        QString absoluteIcPath = fileInfo.absoluteFilePath();
        QString icComponentsDir = fileInfo.absolutePath();

        GlobalProperties::currentDir = icComponentsDir;

        // Load IC definition from file
        try {
            selector->loadFile(absoluteIcPath);
        } catch (const Pandaception &e) {
            throw std::runtime_error(QString("Failed to load ALUSelector5way IC for bit %1: %2")
                                    .arg(i)
                                    .arg(e.what())
                                    .toStdString());
        }

        // Verify IC has correct port counts (5 inputs + 3 opcode = 8 inputs, 1 output)
        if (selector->inputSize() != 8 || selector->outputSize() != 1) {
            throw std::runtime_error(QString("ALUSelector5way IC has incorrect port counts: %1 inputs (expected 8), %2 outputs (expected 1)")
                                    .arg(selector->inputSize())
                                    .arg(selector->outputSize())
                                    .toStdString());
        }

        aluSelectors.append(selector);
    }

    for (int i = 0; i < 8; i++) {
        // NOT B for 2's complement subtraction
        builder.connect(b[i], 0, notB[i], 0);

        // bMux: SELECT between B (port 0, for ADD) and NOT(B) (port 1, for SUB)
        // aluOp[0] = 0 → ADD (B), aluOp[0] = 1 → SUB (NOT B)
        builder.connect(b[i], 0, bMux[i], 0);      // B for ADD
        builder.connect(notB[i], 0, bMux[i], 1);   // NOT(B) for SUB
        builder.connect(aluOp[0], 0, bMux[i], 2);

        // AND operation
        builder.connect(a[i], 0, andGates[i], 0);
        builder.connect(b[i], 0, andGates[i], 1);

        // OR operation
        builder.connect(a[i], 0, orGates[i], 0);
        builder.connect(b[i], 0, orGates[i], 1);

        // XOR operation
        builder.connect(a[i], 0, xorGates[i], 0);
        builder.connect(b[i], 0, xorGates[i], 1);

        // Ripple-carry adder implementation
        // Step 1: Calculate A XOR (B or NOT B) (for sum and carry generation)
        builder.connect(a[i], 0, adderXor[i], 0);      // A
        builder.connect(bMux[i], 0, adderXor[i], 1);   // (B or NOT B)
        // adderXor[i] output = A XOR (B or NOT B)

        // Step 2: Calculate A AND (B or NOT B) (generate signal for carry)
        builder.connect(a[i], 0, adderAnd1[i], 0);     // A
        builder.connect(bMux[i], 0, adderAnd1[i], 1);  // (B or NOT B)
        // adderAnd1[i] output = A AND (B or NOT B)

        // Step 3: Calculate (A XOR (B or NOT B)) AND carry_in (propagate signal for carry)
        builder.connect(adderXor[i], 0, adderXor2[i], 0);  // A XOR (B or NOT B)
        // adderXor2[i] needs carry_in connected below

        builder.connect(adderXor[i], 0, adderAnd2[i], 0);  // A XOR (B or NOT B)
        // adderAnd2[i] needs carry_in connected below

        // Carry propagation wiring (will be completed after bit 0 is set up)
        if (i == 0) {
            // Bit 0: carry_in = aluOp[0] (for 2's complement: 1 when SUB, 0 when ADD)
            // For ADD: sum[0] = A[0] XOR B[0] XOR 0
            // For SUB: sum[0] = A[0] XOR NOT(B[0]) XOR 1 = A[0] - B[0] in 2's complement
            builder.connect(adderXor[i], 0, adderSum[i], 0);  // (A XOR (B or NOT B))
            builder.connect(aluOp[0], 0, adderSum[i], 1);     // carry_in for 2's complement SUB

            // For adderAnd2 to compute ((A XOR B) AND carry_in)
            // Second input should be carry_in, not connected yet - fix this
            builder.connect(aluOp[0], 0, adderXor2[i], 1);    // carry_in to adderXor2
            builder.connect(aluOp[0], 0, adderAnd2[i], 1);    // carry_in to adderAnd2

            // carry_out[0] = (A AND B) OR ((A XOR B) AND carry_in)
            builder.connect(adderAnd1[i], 0, adderCarryOr[i], 0);   // A AND B
            builder.connect(adderAnd2[i], 0, adderCarryOr[i], 1);   // ((A XOR B) AND carry_in)
        }

        // ALUSelector5way IC connections (will be completed after carry propagation)
        // Defer until all adder logic is set up
    }

    // Complete carry propagation chain for bits 1-7
    for (int i = 1; i < 8; i++) {
        // Carry-in for bit i comes from carry-out of bit i-1
        builder.connect(adderCarryOr[i-1], 0, adderXor2[i], 1);  // carry_in to (A XOR B)
        builder.connect(adderCarryOr[i-1], 0, adderAnd2[i], 1);  // carry_in to ((A XOR B) AND carry_in)

        // Sum calculation: (A XOR B) XOR carry_in
        builder.connect(adderXor[i], 0, adderSum[i], 0);     // (A XOR B)
        builder.connect(adderCarryOr[i-1], 0, adderSum[i], 1);  // carry_in

        // Carry-out calculation: (A AND B) OR ((A XOR B) AND carry_in)
        builder.connect(adderAnd1[i], 0, adderCarryOr[i], 0);   // (A AND B)
        builder.connect(adderAnd2[i], 0, adderCarryOr[i], 1);   // ((A XOR B) AND carry_in)
    }

    // Now that carry propagation is complete, connect to ALUSelector5way ICs
    // Each IC selects between 5 operation results (ADD, SUB, AND, OR, XOR)
    // based on 3-bit opcode
    for (int i = 0; i < 8; i++) {
        IC *selector = aluSelectors[i];

        // Connect the 5 operation results to ALUSelector5way IC
        // Inputs 0-4: result0 (ADD), result1 (SUB), result2 (AND), result3 (OR), result4 (XOR)
        builder.connect(adderSum[i], 0, selector, "result0");   // input 0: ADD result
        builder.connect(adderSum[i], 0, selector, "result1");   // input 1: SUB result (same output, correct by design)
        builder.connect(andGates[i], 0, selector, "result2");   // input 2: AND result
        builder.connect(orGates[i], 0, selector, "result3");    // input 3: OR result
        builder.connect(xorGates[i], 0, selector, "result4");   // input 4: XOR result

        // Connect opcode bits to ALUSelector5way IC
        // Inputs 5-7: op0, op1, op2 (3-bit opcode)
        builder.connect(aluOp[0], 0, selector, "op0");      // input 5: op0
        builder.connect(aluOp[1], 0, selector, "op1");      // input 6: op1
        builder.connect(aluOp[2], 0, selector, "op2");      // input 7: op2

        // Connect IC output to result LED
        builder.connect(selector, "out", result[i], 0);
    }

    // Flag generation
    // Zero flag: result == 0 — NOR tree over all 8 result bits
    // Sign flag: result[7] (MSB)
    // Carry flag: carry-out of MSB adder
    // Overflow flag: carry-out XOR carry into MSB (simplified to carry-out)

    // Zero flag: OR tree (4 → 2 → 1) then NOT
    Or *zOr0 = new Or(); Or *zOr1 = new Or(); Or *zOr2 = new Or(); Or *zOr3 = new Or();
    Or *zOr4 = new Or(); Or *zOr5 = new Or(); Or *zOr6 = new Or();
    Not *zNot = new Not();
    builder.add(zOr0, zOr1, zOr2, zOr3, zOr4, zOr5, zOr6, zNot);

    // Level 1: pair up bits 0-7
    builder.connect(aluSelectors[0], 0, zOr0, 0);
    builder.connect(aluSelectors[1], 0, zOr0, 1);
    builder.connect(aluSelectors[2], 0, zOr1, 0);
    builder.connect(aluSelectors[3], 0, zOr1, 1);
    builder.connect(aluSelectors[4], 0, zOr2, 0);
    builder.connect(aluSelectors[5], 0, zOr2, 1);
    builder.connect(aluSelectors[6], 0, zOr3, 0);
    builder.connect(aluSelectors[7], 0, zOr3, 1);

    // Level 2: pair up level-1 results
    builder.connect(zOr0, 0, zOr4, 0);
    builder.connect(zOr1, 0, zOr4, 1);
    builder.connect(zOr2, 0, zOr5, 0);
    builder.connect(zOr3, 0, zOr5, 1);

    // Level 3: final OR
    builder.connect(zOr4, 0, zOr6, 0);
    builder.connect(zOr5, 0, zOr6, 1);

    // Invert: zero flag is HIGH when all bits are 0
    builder.connect(zOr6, 0, zNot, 0);
    builder.connect(zNot, 0, zeroFlag, 0);

    // Sign flag: MSB of result
    // Note: We connect FROM the IC (logic element), not FROM the result LED (output element)
    builder.connect(aluSelectors[7], 0, signFlag, 0);

    // Carry flag: carry-out of the MSB adder stage
    builder.connect(adderCarryOr[7], 0, carryFlag, 0);

    // Overflow flag: signed overflow = carry_into_MSB XOR carry_out_of_MSB
    Xor *overflowXor = new Xor();
    builder.add(overflowXor);
    builder.connect(adderCarryOr[6], 0, overflowXor, 0);  // carry into bit 7
    builder.connect(adderCarryOr[7], 0, overflowXor, 1);  // carry out of bit 7
    builder.connect(overflowXor, 0, overflowFlag, 0);

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildControlUnit
// ============================================================

/**
 * @brief Build a simple Control Unit FSM (FETCH->DECODE->EXECUTE->WRITEBACK->FETCH)
 */
inline std::unique_ptr<WorkSpace> buildControlUnit(InputSwitch* reset,
                                    InputSwitch* clock,
                                    Led* state[3],
                                    Led* pcInc,
                                    Led* irLoad,
                                    Led* regWrite,
                                    Led* aluEnable,
                                    Led* flagWrite)
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 3 D flip-flops for state (FETCH=0, DECODE=1, EXECUTE=2, WRITEBACK=3, HALT=4)
    QVector<DFlipFlop *> stateReg;
    QVector<Mux *> stateMuxes;

    // State increment logic: need XOR/AND gates for ripple-carry increment
    // State goes 0→1→2→3→4→0 (wraps after 4 transitions)
    QVector<Xor *> incrementXor;  // For next state computation
    QVector<And *> carryAnd;

    for (int i = 0; i < 3; i++) {
        stateReg.append(new DFlipFlop());
        stateMuxes.append(new Mux());
        incrementXor.append(new Xor());
        carryAnd.append(new And());
    }

    // Add all elements
    for (int i = 0; i < 3; i++) {
        builder.add(state[i], stateReg[i], stateMuxes[i], incrementXor[i], carryAnd[i]);
    }
    builder.add(reset, clock, pcInc, irLoad, regWrite, aluEnable);
    if (flagWrite) {
        builder.add(flagWrite);
    }

    // Connect clock
    for (int i = 0; i < 3; i++) {
        builder.connect(clock, 0, stateReg[i], 1);
    }

    // State increment logic: compute next_state = current_state + 1
    // Implementation: Use NOT gate for bit 0 (always toggle), then ripple XOR for other bits

    // Create NOT gate for bit 0 inversion
    Not *bitNotZero = new Not();
    builder.add(bitNotZero);
    builder.connect(stateReg[0], 0, bitNotZero, 0);

    // For bit 1: next[1] = current[1] XOR current[0]  (carry from bit 0)
    builder.connect(stateReg[1], 0, incrementXor[1], 0);
    builder.connect(stateReg[0], 0, incrementXor[1], 1);

    // For bit 2: next[2] = current[2] XOR (current[0] AND current[1])
    builder.connect(stateReg[0], 0, carryAnd[0], 0);
    builder.connect(stateReg[1], 0, carryAnd[0], 1);
    builder.connect(stateReg[2], 0, incrementXor[2], 0);
    builder.connect(carryAnd[0], 0, incrementXor[2], 1);

    // WRAP-AROUND DETECTION: State should wrap from 4→0, not continue to 5
    // Detect state == 4 (binary 100): bit2=1, bit1=0, bit0=0
    Not *notBit1 = new Not();
    Not *notBit0 = new Not();
    And *detectState4 = new And();  // AND(bit2, NOT(bit1), NOT(bit0))
    And *gateCarry02 = new And();   // Second stage: AND(bit2_AND_notBit1, notBit0)

    builder.add(notBit1, notBit0, detectState4, gateCarry02);
    builder.connect(stateReg[1], 0, notBit1, 0);     // NOT(bit1)
    builder.connect(stateReg[0], 0, notBit0, 0);     // NOT(bit0)
    builder.connect(stateReg[2], 0, detectState4, 0); // bit2
    builder.connect(notBit1, 0, detectState4, 1);    // NOT(bit1)
    builder.connect(detectState4, 0, gateCarry02, 0); // AND result
    builder.connect(notBit0, 0, gateCarry02, 1);    // NOT(bit0)

    // gateCarry02 now outputs 1 when state==4, 0 otherwise
    // This is the wrap_enable signal

    // When state==4, force next state to 000 instead of incrementing
    // Create AND gates to gate the incremented values
    And *gateIncBit0 = new And();  // bit0_inc AND NOT(wrap_enable)
    And *gateIncBit1 = new And();  // bit1_inc AND NOT(wrap_enable)
    And *gateIncBit2 = new And();  // bit2_inc AND NOT(wrap_enable)
    Not *notWrap = new Not();

    builder.add(gateIncBit0, gateIncBit1, gateIncBit2, notWrap);
    builder.connect(gateCarry02, 0, notWrap, 0);   // NOT(wrap_enable)

    // Gate the incremented values
    builder.connect(bitNotZero, 0, gateIncBit0, 0);   // bit0_inc
    builder.connect(notWrap, 0, gateIncBit0, 1);      // NOT(wrap_enable)
    builder.connect(incrementXor[1], 0, gateIncBit1, 0); // bit1_inc
    builder.connect(notWrap, 0, gateIncBit1, 1);      // NOT(wrap_enable)
    builder.connect(incrementXor[2], 0, gateIncBit2, 0); // bit2_inc
    builder.connect(notWrap, 0, gateIncBit2, 1);      // NOT(wrap_enable)

    // Mux selection logic:
    // Port 0: Q (current state)
    // Port 1: Next state (gated incremented - 0 when state==4)
    // Port 2: Select signal
    //   When reset=0: select port 1 (increment or wrap)
    //   When reset=1: select port 0 (hold Q, which gets overridden by reset D logic)

    Not *resetNot = new Not();
    builder.add(resetNot);
    builder.connect(reset, 0, resetNot, 0);

    // Set up muxes
    for (int i = 0; i < 3; i++) {
        builder.connect(stateReg[i], 0, stateMuxes[i], 0);  // Port 0: Q (current state)
        builder.connect(stateMuxes[i], 0, stateReg[i], 0);  // Mux output to D input
        builder.connect(stateReg[i], 0, state[i], 0);       // Q output to LED
    }

    // Connect gated increment logic to mux port 1 for each bit
    builder.connect(gateIncBit0, 0, stateMuxes[0], 1);     // Bit 0: ~bit0 AND NOT(wrap)
    builder.connect(gateIncBit1, 0, stateMuxes[1], 1);     // Bit 1: (bit1 XOR bit0) AND NOT(wrap)
    builder.connect(gateIncBit2, 0, stateMuxes[2], 1);     // Bit 2: (bit2 XOR carry) AND NOT(wrap)

    // Connect select signal (high=select increment, low=select hold)
    for (int i = 0; i < 3; i++) {
        builder.connect(resetNot, 0, stateMuxes[i], 2);   // Select line
    }

    // Control signals based on state
    // Connect FROM flip-flop outputs (logic elements), not FROM LED outputs
    builder.connect(stateReg[0], 0, pcInc, 0);    // PC increment in FETCH (state bit 0)
    builder.connect(stateReg[0], 0, irLoad, 0);   // IR load in FETCH (state bit 0)
    builder.connect(stateReg[2], 0, aluEnable, 0); // ALU enable in EXECUTE (state bit 2)
    builder.connect(stateReg[1], 0, regWrite, 0);  // Register write in DECODE (state bit 1)

    // FlagWrite signal: assert during EXECUTE state (state 2 = 010)
    // EXECUTE = NOT(bit2) AND bit1 AND NOT(bit0)
    if (flagWrite) {
        Not *notBit2 = new Not();
        And *detectExecute = new And();
        detectExecute->setInputSize(3);  // 3-input AND gate

        builder.add(notBit2, detectExecute);
        builder.connect(stateReg[2], 0, notBit2, 0);      // NOT(bit2)
        builder.connect(notBit2, 0, detectExecute, 0);    // NOT(bit2)
        builder.connect(stateReg[1], 0, detectExecute, 1); // bit1
        builder.connect(notBit0, 0, detectExecute, 2);    // NOT(bit0) - reuse existing
        builder.connect(detectExecute, 0, flagWrite, 0);  // Output to flagWrite LED
    }

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildDecoder3to8Debug
// ============================================================

/**
 * @brief Build a 3-to-8 decoder circuit using IC component
 * Loads pre-built decoder IC from decoder_3to8.panda
 * Returns decoder outputs as LEDs for debugging
 */
inline std::unique_ptr<WorkSpace> buildDecoder3to8Debug(InputSwitch* addr[3],
                                         Led* outputs[8])
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create IC instance and load decoder definition
    IC *decoder = new IC();
    builder.add(decoder);

    // Determine IC file path (relative to test directory)
    QString icFilePath = TestUtils::cpuComponentsDir() + "level2_decoder_3to8.panda";
    QFileInfo fileInfo(icFilePath);

    if (!fileInfo.exists()) {
        throw std::runtime_error(QString("Decoder3to8 IC file not found at: %1").arg(icFilePath).toStdString());
    }

    // Set GlobalProperties::currentDir for nested IC resolution
    QString absoluteIcPath = fileInfo.absoluteFilePath();
    QString icComponentsDir = fileInfo.absolutePath();

    GlobalProperties::currentDir = icComponentsDir;

    // Load IC definition from file
    try {
        decoder->loadFile(absoluteIcPath);
    } catch (const Pandaception &e) {
        throw std::runtime_error(QString("Failed to load decoder IC: %1").arg(e.what()).toStdString());
    }

    // Verify IC has correct port counts
    if (decoder->inputSize() != 3 || decoder->outputSize() != 8) {
        throw std::runtime_error(QString("Decoder IC has incorrect port counts: %1 inputs, %2 outputs")
                                .arg(decoder->inputSize())
                                .arg(decoder->outputSize())
                                .toStdString());
    }

    // Connect external address inputs to IC using semantic port labels
    for (int i = 0; i < 3; i++) {
        builder.add(addr[i]);
        builder.connect(addr[i], 0, decoder, QString("addr[%1]").arg(i));
    }

    // Connect IC outputs to external LEDs using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.add(outputs[i]);
        builder.connect(decoder, QString("out[%1]").arg(i), outputs[i], 0);
    }

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildSingleGatedRegister
// ============================================================

/**
 * @brief Build a single 8-bit register with clock gating
 *
 * Architecture:
 * - 8 D flip-flops for storage
 * - AND gates to gate the clock signal based on decoder output and write enable
 * - Clock is gated: only asserts when BOTH decoderOut and writeEnable are HIGH
 */
inline std::unique_ptr<WorkSpace> buildSingleGatedRegister(InputSwitch* writeData[8],
                                            InputSwitch* decoderOut,
                                            InputSwitch* writeEnable,
                                            InputSwitch* clock,
                                            Led* readOut[8])
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit register (8 flip-flops)
    QVector<DFlipFlop *> registers;
    for (int b = 0; b < 8; b++) {
        registers.append(new DFlipFlop());
        builder.add(registers[b], writeData[b], readOut[b]);
    }

    // Add gating logic: AND(decoderOut, writeEnable, clock) -> gatedClock
    And *andGate1 = new And();  // AND(decoderOut, writeEnable)
    And *andGate2 = new And();  // AND(result, clock)
    builder.add(decoderOut, writeEnable, clock, andGate1, andGate2);

    // First AND: decoderOut AND writeEnable
    builder.connect(decoderOut, 0, andGate1, 0);
    builder.connect(writeEnable, 0, andGate1, 1);

    // Second AND: result AND clock
    builder.connect(andGate1, 0, andGate2, 0);
    builder.connect(clock, 0, andGate2, 1);

    // Connect gated clock to all flip-flops
    for (int b = 0; b < 8; b++) {
        builder.connect(writeData[b], 0, registers[b], 0);  // D input
        builder.connect(andGate2, 0, registers[b], 1);      // Clock input
        builder.connect(registers[b], 0, readOut[b], 0);    // Q output
    }

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildFlagRegister
// ============================================================

/**
 * @brief Build a 2-bit Flag Register for ALU status flags
 *
 * Architecture:
 * - 2 DFlipFlops: one for zero flag, one for negative flag
 * - Write-gated clock: flags update only when flagWrite is asserted
 * - Flags are latched on clock rising edge
 */
inline std::unique_ptr<WorkSpace> buildFlagRegister(InputSwitch* zeroIn,
                                     InputSwitch* negativeIn,
                                     InputSwitch* flagWrite,
                                     InputSwitch* clock,
                                     Led* zeroOut,
                                     Led* negativeOut)
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 2 DFlipFlops for flags
    DFlipFlop *zeroFF = new DFlipFlop();
    DFlipFlop *negativeFF = new DFlipFlop();

    // Clock gating: AND(flagWrite, clock)
    And *clockGate = new And();

    // Add all elements
    builder.add(zeroIn, negativeIn, flagWrite, clock);
    builder.add(zeroFF, negativeFF, clockGate);
    builder.add(zeroOut, negativeOut);

    // Clock gating logic
    builder.connect(flagWrite, 0, clockGate, 0);
    builder.connect(clock, 0, clockGate, 1);

    // Zero flag flip-flop
    builder.connect(zeroIn, 0, zeroFF, 0);         // D input
    builder.connect(clockGate, 0, zeroFF, 1);      // Gated clock
    builder.connect(zeroFF, 0, zeroOut, 0);        // Q output

    // Negative flag flip-flop
    builder.connect(negativeIn, 0, negativeFF, 0); // D input
    builder.connect(clockGate, 0, negativeFF, 1);  // Gated clock
    builder.connect(negativeFF, 0, negativeOut, 0);// Q output

    builder.initSimulation();
    return workspace;
}

// ============================================================
// Helper Function: buildRegisterBank8x8
// ============================================================

/**
 * @brief Build an 8x8-bit Register Bank with dual-port read
 */
inline std::unique_ptr<WorkSpace> buildRegisterBank8x8(InputSwitch* writeAddr[3],
                                        InputSwitch* writeData[8],
                                        InputSwitch* writeEnable,
                                        InputSwitch* clock,
                                        InputSwitch* readAddrA[3],
                                        InputSwitch* readAddrB[3],
                                        Led* readDataA[8],
                                        Led* readDataB[8])
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load ClockGatedDecoder IC (handles write address decode + write enable + clock gating)
    IC *clockGatedDecoder = new IC();
    builder.add(clockGatedDecoder);

    QString decoderPath = TestUtils::cpuComponentsDir() + "level5_clock_gated_decoder.panda";
    QFileInfo decoderInfo(decoderPath);
    if (!decoderInfo.exists()) {
        throw std::runtime_error(QString("ClockGatedDecoder IC file not found at: %1").arg(decoderPath).toStdString());
    }

    // Set GlobalProperties::currentDir for nested IC resolution
    QString absoluteDecoderPath = decoderInfo.absoluteFilePath();
    QString decoderComponentsDir = decoderInfo.absolutePath();

    GlobalProperties::currentDir = decoderComponentsDir;
    try {
        clockGatedDecoder->loadFile(absoluteDecoderPath);
    } catch (const Pandaception &e) {
        throw std::runtime_error(QString("Failed to load ClockGatedDecoder IC: %1").arg(e.what()).toStdString());
    }

    if (clockGatedDecoder->inputSize() != 5 || clockGatedDecoder->outputSize() != 8) {
        throw std::runtime_error(QString("ClockGatedDecoder IC has incorrect port counts: %1 inputs (expected 5), %2 outputs (expected 8)")
                                .arg(clockGatedDecoder->inputSize())
                                .arg(clockGatedDecoder->outputSize())
                                .toStdString());
    }

    // Load 8 Register8bit ICs (one per register address)
    QVector<IC *> registers;
    for (int r = 0; r < 8; r++) {
        IC *registerIC = new IC();
        builder.add(registerIC);

        QString regPath = TestUtils::cpuComponentsDir() + "level6_register_8bit.panda";
        QFileInfo regInfo(regPath);

        if (!regInfo.exists()) {
            throw std::runtime_error(QString("Register8bit IC file not found for register %1 at: %2")
                                    .arg(r)
                                    .arg(regPath)
                                    .toStdString());
        }

        // Set working directory for nested IC resolution
        QString absoluteRegPath = regInfo.absoluteFilePath();
        QString regComponentsDir = regInfo.absolutePath();

        GlobalProperties::currentDir = regComponentsDir;
        try {
            registerIC->loadFile(absoluteRegPath);
        } catch (const Pandaception &e) {
            throw std::runtime_error(QString("Failed to load Register8bit IC for register %1: %2")
                                    .arg(r)
                                    .arg(e.what())
                                    .toStdString());
        }

        if (registerIC->inputSize() != 11 || registerIC->outputSize() != 8) {
            throw std::runtime_error(QString("Register8bit IC has incorrect port counts for register %1: %2 inputs (expected 11), %3 outputs (expected 8)")
                                    .arg(r)
                                    .arg(registerIC->inputSize())
                                    .arg(registerIC->outputSize())
                                    .toStdString());
        }

        registers.append(registerIC);
    }

    // Read muxes: 8:1 mux per bit per read port
    QVector<Mux *> readMuxesA;
    QVector<Mux *> readMuxesB;
    for (int b = 0; b < 8; b++) {
        readMuxesA.append(new Mux());
        readMuxesB.append(new Mux());
        builder.add(readMuxesA[b], readMuxesB[b]);
        readMuxesA[b]->setInputSize(11);  // 8 data inputs + 3 select lines
        readMuxesB[b]->setInputSize(11);
    }

    // Create Vcc and Reset elements for control signals
    InputVcc *vcc = new InputVcc();
    InputSwitch *reset = new InputSwitch();
    reset->setLabel("Reset");
    builder.add(vcc, reset);

    // Add input/output elements
    for (int i = 0; i < 3; i++) {
        builder.add(writeAddr[i], readAddrA[i], readAddrB[i]);
    }
    for (int b = 0; b < 8; b++) {
        builder.add(writeData[b], readDataA[b], readDataB[b]);
    }
    builder.add(writeEnable, clock);

    // Connect write path to ClockGatedDecoder IC using semantic port labels
    // Inputs: addr0, addr1, addr2, clock, writeEnable
    for (int i = 0; i < 3; i++) {
        builder.connect(writeAddr[i], 0, clockGatedDecoder, QString("addr%1").arg(i));
    }
    builder.connect(clock, 0, clockGatedDecoder, "clock");
    builder.connect(writeEnable, 0, clockGatedDecoder, "writeEnable");

    // Connect each Register8bit IC to receive:
    // - Data inputs (8 bits)
    // - Gated clock from ClockGatedDecoder
    // - Shared clock and writeEnable
    for (int r = 0; r < 8; r++) {
        IC *reg = registers[r];

        // Connect 8-bit write data to register inputs using semantic port labels
        for (int b = 0; b < 8; b++) {
            builder.connect(writeData[b], 0, reg, QString("Data[%1]").arg(b));
        }

        // Connect gated clock from ClockGatedDecoder to register using semantic label
        builder.connect(clockGatedDecoder, QString("out%1").arg(r), reg, "Clock");

        // Connect writeEnable to register using semantic label
        builder.connect(writeEnable, 0, reg, "WriteEnable");

        // Connect Reset to reset input (active-high reset, 0 = normal operation)
        builder.connect(reset, 0, reg, "Reset");
    }

    // Connect register outputs to read muxes
    for (int b = 0; b < 8; b++) {
        // Connect all 8 registers to the 8:1 mux data inputs (inputs 0-7)
        for (int r = 0; r < 8; r++) {
            builder.connect(registers[r], b, readMuxesA[b], r);
            builder.connect(registers[r], b, readMuxesB[b], r);
        }

        // Connect read address lines to mux select inputs (inputs 8-10)
        builder.connect(readAddrA[0], 0, readMuxesA[b], 8);
        builder.connect(readAddrA[1], 0, readMuxesA[b], 9);
        builder.connect(readAddrA[2], 0, readMuxesA[b], 10);

        builder.connect(readAddrB[0], 0, readMuxesB[b], 8);
        builder.connect(readAddrB[1], 0, readMuxesB[b], 9);
        builder.connect(readAddrB[2], 0, readMuxesB[b], 10);

        // Output from mux
        builder.connect(readMuxesA[b], 0, readDataA[b], 0);
        builder.connect(readMuxesB[b], 0, readDataB[b], 0);
    }

    // Initialize simulation
    builder.initSimulation();
    auto *sim = workspace->simulation();

    // Perform reset pulse to initialize all registers to 0
    // Reset is active-high (1 = reset), so pulse 1 then set to 0
    reset->setOn(true);
    sim->update();
    sim->update();
    sim->update();
    reset->setOn(false);
    sim->update();
    sim->update();

    return workspace;
}

// ============================================================
// Helper Function: buildMemoryInterface
// ============================================================

/**
 * @brief Build Memory Interface (64x8-bit RAM with 6-bit addressing)
 *
 * Architecture:
 * - 64 memory locations (addresses 0x00-0x3F), each 8 bits wide
 * - 6-bit address decoder using 7-input AND gates (6 address bits + memWrite)
 * - Single-stage decoder for fast settling (~10 cycles vs ~45 cascaded)
 * - 64:1 read mux using cascaded 8:1 multiplexers (3 levels)
 */
inline std::unique_ptr<WorkSpace> buildMemoryInterface(InputSwitch* address[8],
                                        InputSwitch* dataIn[8],
                                        InputSwitch* memRead,
                                        InputSwitch* memWrite,
                                        InputSwitch* memEnable,
                                        InputSwitch* clock,
                                        Led* dataOut[8])
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create memory array: 64 locations x 8 bits
    QVector<QVector<DFlipFlop *>> memoryArray;
    for (int addr = 0; addr < 64; addr++) {
        QVector<DFlipFlop *> word;
        for (int b = 0; b < 8; b++) {
            word.append(new DFlipFlop());
        }
        memoryArray.append(word);
    }

    // 6-bit address decoder: 64 AND gates with 7 inputs each
    // Input pattern: 6 address bits (direct or inverted) + memWrite
    QVector<And *> addressDecoders;      // 64 decoder outputs
    QVector<And *> writeClockGates;      // Clock gating for each address
    QVector<Not *> notGatesAddr;         // 6 inverted address bits

    for (int i = 0; i < 64; i++) {
        And *decoder = new And();
        decoder->setInputSize(7);       // 6 address bits + memWrite
        addressDecoders.append(decoder);
        writeClockGates.append(new And());
    }
    for (int i = 0; i < 6; i++) {
        notGatesAddr.append(new Not());
    }

    // Read muxes: 3-level cascade of 8:1 multiplexers for 64:1 selection
    // Level 0: 8 muxes (select from 8 groups of 8 using addr[2:0])
    // Level 1: 1 mux (select from 8 level-0 outputs using addr[5:3])
    // Per data bit: 8 level-0 muxes + 1 level-1 mux = 9 muxes per bit
    QVector<QVector<IC *>> readMuxesL0;  // 8 data bits x 8 muxes each
    QVector<IC *> readMuxesL1;           // 8 data bits x 1 mux each

    // Helper to load Multiplexer8to1 IC (using level2_mux_8to1 - functionally identical to level3)
    auto loadMux8to1 = [](const QString &context) -> IC* {
        IC *mux = new IC();
        QString muxFilePath = TestUtils::cpuComponentsDir() + "level2_mux_8to1.panda";
        QFileInfo muxFileInfo(muxFilePath);

        if (!muxFileInfo.exists()) {
            throw std::runtime_error(QString("Multiplexer8to1 IC not found: %1").arg(context).toStdString());
        }

        // Set GlobalProperties::currentDir for nested IC resolution
        QString absoluteMuxPath = muxFileInfo.absoluteFilePath();
        QString muxComponentsDir = muxFileInfo.absolutePath();

        GlobalProperties::currentDir = muxComponentsDir;
        try {
            mux->loadFile(absoluteMuxPath);
        } catch (const Pandaception &e) {
            throw std::runtime_error(QString("Failed to load Multiplexer8to1 IC (%1): %2")
                                    .arg(context).arg(e.what()).toStdString());
        }
        return mux;
    };

    // Create level-0 muxes: 8 per data bit (64 total)
    for (int b = 0; b < 8; b++) {
        QVector<IC *> bitMuxes;
        for (int g = 0; g < 8; g++) {
            IC *mux = loadMux8to1(QString("L0 bit%1 group%2").arg(b).arg(g));
            bitMuxes.append(mux);
            builder.add(mux);
        }
        readMuxesL0.append(bitMuxes);
    }

    // Create level-1 muxes: 1 per data bit (8 total)
    for (int b = 0; b < 8; b++) {
        IC *mux = loadMux8to1(QString("L1 bit%1").arg(b));
        readMuxesL1.append(mux);
        builder.add(mux);
    }

    // Add all elements to scene
    for (int addr = 0; addr < 64; addr++) {
        for (int b = 0; b < 8; b++) {
            builder.add(memoryArray[addr][b]);
        }
        builder.add(addressDecoders[addr], writeClockGates[addr]);
    }
    for (int i = 0; i < 6; i++) {
        builder.add(notGatesAddr[i], address[i]);
    }
    // Add remaining address bits (6 and 7) even though unused for 64-location memory
    builder.add(address[6], address[7]);
    for (int b = 0; b < 8; b++) {
        builder.add(dataIn[b], dataOut[b]);
    }
    builder.add(memRead, memWrite, memEnable, clock);

    // Create NOT gates for address bits [5:0]
    for (int i = 0; i < 6; i++) {
        builder.connect(address[i], 0, notGatesAddr[i], 0);
    }

    // 6-bit address decoder: Direct single-stage decoding
    // For each address a (0-63), create 7-input AND:
    //   inputs 0-5: address bits (direct if bit set, inverted if clear)
    //   input 6: memWrite
    for (int a = 0; a < 64; a++) {
        And *decoder = addressDecoders[a];

        // Connect 6 address bits based on binary pattern of 'a'
        for (int bit = 0; bit < 6; bit++) {
            bool bitSet = (a & (1 << bit)) != 0;
            if (bitSet) {
                builder.connect(address[bit], 0, decoder, bit);
            } else {
                builder.connect(notGatesAddr[bit], 0, decoder, bit);
            }
        }

        // Connect memWrite as 7th input
        builder.connect(memWrite, 0, decoder, 6);
    }

    // Write clock gating: AND(decoder[a], clock) for each address
    for (int a = 0; a < 64; a++) {
        builder.connect(addressDecoders[a], 0, writeClockGates[a], 0);
        builder.connect(clock, 0, writeClockGates[a], 1);

        // Connect gated clock to all flip-flops at this address
        for (int b = 0; b < 8; b++) {
            builder.connect(writeClockGates[a], 0, memoryArray[a][b], 1);
        }
    }

    // Write data: Connect dataIn to all 64 memory locations
    for (int a = 0; a < 64; a++) {
        for (int b = 0; b < 8; b++) {
            builder.connect(dataIn[b], 0, memoryArray[a][b], 0);
        }
    }

    // Read logic: 64:1 mux using 2-level cascade of 8:1 muxes
    // Level 0: 8 muxes per bit, each selects from 8 consecutive addresses using addr[2:0]
    // Level 1: 1 mux per bit, selects from 8 L0 outputs using addr[5:3]
    for (int b = 0; b < 8; b++) {
        // Level 0: 8 muxes, each handles 8 addresses
        for (int g = 0; g < 8; g++) {
            IC *mux = readMuxesL0[b][g];

            // Connect 8 memory locations to mux data inputs (ports 0-7)
            // Group g handles addresses g*8 to g*8+7
            for (int i = 0; i < 8; i++) {
                int memAddr = g * 8 + i;
                builder.connect(memoryArray[memAddr][b], 0, mux, i);
            }

            // Connect select lines addr[2:0] to ports 8-10
            builder.connect(address[0], 0, mux, 8);
            builder.connect(address[1], 0, mux, 9);
            builder.connect(address[2], 0, mux, 10);
        }

        // Level 1: Select from 8 L0 outputs using addr[5:3]
        IC *muxL1 = readMuxesL1[b];

        // Connect 8 L0 outputs to L1 data inputs
        for (int g = 0; g < 8; g++) {
            builder.connect(readMuxesL0[b][g], 0, muxL1, g);
        }

        // Connect select lines addr[5:3] to ports 8-10
        builder.connect(address[3], 0, muxL1, 8);
        builder.connect(address[4], 0, muxL1, 9);
        builder.connect(address[5], 0, muxL1, 10);

        // Connect L1 output to dataOut
        builder.connect(muxL1, 0, dataOut[b], 0);
    }

    builder.initSimulation();
    return workspace;
}

