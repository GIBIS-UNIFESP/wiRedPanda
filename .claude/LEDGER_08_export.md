# Ledger: plan 8, export fidelity

Tracks the items of `.claude/PLAN_08_export.md`. The plan says what and why; this file says where each item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P8.4a`, `P8.4b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** its own, from `master`, once plan 7 has landed; not created yet.

**Open decisions:** none yet; P8.16 (yosys and verilator in CI) is settled here once P8.17 has measured the run time.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P8.1 | Identifiers from untranslated names and source names; the three displays under a translator (§2.1) | todo | | Shown to fail: a display port whose source text goes through `tr()` fails both translation tests (as on the branch) |
| P8.2 | Both exporters on the builder's order, Tx map and definitions; the scene's helpers go (§2.2) | todo | | Every changed golden named by cause; no exporter includes the scene or the simulation, which the include check shows |
| P8.3 | Two drivers and an unmappable loop are export errors; the level reads on the builder's values (§2.2) | todo | | Shown to fail: an export that follows the Tx into a net with a wire too |
| P8.4 | SystemVerilog starts and computes as the simulator: unseeded loops, `?:` trees, clocks wrapped, the reset source, the timescale (§2.3) | todo | | Shown to fail: a seeded loop prints 1 where the simulator reads Unknown (as on the branch); a padded select exporting `1'b0` |
| P8.5 | Memory's Unknown in simulation-only code (§2.3) | todo | | Shown to fail: a clock from 0 to x that stores D; yosys's synthesized netlist unchanged |
| P8.6 | The Arduino sketch: rounds, a derived settle bound, the reset flag (§2.5) | todo | | Shown to fail: a ripple counter that ripples through in one round (as on the branch) |
| P8.7 | Exporter notes as data, in the header and shown to the user, the Arduino export's note on dropped delays among them (§2.6) | todo | | Shown to fail: an IC's internal undriven input left out of the notes (as on `temporal-engine`); a delay note in functional mode; a delay note naming an element at its type's default |
| P8.8 | One testbench runner, one `g++` harness with one copy of its stubs (§2.7) | todo | | |
| P8.9 | The oracle: values with no level, x as x, a floor per output, steps out of domain by cause with the race counter, explanations both ways, serialised stimuli (§2.7) | todo | | Shown to fail: a testbench with no readings, which passed as `PASS: 0 steps` (read on the branch); a non-level recorded as 0 (six circuits, as on the branch); an explanation that no longer occurs |
| P8.10 | Comments and test text written for the new exporters (§2.7) | todo | | |
| P8.11 | iverilog in CI; the tests fail instead of skipping; the differential on every run (§2.8) | todo | | Shown to fail: CI without iverilog fails the job instead of skipping |
| P8.12 | SystemVerilog in temporal mode (§2.4) | todo | | Functional goldens byte for byte; shown to fail: `<= #d` on a memory node glitches for 4 units under a double sample |
| P8.13 | The IC generators' two rules in their notes (§2.9) | todo | | |
| P8.14 | The external corpora through the differential, for development (§2.10) | todo | | Outside the repository, at pinned versions; the counters per corpus in the log |
| P8.15 | The IC instances' internal elements deleted, the IC folded into the one element class (§2.11) | todo | | This plan's last step; closes plan 3's P3.26 and plan 4's P4.11 |
| P8.16 | Whether CI installs yosys and verilator too (§6) | todo | | After P8.17 |
| P8.17 | The run time of all validation on every run (§6) | todo | | Measured on CI |
| P8.18 | The share of out-of-domain settles per circuit (§6) | todo | | On the suite and the corpora |
| P8.19 | Whether the export's x matches the simulator's Unknowns that no race causes, from zero-delay loops (§6) | todo | | |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
