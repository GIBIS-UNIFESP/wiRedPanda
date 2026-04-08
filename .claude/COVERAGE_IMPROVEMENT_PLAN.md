# Coverage Improvement Plan

## Context

Starting coverage: **80.7% lines** (10,848/13,447), **87.0% functions** (1,247/1,433).
Current coverage (after Phase 1): **~81.9% lines** (11,087/13,528), **~87.9% functions** (1,266/1,438).
Goal: **100% line coverage** across all App/ source files.
Gap: **~2,441 uncovered lines** across ~85 files.

### Progress

| Date | Lines Hit | Total | Coverage | Delta |
|------|-----------|-------|----------|-------|
| Baseline | 10,848 | 13,447 | 80.7% | — |
| Phase 1 (2026-04-08) | 11,087 | 13,528 | 81.9% | +139 lines |

### Key improvements from Phase 1

| File | Before | After |
|------|--------|-------|
| `BeWavedDolphin/Serializer.cpp` | 0.0% | **91.4%** |
| `Core/Enums.cpp` | 47.4% | **96.5%** |
| `IO/FileUtils.h` | 0.0% | **60.0%** |
| `CodeGen/SystemVerilogCodeGen.h` | 63.6% | **81.8%** |
| `CodeGen/SystemVerilogCodeGen.cpp` | 66.4% | **69.0%** |

### Coverage by Module (baseline)

| Module | Lines | Hit | Uncovered | Coverage |
|--------|------:|----:|----------:|---------:|
| Simulation | 241 | 225 | 16 | 93.4% |
| Nodes | 348 | 313 | 35 | 89.9% |
| Element | 3,643 | 3,164 | 479 | 86.9% |
| Scene | 2,212 | 1,832 | 380 | 82.8% |
| Core | 560 | 451 | 109 | 80.5% |
| UI | 3,283 | 2,568 | 715 | 78.2% |
| BeWavedDolphin | 1,113 | 821 | 292 | 73.8% |
| IO | 277 | 202 | 75 | 72.9% |
| CodeGen | 1,770 | 1,272 | 498 | 71.9% |

---

## Phase 1: High-Impact Pure Logic — DONE (+139 lines)

### 1A. `App/CodeGen/SystemVerilogCodeGen.cpp` — 329→304 uncovered (66.4%→69.0%) ✅

**File**: `Tests/Unit/CodeGen/TestSystemVerilogCodeGenUnit.cpp` + `.h`

Implemented: 6 tests building connected circuits with `CircuitBuilder` (InputSwitch→AND→Led,
InputSwitch→OR→NOT→Led, multi-gate chain, Mux, Demux, empty scene). Each test writes to a
temp file and verifies output contains `module`, `input`, `output`, `assign` keywords.

**Remaining uncovered**: IC module generation paths (requires loading `.panda` files with ICs),
TruthTable codegen, wireless Tx/Rx, name collision resolution.

### 1B. `App/BeWavedDolphin/Serializer.cpp` — 58→5 uncovered (0%→91.4%) ✅

**File**: `Tests/Unit/Serialization/TestDolphinSerializer.cpp` + `.h`

Implemented: 11 tests using the actual `DolphinSerializer` namespace API — binary save/load
round-trips via `QDataStream`, CSV save/load round-trips via `QSaveFile`/`QFile`, edge cases
(empty model, corrupted data, multiple columns/rows). Verifies `inputPorts` and `columns`
match after round-trip.

**Remaining uncovered**: `cols < 2` validation throw path, `rows > maxInputPorts` clamping.

### 1C. `App/Core/Enums.cpp` — 30→2 uncovered (47.4%→96.5%) ✅

**Extend**: `Tests/Unit/Common/TestEnums.cpp`

