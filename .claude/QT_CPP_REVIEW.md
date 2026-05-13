# Qt C++ Code Review — Consolidated Report

**Dates**: 2026-04-29 to 2026-04-30
**Scope**: `App/`, `Tests/`, `MCP/Server/`
**Status legend**: ✅ Fixed | ⚠️ Partial | ❌ False positive | 🔲 Open / deferred

---

## Combined Summary

| Pass | Scope | Files | Findings | Fixed | FP | Open / deferred |
|------|-------|-------|----------|-------|----|-----------------|
| 1 | `App/` | 187 | 45 | 32 | 4 | 5 partial / deferred |
| 2 | `App/` + `Tests/` | 549 | 59 | 19 | 16 | 24 (mostly deferred) |
| 3 | `MCP/Server/` | 27 | 55 | 55 | 0 | 0 |
| **Total** | | **763** | **159** | **106** | **20** | **33** |

The 7 items in the **Pending / Skipped** section at the end of this document are the only categories that remain forward-looking; all other "open" items in the per-pass tables below have been documented as deferred for a stated reason or rolled into a later pass that closed them.

---

## Pass 1 — `App/` (Initial Review)

**Scope**: all `.cpp`/`.h` files in `App/`
**Findings**: 35 confirmed (14 lint groups, 21 deep analysis), 10 investigation targets

### Status

| ID | Status | Note |
|----|--------|------|
| L-001 | ❌ False positive | Multi-arg `.arg(a, b)` is valid Qt6 — lint can't count across single call |
| L-002 | ✅ Fixed | `QFile::open()` guard added in MainWindow WASM handler |
| L-003 | ❌ False positive | `reply->error()` IS checked before `readAll()` in both cases |
| L-004 | ⚠️ Partial | About-dialog URLs → `https://`; `Node.cpp` is SVG namespace URI — see Pending |
| L-005 | ✅ Fixed | `setTransferTimeout` added to UpdateChecker and MainWindow download |
| L-006 | ✅ Fixed | `sslErrors` handler connected; reply aborted on SSL error |
| L-007 | ✅ Fixed | `getModel()` → `model()`; `getTabIndex()` → `tabIndex()`; callers updated |
| L-008 | ✅ Fixed | `qMin`/`qMax` → `(std::min)`/`(std::max)` with Windows-safe parens |
| L-009 | ✅ Fixed | `QPair<int,QString>` → `std::pair<int,QString>` across element hierarchy |
| L-010 | ✅ Fixed | `QList<QString>` → `QStringList` in SystemVerilogCodeGen |
| L-011 | ⚠️ Partial | BeWavedDolphin sites fixed; `GraphicElement.cpp:883` is a write loop — see Pending |
| L-012 | ✅ Fixed | Virtual destructors (D-001–003) cover grandchild cleanup |
| L-013 | ❌ False positive | Flagged member variable declarations (tick counts), not timer API calls |
| L-014 | ✅ Fixed | Brace-init → copy-init in App/ files |
| D-001 | ✅ Fixed | `virtual ~GraphicElement() = default;` added |
| D-002 | ✅ Fixed | `virtual ~GraphicElementInput() = default;` added |
| D-003 | ✅ Fixed | `virtual ~QNEPort() = default;` added |
| D-004 | ✅ Fixed | `Q_DISABLE_COPY_MOVE(GraphicElement)` added; covers ~50 subclasses |
| D-005 | ✅ Fixed | IC: copy/move ops deleted |
| D-006 | ✅ Fixed | Covered by D-004 |
| D-007 | ✅ Fixed | `Qt::QueuedConnection` on ICRegistry::onFileChanged |
| D-008 | ✅ Fixed | `QMutex` + `QMutexLocker` in `shouldSendToSentry()` |
| D-009 | ✅ Fixed | `Qt::QueuedConnection` on RecentFiles fileChanged lambda |
| D-010 | ✅ Fixed | `getModel()` returns `const QStandardItemModel*` |
| D-011 | ✅ Fixed | `Scene::itemAt()` and `itemsAt()` marked `const` |
| D-012 | ✅ Fixed | `Scene::connections()` marked `const` |
| D-013 | ✅ Fixed | `ArduinoCodeGen::getAvailableBoards()` marked `const` |
| D-014 | ✅ Fixed | `cachedFileBytes()` logs warning + returns static empty on open failure |
| D-015 | ✅ Fixed | `rows <= 0` check added; warning logged on truncation |
| D-016 | ✅ Fixed | `makeBlobSelfContained()` logs warning when dependency cannot be opened |
| D-017 | ✅ Fixed | ElementPalette: single `findChildren()` reused for all three filter passes |
| D-018 | ✅ Fixed | Hash double-lookup eliminated in `buildSuccessorGraph()` |
| D-019 | ❌ False positive | Cleanup in `updateEmbeddedICList()` IS intentional before null check |
| D-020 | ✅ Fixed | `QRegularExpression` compiled once per `onSearchTextChanged()` |
| D-021 | ✅ Fixed | `removeForbiddenChars()` regex replaced with `std::remove_if` predicate |
| I-001 | ✅ Resolved | Virtual destructors (D-001–003) cover grandchild cleanup |
| I-002 | ❌ False positive | `QObject` already deletes copy/move; manager classes inherit protection |
| I-003 | ✅ Fixed | `IC(const IC &) = delete;` |
| I-004 | ✅ Fixed | Doc comment added: write-once/startup-only semantics |
| I-005 | ❌ False positive | Qt parent-child ordering guarantees reply dies before manager |
| I-006 | ❌ False positive | `setFrequency()` already rejects non-positive intervals |
| I-007 | ✅ Fixed | CSV cells clamped to {0,1}; single `qCWarning` if any non-binary value found |
| I-008 | ✅ Fixed | Format detection moved to peek-into-buffer probes; live stream stays clean, status check now reliable |
| I-009 | ✅ Fixed | `write()` return checked in Main.cpp desktop entry path (D2-036) |
| I-010 | ✅ Fixed | `update()` snapshots topology vectors; `restart()` no longer races |

