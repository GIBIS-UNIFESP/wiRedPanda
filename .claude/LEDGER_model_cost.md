# Ledger: the model-cost round

Tracks the items of `.claude/PLAN_model_cost.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`MC.4a`, `MC.4b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** its own, from `master`, once plan 9 has landed; not created yet.

**Open decisions:** none yet; MC.11 is settled with plan 10's P10.19 if an optimisation changes the listener's events.

Each row's notes say how its check is shown to fail; "as on the prototype" marks a break `perf/load-hotspots` ran and
saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| MC.1 | Callgrind modes scoped to the load and to start-up, every profiling run with an isolated configuration and no update check (§2.1) | todo | | Shown to fail: a run without the isolated configuration shows the update check in the load's profile (4.8% on the prototype) |
| MC.2 | The probe's phases: rubber bands, select all, a click selection, one and many elements moved, the clock dragged, a wire drag, on the named fixtures (§2.1) | todo | | Shown to fail: a phase whose rubber band covers nothing, or whose move the snap swallows, fails its own state assertion |
| MC.3 | The baseline: every phase counted on `master` as plan 9 leaves it, every one of the 37 looked for in the profiles (§2.2) | todo | | The counts in the log; each of the 37 marked found, gone, or moved to the code that replaced it |
| MC.4 | Selection's costs (§2.3) | todo | | Each change with its measured drop; the selection tests and plan 1's pointer goldens unchanged |
| MC.5 | Move's costs: one element, many, the clock, a wire drag (§2.3) | todo | | Each change with its measured drop; the listener's coverage test and plan 1's geometry goldens unchanged |
| MC.6 | Load's costs (§2.4) | todo | | Each change with its measured drop; plan 1's payload goldens and the backward-compatibility files unchanged |
| MC.7 | Painting's costs on the CPU backend (§2.5) | todo | | Each change with its measured drop; plan 1's wire-pixel goldens and the parity renders unchanged, or the change shown to the user |
| MC.8 | Start-up's costs: the Language menu, the palette's icons, the colour combo built when first shown (§2.6) | todo | | Measured with the start-up mode; each control's first showing tested |
| MC.9 | The measurement after: every phase against the baseline, each of the 37 accounted for (§2.7) | todo | | Re-derived with its drop, not needed, or dropped as too small |
| MC.10 | Whether the new model still pays the spikes' costs: the rubber band's 158 ms, the clock's 70 ms (§6) | todo | | Answered by MC.3 before anything is optimised |
| MC.11 | Whether an optimisation changes what the listener reports (§6) | todo | | Settled with plan 10's P10.19 before plan 10 starts; the coverage test stays green |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