Missing switch cases in `nextElmType`/`prevElmType`:
Implemented 5 new test methods in `Tests/Unit/Common/TestEnums.cpp`:
- `testFlipFlopCycling`: DFlipFlop↔TFlipFlop, JKFlipFlop↔SRFlipFlop (both directions)
- `testOutputCycling`: Led→Buzzer→AudioBox→Led (both directions)
- `testInputCycling`: full 6-element InputVcc→...→Clock→InputVcc cycle (both directions)
- `testNotNodeCycling`: Not↔Node pair
- `testUnknownTypeReturnsUnknown`: Mux, Demux, IC, Unknown → Unknown

**Result**: 47.4%→96.5% (+28 lines) ✅. Remaining 2 uncovered lines are the `operator++`
return statement and one `default` branch.

### 1D. `App/IO/FileUtils.h` — 30 uncovered lines (0%)

**New file**: `Tests/Unit/Serialization/TestFileUtils.cpp` + `.h`

Inline utility functions `copyToDir` and `copyPandaDeps`.

**File**: `Tests/Unit/Serialization/TestFileUtils.cpp` + `.h`

Implemented: 7 tests covering `copyToDir` (empty path, valid file copy, file already exists,
resource path) and `copyPandaDeps` (basic, recursive, no dependencies). Note: `copyToDir` is
`void` (not `bool`), `copyPandaDeps` takes 3 args `(pandaPath, srcDir, destDir)`.

**Result**: 0%→60% ✅. Remaining 40% requires real `.panda` files with valid preamble/metadata
for `copyPandaDeps` to read past the early return.

---

## Phase 2: Scene & Element Logic (+~315 lines → ~85.5%)

### 2A. `App/Scene/PropertyShortcutHandler.cpp` — 61 uncovered lines (51.2%)

**New file**: `Tests/Unit/Scene/TestPropertyShortcutHandler.cpp` + `.h`

Test each element type's property cycling:
- Create element, add to scene, select it
- Call `nextMainProperty()`/`prevMainProperty()` → verify property changed
- Cover: And/Or input count, Clock frequency, Led color, Display14/16, InputRotary, TruthTable

**Expected**: ~55 lines covered

### 2B. `App/Element/GraphicElements/Display14.cpp` (40 uncovered) + `Display16.cpp` (42 uncovered)

**Extend**: `Tests/Unit/Elements/TestDisplays.cpp` (or existing display test)

- Set each color ("White","Red","Green","Blue","Purple") → verify `color()`
- Set specific input port statuses → call `paint()` (offscreen) to cover segment branches
- Save/load round-trip with non-default color

**Expected**: ~70 lines covered

### 2C. `App/UI/ElementEditor.cpp` — 164 uncovered lines (67.7%)

**New file**: `Tests/Unit/Ui/TestElementEditor.cpp` + `.h`

Uncovered branches: property application for Delay, Audio, Trigger, Appearance, Volume, WirelessMode; Mux/Demux input/output size UI; retranslateUi.

**Tests to write**:
- Select Clock → change frequency via spinbox → verify
- Select Buzzer → change audio combo → verify
- Select Led → change color → verify
- Select Mux → verify input size combo options
- Select Demux → verify output size combo options
- Test `retranslateUi()` refreshes combos
- Use `ScopedFileDialogStub` for appearance paths

**Expected**: ~100 lines covered

### 2D. `App/UI/ElementContextMenu.cpp` — 131 uncovered lines (0%)

**New file**: `Tests/Unit/Ui/TestElementContextMenu.cpp` + `.h`

The `exec()` method blocks on `QMenu::exec()`. Strategy: use `QTimer::singleShot(0, ...)` to programmatically trigger menu actions, similar to `TestDialogs` pattern.

- Build Gate element → trigger "Rotate right" action
- Build Input element → test morph menu
- Build IC element → test sub-circuit actions

**Expected**: ~80-100 lines covered

---

## Phase 3: BeWavedDolphin & Small UI (+~135 lines → ~86.5%)

### 3A. `App/UI/ElementTabNavigator.cpp` — 29 uncovered lines (29.3%)

**New file**: `Tests/Unit/Ui/TestElementTabNavigator.cpp` + `.h`

- Build scene with 3-4 elements at different positions
- Simulate Tab key → verify selection advances in reading order
- Test Backtab for reverse, wrap-around behavior