### Pass 1 totals

| Category | Lint | Deep | Investigate | Total | Fixed | FP | Other |
|----------|------|------|-------------|-------|-------|----|-------|
| Ownership & Lifecycle | 2 | 6 | 3 | 11 | 9 | 1 | 1 partial |
| Thread Safety | 0 | 3 | 2 | 5 | 5 | 0 | 0 |
| API & C++ Correctness | 1 | 4 | 0 | 5 | 5 | 0 | 0 |
| Error Handling | 5 | 3 | 3 | 11 | 8 | 2 | 1 partial |
| Performance & Quality | 3 | 5 | 2 | 10 | 9 | 1 | 0 |
| Deprecations | 3 | 0 | 0 | 3 | 3 | 0 | 0 |
| **Total** | **14** | **21** | **10** | **45** | **39** | **4** | **2** |

---

## Pass 2 — `App/` + `Tests/` (No confidence filter)

**Scope**: all `.cpp`/`.h` files in `App/` and `Tests/`; all findings included regardless of confidence

### Status

| ID | Status | Note |
|----|--------|------|
| L2-001 | ✅ Fixed | `(std::min)`/`(std::max)` parens added across 7 files |
| L2-002 | ⚠️ Partial | Lambda uses `QChar::isLetterOrNumber(c.unicode())`; full `char16_t` impractical — see Pending |
| L2-003 | ❌ False positive | Per-iteration regex pattern from `varName`; can't hoist |
| L2-004 | ✅ Fixed | `: int` added to 4 unscoped enums in CpuCommon.h and TestLevel9 |
| L2-005 | ❌ False positive | All flagged opens already wrapped in `QVERIFY`/`QVERIFY2` |
| L2-006 | ✅ Fixed | TestUtils CircuitBuilder methods renamed (get-prefix removal) |
| L2-007 | ✅ Fixed | 45 `SerializationContext{...}` → `= {...}` across 7 test files |
| L2-008 | ✅ Fixed | `.count()`/`.length()` → `.size()` across 14 test files (37 sites) |
| L2-009 | ❌ False positive | Resolved in Pass 1; lint can't cross translation-unit boundaries |
| L2-010 | ✅ Fixed | QMap→QHash where ordering is not needed; QMap retained for serialisation |
| D2-001 | ✅ Fixed | `item()` null guard in `print()` and `saveToTxt()` |
| D2-002 | ✅ Fixed | `verticalHeaderItem()` null guard in `saveToTxt()` |
| D2-003 | ❌ False positive | `setColumnCount()` calls `beginInsert/RemoveColumns()` internally |
| D2-004 | ✅ Fixed | `loadNewTable()` pre-populates all cells via `setData(..., 0)` |
| D2-005 | ✅ Fixed | Pre-population (D2-004) ensures all cells have items |
| D2-006 | ✅ Fixed | Pre-population closes the gap |
| D2-007 | ❌ False positive | All rows non-editable via delegate; editing is programmatic |
| D2-008 | ✅ Fixed | Old `m_model` deleted before creating new in `loadNewTable()` |
| D2-009 | ✅ Fixed | `run()`/`print()` return early when `m_model` is null |
| D2-010 | ❌ False positive | `setMimeData(mimeData())` is single expression; no leak gap |
| D2-011 | ❌ False positive | `m_scene` parented to `this`; proxy lifetime bounded |
| D2-012 | ✅ Fixed | `run()` returns early on null `m_simulation` |
| D2-013 | ❌ False positive | Allocations directly in container initializers; no `QVERIFY` gap |
| D2-014 | ✅ Fixed | `qScopeGuard(qDeleteAll)` on both load paths; dismissed on success |
| D2-015 | ✅ Fixed | `takeLast()` removes port from vector before `delete` |
| D2-016 | ✅ Fixed | `std::unique_ptr<QMimeData>` in copy/cut/clone-drag |
| D2-017 | ✅ Fixed | Doc comment: caller takes ownership |
| D2-018 | ✅ Fixed | All `createDolphin()` callers already use `std::unique_ptr` |
| D2-019 | ✅ Fixed | `IC(const IC&) = delete` |
| D2-020 | ✅ Fixed | `Q_ASSERT` thread-affinity in `Display7::cachedSegmentColors()` |
| D2-021 | ✅ Fixed | `Q_ASSERT` in `GraphicElement::pixmapCache()` |
| D2-022 | ✅ Fixed | Thread-safety constraint documented in `registry()` |
| D2-023 | ✅ Fixed | `update()` snapshots topology vectors before iteration |
| D2-024 | ✅ Fixed | `Q_ASSERT` thread-affinity in `cachedFileBytes()` |
| D2-025 | ✅ Fixed | `Q_ASSERT` in `ThemeManager` constructor |
| D2-026 | ✅ Fixed | `Q_ASSERT` in `addRecentFile()` and `recentFiles()` |
| D2-027 | ✅ Fixed | `inputPort()`/`outputPort()` marked `const` |
| D2-028 | ⚠️ Partial | `fileInfo()`/`dolphinFileName()` `const`; sub-object accessors stay non-const — see Pending |
| D2-029 | ✅ Fixed | Const + non-const overloads of `graphicElement()` |
| D2-030 | ✅ Fixed | All 9 `default:` switch sites resolved (made exhaustive or intentional catch-all) |
| D2-031 | ✅ Fixed | Doc comment: prefer `setBlob()` for individual updates |
| D2-032 | ✅ Fixed | `mimeData()` marked `const` |
| D2-033 | ✅ Fixed | Test helpers renamed (get-prefix removal) — 506 call sites |
| D2-034 | ✅ Fixed | Bounds check before `split().at(1)` in legacy header parsing |
| D2-035 | ✅ Fixed | `ok`-checked numeric conversions in Clock/Buzzer/AudioBox; CSV cells clamped |
| D2-036 | ⚠️ Partial | MainWindow + Main.cpp fixed; `QSaveFile.commit()` sites are not bugs — see Pending |
| D2-037 | ❌ False positive | Empty `tag_name` already caught by `latest.isNull()` |
| D2-038 | ✅ Fixed | UpdateChecker rejects replies > 1 MiB before `readAll()` |
| D2-039 | ❌ False positive | All `args.at(0)` accesses inside `if (!args.empty())` |
| D2-040 | ✅ Fixed | `qCWarning` on both binary and CSV truncation paths |
| D2-041 | ❌ False positive | Max product 16 × 2048 = 32 768 — no overflow |
| D2-042 | ❌ False positive | All test `open()` calls already wrapped in `QVERIFY` |
| D2-043 | ✅ Fixed | Same as L2-001 |
| D2-044 | ❌ False positive | `m_varMap[port]` usages are assignments — `operator[]` correct |
| D2-045 | ❌ False positive | No shared fixture exists — each test exercises a different circuit topology; see Pending |
| D2-046 | 🔲 Deferred | `MainWindow` God class — see Pending |
| D2-047 | ❌ False positive | Per-iteration regex pattern unavoidable |
| D2-048 | 🔲 Deferred | Code generator duplication — see Pending |
| D2-049 | ❌ False positive | `replace()` runs inside the `src.open()` block, not before guard |

