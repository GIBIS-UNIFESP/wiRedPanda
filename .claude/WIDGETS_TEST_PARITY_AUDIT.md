# Widgets → Quick test-parity audit

Tracks disposition of every test class still registered via `add_test(... COMMAND test_wiredpanda
...)` in `CMakeLists.txt` (185 total), per the "full port first, then delete everything" plan in
`.claude/plans/we-were-profiling-in-hidden-emerson.md` ("Current phase" section — read that first
for context). Legend: **SUPERSEDED** (Quick equivalent exists, old file safe to delete, no new
work) · **N/A** (tests a Widgets-only concept with no Quick equivalent needed, confirmed by
reading source, not assumed) · **PARTIAL** (Quick equivalent exists but a real, specific gap
remains) · **MISSING** (zero Quick coverage, needs a real port) · **TBD** (not yet individually
audited this pass).

## SUPERSEDED — confirmed, safe to delete once Phase I lands

| Old class | New class | Basis |
|---|---|---|
| `TestElementHandler` | `TestQuickElementHandler` | 1:1 mirror per `TestQuickElementHandler.h` doc comment |
| `TestFileHandlerSecurity` | `TestQuickFileHandlerSecurity` | 1:1 mirror per its doc comment |
| `TestICHandlerSecurity` | `TestQuickICHandlerSecurity` | 1:1 mirror, security-relevant, prioritized early |
| `TestElementPalette` | `TestQuickElementPalette` | 4→19 tests, closes `TestMainWindowGui`'s drag/double-click-add tests too |
| `TestElementEditor` | `TestQuickElementEditor` | 9→34 tests, closes most of the context-menu gap too |
| `TestGraphicsView` | `TestQuickCanvasZoom` | 4 of 7 ported 1:1; other 3 dispositioned below |
| `TestMinimapWidget` | `TestQuickMinimap` | 7 of 20 ported (real geometry/nav logic); other 13 dispositioned below |
| `TestConnectionManager` | `TestConnectionValidity` + `TestCanvasCommands` + `TestCanvasPortHover` | read in full: its 4 non-hover tests are weak smoke tests with no unique ConnectionManager-specific assertion (e.g. `testMultiPortConnection` only checks `andGate.inputs().size() > 0`); its 5 hover tests are what `TestCanvasPortHover` ports. Zero unique value left unported. |
| `TestCommands` | `TestCanvasCommands` | per `TestCanvasCommands.h` doc comment (mirrors intent, not line-by-line — id-based vs QGraphicsScene-pointer resolution) |
| `TestElementTabNavigator` | `TestQuickElementEditor` | test names are near-identical (`testReadingOrderIsRowMajor`/`TieBreaksLeftToRight`/`EmptyAndSingle` literally match); `testTabNavigation`/`testBacktabNavigation`/`testTabWrapAround` map to `testCycleLabelField*`/`testCycleTriggerField*` |

## N/A — confirmed, no Quick equivalent needed (feature/API doesn't exist in this architecture)

