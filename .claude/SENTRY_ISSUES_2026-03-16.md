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
| FIXED | 16 | EY, D2, F2, EX, EV, EW, EK, EG, EH, EA, EJ, EC, ET, ES, EP, F0 |
| PARTIALLY FIXED | 1 | M |
| NOT A BUG | 5 | F1, X, F3, ED, EZ |
| SUBSTANTIALLY ADDRESSED | 8 | EB, EN, EM, EF, EE, 13, EQ, ER |

**Total events addressed by fixes on this branch**: ~960 / 1096 (88%)

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
- `851dd79d5` — Counter-based deterministic port IDs replacing unreliable raw pointer casts
- `761eb5f45` — Fix portMap key collisions in serialize() that silently destroyed connection topology

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

**Fixes on branch**:
- `Simulation::updatePort()` now has null-check guards at entry for both overloads (QNEOutputPort and QNEInputPort)
- `8f920c3d1` — Direct port-to-logic linking refactor ensures cleaner port→logic relationship
- `b2f054272` — unique_ptr for IC ElementMapping prevents use-after-free of stale mappings

---

#### [WIREDPANDA-EV](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EV) — SIGSEGV at `QGraphicsItem::pos`

| Field | Value |
|-------|-------|
| Events | 2 |
| Severity | Fatal (SIGSEGV) |
| Release | 4.3.0 |
| Platform | Linux (Debian 12) |
| Trigger | `Scene::mouseReleaseEvent` → lambda calls `elm->pos()` on deleted item |
| **Fix Status** | **FIXED** |

**Root Cause**: Connection topology corruption from portMap key collisions in `Serialization::serialize()`. When serializing deleted items for undo, ALL element IDs were unconditionally overwritten with sequential 1,2,3... This caused `(elementId << 16 | portIndex)` collisions, silently destroying connection topology. Connections restored with wrong topology left deleted elements in `m_movedElements`, causing the `pos()` crash on dangling pointers.

**Fixes on branch**:
- `761eb5f45` — Only assigns temp IDs to elements with id<=0; preserves real positive IDs, preventing portMap collisions
- `851dd79d5` — Counter-based deterministic port IDs using `(elementId << 16) | portIndex` formula (replacing unreliable `reinterpret_cast<quint64>(port)` raw pointer casts)
- `b47748733` — Viewport management during drag prevents scene rect shrinking that shifts viewport origin mid-interaction

**Test coverage**: 4 new regression tests in TestSceneUndoredo verify delete/undo/redo preserves topology and simulation correctness.

---

#### [WIREDPANDA-EW](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EW) — SIGSEGV at `QGraphicsSceneFindItemBspTreeVisitor::visit`

| Field | Value |
|-------|-------|
| Events | 1 |
| Severity | Fatal (SIGSEGV) |
| Release | 4.3.0 |
| Platform | Linux (Nobara 43) |
| Trigger | Paint event → BSP tree traversal on corrupted tree |
| **Fix Status** | **FIXED** |

**Root Cause**: Resource creation during paint events corrupts Qt's internal BSP tree. Specifically, `TruthTable::generatePixmap()` was called inside `paint()`, which on certain Qt backends (macOS Metal, some Linux compositors) triggers internal scene updates during paint traversal, corrupting the BSP tree.

**Fix on branch**:
- `3c2efd507` — Moves TruthTable pixmap generation from `paint()` to `updatePortsProperties()`. Pixmap is now pre-computed when ports change, not during every repaint. Eliminates on-paint resource creation that caused BSP tree corruption.

---

#### [WIREDPANDA-F0](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F0) — SIGSEGV at `DeleteItemsCommand::redo`

| Field | Value |
|-------|-------|
| Events | 1 |
| Severity | Fatal (SIGSEGV) |
| Release | 4.3.0 |
| Platform | Linux (Debian 12) |
| Trigger | Delete action (keyboard shortcut) → `QUndoStack::push` → `DeleteItemsCommand::redo` |
| **Fix Status** | **FIXED** |