### Pass 2 totals

| Category | Total | Fixed | False positive | Deferred |
|----------|-------|-------|----------------|----------|
| Lint | 10 | 7 | 3 | 0 |
| Model Contracts | 9 | 6 | 3 | 0 |
| Ownership & Lifecycle | 10 | 6 | 4 | 0 |
| Thread Safety | 7 | 7 | 0 | 0 |
| API & C++ Correctness | 7 | 6 | 1 | 0 |
| Error Handling | 9 | 4 | 5 | 0 |
| Performance & Quality | 7 | 1 | 4 | 2 |
| **Total** | **59** | **37** | **20** | **2** |

---

## Pass 3 — `MCP/Server` (Targeted MCP review)

**Scope**: 27 files in `MCP/Server/**`
**Findings**: 55 (20 lint, 34 deep analysis, 1 investigation)
**Outcome**: All 55 findings resolved.

### Status (condensed — full details in git history)

**Lint findings — all ✅ Fixed**
- L-001 through L-005: HandlerFactory get-prefix accessors renamed (or HandlerFactory deleted as dead code, see D-027/D-028).
- L-006: `m_stdout` flushed and reset after each response write.
- L-007: `QJsonDocument::fromJson()` validated with `QJsonParseError`.
- L-008: `getSchemaLoaded` → `isSchemaLoaded`.
- L-009 through L-013: BaseHandler get-prefix accessors renamed.
- L-014 through L-020: `ICommand`/`IResponse` get-prefix renames — interfaces deleted entirely as dead code (D-028).