- `TestLabeledSlider`, `TestTrashButton`, `TestICDropZone` — test actual `QWidget` subclasses (`LabeledSlider`/`TrashButton`/`ICDropZone`) that don't exist in the QML UI at all.
- `TestGraphicsView::testFastMode` — `QGraphicsView`'s CPU-raster `QPainter` render-hint toggle; `CanvasItem`'s GPU-composited `QSGGeometryNode` rendering has no equivalent (confirmed via `TestQuickCanvasZoom.h`'s doc comment, reading `CanvasItem.h/.cpp` in full).
- `TestGraphicsView::testDragModeToggle` — `QGraphicsView::ScrollHandDrag`/`NoDrag` settable mode; `CanvasItem`'s pan is unconditional gesture-driven state (`m_panning`/`m_spacePanHeld`), no settable-mode concept exists.
- `TestMinimapWidget`'s 12 resize/move/hover-cursor tests (`testResizeModeAt*`, `testApplyResize*`, `testMoveHandleRect*`, `testIsMoveHandle*`, `testMoveByClamps*`, `testHoverStateOver*`) — `MinimapWidget`'s hand-rolled C++ resize/move hit-testing was deliberately not ported; `Minimap.qml`'s resize handles/move strip are QML-native `MouseArea` drag math instead (confirmed via `TestQuickMinimap.h` doc comment + `project_minimap_qml_landed.md`).

## Real, confirmed, already-tracked gaps (not silently dropped by prior passes)

- **Accessibility** (`GraphicsView::setAccessibleName()`/`setWhatsThis()`) — zero `Accessible.*` usage anywhere in `App/QuickShell/` (confirmed via grep). Surfaces as `testAccessibleNameSet` in `TestGraphicsView`, `TestMinimapWidget`, and `TestElementPalette` — one single real feature gap, not three. **Phase F candidate**, but genuinely new feature work (nothing to test yet), not just a test-writing gap.
- **`TestMainWindowGui`** (138 tests) — NOT one big fresh gap. Its scope was already split across `TestQuickAppController` (7e-1), `TestCanvasEmbeddedIC` (7e-2), `TestCanvasInlineIC` (7e-3), `TestCanvasItemInteraction` (7e-4), `TestQuickElementEditor` (7e-5), `TestQuickElementPalette` (7e-6) — each header names exactly which slice it covers. Confirmed remaining PARTIAL gap, per `TestQuickAppController.h`'s own "out of scope" list: dialog/window-chrome-specific tests (About, fullscreen, theme switching, language change, toolbar/status-bar widget presence, Learn menu structure) — needs a real shown-`QQuickWindow` + `QtQuick.Dialogs` interaction technique this branch hasn't needed before. **Phase F.**
- **`TestBewavedDolphinGui`** (51 tests) — mirrored by `TestQuickDolphinController` (40 tests) per its doc comment, with 6 explicitly tracked exceptions: `testAboutDialog`/`testAboutQtDialog` (no About/AboutQt menu in `DolphinWindow.qml` yet), `testSaveAsAction`/`testExitAction` (dialog-driven, underlying `DolphinFile::save()` round-trip already covered), `testMergeSplitDisabled` (feature doesn't exist on either side — true N/A, not a gap), `testShortcutsReconciledWithMainWindow` (QML `Shortcut` items vs `QAction::shortcut()`, needs its own check). **Small, well-scoped PARTIAL — fold into Phase F.**
- **`TestCanvasCommands`** (16 tests, vs old `TestSceneUndoredo`'s 63 and `TestCommands`'s own count) — deliberately deferred, not dropped: `CanvasRegisterBlobCommand`/`RemoveBlobCommand`/`RenameBlobCommand`/`UpdateBlobCommand` (embedded-IC blob bookkeeping — `CanvasICRegistry`'s own Phase 3 verification already covers the underlying registry logic once) and `CanvasToggleTruthTableOutputCommand` (trivial single-bit toggle, low risk). **Needs a real name-by-name comparison against `TestSceneUndoredo`'s 63 tests before calling this fully closed — still TBD below.**
- **`TestElementContextMenu`** (3 tests) — `testMorphMenuAction` is covered by `TestQuickElementEditor::testPrepareContextMenuPopulatesGateMorphCandidates`/`testMorphSelectionToAppliesToWholeSelection`. `testRotateRightAction`/`testICSubcircuitAction` have no confirmed Quick-side equivalent by name yet (may already be covered incidentally by `TestCanvasItemInteraction`'s rotate tests / `TestCanvasEmbeddedIC`'s subcircuit-open tests — not yet cross-checked). **Small residual PARTIAL, fold into Phase E.**
- **`TestDialogs`** (16 tests) — the 5 wireless-Tx-label tests are confirmed closed (Phase 7 commit `b64beac85`). The 11 `ClockDialog`/`LengthDialog` min/max/range tests are likely **N/A-by-redesign**: `ElementEditor.qml` edits clock period inline via a `LabeledSlider`-equivalent QML control (confirmed via grep), not a modal dialog — but the exact clamp/range logic hasn't been confirmed to have an equivalent QuickElementEditor test yet. **PARTIAL, needs one real check in Phase E, likely resolves to mostly-N/A.**
- **`TestICUnit`** (7 tests) — **PORTED 2026-07-20** (4 of 7): `testICLoadFromFile`/`testICPortLabelResolution`/`testICNestedSaveLoad`/`testICInvalidFile`. `testICPreviewPopupRespectsDisabledSetting`/`testPreviewPopupClampsToScreen` confirmed **N/A**: `ICPreviewPopup` is Widgets-only; `ICPreviewPopup.qml`'s `reposition()` function is the real screen-clamp equivalent (confirmed by reading it — its own comment calls out `ICPreviewPopup::clampedPopupPos()` by name) but is QML-internal, not independently unit-testable the way a static C++ method is — a real, permanent testing-technique gap, not a silently-dropped test. `testDoubleClickOpensSubCircuitNotInlineEditor` is a **REAL MISSING FEATURE** (see below), not ported.
- **`TestICRegistry`** (5 tests) — **PORTED 2026-07-20** (2 of 5): `testICRegistration`/`testICValidation`. `testMakeBlobSelfContainedRejectsDeepDependencyChain` matches the already-tracked `makeSelfContained()` deferral. `testICFileWatcher`/`testRecursiveICLoading` confirmed **N/A** on closer read (not just assumed portable, as an earlier pass here guessed): both test `registerBlob()`'s recursive dependency-chain walk (file watching for the first, resolving a short legitimate chain for the second) — `CanvasICRegistry::registerBlob()` performs no recursion at all (`CanvasICRegistry.h`'s own doc comment confirms this), so there is nothing meaningful left to exercise.
- **`TestExerciseEngine`** (7 tests) — **PORTED 2026-07-20** (6 of 7): retranslate/clamp/warn behavior, re-targeted at `ExerciseEngine::setCanvas(CanvasItem*)` (the type-erased `std::function`-based replacement for `setScene(Scene*)`). `testOverlayFontScalesWithApplicationFont` confirmed **N/A** (`ExerciseOverlay` is Widgets-only).
- **`TestSimulationUnit`** (5 tests) — confirmed **zero name overlap** with the already-ported `TestSimulation`. Pure `Simulation` domain logic — **MISSING**.
- **`TestSimulationBlocker`** (3 tests) — pure `SimulationBlocker`/RAII domain logic, small — **MISSING**.
- **`TestDanglingPointer`** (16 tests) — named regression tests for specific historical crash/dangling-pointer bugs in `Simulation` (`bug1_`..`bug8_`, `hardening_*`, `jd_*`, `integration_*`, `hcDrainConnectionsMustCleanRegistry`). Pure `Simulation`/`ICRegistry` domain logic, zero Widgets signal, high real value (these guard against real historical crashes, not just feature coverage) — **MISSING, prioritize in Phase E.**
- **`TestNotifyCatch`** (2 real tests + init/cleanup) — tests `Application::guardedSlot()`'s exception-safety wrapper, which the plan's Step 1 already confirmed is real, load-bearing, portable Quick-side infrastructure (used throughout `QuickAppController`/`QuickWorkSpace`/etc). Pure domain logic, needs re-targeting at the post-split `Application` class — **MISSING**.
- **`TestTruthTable`** (20 tests) — mix of pure domain (`testConstructorInitialization`, `testSetKey`, `testKeyBitArray`, port naming/positioning, save/load-key-version, ~14 tests) and rotation/pivot/flip geometry (`testRotationKeepsSizingPixmap`, `testRotatedIconStaysUpright`, `testBigPivotsAtBoundingRectCenter`, `testBigRotationDoesNotDriftInScene`, `testBigFlipDoesNotDriftInScene` — 5 tests, likely overlapping or related to `TestIc`'s already-known-open "8 rotation/pivot-geometry tests", not yet cross-checked). **MISSING** for the domain half, needs cross-check against `TestIc`'s open rotation gap for the geometry half.
- **`TestICInline`** (164 tests, vs `TestCanvasInlineIC` + `TestCanvasEmbeddedIC`'s 20 combined) — read in full. Not a miscount: `TestCanvasEmbeddedIC`/`TestCanvasInlineIC` mirror slices of `TestMainWindowGui`, a **different** file — `TestICInline` itself (blob registry CRUD, embed/extract/flatten propagation, undo/redo commands, serialization round-trips) has no confirmed port anywhere yet. Breaks down as:
  - **~22 tests are Widgets-UI-specific** (`testICDropZone*`, `testTrashButtonDragAcceptance`, `testSceneDrop*`/`testSceneAddItemMimeDataThrowCleansUp` — old `Scene`'s drag/drop handler, `testElementLabelMime*`, `testElementPaletteUpdateEmbeddedICList*`/`testElementPaletteSearchFindsEmbeddedIC`/`testElementPaletteRefreshAfterRemoveAll`, `testContextMenuICActionConditions`) — likely mostly **N/A** or need a QML-native drop-gesture equivalent (Phase 7 already noted no established technique exists yet for synthesizing QML `DropArea`/`MouseArea` drag gestures) — not yet individually confirmed.
  - **~142 tests are pure domain logic** (blob registry CRUD/persistence/versioning, embed/extract/flatten with all their edge cases, undo/redo for `RegisterBlobCommand`/`RemoveBlobCommand`/`RenameBlobCommand`/`UpdateBlobCommand` — note these map directly to the already-built-but-untested `CanvasRegisterBlobCommand` etc. flagged above under `TestCanvasCommands` — serialization mismatch/fallback, workspace inline-tab/autosave interaction). This is the **single largest remaining MISSING bucket outside the CPU/Level suite** — comparable in scale, deserves its own dedicated port pass in Phase E, likely with real overlap-deduplication against `TestCanvasEmbeddedIC`/`TestCanvasInlineIC`/`TestCanvasCommands` needed as it's ported (not assumed disjoint).

## REAL MISSING FEATURE found this pass (not just a test gap)

**Show Gates / Show Wires visibility toggle** — `App/Scene/VisibilityManager.h/.cpp` ("Controls
the visibility of gates, wires, and port handles... Called by `Scene::setCircuitUpdateRequired()`
and by MainWindow UI toggles" — i.e. a real View-menu feature in the Widgets app) is **not
referenced anywhere in `CMakeSources.cmake`** — fully excluded from `wiredpanda_lib`, and
`grep -rn "showGates\|showWires" App/QuickShell/` returns nothing. This is a genuine unported
user-facing feature, the same category as the port-hover/Tab-cycling gaps Phase 7 found and later
shipped — **not discovered by the earlier Phase 7 audit**, found only by chasing `TestScene`'s
`testShowGatesToggle`/`testShowWiresToggle`/`testGateVisibilityStateTracking`/
`testWireVisibilityStateTracking`/`testShowGatesWithMultipleElements`/
`testShowWiresWithMultipleConnections` (6 tests) down to a real feature check, not just a name
match. **User decision 2026-07-20**: track it (task created) and finish the rest of the Phase B audit
first, then implement — same shape as the port-hover work, separate from the mechanical
test-parity phases. Must land before Phase I deletes `VisibilityManager.*`.

**IC double-click opens inline label editing, not the sub-circuit** — found while porting
`TestICUnit::testDoubleClickOpensSubCircuitNotInlineEditor`, which asserts the opposite
priority order on the Widgets side (`IC::requestOpenSubCircuit` fires, inline-edit does not).
`CanvasItem::mouseDoubleClickEvent()` (`App/QuickShell/Canvas/CanvasItem.cpp`) currently emits
`inlineEditRequested()` for ANY double-clicked element with a label, IC included, and
`grep -rn "requestOpenSubCircuit" App/QuickShell/` confirms that signal (still declared on `IC`)
is never connected to anything in the Quick app — there is no way to open a nested/embedded IC's
sub-circuit as its own tab from a double-click at all right now. Already flagged, not silently
missed: `CanvasItem::mouseDoubleClickEvent()`'s own comment says "requestOpenSubCircuit's actual
tab-opening half stays out of scope, no inline-IC-tab UI trigger exists yet." **Phase F
candidate** — real feature work (needs an actual open-as-tab flow, not just a signal connection),
same shape as the two gaps above.

## TestScene (63 tests) — spot-checked, not exhaustively read

Confirmed via fuzzy name-matching + verifying real Quick-side test bodies call the same
`CanvasItem` methods (`copyAction`/`cutAction`/`pasteAction`/`rotateLeft`/`flipVertically`/
`deleteAction` are all real, ported, and exercised by `TestCanvasCommands`/`TestCanvasEmbeddedIC`/
`TestQuickAppController` under different test-function names): the copy/paste/rotate/flip/delete/
undo/redo/selection/query/sort-topology/morphing/property-cycle test groups (~54 of the 63) are
**SUPERSEDED**, just named differently — this needs a real 1:1 name-to-name mapping written down
before deleting the old file (not done yet, tracked as TBD), not a re-port. `testForgetItemIdC7`/
`testResizeSceneQuantizesSceneRect` are the confirmed-open gap from Phase 7 memory. The
show-gates/show-wires group (6 tests) is the real missing feature above, not a superseded-vs-gap
question. `testDroppedPandaFileDetection`, `testWireAntialiasingDegradesOnSlowPassesAndRestores`
(matches the plan's own already-documented "wire-antialiasing-degradation system... already a
documented deletion, not a migration" — **N/A**), `testMuteSilencesAllAudioOutputElements`,
`testClipboardCanPasteMatchesPasteFormats`, `testBuildDragImageClampsExtremeSelectionExtent`,
`testGeometryChangeKeepsSceneIndexConsistentKS`, `testKeyTriggerIgnoresAutoRepeat`,
`testArrowKeyNudgesSelection`, `testPortAtFindsPortsOnly` need individual confirmation.

## TestSceneUndoredo (63 tests) — one confirmed gap found, rest still TBD

`alignLeft`/`alignRight`/`alignTop`/`alignBottom`/`alignHorizontalCenter`/`alignVerticalCenter`/
`distributeHorizontally`/`distributeVertically` are real, ported `CanvasItem` methods (confirmed
in `CanvasItem.h`) with **zero test coverage anywhere in `test_wiredpanda_quick`** (confirmed via
grep — no Quick test calls any of them). This directly corresponds to `TestSceneUndoredo`'s 9
`testAlign*`/`testDistribute*` tests. **Real, confirmed MISSING test gap — feature exists,
untested.** The other ~54 tests (move/rotate/flip/morph/wireless-mode/undo-redo-stack-behavior/
cross-tab-isolation) are largely likely superseded by `TestCanvasCommands`, same pattern as
`TestScene` above, but not yet individually confirmed.

## TBD — not yet individually audited this pass

`TestDialogs`' exact clamp/range equivalence (see PARTIAL above), `TestIC`/Integration (34 tests,
distinct from `TestICUnit` above — `TestIc`'s 8 rotation/pivot tests already known-open per
`project_phase7_audit_zero_quick_test_coverage.md`, rest unconfirmed), `TestScene`'s ~9 remaining
individual names above, `TestSceneState` (30 — tests `Scene`'s own internal state directly, no
separate `SceneState` class exists; likely maps to `CanvasItem`'s internal state under a very
different structure, not yet cross-checked), `TestSceneUndoredo`'s ~54 remaining tests above,
`TestFileDialogProvider` (20, already known "not started" per Phase 7 memory).

**Recommendation**: rather than continuing this class-by-class audit exhaustively before any
porting starts, the plan's own "Recommended starting point" (Phase C, the harness port) is now
the higher-leverage next step — it unblocks judging/porting the largest single bucket (~80
CPU/Level classes) as a group. The remaining TBD items above are real but individually small;
finishing them can interleave with Phase E rather than gating Phase C's start.

**Update 2026-07-20**: all 11 `TestCPU*` classes (the CPU integration-level sub-bucket) are now
fully ported and green — `TestCPUProgramCounter`, `TestCPUAlu`, `TestCPUBranch`,
`TestCPUControlUnit`, `TestCPUDecoders`, `TestCPUInstructionExecute`, `TestCPUInstructionFetch`,
`TestCPUIntegration`, `TestCPUMemoryInterface`, `TestCPURegisterBank`, `TestCPURegisters`. This
also surfaced a real, confirmed (AddressSanitizer) production heap-use-after-free: destroying a
procedural-render-pixmap element (IC/Mux/Demux/TruthTable) with a remaining multi-fanout output
connection read already-freed input ports during teardown. Fixed in `App/Element/ElementPorts.cpp`
+ `App/Wiring/Port.{h,cpp}` (commit `a9b0f670b`) — a real bug, not a test-porting artifact, so it
was root-cause-fixed in production code, not worked around in the test. Remaining in this bucket:
the ~80-class `TestLevel*` suite (Level1-9).

**~150-class pure-domain bucket** (Level1-9/CPU suite + `TestSerialization`/`TestClocksAdvanced`/
`TestDemux`/`TestDisplays`/`TestLogicGates`/`TestMux`/`TestSequentialLogic`/`TestPriorities`/
`TestMemorySettlingTime`/`TestSequential`/`TestMuxDemuxComprehensive`/`TestArduino`/
`TestArduinoCodeGenUnit`/`TestSystemVerilogExport`/`TestSystemVerilogCodeGenUnit`/`TestTourEngine`/
`TestCircuitExporter`/`TestWorkspaceUnit`/`TestConnections`) — disposition is already known
(MISSING, needs porting), individual per-class audit isn't the bottleneck here, the harness port
(Phase C) is. See the plan file for the full list and the 97-file/2-chokepoint leverage finding.

**Update 2026-07-20 (Phase E batch 1)**: `TestNotifyCatch` (2 tests), `TestSimulationUnit` (5
tests), `TestSimulationBlocker` (3 tests), `TestDanglingPointer` (15 of 16 tests — see below) are
now ported and green. `TestDanglingPointer::bug7_icRegistryFileChangedMustNotLeaveDanglingPointers`
was dropped (not just skipped): it regression-tested `ICRegistry::onFileChanged()`'s
`QFileSystemWatcher`-driven hot-reload, and `CanvasICRegistry.h`'s own doc comment already
confirms file watching is deliberately not ported yet — there is no Quick-side function left to
check. This pass found a third real production bug (distinct from the two found in Phase D):
`Port::drainConnections()` bare-deletes a `Connection` during cascade-teardown, bypassing
`CanvasItem::removeItem(Connection*)` — the connection stayed in `m_connections`/the id registry
pointing at freed memory (WIREDPANDA-HC class), later double-freeing at `~CanvasItem()`'s own
`qDeleteAll(m_connections)`. Fixed via a `Connection::setDestroyedCallback()` hook (commit
`f63ac4b93`), separate from the mechanical test-port commit (`dc0fe23e2`).

## Next steps

1. Phase C (harness port) unblocks judging the ~80-class CPU/Level bucket as a group once a
   handful of Level1/Level2 classes are ported and their test bodies read for real (not just
   counted).
2. Resolve `TestICInline` vs `TestCanvasInlineIC`/`TestCanvasEmbeddedIC` — the single largest
   unresolved count gap (164 vs 20 combined).
3. Read the remaining TBD list above for real (not by name/count alone).
