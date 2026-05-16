// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Instruction Set Architecture (Expanded 4-bit ISA - 16 opcodes)
enum InstructionOpcode {
    // Arithmetic & Logic (0-7)
    ISA_NOP = 0,    // 0000 - No operation
    ISA_LOAD = 1,   // 0001 - Load immediate/memory to register
    ISA_STORE = 2,  // 0010 - Store register to memory
    ISA_ADD = 3,    // 0011 - Addition
    ISA_SUB = 4,    // 0100 - Subtraction
    ISA_AND = 5,    // 0101 - Bitwise AND
    ISA_OR = 6,     // 0110 - Bitwise OR
    ISA_XOR = 7,    // 0111 - Bitwise XOR

    // Control Flow (8-12)
    ISA_JMP = 8,    // 1000 - Unconditional jump
    ISA_BEQ = 9,    // 1001 - Branch if equal (zero flag = 1)
    ISA_BNE = 10,   // 1010 - Branch if not equal (zero flag = 0)
    ISA_BLT = 11,   // 1011 - Branch if less than (negative flag = 1)
    ISA_BGE = 12,   // 1100 - Branch if greater/equal (negative flag = 0)

    // Reserved (13-15)
    ISA_RESERVED_13 = 13,
    ISA_RESERVED_14 = 14,
    ISA_RESERVED_15 = 15
};

// ALU Operations
enum ALUOperation {
    ALU_ADD = 0,
    ALU_SUB = 1,
    ALU_AND = 2,
    ALU_OR = 3,
    ALU_XOR = 4,
    ALU_NOT = 5,
    ALU_SHL = 6,
    ALU_SHR = 7
};

// Control Unit States
enum ControlUnitState {
    CU_FETCH = 0,
    CU_DECODE = 1,
    CU_EXECUTE = 2,
    CU_WRITEBACK = 3,
    CU_HALT = 4
};
