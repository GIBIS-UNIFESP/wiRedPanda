# Ledger: exception boundaries

Tracks the items of `.claude/PLAN_exception_boundaries.md`. The plan says what and why; this file says where each
item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a fix the inventory finds gets the next free
row (`EB.4`, `EB.5`, ...) and a line in the log. Status: `todo`, `doing`, `done` (committed), `blocked` (on what, in
the notes) or `dropped` (why, with the user's decision). The Commits column lists every commit of the item. Every
status change gets a dated line in the log.

**Branch:** `02b-exception-boundaries`, from `02a-qt-floor` once the floor has landed; not created yet.

**Open decisions:** EB.D2 (how the rule is kept), EB.D3 (what an AudioBox restored with its sound gone plays).

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| EB.1 | The inventory of §3: every throw, catch and `guardedSlot()` site, its frames and its class | todo | | Counts of §1 recounted here. A site whose callers cannot be traced by reading is reached by a probe |
| EB.2 | The check that keeps the rule (§5) | blocked | | On EB.D2. Shown to fail: a command made to throw in `undo()` fails it |
| EB.3 | An AudioBox restored with its sound gone (§4) | blocked | | Read 2026-10-24, not run: `AudioBox::setAudio()` throws on a missing file and a snapshot restore calls it. On EB.D3 |
| EB.4 | A command whose first `redo()` throws leaks, its change half applied | todo | | Found 2026-10-26 by `fuzz_undo`'s fourth coverage round (plan 2's P2.78), as a leak: an `UpdateCommand` whose snapshot names an IC that no longer resolves throws from `redo()` inside `QUndoStack::push()`, which takes ownership only after `redo()` returns, so the command leaks and what `redo()` did before the throw stays. The harness reached it by removing a blob still in use, which the application never does (it removes one with its ICs, `WorkSpace::removeEmbeddedIC()`); in the application the same throw needs an IC whose file is gone when a command reloads it. Read, not run, in the application. Each command's first `redo()` goes in the inventory (EB.1) |
| EB.D2 | How the rule is kept: `noexcept` commands, or a restore test per command (§5) | open: the user's choice | | |
| EB.D3 | What an AudioBox restored with its sound gone plays: silence, or its default sound (§4) | open: the user's choice | | |

## Log

- 2026-10-24: the plan file and this ledger written on `02-master-fixes`, at the user's ruling (EB.D1); EB.1 `todo`,
  EB.2 and EB.3 `blocked` on EB.D2 and EB.D3.
- 2026-10-26: EB.4 added, `todo`: a command whose first `redo()` throws leaks from `QUndoStack::push()`, found by
  `fuzz_undo`'s fourth coverage round on `02-master-fixes`.
