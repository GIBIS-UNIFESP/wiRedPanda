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
| 1 | App/BeWavedDolphin/ | in progress | DolphinFile.cpp: **done** (100%, 1 line documented-excluded). Still to do: BeWavedDolphin.cpp (75%, largest gap), DolphinExporter.cpp (81%), DolphinModelBuilder.cpp (84%), SignalDelegate.cpp (87%), Serializer.cpp (91%), DolphinClipboard.cpp (95%), WaveformSimulator.cpp (95%), DolphinZoom.cpp (96%), DolphinEdits.cpp (97%), BeWavedDolphin.h (83%, 6 lines), DolphinHost.h (50%, 2 lines) |
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

## Session notes

- 2026-07-19: Plan approved. Tracking doc created.
- 2026-07-19: Fixed a pre-existing build breakage blocking the coverage preset entirely (Qt 6.16 deprecated `QDragEnterEvent(QPoint, ...)`, 6 call sites across 3 test files) — separate commit, not part of the sweep itself.
- 2026-07-19: Discovered Codecov does not honor `LCOV_EXCL_LINE` on the raw-`gcov`-upload path this repo used — switched `.github/workflows/coverage.yml` to generate the report via `gcovr` (which does honor those markers) instead, per user decision.
- 2026-07-19: `App/BeWavedDolphin/DolphinFile.cpp` done — 100%, one documented exclusion (see above). New `TestDolphinFile` test class added (14 tests), registered through all 4 required tiers (`CMakeSources.cmake` x2, `CMakeLists.txt` x2 — `add_test` + `ALL_TESTS`, `Tests/Runners/TestWiredpanda.cpp` x2).
