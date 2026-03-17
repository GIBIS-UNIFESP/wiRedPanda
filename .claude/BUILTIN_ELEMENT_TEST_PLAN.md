# Built-in Element Test Plan

## Current Coverage

| Area | Status | Detail |
|------|--------|--------|
| Combinational truth tables | Strong | All gates, multi-input variants, exhaustive |
| Basic flip-flop state transitions | OK | Single FF, set/reset/toggle/hold |
| 4-state domination rules | **Missing** | No tests for Unknown/Error through gates |
| Multi-FF cascade (same clock) | **Missing** | The shift register regression had zero coverage |
| Async preset/clear with Unknown | **Missing** | Only tested with Inactive/Active inputs |
| Error state propagation | **Missing** | Status::Error never tested |

## Phase 1: 4-state domination rules (combinational)

Test each gate with Unknown and Error inputs to verify domination.

| Test | What it verifies |
|------|-----------------|
| `testAndDomination` | AND(Inactive, Unknown) = Inactive; AND(Active, Unknown) = Unknown; AND(Active, Error) = Error |
| `testOrDomination` | OR(Active, Unknown) = Active; OR(Inactive, Unknown) = Unknown; OR(Inactive, Error) = Error |
| `testNotUnknownError` | NOT(Unknown) = Unknown; NOT(Error) = Error |
| `testNandDomination` | NAND(Inactive, Unknown) = Active; NAND(Active, Unknown) = Unknown |
| `testNorDomination` | NOR(Active, Unknown) = Inactive; NOR(Inactive, Unknown) = Unknown |
| `testXorUnknown` | XOR(any, Unknown) = Unknown; XOR(any, Error) = Error |
| `testXnorUnknown` | Same as XOR |
| `testMuxUnknownSelect` | Mux with Unknown select → Unknown output |
| `testDemuxUnknownSelect` | Demux with Unknown select → all outputs Unknown |
| `testNodeUnknown` | Node passes Unknown/Error through unchanged |

## Phase 2: Multi-FF cascade (same clock)

Priority: **highest** — catches the shift register regression.

| Test | What it verifies |
|------|-----------------|
| `testDFlipFlopShiftRegister` | 4 DFFs chained, same clock — data shifts one stage per cycle, not all at once |
| `testJKFlipFlopRippleCounter` | 2 JK FFs in toggle mode, Q0→CLK1 — counts 0,1,2,3 |
| `testTFlipFlopDivideByN` | 3 T FFs chained — frequency division 1:2:4:8 |
| `testSRFlipFlopPipeline` | 2 SR FFs, same clock — S propagates one stage per cycle |

## Phase 3: Async preset/clear edge cases

| Test | What it verifies |
|------|-----------------|
| `testPresetClearWithUnknown` | Preset/Clear at Unknown doesn't falsely trigger |
| `testPresetClearWithError` | Preset/Clear at Error doesn't falsely trigger |
| `testPresetOverridesClock` | Preset asserted during clock edge → preset wins |
| `testClearOverridesClock` | Clear asserted during clock edge → clear wins |
| `testBothPresetAndClear` | Both asserted → forbidden state (Q=1, Q'=1) |
| `testPresetReleaseThenClock` | Release preset, next clock edge latches D correctly |

## Phase 4: Error state propagation

| Test | What it verifies |
|------|-----------------|
| `testErrorPropagatesThroughChain` | Error on input → Error on output through AND→NOT→OR chain |
| `testErrorDominatesUnknown` | AND(Error, Unknown) = Error; OR(Error, Unknown) = Error |
| `testFlipFlopWithErrorInput` | D=Error on clock edge → Q=Error |

## Priority Order

1. **Phase 2** — catches the regression we just fixed (multi-FF cascade)
2. **Phase 1** — validates StatusOps domination rules
3. **Phase 3** — preset/clear edge cases with 4-state
4. **Phase 4** — Error state (lowest priority, no known bugs)
