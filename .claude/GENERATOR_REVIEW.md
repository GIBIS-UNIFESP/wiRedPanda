# Generator Review — 4-State Simulation Compatibility

Tracking review of all Python IC generators against the new 4-state logic
and event-driven simulation engine (commit `6926d4216`).

**Review completed 2026-03-17. All 62 unmodified generators reviewed.**

## Key Simulation Changes Checked Against

1. **4-state logic**: Invalid/Inactive/Active/Error — unconnected inputs are `Invalid`, not `Inactive`
2. **Async preset/clear**: triggers on `== Inactive`, not `!= Active` — `Invalid` no longer activates preset/clear
3. **Event-driven FIFO**: blocking propagation, no topological sort
4. **GND/VCC are graph elements**: shared across IC nesting levels
5. **Flip-flop semantics**: read current inputs at clock edge (no `m_lastValue`)
6. **StatusOps**: AND (Inactive dominates), OR (Active dominates), NOT (preserves unknown), XOR (requires all known)

## Key Finding: IC Input Ports vs Primitive Gate Ports

Two kinds of "unconnected" ports behave differently under 4-state:

- **IC input ports** (InputSwitch inside sub-IC): retain their default value (0/Inactive) when
  not connected from the parent. The InputSwitch is a valid source element. **Not affected by 4-state.**
- **Primitive gate input ports** (e.g., DFlipFlop ~Preset/~Clear): become `Invalid` when unconnected.
  Under the new `== Inactive` check, `Invalid` won't trigger preset/clear. **Functionally safe but not robust.**

This distinction means unconnected IC ports (like adder B[1-7] in program counters)
are NOT bugs — they default to 0 via their internal InputSwitch.

## Already Updated (in commit 6926d4216)

- [x] `create_level1_d_flip_flop` — rewritten to textbook master-slave
- [x] `create_level1_jk_flip_flop` — rewritten to textbook master-slave
- [x] `create_level4_binary_counter_4bit` — RST pin to DFF Clear
- [x] `create_level5_modulo_counter_4bit` — RST pin to DFF Clear
- [x] `create_level5_up_down_counter_4bit` — RST pin to DFF Clear
- [x] `create_level9_multi_cycle_cpu_8bit` — inverted counter clock

---

## Review Results

### STYLE — Wire Vcc to Primitive DFlipFlop Async Ports

These 5 generators use **primitive DFlipFlop** elements with `~Preset` and/or `~Clear` left
unconnected. Under 4-state, `Invalid != Inactive` so preset/clear won't false-trigger —
FFs work correctly. However, wiring Vcc to unused async ports is best practice and
consistent with the pattern used in updated counters and other generators.

| Generator | FFs | Issue |
|-----------|-----|-------|
| `level4_ram_4x1` | 4 | ~Preset and ~Clear both unconnected |
| `level4_ram_8x1` | 8 | ~Preset and ~Clear both unconnected |
| `level4_register_4bit` | 4 | ~Preset and ~Clear both unconnected ("No reset for simplicity") |
| `level5_register_file_4x4` | 16 | ~Preset and ~Clear both unconnected |
| `level5_register_file_8x8` | 64 | ~Preset and ~Clear both unconnected |

### DESIGN NOTES (not 4-state bugs, but worth noting)

- `level5_program_counter_4bit`: `reset` input created at line 69 but never wired — dead port
- `level7_cpu_program_counter_8bit`: `WriteEnable` input not connected — documented as intentional
- `level6_stack_pointer_8bit`: Push/Pop adder B inputs always 0 — not fully implemented

### OK — All Remaining Generators (reviewed, no 4-state issues)

#### Level 1 — Latches
- [x] `level1_d_latch` — purely combinational (NOR gates) ✓
- [x] `level1_sr_latch` — purely combinational (NOR gates) ✓

#### Level 2 — Combinational (12 generators, no FFs)
- [x] All level 2 generators — purely combinational ✓