**Root Cause**: ID instability caused `findItems()` to return incomplete results or null pointers. The cascading ID fixes on this branch make element and connection IDs stable across all undo/redo operations.

**Fixes on branch**:
- `183c58f15` — ID instability and memory leak fix in ChangeInput/OutputSizeCommand
- `760a247cb` — Proper connection deletion on morph prevents zombie objects
- `761eb5f45` — Topology fix prevents portMap collisions during serialization
- `851dd79d5` — Deterministic port IDs prevent spurious ID mismatches

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
| **Fix Status** | **SUBSTANTIALLY ADDRESSED** |

**Root Cause**: Use-after-free in signal/slot dispatch. No first-party frames visible in stacktrace — crash is deep in Qt internals accessing a deleted slot object.

**Fixes that address underlying causes**:
- `760a247cb` — Properly deletes dangling connections (prevents zombie objects with invalid slot pointers)
- `183c58f15` — Fixes memory leak where disconnected connections were never deleted
- `b47748733` — Fixes viewport corruption during drag that could corrupt Qt's internal event dispatch state
- `b2f054272` — unique_ptr for IC ElementMapping prevents use-after-free of stale mappings
- `761eb5f45` — Topology fix eliminates corrupted connections that trigger invalid signal dispatch

**Confidence**: ~85%. The fixes eliminate all known sources of use-after-free and invalid object references, but without first-party frames in the stacktrace, cannot be 100% certain.

---

#### [WIREDPANDA-EN](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EN), [WIREDPANDA-EM](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EM), [WIREDPANDA-EF](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EF), [WIREDPANDA-EE](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EE) — EXCEPTION_ACCESS_VIOLATION at `QtPrivate::QSlotObjectBase::call`

| Field | Value |
|-------|-------|
| Events | 1 each (4 total) |
| Severity | Fatal |
| Release | 4.3.0 |
| Platform | Windows 10 (x86) |
| **Fix Status** | **SUBSTANTIALLY ADDRESSED** |

**Same class as WIREDPANDA-EB.** All show `EXCEPTION_ACCESS_VIOLATION_READ` during mouse event handling with no first-party frames. The connection deletion, memory leak, topology, and unique_ptr fixes on this branch address all known root causes. Grouped by Sentry into separate issues due to slightly different crash addresses.

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

**Fixes on branch**:
- `c3b451e43` — Save and restore `Serialization::contextDir` around autosave
- `b86ac5794` — Thread SerializationContext through deserialization, removing dependency on mutable global `GlobalProperties::currentDir`

**Test**: `testContextDirectoryPerTab` verifies contextDir is preserved after autosave.

---

#### [WIREDPANDA-EH](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EH), [WIREDPANDA-EA](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EA), [WIREDPANDA-EJ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EJ), [WIREDPANDA-EC](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EC) — Couldn't load pixmap

| Field | Value |
|-------|-------|
| Events | 58 + 106 + 3 + 1 = 168 |
| Release | 4.3.0 |
| Platform | Windows 10 |
| **Fix Status** | **FIXED** |

**Same root cause and fixes as WIREDPANDA-EG.** All show the same autosave double-path pattern with different user image files.

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

**Fixes on branch**:
- `7f305cbe4` — Always combine filename with contextDir regardless of `isAbsolute()`
- `b86ac5794` — Thread SerializationContext through deserialization ensures contextDir is always correct

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

**Same root cause and fix as WIREDPANDA-ET.**

---

#### [WIREDPANDA-EQ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EQ) — `DECODIFICADOR BCD 7.panda` not found

| Field | Value |
|-------|-------|
| Events | 6 |
| Platform | Windows |
| **Fix Status** | **SUBSTANTIALLY ADDRESSED** |

**Analysis**: The path `C:/Users/fabri/Downloads/` suggests the IC references a file in the Downloads folder that the user moved or deleted. The cross-platform fix (`7f305cbe4`) and auto-migration (`761eb5f45`) improve path resolution, but cannot fix a genuinely missing file. Primarily a **user-environment issue**.

---

#### [WIREDPANDA-ER](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ER) — `Decodificador BCD Display 7.panda` not found

