# Ledger: plan 4, ICs and the document format

Tracks the items of `.claude/PLAN_04_ics_and_format.md`. The plan says what and why; this file says where each item
stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a task found along the way gets the next free
sub-ID of the item it belongs to (`P4.4a`, `P4.4b`, ...) and a line in the log. Status: `todo`, `doing`, `done`
(committed), `blocked` (on what, in the notes) or `dropped` (why, with the user's decision). The Commits column lists the code commits of the item; a record commit is named in the log. Every status change gets a dated line in the log.

**Branch:** `04-ics-and-format`, from `03-element-types` at `2fe7e07f9` (the creation point; the merge-base with `03-element-types` is `d8f9e8765`), created 2026-11-02. Hashes from before the restack that no longer resolve were replaced by the commit each row names; those replacements are unverified.

**Open decisions:** none.

Each row's notes say how its check is shown to fail; "as on the branch" marks a break `gpu-canvas` ran and saw fail.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| P4.1 | The reload's undo as a failing test on `master`: one input after the undo, the cut wire still cut (§2.1) | done | `6f9f80d1f` | Run on the branch's tip, not `master` (plan rules: each plan starts from the one before). The undo restores the input and the wire in the session, so the in-session checks pass. The defect shows only after save and reopen: the file's new definition (one input) comes back, a `QEXPECT_FAIL` marks that check until P4.6. Test `TestICRegistry::testReloadUndoRestoresTheInputAndItsWire`. Shown to fail: the reopen check is the failure; the in-session checks are real. See P4.8 |
| P4.2 | The shared definition, its cache keyed by path and bytes for a file and by bytes for a blob, and the loader reading through `ICFileCache` (§2.2) | todo | | Shown to fail: eviction by insertion order, a failed read kept; a key of path, time and size, which reuses the definition of a file rewritten at the same size within the same millisecond |
| P4.3 | The preview per definition, rendered on first hover; its guard checks the origin (§2.3) | todo | | Shown to fail: the guard checking the size only (as on the branch); a theme change that keeps previews. Plan 1's preview goldens unchanged |
| P4.4 | Pin names from the circuit in one rule; the header in file order; no pin name saved in the interface language (§2.4) | todo | | Shown to fail: the header built in stacking order after an undo's reorder (as on the branch); a derived name made explicit. Payload goldens holding IC headers or pin names change, each named |
| P4.5 | The reload's undo restores the definition in memory (§2.5) | todo | | P4.1 passes |
| P4.6 | The document's copies of file-backed definitions with their hash; the four cases on open (§2.6) | todo | | Each case tested; an undo of a reload saved and reopened keeps its wire; the backward-compatibility files read as before |
| P4.7 | The developer guide's IC and format sections; the release notes' format changes | todo | | |
| P4.8 | The reload's undo behaves on `master` as probed on the branch (§5) | blocked | | The in-session half of §1.2's first defect does not reproduce on this tip (P4.1): the undo restores the input and the wire. The reopen half does. §1.2 and §2.5 rest on the first half, so they need revisiting with the user before P4.5 is started |
| P4.9 | Whether an IC's first reload in a scene always brings another definition (§5) | todo | | Checked here by counting definitions across a first reload; plan 6 checks it for its node ranges |
| P4.10 | The cost of keying a file's definition by its bytes, on the IC-heavy fixture's load (2,000 instances of one file) (§5) | todo | | Measured before P4.2 lands, under callgrind with plan 1's probe; should the profile show the key, it goes back to path, time and size, with the two blind spots stated in the code |
| P4.11 | The hand-over of §2.7: the internal elements, the IC's own serialization and the definitions' move into the document's registry, done by plan 8's last step | todo | | Carried by plan 8 (P8.15); closed here when that step lands |
| P4.12 | The pin order saved in every file as the boundary elements' ids, migrated from positions before any snap; every element snapped on load; landing together (§2.9, P4.D5) | todo | | Added 2026-10-22 from plan 2's snapping rule (P2.42), which leaves the load exact. Shown to fail: the saved order ignored swaps two pins closer than half the grid; the migration taken after the snap swaps them too. P1.14's pin orders unchanged |

## Log

- 2026-10-21: the plan file and this ledger written on `gpu-canvas`, every row `todo`.
- 2026-10-22: P4.12 added with P4.D5, at the user's ruling: an IC's pin order saved in the format and every element
  snapped on load, together, after plan 2 makes setting a position exact and every edit snap (P2.42).
- 2026-11-02: branch `04-ics-and-format` created from `03-element-types` (`2fe7e07f9`).
- 2026-11-02: P4.1 done (`a7e568afe`). P4.8 blocked: the undo restores the input and the wire in the session; the defect is the save-and-reopen case (the file's new definition comes back), which P4.6 addresses. §1.2's first claim needs the user's review before P4.5.
