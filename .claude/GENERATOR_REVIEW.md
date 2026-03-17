# Generator Review — 4-State Simulation Compatibility

Tracking review of all Python IC generators against the new 4-state logic
and event-driven simulation engine (commit `6926d4216`).

## Key Simulation Changes to Check Against

1. **4-state logic**: Invalid/Inactive/Active/Error — unconnected inputs are `Invalid`, not `Inactive`
2. **Async preset/clear**: triggers on `== Inactive`, not `!= Active` — `Invalid` no longer activates preset/clear
3. **Event-driven FIFO**: blocking propagation, no topological sort
4. **GND/VCC are graph elements**: shared across IC nesting levels
5. **Flip-flop semantics**: read current inputs at clock edge (no `m_lastValue`)
6. **StatusOps**: AND (Inactive dominates), OR (Active dominates), NOT (preserves unknown), XOR (requires all known)

## Already Updated (in commit 6926d4216)

- [x] `create_level1_d_flip_flop` — rewritten to textbook master-slave
- [x] `create_level1_jk_flip_flop` — rewritten to textbook master-slave
- [x] `create_level4_binary_counter_4bit` — RST pin to DFF Clear
- [x] `create_level5_modulo_counter_4bit` — RST pin to DFF Clear
- [x] `create_level5_up_down_counter_4bit` — RST pin to DFF Clear
- [x] `create_level9_multi_cycle_cpu_8bit` — inverted counter clock

---

## High-Priority Review Results

### BUGS (must fix — broken under 4-state logic)

#### BUG 1: `level5_program_counter_4bit` — unconnected adder inputs
- **File**: `create_level5_program_counter_4bit.py:175-181`
- **Problem**: Only `B[0]` connected to Vcc. `B[1-3]` and `CarryIn` left unconnected.
- **Impact**: Under 4-state, unconnected = `Invalid`. XOR (in full adder) requires all known inputs → `Sum[0-3]` all become `Invalid`. **PC+1 computation completely broken.**
- **Fix**: Connect `B[1-3]` to InputGnd and `CarryIn` to InputGnd.
- **Also**: `reset` input (line 69) created but never wired — dead port.

#### BUG 2: `level4_shift_register_piso` — unconnected shift-in for MSB
- **File**: `create_level4_shift_register_piso.py:188-191`
- **Problem**: `In1[3]` of the shift BusMux is never connected (loop only covers `i < 3`).
- **Impact**: During shift mode, MSB (FF3) receives `Invalid` instead of 0. After multiple shifts, Invalid cascades through all flip-flops.
- **Fix**: Connect `gnd_id` to `shift_mux_ic_id` port `In1[3]` (GND element already exists at line 152).

### STYLE (functionally OK, but should wire Vcc for robustness)

These generators use **primitive DFlipFlop** elements with `~Preset` and/or `~Clear` left unconnected. Under 4-state logic, `Invalid != Inactive` so preset/clear won't trigger — FFs work normally. However, connecting Vcc to unused async ports is best practice (consistent with updated counters).

#### `level4_ram_4x1` — 4 DFlipFlops, ~Preset and ~Clear both unconnected
- **File**: `create_level4_ram_4x1.py`
- No reset needed for RAM. FFs start at Q=0 (constructor default). Mux feedback holds.
- **Recommendation**: Add Vcc → ~Preset and Vcc → ~Clear for all 4 FFs.

#### `level4_ram_8x1` — 8 DFlipFlops, ~Preset and ~Clear both unconnected
- **File**: `create_level4_ram_8x1.py`
- Same analysis as RAM 4x1.
- **Recommendation**: Add Vcc → ~Preset and Vcc → ~Clear for all 8 FFs.

#### `level4_register_4bit` — 4 DFlipFlops, ~Preset and ~Clear both unconnected
- **File**: `create_level4_register_4bit.py`
- Comment at line 21: "No reset/clear control for simplicity"
- **Recommendation**: Add Vcc → ~Preset and Vcc → ~Clear for all 4 FFs.

