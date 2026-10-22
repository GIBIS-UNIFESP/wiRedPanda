# Ledger: plan 2, fixes to `master`

Tracks the items of `.claude/PLAN_02_master_fixes.md`. The plan says what and why; this file says where each item
stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P2.9a`, `P2.9b`, ...) and a line in the log, and a defect found on `master` later
gets the next free row. Status: `todo`, `doing`, `done` (committed), `blocked` (on what, in the notes) or `dropped`
(why, with the user's decision). The Commits column lists every commit of the item. Every status change gets a dated
line in the log.

**Branch:** `02-master-fixes`, from `01-characterization` once plan 1 has landed; not created yet. Each fix is a slice
of its own.

**Open decisions:** P2.36 may wait on the user, once measured. P2.37 is settled (P2.D10).

Each row's notes say how its test is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P2.1 | Fuzz seeds written behind the header as a compressed payload; a seed check in each seeded harness, the first header read inside a `try` (§2.1) | todo | | Shown to fail: a seed with no elements, one with an unknown header and an empty one each stop the harness with its own message (as on the branch). Lands after the user's overnight run of the harnesses (P2.D10) |
| P2.2 | `ENABLE_FUZZER` instruments the whole project for coverage (§2.2) | todo | | Shown: one harness's edge count before and after (154 against 27,474 on the branch). Finished after the user's overnight run of the harnesses (P2.D10) |
| P2.3 | BeWavedDolphin's text export splits its rows by port (§2.3) | todo | | Shown to fail: the split row with an input element of several ports, on `master` |
| P2.4 | The Python MCP client takes clock delays in [-1, 1] (§2.4) | todo | | Shown to fail: a negative delay refused on `master`. ruff, pyright and mypy pass |
| P2.5 | A save whose write fails is not committed, through one helper for the five saves (§2.5) | todo | | Shown to fail: the failed commit over an existing file, on a local Qt 6.7.3 build with the check removed (as on the branch). Right on every Qt; plan 2's while `master` declares 6.2 (P2.D9) |
| P2.6 | A buzzer's sink is destroyed before its tone generator (§2.6) | todo | | Shown to fail: `TestBuzzer` on a local Qt 6.7.3 build (as on the branch). Right on every Qt; plan 2's while `master` declares 6.2 (P2.D9) |
| P2.7 | An MCP client that closes the connection closes a clean window without the exit question (§2.7) | todo | | Known from reading; the test drives the whole chain and fails on `master` first. The Windows stdin path compiled by CI's Windows jobs |
| P2.8 | `ItemWithId`'s destructor comment gives the right reason (§2.8) | todo | | Comment only: no build or test |
| P2.9 | An open S, R or T reads Inactive (§3.1) | todo | | Shown to fail: each default put back to Unknown in turn (as on the branch). Goldens: port layout (48 lines on the branch), one scene snapshot, any trace with S, R or T open |
| P2.10 | The loader refuses a port count above the type's maximum, and a Demux's inputs its outputs do not need (§3.2) | todo | | Shown to fail: each check removed in turn (as on the branch). The strings golden gains the three messages in the same commit |
| P2.11 | The AudioBox's audio file is one undoable step (§3.3) | todo | | Shown to fail: the push removed (as on the branch). Golden: one editor-writes row |
| P2.12 | A lowercase trigger key pushes one undo command (§3.4) | todo | | Fails on `master` with two commands (as on the branch). Golden: three editor-writes rows |
| P2.13 | Mux, Demux and the truth table offer no custom appearance (§3.5) | todo | | Shown to fail: Mux's flag put back (as on the branch). Golden: three types' property kinds |
| P2.14 | Every input operation that changes saved state is one undoable command (§3.6) | todo | | Each of the four tests fails on `master` (as on the branch) |
| P2.15 | Latches' and flip-flops' themed pictures follow the theme and keep a custom one (§3.7) | todo | | All three tests fail on `master`, the undo test without the snapshot part alone (as on the branch) |
| P2.16 | A morph's undo restores the elements it replaced, the rebuilt ones owned until they replace them (§3.8) | todo | | Fails on `master`: a clock's frequency and a 4-input And's inputs and wires lost. Needs P2.35 first |
| P2.17 | The property editor: a property command that merges an edit session, each field setting its own property (§3.9) | todo | | Each test fails on `master` first; the session's commands name the same elements (two clocks of different delays, two switches) |
| P2.18 | One rule against duplicate Tx labels for every writer (§3.10) | todo | | The inline editor's refusal fails on `master` |
| P2.19 | MCP's property setter validates every value first, refuses a colour or trigger the editor would not take, and pushes one step (§3.11) | todo | | Each refusal fails on `master` (applied, or taken with no undo entry). Golden: MCP's responses to refused requests |
| P2.20 | The lock's tooltip says what a lock does (§3.12) | todo | | One string: the strings golden in the same commit |
| P2.21 | A display's lit segments stay inside its bounds with a small custom picture (§3.13) | todo | | Not checked on `master`: reproduced first, else closed with what was run |
| P2.22 | A flipped IC reopened from a file keeps its ports' positions (§3.14) | todo | | Not checked on `master`: reproduced first, else closed with what was run |
| P2.23 | An empty Text's hint follows a language change (§3.15) | todo | | Read on `master`, not run: reproduced first, else closed with what was run |
| P2.24 | A loaded IC keeps no tooltip after a language change (§4.1) | todo | | Fails on `master`: the tooltip comes back after a retranslation |
| P2.25 | A rebuild keeps each clock's level and phase (§5.1) | todo | | Fails on `master`: an edge on a structural edit. Plan 1's fast-path test of a clock's phase reworked to observe the rebuild, and shown to fail again |
| P2.26 | Setting a port's default leaves a connected input on its wire (§5.2) | todo | | Fails on `master`: a flip-flop's wired ~Preset through a reload |
| P2.27 | The priority walk pops the node it assigns (§5.3) | todo | | Measured first (P2.36) |
| P2.28 | Each SystemVerilog truth table declares its own output regs (§6.1) | todo | | Fails on `master`: iverilog reports the reg already declared |
| P2.29 | Exported names do not depend on the interface language: elements by the factory's untranslated name, ports and an IC's pins by their source names (§6.2) | todo | | Fails on `master`: two languages give two texts, for each segment display and for an IC holding a 7-segment display. Golden: plan 1's tagged lines become English, every English line unchanged |
| P2.30 | Both exporters resolve an Rx from its Tx when a wire also reaches it (§6.3) | todo | | Fails on `master`: the wire's driver exported |
| P2.31 | A wheel zoom keeps the point under the cursor (§7.1) | todo | | Both tests fail on `master` (as on the branch). Golden: the wheel rows |
| P2.32 | The hit test answers the topmost item (§7.2) | todo | | Fails on `master`: a drag of the top element is not undoable. Golden: the hit grid's overlapping rows |
| P2.33 | MCP's image export reports an empty circuit (§7.3) | todo | | Fails on `master`: the error is unreachable |
| P2.34 | Whether the floor's Qt 6.8 (6.8.2 and 6.8.3) shows the two Qt 6.7.3 defects (§12) | todo | | Seen on 6.7.3, not on 6.9.3, 6.8 not checked. For the release notes; the fixes hold on every version |
| P2.35 | What a snapshot restore does on `master` with a custom picture that no longer loads (§12) | todo | | Checked before P2.16 |
| P2.36 | The priority walk's fix against plan 1's traces (§5.3) | todo | | With no trace changed, P2.27 lands; with traces changed, the user decides between the fix with each change named and leaving the walk to plan 6's switch |
| P2.37 | When the fuzz harnesses run (§12) | done | | Settled by the user, 2026-10-21: on the user's machine, overnight, through `Tests/Fuzz/run-unattended.sh`, not in CI (P2.D10). A decision, with nothing to commit; a slice that adds or changes a harness lands after the user's run of it |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-21: P2.37 done, settled by the user ("i dont think its necessary/useful to run fuzzer in CI. i can run it
  overnight on my pc"): the harnesses run on the user's machine through `Tests/Fuzz/run-unattended.sh`, not in CI
  (P2.D10). The CI workflow proposed for them is not added.
- 2026-10-21: the Qt floor raised to 6.8 at the user's ruling ("go with 6.8"). P2.5 and P2.6 stay plan 2's but no
  longer gate the floor, which adds no 6.7.3 build: each fix is right on every Qt, and `master` declares 6.2 until the
  floor (P2.D9); P2.34 records whether 6.8.2 and 6.8.3 show the defects.