**Expected**: ~25 lines

### 3B. `App/BeWavedDolphin/WaveformView.cpp` — 15 uncovered lines (55.9%)

**Extend**: `Tests/System/TestBewavedDolphinGui.cpp`

- Test `zoomIn()`/`zoomOut()`/`resetZoom()` at boundary levels
- Simulate wheel events for zoom redirect paths

**Expected**: ~15 lines

### 3C. Remaining BeWavedDolphin files (~100 uncovered across multiple files)

**Extend**: `Tests/System/TestBewavedDolphinGui.cpp`

- Test binary `.dolphin` save/load round-trip explicitly
- Test CSV export/import path
- Test `merge()`/`split()` operations

**Expected**: ~80-100 lines

---

## Phase 4: Main UI & Scene (+~500 lines → ~91%)

### 4A. `App/UI/MainWindow.cpp` — 310 uncovered lines (71.1%)

**Extend**: `Tests/System/TestMainWindowGui.cpp`

Largest single source of uncovered UI lines. Uncovered areas:
- File operations: `openFile()`, `saveFile()`, `saveAsFile()`, `exportToImage()`, `exportToArduino()`, `exportToSystemVerilog()` — lines 505-540, 566-596
- Toolbar/menu setup: `retranslateUi()` — lines 1644-1710
- Theme switching: `setTheme()`, `updateTheme()` — lines 1824-1895
- Recent files: `populateRecentFiles()`, `openRecentFile()` — lines 989-1005
- View operations: `zoomIn()`, `zoomOut()`, `fitToScreen()` — lines 825-870
- BeWavedDolphin launch: `openBeWavedDolphin()` — lines 1471-1513
- Fullscreen/language: `toggleFullscreen()`, `setLanguage()` — lines 1564-1594

**Tests to write**:
- Use `ScopedFileDialogStub` for all file dialog paths
- Test `openFile()` with a fixture `.panda` file → verify scene loaded
- Test `saveAsFile()` → verify file written
- Test export functions (Arduino, SV, image) with stub dialogs
- Test `retranslateUi()` — call and verify no crash
- Test theme switching: `setTheme(Dark)`, `setTheme(Light)` → verify applied
- Test `zoomIn()`/`zoomOut()`/`fitToScreen()` on a loaded scene
- Test `populateRecentFiles()` with mock settings
- Test `toggleFullscreen()` in offscreen mode

**Expected**: ~200 lines covered (some lines like actual dialog display remain untestable)

### 4B. `App/Scene/Scene.cpp` — 139 uncovered lines (74.3%)

**Extend**: `Tests/Unit/Scene/TestScene.cpp` or `Tests/System/TestSceneUndoredo.cpp`

Uncovered areas:
- `loadElements()` error paths — lines 239-243, 524-546
- `saveToFile()` / `loadFromFile()` error branches — lines 614-755
- `flipHorizontally()` / `flipVertically()` — lines 640-730
- `rotateElements()` for multi-selection — lines 733-764
- Wireless element management — lines 856-870, 883-906
- `clearScene()` — lines 967-979

**Tests to write**:
- Load a scene, select multiple elements, call `flipHorizontally()` → verify positions
- Same for `flipVertically()` and `rotateElements()`
- Test `clearScene()` → verify empty
- Add wireless Tx/Rx elements → verify wireless management
- Test error paths: load corrupt file, save to read-only path

**Expected**: ~110 lines covered

### 4C. `App/Scene/Workspace.cpp` — 63 uncovered lines (83.7%)

**Extend**: existing workspace tests or create `Tests/Unit/Scene/TestWorkspace.cpp`

Uncovered: tab management (lines 85-122), undo/redo state (175-188), IC editing (235-238, 308-309), full-screen handling (341-356), file modification tracking (507-543)

**Tests to write**:
- Create workspace → add/remove tabs → verify count
- Modify scene → check `isModified()` → undo → check again
- Open IC for editing → verify sub-workspace created
- Test `setFullScreen()` in offscreen mode

