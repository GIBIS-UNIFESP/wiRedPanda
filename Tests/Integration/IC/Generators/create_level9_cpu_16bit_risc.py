#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 16-bit RISC CPU IC

Implements a 16-bit single-cycle CPU with a real register file and real
Load/Store data memory (previously this IC only wired Fetch+ALU and left
DestReg as a raw zero-extended ALU operand -- the "RISC Characteristics"
below were aspirational until this revision):
- 16-bit instruction word with 5-bit opcode
- 16-bit ALU with full arithmetic/logic operations
- 32 x 16-bit register file, addressed by the full 5-bit DestReg field
- 16-bit Load/Store data memory
- 16-bit program counter and data paths
- Single-cycle execution: combinational decode straight from the fetched
  instruction word (no pipeline; the level9_single_cycle pattern)

Inputs:
  Clock (synchronization signal)
  Reset (initialize program counter to 0 — must be held asserted for the whole
         instruction-memory programming window, not just pulsed before it: PCInc/
         InstrLoad are unconditionally tied to Vcc, so every clock pulse used to
         write a word would otherwise also advance the fetch PC. Reset's async
         override on the PC/instruction register is what prevents that drift —
         the same mechanism level9_multi_cycle_cpu_8bit documents for its own
         phase counter, applied directly here since there's no phase counter)
  ProgAddr[0..7] (instruction memory programming address — F53)
  ProgData[0..15] (16-bit instruction word to program)
  ProgWrite (instruction memory write enable)
  RegProgAddr[0..4] (register file programming address)
  RegProgData[0..15] (register file programming data)
  RegProgWrite (register file programming write enable)

Outputs:
  PC[0..7] (program counter for memory addressing)
  Result[0..15] (memory-stage output: ALU result, or loaded data when the
                 current instruction is a Load — same DataOut-is-the-Result
                 convention the 8-bit CPUs use)
  Instruction[0..15] (registered 16-bit instruction word)
  OpCode[0..4] (5-bit operation code, LSB-first, from the registered
                instruction — the live/combinational OpCode driving control
                signals is tapped from RawInstr internally, see Architecture)

16-bit Instruction Format:
  Bits [15:11] = OpCode (5 bits): low 3 bits select the ALU op; the two
                 previously-dead high bits now select instruction class via
                 level8_decode_stage's existing MemRead/MemWrite derivation —
                 00=ALU, 10=Load, 11=Store
  Bits [10:6]  = DestReg (5 bits): register file address, used as BOTH the
                 read operand and the write-back destination (Rd = op(Rd,
                 imm) for ALU ops; Rd = Mem[imm] for Load; Mem[imm] = Rd for
                 Store, Rd unchanged) — an accumulator-style single-register-
                 operand ISA like the 8-bit CPUs, except each instruction
                 picks its own accumulator via DestReg instead of always
                 using a fixed R0
  Bits [5:0]   = SrcBits (6 bits): immediate/source value for ALU ops; low 3
                 bits double as the data-memory address for Load/Store
                 (aliased modulo 8 — the memory stage's RAM only decodes 3 of
                 the 8 address bits it exposes, same precedent as
                 level8_memory_stage)

Architecture:
  - 16-bit Fetch stage with programming port and instruction register
    (InstrLoad held high; the CPU decodes from RawInstr for zero-delay
    single-cycle execution, the level9_single_cycle pattern — F53)
  - level8_decode_stage (reused verbatim — it's pure combinational logic on a
    5-bit OpCode with no data-width dependency) turns the live/RawInstr-
    derived OpCode into ALUOp[0..2]/MemRead/MemWrite. Its own RegWrite output
    (NOT(OpCode[4])) is NOT used here — that formula excludes both Load and
    Store from writing back, the existing 8-bit-CPU convention this stage
    was built for, but a Load that never persists anywhere would be nearly
    useless. This CPU derives its own write-back enable instead: NOT(decode
    MemWrite) — true for ALU ops and Load, false only for Store.
  - level9_register_file_32x16 (32 registers, single read port — OperandA is
    always the SrcBits immediate, never a register, so a second read port
    would go unused) provides Reg[DestReg] for both the ALU's OperandB and
    Store's memory data
  - 16-bit ALU computing ALUOp(SrcBits, Reg[DestReg]): OperandA = zero-
    extended SrcBits (RawInstr[0..5]), OperandB = Reg[DestReg] (full 16
    bits), ALUOp from the decode stage — all LSB-first
  - level9_memory_stage_16bit provides Load/Store data memory, addressed by
    zero-extended SrcBits[0..2]; its Result-passthrough is the ALU Result, so
    DataOut is ALU result (ALU ops) or loaded data (Load), matching the
    8-bit CPUs' own Result-is-memory-stage-DataOut convention
  - Register file write-back: Data_In = memory stage DataOut (no separate
    top-level write-back mux needed — the memory stage already selects
    between load data and ALU result); WriteEnable = (NOT(MemWrite) AND NOT
    ProgWrite AND NOT Reset) OR RegProgWrite, gated the same way the 8-bit
    CPUs gate their own register file write enable
  - PCLoad/PCData tied to explicit GND (no jumps; F34 convention)

RISC Characteristics:
  - Load/Store architecture (data memory is only reachable via Load/Store,
    never directly through the ALU)
  - Fixed 16-bit instructions
  - Real 32-register file, addressed by the full 5-bit DestReg field
  - Simple control logic (level8_decode_stage's 2-bit opcode class + 3-bit
    ALU op, reused verbatim from the 8-bit CPU family)

Usage:
    python create_level9_cpu_16bit_risc.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder

# The stage ICs (Fetch/Decode/ALU/Memory) are placed with many ports spread
# symmetrically above and below their nominal y, not a simple top-anchored
# 64px box -- level9_fetch_stage_16bit and level7_alu_16bit are both far
# taller than the flat 64px assumed elsewhere. The gap immediately above the
# stages row can't be derived from a query (nothing is instantiated there
# yet), so use half a documented worst case plus margin; every other gap in
# this file is a real measurement (queried via instantiate_ic_with_size).
STAGE_CLEARANCE_ABOVE = (900.0 / 2) + VERTICAL_STAGE_SPACING


class CPU16BitRISCBuilder(ICBuilderBase):
    """Builder for 16-bit RISC CPU IC"""

    async def create(self) -> bool:
        """Create the 16-bit RISC CPU IC"""
        await self.begin_build("16-bit RISC CPU")
        if not await self.create_new_circuit():
            return False

        base_x = 50.0

        # ---- Band Y anchors (top to bottom) ----
        y_control = 100.0
        y_logic = y_control + 64.0 + VERTICAL_STAGE_SPACING
        y_stages = y_logic + 64.0 + STAGE_CLEARANCE_ABOVE

        # ==== Band: Clock/Reset/Vcc/Gnd (boundary control inputs) ====
        clock_id = await self.create_element("Clock", base_x, y_control, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", base_x + HORIZONTAL_GATE_SPACING, y_control, "Reset")
        if reset_id is None:
            return False

        vcc_id = await self.create_element("InputVcc", base_x + (2 * HORIZONTAL_GATE_SPACING), y_control, "Vcc")
        if vcc_id is None:
            return False

        gnd_id = await self.create_element("InputGnd", base_x + (3 * HORIZONTAL_GATE_SPACING), y_control, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Clock, Reset, Vcc, and Gnd inputs")

        # ==== Band: register-file write-enable logic (not boundary elements —
        # order among them doesn't affect the IC's port order). Labels here
        # are long enough that the standard 1x step isn't enough clearance. ====
        logic_row_step = 1.5 * HORIZONTAL_GATE_SPACING
        prog_not_id = await self.create_element("Not", base_x, y_logic, "ProgWriteNOT")
        if prog_not_id is None:
            return False
        reset_not_id = await self.create_element("Not", base_x + logic_row_step, y_logic, "ResetNOT")
        if reset_not_id is None:
            return False
        # level8_decode_stage's own RegWrite output is NOT(OpCode[4]) — it
        # excludes BOTH Load and Store from writing back (the existing 8-bit
        # CPU convention this stage was built for). A Load that never persists
        # anywhere is barely useful, so this CPU computes its own write-back
        # enable instead: NOT(MemWrite) — true for ALU ops AND Load, false
        # only for Store — reusing decode's already-computed MemWrite rather
        # than re-deriving MemOp/OpCode[3] locally.
        load_regwrite_not_id = await self.create_element(
            "Not", base_x + (2 * logic_row_step), y_logic, "LoadRegWriteNOT"
        )
        if load_regwrite_not_id is None:
            return False
        we_and1_id = await self.create_element("And", base_x + (3 * logic_row_step), y_logic, "WriteEnableAND1")
        if we_and1_id is None:
            return False
        we_and2_x = base_x + (4 * logic_row_step)
        we_and2_id = await self.create_element("And", we_and2_x, y_logic, "WriteEnableAND2")
        if we_and2_id is None:
            return False
        we_or_id = await self.create_element("Or", we_and2_x + logic_row_step, y_logic, "WriteEnableOR")
        if we_or_id is None:
            return False

        await self.log("  ✓ Created register file write-enable logic")

        # ==== Band: pipeline stage ICs, chained left to right by real width ====
        fetch_x = base_x
        fetch_handle = await self.instantiate_ic_with_size("level9_fetch_stage_16bit", fetch_x, y_stages, "Fetch_16bit")
        if fetch_handle is None:
            return False
        fetch_id = fetch_handle.element_id

        decode_x = fetch_x + max(HORIZONTAL_GATE_SPACING * 2, fetch_handle.width + HORIZONTAL_GATE_SPACING)
        decode_handle = await self.instantiate_ic_with_size("level8_decode_stage", decode_x, y_stages, "Decode")
        if decode_handle is None:
            return False
        decode_id = decode_handle.element_id

        alu_x = decode_x + max(HORIZONTAL_GATE_SPACING * 2, decode_handle.width + HORIZONTAL_GATE_SPACING)
        alu_handle = await self.instantiate_ic_with_size("level7_alu_16bit", alu_x, y_stages, "ALU_16bit")
        if alu_handle is None:
            return False
        alu_id = alu_handle.element_id

        memstage_x = alu_x + max(HORIZONTAL_GATE_SPACING * 2, alu_handle.width + HORIZONTAL_GATE_SPACING)
        memstage_handle = await self.instantiate_ic_with_size(
            "level9_memory_stage_16bit", memstage_x, y_stages, "MemStage_16bit"
        )
        if memstage_handle is None:
            return False
        memstage_id = memstage_handle.element_id

        await self.log("  ✓ Instantiated Fetch, Decode, ALU, and Memory stages")

        stages_height = max(fetch_handle.height, decode_handle.height, alu_handle.height, memstage_handle.height)
        y_regfile = y_stages + stages_height + VERTICAL_STAGE_SPACING

        # ==== Band: register file + its own programming muxes (bespoke here —
        # build_cpu_register_programming_block's shape, 8x8/2-read-port/3-bit
        # address, doesn't match this 32x16/1-read-port/5-bit-address file) ====
        regfile_x = base_x
        regfile_handle = await self.instantiate_ic_with_size(
            "level9_register_file_32x16", regfile_x, y_regfile, "RegFile"
        )
        if regfile_handle is None:
            return False
        regfile_id = regfile_handle.element_id

        write_data_mux_low_x = regfile_x + max(
            HORIZONTAL_GATE_SPACING * 2, regfile_handle.width + HORIZONTAL_GATE_SPACING
        )
        write_data_mux_low_handle = await self.instantiate_ic_with_size(
            "level4_bus_mux_8bit", write_data_mux_low_x, y_regfile, "WriteDataMux_Low"
        )
        if write_data_mux_low_handle is None:
            return False
        write_data_mux_low_id = write_data_mux_low_handle.element_id

        write_data_mux_high_x = write_data_mux_low_x + max(
            HORIZONTAL_GATE_SPACING * 2, write_data_mux_low_handle.width + HORIZONTAL_GATE_SPACING
        )
        write_data_mux_high_handle = await self.instantiate_ic_with_size(
            "level4_bus_mux_8bit", write_data_mux_high_x, y_regfile, "WriteDataMux_High"
        )
        if write_data_mux_high_handle is None:
            return False
        write_data_mux_high_id = write_data_mux_high_handle.element_id

        write_addr_mux_base_x = write_data_mux_high_x + max(
            HORIZONTAL_GATE_SPACING * 2, write_data_mux_high_handle.width + HORIZONTAL_GATE_SPACING
        )
        write_addr_mux_ids = []
        for i in range(5):
            mux_id = await self.create_element(
                "Mux", write_addr_mux_base_x + (i * 1.5 * HORIZONTAL_GATE_SPACING), y_regfile, f"WriteAddrMux{i}"
            )
            if mux_id is None:
                return False
            write_addr_mux_ids.append(mux_id)

        await self.log("  ✓ Instantiated register file and its write-back muxes")

        regfile_row_height = max(
            regfile_handle.height, write_data_mux_low_handle.height, write_data_mux_high_handle.height
        )
        y_prog = y_regfile + regfile_row_height + VERTICAL_STAGE_SPACING

        # ==== Band: instruction-memory + register-file programming inputs ====
        prog_addr_inputs = []
        for i in range(8):
            elem_id = await self.create_element(
                "InputSwitch", base_x + (i * HORIZONTAL_GATE_SPACING), y_prog, f"ProgAddr[{i}]"
            )
            if elem_id is None:
                return False
            prog_addr_inputs.append(elem_id)

        # ProgData[10..15]'s two-digit index makes the label long enough to
        # reach into its neighbor at a standard 1x step on platforms that
        # render the label a bit wider than the default Linux font (observed
        # on Windows CI), so this row gets extra clearance.
        prog_data_col_spacing = HORIZONTAL_GATE_SPACING + 32
        y_prog_data = y_prog + VERTICAL_STAGE_SPACING
        prog_data_inputs = []
        for i in range(16):
            elem_id = await self.create_element(
                "InputSwitch", base_x + (i * prog_data_col_spacing), y_prog_data, f"ProgData[{i}]"
            )
            if elem_id is None:
                return False
            prog_data_inputs.append(elem_id)

        prog_write_id = await self.create_element(
            "InputSwitch", base_x + (16 * HORIZONTAL_GATE_SPACING), y_prog, "ProgWrite"
        )
        if prog_write_id is None:
            return False

        await self.log("  ✓ Created instruction memory programming inputs")

        y_regprog = y_prog_data + VERTICAL_STAGE_SPACING
        reg_prog_addr_inputs = []
        for i in range(5):
            elem_id = await self.create_element(
                "InputSwitch", base_x + (i * 1.5 * HORIZONTAL_GATE_SPACING), y_regprog, f"RegProgAddr[{i}]"
            )
            if elem_id is None:
                return False
            reg_prog_addr_inputs.append(elem_id)

        y_regprog_data = y_regprog + VERTICAL_STAGE_SPACING
        reg_prog_data_inputs = []
        for i in range(16):
            elem_id = await self.create_element(
                "InputSwitch", base_x + (i * 1.5 * HORIZONTAL_GATE_SPACING), y_regprog_data, f"RegProgData[{i}]"
            )
            if elem_id is None:
                return False
            reg_prog_data_inputs.append(elem_id)

        reg_prog_write_id = await self.create_element(
            "InputSwitch", base_x + (16 * 1.5 * HORIZONTAL_GATE_SPACING), y_regprog, "RegProgWrite"
        )
        if reg_prog_write_id is None:
            return False

        await self.log("  ✓ Created register file programming inputs")

        y_output = y_regprog_data + VERTICAL_STAGE_SPACING

        # ================= Connections ================= #

        # ---- Fetch stage: Clock/Reset/control signals (unchanged from the
        # prior revision — PCInc/InstrLoad stay tied to Vcc; the PC-drift-
        # during-programming behavior this implies is handled by holding
        # Reset throughout programming, see the docstring above) ----
        if not await self.connect(clock_id, fetch_id, target_port_label="Clock"):
            return False
        if not await self.connect(reset_id, fetch_id, target_port_label="Reset"):
            return False
        if not await self.connect(vcc_id, fetch_id, target_port_label="PCInc"):
            return False
        if not await self.connect(vcc_id, fetch_id, target_port_label="InstrLoad"):
            return False
        if not await self.connect(gnd_id, fetch_id, target_port_label="PCLoad"):
            return False
        for i in range(8):
            if not await self.connect(gnd_id, fetch_id, target_port_label=f"PCData[{i}]"):
                return False

        # Instruction-memory programming pass-through
        for i in range(8):
            if not await self.connect(prog_addr_inputs[i], fetch_id, target_port_label=f"ProgAddr[{i}]"):
                return False
        for i in range(16):
            if not await self.connect(prog_data_inputs[i], fetch_id, target_port_label=f"ProgData[{i}]"):
                return False
        if not await self.connect(prog_write_id, fetch_id, target_port_label="ProgWrite"):
            return False

        await self.log("  ✓ Instantiated and wired 16-bit Fetch stage")

        # ---- Fetch -> Decode: live OpCode, tapped from RawInstr (bits
        # [15:11]) to stay zero-delay/combinational, matching the ALU's own
        # RawInstr-based operands rather than the one-cycle-stale registered
        # OpCode output (which stays exposed as a CPU output further down) ----
        for i in range(5):
            if not await self.connect(
                fetch_id, decode_id, source_port_label=f"RawInstr[{11 + i}]", target_port_label=f"OpCode[{i}]"
            ):
                return False

        await self.log("  ✓ Connected Fetch to Decode (via RawInstr)")

        # ---- Decode -> ALU: ALUOp pass-through ----
        for i in range(3):
            if not await self.connect(
                decode_id, alu_id, source_port_label=f"ALUOp[{i}]", target_port_label=f"ALUOp[{i}]"
            ):
                return False

        await self.log("  ✓ Connected Decode to ALU")

        # ---- Register file: Read_Addr1 = DestReg (RawInstr[6..10]) ----
        for i in range(5):
            if not await self.connect(
                fetch_id, regfile_id, source_port_label=f"RawInstr[{6 + i}]", target_port_label=f"Read_Addr1[{i}]"
            ):
                return False

        await self.log("  ✓ Wired DestReg to register file read port")

        # ---- ALU operands (F26, F53): OperandA = zero-extended SrcBits
        # (RawInstr[0..5]); OperandB = Reg[DestReg] (full 16 bits, the
        # register file's read port) ----
        for i in range(6):
            if not await self.connect(
                fetch_id, alu_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"OperandA[{i}]"
            ):
                return False
        # F34 explicit-tie-off convention: the high 10 bits of the zero-extension
        # aren't a natural side effect of leaving them unconnected.
        for i in range(6, 16):
            if not await self.connect(gnd_id, alu_id, target_port_label=f"OperandA[{i}]"):
                return False
        for i in range(16):
            if not await self.connect(
                regfile_id, alu_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"OperandB[{i}]"
            ):
                return False

        await self.log("  ✓ Wired RawInstr SrcBits and Reg[DestReg] to ALU operands")

        # ---- Memory stage: Address = zero-extended SrcBits[0..2] (aliased
        # modulo 8, F34 explicit-tie-off convention on the unused high bits),
        # DataIn = Reg[DestReg] (Store writes the register back to memory),
        # Result passthrough = ALU Result, MemRead/MemWrite from Decode ----
        for i in range(3):
            if not await self.connect(
                fetch_id, memstage_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"Address[{i}]"
            ):
                return False
        for i in range(3, 8):
            if not await self.connect(gnd_id, memstage_id, target_port_label=f"Address[{i}]"):
                return False
        for i in range(16):
            if not await self.connect(
                regfile_id, memstage_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"DataIn[{i}]"
            ):
                return False
        for i in range(16):
            if not await self.connect(
                alu_id, memstage_id, source_port_label=f"Result[{i}]", target_port_label=f"Result[{i}]"
            ):
                return False
        if not await self.connect(decode_id, memstage_id, source_port_label="MemRead", target_port_label="MemRead"):
            return False
        if not await self.connect(decode_id, memstage_id, source_port_label="MemWrite", target_port_label="MemWrite"):
            return False
        if not await self.connect(clock_id, memstage_id, target_port_label="Clock"):
            return False
        if not await self.connect(reset_id, memstage_id, target_port_label="Reset"):
            return False

        await self.log("  ✓ Instantiated and wired 16-bit Memory stage")

        # ---- Register write-back: Data_In = memory stage DataOut directly
        # (already muxed between load data and ALU result-passthrough inside
        # the memory stage — no separate top-level write-back mux needed,
        # mirroring the 8-bit CPUs' own write_data_mux <- memory DataOut) ----
        for i in range(8):
            if not await self.connect(
                memstage_id, write_data_mux_low_id, source_port_label=f"DataOut[{i}]", target_port_label=f"In0[{i}]"
            ):
                return False
            if not await self.connect(
                memstage_id,
                write_data_mux_high_id,
                source_port_label=f"DataOut[{8 + i}]",
                target_port_label=f"In0[{i}]",
            ):
                return False
            if not await self.connect(reg_prog_data_inputs[i], write_data_mux_low_id, target_port_label=f"In1[{i}]"):
                return False
            if not await self.connect(
                reg_prog_data_inputs[8 + i], write_data_mux_high_id, target_port_label=f"In1[{i}]"
            ):
                return False
        if not await self.connect(reg_prog_write_id, write_data_mux_low_id, target_port_label="Sel"):
            return False
        if not await self.connect(reg_prog_write_id, write_data_mux_high_id, target_port_label="Sel"):
            return False
        for i in range(8):
            if not await self.connect(
                write_data_mux_low_id, regfile_id, source_port_label=f"Out[{i}]", target_port_label=f"Data_In[{i}]"
            ):
                return False
            if not await self.connect(
                write_data_mux_high_id,
                regfile_id,
                source_port_label=f"Out[{i}]",
                target_port_label=f"Data_In[{8 + i}]",
            ):
                return False

        await self.log("  ✓ Connected memory stage write-back to register file")

        # Write address mux: In0 = DestReg (normal — write back to the same
        # register the instruction read), In1 = RegProgAddr, Sel = RegProgWrite
        for i in range(5):
            if not await self.connect(
                fetch_id, write_addr_mux_ids[i], source_port_label=f"RawInstr[{6 + i}]", target_port=0
            ):
                return False
            if not await self.connect(reg_prog_addr_inputs[i], write_addr_mux_ids[i], target_port=1):
                return False
            if not await self.connect(reg_prog_write_id, write_addr_mux_ids[i], target_port=2):
                return False
            if not await self.connect(write_addr_mux_ids[i], regfile_id, target_port_label=f"Write_Addr[{i}]"):
                return False

        # Write enable: (NOT(MemWrite) AND NOT(ProgWrite) AND NOT(Reset)) OR RegProgWrite
        if not await self.connect(prog_write_id, prog_not_id):
            return False
        if not await self.connect(reset_id, reset_not_id):
            return False
        if not await self.connect(decode_id, load_regwrite_not_id, source_port_label="MemWrite"):
            return False
        if not await self.connect(load_regwrite_not_id, we_and1_id, target_port=0):
            return False
        if not await self.connect(prog_not_id, we_and1_id, target_port=1):
            return False
        if not await self.connect(we_and1_id, we_and2_id, target_port=0):
            return False
        if not await self.connect(reset_not_id, we_and2_id, target_port=1):
            return False
        if not await self.connect(we_and2_id, we_or_id, target_port=0):
            return False
        if not await self.connect(reg_prog_write_id, we_or_id, target_port=1):
            return False
        if not await self.connect(we_or_id, regfile_id, target_port_label="WriteEnable"):
            return False
        if not await self.connect(clock_id, regfile_id, target_port_label="Clock"):
            return False

        await self.log("  ✓ Wired register file write-back with programming mux")

        # ---- Create output LED columns ----
        output_x = memstage_x + max(HORIZONTAL_GATE_SPACING * 2, memstage_handle.width + HORIZONTAL_GATE_SPACING)
        pc_x = output_x
        result_x = pc_x + HORIZONTAL_GATE_SPACING
        instr_x = result_x + HORIZONTAL_GATE_SPACING
        opcode_x = instr_x + 1.5 * HORIZONTAL_GATE_SPACING

        # ---- Create Output: PC ----
        for i in range(8):
            led_id = await self.create_element("Led", pc_x, y_output + (i * VERTICAL_STAGE_SPACING), f"PC[{i}]")
            if led_id is None:
                return False
            if not await self.connect(fetch_id, led_id, source_port_label=f"PC[{i}]"):
                return False

        await self.log("  ✓ Created PC outputs")

        # ---- Create Output: 16-bit Result (memory-stage DataOut) ----
        for i in range(16):
            led_id = await self.create_element("Led", result_x, y_output + (i * VERTICAL_STAGE_SPACING), f"Result[{i}]")
            if led_id is None:
                return False
            if not await self.connect(memstage_id, led_id, source_port_label=f"DataOut[{i}]"):
                return False

        await self.log("  ✓ Created 16-bit Result outputs")

        # ---- Create Output: 16-bit Instruction (registered) ----
        for i in range(16):
            led_id = await self.create_element(
                "Led", instr_x, y_output + (i * VERTICAL_STAGE_SPACING), f"Instruction[{i}]"
            )
            if led_id is None:
                return False
            if not await self.connect(fetch_id, led_id, source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Created Instruction outputs")

        # ---- Create Output: OpCode (registered) ----
        for i in range(5):
            led_id = await self.create_element("Led", opcode_x, y_output + (i * VERTICAL_STAGE_SPACING), f"OpCode[{i}]")
            if led_id is None:
                return False
            if not await self.connect(fetch_id, led_id, source_port_label=f"OpCode[{i}]"):
                return False

        await self.log("  ✓ Created OpCode outputs")

        output_file = str(IC_COMPONENTS_DIR / "level9_cpu_16bit_risc.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 16-bit RISC CPU IC "
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log("   Architecture: 16-bit Fetch + Decode + ALU + Memory + 32x16 RegFile")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = CPU16BitRISCBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "16-bit RISC CPU"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
