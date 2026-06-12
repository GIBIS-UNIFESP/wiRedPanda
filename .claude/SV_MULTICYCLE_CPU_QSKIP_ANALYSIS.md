# SystemVerilog Export — Multi-Cycle CPU QSKIP: Root-Cause Analysis

**Date:** 2026-06-12
**Test:** `TestSystemVerilogExport::testSystemVerilogExportMultiCycleCpu8Bit`
(`Tests/Integration/TestSystemVerilogExport.cpp:1420`)
**Fixture:** `level9_multi_cycle_cpu_8bit.panda`

## TL;DR

The QSKIP is a **genuine cycle-based-vs-event-driven simulation-semantics
mismatch**, not a codegen bug. wiRedPanda's zero-delay engine gates a clock
with the cycle counter's **NEW** (same-tick) state; SystemVerilog's non-blocking
model uses the counter's **OLD** state — and **SV is the physically correct
side**. The only proper fix is an **engine-level non-blocking sequential
semantics** change (the "transition to event-driven" the QSKIP names). No
codegen-only fix is proper, because reproducing wiRedPanda's NEW-state behavior
in SV requires non-physical, non-synthesizable RTL. The engine change is a
major, high-risk core rewrite — out of scope for a codegen/cleanup pass — so
the QSKIP stands; this doc records the complete diagnosis.

## Diagnosis (with actual values)

Running the test against the real toolchain (`iverilog`/`vvp`, all present) and
the generated DUT/testbench:

- All 9 sequential check-steps fail. The **first** failure is at **step 7,
  "Initial state after reset" — before any clock edge** (`led5`: SV `1` vs
  wiRedPanda `0`), and every clocked step thereafter diverges.

- The CPU's state DFFs are driven by **gated clocks**. In the generated SV:

  ```systemverilog
  assign aux_and_12 = (clock & aux_not_11 & aux_not_10);
  assign aux_and_13 = (clock & aux_not_11 & aux_d_flip_flop_1_0_q);
  assign aux_and_14 = (clock & aux_d_flip_flop_2_0_q & aux_not_10);
  assign aux_and_15 = (clock & aux_d_flip_flop_2_0_q & aux_d_flip_flop_1_0_q);
  ```

  `clock` is the master clock; the gating term is the one-hot decode of the
  2-bit cycle counter (`aux_d_flip_flop_1/2`).

- **wiRedPanda** (`Simulation::update()` Phase 2, `App/Simulation/Simulation.cpp:90`)
  runs a **single-pass topological update** with immediate `setOutputValue`. The
  counter DFFs (producers) are evaluated before the gated-clock AND (consumer)
  in the same tick, so the gated clock sees the counter's **NEW** same-tick
  state.

- **SystemVerilog** uses non-blocking (`<=`) updates: the counter's new value is
  not visible until the next delta cycle, so the continuous-`assign` gated clock
  sees the counter's **OLD** state.

- This is exactly the QSKIP's stated cause — but the diagnosis adds two facts
  the original comment omits:
  1. The divergence also appears **at reset, before any edge** (undriven/unreset
     state is `X` in SV vs Inactive-`0` in wiRedPanda).
  2. **SV is the physically correct side.** In real hardware the counter's new
     value appears *after* the edge (propagation delay), so the gated clock for
     that edge uses the old value — like SV. wiRedPanda's zero-delay NEW-state
     gating is non-physical. The CPU generator itself documents the gated clock
     as "a textbook clock-gating hazard in real hardware but deterministic in
     this zero-delay simulator."

## The most proper fix

**Give the simulation engine proper non-blocking (synchronous) sequential
semantics**: on a tick, sequential state elements sample their inputs from the
*pre-tick* state of other elements and commit their new outputs only after the
whole update pass. Then any logic in the same tick (gated clocks, downstream
flip-flops) sees the OLD state — matching real synchronous hardware and SV's
non-blocking model. The SV export would then be faithful **by construction**.

### Why no lesser fix is proper

- A **codegen** fix to make SV match wiRedPanda's NEW-state behavior would
  require non-physical, non-synthesizable RTL (gating the current edge with the
  counter's *post-edge* value) — the opposite of proper.
- The F63 settle-loop / boundary-ordering fixes do not apply here: SV gate logic
  is order-independent continuous `assign`, and this divergence is **sequential
  timing**, not combinational settling.

## Why it is deferred (scope and risk)

This is an **engine-core semantics change**, not golden regeneration ("churn"):

- Phase 2's immediate-`setOutputValue` single pass is load-bearing. A two-phase
  (compute-then-commit) sequential update changes inter-element timing for
  *every* circuit where a sequential element feeds another sequential element
  (or its clock) within a tick — counters, shift registers, register files,
  RAMs, and all CPUs.
- The deep-review **F1** fix deliberately preserved the legacy intra-SCC settle
  order *because the gate-built latch ICs depend on it*. Changing sequential
  commit semantics risks destabilizing exactly that, plus dozens of sequential
  test expectations and goldens, with a real chance of subtle breakage.
- It is the documented "transition to event-driven" — a dedicated, carefully
  staged engine project, disproportionately risky against a cleanup pass.

## Recommendation

Keep the QSKIP. The disposition is correct; this document supersedes the
shorter in-code note with the complete root cause (gated-clock NEW-vs-OLD state
**and** reset `X`-init, plus the fact that SV is the physically correct side).

If the engine change is commissioned as its own effort, the path is: implement a
two-phase (compute-then-commit) sequential update in `Simulation::update()` and
the flip-flop elements; regenerate all sequential goldens; run full `ctest`; and
triage the fallout — stopping to reassess if it destabilizes the latch ICs
rather than forcing it. Success criterion: un-QSKIP and the multi-cycle CPU SV
testbench reaches `PASS: N steps` with `ctest` green.
