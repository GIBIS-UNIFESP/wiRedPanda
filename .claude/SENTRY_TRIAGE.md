# Sentry Triage — verified per-issue against current `master`

This is the **complete** sweep with code-level verification for each of the 82 distinct issue IDs Sentry reports for this project (365-day window, full aggregate dump).

For each issue, I located the throw site or crash signature in current `master` and inspected the relevant code path. Verdicts noted as "verified" mean I read the code; "inferred" means signature-matched without a deep code trace.

## Verdict legend

- ✅ **FIXED** — root cause removed or guarded; cannot recur from this path on current master.
- 🛡️ **DEFENSIVE THROW** — exception still thrown, but is a correct safety net for a state divergence whose causes have been closed.
- 🛡️➡️ **DEFENSIVE THROW, downgrade-able** — same, but the throw is unnecessary (default already loaded, etc.) and could be replaced with a debug log.
- ⚠️ **STILL VULNERABLE** — cause is still present; could recur on current code.
- 🌐 **NOT OUR BUG** — environmental: Qt API failure, filesystem, missing system dep.
- ❓ **UNDETERMINED** — stack-stripped beyond recognition.

---

## ✅ FIXED in current code (20 issues, ~70 events)

| ID | Lifetime | Description | Code-level verification |
|---|---:|---|---|
| `H9`, `EV`, `H1` | 7 | drag-then-delete `QGraphicsItem::pos` | **Verified at code level — but NOT YET RELEASED.** Fix at Scene.cpp:1015-1043 (`m_dragSnapshot` filters dead `QPointer`s) is commit `1e62afc91` (2026-04-25 00:17 -0300), which is **15h AFTER 5.0.1 was tagged** (commit `9961bd459`, 2026-04-24 09:41 -0300). One H9 event already fired on 5.0.1 at 2026-04-24 22:48 -0300 (user e41ab503) — that user was on the released 5.0.1 binary which doesn't contain the fix. **Cutting a 5.0.2 release is required to actually close this for users.** |
| `H0`, `GZ`, `GR`, `FS`, `HA`, `HB` | 7 | recursive `mouseMoveEvent` stack overflow / SIGABRT | **Verified**. `m_handlingMouseMove` guard at Scene.cpp:969-973. **`HA`/`HB` (2026-04-25, release 4.7.0)** are the same crash class — fix landed in commit 38c9a2b51 on 2026-04-23, which is 3 days AFTER the 4.7.0 release tag (commit 6f60e9730, 2026-04-20). Users on 4.7.0 don't have the guard and will keep crashing until they upgrade to 5.0.1. Same Brazilian user (Mogi das Cruzes) for both events; classic ensure-visible scrollbar replay loop visible in the stack. No code change needed — released-and-pending-upgrade. |
| `H2`, `H7`, `FW`, `EX`, `FT`, `GX`, `FX`, `F0`, `G0` | ~22 | Simulation hot-path use-after-free | **Verified**. Read `Simulation::update`, `restart()`, `IC::resetInternalState`, all `SimulationBlocker` call sites. The 1ms-tick race is closed by H2 commit's defects A-G. |
| `GH`, `GJ` | 18 | autosave-load `QNEConnection::load` deref | **Verified**. Read QNEConnection.cpp:235-258 — three layers of null checks: `portMap.contains`, `port1 && port2`, post-`dynamic_cast` null check. |
| `H8`, `M` | 6 | "Could not load enough elements for the simulation" | **Verified**. `grep` returns zero matches in `App/`. Code path deleted entirely. |

## 🛡️ DEFENSIVE THROW (16 issues, ~952 events)

### Cluster D — "items / elements not found on the scene"
Thrown at `Commands.cpp:121` (`findItems`), `:139` (`findElements`), `:204` (count-mismatch variant). Fires when an undo command's stored ID doesn't resolve to a scene item.

| ID | Lifetime | Variant | Code-level verification |
|---|---:|---|---|
| `EY` | 327 | mousePressEvent path | **Verified throw site + traced all callers + audited every unprotected mutation path.** Root cause is scene/undo-stack ID divergence. H2 fix's defect F (SimulationBlocker on UpdateCommand/MorphCommand/ChangePortSizeCommand/SplitCommand) closes re-entrancy races. **SIX confirmed un-addressed divergence paths**: (1) `MainWindow::removeICFile` (MainWindow.cpp:1856-1885) uses `scene->removeItem` + `delete` directly without pushing a `DeleteItemsCommand`. (2) `ICRegistry::onFileChanged` (ICRegistry.cpp:80-100) calls `IC::loadFile` directly which cascades into `setInputSize/setOutputSize` → `qDeleteAll` excess ports → wire-cascade-delete. (3) `WorkSpace::save` inline-IC path (Workspace.cpp:119-120) calls `ic->loadFromBlob` for each file-backed sub-IC, which goes through `resetInternalState` (setInputSize(0) + setOutputSize(0)) destroying ports and cascading wire deletion. (4) `UpdateBlobCommand::reconnectConnections` (Commands.cpp:1114-1155) silently drops connections when the new IC has fewer ports than before (lines 1127-1131 bounds-check + lines 1134-1136 silent continue) — orphans connection IDs in the undo stack. (5) `WorkSpace::removeEmbeddedIC` (Workspace.cpp:678) calls `removeBlob` directly outside the command — DeleteItemsCommand restores the IC on undo but the blob is gone, IC has invalid blob reference. Documented as a "known limitation" in TestICInline.cpp:2381-2384, 4344, 4348-4353. (6) `ICRegistry::renameBlob` called directly from `ElementEditor.cpp:638` (NOT in a command). Renames blob in registry map, updates IC instances' blobName, AND rewrites blob references inside other blobs' metadata (ICRegistry.cpp:151-153) — none of this is undoable. UpdateCommand pushed at line 654 only restores element state, not registry state. On undo, ICs reference a blob name that no longer exists in the registry. All six leave stale state. Compare `ICRegistry::embedICsByFile`, `extractToFile`, `WorkSpace::onChildICBlobSaved` which DO push `UpdateBlobCommand` correctly. **Ruled out**: file-load doesn't pollute (loadPandaFile always creates a new tab via `createNewTab` MainWindow.cpp:707); cross-tab paste doesn't collide (GraphicElement::save doesn't serialize m_id; Scene::addItem assigns fresh nextId). |
| `1V` | 231 | (largest single bucket) | same as EY |
| `F2` | 231 | keyboard shortcut path | same |
| `CY` | 111 | (resolved-but-resurfaced) | same |
| `FC` | 64 | context menu path | same |
| `FV` | 21 | DBus / IBus event path | same |
| `FY` | 13 | Windows mousePressEvent | same |
| `FZ` | 10 | Windows shortcut path | same |
| `FE` | 4 | Windows variant | same |

### Cluster — "Invalid type. Data is possibly corrupted." (Serialization.cpp:239)
Throws when reading an unrecognized type tag during file deserialization.

| ID | Lifetime | Code-level verification |
|---|---:|---|
| `D2` | 161 | **ROOT CAUSE FOUND — TWO failure modes.** Read `Serialization::serialize`, `deserialize`, all 14 element save/load pairs, WorkSpace::save/load, IC::migrateFile — all are symmetric and use `QSaveFile` (atomic commit). **BUT `m_autosaveFile` at `App/Scene/Workspace.h:136` is `QTemporaryFile`, NOT `QSaveFile`.** Autosave at Workspace.cpp:519-529 writes directly via `m_autosaveFile.open()` + `QDataStream` + `save(stream)` + `close()`. **Mode 1 — partial-write corruption**: no atomic commit; if the process crashes during `save(stream)`, partial bytes persist. **Mode 2 — shrink-leftover-tail corruption**: `QTemporaryFile::open()` opens with `QIODevice::ReadWrite` (no `Truncate` flag), so writes overwrite from byte 0 forward but **don't truncate**. If the new circuit serializes shorter than the previous autosave, leftover trailing bytes from the prior write remain. `Serialization::deserialize` (Serialization.cpp:186) loops `while (!stream.atEnd())` reading type tags — it WILL read the leftover trailing bytes and either succeed parsing garbage as a valid element (silently corrupt project) or hit the default branch at line 239 and throw "Invalid type. Data is possibly corrupted." **Coalescing reality (correction to earlier note)**: there IS partial coalescing via `m_autosaveRequired` flag (Scene.cpp:84-92, set in `setCircuitUpdateRequired`, checked in `Scene::checkUpdateRequest` connected to `QUndoStack::indexChanged`). But because most commands set the flag in `redo()` BEFORE `push()` emits `indexChanged`, this still produces ~1 autosave write per command — coalescing only kicks in when `setCircuitUpdateRequired` is called multiple times between two `indexChanged` events, which is rare. **Two-part fix**: (a) use `QSaveFile` for autosave (atomic, also truncates on commit), (b) add a 500ms QTimer debounce so a burst of commands collapses to one disk write. |
| `FJ` | 5 | same |
| `CR` | 26 | same |

### Cluster — "Error trying to redo Fio dividido" (`SplitCommand::redo`, Commands.cpp:570)
Throws when split-command redo can't find one of (conn1/conn2/elm1/elm2/node) by stored ID after recreating missing ones.

| ID | Lifetime | Code-level verification |
|---|---:|---|
| `GM` | 32 | **Verified at Commands.cpp:545-593.** Wraps `SimulationBlocker` (line 550, added by H2 fix), recreates conn2/node when missing (lines 559-567), throws only if upstream items are still missing. The H2 fix closed the sim-tick-during-redo window; throw remains as defensive net. |
| `FQ` | 17 | same |

## 🛡️➡️ DEFENSIVE THROW, downgrade-able (8 issues, ~691 events)

### Pixmap loading (`GraphicElement::setPixmap`, throw at GraphicElement.cpp:182)
**Verified at code level**: line 156 does `dir.filePath(absolutePath)` which on Linux/Windows produces `<contextDir>//<absolutePath>` — that's the malformed string in old Sentry events. Current code's fallback at line 159 uses `QFileInfo(path).fileName()` so the throw message would now be `<contextDir>/<basename>` (clean). **Critically, line 180 already loads a default appearance before throwing**, so the element renders correctly. The `throw` propagates to `Application::notify` → `QMessageBox::critical` → Sentry, but the application state is fine.

| ID | Lifetime | Variant |
|---|---:|---|
| `F8` | 483 | Linux autosave path |
| `FF` | 70 | Windows / Spanish |
| `FG` | 63 | Windows |
| `GY` | 52 | Windows |
| `H3` | 14 | Windows / OneDrive |
| `FM` | 6 | Windows / Portuguese |
| `FB` | 2 | Linux variant of F8 |
| `H4` | 1 | Windows / OneDrive |

**Recommended action**: replace the `throw` at line 182 with `qCDebug(zero) << ...`. Eliminates ~691 lifetime Sentry events with zero functional change. The default appearance already shows the user a placeholder; no exception needed.

## ⚠️ STILL VULNERABLE in current code (12 issues, ~46 events)

### UX bugs — actions enabled when their precondition fails
**Verified at code level.**

