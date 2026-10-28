# Exception boundaries

Where an exception may be thrown and where it is caught, across the whole code base. `master` reports exceptions
through one global catcher, `Application::notify()`, and wraps many slots in `Application::guardedSlot()`; neither
reaches an exception thrown inside a frame Qt owns, which on macOS ends the application. This plan sets the rule (§2),
lists every throw site against it (§3), fixes each one that breaks it with a test (§4), and adds a check that keeps the
rule (§5). It comes right after the Qt 6.8 floor, before plan 3.

- **Depends on:** the Qt 6.8 floor, landed, since the plans run one at a time, in their order (the plans' rules, §1);
  plan 2's §3.17 (P2.50), the first case of the rule, fixed there.
- **Lands before:** plan 3. Plan 3 rebuilds the element types on code where nothing Qt calls back into can throw.
- **Branch and ledger:** `02b-exception-boundaries`, from `02a-qt-floor` once the floor has landed; a slice per fix
  (§7). `LEDGER_exception_boundaries.md` has a row for every step, fix, check and open question below, and a new row
  for every site found later.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing". This
  file states its own: the rule of §2, and the inventory of §3 kept whole, every throw and catch site with its class.
  Its decisions are numbered EB.D1 on.

## 1. Where `master` stands

Counted 2026-10-24 on `02-master-fixes` (EB.1 recounts on its branch): 123 `throw` sites in 25 files, nearly all
`PANDACEPTION`; 17 `catch` sites; 88 uses of `guardedSlot()`.

- **`Application::notify()`** catches what an event handler throws and reports it (a message box, and Sentry when
  built in). On Linux and Windows an exception from a slot or event handler unwinds through Qt's frames to it. On macOS
  it does not: the unwinder ends the process in Qt's frames, before any outer `catch` runs (QTBUG-15197;
  `Application.h`'s `guardedSlot()` documentation). Seen on CI: P2.16's test, an exception out of a command's `undo()`
  through `QUndoStack`, ended the test process on both macOS jobs (Build 40135689835) with "terminating due to uncaught
  exception of type Pandaception", where on Linux the test framework caught it.
- **`guardedSlot()`** catches inside the slot it wraps, below Qt's frames, and reports synchronously. It covers a slot
  that is ours and is the outermost frame. It cannot cover code Qt calls from inside its own machinery: the Undo
  action's `triggered` is connected to `QUndoStack::undo()` directly (`Scene.cpp`), which calls the command's `undo()`;
  `paint()`, the event handlers of items, a model's data callbacks and a timer's tick are called by Qt the same way.
- **A catch inside a command is no answer either.** The stack steps only when `undo()` returns, so a command that
  catches its own failure returns with the scene half restored and the stack stepped past it.
- **Known cases.** P2.35 and P2.16 found that a snapshot restore whose custom picture no longer loads threw out of
  `undo()`; plan 2's §3.17 (P2.50) fixed pictures: the slot shows its default, the restore goes on, and opening a file
  still reports it. Found while fixing it, read, not run: `AudioBox::setAudio()` throws on a missing or unreadable
  audio file before it changes anything, and a snapshot restore calls it, so an undo that restores an AudioBox whose
  file is gone throws out of `undo()` (EB.3). `ElementEditor::updateElementAppearance()` was connected to its button
  without `guardedSlot()`; P2.50 made it refuse a picture that does not load before anything changes.

## 2. The rule

- **A boundary is code of ours that Qt does not call back into for it:** opening or saving a file, an import, an
  export, an MCP request, the command line. An exception may be thrown below a boundary and is caught at it, before
  control returns to Qt, and reported there.
- **Nothing thrown reaches a frame Qt owns.** A command's `undo()` and `redo()`, `paint()`, an item's or a widget's
  event handler, a model's callbacks, a timer's tick, a slot Qt calls: each finishes and leaves a consistent state.
  What it calls either cannot throw or is caught inside it, at a point where the state is still whole.
- **An expected condition is not an exception.** A file that is gone, a picture or a sound that does not load, a
  value out of range from a user's edit: each has a fallback, a refusal before anything changes, or a report where the
  user acted.
- **A broken invariant is an assertion** (`Q_ASSERT`, with the release build's fallback where one exists), not a
  `PANDACEPTION`.
- **`notify()` stays, as a last-resort reporter** for Linux and Windows and for Sentry; nothing relies on it.

## 3. The inventory

Every `throw` and `catch` site, in a table in the ledger's appendix: file and line, what throws (or what is caught),
the frames that can reach it (traced from its callers up to a boundary or a frame Qt owns), its class (§2: below a
boundary, expected condition, broken invariant) and what is done with it (stays, fixed by row, made an assertion).
`guardedSlot()`'s uses are listed too, each with whether the slot it wraps can be called by Qt from a frame it owns
(a direct connection to Qt's own slot bypasses it). A site whose callers cannot be traced by reading is run: a probe
reaches it from the frame in question.

## 4. The fixes

One slice per fix, each with a test that fails on the floor's branch first. A test that would throw through a frame
Qt owns calls the code directly, since on macOS the throw ends the process; once the fix is in, the test goes through
the frame Qt owns (the undo stack, the event, the tick), so it checks what the user runs. The first known fix:

- **An AudioBox restored with its sound gone** (EB.3): the restore keeps the path and the element plays its default
  sound (EB.D3), as a picture that no longer loads shows its default (P2.50) and as plan 3's audio service will
  (P3.D5); the sound comes back if the file does. Opening a file still reports it.

The rest come from the inventory.

## 5. Keeping the rule

Two checks keep it (EB.D2), and `notify()` stays the last-resort reporter of §2:

- **The inventory is checked.** A CI check lists every `throw`, `PANDACEPTION`, `catch` and `guardedSlot()` site in
  the sources and fails on any the inventory's table does not hold, or holds at a line that no longer has it. A new
  site is classified (below a boundary, an expected condition, a broken invariant) before it lands, which covers the
  sites nobody thought to test. Shown to fail by a `PANDACEPTION` added to a command and a `catch` removed.
- **Each fixed site has a test through the frame Qt owns** (§4): the undo stack, the event, the tick. For the
  commands, each restores a snapshot with each external file it can name gone (a picture, a sound, an IC file) and
  expects no throw, the scene whole and the stack stepped.

`noexcept` on the commands is not used: once nothing below them throws it adds no protection, and its only effect
would be to end the process where §2 keeps `notify()` to report.

## 6. Pitfalls

- **macOS is the only platform that shows the failure.** A throw through Qt's frames is reported on Linux and
  Windows, so a local run passes; only CI's macOS jobs end the process (P2.16).
- **A catch that hides the throw is not a fix.** The state the throw left half done is the defect; a fix leaves it
  whole or refuses before changing it.
- **The translatable strings move.** A `PANDACEPTION` message is a translatable string (`lupdate`'s alias for
  `PANDACEPTION`), so a site made an assertion or a fallback changes the strings golden in the same commit.

## 7. Steps

| Row | Step | Slice |
|---|---|---|
| EB.1 | The inventory of §3, every throw, catch and `guardedSlot()` site with its frames and class | 1 |
| EB.2 | The checks that keep the rule (§5): the inventory against the sources, a test per fixed site | last |
| EB.3 | An AudioBox restored with its sound gone (§4) | its own |

A fix the inventory finds gets the next row (EB.4 on), one slice each.

## 8. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **EB.D1. A plan of its own, right after the Qt floor, before plan 3.** Basis: the user's ruling, 2026-10-24 ("could
  we make that a 2.5 plan?", then "after the qt floor is fine, go ahead"), after P2.16's macOS failure and P2.50.
- **EB.D2. How the rule is kept** (§5): the inventory checked in CI against the sources, and a test through Qt's
  frame for each fixed site; no `noexcept`, which would replace `notify()`'s last-resort report with an end of the
  process. Basis: a ruling, 2026-10-28 ("wasnt the plan to replace the global catcher with proper localized catches?",
  then "go ahead").
- **EB.D3. What an AudioBox restored with its sound gone plays** (§4): its default sound, the path kept, as P2.50's
  pictures and plan 3's P3.D5. Basis: a ruling, 2026-10-28 ("go ahead").

## 9. Claims to check and open questions

- **The counts of §1** (EB.1): taken on `02-master-fixes` before plan 2 landed; recounted on the branch.
- **Whether Windows ends the process too** for a throw through Qt's frames: not checked; the rule holds either way.

## 10. Risks

| Risk | Mitigation |
|---|---|
| A throw site the inventory misses | Every site is listed by a search for `throw` and `PANDACEPTION`, and EB.2's check fails on any site the inventory does not hold |
| A fix turns a report into silence | A fallback is shown where the user acted, or a file's open still reports (§2) |
| A site fixed here comes back in a later plan's rewrite | The rule is in this file and plan 3 on keep each fix's test green |

## 11. What other plans take from it

- **Plans 3 to 10:** the rule of §2, and each fix pinned by its test. Plan 3's schema and audio service (P3.D5) keep
  the AudioBox fix; plan 9's editor keeps the commands' guarantee.
