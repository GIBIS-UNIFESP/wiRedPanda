# Ledger: plan 5, simulation semantics and its oracles

Tracks the items of `.claude/PLAN_05_semantics.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P5.6a`, `P5.6b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** its own, from `master`, once plan 4 has landed; not created yet.

**Open decisions:** none (P5.25 decided on 2026-10-21: the spec takes the covering rule, P5.D21).

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P5.1 | The trace walks in the test library behind an engine; plan 1's goldens unchanged (§4.1) | todo | | Shown to fail: an engine that drops a step's input change fails plan 1's goldens |
| P5.2 | The spec, a section of the developer guide, written before the reference (§4.2) | todo | | Each rule re-examined as the oracles land; a change goes into the spec first, then the oracles and the reference |
| P5.3 | The random circuits and their walks (§4.3) | todo | | The walks assert that trips, cut-short settles, edits, mode switches, warnings and the budget each happen; shown to fail by a generator that drops the late wiring or the chain past the budget |
| P5.4 | The reference: the circuit read in one place, ICs flattened, aliases, wireless per scope, two drivers, IC boundaries, kernels (§4.4) | todo | | The wiring tests; shown to fail by an alias that costs a round, a duplicated label read from its lowest-id Tx, a clock pin read from its element |
| P5.5 | The kernels checked three ways: against `master` on levels, against the rule on non-levels, the flat-order sweep (§4.5) | todo | | Shown to fail: a gate ranking Error over Unknown, a kernel reading a non-level one way, the old combiner on stored state (as on the branch) |
| P5.6 | The reference: rounds by cause, memory over every order, loops, pulses and divergent Unknowns, trips, the budget (§4.4) | todo | | The hand-written cases, each shown to fail with its rule undone; the repeated state without its pulses runs a latch passing its own pulse to the budget (as on the branch) |
| P5.7 | The reference: clocks, ticks, delays and ties, settles, copies (§4.4) | todo | | Shown to fail: an edge at exactly a half period; a tie merge that wakes readers without undoing the pass (as on the branch); a settle that seeds a clock |
| P5.8 | The reference: rebuilds, an IC's state, the mode switch (§4.4) | todo | | Shown to fail: the shrunk gate's edge in temporal mode and an IC reload that powers its internals on (both as on the branch); a mode switch that seeds |
| P5.9 | The exact rounds, on 1,500 random feedback circuits with ICs, on every run (§4.6) | todo | | Shown to fail: the pulse model's correction undone (94 unsound outputs in 1,500 circuits, as on the branch); its skips counted |
| P5.10 | The every-order oracles, four draws, and the detector (§4.7) | todo | | Shown to fail: the correction undone (158 and 200 unsound outputs on the acyclic draws, as on the branch); the delayed draw shows no exposure |
| P5.11 | Memory over every order: the six kinds, every pattern of inputs to every other in one instant (§4.8) | todo | | Shown to fail: a kind that stores one order's result |
| P5.12 | The levels an Unknown stands for, at the loop settles that start from or read one (§4.9) | todo | | New. Shown to fail: a settle that keeps a level where an assignment of the Unknowns gives the other |
| P5.13 | The exact oracle with delays, on the temporal and delayed draws (§4.10) | todo | | New. Shown to fail: the tie merge removed; the covering rule removed (7 unsound outputs in two circuits of the delayed draw, as on the branch's probe, `b28bb61ad`); agrees with the reference before plan 7 offers temporal mode; reports the outputs read Unknown where one level is certain (180 and 284 on the branch) |
| P5.14 | The spec's traces, pinned; the causes file and its two rules; the examples walked for trips and the budget (§4.11) | todo | | Shown to fail: a cause dropped, a unit removed, a malformed line, a unit named twice (each as on the branch); a display traced through its own copies of its inputs (as on the branch) |
| P5.15 | The list of differences from `master`'s engine, each with its cause (§4.15) | todo | | Each entry backed by the causes file, a scene golden or a test; plan 2's fixes, on `master` before this plan starts, are not in it |
| P5.16 | Every step of every shipped walk against every order, past seven changed inputs by a search over subsets; each loop settle against the exhaustive search; in CI (§4.12) | todo | | Shown to fail: the correction undone (6,522 contradicted values in 18 walks, as on the branch); the subset search checked against the enumeration up to seven |
| P5.17 | The five values at step 87 of the 8-bit stack pointer's walk, decoded (§4.13) | todo | | New. A test pins each value with its cause |
| P5.18 | Larger and more numerous loops at the work limit: which tier answers (§4.14) | todo | | New. Checked by the exact search where it runs; plan 6 times them (P5.24) |
| P5.19 | The developer guide's section on the oracles and how to run them; the tests' README | todo | | |
| P5.20 | Algorithm B's termination with Error values: proven in the spec, asserted in the reference (§7) | todo | | Shown to fail: a non-monotone kernel put in trips the assertion |
| P5.21 | No subset mask wider than its type: a settle the work limit could never cover takes the next tier first (§7) | todo | | Asserted; a test with more events than the limit covers |
| P5.22 | Each cap at its exact value: the budget, the work limit and its tier's, the inputs that move at will, the exact oracle's caps (§7) | todo | | One below, at and one above each |
| P5.23 | The search over subsets against every order past seven events (§7) | todo | | Checked at six and seven only on the branch; sampled orders past seven |
| P5.24 | The work limit's value, 2^18 member evaluations, provisional (§7) | todo | | Set by plan 6's timing of P5.18's loops; if it moves, the spec first |
| P5.25 | Whether the spec needs a covering rule for a delayed node's pending level under a divergent change (§7) | done | `b28bb61ad` | Decided on 2026-10-21 by the exact oracle with delays, run as a probe on `gpu-canvas`: needed; the user's "yes" (P5.D21, §2.10) |
| P5.25a | The covering rule in the spec and the reference: a clock reaching a divergent Unknown makes its memory node store Unknown (§2.10) | todo | | Shown to fail: the probe's traced circuit (a racing NOR latch read through delays into a T flip-flop's clock) holds Q at 0 without it |
| P5.26 | The every-order walks' run time on the reference, on a release build: on every push or in a scheduled job (§7) | todo | | Measured on CI's release build (the user, 2026-10-21); a faster reference only once it equals the current one slot by slot |
| P5.27 | The reset source in the reference, its draws and its tests, with plan 6's element type (§7) | todo | | Carried by plan 6; closed here when that step lands |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-21: P5.25 done: the exact oracle with delays, run as a probe on `gpu-canvas` (`b28bb61ad`), found 7 unsound
  outputs in two circuits of the delayed draw; the spec takes the covering rule (P5.D21, the user's "yes"), built as
  the new row P5.25a. The every-order walks run in CI on a release build (P5.D20, P5.26; the user's word).