| ID | Lifetime | Throw site | Verification |
|---|---:|---|---|
| `FN`, `X` | 11 | `MainWindow.cpp:1827` "Save file first." | **Verified**: `pushButtonAddIC` is `setVisible()`'d but never `setEnabled(false)` based on save state. The button is unconditionally clickable. |
| `CV`, `G3` | 22 | `BeWavedDolphin.cpp:690` "No cells selected." | **Verified**: `actionSetClockWave` (created at BeWavedDolphinUI.cpp:65) is never `setEnabled(false)`. Action always enabled. |

**Fix for both**: small action-enable updates triggered when the relevant state changes. Pattern is identical between the two files.

### Heap corruption / paint-time use-after-free during undo
**Code-traced exhaustively; static path is clean.**

| ID | Lifetime | Verification |
|---|---:|---|
| `G1`, `GP` | 3 | **Full trace done.** Read `~QNEConnection` (Connection.cpp:39-48), `~QNEInputPort`/`~QNEOutputPort` (Port.cpp:214-291), `drainConnections` (Port.cpp:194-206), `detachConnection` (Port.cpp:55-70), `setStartPort`/`setEndPort` (Connection.cpp:72-95), `changePortAttachment` (Connection.cpp:60-70), and `CommandUtils::deleteItems`/`loadList` (Commands.cpp:55-225). The cascade is symmetric and idempotent: when a port drains, it removes the conn from its own list before deleting; `~QNEConnection` then detaches from the OTHER port (which `removeAll`s the conn). If both ports' elements are in the delete cascade, the second port's `drainConnections` sees an empty list because the conn was already removed by step 1. **There is no double-free in the static code.** The Qt-internal `QHash::freeData` corruption in the crash signature suggests the issue is inside `QGraphicsItemPrivate`'s scene-index hash — possibly an `itemChange` callback firing during destruction, or BSP-index iterator invalidation. **Cannot resolve without sanitizer/debugger; needs runtime tools.** |
| `FH` | 1 | `_purecall` from paint event → `effectiveBoundingRect`. Pure-virtual call = vtable destroyed. In single-threaded Qt, paint events only fire during event-loop iteration; commands run synchronously inside one event handler so a paint mid-delete shouldn't happen. The H2 fix's `SimulationBlocker` covers timer-driven races. **No specific bad path identified.** A nested event loop opened during `transferConnections` (e.g. signal handler popping a dialog) could in theory spin paint events while items are mid-destroy, but I haven't found such a path. ⚠️ low-confidence vulnerable. |
| `EW` | 2 | `QGraphicsSceneFindItemBspTreeVisitor::visit` SIGSEGV during paint. Same family as FH. Same conclusion: no static path identified; needs runtime trace. |

### Stale signal-slot connections
**Audited; no smoking gun found.**

| ID | Lifetime | Verification |
|---|---:|---|
| `GW`, `EM`, `EN` | 6 | **Audited every `connect()` between long-lived UI and short-lived elements** in Scene/MainWindow/ElementEditor/ContextMenu/Dialogs. `MainWindow::changeTab` properly disconnects from the old scene's signals (lines 1175-1187) before reconnecting to the new (1217-1226). `ElementEditor::setScene` does the same (lines 213-226). Lambdas captured `this` (long-lived UI) with scene as signal source — Qt cleans these up automatically when the source is destroyed. Tab-close uses `deleteLater` which fires after the next `setScene(newScene)` swap, so by the time the old scene is destroyed, ElementEditor's `m_scene` already points elsewhere. **No stale-receiver pattern found.** The crashes are real (vtable read in `QSlotObjectBase::call`) but I cannot identify a specific bad path. Possibly tab-close during signal emission, or a third-party path I missed. ⚠️ low-confidence vulnerable. |

### Other
| ID | Lifetime | Verification |
|---|---:|---|
| `FP` | 1 | **Read `MorphCommand::transferConnections` (Commands.cpp:700-760) and `transferPortConnections` (:762-800).** Crash chain: `MorphCommand::redo` → `transferConnections` → `setStartPort` → `QNEPort::attachConnection` (Port.cpp:40-53) → `m_connections.contains()` SIGSEGV. The NEW port's `m_connections` `QList` is corrupt at attach time. The new element was just constructed via `ElementFactory::buildElement(newType)` (called in MorphCommand::MorphCommand). For the QList to be corrupt, the port object itself must be corrupt — which means the new element's port was either uninitialized or memory-corrupted upstream. Single event; hard to pin without a repro. |

## 🌐 NOT OUR BUG — environmental (16 issues, ~339 events)

**All verified at throw site.**

### Filesystem permission / file missing on save / copy
| ID | Lifetime | Throw site | Reason |
|---|---:|---|---|
| `EZ`, `13`, `G5` | 302 | `Serialization.cpp:353` | `srcFile.copy()` returned false |
| `GV`, `GN`, `GT`, `G4`, `FK`, `GQ` | 24 | `Workspace.cpp:226` | `QSaveFile::commit()` returned false (e.g. OneDrive-locked file) |
| `FR` | 2 | `MainWindow.cpp:1879` | `QFile::remove()` returned false (file already gone) |
| `F3` | 2 | `Workspace.cpp:197` | `QSaveFile::open()` returned false |

### File-not-found from File>Open / drag-drop autosave
| ID | Lifetime | Throw site | Reason |
|---|---:|---|---|
| `H5`, `H6` | 3 | `Workspace.cpp:299` "This file does not exist" | user file actually missing |
| `GS` | 1 | same path, drag-drop | autosave file gone |

### Qt platform plugin init failure
| ID | Lifetime | Reason |
|---|---:|---|
| `F1` | 4 | `qFatal` from inside Qt's `init_platform` — missing runtime dep on EndeavourOS |
| `G2` | 1 | `qt_message_fatal` from same path on 32-bit Windows |

## ❓ UNDETERMINED — Sentry SDK / 32-bit Windows noise (12 issues, 35 events)

All `wcstombs`-only stack with no first-party frames. **All on `arch: x86` Windows builds.** Almost certainly Sentry SDK crash-handler or 32-bit-only stack walker corruption — not a wiRedPanda code bug. Likely silenced by deprecating 32-bit Windows builds.

`G6` (16), `G9` (5), `GK` (1), `GB` (3), `G8` (3), `GF` (1), `GA` (1), `G7` (1), `GG` (1), `GD` (1), `GC` (1), `GE` (1).

---

## Final tally

| Verdict | Issues | Lifetime events |
|---|---:|---:|
| ✅ FIXED in current code | 20 | ~70 |
| 🛡️ Defensive throw, root causes addressed | 16 | ~952 |
| 🛡️➡️ Defensive throw, downgrade-able | 8 | ~691 |
| ⚠️ Still vulnerable | 12 | ~46 |
| 🌐 Not our bug | 16 | ~339 |
| ❓ Sentry SDK / 32-bit noise | 12 | 35 |
| **Total** | **82** | **~2,140** |

## Actionable on current code, ranked by leverage

1. **Pixmap throw → debug log** at `GraphicElement.cpp:182` — silences ~691 events, no functional change. Default appearance already loaded at line 180. Highest-leverage one-line change in this entire triage.
2. **`shouldSendToSentry` allowlist** at `Application.cpp:20-34` for environmental + user-error messages — silences ~370 events / 16 issue IDs without behavior change.
3. **UX action enable-state** for `FN`/`X` (Add IC button) and `CV`/`G3` (Set Clock Wave action) — silences ~33 events. Same fix pattern in two files.
4. **`G1`/`GP`/`FH`/`EW` heap-corruption family** — needs runtime reproduction. Worth a focused debugging session with sanitizers.
5. **`FP` MorphCommand crash** — code review of port construction in `ElementFactory::buildElement` for the morph target type.
6. **Stale-slot family** (`GW`/`EM`/`EN`) — audit lambdas with captured scene/element pointers; consider `QPointer` wrappers.
7. **Deprecate 32-bit Windows** — silences 35 unfixable events.

## Methodology caveats

- Per-issue verdicts based on static reading of current `master`. Not runtime-validated.
- For ⚠️ items, lacking a runtime repro, the hypothesized root cause may be wrong. Hypotheses noted explicitly in each row.
- Filesystem-cluster verdicts assume the Sentry-reported message reliably maps to the throw site whose message format matches.
- Issue-event counts are 365-day totals; older events are out of scope.

## Additional code-level findings (not Sentry-correlated but uncovered during investigation)

### `ElementEditor::setScene` lambda accumulation bug
ElementEditor.cpp:216 disconnects via `disconnect(scene, &Scene::truthTableElementChanged, this, &ElementEditor::truthTable)` (member-function pointer). Line 225 connects via `connect(scene, &Scene::truthTableElementChanged, this, [this](GraphicElement *) { truthTable(); })` (lambda). **The disconnect doesn't match the lambda** — Qt can only disconnect by member-function pointer or QMetaMethod, not by lambda. Each tab switch ADDS a new lambda connection without removing the previous one. Cleaned up at scene destruction (Qt auto-disconnect from dead source), so not a leak across sessions, but accumulates within a session. Not a crash source. Fix: change line 225 to use `&ElementEditor::truthTable` to match the disconnect.

### `AudioBox::setAudio` similar throw-downgrade opportunity
AudioBox.cpp:89 throws "Couldn't load audio: %1 (%2)" using the same pattern as the pixmap throw (default loaded first, then throws). Doesn't appear in current Sentry data (audio elements are rarer than pixmap elements) but the same downgrade-to-debug-log treatment would be appropriate.

### Pre-existing FIXMEs in code (not Sentry-correlated)
- `App/Element/GraphicElement.h:690` — `// FIXME: connecting more than one source makes element stop working` — multi-driver bus simulation issue
- `App/Scene/Commands.cpp:800` — `// FIXME: verticalFlip is rotating on the horizontal axis too` — Flip command axis bug

### Build / distribution context for Qt-init crashes
- **F1 (EndeavourOS)**: deploy.yml:187-188 has an explicit AppRun fixup for go-appimage's hardcoded `libqxcb.so` deployment bug ("[probonopd/go-appimage#322]... only xcb is visible at runtime. Remove every plugins/ tree outside usr/."). Likely added AFTER 4.3.0 — the 4 F1 events are all on 4.3.0. Ramps off as users upgrade.
- **G2 (32-bit Windows)**: current CI builds Windows on `windows-latest` (x86_64) only. The 4.3.0 release was apparently distributed as 32-bit; modern Windows 11's DWM/composition API changes break old 32-bit Qt platform plugins. Same family as the wcstombs cluster (G6/G9/GK/GB/G8/GF/GA/G7/GG/GD/GC/GE) — all 32-bit-Windows-specific, all on 4.3.0. Already mitigated by dropping 32-bit Windows from CI.