#### Level 3 — Mixed
- [x] `level3_register_1bit` — primitive DFF, Vcc → ~Preset, NOT(Reset) → ~Clear ✓
- [x] `level3_alu_selector_5way` — combinational ✓
- [x] `level3_bcd_7segment_decoder` — combinational ✓
- [x] `level3_comparator_4bit` — combinational ✓
- [x] `level3_comparator_4bit_equality` — combinational ✓

#### Level 4 — Counters, shift registers, etc.
- [x] `level4_bus_mux_4bit` — combinational ✓
- [x] `level4_bus_mux_8bit` — combinational ✓
- [x] `level4_comparator_4bit` — combinational ✓
- [x] `level4_johnson_counter_4bit` — level1 DFF ICs, Preset/Clear properly wired ✓
- [x] `level4_ring_counter_4bit` — level1 DFF ICs, Preset/Clear properly wired ✓
- [x] `level4_ripple_adder_4bit` — combinational ✓
- [x] `level4_ripple_alu_4bit` — combinational ✓
- [x] `level4_shift_register_piso` — level1 DFF ICs, Vcc → Preset/Clear. Unconnected In1[3] on shift BusMux is an IC port (defaults to 0). ✓
- [x] `level4_shift_register_sipo` — level1 DFF ICs, Vcc → Preset/Clear ✓

#### Level 5 — Complex sequential
- [x] `level5_barrel_rotator` — combinational ✓
- [x] `level5_barrel_shifter_4bit` — combinational ✓
- [x] `level5_clock_gated_decoder` — combinational (decoder + AND gates) ✓
- [x] `level5_controller_4bit` — combinational ✓
- [x] `level5_instruction_decoder_4bit` — combinational ✓
- [x] `level5_loadable_counter_4bit` — level1 DFF ICs, Vcc → Preset/Clear ✓
- [x] `level5_program_counter_4bit` — Adder B[1-3] unconnected = IC ports (default 0) ✓

#### Level 6 — Subsystems
- [x] `level6_alu_8bit` — combinational (ripple_alu + XOR/NOT/mux). GND → CarryIn ✓
- [x] `level6_program_counter_8bit_arithmetic` — B[1-7] unconnected = IC ports (default 0). Reset connected ✓
- [x] `level6_ram_256x8` — composes level4_ram_8x1 ICs (inherits style issue) ✓
- [x] `level6_register_8bit` — composes level3_register_1bit ICs (properly wired) ✓
- [x] `level6_register_file_8x8` — primitive DFF, Vcc → ~Preset and ~Clear ✓
- [x] `level6_ripple_adder_8bit` — combinational ✓
- [x] `level6_stack_memory_interface` — composes stack_pointer + RAM + mux ICs ✓
- [x] `level6_stack_pointer_8bit` — level1 DFF ICs, Vcc → Preset/Clear ✓

#### Level 7 — CPU building blocks
- [x] `level7_alu_16bit` — composes two 8-bit ALUs ✓
- [x] `level7_cpu_program_counter_8bit` — wraps level6 PC ✓
- [x] `level7_data_forwarding_unit` — combinational (4-to-1 mux array) ✓
- [x] `level7_execution_datapath` — combinational (ALU + flag logic) ✓
- [x] `level7_flag_register` — level1 DFF ICs, Vcc → Preset/Clear ✓
- [x] `level7_instruction_decoder_8bit` — combinational (decoders + AND) ✓
- [x] `level7_instruction_memory_interface` — wraps RAM IC, GND → WriteEnable ✓
- [x] `level7_instruction_register_8bit` — wraps level6_register_8bit IC ✓

#### Level 8 — Pipeline stages
- [x] `level8_decode_stage` — combinational (control logic) ✓
- [x] `level8_execute_stage` — composes ALU + datapath ICs ✓
- [x] `level8_fetch_stage` — composes PC + memory + instr register ICs ✓
- [x] `level8_memory_stage` — composes RAM + control ICs ✓

#### Level 9 — Full CPUs
- [x] `level9_cpu_16bit_risc` — composes level 8 stage ICs ✓
- [x] `level9_fetch_stage_16bit` — composes level 7 ICs ✓
- [x] `level9_single_cycle_cpu_8bit` — composes level 7 ICs ✓
