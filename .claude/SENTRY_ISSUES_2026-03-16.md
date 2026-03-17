# Sentry Issues Report — wiRedPanda

**Date**: 2026-03-16
**Organization**: rodrigo-torres
**Project**: wiredpanda
**Dashboard**: https://rodrigo-torres.sentry.io/issues/?project=wiredpanda
**Total Issues**: 32 (30 unresolved, 2 resolved)

---

## Summary by Category

| Category | Count | Issues |
|----------|-------|--------|
| Scene item not found | 5 | EY, D2, F2, CY, 1V |
| Pixmap/image load failure | 5 | EJ, EG, EH, EA, EC |
| File not found (.panda) | 4 | ET, ES, EQ, EP |
| File copy error | 2 | EZ, 13 |
| Crash (QtSlotObject) | 5 | EN, EM, EB, EF, EE |
| Crash (Qt internals) | 3 | EX, EV, EW |
| Simulation load failure | 1 | M |
| Undo/redo errors | 3 | F0, F1, EK |
| File save/permission errors | 3 | F3, ED, X |
| Wire split error | 1 | ER |

---

## All Issues

### Crashes & Null Pointer Dereferences

| ID | Title | Status | Events | First Seen | Last Seen |
|----|-------|--------|--------|------------|-----------|
| [WIREDPANDA-EX](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EX) | QNEPort::graphicElement | unresolved | 3 | 3 days ago | 3 days ago |
| [WIREDPANDA-EV](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EV) | QGraphicsItem::pos | unresolved | 2 | 3 days ago | 3 days ago |
| [WIREDPANDA-EW](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EW) | QGraphicsSceneFindItemBspTreeVisitor::visit | unresolved | 1 | 3 days ago | 3 days ago |
| [WIREDPANDA-F1](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F1) | qt_message_fatal | unresolved | 5 | 3 days ago | 3 days ago |
| [WIREDPANDA-F0](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F0) | DeleteItemsCommand::redo | unresolved | 1 | 3 days ago | 3 days ago |
| [WIREDPANDA-EN](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EN) | QtPrivate::QSlotObjectBase::call | unresolved | 1 | 13 days ago | 13 days ago |
| [WIREDPANDA-EM](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EM) | QtPrivate::QSlotObjectBase::call | unresolved | 1 | 13 days ago | 13 days ago |
| [WIREDPANDA-EB](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EB) | QtPrivate::QSlotObjectBase::call | unresolved | 3 | 18 days ago | 14 days ago |
| [WIREDPANDA-EF](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EF) | QtPrivate::QSlotObjectBase::call | unresolved | 1 | 14 days ago | 14 days ago |
| [WIREDPANDA-EE](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EE) | QtPrivate::QSlotObjectBase::call | unresolved | 1 | 14 days ago | 14 days ago |

### Scene Element Not Found (Undo/Redo Related)

| ID | Title | Status | Events | First Seen | Last Seen |
|----|-------|--------|--------|------------|-----------|
| [WIREDPANDA-EY](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EY) | One or more elements was not found on the scene | unresolved | 14 | 3 days ago | 3 days ago |
| [WIREDPANDA-D2](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-D2) | One or more items was not found on the scene | unresolved | 52 | 2025-10-08 | 3 days ago |
| [WIREDPANDA-F2](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F2) | One or more items was not found on the scene | unresolved | 7 | 3 days ago | 3 days ago |
| [WIREDPANDA-CY](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-CY) | One or more items was not found on the scene | **resolved** | 94 | 2025-09-30 | 4 days ago |
| [WIREDPANDA-1V](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-1V) | One or more elements was not found on the scene | **resolved** | 266 | 2025-06-14 | 5 days ago |

### File Not Found (.panda files)

| ID | Title | Status | Events | First Seen | Last Seen |
|----|-------|--------|--------|------------|-----------|
| [WIREDPANDA-ET](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ET) | Z:/Meu Drive/.../decoder.panda not found | unresolved | 4 | 7 days ago | 7 days ago |
| [WIREDPANDA-ES](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ES) | Z:/.../jkflipflop.panda not found | unresolved | 1 | 7 days ago | 7 days ago |
| [WIREDPANDA-EQ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EQ) | DECODIFICADOR BCD 7.panda not found | unresolved | 6 | 13 days ago | 13 days ago |
| [WIREDPANDA-ER](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ER) | Decodificador BCD Display 7.panda not found | unresolved | 1 | 13 days ago | 13 days ago |
| [WIREDPANDA-EP](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EP) | .../GuessNumber/Vidas.panda not found | unresolved | 2 | 13 days ago | 13 days ago |

