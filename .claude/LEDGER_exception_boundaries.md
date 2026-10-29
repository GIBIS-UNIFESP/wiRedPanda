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
| PASTE | Edit's Paste and Duplicate (and Copy, Cut, Delete), each connected by `connectSceneAction()` to a lambda with no catch (`MainWindow.cpp:451`) that calls the scene's action: `ClipboardManager::paste()` reads the clipboard, which another program or another wiRedPanda may have written, and deserializes it | **No** |
| MCP | A request, inside `BaseHandler::tryCommand()` (`BaseHandler.h:57`) or the processor's own catches. A command pushed there runs its first `redo()` inside `QUndoStack::push()`, so a throw from it crosses QtGui before the catch: UNDO's case | Yes, except through a command |

### Throw sites

| Site | In | Paths | Class | Disposition |
|---|---|---|---|---|
| `App/Scene/Commands.cpp:145` | `CommandUtils::findItems()`: an id the command recorded is not on the scene | UNDO | I | Q: an assertion with a fallback (EB.5) |
| `App/Scene/Commands.cpp:163` | `CommandUtils::findElements()`: the same, for elements | UNDO | I | Q: as `:145` (EB.5) |
| `App/Scene/Commands.cpp:230` | `CommandUtils::loadItems()`: a snapshot restored fewer items than it recorded | UNDO | I | Q: as `:145` (EB.5). The snapshot's own throws (`deserialize()` and what it loads) are listed at their sites |
| `App/Scene/Commands.cpp:783` | `SplitCommand`'s constructor: the wire has no start or end port | UNDO (its constructor runs before `push()`, from `SceneInteraction::mouseDoubleClick()`, an event handler, and MCP's split) | I | Q: as `:145` (EB.5); both callers pass a wire with both ports |
| `App/Scene/Commands.cpp:789` | `SplitCommand`'s constructor: a port without its element | UNDO, as `:783` | I | Q: as `:145` (EB.5); every port has its element (`ElementPorts::addPort()`) |
| `App/Scene/Commands.cpp:828` | `SplitCommand::redo()`: a wire or element it recorded is gone | UNDO | I | Q: as `:145` (EB.5) |
| `App/Scene/Commands.cpp:833` | `SplitCommand::redo()`: the wire has no end port | UNDO | I | Q: as `:145` (EB.5) |
| `App/Scene/Commands.cpp:880` | `SplitCommand::undo()`: a wire, node or element it recorded is gone | UNDO | I | Q: as `:145` (EB.5) |
| `App/Scene/Commands.cpp:1315` | `ToggleTruthTableOutputCommand::redo()`: the truth table it recorded is gone | UNDO | I | Q: as `:145` (EB.5) |
| `App/Scene/Commands.cpp:1322` | `ToggleTruthTableOutputCommand::redo()`: the position is outside the key | UNDO | I | Q: as `:145` (EB.5); both callers bound it first (`ElementEditor.cpp:943`, MCP's `ElementHandler.cpp:921`) |

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
| `App/Element/ICLoader.cpp:421` | `ICLoader::renderPreview()` | A definition that no longer loads: no preview | Stays: a fallback |
| `App/IO/Serialization.cpp:911` | `Serialization::copyPandaFile()` | A copied file whose preamble does not read: its sub-circuits not followed | Stays: a fallback |
| `App/Main.cpp:443` | `main()` | Any `std::exception` from the command-line modes: a message box or stderr | Stays: CLI's boundary |
| `App/Scene/ClipboardManager.cpp:140` | `ClipboardManager::paste()` | A blob registry that does not read: pasted without it | Stays: a fallback; the items' own read refuses the paste when they need a missing blob |
| `App/Scene/ClipboardManager.cpp:144` | `ClipboardManager::paste()` | The legacy registry, as `:140` | Stays, as `:140` |
| `App/Scene/ICRegistry.cpp:123` | `ICRegistry::onFileChanged()` | A reload that fails: the scene marked for update, `definitionChanged` emitted, rethrown (`:126`) | Stays: rethrows to its `guardedSlot()` (WATCH) |
| `App/Scene/ICRegistry.cpp:284` | `ICRegistry::embedICsByFile()` | A reload that fails: the new blob removed, rethrown (`:286`) | Stays: rethrows to the IC controller's guarded slots |
| `App/Scene/ICRegistry.cpp:368` | `ICRegistry::reloadTargetsAtomically()` | A target that does not reload: the updated ones rolled back, rethrown (`:370`) | Stays: rolls back for its callers, `:123`, `:284` and the extraction |
| `App/Scene/Workspace.cpp:866` | `WorkSpace::onChildICBlobSaved()` | A target that does not reload: rolled back, the blob put back, rethrown (`:869`) | Q: the rethrow leaves a slot connected straight to the child tab's `icBlobSaved` (`WorkspaceManager.cpp:363`), through Qt's signal dispatch (EB.6) |
| `App/UI/ICController.cpp:67` | `ICController::runAsOneStep()` | A throw part way through the edits: those applied pushed as one step, rethrown (`:70`) | Stays: rethrows to the IC controller's guarded slots |
| `App/UI/ICController.cpp:269` | `ICController::embedSelectedIC()` | Anything: the palette refreshed, rethrown (`:271`) | Stays: inside its `guardedSlot()` |
| `App/UI/ICController.cpp:328` | `ICController::extractSelectedIC()` | Anything: the palette refreshed, rethrown (`:330`) | Stays: inside its `guardedSlot()` |
| `App/UI/ICController.cpp:475` | `ICController::makeSelfContained()` | Anything: the embedded list refreshed, rethrown (`:477`) | Stays: inside its `guardedSlot()` |
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
| `App/UI/ICController.cpp:221` | `ICController::embedSelectedIC()` | Stays |
| `App/UI/ICController.cpp:283` | `ICController::extractSelectedIC()` | Stays |
| `App/UI/ICController.cpp:342` | `ICController::embedICByFile()` | Stays |
| `App/UI/ICController.cpp:381` | `ICController::extractICByBlobName()` | Stays |
| `App/UI/ICController.cpp:419` | `ICController::makeSelfContained()` | Stays |
| `App/UI/ICController.cpp:494` | `ICController::addEmbeddedICFromFile()` | Stays |
| `App/UI/ICController.cpp:525` | `ICController::removeEmbeddedIC()` | Stays |
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
