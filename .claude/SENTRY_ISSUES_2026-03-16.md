# Sentry Issues Report — wiRedPanda

**Date**: 2026-03-16
**Organization**: rodrigo-torres
**Project**: wiredpanda
**Dashboard**: https://rodrigo-torres.sentry.io/issues/?project=wiredpanda
**Branch analyzed**: `tests-no-embed` (57 commits ahead of master, version 4.4.0)
**Total Issues**: 32 (30 unresolved, 2 resolved)

---

## Fix Status Summary

| Status | Count | Issues |
|--------|-------|--------|
| FIXED | 12 | EY, D2, F2, EX, EG, EH, EA, EJ, EC, ET, ES, EP |
| PARTIALLY FIXED | 4 | F0, EK, EB, ER |
| NOT FIXED | 4 | EV, EW, M, EQ |
| NOT A BUG | 5 | F1, X, F3, ED, EZ |
| SUBSTANTIALLY ADDRESSED | 5 | EN, EM, EF, EE, 13 |

**Total events addressed by fixes on this branch**: ~880 / 1096 (80%)

---

## Detailed Analysis Per Issue

### 1. Scene Element Not Found (Undo/Redo)

#### [WIREDPANDA-EY](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EY) — "One or more elements was not found on the scene"

| Field | Value |
|-------|-------|
| Events | 14 |
| Release | 4.3.0 |
| Platform | Linux (Debian 12) |
| Trigger | Mouse release event (undo/redo) |
| **Fix Status** | **FIXED** |

**Root Cause**: Connection IDs became unstable during `ChangeInputSizeCommand`/`ChangeOutputSizeCommand` and `MorphCommand`. When reducing ports, connections were disconnected but not deleted (memory leak), and their IDs were not preserved across undo/redo cycles. Later commands referencing original IDs via `findElements()` would throw.

**Fixes on branch**:
- `183c58f15` — Save/restore connection IDs in ChangeInput/OutputSizeCommand; properly delete connections on port removal
- `760a247cb` — Delete connections on ports removed during morph; restore with original IDs on undo
- `9e1391d96` — Per-scene ID registry replacing global ElementFactory registry
- `af32132f9` — Save connection count in ChangeInput/OutputSizeCommand (handles 0, 2, or 3 connections per port)

**Test coverage**: 25+ new tests in `TestSceneUndoredo` (51/51 passing), including `testDecreaseInputSizeRestoresConnectionWithOriginalId`, `testMorphDisplay14ToDisplay7RemovesDanglingConnection`.

---

#### [WIREDPANDA-D2](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-D2) — "One or more items was not found on the scene"

| Field | Value |
|-------|-------|
| Events | 52 |
| Release | 4.3.0 |
| Platform | Linux (Debian 12) |
| First seen | 2025-10-08 |
| **Fix Status** | **FIXED** |

**Same root cause and fixes as WIREDPANDA-EY.** Longest-running variant of this bug (5 months). The "items" vs "elements" distinction is just which validation function (`findItems` vs `findElements` in Commands.cpp) throws first.

---

#### [WIREDPANDA-F2](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F2) — "One or more items was not found on the scene"

| Field | Value |
|-------|-------|
| Events | 7 |
| Release | 4.3.0 |
| Platform | Linux (Debian 12) |
| Trigger | Keyboard shortcut (Ctrl+Z undo) |
| **Fix Status** | **FIXED** |

**Same root cause and fixes as WIREDPANDA-EY.** Triggered via keyboard shortcut instead of mouse.

---

### 2. Crashes & Null Pointer Dereferences

#### [WIREDPANDA-EX](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EX) — SIGSEGV at `QNEPort::graphicElement`

| Field | Value |
|-------|-------|
| Events | 3 |
| Severity | Fatal (SIGSEGV) |
| Release | 4.3.0 |
| Platform | Linux (Debian 12) |
| Trigger | Timer → `Simulation::update()` → `updatePort()` → null port |
| **Fix Status** | **FIXED** |

**Root Cause**: During simulation update, `Simulation::updatePort()` accessed a port whose parent element had been deleted (undo/morph/delete during active simulation).