**Expected**: ~50 lines covered

### 4D. `App/Scene/ConnectionManager.cpp` — 45 uncovered lines (68.3%)

**New file**: `Tests/Unit/Scene/TestConnectionManager.cpp` + `.h`

Uncovered: connection creation paths (lines 33-42), validation (62-105), multi-port connections (133-170), connection deletion (210-218)

**Tests to write**:
- Create two elements → connect output to input via ConnectionManager → verify connection exists
- Try invalid connections (output-to-output, same-element) → verify rejected
- Connect, then delete connection → verify removed
- Test multi-port fan-out

**Expected**: ~40 lines covered

### 4E. `App/Scene/Commands.cpp` — 50 uncovered lines (93.0%)

**Extend**: `Tests/System/TestSceneUndoredo.cpp`

Uncovered: specific undo/redo edge cases — `DeleteElementCommand` with IC (line 463), `ChangeInputSizeCommand` (764-771), `RotateCommand` (1033-1068), `MorphElementCommand` (1096)

**Tests to write**:
- Delete an IC element → undo → verify restored with connections
- Change input size on a gate → undo → verify original size
- Rotate element → undo → verify original angle
- Morph element (And→Or) → undo → verify original type

**Expected**: ~45 lines covered

### 4F. `App/Scene/GraphicsView.cpp` — 16 uncovered lines (83.0%)

**Extend**: existing scene tests

Uncovered: zoom limits (lines 49-54), key event handling (85-88), drag mode (127-138)

**Tests to write**:
- Test zoom at min/max limits
- Simulate key events (space for pan, delete for element removal)

**Expected**: ~16 lines covered

---

## Phase 5: Element Module (+~479 lines → ~95%)

### 5A. `App/Element/GraphicElement.cpp` — 47 uncovered lines (90.1%)

**Extend**: `Tests/Unit/Elements/TestGraphicElement.cpp`

Uncovered: `paint()` selection visuals (120-130), `save()`/`load()` edge cases (351-371), skin/appearance (562-575, 607-633), tooltip generation (690-692), `mouseDoubleClickEvent()` (938-944)

**Tests to write**:
- Select element → call `paint()` → cover selection highlight code
- Save/load element with custom appearance/skin → verify round-trip
- Set tooltip, verify format
- Double-click on IC element → verify signal emitted

**Expected**: ~40 lines covered

### 5B. `App/Element/ICRegistry.cpp` — 54 uncovered lines (76.0%)

**Extend**: `Tests/Unit/Elements/TestICRegistry.cpp` or create new

Uncovered: registry reload (39-42, 66-90), IC file watching (154), recursive IC loading (182-193), path resolution (219-223, 262-286), IC validation (311-335)

**Tests to write**:
- Register IC files → verify `availableICs()` returns them
- Test file watcher: modify IC file → verify reload triggered
- Test recursive IC dependency resolution
- Test invalid IC file → verify validation error

**Expected**: ~45 lines covered

### 5C. `App/Element/IC.cpp` — 22 uncovered lines (94.4%)

**Extend**: existing IC tests

Uncovered: `loadFromFile()` error path (122-129), port label resolution (260-262), save/load with nested ICs (522-526, 570, 639, 683)

**Tests to write**:
- Load IC with invalid file path → verify error handling
- Test nested IC save/load round-trip
- Test port label lookup for non-existent label

**Expected**: ~20 lines covered

### 5D. `App/Element/GraphicElementSerializer.cpp` — 16 uncovered lines (93.4%)

**Extend**: `Tests/Unit/Serialization/TestSerialization.cpp`

Uncovered: legacy format branches (125, 142, 227), specific element type serialization (390-525)

**Tests to write**:
- Load backward-compatible fixtures that exercise legacy serialization paths
- Save/load each element type with non-default properties

**Expected**: ~14 lines covered

### 5E. `App/Element/ElementLabel.cpp` — 17 uncovered lines (75.0%)

**Extend**: element tests

Uncovered: label positioning (54-58), edit mode (85-109)