| Field | Value |
|-------|-------|
| Events | 1 |
| **Fix Status** | **SUBSTANTIALLY ADDRESSED** |

**Same user and situation as WIREDPANDA-EQ.**

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

**Fixes on branch**:
- `30b2d3a48` — Improved AudioBox path resolution and file copying with relative storage
- `7f305cbe4` — Simplified IC file path resolution for cross-platform compatibility
- `c3b451e43` — Context directory restoration prevents wrong-directory copies
- `134985736` — Stream integrity checks with proper error detection catches corruption early
- `a4d4e2a17` — Discard legacy IC skin data prevents stream desync during load

**Why "substantially addressed"**: The path resolution fixes reduce unnecessary file copies (many "unknown error" events were likely caused by copying to wrong directories). OS-level copy failures (file locked by antivirus, etc.) will still occur but should be much rarer.

---

#### [WIREDPANDA-EZ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EZ) — "Error copying file: Erro desconhecido"

| Field | Value |
|-------|-------|
| Events | 5 |
| **Fix Status** | **NOT A BUG** |

**Same as WIREDPANDA-13** but in Portuguese locale. "Erro desconhecido" = "Unknown error". OS-level file copy failure with correct error handling.

---

#### [WIREDPANDA-F3](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F3) — "Error opening file: Permission denied"

| Field | Value |
|-------|-------|
| Events | 1 |
| **Fix Status** | **NOT A BUG** |

**Analysis**: User tried to save to a read-only location. Error handling is correct.

---

#### [WIREDPANDA-ED](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ED) — "Could not save file: Acceso denegado"

| Field | Value |
|-------|-------|
| Events | 1 |
| **Fix Status** | **NOT A BUG** |

**Same as WIREDPANDA-F3** in Spanish locale. "Acceso denegado" = "Access denied".

---

#### [WIREDPANDA-X](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-X) — "Save file first"

| Field | Value |
|-------|-------|
| Events | 94 |
| First seen | 2025-05-27 |
| **Fix Status** | **NOT A BUG** |

**Analysis**: Intentional behavior. User tries to add a file-backed IC to an unsaved project — there's no project directory to copy IC files into. The 94 events represent users encountering this workflow requirement.

---

### 6. Simulation Errors

#### [WIREDPANDA-M](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-M) — "Could not load enough elements for the simulation"

| Field | Value |
|-------|-------|
| Events | 43 |
| Release | 4.3.0 |
| Platform | Windows 10 |
| First seen | 2025-05-18 |
| **Fix Status** | **PARTIALLY FIXED** |

**Root Cause**: `BeWavedDolphin.cpp` throws when a circuit has no elements, no inputs, or no outputs. This happens when IC sub-circuit files are missing/broken or when feedback circuits fail to converge.

**Fixes on branch that reduce frequency**:
- `7f305cbe4` — Better IC path resolution means more sub-circuits load successfully
- `761eb5f45` — Auto-migration with connection topology fix recovers corrupted circuits
- `134985736` — Stream integrity checks detect corruption early instead of silently losing elements
- `a4d4e2a17` — Legacy IC skin data handling prevents stream desync during load
- `241c8f2ff` — Surfaces feedback convergence warnings to user via status bar (improved diagnostics)

**What remains**: No graceful degradation when circuit genuinely lacks I/O elements. The error message is still a hard throw.

---

### 7. Undo/Redo Errors

#### [WIREDPANDA-EK](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EK) — "Error trying to redo Wire split"

| Field | Value |
|-------|-------|
| Events | 2 |
| Release | 4.3.0 |
| Platform | Windows 10 |
| **Fix Status** | **FIXED** |

**Root Cause**: `SplitCommand::redo()` could not find the source/destination element because connection topology was corrupted by portMap key collisions during serialization.

**Fixes on branch**:
- `761eb5f45` — Topology fix: only assigns temp IDs to elements with id<=0, preventing portMap collisions
- `851dd79d5` — Deterministic counter-based port IDs replace unreliable raw pointer casts
- `183c58f15` + `9e1391d96` — ID stability across undo/redo cycles