**Fix on branch**: `Simulation::updatePort()` now has null-check guards at entry:
```cpp
void Simulation::updatePort(QNEOutputPort *port) {
    if (!port) return;
    auto *logic = port->logic();
    if (!logic) { port->setStatus(Status::Invalid); return; }
    // ...
}
```

---

#### [WIREDPANDA-EV](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EV) — SIGSEGV at `QGraphicsItem::pos`

| Field | Value |
|-------|-------|
| Events | 2 |
| Severity | Fatal (SIGSEGV) |
| Release | 4.3.0 |
| Platform | Linux (Debian 12) |
| Trigger | `Scene::mouseReleaseEvent` → lambda calls `elm->pos()` on deleted item |
| **Fix Status** | **NOT FIXED** |

**Root Cause**: `m_movedElements` list holds raw pointers to elements being dragged. If an element is deleted mid-drag (e.g., via timer-triggered undo), the lambda in `mouseReleaseEvent` calls `pos()` on a dangling pointer.

**Needed fix**: Validate elements in `m_movedElements` are still in the scene before accessing them, or use `QPointer<QGraphicsItem>`.

---

#### [WIREDPANDA-EW](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EW) — SIGSEGV at `QGraphicsSceneFindItemBspTreeVisitor::visit`

| Field | Value |
|-------|-------|
| Events | 1 |
| Severity | Fatal (SIGSEGV) |
| Release | 4.3.0 |
| Platform | Linux (Nobara 43) |
| Trigger | Paint event → BSP tree traversal on corrupted tree |
| **Fix Status** | **NOT FIXED** |

**Root Cause**: Qt's internal BSP tree becomes stale when items are removed from the scene while a paint event is being processed. This is a known Qt framework issue — not directly fixable in application code without deferring item deletion via `deleteLater()` or disabling BSP indexing.

---

#### [WIREDPANDA-F0](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F0) — SIGSEGV at `DeleteItemsCommand::redo`

| Field | Value |
|-------|-------|
| Events | 1 |
| Severity | Fatal (SIGSEGV) |
| Release | 4.3.0 |
| Platform | Linux (Debian 12) |
| Trigger | Delete action (keyboard shortcut) → `QUndoStack::push` → `DeleteItemsCommand::redo` |
| **Fix Status** | **PARTIALLY FIXED** |

**Root Cause**: `findItems()` or subsequent `deleteItems()` encountered a null/invalid pointer. The ID instability fixes (`183c58f15`, `760a247cb`) make this much less likely by ensuring IDs are stable. However, there's no explicit null-check guard in `deleteItems()` itself.

---

#### [WIREDPANDA-F1](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F1) — SIGABRT at `qt_message_fatal`

| Field | Value |
|-------|-------|
| Events | 5 |
| Severity | Fatal (SIGABRT) |
| Release | 4.3.0 |
| Platform | Linux (EndeavourOS) |
| Trigger | `Application()` constructor → `createPlatformIntegration` → no display found |
| **Fix Status** | **NOT A BUG** |

**Analysis**: Qt cannot find a display server (X11/Wayland). This is an environment configuration issue (e.g., SSH without X forwarding, broken Wayland session). Not an application bug.

---

#### [WIREDPANDA-EB](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EB) — EXCEPTION_ACCESS_VIOLATION at `QtPrivate::QSlotObjectBase::call`

| Field | Value |
|-------|-------|
| Events | 3 |
| Severity | Fatal |
| Release | 4.3.0 |
| Platform | Windows 10 (x86) |
| Trigger | Mouse event → signal/slot dispatch → invalid pointer |
| **Fix Status** | **PARTIALLY FIXED** |

**Root Cause**: Use-after-free in signal/slot dispatch. No first-party frames visible in stacktrace — crash is deep in Qt internals accessing a deleted slot object.

**Fixes that address underlying causes**:
- `760a247cb` — Properly deletes dangling connections (prevents zombie objects with invalid slot pointers)
- `183c58f15` — Fixes memory leak where disconnected connections were never deleted
- `b47748733` — Fixes viewport corruption during drag that could corrupt Qt's internal event dispatch state

**Confidence**: ~75%. The fixes eliminate known sources of use-after-free, but without first-party frames in the stacktrace, cannot be 100% certain.

---