**Tests to write**:
- Create element with label → set text → verify position updates
- Enter edit mode → change text → verify saved

**Expected**: ~15 lines covered

### 5F. `App/Element/GraphicElements/InputRotary.cpp` — 44 uncovered lines (76.7%)

**Extend**: `Tests/Unit/Elements/TestInputElements.cpp`

Uncovered: `paint()` dial rendering (164-219), mouse drag rotation (255-263, 282-316)

**Tests to write**:
- Set rotary value → call `paint()` → cover rendering paths
- Simulate mouse press+drag → verify value changes
- Test min/max value clamping

**Expected**: ~35 lines covered

### 5G. `App/Element/GraphicElements/Led.cpp` — 30 uncovered lines (74.6%)

**Extend**: `Tests/Unit/Elements/TestOutputElements.cpp`

Uncovered: `paint()` color blending (224-275), `setColor()` for all color variants (143-145)

**Tests to write**:
- Test each LED color (White, Red, Green, Blue, Purple) with Active/Inactive states
- Call `paint()` for each color → cover blending logic

**Expected**: ~28 lines covered

### 5H. `App/Element/GraphicElements/Display7.cpp` — 20 uncovered lines (84.3%)

**Extend**: `Tests/Unit/Elements/TestDisplays.cpp`

Uncovered: `setColor()` (50), `paint()` segment rendering (151-166), save/load color (209-219)

**Tests to write**:
- Same pattern as Display14/16: set colors, exercise paint, round-trip save/load

**Expected**: ~18 lines covered

### 5I. `App/Element/GraphicElements/AudioBox.cpp` — 16 uncovered lines (87.4%)

**Extend**: element tests

Uncovered: audio playback paths (119, 133, 169, 185), `save()`/`load()` audio file (208-245)

**Tests to write**:
- Create AudioBox → set audio file → save/load round-trip
- Toggle input → verify playback triggered (offscreen, audio may be mocked)

**Expected**: ~14 lines covered

### 5J. Remaining Element files — ~110 uncovered lines total

Files with 1-13 uncovered lines each:
- `Demux.cpp` (13), `TruthTable.cpp` (13), `Buzzer.cpp` (12), `Clock.cpp` (11), `Mux.cpp` (10)
- `InputButton.cpp` (14), `DFlipFlop.cpp` (4), `DLatch.cpp` (4), `JKFlipFlop.cpp` (4)
- `SRFlipFlop.cpp` (4), `SRLatch.cpp` (4), `TFlipFlop.cpp` (4), `Line.cpp` (4)
- `Node.cpp` (2), `Nor.cpp` (2), `Xnor.cpp` (2), `Text.cpp` (6)
- `And.cpp` (1), `Or.cpp` (1), `Not.cpp` (1), `Nand.cpp` (1), `Xor.cpp` (1)
- `InputSwitch.cpp` (1), `InputGND.cpp` (1), `InputVCC.cpp` (1)
- `ElementFactory.cpp` (3), `ElementMetadata.cpp` (1)
- Headers: `GraphicElement.h` (3), `IC.h` (1), `ICRegistry.h` (1)

**Extend**: various existing element test files

**Tests to write**:
- For flip-flops: test `save()`/`load()` with non-default trigger edge
- For Demux/Mux: test `paint()` with various sizes
- For Buzzer/Clock: test property edge cases
- For InputButton: test `mousePress`/`mouseRelease` events
- For gates with 1 uncovered line: typically a `paint()` branch or rare `save()`/`load()` path

**Expected**: ~100 lines covered

---

## Phase 6: CodeGen & IO Completion (+~240 lines → ~97%)

### 6A. `App/CodeGen/ArduinoCodeGen.cpp` — 165 uncovered lines (78.6%)

**New file**: `Tests/Unit/CodeGen/TestArduinoCodeGenUnit.cpp` + `.h`