### Pixmap/Image Load Failures

| ID | Title | Status | Events | First Seen | Last Seen |
|----|-------|--------|--------|------------|-----------|
| [WIREDPANDA-EG](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EG) | Couldn't load pixmap (autosave path mangling) | unresolved | 83 | 14 days ago | 14 days ago |
| [WIREDPANDA-EH](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EH) | Couldn't load pixmap (screenshot path) | unresolved | 58 | 14 days ago | 14 days ago |
| [WIREDPANDA-EA](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EA) | Couldn't load pixmap (downloaded image) | unresolved | 106 | 18 days ago | 18 days ago |
| [WIREDPANDA-EJ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EJ) | Couldn't load pixmap (JPG image) | unresolved | 3 | 14 days ago | 14 days ago |
| [WIREDPANDA-EC](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EC) | Couldn't load pixmap (desktop PNG) | unresolved | 1 | 18 days ago | 18 days ago |

**Common pattern**: Autosave path includes original absolute path, creating double-path like `C:/Users/.../autosaves/C:/Users/.../image.png`.

### File Copy/Save Errors

| ID | Title | Status | Events | First Seen | Last Seen |
|----|-------|--------|--------|------------|-----------|
| [WIREDPANDA-13](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-13) | Error copying file: Unknown error | unresolved | 201 | 2025-05-30 | 3 days ago |
| [WIREDPANDA-EZ](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EZ) | Error copying file: Erro desconhecido | unresolved | 5 | 3 days ago | 3 days ago |
| [WIREDPANDA-F3](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-F3) | Error opening file: Permission denied | unresolved | 1 | 3 days ago | 3 days ago |
| [WIREDPANDA-ED](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-ED) | Could not save file: Acceso denegado | unresolved | 1 | 18 days ago | 18 days ago |
| [WIREDPANDA-X](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-X) | Save file first | unresolved | 94 | 2025-05-27 | 17 days ago |

### Simulation Errors

| ID | Title | Status | Events | First Seen | Last Seen |
|----|-------|--------|--------|------------|-----------|
| [WIREDPANDA-M](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-M) | Could not load enough elements for the simulation | unresolved | 43 | 2025-05-18 | 3 days ago |

### Undo/Redo Errors

| ID | Title | Status | Events | First Seen | Last Seen |
|----|-------|--------|--------|------------|-----------|
| [WIREDPANDA-EK](https://rodrigo-torres.sentry.io/issues/WIREDPANDA-EK) | Error trying to redo Wire split | unresolved | 2 | 14 days ago | 14 days ago |

---

## Top Issues by Event Count

| # | ID | Events | Title |
|---|-----|--------|-------|
| 1 | WIREDPANDA-1V | 266 | One or more elements was not found on the scene |
| 2 | WIREDPANDA-13 | 201 | Error copying file: Unknown error |
| 3 | WIREDPANDA-EA | 106 | Couldn't load pixmap (autosave path) |
| 4 | WIREDPANDA-CY | 94 | One or more items was not found on the scene |
| 5 | WIREDPANDA-X | 94 | Save file first |
| 6 | WIREDPANDA-EG | 83 | Couldn't load pixmap (autosave path) |
| 7 | WIREDPANDA-EH | 58 | Couldn't load pixmap (autosave path) |
| 8 | WIREDPANDA-D2 | 52 | One or more items was not found on the scene |
| 9 | WIREDPANDA-M | 43 | Could not load enough elements for simulation |
| 10 | WIREDPANDA-EY | 14 | One or more elements was not found on the scene |

---

## Key Patterns & Recommended Fixes

### 1. Autosave Pixmap Path Mangling (251 events across 5 issues)
Autosave creates paths like `C:/.../autosaves/C:/.../image.png`. The autosave path construction appends the full original path instead of just the filename.

### 2. Scene Item Not Found (433 events across 5 issues)
Most frequent category overall. Undo/redo operations reference elements that no longer exist in the scene. Likely related to ID management during command execution.

### 3. File Copy Unknown Error (206 events across 2 issues)
Long-standing issue since May 2025. Same root cause in different locales (English vs Portuguese).

### 4. Simulation Load Failure (43 events)
Persistent since May 2025. Circuits with missing dependencies or broken IC references fail to initialize simulation.
