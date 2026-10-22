# Ledger: plan 6, the simulation core

Tracks the items of `.claude/PLAN_06_core.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P6.6a`, `P6.6b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** `06-core`, from `05-semantics` once plan 5 has landed; not created yet.

**Open decisions:** none yet; P6.31 needs the user once its pictures are drawn, and P6.30 if a single instant needs a
lower round budget.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P6.1 | Status in one byte (§2.1) | todo | | A test checks the size; plan 1's payload goldens show no byte of a file moves |
| P6.2 | Each logic type's delay in its descriptor (§2.2) | todo | | A test reads all 16 types' delays and that the others have none; shown to fail by a changed value |
| P6.3 | The netlist builder and its tests, the first landing (§2.3) | todo | | Its nodes compared with plan 5's reference on random circuits and the IC fixtures; shown to fail by a positional order, a duplicated label read from one Tx, a nested change not recompiled, a failed build that changes the netlist, a gate fed through a junction taking the open-Unknown shortcut |
| P6.4 | The kernels, checked as plan 5 checks the reference's and against them (§2.4) | todo | | Shown to fail: a gate ranking Error over Unknown, a kernel reading a non-level one way, a memory kind storing one order's result; the memory test moves one switch a tick |
| P6.5 | The core: rounds, memory, loops, pulses and divergence, trips and the budget (§2.5) | todo | | Plan 5's hand-written cases in lockstep, each shown to fail with its rule undone; the repeated state without its pulses runs a latch passing its own pulse to the budget (as on the branch) |
| P6.6 | The core: delays and ties, clocks, ticks and the deadline (§2.5) | todo | | Shown to fail: an edge at exactly a half period; a tie merge that wakes readers without undoing the pass (as on the branch); a temporal start with no change due that runs an instant (as on the branch); a clock owed two edges in one tick that runs one |
| P6.7 | Snapshots, the sweep guard and settle (§2.6) | todo | | Shown to fail: a snapshot restored onto another build, fatal in every build (as on the branch); a seed back to the published level left live under a zero budget (as on the branch); a settle that seeds a clock |
| P6.8 | Rebuilds and the mode switch (§2.7) | todo | | Shown to fail: the shrunk gate's edge in temporal mode (as on the branch) and its functional twin, new; an IC reload that powers its internals on (as on the branch); a mode switch that seeds |
| P6.9 | The lockstep, IC internals by place, and plan 5's hand-written cases on it (§2.9) | todo | | Shown to fail: two nodes inside one IC swapping values, which the branch's multiset missed; a dropped trip or warning |
| P6.10 | Plan 5's oracles and walks on the core (§2.9) | todo | | Shown to fail: the pulse model's correction undone in the core alone |
| P6.11 | The fuzzer (§2.9) | todo | | Registered in both fuzzer presets, the unattended-run script and the coverage report, with its corpus directory; its seed checked at start-up; shown to fail by a planted difference |
| P6.12 | Every fixture, example and IC component at power-on and clocked, nothing tripping unless meant to (§2.9) | todo | | Shown to fail: a free-running ring without its enable trips; each circuit meant to trip is named in the test |
| P6.13 | The review of `temporal-engine`, its core-level cases as regression tests (§2.9) | todo | | Each shown to fail with its defect put back: clocks on wall time in temporal mode, publications applied one at a time, sampling before a trip's cone settled; sampling timed at 8,000 flip-flops on one clock |
| P6.14 | Plan 5's larger loops timed on the core; the work limit set, in the spec first (§2.8) | todo | | Closes plan 5's P5.24 |
| P6.15 | The reset source, whole (§2.10) | todo | | Shown to fail: a settle that skips a release falling due (as on the branch); an element golden without its row (as on the branch); the MCP client's type-coverage test without the type |
| P6.16 | The suspend count in the model (§2.12) | todo | | Shown to fail: a model file that includes the runner's header fails the include check; blockers nest |
| P6.17 | The runner (§2.11) | todo | | Its headless tests with a manual clock, each shown to fail by its break: a pause that loses phase, a publication of unchanged slots, a timer that wakes on an idle circuit or misses a due edge, a user change stamped late |
| P6.18 | The switch: callers, blockers, the sweep, ports and outputs from the runner, the audio (§2.13) | todo | | The callers counted again on `master` when the switch starts; every caller compiled against the runner |
| P6.19 | The old engine deleted, its tests rewritten or deleted by name; the tests moved onto the core (§2.13) | todo | | Each deleted test named in its commit; each changed expectation named by its cause |
| P6.20 | Every place a status is read as a level, decided (§2.13) | todo | | The search run again; each place's test reads a non-level through plan 1's readers and fails on a level for Unknown |
| P6.21 | `master`'s behaviours kept or dropped by name (§2.13) | todo | | Each kept behaviour has a test; the idle test fails on a timer that keeps waking |
| P6.22 | The goldens: the spec's traces, every changed golden named (§2.13) | todo | | Each changed golden named by plan 5's list |
| P6.23 | What the exporters read, kept until plan 8 (§2.13) | todo | | The exporters' goldens unchanged by the switch |
| P6.24 | The lock tooltip's sentence on unlocking (§2.13) | todo | | A test reads the tooltip |
| P6.25 | The counters' reset input, their embedders regenerated; the JK's reset helper (§2.14) | todo | | Every embedder found in the generators; the counters' suites run on the core |
| P6.26 | The 4-bit display counter example (§2.14) | todo | | Its walk reaches a defined count |
| P6.27 | `CLAUDE.md` and the developer guide on the core and the runner (§2.14) | todo | | |
| P6.28 | The measurement after the plan (§2.16) | todo | | Tick, rebuild, status gather and memory on the named fixtures, both modes, against `master`'s engine; sampling linear in the flip-flops on one clock |
| P6.29 | Whether an IC's first reload in a scene always brings another definition (§6) | todo | | With plan 4's P4.9: what the core's node ranges do across it |
| P6.30 | A tick's bound: one deadline for both modes (P6.D11); the worst single instants timed (§6) | todo | | Shown to fail: a functional tick on the ring of latches with pulsing inputs that runs past its deadline; a running time that runs ahead of a core behind it (the backlog grows); a lower round budget, if one instant stalls, goes into the spec first, and to the user |
| P6.31 | What the user sees new: the reset source's picture and icon, a display's and an LED's non-level (§6) | todo | | Shown to the user before their slices are finished |
| P6.32 | No release between this plan's switch and plan 8's landing, plan 7 landing in between (§6) | todo | | Closed when plan 8 lands |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-21: P6.30's bound decided by the user ("go ahead"): one deadline for both modes, P6.D11; the row keeps the
  timing of the worst single instants.