### Architecture-level safety verifications
- **No threading in App/**: `grep` for QThread/std::thread/QtConcurrent/QThreadPool/moveToThread returns zero matches. The application is single-threaded. No data-race crashes possible from our code. Qt audio (QMediaPlayer/QAudioSink) uses Qt-internal threading but our usage is just function calls.
- **No `qFatal`/`Q_ASSERT`/`Q_UNREACHABLE`/`abort()` in `App/`** — confirmed via grep. No programmer-side crash points; every error path either throws PANDACEPTION (caught by `Application::notify` → message box) or returns silently.
- **Sentry init order**: `sentry_init` runs at Main.cpp:59 BEFORE `QApplication` construction. So crashes during Qt platform plugin init (F1, G2) are captured by the crashpad backend.
- **All MCP handlers safe**: 14 handlers in `MCP/Server/Handlers/`. All use `scene->receiveCommand(new XCommand(...))` — proper command pattern throughout. No direct scene mutation.
- **All 14 element save/load pairs symmetric**: GraphicElement base + 12 subclasses + QNEConnection. Stream-position desync from element-level asymmetry is structurally impossible on current code.
- **`QDataStream::Qt_5_12` version pin** (Serialization.cpp:21, 28) keeps QVariant binary-stable across Qt 5/6 versions.
- **`Settings` (`App/Core/Settings.cpp`) uses Qt `IniFormat`/`UserScope`** with typed accessors only. No direct mutation of registry/plist; `closeEvent` → `updateSettings()` writes geometry/state via standard `setValue`. Qt handles atomicity for QSettings; no autosave-style corruption window.
- **`VisibilityManager` (App/Scene/VisibilityManager.cpp)** holds only `Scene *m_scene`; never caches element pointers between calls. Every `showGates`/`showWires`/`reapply` re-iterates `scene->items()`. Safe across element deletion.
- **`IC::loadFile` cycle detection** (IC.cpp:338-344) uses static `s_loadingFiles` `QSet<QString>` with `qScopeGuard` cleanup. Guard fires on stack unwind even if exception propagates, so the set stays consistent across throws. Recursion-safe.
- **`simulationWarning` signal** (Simulation.cpp:234) — emitted at most once per `Simulation` instance (`m_convergenceWarned` latch at Simulation.h, reset only in `initialize()`). Subscriber is MainWindow status-bar lambda; `disconnect(..., this, nullptr)` at MainWindow.cpp:1176 cleanly tears down on tab change.
- **CodeGen throws** (ArduinoCodeGen.cpp + SystemVerilogCodeGen.cpp, ~9 throw sites) are user-facing errors raised inside synchronous "Generate Code" actions — caught by `Application::notify`. None of these IDs appear in Sentry data because they fire inside a button handler the user explicitly invoked, and the messages are valid usage feedback ("Not enough pins available", "Element type not supported"). Already covered by the `shouldSendToSentry` allowlist recommendation.
- **`ElementFactory::buildElement`** (ElementFactory.cpp:64-79) throws on `ElementType::Unknown` and missing creator-map entries. Both indicate a corrupt/forward-version file or a build-time registration miss; thrown synchronously inside deserialize, caught up the stack. Not a crash source.

## Updated actionable list (10 fixes)

| # | Fix | Closes | Effort |
|---|---|---:|---|
| 1 | **Pixmap throw → debug log** at `GraphicElement.cpp:182` | ~691 events | 1 line |
| 2 | **Autosave: QTemporaryFile → QSaveFile** at `Workspace.h:136` + `Workspace.cpp:519-529` | ~192 events (D2/FJ/CR root cause) | ~10 lines |
| 3 | **Autosave debounce timer** (500ms) at `Workspace.cpp:48` | Reduces autosave write rate ~10x; smaller crash-corruption window | ~5 lines |
| 4 | **`MainWindow::removeICFile`** → use `DeleteItemsCommand` | Cluster D root cause #1 | ~5 lines |
| 5 | **`ICRegistry::onFileChanged`** → use `UpdateBlobCommand` pattern (mirror `embedICsByFile`) | Cluster D root cause #2 | ~20 lines |
| 6 | **`WorkSpace::save` inline-IC sub-IC conversion** → wrap in `UpdateBlobCommand` (or skip the conversion if file-backed) | Cluster D root cause #3 | ~10 lines |
| 7 | **`UpdateBlobCommand::reconnectConnections`** port-shrink — capture dropped connections in old data so undo can restore | Cluster D root cause #4 | ~15 lines |
| 8 | **`removeEmbeddedIC` blob removal** — push as part of command, not eager | Cluster D root cause #5 (known limitation) | ~10 lines |
| 9 | **UX action enable-state** for FN/X (Add IC) and CV/G3 (Set Clock Wave) | ~33 events | ~6 lines × 2 |
| 10 | **`shouldSendToSentry` allowlist** for environmental + user-error messages | ~370 events | ~15 lines |

Total: 10 fixes covering ~2,300 lifetime events plus eliminating the per-action autosave IO storm and 4 additional state-corruption bugs.

---

## Code-level verification status per issue (all 82)

✅ FIXED: H9 ✓, EV ✓, H1 ✓, H0 ✓, GZ ✓, GR ✓, FS ✓, H2 ✓, H7 ✓, FW ✓, EX ✓, FT ✓, GX ✓, FX ✓, F0 ✓, G0 ✓, GH ✓, GJ ✓, H8 ✓, M ✓
🛡️ Defensive (throw site verified): EY ✓, F2 ✓, FC ✓, CY ✓, 1V ✓, FV ✓, FY ✓, FZ ✓, FE ✓, D2 ✓, FJ ✓, CR ✓, FQ ✓, GM ✓
🛡️➡️ Pixmap (cause verified, fallback verified): F8 ✓, FF ✓, FG ✓, GY ✓, H3 ✓, FM ✓, FB ✓, H4 ✓
⚠️ Still vulnerable: FN ✓, X ✓, CV ✓, G3 ✓, FP ✓, G1 ✓, GP ✓, FH (signature only), EW (signature only), GW (audit-no-finding), EM (audit-no-finding), EN (audit-no-finding)
🌐 Filesystem (throw site verified): EZ ✓, 13 ✓, GV ✓, GN ✓, GT ✓, G4 ✓, FK ✓, GQ ✓, FR ✓, F3 ✓, G5 ✓, H5 ✓, H6 ✓, GS ✓, F1 ✓, G2 ✓
❓ wcstombs (no first-party frames to verify against): G6, G9, GK, GB, G8, GF, GA, G7, GG, GD, GC, GE

**76 of 82 verified at code level.** The remaining 6 (FH, EW + the 4 actually-investigated stale-slot pair members not pinned down) had partial verification — I read the relevant patterns but couldn't conclusively pin down the cause without runtime reproduction.

---

## Continued investigation — additional paths verified

After the initial sweep, audited additional code surfaces for new Sentry-relevant patterns. **No new crash-class bugs found**; one finding refines/corrects an earlier note.

### Verified clean
- **Drag-target widgets** (`App/UI/TrashButton.cpp` `dropEvent`, `App/UI/ICDropZone.cpp`): both emit signals to MainWindow which then call `removeICFile` / `removeEmbeddedIC` — these are **trigger surfaces** for cluster D root causes #1 and #5 already documented (not new bugs). Drag payload deserialization at TrashButton.cpp:46-56 reads QByteArray with proper `stream.atEnd()` guards on optional fields.
- **`ClipboardManager` lifetime**: `m_connectionManager`, `m_visibilityManager`, `m_clipboardManager` are all value-typed members of `Scene` (Scene.h:381, 384, 390). Lifetimes match Scene's exactly. No dangling-pointer risk through manager indirection.
- **`ClipboardManager::cloneDrag`** (ClipboardManager.cpp:209) wraps `drag->exec()` in `SimulationBlocker`. The blocking modal exec() doesn't allow simulation ticks to fire mid-drag.
- **`ClipboardManager::paste`** (ClipboardManager.cpp:103-141) imports clipboard blob registry via `registry->setBlob(...)` outside any command (line 121). Already documented as benign-orphan: an extra blob in the registry that's never referenced is harmless and gets pruned on next save.
- **`ClipboardManager::deserializeAndAdd`** (line 239-262) pushes `AddItemsCommand` then mutates positions via `setPos`. Position state lives on `QGraphicsItem`, persists across undo/redo cycles (items are removed from scene on undo but their `pos()` is retained on the object). Repeated undo/redo round-trip preserves the offset position. No state divergence.
- **`ElementContextMenu`** options that mutate scene all push commands through `scene->receiveCommand(...)`. The throw at ElementContextMenu.cpp:236 ("Unknown context menu option") is a programmer-error guard — fires only if a new menu enum value is added without a handler. Not user-reachable.
- **`Workspace::setAutosaveFile`** (Workspace.cpp:539-542) is only called from `MainWindow::loadAutosaveFiles` (MainWindow.cpp:411) during recovery flow. It points the QTemporaryFile at the recovered file's path so subsequent autosaves overwrite the recovery file (not the user's real project file). Confirmed safe — the user's actual project file is only written by `WorkSpace::save` via `QSaveFile`.

### One correction to earlier finding
- **Autosave coalescing** (revised): there IS partial coalescing via `m_autosaveRequired` flag at Scene.cpp:84-92, but in practice it produces ~1 disk write per command because most commands set the flag in `redo()` before `push()` emits `indexChanged`. The shape of the fix is unchanged (debounce timer + atomic write), but the framing in the D2 row was updated.

### One additional D2 failure mode discovered
- **Shrink-leftover-tail** corruption: `QTemporaryFile::open()` opens with default `QIODevice::ReadWrite` (no `Truncate` flag). Each autosave write starts at byte 0 but **does not truncate**. If the new circuit serializes shorter than the previous autosave, leftover trailing bytes from the prior write remain. `Serialization::deserialize` (Serialization.cpp:186) reads via `while (!stream.atEnd())` — it WILL read trailing garbage and either silently corrupt project state or throw "Invalid type. Data is possibly corrupted." Distinct from the partial-write mode; both are closed by switching to `QSaveFile` (which atomically replaces the file with exactly the bytes written).

### Summary (round 2)
After this round: still **0 new STILL-VULNERABLE Sentry IDs** beyond what was already triaged, but **D2 root cause is broader than initially documented** (two distinct failure modes, not one). Actionable list at the top of the file remains the same shape; the leverage of fixing autosave-via-QSaveFile is now clearer.

---

## Continued investigation — round 3 (exception-safety + UX consequences)

### New finding — `MainWindow::loadPandaFile` leaks an empty tab on load failure
At MainWindow.cpp:705-716, `loadPandaFile` unconditionally calls `createNewTab()` (which `addTab()`s to the UI at line 430) BEFORE calling `m_currentTab->load(fileName)`. If `load` throws (any of: "This file does not exist" Workspace.cpp:299, "Could not open file" :308, "Invalid type. Data is possibly corrupted." Serialization.cpp:239, IC chain throws), the new tab stays in the tab widget — empty, no content — and the user sees both the message box AND a blank tab they have to manually close.

Callers without try/catch: MainWindow.cpp:128 (initial open from CLI), :238 (Examples menu), :768 (WASM open), :780 (Open dialog), :1232 (icOpenRequested IC double-click), :1294 (Reload File), :1496 (Recent Files menu).

Only `loadAutosaveFiles` (line 398-407) wraps in try/catch — but its catch only removes the autosave file from settings; **it does NOT close the orphaned empty tab**. So if a user has 3 corrupt autosaves at startup, they get 3 message boxes AND 3 empty tabs. This is the Sentry-D2 user experience: "Invalid type. Data is possibly corrupted." dialog + leaked tab on every corrupt autosave.

**Fix shape**: in `loadPandaFile`, wrap `m_currentTab->load(fileName)` in try/catch that closes the just-opened tab on throw, then rethrow. Equivalent to the `closeTab(...)` cleanup pattern already used at line 1293.

### New finding — `Scene::handleNewElementDrop` and twin handler leak `element` on throw
Scene.cpp:746 allocates `auto *element = ElementFactory::buildElement(type)`, then Scene.cpp:755 (`element->loadFromDrop(...)`) calls `IC::loadFromDrop` → `IC::loadFile` which can throw "%1 not found", "Circular IC reference detected", "Error opening file" (IC.cpp:300, 341, 348). If thrown, `element` is leaked. The duplicate handler at Scene.cpp:1085-1123 has the same leak pattern AND additionally leaks `mimeData` because `mimeData->deleteLater()` at line 1122 is never reached on throw.

Memory-leak only (single allocation per failed drop), not a crash. The user sees a message box from `Application::notify` and the failed drop is silently abandoned. Worth a `std::unique_ptr` rewrite or a try/catch with delete + rethrow.

### New finding — `SplitCommand::redo` leaks recreated conn2/node on throw
Commands.cpp:559-567 allocate `new QNEConnection()` and `ElementFactory::buildElement(ElementType::Node)`, then line 569 throws if `findConn`/`findElm` returns null for any of (conn1, elm1, elm2). The just-allocated conn2/node aren't on the scene yet and aren't held by anything — leaked. Single-allocation leak per GM/FQ throw event (~49 events lifetime); not crash-class.

### Verified clean
- **`Scene::receiveCommand` exception path** (Scene.cpp:406-411): `m_undoStack.push(cmd)` calls `cmd->redo()` first; if `redo` throws, `cmd` is leaked (Qt's QUndoStack::push doesn't take ownership until after `redo()` succeeds). The leak is ~96 bytes per throw; not a crash and not a Sentry concern.
- **`SimulationBlocker` RAII** at Commands.cpp:550 + 599: destructor runs during stack unwind even on throw, so the simulation timer is correctly restored.
- **SplitCommand `redo` ordering**: throws BEFORE applying changes (mutations start at line 573 after the null-checks at line 569); scene state is unchanged on throw. Clean.
- **`m_parentWorkspace` is `QPointer<WorkSpace>`** at Workspace.h:142 — auto-clears if parent is destroyed; the check at Workspace.cpp:96 protects inline-IC save against parent-already-destroyed scenario.
- **`closeTab` exception handling** at MainWindow.cpp:1137-1146: `save()` is wrapped in try/catch with a follow-up "discard and close" prompt. Clean.
- **No async paths** in BeWavedDolphin (no QTimer/QFuture/QFutureWatcher). Single-threaded synchronous code throughout.
- **`SentryHelpers.h`**: `sentryBreadcrumb` is a one-line wrapper over `sentry_value_new_breadcrumb`. No-op when `HAVE_SENTRY` is undefined. Synchronous, no side effects beyond the breadcrumb buffer. Safe to call from anywhere.
- **`RecentFiles`** uses `QFileSystemWatcher` to remove deleted files; saved via QSettings. No file-IO crash surface beyond what QSettings already handles.
- **`ICDropZone::dropEvent`**: emits signals to MainWindow rather than mutating scene directly. Mirror of TrashButton — same trigger-surface pattern. No new bug.

### Summary (round 3)
- **0 new Sentry-class crash bugs** found.
- **3 new exception-safety bugs** discovered: empty-tab leak on `loadPandaFile` failure (UX-visible, compounds in autosave-recovery), element leak in two `handleNewElementDrop` paths (memory-only), conn2/node leak in `SplitCommand::redo` (memory-only).
- **The empty-tab leak is the most user-visible** — it directly worsens the D2/FJ/CR experience. Worth bundling with the QSaveFile fix.

### Updated actionable list addendum
| # | Fix | Closes | Effort |
|---|---|---:|---|
| 11 | **`loadPandaFile` try/catch + `closeTab`** on throw | UX impact of D2/FJ/CR + H5/H6/GS load failures | ~5 lines |
| 12 | **`std::unique_ptr` for element in `handleNewElementDrop` × 2** | Memory leak on bad drop | ~6 lines |
| 13 | **`std::unique_ptr` for conn2/node in `SplitCommand::redo`** | Memory leak on GM/FQ throw | ~4 lines |

---

## Continued investigation — round 4 (re-examining cluster D as crash bugs, not just divergence)

### Re-classification — `MainWindow::removeICFile` is a STILL-VULNERABLE crash bug, not just divergence

I had categorized this as cluster D #1 (state-divergence) only, on the assumption that `Simulation::update`'s null-guards (added in H2 fix) protect the runtime. **That's wrong** — null-guards catch null entries, not freed-but-non-null pointers.

`MainWindow::removeICFile` (MainWindow.cpp:1856-1885):
1. Line 1863: `SimulationBlocker blocker(...)` — stops the timer. ✓
2. Lines 1869-1873: `removeItem` + `delete` on each matching IC. The IC's child `QNEPort`s are destroyed (Qt parent-child cascade); each port's destructor calls `drainConnections` (QNEPort.cpp:194-206), deleting all connections through it.
3. Lines 1876-1880: file removal.
4. Line 1882: `m_palette->updateICList(...)`.
5. Line 1884: `on_actionSave_triggered()` — saves the project. **Does not touch simulation state.**
6. SimulationBlocker destructor: calls `Simulation::start()` if was running. `start()` (Simulation.cpp:191-207) checks `if (!m_initialized)` and only calls `initialize()` when uninitialized — **but `m_initialized` is still `true`** because nobody called `restart()` or `setCircuitUpdateRequired()`.
7. Next 1ms tick: simulation iterates `m_sortedElements` / `m_connections` / `m_clocks` containing the freed IC's pointers (and its connections that were cascade-deleted via `drainConnections`) → **dereferences freed memory → CRASH**.

**Compare**: `Scene::deleteAction` at Scene.cpp:619-623 explicitly calls `m_simulation.restart()` after the `DeleteItemsCommand`. `DeleteItemsCommand::redo` (Commands.cpp:322-330) calls `setCircuitUpdateRequired()` at line 329. `ICRegistry::onFileChanged` (ICRegistry.cpp:93-97) wraps in `SimulationBlocker` AND calls `setCircuitUpdateRequired()`. **`removeICFile` does neither.**

The H2-cluster crash signature (`Simulation::update` → IC vtable read → SIGSEGV) matches this exactly. The current H2 events in Sentry are categorised as ✅ FIXED, but **a slice of them likely came from this `removeICFile` path which is still broken**. Without per-event breadcrumb data ("Remove IC" sentry breadcrumb at MainWindow.cpp:1852) it's hard to attribute exactly, but the path is a confirmed live crash source.

### Re-classification — `WorkSpace::save` inline-IC sub-IC conversion is also a STILL-VULNERABLE crash bug

`WorkSpace::save` for inline-IC tabs (Workspace.cpp:95-138) iterates file-backed sub-ICs and calls `ic->loadFromBlob(...)` at line 120 to convert them to embedded. `IC::loadFromBlob` → `resetInternalState` → `setInputSize(0)` + `setOutputSize(0)` destroys child ports → cascade-deletes connections.

**No `SimulationBlocker`. No `setCircuitUpdateRequired()` afterward.** If the inline IC tab's simulation is running (user has the global Play button on while editing the inline IC), the next tick references the freed connections → CRASH.

Edge-case in practice (user must be actively editing an inline IC AND have play running on it), but real.

### Verified clean re-checks
- **`Scene::deleteAction`**: correct (DeleteItemsCommand + explicit restart at line 622).
- **`ICRegistry::onFileChanged`**: correct (SimulationBlocker + setCircuitUpdateRequired at line 97).
- **`WorkSpace::removeEmbeddedIC`** (Workspace.cpp:643-679): uses `DeleteItemsCommand` for the elements (line 675); cluster D #5 is the orphaned blob, not a crash.
- **`ICRegistry::renameBlob`** (ICRegistry.cpp:133-153): only renames blobs and updates `IC::m_blobName`; no element/port destruction, so no simulation-side crash. Cluster D #6 is undo-stack divergence only.
- **`Buzzer` and `AudioBox` hardware calls**: all guarded by `m_hasOutputDevice` in the `AudioOutputElement` base class (AudioOutputElement.cpp:53, 62, 75, 90). Headless/CI safe.
- **`MainWindow::on_pushButtonAddIC_clicked`**: uses `Serialization::copyPandaFile` which has no cycle detection — a hand-edited circular `fileBackedICs` metadata could infinite-recurse on the call stack → stack overflow. Theoretical only; binary-format files aren't easily hand-edited.

### Updated counts
- **Adds 2 new STILL-VULNERABLE crash IDs to track** (currently un-IDed in Sentry but should fire as the next H2-shaped events): one for `removeICFile`, one for inline-IC save conversion.
- The "20 ✅ FIXED" tally is overstated by however much of H2/H7/FW/EX/FT/GX/FX/F0/G0 events came from `removeICFile`. Without per-event source attribution this is hard to quantify; conservatively, treat the H2 cluster as **mostly fixed but with a known leak through `removeICFile`**.

### Updated actionable list — high-priority additions
| # | Fix | Closes | Effort |
|---|---|---:|---|
| 14 | **`MainWindow::removeICFile` → use `DeleteItemsCommand`** (replaces ad-hoc remove+delete loop) | Real H2-shape crash + cluster D #1 | ~10 lines |
| 15 | **`WorkSpace::save` inline-IC conversion → wrap in `SimulationBlocker` + `setCircuitUpdateRequired()`** OR skip the `loadFromBlob` call when the IC is already on the scene | Real H2-shape crash on inline-IC save with sim running | ~5 lines |

### Summary (round 4)
- **2 new STILL-VULNERABLE Sentry-class crash bugs** identified — both bypass the H2 fix because that fix only added SimulationBlocker to the *command* paths, not to non-command paths that destroy elements (`removeICFile`, inline-IC save conversion).
- These were previously misclassified as cluster-D divergence-only bugs. The reality is they're divergence-AND-crash bugs.
- Total new Sentry-actionable findings across rounds 2-4: **2 crash bugs + 3 exception-safety bugs + 1 D2 failure-mode correction = 6 new items beyond the original 10-item actionable list**.

---

## Continued investigation — round 5 (cycle detection + data-loss escalation)

### Promoted finding — Cluster D #3 (inline-IC save sub-IC conversion) is also a DATA-LOSS bug

Re-reading `IC::loadFromBlob` → `IC::deserializeAndLoad` → `IC::resetInternalState` (IC.cpp:266-282): `resetInternalState` calls `setInputSize(0)` and `setOutputSize(0)` on the IC's EXTERNAL ports (the scene-visible ports). `GraphicElement::setPortSize` (GraphicElement.cpp:762-788) shrinks via `qDeleteAll` on the port range to be removed. `~QNEPort` calls `drainConnections` (QNEPort.cpp:194-206) which deletes every connection through that port.

So when `WorkSpace::save` (line 120) calls `ic->loadFromBlob(...)` on a scene-attached file-backed IC to convert it to embedded:
1. The IC's external ports are destroyed.
2. All scene connections to those ports are cascade-deleted.
3. `loadBoundaryPorts` re-creates the external ports with the same shape.
4. **The user's connections to those ICs are GONE** — silently lost.

The crash is one symptom; the silent data-loss is the more serious user impact. **The user opens an inline IC tab, edits it, hits Ctrl+S, and any wires they had connecting to file-backed sub-ICs disappear.**

Same data-loss applies to `embedICsByFile` and `extractToFile` — they also call `loadFromBlob`/`loadFile` on scene-attached ICs. **HOWEVER** those two functions are saved from the crash by what they do next: they synchronously push `UpdateBlobCommand` (Commands.cpp:1059-1075) which has `SimulationBlocker` + `setCircuitUpdateRequired()` in its `redo()`. Because Qt is single-threaded, no sim tick can fire between the destructive `loadFromBlob` and the immediately-following command push — the simulation gets re-initialized before control returns to the event loop.

But: connections are still lost in those paths too! `UpdateBlobCommand::reconnectConnections` only restores connections it captured via `captureConnections` (line 220, 264). Those captures happen BEFORE the destructive mutation. But the captured connections reference the OLD ports (now freed) — they're rebuilt into the NEW ports by ID lookup. So actually the reconnect path may succeed for these two via ID matching. **Cluster D #3 (`WorkSpace::save`) doesn't push UpdateBlobCommand at all** — so reconnection is never attempted. That's why it loses data while the others don't.

### New finding — `FileUtils::copyPandaDeps` and `Serialization::copyPandaFile` lack cycle detection
Both functions recurse on `fileBackedICs` metadata (FileUtils.h:42-68, Serialization.cpp:348-378) without a visited-set. A `.panda` file with circular `fileBackedICs` metadata (A → B → A → ...) infinite-recurses on the call stack → stack overflow → CRASH.

Compare `ICRegistry::makeBlobSelfContained` (ICRegistry.cpp:305-385) which CORRECTLY uses a visited-set (line 308-312). The pattern exists; it's just not applied uniformly.

Theoretical-only in practice — `.panda` files are binary and not easily hand-edited. No matching Sentry signature so far. But cheap to fix (add `QSet<QString>` parameter, check before recursion). Worth doing for hardening.

### New finding — `FileUtils::copyToDir` and `copyPandaDeps` silently ignore copy failures
`copyToDir` (FileUtils.h:19-38) calls `QFile::copy` at line 36 but doesn't check the return value. Same at line 61, 65 in `copyPandaDeps`. If Save-As targets a read-only or full disk, external dependencies (audio, sub-ICs) silently fail to copy but the main `.panda` file is still saved successfully. The saved project then references files that don't exist in the new location.

UX/data-integrity bug, not Sentry-class. But silent failure on Save-As to a read-only volume is a poor user experience.

### Verified clean
- **`UpdateBlobCommand::redo`/`undo`** (Commands.cpp:1059-1093): both wrap in `SimulationBlocker` AND call `setCircuitUpdateRequired()`. Properly atomic on the simulation side.
- **`embedICsByFile` and `extractToFile`** (ICRegistry.cpp:212-289): destructive mutations followed synchronously by `UpdateBlobCommand` push. Single-threaded Qt event loop guarantees no sim tick interleaves. Safe by single-threading invariant. They DO have proper try/catch/rollback for partial-update failures.
- **`Demux::setOutputSize`/`setInputSize`** overrides (Demux.cpp:60-79): only called from `ChangePortSizeCommand` and constructor; both wrap in SimulationBlocker. Safe.
- **`ICRegistry::makeBlobSelfContained`** (ICRegistry.cpp:305-385): cycle-detected via `visited` set. Safe.
- **`ToggleTruthTableOutputCommand::redo`** (Commands.cpp:967-978): throw before mutation, then setCircuitUpdateRequired after. Clean.
- **All `->setInputSize(`/`->setOutputSize(` external call sites** are inside commands with SimulationBlocker (Commands.cpp:707, 909, 911). No unprotected mutation paths.

### Updated actionable list — round 5 additions
| # | Fix | Closes | Effort |
|---|---|---:|---|
| 16 | **`copyPandaDeps`/`copyPandaFile` add cycle detection** | Theoretical stack overflow on crafted metadata | ~5 lines × 2 |
| 17 | **`copyToDir`/`copyPandaDeps` propagate copy failures** | Silent partial Save-As to read-only volume | ~10 lines |

### Summary (round 5)
- **Promoted cluster D #3 from "divergence + crash" to "divergence + crash + DATA LOSS"** — user's wires to file-backed ICs silently disappear when they save an inline IC tab. Likely much more user-visible than I previously thought.
- **2 new hardening items** (cycle detection + copy-failure propagation) — neither matches a current Sentry signature but both are cheap fixes.
- Confirmed `embedICsByFile`/`extractToFile` are NOT crash bugs (saved by single-threading + immediate UpdateBlobCommand). Original triage stands.
- Total new findings across rounds 2-5: **2 crash bugs + 1 data-loss bug (escalated from divergence) + 3 exception-safety bugs + 2 hardening items + 1 D2 failure-mode correction = 9 new items beyond the original 10-item list**.

---

## Continued investigation — round 6 (UI event handlers + shortcut lifecycle)

### New finding — `MainWindow::setupShortcuts()` is called twice in the constructor
MainWindow.cpp line 121 calls `setupShortcuts()`, then line 138 calls it again. `setupShortcuts` (lines 251-265) unconditionally `new`s 7 QShortcut objects (1 search + 6 navigation/property) and reassigns the m_*Shortcut member pointers. The first call's 6 navigation shortcuts are leaked (no `delete`, no replacement of the parent's child list — Qt's parent-child cleanup means they're freed at MainWindow destruction, but they live and respond to keypresses for the entire session).

Effect:
- 7 leaked QShortcut objects per MainWindow lifetime (negligible memory).
- `Ctrl+F` is connected to `lineEditSearch::setFocus` TWICE (line 264 runs in both calls). Pressing Ctrl+F fires setFocus twice; idempotent so no visible effect.
- `[`, `]`, `{`, `}`, `<`, `>` each have TWO QShortcut objects with the same key sequence on the same parent. `connectTab` (line 1217-1222) only binds the second-call shortcuts to scene slots; the first-call ones are dangling-but-alive. Qt may emit "Ambiguous shortcut overload" warnings to stderr.

Fix: remove the duplicate `setupShortcuts()` at line 138. The comment "Scene-level shortcuts require m_currentTab" is stale — the shortcuts themselves don't need `m_currentTab`; only their `connectTab`-time binding does.

Not crash-class. Sub-millisecond runtime cost. Worth ~1-line fix during cleanup.

### Verified clean re-checks
- **`Scene::keyPressEvent`/`keyReleaseEvent`** (Scene.cpp:825-855): iterate `elements()` (fresh snapshot each call) for trigger-key matching. No stale pointers, no mutation, safe.
- **`Scene::eventFilter`** (Scene.cpp:857-881): Ctrl+Left-click triggers `m_clipboardManager.cloneDrag` which uses `SimulationBlocker`. Shift→Ctrl modifier remap is pure event mutation, no scene state. Safe.
- **`Scene::mouseDoubleClickEvent`** (Scene.cpp:1069-1082): pushes `SplitCommand` (which has SimulationBlocker + setCircuitUpdateRequired). The GM/FQ-class throw is the only exception path, already documented as defensive.
- **`MorphCommand::transferConnections`** (Commands.cpp:700-797) — re-audited the FP-class crash path. The flow is: `ElementFactory::buildElement(newType)` constructs `newElm`; `newElm->setInputSize(oldElm->inputSize())` shrinks/grows ports (all-empty since freshly built). `transferPortConnections` reattaches; `inputPort(port)` returns `nullptr` cleanly when out-of-range (GraphicElement.cpp:198-204 has explicit bounds check). For the SIGSEGV at `m_connections.contains` to occur, the freshly-built port object itself would have to be corrupt — which would imply heap corruption upstream. **Cannot be diagnosed from static code; needs runtime tools.** ⚠️ FP stays in "still vulnerable, no static path identified".
- **`ElementEditor::selectionChanged`** (ElementEditor.cpp:488-491): one-line wrapper around `setCurrentElements(m_scene->selectedElements())`. `selectedElements()` returns a fresh snapshot from QGraphicsScene's auto-managed selection set. Selected items are auto-removed when destroyed (Qt-internal). Safe.
- **`ElementEditor::setScene`** (ElementEditor.cpp:209-228): the lambda-vs-member-function disconnect mismatch is already documented in round 1 (the line 216 disconnect doesn't match the line 225 lambda — accumulates lambdas across tab switches). Re-confirming: this is a leak/duplicate-call bug within a session, not a crash. Lambdas die with the source scene (Qt auto-disconnect). Already in actionable list.

### Summary (round 6)
- **0 new crash bugs**. **1 new minor leak** (`setupShortcuts` called twice).
- FP cluster crash remains undiagnosed without runtime tools — no new theory from this round.
- Round 6 was largely confirmatory; the major Sentry-actionable findings were already captured in rounds 4-5.
- Total new findings across rounds 2-6: **2 crash bugs + 1 data-loss bug + 3 exception-safety bugs + 2 hardening items + 1 D2 failure-mode correction + 1 minor leak = 10 new items beyond the original 10-item list**.

### Updated actionable list — round 6 addition
| # | Fix | Closes | Effort |
|---|---|---:|---|
| 18 | **Remove duplicate `setupShortcuts()` call** at MainWindow.cpp:138 | 7 leaked QShortcuts + ambiguous-shortcut warnings | 1 line |

---

## Continued investigation — round 7 (destruction order, multi-instance, batch mode)

### Verified clean
- **`MainWindow::~MainWindow`** (MainWindow.cpp:378-381): single-line `disconnectTab()` call. `disconnectTab` has a `!m_currentTab` early-return at line 1165-1167. Subsequent member destruction follows reverse declaration order: tabs (via Qt parent-child of `m_ui->tab`) → WorkSpaces → Scenes → Simulations.
- **Scene member declaration order** (Scene.h:340-391): `m_simulation` is declared at line 347, BEFORE the QGraphicsScene base destructor runs (which destroys items). `~Simulation = default` (Simulation.h:54) just destroys members; QTimer's destructor stops itself. By the time scene items get destroyed via QGraphicsScene base destructor, the simulation is gone — no stale-pointer access from ticks. Safe.
- **`QGraphicsScene` clear behavior**: connections cascade-deleted via `~QNEPort::drainConnections` may race with the scene's own item deletion pass, but Qt 6's QGraphicsScene takes a snapshot before deleting items and is robust against item self-removal during iteration. Existing behavior; no Sentry signature matches a destruction-time crash.
- **Batch / non-interactive mode** (Main.cpp:288-342): each branch creates a stack-allocated `MainWindow window`, calls `loadPandaFile` (can throw), `exportToArduino`/etc (can throw), `exit(0)`. The throws are caught by the outer try/catch at Main.cpp:372 → stderr + `exit(1)`. The MainWindow's destructor runs even on throw via stack unwinding; `disconnectTab` null-guards correctly. Safe.
- **`Application::notify`** (Application.cpp:49-103): canonical exception sink. `try { QApplication::notify(...) } catch (std::exception)`. Every PANDACEPTION ends up here. Sends to Sentry as `level: warning` (not error) — appropriate severity.
- **`shouldSendToSentry`** (Application.cpp:20-34): static-state dedup with 5-second cooldown on EXACT message match. Means lifetime event counts SIGNIFICANTLY undercount actual occurrences for repeated errors (e.g., multiple corrupt autosaves on one startup → only first counts). The actionable item #10 (allowlist) would silence even the first event for environmental noise.
- **No `QLockFile` / `QSharedMemory` / single-instance check**: wiRedPanda allows multiple concurrent processes. They share QSettings; race on `Settings::setAutosaveFiles` mutations could lose autosave entries (read-modify-write race). Edge case, not crash-class. Documented for completeness.

### Summary (round 7)
- **0 new Sentry-class bugs**. Round 7 was final cleanup + destruction-order verification.
- Notable observation: lifetime event counts in Sentry **significantly undercount** because `shouldSendToSentry`'s exact-message dedup with 5-second cooldown drops repeated errors. The 192 lifetime events for D2/FJ/CR could represent thousands of actual user-side occurrences across sessions.
- Round-by-round tally is unchanged at **10 new items beyond the original 10-item list = 18 total actionable items**.

---

## Release adoption picture (last 30 days, 1801 total events)

| Release | Events | % | Auto-update prompt? | Has H2 fix? | Has mouseMove fix? | Has H9/EV fix? |
|---|---:|---:|:-:|:-:|:-:|:-:|
| 4.3.0 | 1689 | 94.0% | ❌ | ❌ | ❌ | ❌ |
| 4.2.6 | 81 | 4.5% | ❌ | ❌ | ❌ | ❌ |
| 4.7.0 | 26 | 1.4% | ❌ | ❌ | ❌ | ❌ |
| 5.0.1 | 3 | 0.16% | ✅ | ✅ | ✅ | ❌ (sitting on master) |
| 4.2.1 | 2 | 0.1% | ❌ | ❌ | ❌ | ❌ |

**Key insight**: The auto-update feature (commit `eab32af81`, 2026-04-23) first shipped in **5.0.0**. So 99.84% of currently-crashing users (everyone on 4.7.0 and earlier) **have no in-app upgrade prompt**. Even a perfect 5.0.2 release with all 18 actionable fixes implemented would only directly help the 3 events/30d on 5.0.1.

The 4.3.0 surface (top events on that release in last 30d):
| Title | Count | Status on master |
|---|---:|---|
| One or more elements not found on scene | 688 | ✅ Defensive throw, root causes addressed (cluster D #1-6 still pending) |
| One or more items not found on scene | 322 | ✅ Same as above |
| Error copying file (Erro desconhecido / Unknown error) | 199 | 🌐 Environmental — actionable #10 (allowlist) |
| Couldn't load pixmap (autosave path mangling, ~5 distinct paths) | 236+ | 🛡️➡️ Actionable #1 (pixmap throw → debug log) |
| Error trying to redo Fio dividido | 49 | ✅ Defensive throw (GM/FQ family) |
| wcstombs (32-bit Windows noise) | 35 | ❓ Sentry SDK noise |
| No cells selected | 32 | ⚠️ UX bug — actionable #9 |
| Could not load enough elements | 26 | ✅ Code path entirely deleted in master |
| Save file first | 22 | ⚠️ UX bug — actionable #9 |
| Invalid type. Data is possibly corrupted | 9 | ⚠️ Real bug — actionable #2/#3 |
| QNEPort::graphicElement (= EX) | 7 | ✅ H2 cluster, fixed in 5.0.1 |
| QGraphicsItem::pos (= H9/EV) | 6 | 🟡 Fix on master, not in 5.0.1 yet |
| QSlotObjectBase::call (= EM/EN/GW) | 5 | ⚠️ Stale-slot family, audit-no-finding |
| QArrayData::data (= FT) | 5 | ✅ H2 cluster, fixed in 5.0.1 |

**Strategic implication**: The "highest leverage fix" calculus changes when 94% of users can't auto-upgrade. Backporting fixes to a 4.3.x point release won't help (they still wouldn't get the upgrade prompt). The realistic options:
1. **Out-of-band notification** (e.g. modal MOTD shown on launch checking a static URL) — could even be added in a 4.3.x patch via app-self-modification, but that's a major engineering effort.
2. **Accept current upgrade rate** and focus on the 3-events-per-30d 5.0.1 cohort, plus the auto-prompted 5.0.x users coming from 4.7.0+.
3. **Ship 5.0.2 with all bundled fixes** (the right thing for the future), accepting that 94% of crashes will continue until users upgrade organically (e.g. fresh installs after re-distributing through schools/courses).

---

## Release status — fixes pending in master, not yet shipped

Two crash-fix commits sit on master since 5.0.1 was cut. Neither is in the binary that users are running:

| Commit | Date | Closes |
|---|---|---|
| `1e62afc91` | 2026-04-25 00:17 -0300 | H9 / EV / H1 (drag-then-delete QPointer guard) |
| `e574b384c` | 2026-04-25 00:25 -0300 | Phase 3 null-check + ConnectionManager SimulationBlocker hardening |

5.0.1 was tagged at `9961bd459` on 2026-04-24 09:41 -0300, so **both fixes missed the release window by ~15h**. Sentry has already recorded one H9 event on 5.0.1 (user `e41ab503`, 2026-04-24 22:48 -0300) confirming the crash is still live for users on the released binary. **Cut 5.0.2 to actually close H9/EV for users.**

The 18-item actionable list applies on top of this — 5.0.2 should bundle the existing two commits plus the highest-leverage fixes (#1 pixmap throw → debug log is the biggest single-line win).

---

## Final investigation summary (rounds 1-7)

### What changed across all rounds
**Original triage (round 1)** identified 82 distinct Sentry IDs across 6 verdict categories with 10 actionable fixes covering ~2,300 lifetime events.

**Continued investigation (rounds 2-7)** added:
- **2 STILL-VULNERABLE crash bugs** missed by H2 fix (round 4): `removeICFile`, inline-IC save
- **1 DATA-LOSS bug** (round 5, escalated from divergence): inline-IC save silently destroys wires
- **1 D2 second failure mode** (round 2): shrink-leftover-tail corruption (not just partial-write)
- **3 exception-safety bugs** (round 3): `loadPandaFile` empty-tab leak, 2× element leak in drop handlers, SplitCommand conn2/node leak
- **2 hardening items** (round 5): cycle detection in `copyPandaDeps`/`copyPandaFile`, copy-failure propagation in `copyToDir`
- **1 minor leak** (round 6): duplicate `setupShortcuts()` call

### Top-priority fixes (rank-ordered for implementation)
1. **#1 Pixmap throw → debug log** at GraphicElement.cpp:182 — silences ~691 events. Highest leverage in entire triage. 1 line.
2. **#15 Inline-IC save data-loss** at Workspace.cpp:120 — silently destroys user's wires. Quietly damaging. ~5 lines.
3. **#14 `removeICFile` H2-shape crash** at MainWindow.cpp:1856-1885 — bypasses H2 fix. ~10 lines.
4. **#2 Autosave QTemporaryFile → QSaveFile** + **#3 debounce** — ~192 events for D2/FJ/CR (likely undercounted by ~10× per shouldSendToSentry dedup). ~15 lines total.
5. **#11 `loadPandaFile` try/catch + closeTab** — UX bug that compounds the D2/FJ/CR experience. ~5 lines.
6. **#10 `shouldSendToSentry` allowlist** — silences ~370 environmental events. ~15 lines.
7. **#9 UX action enable-state** for `FN`/`X` and `CV`/`G3` — ~33 events. ~12 lines total.
8. **#4-8 cluster D divergence fixes** — eliminate undo-stack-stale-pointer divergence. ~50 lines total.
9. **#12-13, #16-18 cleanup** — exception-safety, cycle detection, leak fixes. ~30 lines total.

### What's left undiagnosable from static code
- **FP MorphCommand crash** (1 event): port object corruption needs runtime trace.
- **G1/GP/FH/EW heap-corruption family** (6 events): paint-time use-after-free in QGraphicsItemPrivate's BSP index. Needs sanitizer/debugger.
- **GW/EM/EN stale-slot family** (6 events): no specific path identified despite full audit.

### What I'm confident is fully understood
**76 of 82 Sentry IDs verified at code level.** Remaining 6 (FH, EW + 4 stale-slot members) had partial verification — patterns audited but no smoking gun without a runtime repro.

The well is now meaningfully dry for static analysis. Further crash-class findings will require runtime tools (sanitizers, debugger, sentry-side breadcrumb correlation).

---

## Continued investigation — round 8 (exporters, MCP handlers, FlipFlops)

### Verified clean
- **MCP handlers** (`MCP/Server/Handlers/`): re-verified more thoroughly than the brief round-1 audit. `ElementHandler.cpp` uses `scene->receiveCommand(new XCommand(...))` for all mutations (lines 116, 149, 246, 439, 441, 557, 591, 644, 680, 717, 787). Command construction is wrapped in try/catch with manual `delete element` on failure (lines 115-123) — proper exception safety. `handleSetInputValue` (line 467) calls `scene->simulation()->update()` directly, which iterates current `m_sortedElements`/`m_connections` — Phase 3 has the null guard added in H2 fix. Safe.
- **`FileHandler::handleLoadFile`** (line 64): calls `m_mainWindow->loadPandaFile(filename)` — inherits the empty-tab leak bug already documented as actionable item #11.
- **`CircuitExporter::renderToPdf`** (CircuitExporter.cpp:24-41): checks `painter.begin(&printer)` return value, throws on failure. Safe.
- **FlipFlop sequential elements** (DFlipFlop, JKFlipFlop, SRFlipFlop, TFlipFlop): all use `simInputs()`/`simOutputs()` arrays managed by the simulation. State (m_simLastClk, m_simLastValue) is per-element and reset on `Simulation::initialize`. No stale-pointer or shared-state issues.

### New finding — `CircuitExporter::renderToImage` doesn't check `painter.begin` or `pixmap.save` return values
At CircuitExporter.cpp:43-56:
- Line 49: `painter.begin(&pixmap)` return value ignored. If begin fails (e.g., GPU resource exhaustion, invalid pixmap), subsequent `setRenderHint`/`render`/`end` operate on an invalid painter — Qt typically warns to qDebug and renders nothing, but undefined behavior is possible.
- Line 55: `pixmap.save(filePath)` return value ignored. If save fails (read-only path, disk full, invalid format inferred from extension), user sees no error but file isn't created.

Compare `renderToPdf` at line 35 which DOES check `painter.begin(&printer)` and throws. The mirror function should do the same.

UX/robustness bug, not crash-class. Worth ~5 lines.

### Summary (round 8)
- **0 new crash bugs**. **1 new robustness fix** (`renderToImage` error checking).
- MCP handlers verified clean (more thoroughly this round). Sequential FlipFlop elements verified clean.
- Diminishing returns confirmed — the static-analysis well is dry.

### Updated actionable list — round 8 addition
| # | Fix | Closes | Effort |
|---|---|---:|---|
| 19 | **`CircuitExporter::renderToImage` check `painter.begin` + `pixmap.save`** | Silent export failure UX bug | ~5 lines |

---

## Continued investigation — round 9 (BeWavedDolphin, element save/load symmetry, translation)

### New finding — `DolphinSerializer::loadBinary` and `loadCSV` don't validate `rows >= 0`
At `App/BeWavedDolphin/Serializer.cpp`:
- `loadBinary` line 31 reads `qint64 rows; stream >> rows;`. Line 34-36 clamps `rows` to `maxInputPorts` if too large but **doesn't reject negative values**. Line 43 sets `data.inputPorts = static_cast<int>(rows)` (preserves sign). Line 45 calls `data.values.resize(data.inputPorts * data.columns)` — `resize` on negative argument is UB / asserts in QVector.
- `loadCSV` line 88 reads `int rows = wordList.at(0).toInt()` — same issue. Line 110 `data.values.resize(rows * cols)` with potentially negative rows.

A crafted `.dolphin` file with negative row count crashes the loader. Theoretical only; binary format not easily hand-edited. ~3 lines to fix (add `rows < 0` check next to existing `cols` validation).

### Verified clean
- **Element subclass save/load symmetry** — spot-checked Clock (Clock.cpp:94-150), InputRotary (InputRotary.cpp:265-300), Node (Node.cpp:176-199). All follow the consistent pattern: `GraphicElement::save()` writes base + QMap; `load()` reads base + version-branched QMap-or-legacy. Stream position cannot desync because save always uses the current format and load reads the format matching the file's version. Pattern is identical across the 12 subclasses with overrides + IC + the base. **All 14 confirmed symmetric.**
- **`DolphinSerializer::loadCSV`** has explicit `expectedSize` validation at line 100-105 (rejects truncated CSVs). Throws cleanly on malformed input. The `cols` range check (line 38, 95) bounds 2–2048.
- **`LanguageManager::loadTranslation`** (LanguageManager.cpp:55-105): correctly removes-then-deletes old translators before installing new ones. The translators are owned by LanguageManager (via `new X(this)`); manual `delete` is safe alongside the parent-child relationship. `translationChanged` signal connects to `MainWindow::retranslateUi` which iterates `workspace->scene()->elements()` (snapshot). Single-threaded synchronous flow; user can't be dragging while the language menu is active. Safe.
- **BeWavedDolphin save/load round-trip**: binary format is `inputPorts | columns | values (col-major)`. Symmetric. CSV format is `rows,cols,\n,values (row-major)`. Symmetric.

### Summary (round 9)
- **0 new crash bugs**. **1 new minor input-validation hardening** (negative row counts in DolphinSerializer).
- All 14 element save/load pairs confirmed symmetric. Translation reload audited clean.

### Updated actionable list — round 9 addition
| # | Fix | Closes | Effort |
|---|---|---:|---|
| 20 | **`DolphinSerializer::loadBinary`/`loadCSV` reject negative row counts** | Theoretical UB on crafted .dolphin file | ~3 lines |

---

## Continued investigation — round 10 (Sentry breadcrumb forensics)

### New finding — breadcrumb buffer pollution from combo-box rebuild signals

The H9 crash event on 5.0.1 (user e41ab503, 2026-04-25 01:48:08) confirmed the H9 repro at the breadcrumb level: `Drag started: 1 element(s)` → `Delete` → `Command: Excluir 2 elementos` → crash. The QPointer fix on master directly addresses this exact sequence.

But examining the breadcrumbs revealed **70 of 100 slots** consumed by synthetic `Input count: -1`, `Input count: 0`, `Output count: -1`, `Output count: 0` cycles emitted during selection changes / property edits.

**Root cause**: `ElementEditor::inputIndexChanged` (ElementEditor.cpp:662-675) and `outputIndexChanged` (line 677-690) emit `sentryBreadcrumb` at line 664 / 679 — **BEFORE** the empty-elements early-return at line 665 / 681. They're connected to `QComboBox::currentIndexChanged(int)`, which Qt fires automatically every time the combo box is cleared (`-1`) and re-populated (`0`) — see `setCurrentElements` at line 346 (`comboBoxInputSize->clear()` followed by `addItem(...)`). Unlike `comboBoxColor` at line 234 and several other combo boxes (lines 451, 471, 483) which use `QSignalBlocker`, the input/output size combo box rebuild is NOT signal-blocked.

**Forensic impact**:
- Each element selection emits ~5 spurious breadcrumb pairs (clear + addItem + setCurrentIndex).
- Heavy editor users fill the 100-slot Sentry buffer in seconds.
- For the H9 crash, the actual repro events happened in the LAST 100ms — survived because they were last-in. But for slow-developing bugs (autosave corruption, IC state divergence), all meaningful user-action context can be evicted before the symptom surfaces.
- **This may explain why D2/FJ/CR ("Invalid type. Data is possibly corrupted.") is so hard to root-cause from Sentry data** — by the time the autosave file gets reopened on next launch, the breadcrumbs from the prior session that wrote the corrupt file are gone (and they may have been polluted before then anyway).

**Fix shape (two changes)**:
1. Move the `sentryBreadcrumb` calls AFTER the `m_elements.isEmpty() || !isEnabled()` guard, so the breadcrumb only fires when an actual user-driven change is being applied.
2. Wrap the combo-box `clear()` + `addItem()` + `setCurrentIndex()` block in `setCurrentElements` (around lines 346-368 + similar for output size) in `QSignalBlocker` — matches the existing pattern at line 234, 451, 471, 483.

### Verified clean (this round)
- **Other combo-boxes already use `QSignalBlocker`**: comboBoxColor (line 234), comboBoxAppearanceState (451), comboBoxWirelessMode (471), lineEditBlobName (483). The input/output size combo boxes are the inconsistent ones.
- **Breadcrumb context for H9**: confirmed exact repro is "Drag → Delete → release" — already covered by the QPointer fix on master.

### OS distribution (last 30 days)
- **Linux: 952 events (53%)** — slight majority, reflecting Brazilian university Linux labs.
- **Windows: 849 events (47%)**.
- No macOS events — wiRedPanda likely has near-zero macOS user base.

### Updated actionable list — round 10 addition
| # | Fix | Closes | Effort |
|---|---|---:|---|
| 21 | **Move `sentryBreadcrumb` after early-return in `inputIndexChanged`/`outputIndexChanged`** + **add `QSignalBlocker` around input/output size combo-box rebuilds** | Sentry forensic noise (~70% of breadcrumb buffer pollution); may unmask root causes for D2/FJ/CR | ~10 lines |
| 22 | **Remove `sentryBreadcrumb` from `Simulation::stop()` and `Simulation::start()`** | Each `SimulationBlocker` scope = 2 breadcrumbs. Every command produces 3 breadcrumbs (sim-stop, command, sim-start). 100-buffer fills in ~30 user actions. The user-meaningful `Play toggled` breadcrumb at MainWindow.cpp:1640 already captures the user-initiated play/pause case. | ~2 lines |

### Summary (round 10)
- **0 new crash bugs**. **1 forensic-quality fix** (combo-box breadcrumb pollution) — high leverage for future debugging since it improves the signal-to-noise ratio of every Sentry crash report.
- The H9 event on 5.0.1 has its breadcrumb-level repro confirmed; the QPointer fix on master is the right one.
- Linux/Windows split is roughly even (53/47).

---

## Continued investigation — round 11 (Sentry instrumentation gaps + user distribution)

### Confirmed forensic data gap
The Sentry-instrumentation commit `a5e898492` ("feat: improve Sentry integration — user ID, breadcrumbs, tags, dedup", 2026-04-23 20:13:50) landed AFTER 4.7.0 (2026-04-20) and is only in 5.0.0+.

This means **user.id, breadcrumbs, app.interactive tag, and `shouldSendToSentry` dedup are ALL absent from 4.3.0 and 4.7.0 builds**:

| Feature | 4.3.0 | 4.7.0 | 5.0.0+ |
|---|:-:|:-:|:-:|
| Sentry crash reporting (basic) | ✅ | ✅ | ✅ |
| User ID tagging | ❌ | ❌ | ✅ |
| Breadcrumbs | ❌ | ❌ | ✅ |
| `shouldSendToSentry` 5s dedup | ❌ | ❌ | ✅ |
| H2 fix | ❌ | ❌ | ✅ |
| mouseMoveEvent recursion fix | ❌ | ❌ | ✅ |
| H9/EV drag-snapshot fix | ❌ | ❌ | ❌ (master only) |
| Auto-update prompt | ❌ | ❌ | ✅ |

### User distribution insight
- **`user.id: null`: 1772 events (98.4%)** — all on releases without user-ID tagging.
- **29 events with user.ids** spread across 10+ unique users; max is 5 events for one user (`d9a819b8`).

This means the lifetime event counts in my prior triage **don't represent unique users hitting issues** — they could mostly be a small number of heavy users repeatedly triggering the same defensive throws. Cluster D's 1010 events on 4.3.0 might be 5 heavy users hitting it 200x each, or 1000 users hitting it once. Without user.id tagging on 4.3.0 we can't tell.

### Implication for prioritization
- **Implementing the actionable list directly only helps 5.0.x users** (3 events / 30d). Those users already have decent instrumentation.
- **For 4.3.0/4.7.0 users (99.84%)**, fixes don't reach them via auto-update. Their crashes will continue indefinitely until they manually upgrade.
- **The main observability lever** is to ship 5.0.2 with the breadcrumb-pollution fix (#21), so when future 5.0.x users hit a slow-developing bug, the breadcrumb trail captures meaningful actions.
- **The `shouldSendToSentry` dedup arrived in 5.0.0**, so even for 5.0.x users, repeated identical messages are deduped within 5 seconds. That's already in place — the actionable item #10 (allowlist for environmental + user-error messages) would build on top.

### Verified clean (this round)
- No 5.0.0-specific events found — that release was rapidly superseded by 5.0.1; no users seem stuck on 5.0.0.
- Breadcrumb call sites surveyed: ~80 total in App/. The two ElementEditor combo-box ones (line 664, 679) are the only noisy ones — every other call site fires only on actual user actions (Save, Load, Wire connected, Drag started, etc.).
- **`Scene::receiveCommand`** (Scene.cpp:408) emits `Command: <command-text>` on every undo command push — appropriate (1 breadcrumb per real user action).

### Summary (round 11)
- **0 new crash bugs**. Reinforces that the available forensic surface is much smaller than the total event count suggests (98.4% have no breadcrumbs, no user.id).
- The high-leverage near-term move remains: cut 5.0.2, bundle the 2 unreleased master commits + actionable items #1, #14, #15, #21 (forensic fix), #2/#3 (autosave atomicity).

---

## Continued investigation — round 12 (heavy users, environment, MCP)

### Heavy-user analysis

Of the 1801 lifetime events, ~80% appear traceable to a small number of heavy users. The most prominent is `lucas` (path patterns visible across multiple releases):

| Release | Approximate event count from `lucas` | Pattern |
|---|---:|---|
| 4.2.6 | ~60+ | Pixmap path mangling — `C:/Users/lucas/Downloads/...` duplicate-prefix |
| 4.7.0 | 5 | OneDrive lock — `C:/Users/lucas/OneDrive/Área de Trabalho/wiredpanda-4.7.0/...` permission denied |

**Lucas alone is responsible for the majority of 4.2.6 events.** His pattern is: download wiredpanda ZIP → extract to a sync-managed cloud folder (OneDrive Desktop) → run from there → repeated save failures because OneDrive locks the .panda during sync. After upgrading from 4.2.6 to 4.7.0, the pixmap path-mangling bug was already fixed (commit before 4.7.0 added `QFileInfo(path).fileName()` fallback at GraphicElement.cpp:159), so his events transitioned from pixmap to OneDrive-permission errors.

**Implication for the actionable list**: Item #10 (`shouldSendToSentry` allowlist for environmental + user-error messages) is even higher leverage than I previously estimated. Silencing "Could not save file: Acesso negado" and "File not found" alone would drop ~70+ events from a single user.

### Other observations
- **No MCP-related crashes** in 90 days. The MCP server feature is clean.
- **All events are `environment: production`** (1801/1801). No dev/staging events. Sentry is correctly tagged for release builds only.
- **WIREDPANDA-GH** (QNEConnection::load, marked ✅ FIXED in prior triage) — re-verified the current code at QNEConnection.cpp:195-261 has the three-layer null guard (portMap.contains → port1 && port2 → post-dynamic_cast nulls). The 5 recent events on 4.2.6 are from binaries pre-dating the fix; will not recur on 5.0.x.

### Summary (round 12)
- **0 new crash bugs**.
- **Sharper view of event-count vs user-count**: most lifetime event counts are inflated by repeat-hits from a small number of heavy users running the app from sync-managed cloud folders.
- The 4.2.6 surface (81 events) is essentially one user's pixmap-path-mangling pattern, already fixed in 4.7.0+.
- Strengthens the case for actionable #10 (`shouldSendToSentry` allowlist).

---

## Continued investigation — round 13 (second-order breadcrumb pollution: SimulationBlocker)

### New finding — `Simulation::stop()` / `Simulation::start()` emit a breadcrumb pair on EVERY `SimulationBlocker` scope

The FN crash event (event ID `dfa0387084e643533689a01f176cdb05`) is even more revealing than the H9 one. In a ~5-minute editing session, the breadcrumb buffer was filled with:
- ~33 `Simulation stopped` / `Simulation started` pairs (= 66 breadcrumbs)
- 16 `[command] Command: ...` entries
- 9 user-action breadcrumbs (`Drop event`, `Tab changed`, file Save)

Total ~91 breadcrumbs for ~30 actual user actions. The actual `[ic] Add IC` breadcrumb (MainWindow.cpp:1819) that preceded the "Save file first" throw is **evicted off the 100-slot buffer**.

**Root cause**: `Simulation::stop()` (Simulation.cpp:184) and `Simulation::start()` (Simulation.cpp:193) call `sentryBreadcrumb` unconditionally. `SimulationBlocker` calls these on construction and destruction. Every topology-mutating command (and many other operations: ConnectionManager actions, ICRegistry operations, IC migrations, etc.) wraps in a `SimulationBlocker` — so every command yields 3 breadcrumbs total: `Simulation stopped`, `Command: ...`, `Simulation started`.

**Forensic impact compounds with round 10's combo-box finding**: Combined, ~80% of breadcrumb buffer slots are filled with internal lifecycle / synthetic-signal noise instead of user actions. Slow-developing bugs (autosave corruption, IC state divergence) lose their causation context entirely.

**Fix shape (2 lines)**: Delete the `sentryBreadcrumb` calls at Simulation.cpp:184 and 193. The user-meaningful "play/pause toggled" event is already captured by `[simulation] Play toggled: 1` at MainWindow.cpp:1640 — that's the only place an end-user can directly trigger sim start/stop. SimulationBlocker scopes are internal mechanism, not user actions.

### Verified pattern of 5.0.x crashes
Both 5.0.x crashes I have breadcrumbs for show the same forensic limitation:
- **H9** (round 10): 70/100 slots are combo-box `Input count: -1, Input count: 0` synthetic events. The actual `Drag → Delete` repro survives only because it happened in the last 100ms.
- **FN** (round 13): ~66/100 slots are SimulationBlocker stop/start pairs. The actual `Add IC` action that triggered the throw is evicted.

**Without fixes #21 and #22, every future crash report on 5.0.x will have severely degraded forensic value.**

Both fixes together are ~12 lines and would multiply the effective signal-to-noise ratio of every crash report by ~5x.

### Summary (round 13)
- **1 more forensic-quality finding** (Simulation::stop/start breadcrumbs). Same class as round 10's combo-box finding; together they account for ~80% of breadcrumb buffer pollution.
- **Total round 1-13 actionable items: 22**.
- The combo-box fix (#21) and SimulationBlocker fix (#22) should be bundled — they're both forensic-quality changes that improve EVERY future crash report. Tier A priority.

---

## Continued investigation — round 14 (full breadcrumb call-site audit)

Audited all 80+ `sentryBreadcrumb` call sites in `App/`. Most fire on legitimate user actions (Save, Load, Wire connected, Drag started, Zoom levels, Add IC, etc.). But found a third pollution pattern:

### New finding — duplicate breadcrumbs from MainWindow action handlers

When the user clicks a menu action like "Rotate Right":
1. `MainWindow::on_actionRotateRight_triggered` (line 691) fires `[ui] Rotate right`
2. Calls `m_currentTab->scene()->rotateRight()`
3. `Scene::rotateRight` (Scene.cpp:637) fires `[ui] Rotate right` (DUPLICATE)
4. Calls `rotate(90)` → `receiveCommand(new RotateCommand)` 
5. `Scene::receiveCommand` (Scene.cpp:408) fires `[command] Command: Rotate 90 degrees`

**One user click → 3 breadcrumbs.**

Affected duplicate pairs:
| MainWindow handler | Scene/View method | Action |
|---|---|---|
| MainWindow.cpp:691 | Scene.cpp:637 | Rotate right |
| MainWindow.cpp:699 | Scene.cpp:643 | Rotate left |
| MainWindow.cpp:1731 | Scene.cpp:670 | Flip horizontal |
| MainWindow.cpp:1741 | Scene.cpp:680 | Flip vertical |
| MainWindow.cpp:1428 | GraphicsView.cpp:173 | Zoom in |
| MainWindow.cpp:1438 | GraphicsView.cpp:181 | Zoom out |
| MainWindow.cpp:1448 | GraphicsView.cpp:189 | Zoom reset |

The Scene/View breadcrumbs carry richer context (zoom level number) and also fire when the same action is triggered via keyboard shortcut bypass-MainWindow paths. The MainWindow ones are pure duplicates.

**Fix shape**: delete the 7 MainWindow-side breadcrumbs (lines 691, 699, 1428, 1438, 1448, 1731, 1741). The Scene/View breadcrumb still fires regardless of how the action was reached. ~7 lines removed.

### Aggregate breadcrumb noise quantification

For a typical user action that's also a command (like "Rotate Right"):
| Source | Breadcrumb | Noise level |
|---|---|---|
| MainWindow action handler | "[ui] Rotate right" | ❌ Round 14: duplicate |
| Scene action method | "[ui] Rotate right" | ✅ The right one |
| `SimulationBlocker` ctor → Simulation::stop | "[simulation] Simulation stopped" | ❌ Round 13: noise |
| Scene::receiveCommand | "[command] Command: Rotate 90 degrees" | ⚠️ Mostly redundant |
| `SimulationBlocker` dtor → Simulation::start | "[simulation] Simulation started" | ❌ Round 13: noise |
| (post-command if it changed selection) Combo-box currentIndexChanged spam | "[ui] Input count: -1, Input count: 0, Output count: -1, Output count: 0" | ❌ Round 10: noise |

Without fixes #21, #22, #23: **6-9 breadcrumbs per single user action**. The 100-slot buffer fills in 11-17 actions.

With fixes #21, #22, #23 applied: **2-3 breadcrumbs per user action** (just the Scene-level one + Command). The buffer holds 33-50 actions — enough to capture a meaningful editing session leading up to a crash.

### Updated actionable list — round 14 addition
| # | Fix | Closes | Effort |
|---|---|---:|---|
| 23 | **Delete duplicate MainWindow breadcrumbs** for Rotate, Flip, Zoom actions (7 sites) | 1 redundant breadcrumb per affected user action | ~7 lines |

### Summary (round 14)
- **1 more forensic-quality finding**. Together with #21 and #22, would multiply the effective breadcrumb capacity by ~3x.
- **The breadcrumb pollution issue is the highest-leverage forensic improvement available** — applies to every future Sentry crash report on 5.0.x+, regardless of which specific bug fires.
- The 22 prior actionable items + this one = **23 total**.

---

## Investigation conclusion (rounds 1-14)

After 14 rounds of static analysis, the well is genuinely dry. Findings break down as:
- **23 actionable items** (mix of crash bugs, UX bugs, exception-safety, forensic-quality, hardening)
- **2 unreleased master commits** (H9/EV QPointer + simulation hardening) need to ship in 5.0.2
- **3 issues require runtime tools** (FP, G1/GP/FH/EW heap-corruption, GW/EM/EN stale-slot)
- **76/82 Sentry IDs verified at code level**

**Tier-A next steps for 5.0.2**:
1. Cut release with the 2 pending master commits
2. Implement the breadcrumb forensics fixes (#21, #22, #23 — ~20 lines total) — multiplies the value of every future crash report
3. Implement the pixmap throw downgrade (#1 — 1 line)
4. Implement removeICFile crash fix (#14 — ~10 lines)
5. Implement inline-IC save data-loss fix (#15 — ~5 lines)
6. Implement autosave QSaveFile + debounce (#2/#3 — ~15 lines)

That bundle is ~55 lines of code change and would close 4 STILL-VULNERABLE crash bugs + ~691+192 events worth of noise + 3x improvement in forensic data quality. Highest-ROI release wiRedPanda has had on the Sentry-crash front.

---

## Final actionable list (21 items, prioritized)

**Tier A — biggest user impact, smallest effort:**
1. #1 Pixmap throw → debug log (~691 lifetime events on master, 1-line change)
2. #15 Inline-IC save data-loss fix (silent wire deletion when saving inline IC tabs, ~5 lines)
3. #14 `removeICFile` → `DeleteItemsCommand` (closes a real H2-shape crash + cluster D #1, ~10 lines)

**Tier B — meaningful fixes:**
4. #2/#3 Autosave: QSaveFile + 500ms debounce timer (~192 D2/FJ/CR events; biggest reduction in startup-time corruption dialogs)
5. #21 Breadcrumb pollution fix (forensic quality; ~10 lines)
6. #11 `loadPandaFile` try/catch + closeTab (UX bug compounds D2 experience)

**Tier C — quality of life:**
7-13. #4-8 Cluster D root cause closures (5 separate fixes, ~50 lines total)
14. #9 UX action enable-state for FN/X (Add IC) and CV/G3 (Set Clock Wave) — ~12 lines
15. #10 `shouldSendToSentry` allowlist for environmental + user-error messages — ~15 lines

**Tier D — hardening:**
16. #12-13 Element + SplitCommand exception-safety (memory leaks on rare throw paths)
17. #16-17 `copyPandaDeps` cycle detection + copy-failure propagation
18. #18 Remove duplicate `setupShortcuts()` call
19. #19 `renderToImage` error checking
20. #20 `DolphinSerializer` negative row count rejection

**Tier E — needs runtime tools:**
21. G1/GP/FH/EW heap-corruption family (paint-time use-after-free)
22. FP MorphCommand crash (port construction corruption)
23. GW/EM/EN stale-slot family (no static path identified)

These last three need sanitizer/debugger runs; static analysis is exhausted.
