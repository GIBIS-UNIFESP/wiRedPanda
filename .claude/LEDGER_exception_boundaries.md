# Ledger: exception boundaries

Tracks the items of `.claude/PLAN_exception_boundaries.md`. The plan says what and why; this file says where each
item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a fix the inventory finds gets the next free
row (`EB.4`, `EB.5`, ...) and a line in the log. Status: `todo`, `doing`, `done` (committed), `blocked` (on what, in
the notes) or `dropped` (why, with the user's decision). The Commits column lists every commit of the item. Every
status change gets a dated line in the log.

**Branch:** `02c-exception-boundaries`, from `02b-qt-ceiling` at `10ea721bb`, created 2026-10-29.

**Open decisions:** none.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| EB.1 | The inventory of §3: every throw, catch and `guardedSlot()` site, its frames and its class | doing | | Counts of §1 recounted here. A site whose callers cannot be traced by reading is reached by a probe |
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
- 2026-10-29: the branch created from `02b-qt-ceiling` at `10ea721bb`. EB.1 `doing`: the sites recounted (the
  appendix), the entry paths begun; clangd set up for navigation to trace callers.

## Appendix: the inventory (EB.1)

Taken 2026-10-29 on `02c-exception-boundaries` at the Qt ceiling's tip, with comments and string literals stripped:
135 `throw` sites (124 `PANDACEPTION`s, 11 bare rethrows, one `std::runtime_error`) in 85 functions, 42 `catch` sites
(21 in `App/`, 21 in `MCP/`) and 83 `guardedSlot()` uses. Every `PANDACEPTION` is thrown where it is built.

Classes (the plan's §2): **B**, below a boundary, caught there; **E**, an expected condition, which needs a fallback, a
refusal before anything changes, or a report where the user acted; **I**, a broken invariant, to be an assertion; **Q**,
reaches a frame Qt owns, which breaks the rule whatever its class.

### Entry paths

Each throw site names the paths that reach it. A path is a boundary (it catches before control returns to Qt) or a
frame Qt owns.

| Path | Where control enters | Boundary? |
|---|---|---|
| CLI | `Main.cpp`'s command-line modes, inside its `try` (`Main.cpp:443`) | Yes |
| OPEN-G | Opening a file through a `guardedSlot()`: the Open dialog (`WorkspaceManager::loadPandaFile`'s callers in `WorkspaceManager.cpp:412`), Reload (`:515`), the autosave restore (`loadAutosaveFiles()`, its own catch at `:616`) | Yes |
| OPEN-Q | Opening a file from a frame Qt owns, no catch between: the Examples menu's lambda (`MainWindow.cpp:257`), `openRecentFile()` (connected at `:993`), `SceneUiBinder::loadFileRequested` (`:110`) and `ElementEditor::openSubcircuitFileRequested` (`:387`) connected straight to `loadPandaFile()`, WebAssembly's file-content callback (`WorkspaceManager.cpp:401`), and a `.panda` file dropped on the scene (`Scene::fileDropRequested`, relayed by a lambda at `SceneUiBinder.cpp:169` to `loadFileRequested`) | **No** |
| UNDO | A command's `undo()` or `redo()`: the Undo and Redo actions are connected straight to `QUndoStack` (`Scene.cpp:92`, `:100`; BeWavedDolphin's at `BeWavedDolphin.cpp:112`, `:113`), and `QUndoStack::push()` runs the first `redo()`. QtGui is built without unwinding, so a throw ends the process on macOS (P2.16) even under a `guardedSlot()` | **No** |
| TIMER | `WorkSpace::autosave()`, connected straight to its debounce timer's `timeout` (`Workspace.cpp:79`) | **No** |
| WATCH | `ICRegistry::onFileChanged()`, a queued slot wrapped in `guardedSlot()` (`ICRegistry.cpp:96`), its rethrow at `:126` caught there | Yes |
| DROP | A drop's or a drag's event handler, no catch between: `Scene::dropEvent()` to `SceneDropHandler::handleNewElementDrop()` (builds the element, a file-backed IC loading its file) and `handleCloneDrag()` (deserializes the drag), `TrashButton::dropEvent()` and `ICDropZone`'s `dragEnterEvent()` and `dropEvent()` (`extractDragPayload()`, reading the drag's header). A drop's data can come from another program or another wiRedPanda | **No** |
| PALETTE | The palette's `addElementRequested`, connected to a lambda (`MainWindow.cpp:403`) that calls `Scene::addItem()`, `SceneDropHandler::addFromMimeData()`: builds the element, a file-backed IC loading its file | **No** |
