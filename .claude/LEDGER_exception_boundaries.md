# Ledger: exception boundaries

Tracks the items of `.claude/PLAN_exception_boundaries.md`. The plan says what and why; this file says where each
item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a fix the inventory finds gets the next free
row (`EB.4`, `EB.5`, ...) and a line in the log. Status: `todo`, `doing`, `done` (committed), `blocked` (on what, in
the notes) or `dropped` (why, with the user's decision). The Commits column lists every commit of the item. Every
status change gets a dated line in the log.

**Branch:** `02b-exception-boundaries`, from `02a-qt-floor` once the floor has landed; not created yet.

**Open decisions:** none.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| EB.1 | The inventory of §3: every throw, catch and `guardedSlot()` site, its frames and its class | todo | | Counts of §1 recounted here. A site whose callers cannot be traced by reading is reached by a probe |
| EB.2 | The checks that keep the rule (§5): the inventory against the sources, a test per fixed site | todo | | Shown to fail: a `PANDACEPTION` added to a command and a `catch` removed each fail the inventory check |
| EB.3 | An AudioBox restored with its sound gone (§4) | todo | | Read 2026-10-24, not run: `AudioBox::setAudio()` throws on a missing file and a snapshot restore calls it. It plays its default sound, the path kept (EB.D3) |
| EB.4 | A command whose first `redo()` throws leaks, its change half applied | todo | | Found 2026-10-26 by `fuzz_undo`'s fourth coverage round (plan 2's P2.78), as a leak: an `UpdateCommand` whose snapshot names an IC that no longer resolves throws from `redo()` inside `QUndoStack::push()`, which takes ownership only after `redo()` returns, so the command leaks and what `redo()` did before the throw stays. The harness reached it by removing a blob still in use, which the application never does (it removes one with its ICs, `WorkSpace::removeEmbeddedIC()`); in the application the same throw needs an IC whose file is gone when a command reloads it. Read, not run, in the application. Each command's first `redo()` goes in the inventory (EB.1). Inside an undo macro the same throw also leaves the macro open, every later edit recorded into it, as plan 2's P2.57 (paste) and P2.76 (`createEmbeddedIC()`) did; both now load before the macro opens. The other macros, read 2026-10-27 (plan 2's audit, AU.8): `Workspace.cpp:914`, `PropertyShortcutHandler.cpp:44`, `:145`, `:221`, `:252`, `ElementEditor.cpp:725`, and `:250`, unreachable; none of their commands can throw today (built from live items, loading no file; the shortcuts' `UpdateCommand` reloads only clocks, buzzers, displays and LEDs, never an IC or an AudioBox; a missing picture shows the default since P2.50). Every macro site goes in the inventory with the commands it pushes, so a command that comes to throw closes its macro |
| EB.D2 | How the rule is kept (§5) | done | | Decided 2026-10-28 by the user: the inventory checked in CI against the sources, and a test through Qt's frame per fixed site; no `noexcept`, since `notify()` stays the last-resort reporter |
| EB.D3 | What an AudioBox restored with its sound gone plays (§4) | done | | Decided 2026-10-28 by the user: its default sound, the path kept, as P2.50's pictures and plan 3's P3.D5 |

## Log

- 2026-10-24: the plan file and this ledger written on `02-master-fixes`, at the user's ruling (EB.D1); EB.1 `todo`,
  EB.2 and EB.3 `blocked` on EB.D2 and EB.D3.
- 2026-10-26: EB.4 added, `todo`: a command whose first `redo()` throws leaks from `QUndoStack::push()`, found by
  `fuzz_undo`'s fourth coverage round on `02-master-fixes`.
- 2026-10-27: EB.4's notes add the undo macros: a throw inside one leaves it open (plan 2's P2.57, P2.76); the nine
  sites read for plan 2's audit (AU.8), none of whose commands can throw today, go in the inventory.
- 2026-10-28: EB.D2 and EB.D3 decided by the user: the rule kept by the inventory checked against the sources and a
  test per fixed site, no `noexcept`; an AudioBox restored with its sound gone plays its default. EB.2 and EB.3 `todo`.
