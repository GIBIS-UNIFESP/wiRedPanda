# Ledger: exception boundaries

Tracks the items of `.claude/PLAN_exception_boundaries.md`. The plan says what and why; this file says where each
item stands.

**Rules** (the plans' rules, §3): one row per item, with the plan's ID; a fix the inventory finds gets the next free
row (`EB.4`, `EB.5`, ...) and a line in the log. Status: `todo`, `doing`, `done` (committed), `blocked` (on what, in
the notes) or `dropped` (why, with the user's decision). The Commits column lists every commit of the item. Every
status change gets a dated line in the log.

**Branch:** `02c-exception-boundaries`, from `02b-qt-ceiling` at `5b783f1f2`, created 2026-10-29.

**Open decisions:** none.

| ID | Item | Status | Commits | Notes |
|---|---|---|---|---|
| EB.1 | The inventory of §3: every throw, catch and `guardedSlot()` site, its frames and its class | done | `be1b42bed`, `3c3d04de2`, `386cc958a`, `432cd6cb8`, `3bf8621f7` | Counts of §1 recounted here. A site whose callers cannot be traced by reading is reached by a probe. The appendix holds it: 135 throw sites, 42 catches, 82 `guardedSlot()` uses; the throw sites' paths walked with clangd. Found: EB.5 to EB.9 |
| EB.2 | The checks that keep the rule (§5): the inventory against the sources, a test per fixed site | todo | | Shown to fail: a `PANDACEPTION` added to a command and a `catch` removed each fail the inventory check |
| EB.3 | An AudioBox restored with its sound gone (§4) | done | `76fd88822` | Read 2026-10-24, not run: `AudioBox::setAudio()` throws on a missing file and a snapshot restore calls it. It plays its default sound, the path kept (EB.D3). Fixed 2026-10-29: `setAudio()` keeps the path of a sound that is gone or unreadable and plays the built-in sound in its place, and plays the sound again once its file is back (`startAudio()`); opening a file still reports it, through `GraphicElement::reportFailedFiles()`, P2.50's `reportFailedPicture()` made virtual so an AudioBox adds its sound; the editor's picker refuses a file it cannot read before anything changes, which the throw did before. Four tests: the box keeps the path, plays the default, reports it, and plays the sound once it is back (`TestAudioBox`, replacing the test that expected a throw); an undo through the stack with the sound gone restores the path and steps (`TestCommands`); the picker's refusal (`TestElementEditor`); opening a file with the sound gone reports it (`TestWorkspace`). The first three failed on the floor's branch, two with an unhandled Pandaception; the last passes there, holding the report. Shown to fail with the fix put back part by part: no report fails the box's and the file's, no refusal the picker's, no return of the sound the box's. Found while fixing: a sound passed back from `m_failedAudioPath` to `setAudio()` was the member it clears, so the path is copied first. The strings golden gains the picker's two strings; the report keeps `Couldn't load audio: %1 (%2)` in its context. Tip checked 2026-10-29: suite 227/227, eight sources compiled alone, three at `-O3`, `fix_style.py` clean. CI at `9559fa4fa` failed the box's test on every job (Build 40887717428, Sanitizers 40887717370): with no output device, which CI's runners have, the box never plays, so a sound back since was still reported. The report now names a sound only while its file still cannot be read; the test passes with and without a device here (a run with PulseAudio and PipeWire unreachable reproduced the failure), folded into the fix |
| EB.4 | A command whose first `redo()` throws leaks, its change half applied | done | `1458eddcf` | Found 2026-10-26 by `fuzz_undo`'s fourth coverage round (plan 2's P2.78), as a leak: an `UpdateCommand` whose snapshot names an IC that no longer resolves throws from `redo()` inside `QUndoStack::push()`, which takes ownership only after `redo()` returns, so the command leaks and what `redo()` did before the throw stays. The harness reached it by removing a blob still in use, which the application never does (it removes one with its ICs, `WorkSpace::removeEmbeddedIC()`); in the application the same throw needs an IC whose file is gone when a command reloads it. Read, not run, in the application. Each command's first `redo()` goes in the inventory (EB.1). Inside an undo macro the same throw also leaves the macro open, every later edit recorded into it, as plan 2's P2.57 (paste) and P2.76 (`createEmbeddedIC()`) did; both now load before the macro opens. The other macros, read 2026-10-27 (plan 2's audit, AU.8): `Workspace.cpp:914`, `PropertyShortcutHandler.cpp:44`, `:145`, `:221`, `:252`, `ElementEditor.cpp:725`, and `:250`, unreachable; none of their commands can throw today (built from live items, loading no file; the shortcuts' `UpdateCommand` reloads only clocks, buzzers, displays and LEDs, never an IC or an AudioBox; a missing picture shows the default since P2.50). Every macro site goes in the inventory with the commands it pushes, so a command that comes to throw closes its macro. Fixed 2026-10-29 for the application's case, an IC whose file is gone (EB.D4): each IC records every IC file its load read, nested ones included (`ICLoader::recordFilesRead()`); an in-memory snapshot carries them (`IC::save()`, the `files` entry, never written to a file), and its restore loads a file deleted since from them (`ICLoader::withCarriedFiles()`), so the undo, the redo and the first redo give back the working IC. Opening a file still reports an IC whose file is gone. The removal from the palette takes the file back itself (EB.10). Test: `TestCommands` deletes an IC whose file holds a file-backed IC, deletes both files, and undoes, redoes and undoes; it failed on the floor's branch (`outer.panda not found.`), and fails with the carried file not served and with the nested file not recorded into the outer IC's (`inner.panda not found.`). A throw from a first `redo()` for any other reason is EB.5's: a broken invariant made an assertion with a fallback. The harness's blob removed while in use stays out of reach of the application. Tip checked 2026-10-29: suite 227/227, three sources compiled alone, two at `-O3` and with GCC 13, `fix_style.py` clean |
| EB.5 | A command's `undo()`, `redo()` or constructor throws: its broken invariants (an id not on the scene, a snapshot short of what it recorded, a split's or a toggle's target gone) become assertions with a fallback that leaves the scene whole, and its snapshots' reads of the application's own bytes the same | todo | | Found by EB.1 (UNDO, BUILD; the commands' rows and every reader's). The external files a restore names are EB.3 and EB.4 |
| EB.6 | Slots and lambdas connected with no catch reach throws: the open path's (the Examples menu, Recent Files, a file dropped on the scene, a subcircuit link, WebAssembly's open), Edit's clipboard actions, the palette's add, a sub-circuit tab's save to its parent, `ICController::removeICFile()`, and the UI slots of the UI path | todo | | Found by EB.1 (OPEN-Q, PASTE, PALETTE, CHILD, UI). Each catches where the user acted and reports there, or refuses before changing anything |
| EB.7 | The autosave timer's `WorkSpace::autosave()` throws when its file cannot be written, or a sub-circuit does not read to embed | todo | | Found by EB.1 (TIMER). An autosave that fails is an expected condition: it reports once without throwing, and the next tick tries again |
| EB.8 | A drop's handlers read the drag's data, build the element and load a file-backed IC's file with no catch: a drop from another program or another wiRedPanda, or an IC whose file is gone, throws in an event handler | todo | | Found by EB.1 (DROP): `Scene::dropEvent()`, `TrashButton::dropEvent()`, `ICDropZone`'s drag handlers, `IC::loadFromDrop()`. The drop is refused before anything changes, and reported |
| EB.9 | Broken invariants reached from the interface throw: `ElementMetadataRegistry::metadata()` for a type with no metadata, `ElementFactory::buildElement()` for an unknown type, `ElementContextMenu::exec()` for an option it does not know | todo | | Found by EB.1 (UI, CREATE, every path to the factory). Each becomes an assertion with the release build's fallback (§2). An element's built-in sound failing to set was a fourth, gone with EB.3: `AudioBox::setAudio()` throws no more |
| EB.10 | An IC file removed from the palette goes to the trash, and its instances are deleted, but undoing it restores only the instances: each loads a file still in the trash and throws out of `undo()` | done | `03fc09257` | Found 2026-10-29 by the user's question while ruling EB.D4: `ICController::removeICFile()` moves the file with `QFile::moveToTrash()` and pushes a `DeleteItemsCommand`; nothing takes the file back. The user's ruling (EB.D4): the removal is one command whose undo takes the file back out of the trash before restoring the instances, and whose redo moves it again. `RemoveICFileCommand`: the controller still moves the file before pushing, so a file it cannot move is refused before anything changes; the command's undo renames it back and removes the freedesktop.org trash's `info/*.trashinfo` record; a file that cannot come back is left to EB.4's fallback. Test: `TestICController` removes, undoes, redoes and undoes again; shown to fail with the file not taken back (the undo threw `dep.panda not found.`, the old behaviour) and with the trash record left. The strings golden gains the command's text. CI's API docs at `0fcfae66d` failed: the constructor documented some of its parameters, so Doxygen requires them all; each documented, Doxygen 1.16.1 clean here, folded into the fix. Tip checked 2026-10-29: suite 227/227, three sources compiled alone, two at `-O3`, `fix_style.py` clean; the inventory's lines in `Commands.cpp` and `ICController.cpp` moved with it |
| EB.D2 | How the rule is kept (§5) | done | | Decided 2026-10-28 by the user: the inventory checked in CI against the sources, and a test through Qt's frame per fixed site; no `noexcept`, since `notify()` stays the last-resort reporter |
| EB.D3 | What an AudioBox restored with its sound gone plays (§4) | done | | Decided 2026-10-28 by the user: its default sound, the path kept, as P2.50's pictures and plan 3's P3.D5 |
| EB.D4 | What an IC restored with its file gone loads (EB.4) | done | | Decided 2026-10-29 by the user. Removing an IC file from the palette is undone with the file taken back out of the trash (EB.10). A file deleted outside the application: the snapshot carries the IC's content and the restore loads it, so the undo gives back the working IC; reopening the circuit still reports the file missing. A placeholder with no internals (the first proposal) was turned down: the user could do nothing with it, and the claim that it would reload when the file came back was wrong, since `ICRegistry::onFileChanged()` drops the watch of a deleted file. Refusing the undo would block the history |

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
- 2026-10-29: EB.1's inventory written (the appendix): every throw, catch and `guardedSlot()` site, the throw
  sites' paths from clangd's call graph. EB.5 to EB.9 found, `todo`.
- 2026-10-29: EB.1 done (`3bf8621f7`).
- 2026-10-29: rebased onto `02b-qt-ceiling` at `5b783f1f2`, the ceiling landed (QC.2 done); the hashes above
  re-mapped.
- 2026-10-29: EB.3 done (`76fd88822`): the box plays its built-in sound; the inventory's AudioBox site moved to
  `reportFailedFiles()`, and EB.9 loses the built-in sound.
- 2026-10-29: EB.D4 decided by the user: an IC restored with its file gone loads the content its snapshot carries.
  EB.10 found asking it, `doing`: undoing an IC file's removal from the palette leaves the file in the trash.
- 2026-10-29: EB.10 done (`03fc09257`).
- 2026-10-29: EB.4 done (`1458eddcf`): a snapshot carries the IC files its ICs read (EB.D4).
- 2026-10-29: CI at `9559fa4fa` and `0fcfae66d`: EB.3's test failed with no output device and EB.10's constructor
  lacked parameter docs; both fixed and folded, the hashes above re-mapped.

## Appendix: the inventory (EB.1)

Taken 2026-10-29 on `02c-exception-boundaries` at the Qt ceiling's tip, with comments and string literals stripped:
135 `throw` sites (124 `PANDACEPTION`s, 11 bare rethrows, one `std::runtime_error`) in 85 functions, 42 `catch` sites
(21 in `App/`, 21 in `MCP/`) and 83 `guardedSlot()` uses. Every `PANDACEPTION` is thrown where it is built.

Classes (the plan's §2): **B**, below a boundary, caught there; **E**, an expected condition, which needs a fallback, a
refusal before anything changes, or a report where the user acted; **I**, a broken invariant, to be an assertion; **Q**,
reaches a frame Qt owns, which breaks the rule whatever its class.

### Entry paths

Each throw site names the paths that reach it. A path is a boundary (it catches before control returns to Qt) or a frame
Qt owns. The paths come from clangd's incoming calls (`build-clangd`'s index), walked up from each throwing function,
each call judged where it stands: inside a `try` whose `catch` does not rethrow, or inside a `guardedSlot()`, it is
covered; at a connection (`&Class::slot`, or a lambda given to `connect()`) it enters through Qt's dispatch, covered
only when that slot is guarded; otherwise the walk goes on to its caller. A call through a virtual `load()` reaches
every override, so a reader's paths are what any element's `load()` can meet: more than some can reach (a BUILD round
trip only reloads nodes), which each fix narrows by reading.

| Path | Where control enters | Boundary? |
|---|---|---|
| CLI | `Main.cpp`'s command-line modes, inside its `try` (`Main.cpp:443`) | Yes |
| OPEN-G | Opening a file through a `guardedSlot()`: the Open dialog (`WorkspaceManager::loadPandaFile`'s callers in `WorkspaceManager.cpp:412`), Reload (`:515`), the autosave restore (`loadAutosaveFiles()`, its own catch at `:616`) | Yes |
| OPEN-Q | Opening a file from a frame Qt owns, no catch between: the Examples menu's lambda (`MainWindow.cpp:257`), `openRecentFile()` (connected at `:993`), `SceneUiBinder::loadFileRequested` (`:110`) and `ElementEditor::openSubcircuitFileRequested` (`:387`) connected straight to `loadPandaFile()`, WebAssembly's file-content callback (`WorkspaceManager.cpp:401`), and a `.panda` file dropped on the scene (`Scene::fileDropRequested`, relayed by a lambda at `SceneUiBinder.cpp:169` to `loadFileRequested`) | **No** |
| UNDO | A command's `undo()` or `redo()`: the Undo and Redo actions are connected straight to `QUndoStack` (`Scene.cpp:92`, `:100`; BeWavedDolphin's at `BeWavedDolphin.cpp:112`, `:113`), and `QUndoStack::push()` runs the first `redo()`. QtGui is built without unwinding, so a throw ends the process on macOS (P2.16) even under a `guardedSlot()` | **No** |
| TIMER | `WorkSpace::autosave()`, connected straight to its debounce timer's `timeout` (`Workspace.cpp:79`) | **No** |
| WATCH | `ICRegistry::onFileChanged()`, a queued slot wrapped in `guardedSlot()` (`ICRegistry.cpp:96`), its rethrow at `:126` caught there | Yes |
| DROP | A drop's or a drag's event handler, no catch between: `Scene::dropEvent()` to `SceneDropHandler::handleNewElementDrop()` (builds the element, a file-backed IC loading its file) and `handleCloneDrag()` (deserializes the drag), `TrashButton::dropEvent()` and `ICDropZone`'s `dragEnterEvent()` and `dropEvent()` (`extractDragPayload()`, reading the drag's header). An IC dropped from the palette loads its file (`IC::loadFromDrop()`). A drop's data can come from another program or another wiRedPanda | **No** |
| PALETTE | The palette's `addElementRequested`, connected to a lambda (`MainWindow.cpp:403`) that calls `Scene::addItem()`, `SceneDropHandler::addFromMimeData()`: builds the element, a file-backed IC loading its file | **No** |
| PASTE | Edit's Paste and Duplicate (and Copy, Cut, Delete), each connected by `connectSceneAction()` to a lambda with no catch (`MainWindow.cpp:451`) that calls the scene's action: `ClipboardManager::paste()` reads the clipboard, which another program or another wiRedPanda may have written, and deserializes it | **No** |
| BUILD | Building a command, in whatever frame builds it, no catch between: the scene's key, mouse and double-click handlers (`Scene.cpp:1499`, `:1585`, `:1656`), the property shortcuts (`:900` to `:906`), a lambda at `Scene.cpp:151`, the inline label editor's lambdas (`InlineLabelEditor.cpp:73`, `:74`), the element editor's slots that apply an edit, and a main window lambda (`MainWindow.cpp:382`). `UpdateCommand`'s constructor reloads its old and new snapshots when the selection holds a wireless-capable element (a node), and `SplitCommand`'s checks its wire | **No** |
| CHILD | `WorkSpace::onChildICBlobSaved()`, connected straight to a sub-circuit tab's `icBlobSaved` (`WorkspaceManager.cpp:363`), and `openICInTab()`, which saves through it: its rethrow (`Workspace.cpp:869`) crosses Qt's signal dispatch | **No** |
| UI | Other slots and handlers with no catch that reach a throw: the main window's `retranslateUi()`, `updateTheme()` and `onCurrentTabChanged()` and lambdas at `MainWindow.cpp:1400` and `:1480`, the element editor's `contextMenu()` and `selectionChanged()`, `ElementTabNavigator`'s event filter, and `ICController::removeICFile()` | **No** |
| CREATE | An element's constructor setting its built-in sound (`AudioBox`, `startAudio()`) or reading its metadata (`Buzzer`, every element through `GraphicElement`'s constructor) | Its caller's |
| TEST-ONLY | `ArduinoCodeGen::generateTestbench()`, which only the tests call | n/a |
| MCP | A request, inside `BaseHandler::tryCommand()` (`BaseHandler.h:57`) or the processor's own catches. A command pushed there runs its first `redo()` inside `QUndoStack::push()`, so a throw from it crosses QtGui before the catch: UNDO's case | Yes, except through a command |

### Throw sites

| Site | In | Paths | Class | Disposition |
|---|---|---|---|---|
| `App/Scene/Commands.cpp:148` | `CommandUtils::findItems()`: an id the command recorded is not on the scene | UNDO | I | Q: an assertion with a fallback (EB.5) |
| `App/Scene/Commands.cpp:166` | `CommandUtils::findElements()`: the same, for elements | UNDO | I | Q: as `:148` (EB.5) |
| `App/Scene/Commands.cpp:233` | `CommandUtils::loadItems()`: a snapshot restored fewer items than it recorded | UNDO | I | Q: as `:148` (EB.5). The snapshot's own throws (`deserialize()` and what it loads) are listed at their sites |
| `App/Scene/Commands.cpp:786` | `SplitCommand`'s constructor: the wire has no start or end port | UNDO (its constructor runs before `push()`, from `SceneInteraction::mouseDoubleClick()`, an event handler, and MCP's split) | I | Q: as `:148` (EB.5); both callers pass a wire with both ports |
| `App/Scene/Commands.cpp:792` | `SplitCommand`'s constructor: a port without its element | UNDO, as `:786` | I | Q: as `:148` (EB.5); every port has its element (`ElementPorts::addPort()`) |
| `App/Scene/Commands.cpp:831` | `SplitCommand::redo()`: a wire or element it recorded is gone | UNDO | I | Q: as `:148` (EB.5) |
| `App/Scene/Commands.cpp:836` | `SplitCommand::redo()`: the wire has no end port | UNDO | I | Q: as `:148` (EB.5) |
| `App/Scene/Commands.cpp:883` | `SplitCommand::undo()`: a wire, node or element it recorded is gone | UNDO | I | Q: as `:148` (EB.5) |
| `App/Scene/Commands.cpp:1318` | `ToggleTruthTableOutputCommand::redo()`: the truth table it recorded is gone | UNDO | I | Q: as `:148` (EB.5) |
| `App/Scene/Commands.cpp:1325` | `ToggleTruthTableOutputCommand::redo()`: the position is outside the key | UNDO | I | Q: as `:148` (EB.5); both callers bound it first (`ElementEditor.cpp:944`, MCP's `ElementHandler.cpp:921`) |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:487` | `BewavedDolphin::saveToTxt()`: a waveform whose circuit changed | MCP | E | B: stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:612` | `BewavedDolphin::on_actionSetClockWave_triggered()`: no cells selected | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinExporter.cpp:70` | `exportToPdf()`: a PDF that cannot be written | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:26` | `save()`: a waveform file that cannot be written | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:43` | `save()`: a waveform file that cannot be written | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:52` | `load()`: a waveform file that does not read | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:57` | `load()`: a waveform file that does not read | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:72` | `load()`: a waveform file that does not read | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:87` | `parseTerminal()`: a waveform text file that does not parse | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:94` | `parseTerminal()`: a waveform text file that does not parse | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:103` | `parseTerminal()`: a waveform text file that does not parse | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinFile.cpp:116` | `parseTerminal()`: a waveform text file that does not parse | boundaries only | E | B: stays |
| `App/BeWavedDolphin/DolphinModelBuilder.cpp:65` | `collect()`: a circuit with no inputs or outputs | MCP | E | B: stays |
| `App/BeWavedDolphin/DolphinModelBuilder.cpp:97` | `collect()`: a circuit with no inputs or outputs | MCP | E | B: stays |
| `App/BeWavedDolphin/DolphinModelBuilder.cpp:101` | `collect()`: a circuit with no inputs or outputs | MCP | E | B: stays |
| `App/BeWavedDolphin/DolphinModelBuilder.cpp:105` | `collect()`: a circuit with no inputs or outputs | MCP | E | B: stays |
| `App/BeWavedDolphin/Serializer.cpp:36` | `loadBinary()`: a waveform file that does not read | boundaries only | E | B: stays |
| `App/BeWavedDolphin/Serializer.cpp:44` | `loadBinary()`: a waveform file that does not read | boundaries only | E | B: stays |
| `App/BeWavedDolphin/Serializer.cpp:92` | `loadCSV()`: a waveform CSV that does not parse | boundaries only | E | B: stays |
| `App/BeWavedDolphin/Serializer.cpp:99` | `loadCSV()`: a waveform CSV that does not parse | boundaries only | E | B: stays |
| `App/BeWavedDolphin/Serializer.cpp:107` | `loadCSV()`: a waveform CSV that does not parse | boundaries only | E | B: stays |
| `App/BeWavedDolphin/Serializer.cpp:117` | `loadCSV()`: a waveform CSV that does not parse | boundaries only | E | B: stays |
| `App/CodeGen/ArduinoCodeGen.cpp:31` | `ArduinoCodeGen::ArduinoCodeGen()`: a file that cannot be opened | MCP | E | B: stays |
| `App/CodeGen/ArduinoCodeGen.cpp:197` | `ArduinoCodeGen::generate()`: rethrow (catch at `:194`) | MCP | — | B: stays |
| `App/CodeGen/ArduinoCodeGen.cpp:783` | `ArduinoCodeGen::assignVariablesRec()`: a circuit the generator cannot express | MCP, TEST-ONLY | E | B: stays |
| `App/CodeGen/ArduinoCodeGen.cpp:1012` | `ArduinoCodeGen::emitTruthTable()`: a circuit the generator cannot express | MCP, TEST-ONLY | E | B: stays |
| `App/CodeGen/ArduinoCodeGen.cpp:1092` | `ArduinoCodeGen::throwPinOverflow()`: more pins than the board has | MCP | E | B: stays |
| `App/CodeGen/ArduinoCodeGen.cpp:1304` | `ArduinoCodeGen::generateTestbench()`: a testbench file that cannot be opened, or the rethrow at `:1439` | TEST-ONLY | — | B: stays |
| `App/CodeGen/ArduinoCodeGen.cpp:1439` | `ArduinoCodeGen::generateTestbench()`: a testbench file that cannot be opened, or the rethrow at `:1439` | TEST-ONLY | — | B: stays |
| `App/CodeGen/SystemVerilogCodeGen.cpp:39` | `SystemVerilogCodeGen::SystemVerilogCodeGen()`: a file that cannot be opened | MCP | E | B: stays |
| `App/CodeGen/SystemVerilogCodeGen.cpp:1212` | `SystemVerilogCodeGen::assignVariablesRec()`: a circuit the generator cannot express | MCP | E | B: stays |
| `App/CodeGen/SystemVerilogCodeGen.cpp:1270` | `SystemVerilogCodeGen::assignVariablesRec()`: a circuit the generator cannot express | MCP | E | B: stays |
| `App/Element/ElementAppearance.cpp:338` | `ElementAppearance::reportFailedPicture()`: a picture that does not load, reported when a file opens (P2.50 made a restore fall back) | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/ElementFactory.cpp:118` | `ElementFactory::buildElement()`: an unknown element type (a stream's type is checked before) | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | I | Q: an assertion with a fallback (EB.9) |
| `App/Element/ElementFactory.cpp:124` | `ElementFactory::buildElement()`: an unknown element type (a stream's type is checked before) | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | I | Q: an assertion with a fallback (EB.9) |
| `App/Element/ElementMetadata.cpp:30` | `ElementMetadataRegistry::metadata()`: a type with no metadata | BUILD, CHILD, CREATE, DROP, MCP, OPEN-Q, PALETTE, PASTE, TEST-ONLY, UI, UNDO | I | Q: an assertion with a fallback (EB.9) |
| `App/Element/GraphicElementSerializer.cpp:48` | `GraphicElementSerializer::readPortList()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:52` | `GraphicElementSerializer::readPortList()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:62` | `GraphicElementSerializer::readPortList()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:271` | `GraphicElementSerializer::loadNewFormat()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:425` | `GraphicElementSerializer::loadNewFormat()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:433` | `GraphicElementSerializer::loadNewFormat()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:487` | `GraphicElementSerializer::validateFinitePos()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:505` | `GraphicElementSerializer::validateFiniteAngle()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:521` | `GraphicElementSerializer::validatePortCounts()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:528` | `GraphicElementSerializer::validatePortCounts()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:591` | `GraphicElementSerializer::loadInputPorts()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:626` | `GraphicElementSerializer::loadOutputPorts()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:692` | `GraphicElementSerializer::loadPixmapAppearanceNames()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElementSerializer.cpp:721` | `GraphicElementSerializer::loadPixmapAppearanceName()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElements/AudioBox.cpp:166` | `AudioBox::reportFailedFiles()`: a sound that is gone or unreadable, reported when a file opens (EB.3 made `setAudio()` fall back, where this site was, at line 81) | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/GraphicElements/Demux.cpp:243` | `Demux::load()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/IC.cpp:157` | `IC::load()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/ICLoader.cpp:148` | `ICLoader::loadFile()`: an IC's file that is gone or does not read: a snapshot restore reaches it (EB.4) | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/ICLoader.cpp:191` | `ICLoader::loadFileDirectly()`: an IC's file that is gone or does not read: a snapshot restore reaches it (EB.4) | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/ICLoader.cpp:202` | `ICLoader::loadFileDirectly()`: an IC's file that is gone or does not read: a snapshot restore reaches it (EB.4) | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/ICLoader.cpp:209` | `ICLoader::loadFileDirectly()`: an IC's file that is gone or does not read: a snapshot restore reaches it (EB.4) | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/ICLoader.cpp:293` | `ICLoader::migrateFile()`: an IC's file that cannot be migrated or backed up | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/ICLoader.cpp:302` | `ICLoader::migrateFile()`: an IC's file that cannot be migrated or backed up | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Element/ICLoader.cpp:371` | `ICLoader::deserializeAndLoad()`: an IC's file that does not read | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/FileUtils.h:37` | `copyToDir()`: a file that cannot be copied | MCP, OPEN-Q | E | Q: OPEN-Q (EB.6) |
| `App/IO/Serialization.cpp:43` | `readVersionNumber()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:89` | `readBoundedString()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:107` | `readBoundedByteArray()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:126` | `readBoundedStringList()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:142` | `readBoundedKeySequence()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:156` | `readBoundedBitArray()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:226` | `readBoundedVariant()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:257` | `readBoundedVariant()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:283` | `Serialization::readBoundedMetadata()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:286` | `Serialization::readBoundedMetadata()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:303` | `Serialization::readBoundedBlobMap()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:306` | `Serialization::readBoundedBlobMap()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:345` | `Serialization::readPayload()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:356` | `Serialization::readPayload()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:387` | `Serialization::inflatePayload()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:399` | `Serialization::readPandaHeader()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:435` | `Serialization::readPandaHeader()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:446` | `Serialization::readPandaHeader()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:453` | `Serialization::readPandaHeader()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:474` | `Serialization::readDolphinHeader()`: a waveform file or clipboard that does not read | boundaries only | E | B: stays |
| `App/IO/Serialization.cpp:494` | `Serialization::readDolphinHeader()`: a waveform file or clipboard that does not read | boundaries only | E | B: stays |
| `App/IO/Serialization.cpp:499` | `Serialization::readDolphinHeader()`: a waveform file or clipboard that does not read | boundaries only | E | B: stays |
| `App/IO/Serialization.cpp:517` | `Serialization::readDolphinHeader()`: a waveform file or clipboard that does not read | boundaries only | E | B: stays |
| `App/IO/Serialization.cpp:630` | `Serialization::deserialize()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:642` | `Serialization::deserialize()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:654` | `Serialization::deserialize()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:685` | `Serialization::deserialize()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:700` | `Serialization::deserialize()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:753` | `Serialization::createVersionedBackup()`: a backup that cannot be written | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:782` | `Serialization::readPreamble()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/IO/Serialization.cpp:846` | `Serialization::copyPandaFile()`: a file that cannot be copied | MCP, OPEN-Q | E | Q: OPEN-Q (EB.6) |
| `App/IO/Serialization.cpp:865` | `Serialization::copyPandaFile()`: a file that cannot be copied | MCP, OPEN-Q | E | Q: OPEN-Q (EB.6) |
| `App/Scene/ICRegistry.cpp:126` | `ICRegistry::onFileChanged()`: rethrow (catch at `:123`) | boundaries only | — | B: stays |
| `App/Scene/ICRegistry.cpp:286` | `ICRegistry::embedICsByFile()`: rethrow (catch at `:284`) | boundaries only | — | B: stays |
| `App/Scene/ICRegistry.cpp:306` | `ICRegistry::extractToFile()`: a file that cannot be written | boundaries only | E | B: stays |
| `App/Scene/ICRegistry.cpp:313` | `ICRegistry::extractToFile()`: a file that cannot be written | boundaries only | E | B: stays |
| `App/Scene/ICRegistry.cpp:370` | `ICRegistry::reloadTargetsAtomically()`: rethrow (catch at `:368`) | boundaries only | — | B: stays |
| `App/Scene/ICRegistry.cpp:378` | `ICRegistry::makeBlobSelfContained()`: a sub-circuit that does not read | MCP, OPEN-Q, PASTE, UNDO | E | Q: OPEN-Q (EB.6); PASTE (EB.6); UNDO (EB.5) |
| `App/Scene/Workspace.cpp:293` | `WorkSpace::save()`: a file that cannot be written, or a sub-circuit that does not read to embed | MCP, OPEN-Q | E | Q: OPEN-Q (EB.6) |
| `App/Scene/Workspace.cpp:374` | `WorkSpace::save()`: a file that cannot be written, or a sub-circuit that does not read to embed | MCP, OPEN-Q | E | Q: OPEN-Q (EB.6) |
| `App/Scene/Workspace.cpp:398` | `WorkSpace::save()`: a file that cannot be written, or a sub-circuit that does not read to embed | MCP, OPEN-Q | E | Q: OPEN-Q (EB.6) |
| `App/Scene/Workspace.cpp:481` | `WorkSpace::load()`: a file that is gone or does not open | MCP, OPEN-Q | E | Q: OPEN-Q (EB.6) |
| `App/Scene/Workspace.cpp:490` | `WorkSpace::load()`: a file that is gone or does not open | MCP, OPEN-Q | E | Q: OPEN-Q (EB.6) |
| `App/Scene/Workspace.cpp:732` | `WorkSpace::autosave()`: an autosave file that cannot be written | TIMER | E | Q: TIMER (EB.7) |
| `App/Scene/Workspace.cpp:743` | `WorkSpace::autosave()`: an autosave file that cannot be written | TIMER | E | Q: TIMER (EB.7) |
| `App/Scene/Workspace.cpp:869` | `WorkSpace::onChildICBlobSaved()`: rethrow (catch at `:866`) | CHILD | — | Q: CHILD (EB.6) |
| `App/UI/CircuitExporter.cpp:39` | `renderToPdf()`: an export that cannot be written | boundaries only | E | B: stays |
| `App/UI/CircuitExporter.cpp:75` | `renderScaledImage()`: an image too large to render | boundaries only | E | B: stays |
| `App/UI/CircuitExporter.cpp:90` | `renderToImage()`: an image that cannot be written | boundaries only | E | B: stays |
| `App/UI/ElementContextMenu.cpp:250` | `ElementContextMenu::exec()`: a menu option it does not know | UI | I | Q: an assertion with a fallback (EB.9) |
| `App/UI/ExportController.cpp:38` | `ExportController::exportToArduino()`: an export that cannot be written | MCP | E | B: stays |
| `App/UI/ExportController.cpp:44` | `ExportController::exportToArduino()`: an export that cannot be written | MCP | E | B: stays |
| `App/UI/ExportController.cpp:70` | `ExportController::exportToSystemVerilog()`: an export that cannot be written | MCP | E | B: stays |
| `App/UI/ExportController.cpp:76` | `ExportController::exportToSystemVerilog()`: an export that cannot be written | MCP | E | B: stays |
| `App/UI/ExportController.cpp:95` | `ExportController::exportToWaveFormFile()`: an export that cannot be written | boundaries only | E | B: stays |
| `App/UI/ICController.cpp:70` | `ICController::runAsOneStep()`: rethrow (catch at `:67`) | boundaries only | — | B: stays |
| `App/UI/ICController.cpp:168` | `ICController::removeICFile()`: a file the trash refuses | UI | E | Q: UI (EB.6) |
| `App/UI/ICController.cpp:278` | `ICController::embedSelectedIC()`: rethrow (catch at `:269`) | boundaries only | — | B: stays |
| `App/UI/ICController.cpp:337` | `ICController::extractSelectedIC()`: rethrow (catch at `:328`) | boundaries only | — | B: stays |
| `App/UI/ICController.cpp:484` | `ICController::makeSelfContained()`: rethrow (catch at `:475`) | boundaries only | — | B: stays |
| `App/UI/WorkspaceManager.cpp:310` | `WorkspaceManager::loadPandaFile()`: rethrow (catch at `:304`) | MCP, OPEN-Q | — | Q: OPEN-Q (EB.6) |
| `App/Wiring/ConnectionSerializer.cpp:40` | `ConnectionSerializer::load()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `App/Wiring/ConnectionSerializer.cpp:50` | `ConnectionSerializer::load()`: a stream that does not read: a file, the clipboard, a drag, an IC's file; from UNDO and BUILD, the snapshot is the application's own | BUILD, CHILD, DROP, MCP, OPEN-Q, PALETTE, PASTE, UI, UNDO | E; I from UNDO and BUILD | Q: BUILD (EB.5); CHILD (EB.6); DROP (EB.8); OPEN-Q (EB.6); PALETTE (EB.6); PASTE (EB.6); UI (EB.6); UNDO (EB.5) |
| `MCP/Server/Core/MCPValidator.cpp:291` | `MCPValidator::qjsonToNlohmann()`: translated for its callers (catch at `:290`) | boundaries only | — | B: stays |

### Catch sites

| Site | In | Catches | Disposition |
|---|---|---|---|
| `MCP/Server/Core/MCPProcessor.cpp:361` | `MCPProcessor::processCommand()` | Any `std::exception` from a request's handler: an error response | Stays: MCP's boundary |
| `MCP/Server/Core/MCPProcessor.cpp:364` | `MCPProcessor::processCommand()` | Anything else: an error response | Stays: MCP's boundary |
| `MCP/Server/Core/MCPValidator.cpp:58` | `MCPValidator::loadSchema()` | The schema's JSON parse error: `false` | Stays: a fallback |
| `MCP/Server/Core/MCPValidator.cpp:60` | `MCPValidator::loadSchema()` | nlohmann's other errors: `false` | Stays: a fallback |
| `MCP/Server/Core/MCPValidator.cpp:69` | `MCPValidator::loadSchema()` | The schema validator's errors: `false` | Stays: a fallback |
| `MCP/Server/Core/MCPValidator.cpp:88` | `MCPValidator::validateCommand()` | A malformed command: an invalid result | Stays: a refusal |
| `MCP/Server/Core/MCPValidator.cpp:123` | `MCPValidator::validateResponse()` | A malformed response: an invalid result | Stays: a refusal |
| `MCP/Server/Core/MCPValidator.cpp:208` | `MCPValidator::validateAgainstSchema()` | The validator's error: an invalid result with its message | Stays: a refusal |
| `MCP/Server/Core/MCPValidator.cpp:290` | `MCPValidator::qjsonToNlohmann()` | nlohmann's parse error, rethrown as `std::runtime_error` (`:291`) | Stays: translates for its callers, `:88` and `:123` |
| `MCP/Server/Handlers/BaseHandler.h:59` | `BaseHandler::tryCommand()` | Any `std::exception` from a handler's body: an error response | Stays: MCP's boundary; it cannot catch a command's first `redo()` on macOS (the MCP path) |
| `MCP/Server/Handlers/BaseHandler.h:62` | `BaseHandler::tryCommand()` | Anything else: an error response | Stays, as `:59` |
| `MCP/Server/Handlers/ConnectionHandler.cpp:109` | `ConnectionHandler::handleConnectElements()` | A throw from pushing the wire's `AddItemsCommand`: an error response | Stays, as `BaseHandler.h:59`: its body is a command's first `redo()`, which must not throw (EB.5) |
| `MCP/Server/Handlers/ConnectionHandler.cpp:112` | `ConnectionHandler::handleConnectElements()` | Anything else, as `:109` | Stays, as `:109` |
| `MCP/Server/Handlers/ElementHandler.cpp:255` | `ElementHandler::handleCreateElement()` | `ElementFactory::buildElement()`'s throw: an error response | Stays: a refusal before anything changes |
| `MCP/Server/Handlers/ElementHandler.cpp:258` | `ElementHandler::handleCreateElement()` | Anything else, as `:255` | Stays |
| `MCP/Server/Handlers/ElementHandler.cpp:288` | `ElementHandler::handleCreateElement()` | A throw from pushing the element's `AddItemsCommand`: the element deleted, an error response | Stays, as `ConnectionHandler.cpp:109` |
| `MCP/Server/Handlers/ElementHandler.cpp:292` | `ElementHandler::handleCreateElement()` | Anything else, as `:288` | Stays, as `:288` |
| `MCP/Server/Handlers/ElementHandler.cpp:430` | `ElementHandler::handleMoveElement()` | A throw from pushing the `MoveCommand`: the position put back, an error response | Stays, as `ConnectionHandler.cpp:109` |
| `MCP/Server/Handlers/ElementHandler.cpp:434` | `ElementHandler::handleMoveElement()` | Anything else, as `:430` | Stays, as `:430` |
| `MCP/Server/Handlers/ICHandler.cpp:272` | `ICHandler::handleListICs()` | A listed IC file that does not load (`IC::loadFile()`): listed as without a valid definition | Stays: a fallback |
| `MCP/Server/Handlers/ICHandler.cpp:283` | `ICHandler::handleListICs()` | A listed file whose header does not read: skipped | Stays: a fallback |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:533` | `BewavedDolphin::exportToPng()` | Anything from `DolphinExporter::exportToPng()`, which throws nothing today: `false` | Stays: a refusal its callers report |
| `App/CodeGen/ArduinoCodeGen.cpp:194` | `ArduinoCodeGen::generate()` | Anything: the partial file removed, rethrown (`:197`) | Stays: cleans up for the export's boundary (EXPORT) |
| `App/CodeGen/ArduinoCodeGen.cpp:1436` | `ArduinoCodeGen::generateTestbench()` | Anything: the stream restored, the file closed, rethrown (`:1439`) | Stays, as `:194` |
| `App/Core/Application.cpp:110` | `Application::notify()` | Any `std::exception` from an event's dispatch: reported | Stays: the last-resort reporter (§2); nothing relies on it |
| `App/Core/Application.h:141` | `Application::guardedSlot()` | Any `std::exception` from the slot's body: reported | Stays: the boundary of every guarded slot |
| `App/Element/ICLoader.cpp:490` | `ICLoader::renderPreview()` | A definition that no longer loads: no preview | Stays: a fallback |
| `App/IO/Serialization.cpp:911` | `Serialization::copyPandaFile()` | A copied file whose preamble does not read: its sub-circuits not followed | Stays: a fallback |
| `App/Main.cpp:443` | `main()` | Any `std::exception` from the command-line modes: a message box or stderr | Stays: CLI's boundary |
| `App/Scene/ClipboardManager.cpp:140` | `ClipboardManager::paste()` | A blob registry that does not read: pasted without it | Stays: a fallback; the items' own read refuses the paste when they need a missing blob |
| `App/Scene/ClipboardManager.cpp:144` | `ClipboardManager::paste()` | The legacy registry, as `:140` | Stays, as `:140` |
| `App/Scene/ICRegistry.cpp:123` | `ICRegistry::onFileChanged()` | A reload that fails: the scene marked for update, `definitionChanged` emitted, rethrown (`:126`) | Stays: rethrows to its `guardedSlot()` (WATCH) |
| `App/Scene/ICRegistry.cpp:284` | `ICRegistry::embedICsByFile()` | A reload that fails: the new blob removed, rethrown (`:286`) | Stays: rethrows to the IC controller's guarded slots |
| `App/Scene/ICRegistry.cpp:368` | `ICRegistry::reloadTargetsAtomically()` | A target that does not reload: the updated ones rolled back, rethrown (`:370`) | Stays: rolls back for its callers, `:123`, `:284` and the extraction |
| `App/Scene/Workspace.cpp:866` | `WorkSpace::onChildICBlobSaved()` | A target that does not reload: rolled back, the blob put back, rethrown (`:869`) | Q: the rethrow leaves a slot connected straight to the child tab's `icBlobSaved` (`WorkspaceManager.cpp:363`), through Qt's signal dispatch (EB.6) |
| `App/UI/ICController.cpp:67` | `ICController::runAsOneStep()` | A throw part way through the edits: those applied pushed as one step, rethrown (`:70`) | Stays: rethrows to the IC controller's guarded slots |
| `App/UI/ICController.cpp:276` | `ICController::embedSelectedIC()` | Anything: the palette refreshed, rethrown (`:271`) | Stays: inside its `guardedSlot()` |
| `App/UI/ICController.cpp:335` | `ICController::extractSelectedIC()` | Anything: the palette refreshed, rethrown (`:330`) | Stays: inside its `guardedSlot()` |
| `App/UI/ICController.cpp:482` | `ICController::makeSelfContained()` | Anything: the embedded list refreshed, rethrown (`:477`) | Stays: inside its `guardedSlot()` |
| `App/UI/WorkspaceManager.cpp:304` | `WorkspaceManager::loadPandaFile()` | A load that fails: the half-loaded tab closed, rethrown (`:310`) | Stays: cleans up for its callers; OPEN-Q's callers have no catch (EB.6) |
| `App/UI/WorkspaceManager.cpp:616` | `WorkspaceManager::loadAutosaveFiles()` | An autosave that does not load: reported, dropped from the list | Stays: OPEN-G's boundary for autosaves |
| `App/UI/WorkspaceManager.cpp:714` | `WorkspaceManager::closeTab()` | A save on close that fails: reported, then asked whether to close anyway | Stays: a boundary where the user acted |

### `guardedSlot()` uses

Each is the boundary of the slot whose body it wraps, below Qt's frames, so it covers the body whatever calls the slot.
It does not cover a command the body pushes, directly or through the scene: that command's first `redo()` runs inside
`QUndoStack::push()` (UNDO), so the commands themselves must not throw (EB.5). `Application.h:137` is its definition.

| Site | Slot | Disposition |
|---|---|---|
| `App/BeWavedDolphin/BeWavedDolphin.cpp:339` | `BewavedDolphin::on_actionExit_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:579` | `BewavedDolphin::on_actionSetTo0_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:588` | `BewavedDolphin::on_actionSetTo1_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:597` | `BewavedDolphin::on_actionInvert_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:606` | `BewavedDolphin::on_actionSetClockWave_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:644` | `BewavedDolphin::on_actionCombinational_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:664` | `BewavedDolphin::on_actionSetLength_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:712` | `BewavedDolphin::on_actionZoomOut_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:721` | `BewavedDolphin::on_actionZoomIn_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:730` | `BewavedDolphin::on_actionResetZoom_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:745` | `BewavedDolphin::on_actionFitScreen_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:754` | `BewavedDolphin::on_actionClear_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:770` | `BewavedDolphin::on_actionAutoCrop_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:779` | `BewavedDolphin::on_actionCopy_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:794` | `BewavedDolphin::on_actionCut_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:812` | `BewavedDolphin::on_actionPaste_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:829` | `BewavedDolphin::on_actionSave_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:844` | `BewavedDolphin::on_actionSaveAs_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:908` | `BewavedDolphin::on_actionLoad_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:971` | `BewavedDolphin::on_actionShowNumbers_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:982` | `BewavedDolphin::on_actionShowWaveforms_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:991` | `BewavedDolphin::on_actionExportToPng_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:1009` | `BewavedDolphin::on_actionExportToPdf_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:1027` | `BewavedDolphin::on_actionAbout_triggered()` | Stays |
| `App/BeWavedDolphin/BeWavedDolphin.cpp:1046` | `BewavedDolphin::on_actionAboutQt_triggered()` | Stays |
| `App/Scene/ICRegistry.cpp:96` | `ICRegistry::onFileChanged()` | Stays |
| `App/Simulation/Simulation.cpp:39` | `Simulation::Simulation()` | Stays |
| `App/UI/ExportController.cpp:114` | `ExportController::exportArduinoDialog()` | Stays |
| `App/UI/ExportController.cpp:136` | `ExportController::exportSystemVerilogDialog()` | Stays |
| `App/UI/ExportController.cpp:158` | `ExportController::exportPdfDialog()` | Stays |
| `App/UI/ExportController.cpp:187` | `ExportController::exportImageDialog()` | Stays |
| `App/UI/ICController.cpp:77` | `ICController::addICFromFile()` | Stays |
| `App/UI/ICController.cpp:136` | `ICController::showRemoveICHint()` | Stays |
| `App/UI/ICController.cpp:228` | `ICController::embedSelectedIC()` | Stays |
| `App/UI/ICController.cpp:290` | `ICController::extractSelectedIC()` | Stays |
| `App/UI/ICController.cpp:349` | `ICController::embedICByFile()` | Stays |
| `App/UI/ICController.cpp:388` | `ICController::extractICByBlobName()` | Stays |
| `App/UI/ICController.cpp:426` | `ICController::makeSelfContained()` | Stays |
| `App/UI/ICController.cpp:501` | `ICController::addEmbeddedICFromFile()` | Stays |
| `App/UI/ICController.cpp:532` | `ICController::removeEmbeddedIC()` | Stays |
| `App/UI/MainWindow.cpp:423` | `MainWindow::setupConnections()` | Stays |
| `App/UI/MainWindow.cpp:485` | `MainWindow::on_actionExit_triggered()` | Stays |
| `App/UI/MainWindow.cpp:539` | `MainWindow::on_actionWires_triggered()` | Stays |
| `App/UI/MainWindow.cpp:549` | `MainWindow::on_actionRotateRight_triggered()` | Stays |
| `App/UI/MainWindow.cpp:558` | `MainWindow::on_actionRotateLeft_triggered()` | Stays |
| `App/UI/MainWindow.cpp:577` | `MainWindow::on_actionAbout_triggered()` | Stays |
| `App/UI/MainWindow.cpp:651` | `MainWindow::on_actionShortcuts_and_Tips_triggered()` | Stays |
| `App/UI/MainWindow.cpp:658` | `MainWindow::on_actionAboutQt_triggered()` | Stays |
| `App/UI/MainWindow.cpp:665` | `MainWindow::on_actionReportTranslationError_triggered()` | Stays |
| `App/UI/MainWindow.cpp:783` | `MainWindow::on_actionSelectAll_triggered()` | Stays |
| `App/UI/MainWindow.cpp:873` | `MainWindow::on_actionGates_triggered()` | Stays |
| `App/UI/MainWindow.cpp:910` | `MainWindow::on_actionZoomIn_triggered()` | Stays |
| `App/UI/MainWindow.cpp:921` | `MainWindow::on_actionZoomOut_triggered()` | Stays |
| `App/UI/MainWindow.cpp:932` | `MainWindow::on_actionResetZoom_triggered()` | Stays |
| `App/UI/MainWindow.cpp:943` | `MainWindow::on_actionZoomToFit_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1094` | `MainWindow::on_actionRestart_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1106` | `MainWindow::on_actionFastMode_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1115` | `MainWindow::on_actionWaveform_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1142` | `MainWindow::on_actionLightTheme_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1150` | `MainWindow::on_actionDarkTheme_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1158` | `MainWindow::on_actionSystemTheme_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1178` | `MainWindow::on_actionFlipHorizontally_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1189` | `MainWindow::on_actionFlipVertically_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1200` | `MainWindow::on_actionAlignLeft_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1211` | `MainWindow::on_actionAlignRight_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1222` | `MainWindow::on_actionAlignTop_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1233` | `MainWindow::on_actionAlignBottom_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1244` | `MainWindow::on_actionAlignHorizontalCenter_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1255` | `MainWindow::on_actionAlignVerticalCenter_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1266` | `MainWindow::on_actionDistributeHorizontally_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1277` | `MainWindow::on_actionDistributeVertically_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1298` | `MainWindow::on_actionFullscreen_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1306` | `MainWindow::on_actionMute_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1319` | `MainWindow::on_actionLabelsUnderIcons_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1328` | `MainWindow::on_actionICPreview_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1336` | `MainWindow::on_actionCheckForUpdates_triggered()` | Stays |
| `App/UI/MainWindow.cpp:1344` | `MainWindow::on_actionShowMinimap_triggered()` | Stays |
| `App/UI/WorkspaceManager.cpp:281` | `WorkspaceManager::newTab()` | Stays |
| `App/UI/WorkspaceManager.cpp:386` | `WorkspaceManager::openFile()` | Stays |
| `App/UI/WorkspaceManager.cpp:419` | `WorkspaceManager::saveFile()` | Stays |
| `App/UI/WorkspaceManager.cpp:462` | `WorkspaceManager::saveFileAs()` | Stays |
| `App/UI/WorkspaceManager.cpp:506` | `WorkspaceManager::reloadFile()` | Stays |
