// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/// 8-bit CPU ISA shared by the single-cycle and multi-cycle CPU tests.
///
/// Both test translation units need the same opcode encoding. Defining it once
/// here (rather than copy-pasting into each .cpp) keeps them in agreement and
/// avoids duplicate-definition errors when the tests are combined into a single
/// unity-build translation unit.

/// ALU operation codes.
enum ALUOp { ADD = 0, SUB = 1, AND = 2, OR = 3, XOR = 4, NOT = 5, SHL = 6, SHR = 7 };

/// Encode an 8-bit ALU instruction: ALUOp in bits [5:3], regAddr in bits [2:0].
/// Bit 7 = 0, bit 6 = 0 for ALU/register operations.
inline int encodeInstruction(int aluOp, int regAddr)
{
    return ((aluOp & 0x7) << 3) | (regAddr & 0x7);
}

/// Encode a STORE instruction: write R0 to data memory at address regAddr.
/// Bit 7 = 1, bit 6 = 1 → MemWrite=1, RegWrite=0.
inline int encodeStore(int regAddr)
{
    return 0xC0 | (regAddr & 0x7);
}

/// Encode a LOAD instruction: read data memory at address regAddr.
/// Bit 7 = 1, bit 6 = 0 → MemRead=1, RegWrite=0.
inline int encodeLoad(int regAddr)
{
    return 0x80 | (regAddr & 0x7);
}