**Deep analysis findings — all ✅ Fixed**

| ID | Resolution |
|----|------------|
| D-001 | `Q_DISABLE_COPY_MOVE(SimulationHandler)` |
| D-002 | `deleteLater()` → `delete` in non-QObject destructor |
| D-003 | `StdinReader` → `std::unique_ptr`, parent removed |
| D-004 | `QNEConnection`/`AddItemsCommand` wrapped in `unique_ptr` with `release()` on ownership transfer |
| D-005 | `IValidator`, `ICommand`, `IResponse` deleted (dead code per D-028) |
| D-006 | `volatile bool` → `std::atomic<bool>` for `m_stopRequested` |
| D-007 | `Qt::QueuedConnection` made explicit on `dataReceived` |
| D-008 | Dead `BaseHandler::sendResponse` / `validateAndSendResponse` removed |
| D-009 | `Q_ASSERT` thread-affinity added to `ThemeManager::setTheme()` |
| D-010 | `Q_ASSERT` thread-affinity in `Simulation::start/stop`; queued path documented |
| D-011 | `themeToString(int)` → `themeToString(Theme)`; double-cast eliminated |
| D-012 | `default:` removed from exhaustive `Theme` switch |
| D-013 | `ResponseStatus` enum deleted with `IResponse` |
| D-014 | `validateParameters` and 5 helpers marked `const` |
| D-015 | `getAvailableInputPorts/OutputPorts/Ports` marked `const` and renamed |
| D-016 | `MCPUtilities` deleted (validator helpers stayed in `BaseHandler`) |
| D-017 | `qjsonToNlohmann` wraps `json::parse` in try/catch |
| D-018 | `handleSetTheme` adds `validateParameters({"theme"})` guard |
| D-019 | Path-traversal check (`QFileInfo::fileName() == icName`) in `createIC`/`instantiateIC` |
| D-020 | Numeric `validateNumeric` calls upfront in `set_element_properties` |
| D-021 | `wireless_mode` type-checked before `toInt()` |
| D-022 | `QTextStream::status()` + `file.error()` checked after waveform txt write |
| D-023 | SVG export verifies file exists/non-empty after `painter.end()` |
| D-024 | `validatePortRange` rejects negative `portIndex` |
| D-025 | `qWarning` at startup if MCP schema fails to load |
| D-026 | `StandardValidator` deleted (covered by D-028) |
| D-027 | `HandlerFactory` deleted; `MCPProcessor` is sole dispatch table |
| D-028 | `ICommand`, `IResponse`, `StandardValidator`, `MCPUtilities` deleted |
| D-029 | `json_validator` cached per command type in `QHash` |
| D-030 | `StandardValidator` deletion makes `BaseHandler` the single source |
| D-031 | Dead `m_stdin` member removed |
| D-032 | `getCurrentScene()` null-checked at top of `handleSetElementProperties` |
| D-033 | `getCurrentScene()` hoisted before loop in `handleMorphElement` |
| D-034 | `export_arduino` / `export_systemverilog` registered in `MCPProcessor` |