Existing integration tests use `.panda` → golden `.ino` comparisons. Uncovered paths:
- Simple non-IC circuit generation (57, 96, 101-136)
- Mux/Demux Arduino code generation (630-649, 713, 721-725)
- TruthTable Arduino generation (869-882)
- Memory/FlipFlop Arduino generation (911-1117) — large block of ~150 lines
- Error handling / element type dispatch (282, 318, 397, 521, 530, 542)

**Tests to write**:
- Build simple circuits with `CircuitBuilder` → call `generate()` → verify output
- Circuits with: Mux, Demux, TruthTable, DFlipFlop, JKFlipFlop, SRFlipFlop, DLatch, SRLatch
- Each sequential element exercises specific Arduino codegen (set/reset/toggle patterns)

**Expected**: ~140 lines covered

### 6B. `App/IO/Serialization.cpp` — 41 uncovered lines (78.6%)

**Extend**: `Tests/Unit/Serialization/TestSerialization.cpp`

Uncovered: legacy format loading (98-106), IC dependency resolution (203-213), error recovery (336-378)

**Tests to write**:
- Load files from `Tests/BackwardCompatibility/` to exercise legacy paths
- Test file with missing IC dependency → verify graceful error
- Test corrupt file → verify error handling

**Expected**: ~35 lines covered

### 6C. `App/CodeGen/SystemVerilogCodeGen.h` — 4 uncovered lines (63.6%)

Covered by Phase 1A tests when exercising header-inlined methods.

**Expected**: ~4 lines covered

### 6D. `App/IO/RecentFiles.cpp` — 4 uncovered lines (88.2%)

**Extend**: existing tests

- Test `addFile()` with duplicate → verify no duplicate in list
- Test list overflow (max entries) → verify oldest removed

**Expected**: ~4 lines covered

---

## Phase 7: Core, Nodes & Simulation (+~160 lines → ~98.5%)

### 7A. `App/Core/Settings.cpp` — 19 uncovered lines (78.9%)

**Extend**: existing tests or create `Tests/Unit/Core/TestSettings.cpp`

Uncovered: theme-specific setting paths (135-205)

**Tests to write**:
- Save settings → load → verify round-trip for all setting types
- Test default value fallbacks for missing keys

**Expected**: ~17 lines covered

### 7B. `App/Core/Application.cpp` — 9 uncovered lines (50.0%)

Uncovered: `notify()` exception catch block, Sentry code.

- Test: trigger exception in event handler → verify caught gracefully
- Sentry paths only compile with `HAVE_SENTRY` flag — exclude from coverage target or build with Sentry for coverage run

**Expected**: ~5 lines (non-Sentry)

### 7C. `App/Core/SystemThemeDetector.cpp` — 8 uncovered lines (88.1%)

Uncovered: platform-specific theme detection fallbacks

- Test: call `detectTheme()` → verify returns valid theme
- Platform branches may need conditional testing

**Expected**: ~6 lines covered

### 7D. `App/Core/Common.cpp` — 5 uncovered lines (84.4%)

**Extend**: existing tests

- Test utility functions with edge-case inputs

**Expected**: ~5 lines covered

### 7E. `App/Core/ThemeManager.cpp` — 5 uncovered lines (95.0%)

Uncovered: rare theme switching paths

- Test: switch between all available themes

**Expected**: ~5 lines covered

### 7F. `App/Nodes/QNEConnection.cpp` — 24 uncovered lines (87.0%)

**Extend**: `Tests/Unit/Nodes/TestQNEConnection.cpp`

Uncovered: connection path update (119-135), hover effects (157-163), paint selection (204-215), destruction (320-323, 372-386)

**Tests to write**:
- Create connection → update path → verify geometry
- Set hover state → call `paint()` → cover highlight path
- Select connection → `paint()` → cover selection path
- Delete connection → verify cleanup

**Expected**: ~22 lines covered

### 7G. `App/Nodes/QNEPort.cpp` — 7 uncovered lines (95.4%)

**Extend**: existing port tests

- Test tooltip generation, hover state painting

**Expected**: ~7 lines covered

### 7H. `App/Nodes/QNEConnection.h` — 2 uncovered, `QNEPort.h` — 2 uncovered

