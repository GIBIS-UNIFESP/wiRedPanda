# Coverage → 100% Tracking

Master plan: directory-by-directory sweep, real behavioral tests only, no
weak assertions, no blanket exclusions. See conversation/plan for full ground
rules. `App/Resources/**` and `MCP/Client/**` are excluded per `codecov.yml`
and out of scope here.

Baseline (Codecov, master, checked 2026-07-19): 89.6% overall. App 80.84%,
Tests 95.66%, MCP/Server 63.71%.

## Sweep order & status

| # | Directory | Status | Notes |
|---|-----------|--------|-------|
| 1 | App/BeWavedDolphin/ | **done** | Every file 100% (176/176 functions), 18/18 files. Fixed 5 real bugs total: 4 null-deref crashes on nullable `m_host`, plus `WaveformSimulator::restoreInputs()` corrupting `InputRotary`'s one-hot dial position instead of restoring it. |
| 2 | App/CodeGen/ | not started | |
| 3 | App/Core/ | not started | |
| 4 | App/Element/ | not started | |
| 5 | App/Exercise/ | not started | |
| 6 | App/IO/ | not started | |
| 7 | App/Scene/ | not started | Scene.cpp (1386 lines, 84.3%), SceneDropHandler.cpp (59.1%, no test file yet), PropertyShortcutHandler.cpp (83.3%, has test file) |
| 8 | App/Simulation/ | not started | |
| 9 | App/Tour/ | not started | |
| 10 | App/UI/ | not started | LanguageManager.cpp (73.2%, has test file) |
| 11 | App/Wiring/ | not started | |
| 12 | MCP/Server/Core/ | not started | MCPValidator.cpp (56.7%) |
| 13 | MCP/Server/Handlers/ | not started | ThemeHandler/ServerInfoHandler/ConnectionHandler/HistoryHandler/SimulationHandler have zero test files; FileHandler/ICHandler have partial (Security-focused) tests |
| 14 | Tests/** (dead branches in harness itself) | not started | e.g. TestSystemVerilogExport.cpp differential mode (50.4%) |

## Documented exclusions log

| File | Line | Justification |
|------|------|----------------|
| App/BeWavedDolphin/DolphinFile.cpp | `save()`'s commit-failure throw | Not deterministically reachable from a single-user test process: `QSaveFile::open()` already rejects a non-regular-file destination up front, so the remaining commit()-only failure modes (e.g. losing rename permission via a sticky-bit dir) need a second OS user or root. |
| App/BeWavedDolphin/BeWavedDolphin.cpp | `checkSave()`'s `default:` switch case | The dialog offers exactly 3 StandardButtons (Save/Discard/Cancel), so `QMessageBox::question()` can only return one of those — `default` can't be hit. |
| App/BeWavedDolphin/BeWavedDolphin.cpp | `exportToPng()`'s `catch (...)` block | `DolphinExporter::renderToPixmap()` only calls Qt widget/painting APIs (none throw) and `QPixmap::save()` reports failure via its bool return, not an exception. Kept as a defensive backstop; provably dead given the current call graph. |
| App/BeWavedDolphin/BeWavedDolphin.cpp | `snapshot()` / `allCellIndexes()` closing braces | Compiler-generated cleanup for local non-trivial-destructor objects (`WaveformSnapshot`, `QModelIndexList`) on an exception-unwind path neither function ever takes. |
| App/BeWavedDolphin/DolphinModelBuilder.cpp | `collect()`'s `if (!elm \|\| type() != GraphicElement::Type) continue;` | `Scene::elements()` (the only real caller) already filters via `qgraphicsitem_cast` to items whose `type() == GraphicElement::Type`, so this can't be null or the wrong type from a real `Scene`. |
| App/BeWavedDolphin/DolphinModelBuilder.cpp | `buildLabels()`'s closing brace (both template instantiations) | Same compiler-generated `QStringList` cleanup pattern as above. |
| App/BeWavedDolphin/Serializer.cpp | `loadBinary()`'s closing brace | Same pattern, for `WaveformData`. |
| App/BeWavedDolphin/WaveformSimulator.cpp | `captureInputs()`'s closing brace | Same pattern, for the returned `QVector<Status>`. |
| App/BeWavedDolphin/DolphinHost.h | `virtual ~DolphinHost() = default;` | Defaulted virtual destructor calls are inlined at each derived-class destruction site (`StubDolphinHost` etc. destruct constantly in tests) — gcov just never attributes an inlined call back to this declaration line. **Watch for this pattern recurring throughout the rest of the sweep** — any `= default` special member function is a likely false-miss candidate before assuming a real gap. |

**Recurring exclusion patterns to recognize on sight** (don't re-derive from scratch each time):
1. A function's closing `}` shown as uncovered when the function itself clearly ran → compiler-generated cleanup for a local non-trivial-destructor object's exception-unwind path that's never taken. Exclude the closing brace line.
2. A `qCDebug`/`qCWarning` call split across multiple physical lines shows the first line covered but continuation lines `#####` even when the log call is definitely reached → the streamed arguments only evaluate when the category is enabled (default verbosity is 0 in tests), and gcov only credits the *first* line for the always-evaluated enabled-check. Fix: reformat onto one physical line (matches this codebase's existing single-line convention anyway) rather than fiddling with `Comment::setVerbosity()` in tests.
3. `= default` special member functions (esp. virtual destructors) may show as uncovered even when heavily exercised, since the trivial body gets inlined at each call site with no attributable standalone execution. Exclude with a one-line justification rather than chasing it.

## Session notes

- 2026-07-19: Plan approved. Tracking doc created.
- 2026-07-19: Fixed a pre-existing build breakage blocking the coverage preset entirely (Qt 6.16 deprecated `QDragEnterEvent(QPoint, ...)`, 6 call sites across 3 test files) — separate commit, not part of the sweep itself.
- 2026-07-19: Discovered Codecov does not honor `LCOV_EXCL_LINE` on the raw-`gcov`-upload path this repo used — switched `.github/workflows/coverage.yml` to generate the report via `gcovr` (which does honor those markers) instead, per user decision.
- 2026-07-19: `App/BeWavedDolphin/DolphinFile.cpp` done — 100%, one documented exclusion (see above). New `TestDolphinFile` test class added (14 tests), registered through all 4 required tiers (`CMakeSources.cmake` x2, `CMakeLists.txt` x2 — `add_test` + `ALL_TESTS`, `Tests/Runners/TestWiredpanda.cpp` x2).
- 2026-07-19: `App/BeWavedDolphin/BeWavedDolphin.cpp` done — 100%, three documented exclusions (see above). Found and fixed 4 real null-pointer-dereference crashes on `m_host` (documented/tested as nullable, but dereferenced unconditionally in `on_actionSaveAs_triggered()`, `associateToWiRedPanda()`, `createWaveform(fileName)`, and `on_actionLoad_triggered()`) — caught by two new tests that segfaulted before the fix. 30 new cases added to the existing `TestBewavedDolphinGui` (no new registration tiers needed, extended an existing class). Full suite (193 tests) passes; one pre-existing parallel-ctest flake (`TestSystemVerilogExport::testSystemVerilogExportClockGatedDecoder`) observed and confirmed unrelated (passes in isolation) — matches the known `project_flaky_parallel_tests` pattern, not a regression.
- 2026-07-19: `App/BeWavedDolphin/` **fully done** — every file 100%. Closed out DolphinModelBuilder.cpp, DolphinExporter.cpp, Serializer.cpp, SignalDelegate.cpp, DolphinClipboard.cpp, WaveformSimulator.cpp, DolphinZoom.cpp, DolphinEdits.cpp, plus the last two header lines, via 6 new test classes and additions to 3 existing ones. Found and fixed a real bug: `WaveformSimulator::restoreInputs()` corrupted `InputRotary`'s dial position (one-hot: selecting any port deselects the others) instead of restoring it, since it unconditionally called `setOn(value, port)` for every port rather than only the one actually captured as active — caught by a new test that failed before the fix. Discovered and documented two recurring gcov false-miss patterns (compiler-generated cleanup braces, multi-line qCWarning/qCDebug attribution) — see the exclusion table above; expect both again in later directories. Full suite (199 tests) green.