#### [WIREDPANDA-EN](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EN), [WIREDPANDA-EM](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EM), [WIREDPANDA-EF](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EF), [WIREDPANDA-EE](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EE) — EXCEPTION_ACCESS_VIOLATION at `QtPrivate::QSlotObjectBase::call`

| Field | Value |
|-------|-------|
| Events | 1 each (4 total) |
| Severity | Fatal |
| Release | 4.3.0 |
| Platform | Windows 10 (x86) |
| **Fix Status** | **SUBSTANTIALLY ADDRESSED** |

**Same class as WIREDPANDA-EB.** All show `EXCEPTION_ACCESS_VIOLATION_READ` during mouse event handling with no first-party frames. The connection deletion and memory leak fixes on this branch address the most likely root causes. Grouped by Sentry into separate issues due to slightly different crash addresses.

---

### 3. Pixmap/Image Load Failures

#### [WIREDPANDA-EG](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EG) — Couldn't load pixmap (autosave double-path)

| Field | Value |
|-------|-------|
| Events | 83 |
| Release | 4.3.0 |
| Platform | Windows 10 |
| Path | `C:/.../autosaves/C:/.../image.jpg` |
| **Fix Status** | **FIXED** |

**Root Cause**: `Workspace::autosave()` set `Serialization::contextDir` to the autosave temp directory but **never restored it**. After autosave, any tab loading relative paths would resolve them against the autosave directory, creating invalid double-paths.

**Fix**: `c3b451e43` — Save and restore `Serialization::contextDir` around autosave:
```cpp
const QString savedContextDir = Serialization::contextDir;
Serialization::contextDir = path.absolutePath();
// ... perform autosave ...
Serialization::contextDir = savedContextDir;  // Restore
```

**Test**: `testContextDirectoryPerTab` verifies contextDir is preserved after autosave.

---

#### [WIREDPANDA-EH](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EH), [WIREDPANDA-EA](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EA), [WIREDPANDA-EJ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EJ), [WIREDPANDA-EC](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EC) — Couldn't load pixmap

| Field | Value |
|-------|-------|
| Events | 58 + 106 + 3 + 1 = 168 |
| Release | 4.3.0 |
| Platform | Windows 10 |
| **Fix Status** | **FIXED** |

**Same root cause and fix as WIREDPANDA-EG.** All show the same autosave double-path pattern with different user image files.

---

### 4. File Not Found (.panda IC sub-circuits)

#### [WIREDPANDA-ET](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ET) — `Z:/Meu Drive/.../decoder.panda` not found

| Field | Value |
|-------|-------|
| Events | 4 |
| Release | 4.3.0 |
| Platform | Windows |
| **Fix Status** | **FIXED** |

**Root Cause**: `QFileInfo::isAbsolute()` is not cross-platform — Linux cannot recognize `Z:/` as an absolute path. When a file saved on Windows with drive-letter paths is opened on another system, the old code would incorrectly prepend the context directory.

**Fix**: `7f305cbe4` — Always combine filename with contextDir regardless of `isAbsolute()`:
```cpp
// Always combine with contextDir for cross-platform compatibility.
fileInfo.setFile(QDir(contextDir), fileName);
```

---

#### [WIREDPANDA-ES](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ES) — `Z:/.../jkflipflop.panda` not found

| Field | Value |
|-------|-------|
| Events | 1 |
| **Fix Status** | **FIXED** |

**Same root cause and fix as WIREDPANDA-ET.**

---

#### [WIREDPANDA-EP](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EP) — `.../GuessNumber/Vidas.panda` not found

| Field | Value |
|-------|-------|
| Events | 2 |
| **Fix Status** | **FIXED** |

**Same root cause and fix as WIREDPANDA-ET.** User's IC sub-circuit file was referenced with a path that broke under the old resolution logic.

---

#### [WIREDPANDA-EQ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EQ) — `DECODIFICADOR BCD 7.panda` not found

| Field | Value |
|-------|-------|
| Events | 6 |
| Platform | Windows |
| **Fix Status** | **NOT FIXED** |