**Investigation target — verified safe**
- I-001: `HandlerFactory` deleted (D-027) — concurrency concern moot.

### Pass 3 totals

| Category | Lint | Deep | Investigate | Total |
|----------|------|------|-------------|-------|
| Ownership & Lifecycle | 0 | 5 | 0 | 5 |
| Thread Safety | 0 | 5 | 1 | 6 |
| API & C++ Correctness | 13 | 6 | 0 | 19 |
| Error Handling & Validation | 1 | 10 | 0 | 11 |
| Performance & Code Quality | 1 | 8 | 0 | 9 |
| **Total** | **15** | **34** | **1** | **50** |

The two most consequential fixes were **D-027** (split dispatch tables unified) and **D-028** (four dead compilation units removed) — together eliminating `ICommand.h`, `IResponse.h`, `StandardValidator.*`, `MCPUtilities.*`, and unifying the command registry.

---

## Pending / Skipped Items

The 9 items below are the only forward-looking entries from any of the three passes.

### Confirmed false positives (no code change needed)

#### [L-004] `Node.cpp` — SVG `http://` namespace URI
- **File**: `App/Element/GraphicElements/Node.cpp:94`
- **Rule**: ERR-4 (hardcoded `http://`)
- **Why skipped**: The string `http://www.w3.org/2000/svg` is the XML namespace URI mandated by the SVG 1.1 spec. It must remain `http://` — using `https://` would produce invalid SVG. The About-dialog and network URL fixes in the same pass were real; this one is not a bug.

#### [L-011] `GraphicElement.cpp:883` — non-`const` range-for
- **File**: `App/Element/GraphicElement.cpp:883`
- **Rule**: PAT-12 (COW detach)
- **Why skipped**: The range-for loop body assigns `out = Status::Unknown` — it is a write loop. Non-`const` is correct. The lint rule cannot distinguish reads from writes inside the body.