Inline methods — covered when exercised through .cpp tests above.

**Expected**: ~4 lines covered

### 7I. `App/Simulation/Simulation.cpp` — 16 uncovered lines (93.4%)

**Extend**: `Tests/Unit/Simulation/TestSimulation.cpp`

Uncovered: error handling (57-58), clock management (90, 119-120), element removal (183-184), loop detection (229, 250, 263)

**Tests to write**:
- Test simulation with no elements → verify no crash
- Add/remove clock elements during simulation
- Create feedback loop → verify detection
- Remove element mid-simulation → verify cleanup

**Expected**: ~14 lines covered

---

## Phase 8: Hard-to-Test UI Remnants (+~155 lines → ~99.5%)

### 8A. `App/UI/TrashButton.cpp` — 32 uncovered lines (22.0%)

Uncovered: drag-enter/drop events, delete confirmation dialog.

**Tests to write**:
- Programmatic `QDragEnterEvent` with valid mime data → verify accepted
- Programmatic `QDropEvent` → use `QTimer::singleShot` to dismiss QMessageBox
- Test with empty drop → verify rejected

**Expected**: ~25 lines covered

### 8B. `App/UI/FileDialogProvider.cpp` — 13 uncovered lines (31.6%)

The real provider calls native OS file dialogs — inherently untestable in headless mode.

**Strategy**: Accept as-is OR refactor to pass `QFileDialog` as dependency for testability.

**Expected**: ~0 lines (accept as untestable)

### 8C. `App/Core/UpdateChecker.cpp` — 31 uncovered lines (0%)

Requires network access to GitHub API.

**Strategy**: Introduce a mock `QNetworkAccessManager` or use `QNetworkAccessManager` subclass that returns canned responses.

**Tests to write**:
- Mock HTTP response with newer version → verify update available signal
- Mock HTTP response with same version → verify no update signal
- Mock network error → verify error handling

**Expected**: ~28 lines covered

### 8D. `App/UI/LanguageManager.cpp` — 14 uncovered lines (82.5%)

Uncovered: language file scanning (86-100), translator installation (132-136, 170-171)

**Tests to write**:
- Call `availableLanguages()` → verify returns list
- Call `setLanguage("pt_BR")` → verify translator loaded

**Expected**: ~12 lines covered

### 8E. `App/UI/ICDropZone.cpp` — 8 uncovered lines (83.3%)

Uncovered: drop event handling, IC instantiation from palette drag

**Tests to write**:
- Programmatic drop event with IC mime data

**Expected**: ~6 lines covered

### 8F. `App/UI/ElementPalette.cpp` — 6 uncovered lines (96.1%)

Uncovered: rare UI paths (search filtering edge case, palette rebuild)

**Expected**: ~4 lines covered

### 8G. `App/UI/LabeledSlider.cpp` — 3 uncovered lines (94.4%)

Uncovered: value change signal edge case

**Expected**: ~3 lines covered

### 8H. `App/UI/LengthDialog.cpp` — 2 uncovered lines (83.3%)

Uncovered: dialog accept/reject paths

**Expected**: ~2 lines covered

### 8I. `App/UI/CircuitExporter.cpp` — 1 uncovered line (95.8%)

**Expected**: ~1 line covered

### 8J. `App/UI/SelectionCapabilities.cpp` — 1 uncovered line (98.5%)

**Expected**: ~1 line covered

### 8K. Remaining headers with 1 uncovered line each

`Core/Application.h` (1), `Core/Enums.h` (1), `Scene/Commands.h` (1), `Scene/VisibilityManager.cpp` (1), `Element/ToneGenerator.h` (1), `UI/TrashButton.h` (1)

**Expected**: ~6 lines covered

---

## Coverage Projection