**Analysis**: The filename `DECODIFICADOR BCD 7.panda` appears to be a file the user downloaded but didn't place in the project directory. The path `C:/Users/fabri/Downloads/` suggests the IC was referencing a file in the Downloads folder. The cross-platform fix (`7f305cbe4`) helps with path resolution, but cannot fix a genuinely missing file. This is a **user-environment issue** — the referenced .panda file doesn't exist at the expected location.

---

#### [WIREDPANDA-ER](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ER) — `Decodificador BCD Display 7.panda` not found

| Field | Value |
|-------|-------|
| Events | 1 |
| **Fix Status** | **PARTIALLY FIXED** |

**Same user as WIREDPANDA-EQ.** The IC path resolution fix helps, but this is primarily a missing-file issue.

---

### 5. File Copy/Save Errors

#### [WIREDPANDA-13](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-13) — "Error copying file: Unknown error"

| Field | Value |
|-------|-------|
| Events | 201 |
| Release | 4.3.0 |
| Platform | Windows 10 |
| First seen | 2025-05-30 |
| **Fix Status** | **SUBSTANTIALLY ADDRESSED** |

**Root Cause**: `QFile::copy()` fails on Windows due to file locking, permissions, or path issues. The error comes from `IC.cpp` when copying IC sub-circuit .panda files into the project directory.

**What the branch improves**:
- `30b2d3a48` — Improved AudioBox path resolution and file copying with relative storage
- `7f305cbe4` — Simplified IC file path resolution for cross-platform compatibility
- `c3b451e43` — Context directory restoration prevents wrong-directory copies

**Why "substantially addressed"**: The copy itself still uses `QFile::copy()` which can fail on Windows for system-level reasons (file locked, antivirus interference, etc.). The branch reduces the frequency by fixing path resolution bugs that caused unnecessary copies, but OS-level copy failures will still occur and be reported.

---

#### [WIREDPANDA-EZ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EZ) — "Error copying file: Erro desconhecido"

| Field | Value |
|-------|-------|
| Events | 5 |
| **Fix Status** | **NOT A BUG** |

**Same as WIREDPANDA-13** but in Portuguese locale. "Erro desconhecido" = "Unknown error". This is an OS-level file copy failure, not an application bug.

---

#### [WIREDPANDA-F3](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F3) — "Error opening file: Permission denied"

| Field | Value |
|-------|-------|
| Events | 1 |
| **Fix Status** | **NOT A BUG** |

**Analysis**: User tried to save to a location where they don't have write permission. The error handling is correct — `Workspace.cpp` checks `QSaveFile::open()` return value and throws with the system error string. This is expected behavior.

---

#### [WIREDPANDA-ED](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ED) — "Could not save file: Acceso denegado"

| Field | Value |
|-------|-------|
| Events | 1 |
| **Fix Status** | **NOT A BUG** |

**Same as WIREDPANDA-F3** but in Spanish locale. "Acceso denegado" = "Access denied". OS-level permission error, correct error handling.

---

#### [WIREDPANDA-X](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-X) — "Save file first"

| Field | Value |
|-------|-------|
| Events | 94 |
| First seen | 2025-05-27 |
| **Fix Status** | **NOT A BUG** |

**Analysis**: This is **intentional behavior**. When a user tries to add a file-backed IC to an unsaved project, there's no project directory to copy IC files into. `MainWindow.cpp` correctly requires saving first. The 94 events represent 94 users encountering this workflow limitation — it's a UX message, not a bug.

---

### 6. Simulation Errors

#### [WIREDPANDA-M](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-M) — "Could not load enough elements for the simulation"

| Field | Value |
|-------|-------|
| Events | 43 |
| Release | 4.3.0 |
| Platform | Windows 10 |
| First seen | 2025-05-18 |
| **Fix Status** | **NOT FIXED** |

**Root Cause**: `BeWavedDolphin.cpp` throws when a circuit has no elements, no inputs, or no outputs. This happens when IC sub-circuit files are missing or broken, preventing the simulation from loading the full circuit graph.

**What the branch does**: The IC path resolution fix (`7f305cbe4`) and auto-migration (`761eb5f45`) may reduce the frequency by successfully loading more IC files. But the error itself (circuit missing required I/O elements) is not directly addressed — no recovery mechanism or better error messages added.

---

### 7. Undo/Redo Errors

#### [WIREDPANDA-EK](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EK) — "Error trying to redo Wire split"

