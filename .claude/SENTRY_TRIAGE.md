# Sentry Triage — wiRedPanda

Static-code triage of every distinct Sentry issue ID for this project (365-day window, full aggregate dump). Each issue was located at its throw site or crash signature on current `master` and the relevant code path inspected.

- **82 distinct issue IDs**, ~2,140 lifetime events
- **76 of 82 verified at code level**; 6 partial (signature-only, no static repro path)
- **23 actionable items** (mix of crash bugs, UX, exception-safety, forensic-quality, hardening)
- **2 unreleased master commits** must ship in 5.0.2 to actually close H9/EV for users
- **3 issue families need runtime tools** (sanitizer/debugger) — static analysis cannot resolve them

> **Read first:** [Release status — pending in master](#release-status--pending-in-master) and [Actionable list (tier-ordered)](#actionable-list-tier-ordered).

---

## Verdict legend

- ✅ **FIXED** — root cause removed or guarded; cannot recur from this path on current master.
- 🛡️ **DEFENSIVE THROW** — exception still thrown, but is a correct safety net for a state divergence whose causes have been closed.
- 🛡️➡️ **DEFENSIVE THROW, downgrade-able** — same, but the throw is unnecessary (default already loaded, etc.) and could be replaced with a debug log.
- ⚠️ **STILL VULNERABLE** — cause is still present; could recur on current code.
- 🌐 **NOT OUR BUG** — environmental: Qt API failure, filesystem permission, missing system dep.
- ❓ **UNDETERMINED** — stack-stripped beyond recognition.

---

## Final tally

| Verdict | Issues | Lifetime events |
|---|---:|---:|
| ✅ FIXED in current code | 20 | ~70 |
| 🛡️ Defensive throw, root causes addressed | 16 | ~952 |
| 🛡️➡️ Defensive throw, downgrade-able | 8 | ~691 |
| ⚠️ Still vulnerable | 12 | ~46 |
| 🌐 Not our bug (environmental) | 16 | ~339 |
| ❓ Sentry SDK / 32-bit Windows noise | 12 | 35 |
| **Total** | **82** | **~2,140** |

---

## ✅ FIXED in current code (20 issues, ~70 events)

| ID | Lifetime | Description | Code-level verification |
|---|---:|---|---|
| `H9`, `EV`, `H1` | 7 | drag-then-delete `QGraphicsItem::pos` | **NOT YET RELEASED.** Fix at `Scene.cpp:1015-1043` (`m_dragSnapshot` filters dead `QPointer`s) is commit `1e62afc91` (2026-04-25 00:17 -0300), **15h after 5.0.1 was tagged** (`9961bd459`, 2026-04-24 09:41 -0300). One H9 event already fired on 5.0.1 at 2026-04-24 22:48 (user `e41ab503`). **Cutting 5.0.2 is required to actually close this for users.** |
| `H0`, `GZ`, `GR`, `FS`, `HA`, `HB` | 7 | recursive `mouseMoveEvent` stack overflow / SIGABRT | `m_handlingMouseMove` guard at `Scene.cpp:969-973`. `HA`/`HB` (2026-04-25, release 4.7.0) are the same crash class — fix landed in commit `38c9a2b51` 3 days *after* 4.7.0 was tagged (`6f60e9730`, 2026-04-20). Users on 4.7.0 don't have the guard and will keep crashing until they upgrade. Same Brazilian user (Mogi das Cruzes) for both events; classic ensure-visible scrollbar replay loop in the stack. |
| `H2`, `H7`, `FW`, `EX`, `FT`, `GX`, `FX`, `F0`, `G0` | ~22 | Simulation hot-path use-after-free | Read `Simulation::update`, `restart()`, `IC::resetInternalState`, all `SimulationBlocker` call sites. The 1ms-tick race is closed by H2 commit's defects A-G. **Caveat:** see [Still-vulnerable § H2-shape escapes](#h2-shape-escapes) — two non-command paths bypass the H2 fix and likely contribute a slice of these events. |
| `GH`, `GJ` | 18 | autosave-load `QNEConnection::load` deref | Three layers of null checks at `QNEConnection.cpp:235-258`: `portMap.contains`, `port1 && port2`, post-`dynamic_cast` null check. |
| `H8`, `M` | 6 | "Could not load enough elements for the simulation" | `grep` returns zero matches in `App/`. Code path deleted entirely. |

---

## 🛡️ DEFENSIVE THROW, root causes addressed (16 issues, ~952 events)

### Cluster D — "items / elements not found on the scene"

Thrown at `Commands.cpp:121` (`findItems`), `:139` (`findElements`), `:204` (count-mismatch). Fires when an undo command's stored ID doesn't resolve to a scene item.

| ID | Lifetime | Variant |
|---|---:|---|
| `EY` | 327 | mousePressEvent path |
| `1V` | 231 | (largest single bucket) |
| `F2` | 231 | keyboard shortcut path |
| `CY` | 111 | resolved-but-resurfaced |
| `FC` | 64 | context menu path |
| `FV` | 21 | DBus / IBus event path |
| `FY` | 13 | Windows mousePressEvent |
| `FZ` | 10 | Windows shortcut path |
| `FE` | 4 | Windows variant |

H2 fix's defect F (`SimulationBlocker` on `UpdateCommand`/`MorphCommand`/`ChangePortSizeCommand`/`SplitCommand`) closes re-entrancy races. **Six confirmed un-addressed divergence paths remain** — see [Cluster D root causes](#cluster-d-root-causes-still-pending) below.

**Ruled out:** file-load doesn't pollute (`loadPandaFile` always creates a new tab via `createNewTab`); cross-tab paste doesn't collide (`GraphicElement::save` doesn't serialize `m_id`; `Scene::addItem` assigns fresh `nextId`).

### Cluster — "Invalid type. Data is possibly corrupted." (`Serialization.cpp:239`)

| ID | Lifetime |
|---|---:|
| `D2` | 161 |
| `CR` | 26 |
| `FJ` | 5 |

**Two failure modes — both rooted in the autosave file using `QTemporaryFile` instead of `QSaveFile`:**

1. **Partial-write corruption.** Autosave at `Workspace.cpp:519-529` writes via `m_autosaveFile.open()` + `QDataStream` + `save(stream)` + `close()`. No atomic commit — if the process crashes mid-write, partial bytes persist.
2. **Shrink-leftover-tail corruption.** `QTemporaryFile::open()` opens with `QIODevice::ReadWrite` (no `Truncate`). Writes overwrite from byte 0 forward but **don't truncate**. If the new circuit serializes shorter than the prior autosave, leftover trailing bytes remain. `Serialization::deserialize` (`Serialization.cpp:186`) loops `while (!stream.atEnd())` and either parses garbage as a "valid" element (silent project corruption) or hits the default branch and throws.

`m_autosaveRequired` (`Scene.cpp:84-92`) provides partial coalescing via `QUndoStack::indexChanged`, but most commands set the flag in `redo()` *before* `push()` emits the signal, so it still produces ~1 disk write per command.

**Two-part fix:** (a) `QSaveFile` (atomic + truncates on commit), (b) 500ms debounce timer to coalesce bursts. → actionable items [#2](#a2-autosave-qtemporaryfile--qsavefile) and [#3](#a3-autosave-debounce-timer).

### Cluster — "Error trying to redo Fio dividido" (`SplitCommand::redo`, `Commands.cpp:570`)

| ID | Lifetime |
|---|---:|
| `GM` | 32 |
| `FQ` | 17 |

Wraps `SimulationBlocker` (line 550, added by H2 fix), recreates conn2/node when missing (lines 559-567), throws only if upstream items are still missing. The H2 fix closed the sim-tick-during-redo window; throw remains as defensive net.

> **Memory leak on this throw path** — recreated `conn2`/`node` (`new` at lines 559-567) aren't on the scene yet and aren't held by anything. Single-allocation leak per throw event; not crash-class. → actionable item [#13](#d13-element--splitcommand-exception-safety).

---

## 🛡️➡️ DEFENSIVE THROW, downgrade-able (8 issues, ~691 events)

### Pixmap loading (`GraphicElement::setPixmap`, throw at `GraphicElement.cpp:182`)

Old Sentry events show `<contextDir>//<absolutePath>` — caused by `dir.filePath(absolutePath)` at line 156 on Linux/Windows. Current code's fallback at line 159 uses `QFileInfo(path).fileName()` so the throw message would now be `<contextDir>/<basename>` (clean).

**Critically, line 180 already loads a default appearance before throwing.** The element renders correctly. The `throw` propagates to `Application::notify` → `QMessageBox::critical` → Sentry, but application state is fine.

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

**Recommended:** replace the `throw` at line 182 with `qCDebug(zero) << ...`. Eliminates ~691 lifetime events with zero functional change. → actionable item [#1](#a1-pixmap-throw--debug-log).

> **Sibling pattern:** `AudioBox::setAudio` (`AudioBox.cpp:89`) throws "Couldn't load audio: %1 (%2)" using the same default-loaded-then-throw pattern. Doesn't appear in current Sentry data (audio elements are rarer) but the same downgrade-to-debug-log treatment would be appropriate.

---

## ⚠️ STILL VULNERABLE in current code (12 issues, ~46 events + un-IDed paths)

### UX bugs — actions enabled when their precondition fails

| ID | Lifetime | Throw site | Verification |
|---|---:|---|---|
| `FN`, `X` | 11 | `MainWindow.cpp:1827` "Save file first." | `pushButtonAddIC` is `setVisible()`'d but never `setEnabled(false)` based on save state. The button is unconditionally clickable. |
| `CV`, `G3` | 22 | `BeWavedDolphin.cpp:690` "No cells selected." | `actionSetClockWave` (created at `BeWavedDolphinUI.cpp:65`) is never `setEnabled(false)`. Action always enabled. |

→ actionable item [#9](#c9-ux-action-enable-state).

### H2-shape escapes

Two non-command paths destroy elements without `SimulationBlocker` *and without* `setCircuitUpdateRequired()`. The H2 fix only added `SimulationBlocker` to *command* paths. The H2 cluster is "mostly fixed but with a known leak through these two paths" — they're un-IDed in Sentry but should fire as the next H2-shaped events.

1. **`MainWindow::removeICFile`** (`MainWindow.cpp:1856-1885`) uses `removeItem` + `delete` directly. The IC's child `QNEPort`s cascade-delete via Qt parent-child; each port's destructor calls `drainConnections` and deletes connections. Then `on_actionSave_triggered()` runs (doesn't touch sim state) and the `SimulationBlocker` destructor calls `Simulation::start()` — but `m_initialized` is still `true`, so `initialize()` doesn't run. Next 1ms tick iterates `m_sortedElements`/`m_connections`/`m_clocks` containing freed pointers → crash. **Compare:** `Scene::deleteAction` (`Scene.cpp:619-623`) uses `DeleteItemsCommand` AND calls `m_simulation.restart()`; `ICRegistry::onFileChanged` (`ICRegistry.cpp:93-97`) wraps in `SimulationBlocker` AND calls `setCircuitUpdateRequired()`. `removeICFile` does neither. → actionable item [#14](#a14-removeicfile-h2-shape-crash).

2. **`WorkSpace::save` inline-IC sub-IC conversion** (`Workspace.cpp:95-138`). For inline-IC tabs, iterates file-backed sub-ICs and calls `ic->loadFromBlob(...)` at line 120 to convert them to embedded. `loadFromBlob` → `resetInternalState` → `setInputSize(0)` + `setOutputSize(0)` destroys child ports → cascade-deletes connections. **No `SimulationBlocker`. No `setCircuitUpdateRequired()`.** If the inline IC tab's simulation is running, the next tick references the freed connections → crash. Edge-case in practice (user must be actively editing an inline IC AND have play running). → actionable item [#15](#a15-inline-ic-save-data-loss).

> **Path #2 is also a SILENT DATA-LOSS bug.** `loadFromBlob` destroys the IC's external ports → all scene wires connecting to those ICs are cascade-deleted, then `loadBoundaryPorts` re-creates the ports with the same shape. **The user's wires to file-backed sub-ICs disappear** when they save an inline IC tab. The crash is a symptom; the data loss is the more serious user impact.
>
> The sibling paths `embedICsByFile` and `extractToFile` use the same `loadFromBlob` mutation but are saved by an immediately-following `UpdateBlobCommand` push (which captures connections via `captureConnections` BEFORE the destructive mutation, then rebuilds them by ID lookup in the new ports). Single-threaded Qt event loop guarantees no sim tick interleaves between mutation and command push. `WorkSpace::save` doesn't push `UpdateBlobCommand` at all — that's why it loses data while the others don't.

### Cluster D root causes (still pending)

Six confirmed paths leave stale state in the undo stack. All match the cluster D defensive-throw signature.

1. **`MainWindow::removeICFile`** — also a crash bug (above). → actionable [#14](#a14-removeicfile-h2-shape-crash).
2. **`ICRegistry::onFileChanged`** (`ICRegistry.cpp:80-100`) calls `IC::loadFile` directly which cascades into `setInputSize/setOutputSize` → `qDeleteAll` excess ports → wire-cascade-delete. → actionable [#5](#c5-icregistryonfilechanged--updateblobcommand).
3. **`WorkSpace::save` inline-IC sub-IC conversion** (above). → actionable [#6](#c6-worksspacesave-inline-ic-conversion-wrap).
4. **`UpdateBlobCommand::reconnectConnections`** (`Commands.cpp:1114-1155`) silently drops connections when the new IC has fewer ports than before (lines 1127-1131 bounds check + 1134-1136 silent continue) — orphans connection IDs in the undo stack. → actionable [#7](#c7-updateblobcommand-port-shrink-capture).
5. **`WorkSpace::removeEmbeddedIC`** (`Workspace.cpp:678`) calls `removeBlob` directly outside the command — `DeleteItemsCommand` restores the IC on undo but the blob is gone, IC has invalid blob reference. Documented as known limitation in `TestICInline.cpp:2381-2384, 4344, 4348-4353`. → actionable [#8](#c8-removeembeddedic-blob-removal-via-command).
6. **`ICRegistry::renameBlob`** called directly from `ElementEditor.cpp:638` (NOT in a command). Renames blob in registry map, updates IC instances' `blobName`, AND rewrites blob references inside other blobs' metadata (`ICRegistry.cpp:151-153`) — none of this is undoable. `UpdateCommand` at line 654 only restores element state, not registry state. On undo, ICs reference a blob name that no longer exists. (No specific actionable assigned — same pattern as #5/#7; bundle when fixing those.)

**Compare correct paths:** `ICRegistry::embedICsByFile`, `extractToFile`, `WorkSpace::onChildICBlobSaved` all push `UpdateBlobCommand` correctly.

### Heap corruption / paint-time use-after-free during undo

| ID | Lifetime | Verification |
|---|---:|---|
| `G1`, `GP` | 3 | Full trace of `~QNEConnection`, `~QNEInputPort`/`~QNEOutputPort`, `drainConnections`, `detachConnection`, `setStartPort`/`setEndPort`, `changePortAttachment`, and `CommandUtils::deleteItems`/`loadList`. The cascade is symmetric and idempotent. **No double-free in the static code.** The Qt-internal `QHash::freeData` corruption in the crash signature suggests the issue is inside `QGraphicsItemPrivate`'s scene-index hash — possibly an `itemChange` callback firing during destruction, or BSP-index iterator invalidation. **Cannot resolve without sanitizer/debugger.** |
| `FH` | 1 | `_purecall` from paint event → `effectiveBoundingRect`. Pure-virtual call = vtable destroyed. In single-threaded Qt, paint events only fire during event-loop iteration; commands run synchronously inside one event handler so a paint mid-delete shouldn't happen. The H2 fix's `SimulationBlocker` covers timer-driven races. **No specific bad path identified.** |
| `EW` | 2 | `QGraphicsSceneFindItemBspTreeVisitor::visit` SIGSEGV during paint. Same family as FH. Same conclusion: needs runtime trace. |

→ Tier-E [needs runtime tools](#tier-e--needs-runtime-tools).

### Stale signal-slot connections

| ID | Lifetime | Verification |
|---|---:|---|
| `GW`, `EM`, `EN` | 6 | Audited every `connect()` between long-lived UI and short-lived elements in `Scene/MainWindow/ElementEditor/ContextMenu/Dialogs`. `MainWindow::changeTab` properly disconnects from the old scene's signals (lines 1175-1187) before reconnecting (1217-1226). `ElementEditor::setScene` does the same (lines 213-226). Lambdas captured `this` (long-lived UI) with scene as signal source — Qt cleans these up automatically when source is destroyed. Tab-close uses `deleteLater` which fires after the next `setScene(newScene)` swap. **No stale-receiver pattern found.** |

→ Tier-E [needs runtime tools](#tier-e--needs-runtime-tools).

### Other

| ID | Lifetime | Verification |
|---|---:|---|
| `FP` | 1 | `MorphCommand::redo` → `transferConnections` → `setStartPort` → `QNEPort::attachConnection` → `m_connections.contains()` SIGSEGV. The NEW port's `m_connections` `QList` is corrupt at attach time. The new element was just constructed via `ElementFactory::buildElement(newType)`. For the QList to be corrupt, the port object itself must be corrupt — heap corruption upstream. Single event; cannot pin without runtime tools. |

→ Tier-E [needs runtime tools](#tier-e--needs-runtime-tools).

---

## 🌐 NOT OUR BUG — environmental (16 issues, ~339 events)

All verified at throw site.

### Filesystem permission / file missing on save / copy

| ID | Lifetime | Throw site | Reason |
|---|---:|---|---|
| `EZ`, `13`, `G5` | 302 | `Serialization.cpp:353` | `srcFile.copy()` returned false |
| `GV`, `GN`, `GT`, `G4`, `FK`, `GQ` | 24 | `Workspace.cpp:226` | `QSaveFile::commit()` returned false (e.g. OneDrive-locked file) |
| `FR` | 2 | `MainWindow.cpp:1879` | `QFile::remove()` returned false (file already gone) |
| `F3` | 2 | `Workspace.cpp:197` | `QSaveFile::open()` returned false |

> **Heavy-user pattern.** A small number of users running wiRedPanda from sync-managed cloud folders (OneDrive, Google Drive Desktop) generate the majority of these events. One identifiable user (`lucas`, multiple releases) accounts for ~70+ of these alone — pattern is: download ZIP → extract to OneDrive/Desktop folder → repeated save failures because OneDrive locks the .panda during sync. **These events represent a small number of users hitting the same wall, not a wide-spread problem.** The lifetime counts in this triage are inflated by repeat-hits; once `shouldSendToSentry` 5s dedup is in (5.0.0+), counts under-represent occurrences but only after that release.

→ The right fix is **not** an allowlist — it's gracefully recovering. See actionable [#24](#b24-graceful-save-failure-recovery).

### File-not-found from File>Open / drag-drop autosave

| ID | Lifetime | Throw site |
|---|---:|---|
| `H5`, `H6` | 3 | `Workspace.cpp:299` "This file does not exist" |
| `GS` | 1 | same path, drag-drop autosave gone |

### Qt platform plugin init failure

| ID | Lifetime | Reason |
|---|---:|---|
| `F1` | 4 | `qFatal` from inside Qt's `init_platform` — missing runtime dep on EndeavourOS. `deploy.yml:187-188` has an explicit AppRun fixup for go-appimage's hardcoded `libqxcb.so` deployment bug; F1 events are all on 4.3.0 (pre-fixup) and ramp off as users upgrade. |
| `G2` | 1 | `qt_message_fatal` from same path on 32-bit Windows. Already mitigated by dropping 32-bit Windows from CI (`windows-latest` is x86_64 only). |

---

## ❓ UNDETERMINED — Sentry SDK / 32-bit Windows noise (12 issues, 35 events)

All `wcstombs`-only stack with no first-party frames. **All on `arch: x86` Windows builds.** Almost certainly Sentry SDK crash-handler or 32-bit-only stack walker corruption — not a wiRedPanda code bug. Already silenced by deprecating 32-bit Windows builds.

`G6` (16), `G9` (5), `GK` (1), `GB` (3), `G8` (3), `GF` (1), `GA` (1), `G7` (1), `GG` (1), `GD` (1), `GC` (1), `GE` (1).

---

## Release status — pending in master

Two crash-fix commits sit on master since 5.0.1 was cut. Neither is in any released binary:

| Commit | Date | Closes |
|---|---|---|
| `1e62afc91` | 2026-04-25 00:17 -0300 | H9 / EV / H1 (drag-then-delete `QPointer` guard) |
| `e574b384c` | 2026-04-25 00:25 -0300 | Phase 3 null-check + ConnectionManager `SimulationBlocker` hardening |

5.0.1 was tagged at `9961bd459` on 2026-04-24 09:41 -0300, so **both fixes missed the release window by ~15h**. Sentry has already recorded one H9 event on 5.0.1 (`e41ab503`, 2026-04-24 22:48) confirming the crash is still live.

**Action:** Cut 5.0.2 to actually close H9/EV for users. The actionable list below should bundle into the same release.

### Release adoption picture (last 30 days, 1801 events)

| Release | Events | % | Auto-update prompt? | Has H2 fix? | Has mouseMove fix? | Has H9/EV fix? |
|---|---:|---:|:-:|:-:|:-:|:-:|
| 4.3.0 | 1689 | 94.0% | ❌ | ❌ | ❌ | ❌ |
| 4.2.6 | 81 | 4.5% | ❌ | ❌ | ❌ | ❌ |
| 4.7.0 | 26 | 1.4% | ❌ | ❌ | ❌ | ❌ |
| 5.0.1 | 3 | 0.16% | ✅ | ✅ | ✅ | ❌ |
| 4.2.1 | 2 | 0.1% | ❌ | ❌ | ❌ | ❌ |

**The auto-update prompt** (commit `eab32af81`, 2026-04-23) first shipped in 5.0.0. **99.84% of currently-crashing users (4.7.0 and earlier) have no in-app upgrade prompt.** Even a perfect 5.0.2 with all 23 actionable fixes only directly helps the 3 events/30d on 5.0.1. Cluster D's 1010 events on 4.3.0 cannot be fixed for those users without an out-of-band notification mechanism.

### Sentry feature availability by release

The Sentry-instrumentation commit `a5e898492` ("feat: improve Sentry integration — user ID, breadcrumbs, tags, dedup", 2026-04-23) landed AFTER 4.7.0:

| Feature | 4.3.0 | 4.7.0 | 5.0.0+ | master |
|---|:-:|:-:|:-:|:-:|
| Sentry crash reporting (basic) | ✅ | ✅ | ✅ | ✅ |
| User ID tagging | ❌ | ❌ | ✅ | ✅ |
| Breadcrumbs | ❌ | ❌ | ✅ | ✅ |
| `shouldSendToSentry` 5s dedup | ❌ | ❌ | ✅ | ✅ |
| H2 fix | ❌ | ❌ | ✅ | ✅ |
| `mouseMoveEvent` recursion fix | ❌ | ❌ | ✅ | ✅ |
| H9/EV drag-snapshot fix | ❌ | ❌ | ❌ | ✅ |
| Auto-update prompt | ❌ | ❌ | ✅ | ✅ |

`user.id: null` for 1772/1801 events (98.4%), all on releases without user-ID tagging. This means the lifetime event counts above **don't represent unique users hitting issues** — they could mostly be repeat hits from a small heavy-user cohort. Cluster D's 1010 events on 4.3.0 might be a few users hitting it 200x each, or 1000 users hitting it once. Without user.id tagging on 4.3.0 we can't tell.

### OS distribution (last 30d)

- **Linux: 952 events (53%)** — slight majority, reflecting Brazilian university Linux labs.
- **Windows: 849 events (47%)**.
- **macOS: 0** — wiRedPanda has near-zero macOS user base.

---

## Forensic data quality on 5.0.x — breadcrumb pollution

Both 5.0.x events I have breadcrumbs for show severe forensic degradation:

- **H9 event** (user `e41ab503`, 2026-04-25 01:48): 70/100 slots filled with synthetic `Input count: -1` / `Input count: 0` / `Output count: -1` / `Output count: 0` from combo-box rebuilds. The actual `Drag → Delete` repro survives only because it happened in the last 100ms.
- **FN event** (`dfa0387084e643533689a01f176cdb05`): ~66/100 slots filled with `Simulation stopped`/`Simulation started` pairs from `SimulationBlocker` scopes. The `Add IC` action that triggered the throw is evicted.

### Three pollution sources

| Source | Per-action cost | Fix |
|---|---:|---|
| `ElementEditor::inputIndexChanged`/`outputIndexChanged` emit `sentryBreadcrumb` BEFORE the empty-elements early-return (`ElementEditor.cpp:664, 679`); combo-box `clear()` + `addItem()` in `setCurrentElements` (line 346) is NOT wrapped in `QSignalBlocker` (other combo-boxes already are: lines 234, 451, 471, 483) | ~5 spurious breadcrumbs per element selection | [#21](#b21-breadcrumb-pollution--combo-box) |
| `Simulation::stop()`/`start()` (`Simulation.cpp:184, 193`) emit `sentryBreadcrumb` unconditionally; `SimulationBlocker` calls these on every scope; every command yields 3 breadcrumbs (sim-stop, command, sim-start) | 2 noise breadcrumbs per `SimulationBlocker` scope | [#22](#b22-breadcrumb-pollution--simulationblocker) |
| `MainWindow::on_actionRotateRight_triggered` etc. emit a breadcrumb, then `Scene::rotateRight` emits the same breadcrumb. Affects Rotate (×2), Flip (×2), Zoom (×3) — 7 sites total. Scene/View ones carry richer context (zoom level number); MainWindow ones are pure duplicates | 1 redundant breadcrumb per affected user action | [#23](#b23-breadcrumb-pollution--mainwindow-duplicates) |

**Combined effect.** Without these fixes: 6-9 breadcrumbs per single user action, 100-slot buffer fills in 11-17 actions. With all three applied: 2-3 per action, buffer holds 33-50 actions — enough to capture a meaningful editing session leading up to a crash.

This may explain why D2/FJ/CR ("Invalid type. Data is possibly corrupted.") is so hard to root-cause from Sentry data — by the time the autosave file gets reopened on next launch, the breadcrumbs from the prior session that wrote the corrupt file are gone, and they may have been polluted before then anyway.

**Combined fixes #21-#23 are ~20 lines of code change** and apply to every future crash report on 5.0.x+, regardless of which specific bug fires.

---

## Findings beyond Sentry signatures

Code-level issues uncovered during the triage that don't currently match a Sentry signature but are worth fixing.

### Exception-safety leaks
- **`MainWindow::loadPandaFile`** (`MainWindow.cpp:705-716`) unconditionally `createNewTab()`s BEFORE `m_currentTab->load(fileName)`. If `load` throws (corrupt file, missing IC dep, etc.) the new tab stays in the tab widget — empty. **`loadAutosaveFiles`** wraps in try/catch but only removes the autosave file from settings; it does NOT close the orphaned empty tab. Three corrupt autosaves at startup → 3 message boxes AND 3 empty tabs. → actionable [#11](#b11-loadpandafile-trycatch--closetab).
- **`Scene::handleNewElementDrop`** (`Scene.cpp:746`) and twin handler at `Scene.cpp:1085-1123` allocate `element = ElementFactory::buildElement(type)`, then `element->loadFromDrop(...)` which can throw. `element` is leaked. The duplicate handler additionally leaks `mimeData` because `mimeData->deleteLater()` is unreachable on throw. → actionable [#12](#d12-element-leak-in-drop-handlers).
- **`SplitCommand::redo`** (`Commands.cpp:559-567`) allocates new `QNEConnection` and `Node`, then throws if findConn/findElm fails for upstream items. Just-allocated conn2/node aren't on the scene yet — leaked. ~49 events lifetime. → actionable [#13](#d13-element--splitcommand-exception-safety).

### `ElementEditor::setScene` lambda accumulation
`ElementEditor.cpp:216` disconnects via `&Scene::truthTableElementChanged, this, &ElementEditor::truthTable` (member-function pointer). Line 225 connects via `[this](GraphicElement *) { truthTable(); }` (lambda). **Disconnect doesn't match the lambda** — Qt can't disconnect by lambda. Each tab switch ADDS a new lambda connection without removing the previous. Cleaned up at scene destruction (Qt auto-disconnect from dead source), so not a leak across sessions, but accumulates within. **Fix:** change line 225 to use `&ElementEditor::truthTable` to match the disconnect.

### `MainWindow::setupShortcuts()` called twice
`MainWindow.cpp` line 121 calls `setupShortcuts()`, line 138 calls it again. `setupShortcuts` (lines 251-265) unconditionally `new`s 7 QShortcut objects. The first call's shortcuts are never deleted (parent-child cleanup at MainWindow destruction means they live for the entire session). `Ctrl+F` is bound twice (idempotent); `[`, `]`, `{`, `}`, `<`, `>` each have two QShortcut objects — Qt may emit "Ambiguous shortcut overload" warnings to stderr. → actionable [#18](#d18-duplicate-setupshortcuts-call).

### `CircuitExporter::renderToImage` missing error checks
`CircuitExporter.cpp:43-56` — `painter.begin(&pixmap)` return ignored; `pixmap.save(filePath)` return ignored. User sees no error if export silently fails. Compare `renderToPdf` (line 35) which checks `painter.begin(&printer)` and throws. → actionable [#19](#d19-rendertoimage-error-checking).

### `DolphinSerializer` negative row count
`Serializer.cpp` `loadBinary` line 31 reads `qint64 rows`; line 34-36 clamps to `maxInputPorts` if too large but **doesn't reject negative values**. Line 45 calls `data.values.resize(data.inputPorts * data.columns)` — `resize` on negative argument is UB. Same in `loadCSV` line 88, 110. Theoretical only (binary format hard to hand-edit). → actionable [#20](#d20-dolphinserializer-negative-row-count).

### `FileUtils::copyPandaDeps` and `Serialization::copyPandaFile` lack cycle detection
Both recurse on `fileBackedICs` metadata (`FileUtils.h:42-68`, `Serialization.cpp:348-378`) without a visited-set. A circular `fileBackedICs` (A → B → A → ...) infinite-recurses → stack overflow. **`ICRegistry::makeBlobSelfContained`** (`ICRegistry.cpp:305-385`) correctly uses a visited-set (line 308-312); the pattern just isn't applied uniformly. Theoretical only (.panda is binary). → actionable [#16](#d16-cycle-detection-in-copypandadeps).

### `FileUtils::copyToDir` and `copyPandaDeps` silently ignore copy failures
`copyToDir` (`FileUtils.h:36`) calls `QFile::copy` without checking the return value. Same at lines 61, 65 in `copyPandaDeps`. Save-As to a read-only or full disk silently fails to copy external dependencies (audio, sub-ICs); the `.panda` is still saved successfully but references files that don't exist in the new location. UX/data-integrity bug. → actionable [#17](#d17-copy-failure-propagation).

### Pre-existing FIXMEs (not Sentry-correlated)
- `App/Element/GraphicElement.h:690` — multi-driver bus simulation issue.
- `App/Scene/Commands.cpp:800` — `verticalFlip` is rotating on the horizontal axis.

---

## Architecture-level safety verifications

Confirmed during the audit; recorded so a future investigation doesn't re-derive them.

- **No threading in `App/`** — grep for `QThread`/`std::thread`/`QtConcurrent`/`QThreadPool`/`moveToThread` returns zero matches. Single-threaded application. No data-race crashes possible from our code. Qt audio (`QMediaPlayer`/`QAudioSink`) uses Qt-internal threading but our usage is just function calls.
- **No `qFatal`/`Q_ASSERT`/`Q_UNREACHABLE`/`abort()` in `App/`** — confirmed via grep. No programmer-side crash points; every error path either throws PANDACEPTION (caught by `Application::notify` → message box) or returns silently.
- **Sentry init order** — `sentry_init` runs at `Main.cpp:59` BEFORE `QApplication` construction, so crashes during Qt platform plugin init (F1, G2) are captured by the crashpad backend.
- **MCP handlers safe** — 14 handlers in `MCP/Server/Handlers/`; all use `scene->receiveCommand(new XCommand(...))`. Command construction wrapped in try/catch with manual `delete element` on failure (`ElementHandler.cpp:115-123`). No direct scene mutation. No MCP-related crashes in 90 days.
- **All 14 element save/load pairs symmetric** — `GraphicElement` base + 12 subclasses + `QNEConnection`. Stream-position desync from element-level asymmetry is structurally impossible.
- **`QDataStream::Qt_5_12` version pin** (`Serialization.cpp:21, 28`) keeps `QVariant` binary-stable across Qt 5/6.
- **`Settings`** uses Qt `IniFormat`/`UserScope` with typed accessors; Qt handles QSettings atomicity. No autosave-style corruption window.
- **`VisibilityManager`** holds only `Scene *m_scene`; never caches element pointers between calls. Every `showGates`/`showWires`/`reapply` re-iterates `scene->items()`. Safe across element deletion.
- **`IC::loadFile` cycle detection** (`IC.cpp:338-344`) uses static `s_loadingFiles` `QSet<QString>` with `qScopeGuard` cleanup. Safe across throws.
- **`simulationWarning` signal** (`Simulation.cpp:234`) emitted at most once per `Simulation` instance (`m_convergenceWarned` latch). `disconnect(..., this, nullptr)` at `MainWindow.cpp:1176` cleanly tears down on tab change.
- **CodeGen throws** (`ArduinoCodeGen.cpp` + `SystemVerilogCodeGen.cpp`, ~9 sites) are user-facing errors raised inside synchronous "Generate Code" actions, with valid usage feedback ("Not enough pins available", etc.). None match current Sentry data.
- **`ElementFactory::buildElement`** (`ElementFactory.cpp:64-79`) throws on `Unknown` or missing creator — corrupt/forward-version file or build-time registration miss. Not a crash source.
- **`Scene::keyPressEvent`/`eventFilter`/`mouseDoubleClickEvent`** all snapshot-iterate or push commands properly.
- **`MainWindow::~MainWindow`** + scene member declaration order — `m_simulation` declared before `QGraphicsScene` items, so simulation stops before items destruct. No stale-pointer access from ticks at destruction.
- **Batch / non-interactive mode** (`Main.cpp:288-342`) — outer try/catch at `Main.cpp:372` → stderr + `exit(1)`. MainWindow destructor runs even on throw via stack unwinding; `disconnectTab` null-guards correctly.
- **No `QLockFile` / `QSharedMemory` / single-instance check** — wiRedPanda allows multiple concurrent processes. They share QSettings; race on `Settings::setAutosaveFiles` mutations could lose autosave entries. Edge case, not crash-class.

---

## Actionable list (tier-ordered)

23 items total. Tier ordering reflects impact ÷ effort.

### Tier A — biggest user impact, smallest effort

#### A1. Pixmap throw → debug log — **SKIPPED**
**File:** `App/Element/GraphicElement.cpp:182`
**Closes:** ~691 lifetime events (F8, FF, FG, GY, H3, FM, FB, H4)
**Effort:** 1 line
**Change:** Replace `throw PANDACEPTION(...)` with `qCDebug(zero) << ...`. Line 180 already loads default appearance — no functional change.
**Status:** Skipped — keep the user-visible message-box on missing pixmaps.

#### A14. `removeICFile` → `DeleteItemsCommand`
**File:** `App/UI/MainWindow.cpp:1856-1885`
**Closes:** Real H2-shape crash + cluster D #1
**Effort:** ~10 lines
**Change:** Replace ad-hoc `removeItem` + `delete` loop with `scene->receiveCommand(new DeleteItemsCommand(...))`. Mirror `Scene::deleteAction`'s pattern (also calls `m_simulation.restart()`).

#### A15. Inline-IC save data-loss + crash fix
**File:** `App/Scene/Workspace.cpp:120`
**Closes:** Silent wire deletion when saving inline IC tabs + H2-shape crash if sim is running
**Effort:** ~5 lines
**Change:** Wrap the `loadFromBlob` conversion in `SimulationBlocker` + `setCircuitUpdateRequired()`, OR skip the `loadFromBlob` call when the IC is already on the scene (the destructive port-recreation is unnecessary in that case).

### Tier B — meaningful fixes

#### B2. Autosave: `QTemporaryFile` → `QSaveFile`
**File:** `App/Scene/Workspace.h:136` + `Workspace.cpp:519-529`
**Closes:** ~192 events (D2, FJ, CR root cause — both partial-write and shrink-leftover-tail modes)
**Effort:** ~10 lines

#### B3. Autosave debounce timer (500ms)
**File:** `App/Scene/Workspace.cpp` (constructor + new slot)
**Closes:** Reduces autosave write rate ~10x; smaller crash-corruption window
**Effort:** ~5 lines

#### B11. `loadPandaFile` try/catch + `closeTab`
**File:** `App/UI/MainWindow.cpp:705-716`
**Closes:** Empty-tab leak compounding D2/FJ/CR + H5/H6/GS UX
**Effort:** ~5 lines
**Change:** Wrap `m_currentTab->load(fileName)` in try/catch; on throw, close the just-opened tab using the existing `closeTab(...)` pattern from line 1293, then rethrow.

#### B21. Breadcrumb pollution — combo-box
**File:** `App/UI/ElementEditor.cpp` lines 346, 664, 679
**Closes:** ~70% of breadcrumb buffer pollution; multiplies forensic value of every future crash report
**Effort:** ~10 lines
**Change:** (a) Move `sentryBreadcrumb` calls AFTER the `m_elements.isEmpty() || !isEnabled()` guard at lines 665, 681. (b) Wrap the combo-box `clear()` + `addItem()` + `setCurrentIndex()` block in `setCurrentElements` (~lines 346-368) in `QSignalBlocker` — matches existing pattern at lines 234, 451, 471, 483.

#### B22. Breadcrumb pollution — SimulationBlocker
**File:** `App/Simulation/Simulation.cpp:184, 193`
**Closes:** 2 noise breadcrumbs per `SimulationBlocker` scope
**Effort:** 2 lines
**Change:** Delete `sentryBreadcrumb` calls in `stop()` and `start()`. The user-meaningful "play/pause toggled" event is already captured by `[simulation] Play toggled: 1` at `MainWindow.cpp:1640`. SimulationBlocker scopes are internal mechanism, not user actions.

#### B23. Breadcrumb pollution — MainWindow duplicates
**File:** `App/UI/MainWindow.cpp` lines 691, 699, 1428, 1438, 1448, 1731, 1741
**Closes:** 1 redundant breadcrumb per Rotate / Flip / Zoom action
**Effort:** 7 lines (deletions)
**Change:** Delete the MainWindow-side breadcrumbs. Scene/View ones (`Scene.cpp:637, 643, 670, 680`; `GraphicsView.cpp:173, 181, 189`) carry richer context (zoom level number) and fire regardless of how the action was reached.

#### B24. Graceful save-failure recovery (Save-As re-prompt)
**Files:** `App/Scene/Workspace.cpp:225-227` + sibling sites at `:197`, `Serialization.cpp:353`, `MainWindow.cpp:1879`
**Closes:** ~330 lifetime events (EZ, 13, G5, GV, GN, GT, G4, FK, GQ, F3, FR family) — real fix instead of suppression
**Effort:** ~15 lines per call site

When a user does `Ctrl+S` and the target is read-only (OneDrive lock, ZIP-extracted folder, network drive, write-protect attribute), the app currently throws → modal "Acesso negado" → user is stuck. The autosave path at `Workspace.cpp:502-512` already has read-only-aware fallback (writes to `AppDataLocation/autosaves/`) — user-initiated saves should be similarly aware.

**Implementation sketch** (`WorkSpace::save`):

```cpp
if (!saveFile.commit()) {
    if (Application::interactiveMode &&
        saveFile.error() == QFileDevice::PermissionsError) {
        const QString newPath = FileDialogs::provider()->getSaveFileName(
            this, tr("Save File (original location is read-only)"),
            QFileInfo(fileName_).fileName(),
            tr("Panda files (*.panda)"));
        if (newPath.isEmpty()) {
            return;  // user cancelled
        }
        save(newPath);  // recurse with new path
        return;
    }
    throw PANDACEPTION("Could not save file: %1", saveFile.errorString());
}
```

| Sentry IDs | Lifetime | Throw site | Fixed by #24? |
|---|---:|---|:-:|
| EZ, 13, G5 | 302 | `Serialization.cpp:353` (Save-As file copy) | partial — see [#17](#d17-copy-failure-propagation) |
| GV, GN, GT, G4, FK, GQ | 24 | `Workspace.cpp:226` (`QSaveFile::commit`) | ✅ |
| F3 | 2 | `Workspace.cpp:197` (`QSaveFile::open`) | ✅ |
| FR | 2 | `MainWindow.cpp:1879` (`QFile::remove`) | partial — needs Retry dialog |

### Tier C — quality of life

#### C5. `ICRegistry::onFileChanged` → `UpdateBlobCommand`
**File:** `App/Element/ICRegistry.cpp:80-100`
**Closes:** Cluster D root cause #2
**Effort:** ~20 lines (mirror `embedICsByFile` pattern)

#### C6. `WorkSpace::save` inline-IC conversion wrap
Same as [A15](#a15-inline-ic-save-data-loss--crash-fix) — listed twice because it's both Tier A (data loss/crash) and the cluster D #3 fix.

#### C7. `UpdateBlobCommand` port-shrink capture
**File:** `App/Scene/Commands.cpp:1114-1155`
**Closes:** Cluster D root cause #4
**Effort:** ~15 lines
**Change:** Capture dropped connections in old data so undo can restore them. Currently silent-continues at lines 1134-1136.

#### C8. `removeEmbeddedIC` blob removal via command
**File:** `App/Scene/Workspace.cpp:678`
**Closes:** Cluster D root cause #5 (known limitation, see `TestICInline.cpp`)
**Effort:** ~10 lines
**Change:** Push blob removal as part of the command, not eagerly.

#### C9. UX action enable-state
**Files:** `App/UI/MainWindow.cpp` (Add IC button) + `App/BeWavedDolphin/BeWavedDolphinUI.cpp:65` (Set Clock Wave)
**Closes:** ~33 events (FN, X, CV, G3)
**Effort:** ~6 lines × 2

#### C10. `shouldSendToSentry` allowlist
**File:** `App/Core/Application.cpp:20-34`
**Closes:** Environmental noise that doesn't have a UX-recovery path (qFatal from Qt platform plugin init, F1/G2)
**Effort:** ~15 lines
**Note:** After [#24](#b24-graceful-save-failure-recovery) is in place, this becomes mostly redundant for the filesystem cluster. Keep only for genuinely no-recovery messages.

### Tier D — hardening

#### D12. Element leak in drop handlers
**Files:** `App/Scene/Scene.cpp:746` + duplicate handler at `:1085-1123`
**Closes:** Memory leak on bad drop (single allocation per failed drop)
**Effort:** ~6 lines (`std::unique_ptr` rewrite, or try/catch with `delete` + rethrow)

#### D13. `SplitCommand` exception safety
**File:** `App/Scene/Commands.cpp:559-567`
**Closes:** Memory leak on GM/FQ throw (~49 events lifetime, ~96 bytes per throw)
**Effort:** ~4 lines

#### D16. Cycle detection in `copyPandaDeps` / `copyPandaFile`
**Files:** `App/IO/FileUtils.h:42-68`, `App/IO/Serialization.cpp:348-378`
**Closes:** Theoretical stack overflow on crafted metadata
**Effort:** ~5 lines × 2 (mirror `ICRegistry::makeBlobSelfContained:308-312`)

#### D17. Copy-failure propagation
**File:** `App/IO/FileUtils.h:36, 61, 65`
**Closes:** Silent partial Save-As to read-only volume
**Effort:** ~10 lines

#### D18. Duplicate `setupShortcuts()` call
**File:** `App/UI/MainWindow.cpp:138`
**Closes:** 7 leaked QShortcuts + ambiguous-shortcut warnings
**Effort:** 1 line (deletion)

#### D19. `renderToImage` error checking
**File:** `App/UI/CircuitExporter.cpp:43-56`
**Closes:** Silent export failure UX bug
**Effort:** ~5 lines (mirror `renderToPdf` at line 35)

#### D20. `DolphinSerializer` negative row count
**File:** `App/BeWavedDolphin/Serializer.cpp:31, 88`
**Closes:** Theoretical UB on crafted .dolphin file
**Effort:** ~3 lines (add `rows < 0` check next to existing `cols` validation)

### Tier E — needs runtime tools

These cannot be resolved by static analysis. Sanitizer / debugger / Sentry session replay required.

- **`G1`/`GP`/`FH`/`EW` heap-corruption family** (6 events): paint-time use-after-free in `QGraphicsItemPrivate`'s BSP index. Cascade is symmetric and idempotent in static code; corruption signature suggests `itemChange` callback firing during destruction, or BSP-index iterator invalidation.
- **`FP` MorphCommand crash** (1 event): port object corruption needs runtime trace.
- **`GW`/`EM`/`EN` stale-slot family** (6 events): full audit found no specific path. All disconnects are correctly paired. Possibly a third-party path missed, or tab-close during signal emission.

---

## Recommended 5.0.2 release bundle

~55 lines of code change, would close 4 STILL-VULNERABLE crash bugs + ~192 events worth of noise + ~3x improvement in forensic data quality.

1. The 2 pending master commits (`1e62afc91` + `e574b384c`) — already done, just needs a release.
2. ~~**A1** — pixmap throw → debug log~~ — **SKIPPED**
3. **A14** — `removeICFile` crash (~10 lines)
4. **A15** — inline-IC save data-loss + crash (~5 lines)
5. **B2 + B3** — autosave atomicity + debounce (~15 lines)
6. **B11** — `loadPandaFile` try/catch (~5 lines)
7. **B21 + B22 + B23** — breadcrumb forensics (~20 lines, multiplies value of every future crash report)
8. **B24** — graceful save-failure recovery (~15 lines, real fix for OneDrive-lock cluster)

---

## Methodology caveats

- Per-issue verdicts based on static reading of current `master`. Not runtime-validated.
- For ⚠️ items, lacking a runtime repro, the hypothesized root cause may be wrong. Hypotheses noted explicitly.
- Filesystem-cluster verdicts assume the Sentry-reported message reliably maps to the throw site whose message format matches.
- Issue-event counts are 365-day totals; older events out of scope.
- **Lifetime event counts SIGNIFICANTLY undercount actual occurrences for repeated errors** because `shouldSendToSentry` (5.0.0+) deduplicates exact-message matches within a 5-second cooldown. The 192 D2/FJ/CR events could represent thousands of actual user-side occurrences across sessions.
- **Event counts reflect repeat hits from a small heavy-user cohort, not unique users.** With `user.id: null` for 98.4% of events on 4.3.0/4.7.0 builds, we can't separate "many users hit once" from "a few users hit many times." Heavy-user analysis suggests the latter dominates filesystem-cluster IDs.
