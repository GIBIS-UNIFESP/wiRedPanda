# Ledger: plan 1, characterization on `master`

Tracks the items of `.claude/PLAN_01_characterization.md`. The plan says what and why; this file says where each item
stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P1.4a`, `P1.4b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists
every commit of the item. Every status change gets a dated line in the log.

**Branch:** its own, from `master`, once the plan files have landed; not created yet.

**Open decisions:** none.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P1.1 | The golden helpers (§2): a text and a byte comparison, both writing the golden under `GENERATE_EXPECTED_SNAPSHOTS`, also over an existing one, and failing on a missing golden; a missing input directory as a failing row; the shared printers and the elements-by-id helper; the test-only friend class; the one list of types left out on purpose | todo | | Shown to fail: a golden moved aside fails, and a renamed input directory fails its row, naming it (both as on the branch); a byte golden changed by hand is rewritten under the switch |
| P1.2 | The element count kept by the item registry (§4.1): the status bar reads it; a test compares it with a full walk after every mutation path, in every build; a debug assertion checks it on every walk | todo | | Shown to fail: an id forgotten while its element lives, the branch's defect, fails the test and the assertion (as on the branch) |
| P1.3 | The perf fixtures' generator (§4.2): the small sizes committed, the large ones written outside the repository; status 1 and a message on any failure | todo | | Checked before its slice lands: run against the debug and the relwithdebinfo builds, every file byte-identical. Needs P1.2 |
| P1.4 | The scene snapshot of every example, IC component, fixture and perf fixture, and the test that fails on a golden whose circuit is gone (§3.1) | todo | | Shown to fail: JKLatch taken off the left-out list prints `NO CREATOR` and fails (as on the branch, in the trace dump); a golden left for a removed circuit fails the second test |
| P1.5 | Wire geometry after a move, a rotation, a flip, a port-count change and a resize, with rendering on and the rotation and flip rows on an IC, an And and a Not (§3.2) | todo | | Shown to fail: with the re-routing paths and `orientPort()` off, the move, rotate, flip and resize rows fail (24 of 30 wire rows on the branch); the resize made a no-op fails its ports golden (as on the branch) |
| P1.6 | Port layout: every type at every legal port count, the full-adder IC, sixteen orientations in full, names under the tagging translator, and the orient-first pass (§3.3) | todo | | Shown to fail: one port's position moved in one orientation fails, naming the port; JKLatch off the left-out list fails as `NO CREATOR` |
| P1.7 | Labels: every labelled type and orientation, the IC and the truth table at their fewest and most ports, the label-less types listed (§3.6) | todo | | Shown to fail: a silent skip put back for one type fails (as on the branch) |
| P1.8 | The saved payloads of every example, fixture, IC component, perf fixture and backward-compatibility file, the header checked apart (§3.14) | todo | | Shown to fail: a golden moved aside fails; the backward-compatibility directory renamed fails its row (both as on the branch) |
| P1.9 | Hit testing and pointer behaviour: the grid, the Error wire's click width, and every gesture of §3.4 as its own test function, the drop sent as events, paste under the clipboard lock | todo | | Shown to fail: `portAt()` made to answer the topmost port changes the rows where ports overlap, which is plan 2's diff; each gesture test fails with its path disabled (as on the branch for the drop and paste). Closes P1.22 |
| P1.10 | Stacking: the z-order goldens of the 27 circuits, the order after a delete's and a morph's undo, the render against Qt's own, and the rubber band and a wire being routed across overlapping elements, which the branch never pinned (§3.8) | todo | | Shown to fail: one element's z changed fails the order goldens (all 27 on the branch); the paint order reversed fails the comparison with Qt's render (1,640 pixels on the branch) |
| P1.11 | Wire pixels: the synthetic circuit and the 2,000-element clocked fixture at several scales and positions, antialiasing held on and asserted, Light theme (§3.7) | todo | | Shown to fail: one wire left unpainted fails; the bound is P1.21 |
| P1.12 | Snapping and the view: align, distribute, rotate, flip, move and nudge with undo and redo; drag, drop and paste; the zoom ladder, fit, centring, scrolling into view and the wheel zoom, the scroll position in every row (§3.9) | todo | | Shown to fail: the wheel zoom's fix, plan 2's, changes exactly the wheel rows (two on the branch); a nudge of one grid step more fails its rows |
| P1.13 | Simulation traces: every logic type at its fewest and most ports, every combination up to eleven inputs, the seeded walks, the fixtures, the 73 IC components, the four-gate IC and the IC-heavy circuit; clocks frozen and asserted; driverless elements through their input ports (§3.5) | todo | | Shown to fail: JKLatch off the left-out list fails as `NO CREATOR` (as on the branch); a display traced through its simulation inputs reads all zeros and fails |
| P1.14 | IC previews: port counts and ink for every component, `level2_mux_8to1`'s pixels with its labels masked and the masks checked, taken with rendering on (§3.10) | todo | | Shown to fail: the render skipped fails every non-null row's ink (71 on the branch); the labels emptied in the renderer fail the pixel golden (as on the branch) |
| P1.15 | The property kinds of the 31 creatable types, what the editor writes for each kind with the undo commands, and MCP's responses per type (§3.11) | todo | | Shown to fail: the editor's volume scale changed fails the writes golden; the rotate response's angle changed fails the responses golden (both as on the branch). Pins three `master` defects for plan 2 |
| P1.16 | The Arduino and SystemVerilog generators under the `[tr %1]` translator: an And gate and the 7-, 14- and 16-segment displays, as a sketch and a module (§3.12) | todo | | Shown to fail: a generator made to read the untranslated names, plan 2's fix, changes exactly the tagged lines |
| P1.17 | The translatable-strings target and golden; CI builds the target on one job; every check before a slice lands builds it before the suite (§3.13) | todo | | Shown to fail: a changed `tr()` string fails, naming the line (as on the branch) |
| P1.18 | The probe harness and its script, run only when `WIREDPANDA_PERF_PROBE` is set, the variable named the same in the test, the script and the tests' README (§4.3) | todo | | Shown to fail: each of its checks once. Closes P1.23 |
| P1.19 | The test readers return what a port holds: `inputStatus()` and `outputStatus()` a `Status`, the multi-bit reader a value carrying each bit's status (§4.4) | todo | | The user's choice of 2026-10-21: part of plan 1. Each expectation that changes is named by its cause: a `master` defect for plan 2, a reading plan 5 changes, or a wrong test. Shown to fail: the readers' own test fails on a reader that reads Unknown as 0 |
| P1.20 | The two fast-path tests of the update command build the simulation first and update after the command (§4.5) | todo | | Shown to fail: each with its fast path removed |
| P1.21 | The wire-pixel bound (each channel within 2, no pixel more) holds on CI's renderers (§8) | todo | | Measured on the development machine only. Plan 1's pull request runs the goldens on CI; a failure there widens the bound, with the run named |
| P1.22 | `QDrag::exec()`, the clipboard and the zoom-to-selection bounds behave under the offscreen platform as on a desktop; the hit grid's run time (§8) | todo | | Left open on the branch. Closed or recorded with P1.9 |
| P1.23 | Whether `master` still spends about 90 s in Qt's BSP index rebuild before the first paint of the 8,000-element circuit, as a profile of July 2026 found (§8) | todo | | Not measured since. The probe (P1.18) times the load and its first frames, and the result is recorded here |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