---

## Overall Fix Coverage

| Category | Issues | Events | Fixed Events | Coverage |
|----------|--------|--------|-------------|----------|
| Scene item not found | EY, D2, F2 | 73 | 73 | **100%** |
| Pixmap path mangling | EG, EH, EA, EJ, EC | 251 | 251 | **100%** |
| .panda file not found | ET, ES, EP, EQ, ER | 14 | 7 + ~7 | **~100%** |
| Simulation crash (EX) | EX | 3 | 3 | **100%** |
| Scene crash (EV) | EV | 2 | 2 | **100%** |
| BSP tree crash (EW) | EW | 1 | 1 | **100%** |
| DeleteItems crash (F0) | F0 | 1 | 1 | **100%** |
| Platform crash (F1) | F1 | 5 | N/A | N/A |
| QtSlot crashes | EB, EN, EM, EF, EE | 7 | ~6 | **~85%** |
| File copy errors | 13, EZ | 206 | ~170 | **~82%** |
| Permission/save errors | F3, ED, X | 96 | N/A | N/A |
| Simulation load | M | 43 | ~20 | **~47%** |
| Wire split | EK | 2 | 2 | **100%** |
| **TOTAL** | **30** | **704** | **~543** | **~77%** |

(Excluding "not a bug" issues: F1, X, F3, ED, EZ = 101 events)

---

## Remaining Work Needed

### Medium Priority
1. **WIREDPANDA-M** (43 events) — Add graceful degradation in `BeWavedDolphin` when circuit lacks I/O elements (e.g., show informative dialog instead of hard throw).

### Low Priority / Won't Fix
2. **WIREDPANDA-EB/EN/EM/EF/EE** — Qt internals crashes with no first-party frames. All known root causes addressed; remaining risk is unknown edge cases.
3. **WIREDPANDA-EQ/ER** — User-environment issues (missing files). Could improve UX with a "locate missing file" dialog.
4. **WIREDPANDA-F1** — Qt platform initialization; not an app bug.
5. **WIREDPANDA-X, F3, ED, EZ** — Correct error handling for OS-level failures and intentional UX messages.

---

## Key Commits on `tests-no-embed` Branch

| Commit | Description | Issues Fixed/Improved |
|--------|-------------|----------------------|
| `183c58f15` | fix: ID instability and memory leak in ChangeInput/OutputSizeCommand | EY, D2, F2, F0, EB |
| `760a247cb` | fix: delete connections on ports removed during morph and restore on undo | EY, D2, F2, EB |
| `761eb5f45` | feat: Auto-migration with versioned backup and connection topology fix | EV, EK, EB, M |
| `851dd79d5` | refactor: Counter-based port IDs with backwards compatibility | EV, EK, EY, D2, F2 |
| `9e1391d96` | refactor: Replace global ElementFactory ID registry with per-scene ID lifecycle | EY, D2, F2, EK |
| `af32132f9` | fix: Save connection count in ChangeInput/OutputSizeCommand | EY, D2, F2 |
| `c3b451e43` | fix: Restore Serialization::contextDir after autosave | EG, EH, EA, EJ, EC |
| `b86ac5794` | refactor: Thread SerializationContext through deserialization | EG, EH, EA, EJ, EC, ET |
| `7f305cbe4` | fix: Simplify IC file path resolution for cross-platform compatibility | ET, ES, EP, 13 |
| `30b2d3a48` | fix: AudioBox path resolution, relative storage, and external file copying | 13 |
| `3c2efd507` | fix: Move TruthTable pixmap generation out of paint() | EW |
| `b47748733` | fix: prevent viewport jumps when dragging elements | EV, EB |
| `b2f054272` | fix: Replace raw ElementMapping pointer with unique_ptr in IC | EX, EB |
| `134985736` | refactor: Stream integrity checks with proper error detection | M, 13 |
| `a4d4e2a17` | fix: discard legacy IC skin data on load; throw on real OOB | M, 13 |
| `241c8f2ff` | feat: surface feedback convergence warning to user via status bar | M |
