# Ledger: plan 7, temporal mode in the application

Tracks the items of `.claude/PLAN_07_temporal_mode.md`. The plan says what and why; this file says where each item
stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P7.5a`, `P7.5b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** its own, from `master`, once plan 6 has landed; not created yet.

**Open decisions:** none yet; P7.15 needs the user once the selectors and BeWavedDolphin's controls are drawn.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break an earlier branch ran and saw
fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P7.1 | The delay parameter: schema, file key, snapshots, morphs, one setter; the netlist reads it (§2.2) | todo | | Shown to fail: a delay-free type that keeps a delay through save and load (as on the branch); a snapshot restored from before the first edit that keeps the edit (as on the branch); a file at the default that gains the key; a morph's undo that loses it |
| P7.2 | The delay in the property editor (§2.3) | todo | | Shown to fail: the field missing on a gate (as on the branch); an edited delay that never reaches the core (as on the branch); one undo entry per keystroke |
| P7.3 | MCP's delay property and the Python client's models (§2.6) | todo | | Shown to fail: a delay accepted on a delay-free type; a write applied before another value fails validation |
| P7.4 | The gate: plan 5's exact oracle with delays agrees, and its covering rule is in the spec (§2.1) | todo | | Closes when plan 5's P5.13 and P5.25 have; the selectors wait on it |
| P7.5 | The mode and speed selectors and the readout (§2.4) | todo | | Shown to fail: a mode switch that seeds or resets (as on the branch); a tab switch that keeps the other tab's mode; the readout's timer running in functional mode (as on the branch); a speed that changes what a circuit does |
| P7.6 | MCP's mode, speed and simulated time (§2.6) | todo | | Declared in the schema and the client; shown to fail by a mode set on another tab than the current one |
| P7.7 | `set_input_value` through the settle, its response declared (§2.6) | todo | | Shown to fail: one tick in temporal mode reads the value before it settles (as on the branch); a free-running clock answers instead of looping |
| P7.8 | BeWavedDolphin's temporal sweeps through the runner's sweep (§2.5) | todo | | Shown to fail: a sweep during a live temporal run that drops a capture in flight or rewinds time (both as on the branch); a value off the presets shown as a preset (as on the branch) |
| P7.9 | MCP's waveform commands: `temporal` and `ns_per_column` (§2.6) | todo | | Shown to fail: `ns_per_column` without `temporal` accepted and ignored (as on the branch) |
| P7.10 | The three temporal examples, their generators and the script that runs them (§2.7) | todo | | Each generator run; each example's description checked against what it shows in both modes |
| P7.11 | Every example run in temporal mode, nothing tripping or oscillating unless meant to (§2.7) | todo | | Shown to fail: the ring with no enable stays Unknown; a ring meant to oscillate is named in the test |
| P7.12 | The developer guide, `CLAUDE.md`, the READMEs and the MCP text (§2.6, §2.8) | todo | | |
| P7.13 | The manual pass (§2.9) | todo | | Its findings in the log |
| P7.14 | The Python client reads the bounds from the schema file (§6) | todo | | Or keeps a copy that a test compares with the schema |
| P7.15 | What the user sees new: the selectors, the readout, BeWavedDolphin's controls and its sweep mode (§6) | todo | | Shown to the user before their slices land |
| P7.16 | The slowest speeds show a delay on the canvas at the display rate: two presets slower than `temporal-engine`'s (P7.D5, §6) | todo | | Decided 2026-10-21 by arithmetic (P7.D5); watched by hand on the gate delay chain and the ring at each preset: the slower two step visibly; a test reads the presets, the slowest 1 ns per 100 ms |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-21: P7.16's question decided by the user ("go ahead"): two speed presets slower than `temporal-engine`'s,
  1 ns per 10 ms and per 100 ms (P7.D5); the row keeps the watch by hand.