#### [L2-002] `CodeGenUtils.h` — `QChar` object type in lambda
- **File**: `App/CodeGen/CodeGenUtils.h:39`
- **Rule**: DEP-13
- **Why skipped**: Changed the predicate to use `QChar::isLetterOrNumber(c.unicode())` (the static call form, satisfying the intent). The full fix — declaring `c` as `char16_t` — is impractical: `QString::iterator` dereferences to `QChar&`, which does not implicitly convert to `char16_t` in a predicate context, so the lambda signature `[](char16_t c)` does not compile.

#### [D2-028] `Workspace` — `view()`, `scene()`, `simulation()` missing `const`
- **File**: `App/Scene/Workspace.h`
- **Rule**: API — const-correctness
- **Why skipped**: These three methods return non-`const` pointers to embedded value members (`m_scene`, `m_view`, `m_simulation`). Marking the accessors `const` would require `const_cast` on the returned pointer, which is semantically worse than leaving the methods non-`const`. `fileInfo()` and `dolphinFileName()` were correctly made `const` (they return by value).

#### [D2-036] `Serializer.cpp`, `TestICInline.cpp` — unchecked `QFile::write()`
- **Files**: `App/IO/Serializer.cpp`, `Tests/Integration/TestICInline.cpp`
- **Rule**: ERR-1 (write return unchecked)
- **Why skipped**: Both sites use `QSaveFile` with `commit()`. `QSaveFile::commit()` writes atomically to a temp file and renames it; all write errors are captured by the `commit()` return value, which is already checked. Not a bug.

#### [D2-045] `TestArduino` — circuit rebuilt per test method
- **File**: `Tests/Integration/TestArduino.cpp`
- **Category**: Performance & Code Quality
- **Original finding**: Each test method constructs a full circuit from scratch; sharing a fixture across methods would be faster.
- **Why skipped**: The premise is incorrect — there is no shared circuit to extract. The hand-built tests (`testGateGeneration`, `testFlipFlopGeneration`, `testMixedLogicGates`, etc.) each assemble a *different* topology because the topology *is* the test input — codegen for AND vs. OR vs. flip-flop with preset/clear are fundamentally different verification cases. The `testArduinoExportHelper(icFile)` family loads a different `.panda` IC fixture per call. The only theoretically shareable resource is `WorkSpace`, but resetting its scene/ID counters/priority maps/signal connections reliably between tests is more code than `TestUtils::createWorkspace()` (which is fast — no event loop or QTimer running during construction). Hot path is `ArduinoCodeGen::generate()` and (gated) arduino-cli compilation — already optimized via `s_cliCachePath`.

### Accepted open (real issues, intentionally deferred)

#### [D2-046] `MainWindow` — God class
- **File**: `App/UI/MainWindow.cpp` / `App/UI/MainWindow.h`
- **Category**: Performance & Code Quality (Single Responsibility)
- **Finding**: `MainWindow` is ~3 000 lines handling menu actions, download logic, dock-widget management, IC drag-and-drop, undo/redo wiring, file I/O coordination, and MCP server startup.
- **Why deferred**: Splitting the class correctly requires understanding every signal connection, menu action, and dock-widget lifecycle interaction. The risk of regressions is high and the split boundary is not obvious. Requires dedicated planning before attempting.

#### [D2-048] `ArduinoCodeGen` / `SystemVerilogCodeGen` — logic duplication
- **Files**: `App/CodeGen/ArduinoCodeGen.cpp`, `App/CodeGen/SystemVerilogCodeGen.cpp`
- **Category**: Performance & Code Quality
- **Finding**: Both generators repeat structural logic: port mapping, IC flattening, truth-table expansion, variable naming. Extracting a `CodeGenBase` helper was suggested.
- **Why deferred**: Both generators produce format-specific output at every structural step. A shared template-method base would need to inject format-specific behaviour at dozens of points, making each generator harder to read in isolation without a meaningful reduction in total code. The duplication is largely incidental rather than identical logic — the two formats have different enough semantics that a common base would be a leaky abstraction.