| Phase | New Lines | Cumulative | Coverage % |
|-------|-----------|------------|------------|
| Current | — | 10,848 | 80.7% |
| Phase 1: Pure Logic | ~330 | 11,178 | 83.1% |
| Phase 2: Scene & Element Logic | ~315 | 11,493 | 85.5% |
| Phase 3: BeWavedDolphin & Small UI | ~135 | 11,628 | 86.5% |
| Phase 4: Main UI & Scene | ~460 | 12,088 | 89.9% |
| Phase 5: Element Module | ~330 | 12,418 | 92.3% |
| Phase 6: CodeGen & IO Completion | ~183 | 12,601 | 93.7% |
| Phase 7: Core, Nodes & Simulation | ~85 | 12,686 | 94.3% |
| Phase 8: Hard-to-Test UI Remnants | ~88 | 12,774 | 95.0% |

### Residual Untestable Lines (~73 lines, ~0.5%)

Some lines are inherently untestable in headless CI:
- `FileDialogProvider.cpp` real dialog calls (~13 lines)
- `Main.cpp` entry point
- `#ifdef HAVE_SENTRY` blocks (only in deploy builds)
- Platform-specific rendering paths that only execute on real display servers

Reaching true 100% requires either: (a) excluding these files from coverage measurement via lcov filters, or (b) refactoring to make them injectable/mockable.

---

## New Test Files Summary

| Phase | File | Target |
|-------|------|--------|
| 1A | `Tests/Unit/CodeGen/TestSystemVerilogCodeGenUnit.cpp` | SystemVerilog non-IC paths |
| 1B | `Tests/Unit/Serialization/TestDolphinSerializer.cpp` | BeWavedDolphin serialization |
| 1D | `Tests/Unit/Serialization/TestFileUtils.cpp` | File copy utilities |
| 2A | `Tests/Unit/Scene/TestPropertyShortcutHandler.cpp` | Property cycling |
| 2C | `Tests/Unit/Ui/TestElementEditor.cpp` | Editor property branches |
| 2D | `Tests/Unit/Ui/TestElementContextMenu.cpp` | Context menu actions |
| 3A | `Tests/Unit/Ui/TestElementTabNavigator.cpp` | Tab navigation |
| 4D | `Tests/Unit/Scene/TestConnectionManager.cpp` | Connection creation/validation |
| 5B | `Tests/Unit/Elements/TestICRegistry.cpp` | IC file management |
| 6A | `Tests/Unit/CodeGen/TestArduinoCodeGenUnit.cpp` | Arduino non-IC paths |

## Registration Checklist (for each new test class)

1. Add `.cpp` + `.h` to `CMakeSources.cmake` (`TEST_WIREDPANDA_SOURCES` / `TEST_WIREDPANDA_HEADERS`, alphabetically sorted)
2. Add `#include` + `TestEntry` in `Tests/Runners/TestWiredpanda.cpp`
3. **CRITICAL**: Add `add_test(NAME TestClassName COMMAND test_wiredpanda TestClassName ...)` in `CMakeLists.txt` — without this, `ctest` won't run the test and coverage won't be collected!
4. If the class name collides with an existing test class (check `Tests/Integration/`), append `Unit` suffix to the new class (e.g. `TestWorkspaceUnit`, `TestICUnit`, `TestRecentFilesUnit`)

### Naming collisions found so far

| New Unit class | Collides with | Renamed to |
|----------------|---------------|------------|
| TestRecentFiles | Tests/Unit/Common/TestRecentFiles | TestRecentFilesUnit |
| TestFileDialogProvider | Tests/Integration/TestFileDialogProvider | TestFileDialogProviderUnit |
| TestWorkspace | Tests/Integration/TestWorkspace | TestWorkspaceUnit |
| TestIC | Tests/Integration/TestIc | TestICUnit |

## Verification

After each phase:
```bash
cmake --preset coverage && cmake --build --preset coverage --clean-first
find build -name '*.gcda' -delete
ctest --preset coverage
lcov --capture --directory build --output-file build/coverage.info --ignore-errors mismatch
lcov -r build/coverage.info '/usr/*' '*/Qt/*' '*/build/*' '*/Tests/*' -o /tmp/cov.info --ignore-errors inconsistent
lcov --list /tmp/cov.info | grep -E 'target_file_pattern'
```