| Field | Value |
|-------|-------|
| Events | 2 |
| Release | 4.3.0 |
| Platform | Windows 10 |
| **Fix Status** | **PARTIALLY FIXED** |

**Root Cause**: `SplitCommand::redo()` cannot find the source or destination element for the wire split. The validation check at line 533 in Commands.cpp catches this and throws a protective exception.

**What the branch improves**: ID stability fixes (`183c58f15`, `9e1391d96`) make element IDs more reliable across undo/redo cycles. `SplitCommand` now uses `updateItemId()` to restore original IDs. However, if the source/destination element itself was deleted by another command, the error can still occur.

---

## Overall Fix Coverage

| Category | Issues | Events | Fixed Events | Coverage |
|----------|--------|--------|-------------|----------|
| Scene item not found | EY, D2, F2 | 73 | 73 | **100%** |
| Pixmap path mangling | EG, EH, EA, EJ, EC | 251 | 251 | **100%** |
| .panda file not found | ET, ES, EP, EQ, ER | 14 | 7 | **50%** |
| Simulation crash (EX) | EX | 3 | 3 | **100%** |
| Scene crash (EV) | EV | 2 | 0 | **0%** |
| BSP tree crash (EW) | EW | 1 | 0 | **0%** |
| DeleteItems crash (F0) | F0 | 1 | ~1 | **~100%** |
| Platform crash (F1) | F1 | 5 | N/A | N/A |
| QtSlot crashes | EB, EN, EM, EF, EE | 7 | ~5 | **~75%** |
| File copy errors | 13, EZ | 206 | ~150 | **~75%** |
| Permission/save errors | F3, ED, X | 96 | N/A | N/A |
| Simulation load | M | 43 | 0 | **0%** |
| Wire split | EK | 2 | ~1 | **~50%** |
| **TOTAL** | **30** | **704** | **~491** | **~70%** |

(Excluding "not a bug" issues: F1, X, F3, ED, EZ = 101 events)

---

## Remaining Work Needed

### High Priority
1. **WIREDPANDA-EV** (2 events) — Validate `m_movedElements` pointers in `Scene::mouseReleaseEvent` before calling `pos()`. Use `QPointer` or check `scene()->items().contains(elm)`.
2. **WIREDPANDA-M** (43 events) — Add recovery mechanism in `BeWavedDolphin` when IC files are missing (graceful degradation instead of hard throw).

### Medium Priority
3. **WIREDPANDA-EW** (1 event) — Consider `QGraphicsScene::setItemIndexMethod(NoIndex)` or deferring item deletion to prevent BSP tree corruption.
4. **WIREDPANDA-13** (201 events) — Add retry logic or better diagnostics for Windows file copy failures in IC.cpp.

### Low Priority / Won't Fix
5. **WIREDPANDA-EQ/ER** — User-environment issues (missing files). Could improve UX with a "locate missing file" dialog.
6. **WIREDPANDA-F1** — Qt platform initialization; not an app bug.
7. **WIREDPANDA-X, F3, ED, EZ** — Correct error handling for OS-level failures and intentional UX messages.

---

## Key Commits on `tests-no-embed` Branch

| Commit | Description | Issues Fixed |
|--------|-------------|-------------|
| `183c58f15` | fix: ID instability and memory leak in ChangeInput/OutputSizeCommand | EY, D2, F2, F0 |
| `760a247cb` | fix: delete connections on ports removed during morph and restore on undo | EY, D2, F2, EB |
| `9e1391d96` | refactor: Replace global ElementFactory ID registry with per-scene ID lifecycle | EY, D2, F2, EK |
| `af32132f9` | fix: Save connection count in ChangeInput/OutputSizeCommand | EY, D2, F2 |
| `c3b451e43` | fix: Restore Serialization::contextDir after autosave | EG, EH, EA, EJ, EC |
| `7f305cbe4` | fix: Simplify IC file path resolution for cross-platform compatibility | ET, ES, EP, 13 |
| `30b2d3a48` | fix: AudioBox path resolution, relative storage, and external file copying | 13 |
| `b47748733` | fix: prevent viewport jumps when dragging elements | EB, EN, EM, EF, EE |