#### `level5_register_file_4x4` — 16 DFlipFlops, ~Preset and ~Clear both unconnected
- **File**: `create_level5_register_file_4x4.py`
- **Recommendation**: Add Vcc → ~Preset and Vcc → ~Clear for all 16 FFs.

#### `level5_register_file_8x8` — 64 DFlipFlops, ~Preset and ~Clear both unconnected
- **File**: `create_level5_register_file_8x8.py`
- **Recommendation**: Add Vcc → ~Preset and Vcc → ~Clear for all 64 FFs.

### OK (reviewed, no issues)

- [x] `level3_register_1bit` — Vcc → ~Preset, NOT(Reset) → ~Clear. Both wired correctly. ✓
- [x] `level4_johnson_counter_4bit` — Uses level1 DFF ICs. PRESET → FF[0].Preset, PRESET → FF[1-3].Clear, Vcc → FF[1-3].Preset, Vcc → FF[0].Clear. All wired. ✓
- [x] `level4_ring_counter_4bit` — Same wiring pattern as Johnson. All wired. ✓
- [x] `level4_shift_register_sipo` — Uses level1 DFF ICs. Vcc → Preset and Clear on all 4 FFs. ✓
- [x] `level5_loadable_counter_4bit` — Uses level1 DFF ICs. Vcc → Preset and Clear on all 4 FFs. ✓
- [x] `level5_controller_4bit` — Purely combinational (OR gate + pass-through). No FFs. ✓
- [x] `level5_clock_gated_decoder` — Purely combinational (decoder + AND gates). No FFs. ✓

---

## Medium-Priority (not yet reviewed)

These use sub-ICs that contain flip-flops. Issues cascade from lower levels.

- [ ] `create_level6_alu_8bit`
- [ ] `create_level6_program_counter_8bit_arithmetic` — **uses level5_program_counter_4bit (BUG 1 cascades)**
- [ ] `create_level6_ram_256x8` — **likely uses level4_ram ICs (STYLE issues cascade)**
- [ ] `create_level6_register_8bit`
- [ ] `create_level6_register_file_8x8`
- [ ] `create_level6_ripple_adder_8bit`
- [ ] `create_level6_stack_memory_interface`
- [ ] `create_level6_stack_pointer_8bit`
- [ ] `create_level7_alu_16bit`
- [ ] `create_level7_cpu_program_counter_8bit` — **uses counters/PC: check cascade**
- [ ] `create_level7_data_forwarding_unit`
- [ ] `create_level7_execution_datapath`
- [ ] `create_level7_flag_register`
- [ ] `create_level7_instruction_decoder_8bit`
- [ ] `create_level7_instruction_memory_interface`
- [ ] `create_level7_instruction_register_8bit`
- [ ] `create_level8_decode_stage`
- [ ] `create_level8_execute_stage`
- [ ] `create_level8_fetch_stage`
- [ ] `create_level8_memory_stage`

## Low-Priority (not yet reviewed)

Purely combinational — unlikely to break under 4-state logic.

- [ ] `create_level1_d_latch`
- [ ] `create_level1_sr_latch`
- [ ] All `level2_*` generators (12 files)
- [ ] `create_level3_alu_selector_5way`
- [ ] `create_level3_bcd_7segment_decoder`
- [ ] `create_level3_comparator_4bit`
- [ ] `create_level3_comparator_4bit_equality`
- [ ] `create_level4_bus_mux_4bit` — purely combinational ✓
- [ ] `create_level4_bus_mux_8bit` — purely combinational ✓
- [ ] `create_level4_comparator_4bit`
- [ ] `create_level4_ripple_adder_4bit` — purely combinational ✓
- [ ] `create_level4_ripple_alu_4bit`
- [ ] `create_level9_cpu_16bit_risc`
- [ ] `create_level9_fetch_stage_16bit`
- [ ] `create_level9_single_cycle_cpu_8bit`
