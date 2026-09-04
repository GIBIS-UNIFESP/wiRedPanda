# qtquick-rewrite-v2 progress

Recreation of `qtquick-rewrite` (130 commits, `wiredpanda-quick` worktree) by intent, reordered
for a proper dependency trail. Full rationale, disposition table, and reorder table live in
`~/.claude/plans/before-doing-any-of-wondrous-music.md` (the plan this branch executes).

Branch: `qtquick-rewrite-v2`, off `master` (`1220dfc98`).

## Status

| Phase | Status | Source commits covered | Target commits produced | Verification run |
|---|---|---|---|---|
| 1 — Setup | DONE | — | worktree + branch + this file | — |
| 2 — Domain decoupling refactor | DONE | Sub-phase A (incl. `7fab413`, `f852554bf`, `0d92e52`) | 1 commit (108 files) | `cmake --build --preset debug` clean from scratch; `nm` confirms zero `QWidget` symbols in `wiredpanda_lib`; `WIREDPANDA_BUILD_WIDGETS_APP=ON` configures but fails to build (expected/accepted — Scene/UI/Commands/MCP-handlers layer, to be rebuilt fresh in Phase 6, not patched) |
| 3 — Test harness + QuickCircuitBuilder | DONE | `36bacf7d`, `d515d3a7e` | 7 commits | `test_wiredpanda_quick` standalone (no `WIREDPANDA_BUILD_WIDGETS_APP`), all 7 KEEP-tagged classes ported and green: 148/148 test functions passing (14+18+12+16+14+25+39, incl. init/cleanup) via direct run and `ctest`; `nm` still zero `QWidget` symbols; valgrind clean on every class (only the harness's own documented `QTemporaryDir` leak) |
| 4 — Domain regression-suite port (expanded) | DONE — original scope plus the 2026-08-31-audit-found pure-domain bucket, both closed (see Feature-gap tracking) | Sub-phases C+D+E + `TestSerialization`/`TestTourEngine`/`TestSystemVerilogExport`/`TestSystemVerilogCodeGenUnit`/`TestArduinoCodeGenUnit`/`TestConnections`; `TestCircuitExporter`/`TestICRegistry` confirmed non-portable, moved to Phase 13 | 113 commits | Full verification pass run for the first time this phase (build/test had been deferred file-by-file throughout): `cmake --build --preset debug --target test_wiredpanda_quick` clean; all 114 Quick CTest targets pass (0 failed); `nm` confirms zero `QWidget` symbols. Found and fixed 5 real defects the deferred-build strategy had hidden (2 pre-existing from earlier commits this phase, 3 from the session's own final files) plus 4 pre-existing IC-fixture label/element overlaps surfaced by `TestICFixtureLayout` running for the first time — see detail below |
| 5 — Canvas + rendering core, complete | DONE | Phase 1 + Phase 2 + Phase 7.5 + port-hover deferral + Show Gates/Wires (new) | 15 commits | Every commit built clean and ran crash-free under both offscreen and real X11/GPU rendering (qmllint clean throughout); full 114-target Quick CTest suite re-run and green after every commit touching shared production headers (GraphicElement.h, Text.h). See detail below |
| 6 — CanvasItem services + commands | DONE (core commands, including `PropertyShortcutHandler` keyboard-cycling — landed here originally; a 2026-08-31 audit mistakenly flagged it as missing, corrected 2026-09-01); IC-double-click fix deferred to Phase 7 (now fixed, see Feature-gap tracking) | Phase 3 (all 7 sub-steps) | 8 commits | Every commit built clean and ran crash-free under both offscreen and real X11/GPU; qmllint clean throughout; full 114-target Quick CTest suite re-run and green after every commit. See detail below |
| 7 — Dialogs + workspace + menu/toolbar shell | DONE (original scope); `ICController`'s 5 remaining toolbar/menu methods (found unported 2026-08-31) DONE 2026-09-01, post-hoc | Phase 4 sub-steps 1-3 | 7 commits | Every commit built clean and ran crash-free under both offscreen and real X11; qmllint clean throughout. The menu/toolbar/action-shell commit surfaced and fixed 3 real production bugs via GDB + AddressSanitizer (ThemeManager GUI-thread assertion on the render thread, `static`-storage teardown-order SIGSEGV, and a real heap-use-after-free double-free of `QuickWorkSpace` between QML's GC and `m_tabs`' `unique_ptr`). IC-double-click fix was NOT done as of this row's original writing (needed the inline-label editor too, Phase 9) — confirmed DONE by a later phase (`CanvasItem::mouseDoubleClickEvent()` special-cases IC, emitting `icOpenRequested()`); this row's own text was simply never updated. Found completely untested (2026-09-01) despite being implemented; test added, see Phase 6 row. See detail below |
| 8 — Element palette + editor | DONE | Phase 4 sub-steps 4-5 | 3 commits | Every commit built clean and ran crash-free under both offscreen and real xcb; qmllint clean throughout; full 114-target Quick CTest suite re-run and green. Surfaced and fixed one real, previously-unnoticed bug in this branch's own canvas (plain click-to-select never worked) as foundational work for the editor. See detail below |
| 9 — IC hover preview, misc widgets, theme/i18n, export wiring | DONE | Phase 4 sub-steps 6-8 | 5 commits | Every commit built clean and ran crash-free under both offscreen and real xcb; qmllint clean throughout; full 114-target Quick CTest suite re-run and green after every commit touching shared production headers. Found and fixed one real, ASan-confirmed use-after-free (QuickElementEditor/QuickICPreview's m_canvas needed QPointer) and one real, empirically-confirmed CMake gap (wiredpanda_quick had zero embedded translation catalogs -- its qt_add_translations() call was nested inside the default-off WIREDPANDA_BUILD_WIDGETS_APP block). See detail below |
| 10 — Pan/zoom + minimap | DONE | Phase 4 sub-step 7 | 2 commits | Every commit built clean and ran crash-free under both offscreen and real xcb; qmllint clean on every touched/new QML file; full 114-target Quick CTest suite re-run and green after both commits (`CanvasItem.h/.cpp` changed). See detail below |
| 11 — Startup polish + CLI + perf pass 1 | DONE (original scope); update-check feature (found missing 2026-08-31) DONE 2026-09-01, post-hoc | Phase 4 deferrals + CLI + perf commits | 7 commits (6 landed + 1 found bugfix) | Every commit built clean and ran crash-free under both offscreen and real xcb; qmllint clean on every touched QML file; full 114-target Quick CTest suite re-run and green after every commit. See detail below |
| 12a — Exercise/Tour overlays + MCP hosting | DONE | Phase 5 (`40635c751`) | 1 commit | Full 114-target Quick CTest suite green (2267 individual test functions); qmllint clean; live MCP client round-trip (element create/wire/simulate/undo-redo, `close_circuit`, clean exit on stdin EOF) under both offscreen and real xcb. See detail below |
| 12b — Dolphin waveform port | DONE | Phase 6 (`dc94754ee`) | 1 commit | Full 114-target/2267-function Quick CTest suite green; qmllint clean; `WP_QUICK_SELFTEST` full waveform lifecycle under both offscreen and real xcb; live MCP `create_waveform`/`export_waveform` round-trip. Full `ctest --preset debug` gate run at the end of Phase 12 per the plan. See detail below |
| 13 — UI-specific test suite (expanded) | DONE (2026-09-02) — all commit-backed Phase 7 sub-steps DONE; `TestArduino`/`TestICInline` DONE 2026-08-31; `QuickWorkSpace` residual scope DONE 2026-08-31; MCP handler parity DONE 2026-08-31; Exercise/Tour Quick controller tests DONE 2026-09-01; `TestScene`/`TestSceneState` audit CLOSED 2026-09-02; final 2 gaps (`TestFileDialogProvider` real-dialog tests, `TestWorkspaceUnit` minimap-geometry-clamp tests) both CLOSED 2026-09-02 — neither needed the new test infrastructure originally assumed; see detail below | Phase 7 (7a-7h, all 14 sub-commits) | 17 commits | Full 174-target Quick CTest suite green after every commit. See detail below |
| 14 — Engine perf hardening | DONE (2026-09-02) | Unnumbered engine-hardening series — direct verification found the real scope was 6 items, not 8: `appearanceKeyFor()`'s `QString::arg()` removal was already done (front-loaded in Phase 5); Display7/14/16 classification memoization was moot (superseded by the dirty-flag item below in the source history too, and v2's cache never had a separate field for it) | 6 commits | Full 174-target Quick CTest suite green after every commit. See detail below |
| 15 — Real stock-Fusion pass | TODO | Final chrome-redesign day, re-scoped | | |
| 16 — Legacy Widgets removal + rename | DONE (2026-09-03) | User-directed, ahead of Phase 15 -- the sibling `wiredpanda` worktree stays available as the Widgets visual reference instead | 1 commit | Full 175-target Quick CTest suite green; `cmake --install` smoke test (binary, Examples, .desktop, 5 icon sizes, mime XML) verified against the renamed `wiredpanda` binary. See detail below |

## Phase 2 detail

`GraphicElement`/`Port`/`Connection`/`IC` no longer inherit `QGraphicsItem`/`QGraphicsObject`/
`QGraphicsPathItem`. Every element subclass, `ElementOrientation`, `ElementAppearance`,
`ElementPorts`, `ICLoader`, `ICRenderer`, `Serialization`, `Simulation`/`SimulationHost`, and
`Application` (split into a framework-agnostic core + the new `App/UI/WidgetsApplication`) were
updated to match. `wiredpanda_lib`'s own `QT_LIBS` no longer includes `Qt6::Widgets`/
`Qt6::PrintSupport`/`Qt6::Test` (moved to direct links on the Widgets-only targets) so nothing
that links `wiredpanda_lib` in a later phase (including the eventual `wiredpanda_quick`)
transitively depends on them; `pch.h` was trimmed to the same portable subset.

The rotate/flip geometry (`GraphicElement::pointToScene()`/`rotateFlipTransform()`,
`Port::scenePos()`) was derived and verified three independent ways before writing any code:
an empirical probe against real `QGraphicsItem` behavior, reading Qt's own
`qgraphicsitem.cpp`/`qtransform.cpp` source, and standalone formula validation against both --
per the plan's own flag that this step lost its original visual-verification oracle. Dedicated
geometry unit tests (hand-computed expected matrices) are still owed before Phase 5 builds any
renderer on top -- **not yet done, tracked below**.

`App/Scene/*` (except `SceneItemRegistry`, genuinely portable) and all of `App/UI/*` moved to a
new `WIDGETS_ONLY_SOURCES`/`WIDGETS_ONLY_HEADERS` bucket in `CMakeSources.cmake`, compiled
directly into the `wiredpanda`/`test_wiredpanda` targets instead of `wiredpanda_lib`. Also moved
there: `App/BeWavedDolphin/*` and `App/Exercise/ExerciseEngine`/`ExerciseOverlay` (both
Scene-coupled or `QWidget`-based, and squarely Phase 12 scope, not Phase 2's), `MCP/Server/
Handlers/*` + `MCPProcessor` (all `MainWindow`-coupled; `MCPValidator` stays portable), and two
misplaced-by-directory `QWidget` classes found only by compiling and checking real dependencies
rather than trusting directory names: `App/Element/ElementLabel` (palette drag-label) and
`App/Element/ICPreviewPopup` (hover-preview popup). `App/Tour/TourEngine` and `App/CodeGen/*`
stayed portable; the latter needed `Scene::sortByTopology()`/`wirelessTxInputPorts()` extracted
into a new `App/Element/ElementGraphUtils.h/.cpp` (pure functions, no `Scene` dependency).

`ICRenderer::generatePreviewPixmap()` (the IC hover-preview thumbnail) no longer borrows items
into a throwaway `QGraphicsScene` to render them -- there is no `QGraphicsScene` any more possible
for plain `GraphicElement`/`Port`/`Connection` instances to be added to. Rewritten to draw each
element/port/label/connection directly via their own `paint()`, composing the same
`pointToScene()`/`mapToOwnerLocal()` transforms used everywhere else, plus a manually-computed
scene→device view transform (previously handled implicitly by `QGraphicsScene::render()`).

Two more real, deliberate scope decisions, beyond the 5 feature gaps below: `ICLoader::loadFile()`
dropped an `ICRegistry` cached-file-bytes fast path that needed `GraphicElement::scene()` (a
concept that no longer exists) -- correctness is unaffected (falls back to always reading the
file directly), only a caching optimization is lost, to be rebuilt once a Canvas-side registry
exists. `InputSwitch::handleClick()`'s toggle no longer pushes through the Widgets `Scene`'s
`UpdateCommand` undo/redo path (no command stack to hook into yet) -- the domain-level toggle
still happens, but undo/modified-flag integration for click-toggled inputs needs re-adding once
CanvasItem's own commands land (Phase 6).

**Owed before Phase 5**: the geometry unit tests mentioned above (rotate-then-flip for a
rotatable element about `pixmapCenter()`; scale-then-rotate for a non-rotatable element's own
per-port transform -- the two are genuinely opposite composition orders, confirmed by the same
verification pass, and deserve a regression test each).

## Phase 2 deep-review findings (commit `63ea2139e`)

A deep review of `a85a1d128` (full diff re-read + independent re-derivation of the geometry math
against Qt's own source + empirical verification, not just re-trusting the commit's own claims)
found and fixed two real bugs, both confirmed with a standalone throwaway binary linking
`libwiredpanda_lib.a` directly (something Phase 2's own verification never did, since
`WIREDPANDA_BUILD_WIDGETS_APP=OFF` only ever builds the static library, never an executable):

- `App/Element/GraphicElementLabel.cpp/.h` were never added to `CMakeSources.cmake` — a missing
  translation unit that a static-library-only build can't catch (`ar` doesn't resolve symbols).
  Phase 3's `test_wiredpanda_quick` would have hit this immediately as an undefined-reference
  link error. Fixed by adding both to the SOURCES/HEADERS lists.
- `ElementPorts` allocated ports as plain heap objects but never deleted them outside the
  resize-shrink path — the old `QGraphicsItem`-parent-child auto-deletion this relied on is
  gone, but no replacement was added. Valgrind confirmed 150 blocks / 36,000 bytes definitely
  lost across 50 construct/destroy cycles of a 2-input/1-output element. Fixed with an
  `ElementPorts` destructor; re-verified leak-free.

The geometry composition itself (`GraphicElement::rotateFlipTransform()`/`pointToScene()`,
`Port::scenePos()`/`mapToOwnerLocal()`, `GraphicElementLabel::mapToOwnerLocal()`,
`ICRenderer::generatePreviewPixmap()`'s from-scratch transform chain) was independently
re-derived line-by-line against Qt's actual `qtransform.cpp`/`qgraphicsitem.cpp`/
`qgraphicsitem_p.h` source (not re-trusting the commit message's own claim) and confirmed
correct in every case, including the two deliberately-opposite composition orders.

Two lower-severity items surfaced but were left as-is (judgment calls, not bugs):
- `Text::boundingRect()` no longer unconditionally includes the empty-hint label's box the way
  `QGraphicsItem::childrenBoundingRect()` used to (that Qt method doesn't filter by visibility;
  the new code intentionally does). Likely a minor improvement, but it's an undocumented
  deviation from the "refactor preserves behavior 1:1" rule — flagging in case it's ever visibly
  wrong (e.g. a Text element's hit-box shrinking when its label goes from empty to non-empty).
- The Ctrl+click-swallow logic (`GraphicElement`/`Connection`'s old `sceneEvent()` overrides,
  letting the canvas's own rubber-band/multi-select win over an individual item's click) was
  dropped outright rather than tracked. It's dead code with no current caller either way (no
  interaction layer exists until Phase 5/6), but its *intent* — canvas-level selection should
  still win over an individual element/wire on Ctrl+click — needs to be re-derived as part of
  Phase 5/6's own gesture design, not forgotten. Tracked here since neither the plan nor this
  file mentioned it anywhere else.

## Phase 3 detail

`Simulation::initialize()` had a real, currently-exercised bug found before any test code was
written: a `items.size() == 1` early-return meant "just the Widgets Scene's background-rectangle
padding item, no real circuit" -- an assumption baked into the general `SimulationHost` consumer
that only `Scene::simulationItems()` (which pads with that item) actually satisfies. A genuine
single-element circuit built through any host that doesn't pad the list (`QuickCircuitBuilder`,
and eventually `CanvasItem`'s own host) would silently never simulate. Fixed by removing the
guard entirely (commit `9f36763ea`) -- an empty/near-empty circuit now just builds empty,
harmless simulation state instead of returning `false` forever.

`Tests/QuickShell/QuickCircuitBuilder.h/.cpp` re-derives the reference branch's design (Scene-free
`SimulationHost` implementation wrapping a flat element list + `SceneItemRegistry` for id
assignment) against quick-v2's own current API -- confirmed identical in every place that
mattered (`ItemWithId`/`SceneItemRegistry`/`SimulationHost` signatures all matched already).
Member destruction order (`m_itemRegistry` before `m_owned`) is deliberate, not incidental: the
registry nulls every mapped item's back-pointer in its own destructor specifically so owned
elements destroyed afterward don't self-unregister into an already-dead registry.

Found and fixed a second real, pre-existing bug while wiring up `TestSelectionCapabilities` (the
first ported class): `App/UI/SelectionCapabilities.h/.cpp` has zero Qt Widgets dependency
(verified: no `QWidget`/`QDialog`/`QtWidgets` anywhere) but was swept into `WIDGETS_ONLY_SOURCES`
during Phase 2's CMake categorization anyway -- the same class of directory-vs-dependency mistake
Phase 2 already caught twice for `ElementLabel`/`ICPreviewPopup`, just missed for this one. Fixed
by moving its CMakeSources.cmake registration to the portable list (file path unchanged, matching
the established "categorize by dependency, not directory" precedent) -- `computeCapabilities()`
is now available to `wiredpanda_lib` consumers, confirmed via `nm` both that the symbol exists and
that `QWidget` symbol count is still zero.

New CMake target `test_wiredpanda_quick`: portable-only (`wiredpanda_lib` + `Qt6::Test`, no
Widgets/PrintSupport), defined outside the `WIREDPANDA_BUILD_WIDGETS_APP` guard so it always
builds. Reuses `wiredpanda_lib`'s own PCH (`REUSE_FROM`) since its dependency set is a subset.
CTest names get a `Quick` suffix (e.g. `TestSelectionCapabilitiesQuick`) to avoid colliding with
the Widgets-side `test_wiredpanda`'s same-named registrations when both targets exist together.

Found a second, larger pre-existing bug while porting: `App/UI/SelectionCapabilities.h/.cpp` has
zero Qt Widgets dependency (verified: no `QWidget`/`QDialog`/`QtWidgets` anywhere) but was swept
into `WIDGETS_ONLY_SOURCES` during Phase 2's CMake categorization anyway -- the same class of
directory-vs-dependency mistake Phase 2 already caught twice for `ElementLabel`/`ICPreviewPopup`,
just missed for this one. Fixed by moving its CMakeSources.cmake registration to the portable
list (file path unchanged, matching the established "categorize by dependency, not directory"
precedent).

Similarly, `ConnectionManager::isConnectionAllowed()`/`connectionRejectionReason()` (needed by
`TestConnectionValidity`) turned out to be pure functions over two `Port` pointers with zero
real Widgets dependency, unlike the rest of `ConnectionManager` (which genuinely needs
`QGraphicsView`/`Scene`/`Commands`). Extracted into a new, portable
`App/Wiring/ConnectionValidity.h/.cpp` (same pattern as Phase 2's `ElementGraphUtils`
extraction) -- `ConnectionManager`'s own methods now just delegate, so existing Widgets call
sites are unaffected, and the `QCoreApplication::translate()` context string stays
`"ConnectionManager"` so `.ts` catalogs don't churn.

**All 7 KEEP-tagged classes ported**, each re-derived from master's *current* (often
considerably more expanded than the reference branch's older snapshot) version, not
mechanically transliterated:
- `TestSelectionCapabilities` (12 tests) -- pure `ElementFactory`-built elements, no
  Scene/builder involvement at all beyond direct construction/deletion.
- `TestPort` (17 tests) -- ports verbatim; already fully portable on master.
- `TestConnection` (10 of 12 tests) -- 2 dropped with an explanatory comment
  (`testPaintDisablesAntialiasingWhenSceneWireAaDisabled`: the Scene-coupled adaptive-AA
  feature Phase 2 removed; `testSceneEventSwallowsCtrlClick`: `Connection::sceneEvent()`'s
  Ctrl+click swallow, tied to `QGraphicsScene`'s own event delivery, also removed in Phase 2 --
  both are the same already-tracked Phase 2 gap, not new porting gaps).
- `TestConnectionValidity` (16 tests) -- no Scene/builder needed at all; elements owned via
  `std::unique_ptr`, using the newly-extracted `ConnectionValidity.h` free functions.
- `TestSceneConnections` (13 tests) -- likewise no Scene/builder needed; a couple of
  `scene.elements().size()` sanity checks (redundant with adjacent port-level assertions)
  dropped.
- `TestFeedback` (23 tests) -- first class to actually exercise `QuickCircuitBuilder`'s
  `addOwnedElement()`/`connect()` convenience API end-to-end (SR/D latches, ring oscillators,
  self-loops, 10-element rings, 5 simultaneous independent loops).
- `TestSimulation` (37 tests, the largest at ~1500 lines) -- exercises the harness most
  thoroughly: `WorkSpace`+`CircuitBuilder(Scene*)` maps almost 1:1 onto a bare
  `QuickCircuitBuilder` (identical method names/signatures for add/connect/simulation/
  initSimulation); `Scene::sortByTopology()` calls became the Phase 2-extracted
  `sortByTopology()` free function; heap-allocated `Node`/`IC` instances each wrapped in
  `addOwnedElement()` so the builder owns them (the Widgets `CircuitBuilder`'s `add()` didn't
  need this since the real `Scene` behind it takes ownership on `addItem()` -- `QuickCircuitBuilder`
  deliberately doesn't); local, Quick-native `inputStatus()`/`cpuComponentsDir()` replace the
  two `Tests/Common/TestUtils.h` helpers this file used (that header pulls in
  `App/Scene/Scene.h`/`Workspace.h`, which don't build). Includes a real file-based IC load
  (`wireless_passthrough.panda`) with wireless Tx/Rx nodes inside it, confirming
  `ICLoader`/`ICRenderer`'s Phase 2 rewrite works correctly end-to-end under the new harness.

**Verification**: 148 test functions total across the 7 classes (14+18+12+16+14+25+39,
each count including `initTestCase`/`cleanupTestCase`), all green via both direct run and
`ctest`; full clean rebuild from scratch; `nm` still confirms zero `QWidget` symbols in
`wiredpanda_lib`; valgrind run on every class -- the only leak reported anywhere is the
harness's own already-documented, deliberate `QTemporaryDir` leak in `QuickRunnerUtils.h`
(confirmed by reproducing the identical byte count across multiple classes and multiple runs;
one single non-reproducing "possibly lost" blip during the `TestFeedback` run did not recur in
3 follow-up runs, confirmed as noise rather than a real leak before concluding clean).

## Phase 4 detail (DONE)

User directive for this phase: same exhaustive rigor as Phase 3 for every file -- no
batch/pattern shortcuts, each of the ~84 CPU/Level test files read in full from master,
re-derived individually, built, run standalone, then verified against the full `ctest`
suite before its own commit.

**Shared infrastructure built first** (one-time, all portable, zero Widgets/Scene
dependency): `Tests/QuickShell/IC/QuickTestUtils.h` (Quick-native `inputStatus`/
`setMultiBitInput`/`readMultiBitOutput`/`clockCycle`/`cpuComponentsDir`, replacing the
`Tests/Common/TestUtils.h` free functions this suite needs -- that header pulls in
`App/Scene/Scene.h`/`Workspace.h`, which don't build); `CpuTestUtils.h` (pattern constants +
`get8BitValue`/`loadBuildingBlockIC`); `CpuCommon.h`/`Cpu8bitIsa.h` (copied verbatim --
already fully portable plain enums/free functions); `CpuHelpers.h` (~1150 lines, all 11
circuit-building helper functions ported from `std::unique_ptr<WorkSpace> buildX(Scene*)`
+ `CircuitBuilder builder(workspace->scene())` to `std::unique_ptr<QuickCircuitBuilder>
buildX(...)` + `auto builder = std::make_unique<QuickCircuitBuilder>()`, every
`builder.add(...)` becoming `builder->addOwned(...)`).

**All 11 CPU classes ported** (`TestCPUProgramCounter`, `TestCPUAlu`, `TestCPUBranch`,
`TestCPUControlUnit`, `TestCPUDecoders`, `TestCPUInstructionExecute`,
`TestCPUInstructionFetch`, `TestCPUIntegration`, `TestCPUMemoryInterface`,
`TestCPURegisterBank`, `TestCPURegisters`), each re-derived 1:1 against the already-ported
`CpuHelpers.h` builders, each independently built/run/committed. `initTestCase()`/
`cleanup()` bodies that were either empty comments or a bare
`TestUtils::setupTestEnvironment()` call were dropped throughout -- `runQuickTestSuite()`
(`Tests/QuickShell/Runners/QuickRunnerUtils.h`) already does the offscreen-QPA/
QSettings-redirect/`Application::interactiveMode` equivalent once, globally, for the whole
`test_wiredpanda_quick` binary.

**Two critical, ASan-confirmed engine bugs found and fixed** while porting `TestCPUAlu`
(the first test to destroy heap-allocated elements that still had live wires attached --
routine in this suite's builder-destructor teardown, rare in Phase 3's hand-written unit
tests): (1) `ElementPorts::~ElementPorts()` called `qDeleteAll()` on `m_inputPorts`/
`m_outputPorts` without clearing the vectors afterward, leaving dangling pointers readable
by a reentrant `GraphicElement::portsBoundingRect()` call triggered mid-teardown (a port's
own destructor drains its connections, which recomputes connection geometry, which reads
both endpoints' `scenePos()`, which calls back into `allPorts()`) -- a real heap-use-after-free,
confirmed via `cmake --preset asan`. (2) `GraphicElement.h`'s member declaration order had
`ElementPorts m_ports` declared *before* `m_appearance`/`m_orientation`/`m_sim`, so C++'s
reverse-declaration-order destruction destructed `m_ports` *last*, after those collaborators
were already gone -- the same reentrant geometry computation then read post-destruction
state. Fixed by (1) swapping each port vector to empty before `qDeleteAll()`, and (2)
reordering `m_ports` to be declared last among `GraphicElement`'s collaborator members.
Diagnosed per explicit user instruction to use GDB rather than guess from static stack
traces; ASan (not GDB alone) gave the authoritative before/after confirmation. Verified via
full ASan rebuild+run (all CPU classes green, only the already-documented benign
`QTemporaryDir` LeakSanitizer flag) plus a full debug-preset rebuild+`ctest` (all classes
passing) before committing.

**Verification**: every one of the 11 CPU classes built and run standalone immediately
after being written, then the full `ctest --preset debug -R Quick` suite re-run before each
commit (18 Quick classes total by the end of the CPU sub-scope, 100% passing throughout);
`nm` re-confirms zero `QWidget` symbols after the `GraphicElement.h`/`ElementPorts.cpp` fix.

**Level1 (4/4) and Level2 (13/13) sub-scopes both done**, same one-file-at-a-time rigor,
each independently built/run/committed: `TestLevel1DFlipFlop`, `TestLevel1DLatch`,
`TestLevel1JKFlipFlop`, `TestLevel1SRLatch` (all 4 IC-loading flip-flop/latch classes);
`TestLevel2Decoder2To4/3To8/4To16/5To32`, `TestLevel2FullAdder1Bit`, `TestLevel2HalfAdder`,
`TestLevel2MUX2To1/4To1/8To1`, `TestLevel2ParityChecker`, `TestLevel2ParityGenerator`,
`TestLevel2PriorityEncoder8To3`, `TestLevel2PriorityMUX3To1` (all 13 combinational-IC
classes). All follow the same `IC *ic = loadBuildingBlockIC(...)` + `QuickCircuitBuilder`
shape as the CPU classes; several master originals carried unused
`App/Scene/Workspace.h`/`Tests/Integration/IC/Tests/Cpu/CpuCommon.h`/`App/Core/Common.h`
includes, dropped during the port since they were never referenced by the file's own code.
34 Quick test classes total by the end of Level2, still 100% green via `ctest`.

**Level3 (5/5) and Level4 (13/13) sub-scopes both done**, same rigor:
`TestLevel3ALUSelector5Way`, `TestLevel3BCD7SegmentDecoder`, `TestLevel3Comparator4Bit`,
`TestLevel3Comparator4BitEquality`, `TestLevel3Register1Bit`; `TestLevel4BinaryCounter4Bit`,
`TestLevel4BusMUX4Bit/8Bit`, `TestLevel4Comparator4Bit`, `TestLevel4JohnsonCounter4Bit`,
`TestLevel4RAM4X1/8X1`, `TestLevel4Register4Bit`, `TestLevel4RingCounter4Bit`,
`TestLevel4RippleAdder4Bit`, `TestLevel4RippleALU4Bit`, `TestLevel4ShiftRegisterPISO/SIPO`.
Same shape throughout; several more master originals carried the same unused
Workspace.h/CpuCommon.h/Common.h includes, dropped during the port. 53 Quick test classes
total by the end of Level4, still 100% green via `ctest`.

**Level5 (11/11) sub-scope done** — same rigor: `TestLevel5BarrelRotator`,
`TestLevel5BarrelShifter4Bit`, `TestLevel5ClockGatedDecoder`, `TestLevel5Controller4Bit`,
`TestLevel5InstructionDecoder4Bit`, `TestLevel5LoadableCounter4Bit`,
`TestLevel5ModuloCounter4Bit`, `TestLevel5ProgramCounter4Bit`, `TestLevel5RegisterFile4X4`
(the largest file so far, 49 tests: dual-read-port + 22 memory-timing scenarios incl. a
1000-op stress test), `TestLevel5RegisterFile8X8`, `TestLevel5UpDownCounter4Bit`. 64 Quick
test classes total by the end of Level5, still 100% green via `ctest`.

**Level6 (8/8), Level7 (8/8), Level8 (4/4), Level9 (6/6) sub-scopes all done**, same
rigor throughout: `TestLevel6ALU8Bit`, `TestLevel6ProgramCounter8BitArithmetic`,
`TestLevel6RAM8X8`, `TestLevel6Register8Bit`, `TestLevel6RegisterFile8X8`,
`TestLevel6RippleAdder8Bit`, `TestLevel6StackMemoryInterface`, `TestLevel6StackPointer8Bit`;
`TestLevel7ALU16Bit`, `TestLevel7CPUProgramCounter8Bit`, `TestLevel7DataForwardingUnit`,
`TestLevel7ExecutionDatapath`, `TestLevel7FlagRegister`, `TestLevel7InstructionDecoder8bit`,
`TestLevel7InstructionMemoryInterface`, `TestLevel7InstructionRegister8bit`;
`TestLevel8DecodeStage/ExecuteStage/FetchStage/MemoryStage`; `TestLevel9RegisterFile32x16`,
`TestLevel9FetchStage16bit`, `TestLevel9MemoryStage16bit`, `TestLevel9Cpu16bitRisc`,
`TestLevel9MultiCycleCpu8bit`, `TestLevel9SingleCycleCpu8bit`. This completes the full
CPU (11) + Level1-9 (84) domain regression suite -- 95 classes, matching the branch's own
original CPU/Level scope.

**Non-CPU/Level Phase 4 scope, all done**: `TestICFixtureLayout` (recreated by intent --
a `loadPandaFileRaw()`-based layout linter over every IC Components fixture, not present
in the original branch under this name); `TestNotifyCatch`, `TestSimulationUnit`,
`TestSimulationBlocker`, `TestDanglingPointer` (triaged -- several UI-event-routing and
CanvasItem-lifecycle-specific sub-tests deferred with the same explanatory-comment
precedent as `TestDanglingPointer::bug7`); `TestDecoder8to256`, `TestRamCell1bit`,
`TestMemorySettlingTime`, `TestSequential`, `TestMuxDemuxComprehensive`, `TestDemux`,
`TestMux`, `TestPriorities` (2 ownership bugs found and fixed while porting -- master's
`CircuitBuilder::add()` took ownership via the underlying `QGraphicsScene`;
`QuickCircuitBuilder::add()` is explicitly non-owning, so every heap-allocated element
needed `addOwnedElement()`/`addOwned()` instead); `TestClock`+`TestClocksAdvanced`,
`TestLogicGates`, `TestDisplays` (mechanical ports; `TestClock`'s master original turned
out to already be fully Scene-free); `TestICUnit` (18/28 tests -- 10 UI-dependent
ICPreviewPopup/CanvasItem-event tests deferred; this port drove the creation of
`QuickTestUtils::savePandaFile()`, `backwardCompatibilityDir()`, `ScopedTinyFsizeLimit`);
`TestConnections` (14/18 -- all 4 `ConnectionManager`-based tests deferred after
discovering `ConnectionManager.cpp` is in `WIDGETS_ONLY_SOURCES`, a link-failure risk
initially missed for 2 of the 4); `TestArduinoCodeGenUnit`, `TestSystemVerilogCodeGenUnit`
(full ports, both already Scene-free once their local `sceneElements(Scene*)` helper was
replaced with `builder.elements()`); `TestTourEngine` (20/40 -- the other 20 are
`TourOverlay`-based, a `WIDGETS_ONLY_SOURCES` `QWidget`; belongs in Phase 9); finally
`TestSystemVerilogExport` (all 76 tests port unchanged -- `SystemVerilogCodeGen` takes a
plain `QVector<GraphicElement*>`, never a `Scene`) and `TestSerialization` (all ~100
tests -- `WorkSpace::save()`/`load()`'s own core never touches `Scene`/`WorkSpace` itself;
this port added `QuickTestUtils::savePandaBytes()`/`loadPandaBytes()`/`loadPandaStream()`,
Scene-free replacements for `WorkSpace::save(QDataStream&)`/`load(QDataStream&,...)`,
sharing a new `writePandaPayload()` core with the already-existing `savePandaFile()`).

**Confirmed non-portable, moved to Phase 13** (the plan's own text under-scoped both):
`TestICRegistry` -- every test constructs a real `Scene`/`WorkSpace` (`ICRegistry`'s
constructor is `explicit ICRegistry(Scene *scene)`, "Owned by Scene"), not just the one
`CanvasItem`-dependent sub-test the plan assumed; and `TestCircuitExporter` --
`CircuitExporter::renderToPdf/renderToImage/renderScaledImage` all take `Scene*` directly
and `CircuitExporter.cpp` is itself in `WIDGETS_ONLY_SOURCES`.

**Full verification pass** (build/test had been deferred file-by-file the entire phase,
per explicit user instruction, until every file above was ported and committed): found
and fixed 5 real compile errors the deferred strategy had hidden -- `QVERIFY_THROWS`
doesn't exist before Qt 6.7 and this devcontainer's own Qt 6.9.3 doesn't define it at all
(only `QVERIFY_THROWS_EXCEPTION`); `TestUtils.h` already shimmed this for the Widgets
target, so the same shim was added to `QuickTestUtils.h` (`TestArduinoCodeGenUnit.cpp` and
`TestSerialization.cpp` both needed it); `TestSerialization.cpp` used
`QUOTE(CURRENTDIR)` without defining the macro pair, and called
`GraphicElement::x()/y()` (removed with `QGraphicsItem` in Phase 2 -- `->pos().x()/.y()`
instead); `TestClock.cpp` and `TestSimulationBlocker.cpp` (both from earlier in this
phase) were missing `App/Core/Common.h`/`App/Element/GraphicElement.h` respectively.
After those fixes, `cmake --build --preset debug --target test_wiredpanda_quick` built
clean and all 114 registered Quick CTest targets passed except `TestICFixtureLayoutQuick`,
which found 4 real, pre-existing pixel-level label/element overlaps in IC building-block
fixtures never caught before (nothing had ever asserted on layout): investigated and fixed
at the generator-script level (`level4_johnson_counter_4bit`/`level4_ring_counter_4bit`'s
shared scaffold had `hold_mux0`'s row landing almost exactly level with `CountEnable`'s
own input row; `level5_up_down_counter_4bit`'s `dec_not_borrowN` label is one character
longer than the `inc_not_carryN` sibling that clears the same flat spacing;
`level5_clock_gated_decoder`'s `output_andN`/`gating_andN` labels are the same length but
differ under real font metrics), then regenerated all 4 `.panda` fixtures via
`wiredpanda --mcp` (this branch's own build has `WIREDPANDA_BUILD_WIDGETS_APP=OFF`; used
master's already-built Widgets binary against the identical generator scripts, copied the
regenerated fixtures over, left master's working tree untouched). Purely a
position/spacing change -- `TestSystemVerilogExportQuick`'s connectivity/gate-type-based
golden `.sv` snapshots were unaffected. Final state: all 114 Quick CTest targets pass, 0
failed, `nm` re-confirms zero `QWidget` symbols.

**Next**: Phase 5 (canvas + rendering core).

## Phase 5 detail (DONE)

The real Qt Quick app now exists: `App/QuickShell/CMakeLists.txt`/`Main.cpp`/`Main.qml` (empty
shell, builds unconditionally rather than gated behind `WIREDPANDA_BUILD_WIDGETS_APP`) plus
`App/QuickShell/Canvas/{CanvasItem,SpatialIndex,TextureAtlas}.{h,cpp}`. 15 commits, all built
clean and verified crash-free under both offscreen and real X11/GPU rendering (qmllint clean
throughout).

**Canvas foundation + gestures**: `SpatialIndex` (uniform-grid, box+shape entries) copied
verbatim from the original branch -- already fully Scene/QGraphicsItem-free. `CanvasItem`
renders a small hardcoded demo circuit (grows to 16 elements across this phase) of real
`GraphicElement`/`Connection`/`Simulation` instances, never added to a `QGraphicsScene`.
Drag-to-move, rubber-band multi-select, and wire-creation-by-dragging all ported from
`SceneInteraction`/`ConnectionManager`'s algorithms, reimplemented directly against
`SpatialIndex` rather than calling into those classes (both live in `WIDGETS_ONLY_SOURCES`).
Wire-creation uses the already-portable `App/Wiring/ConnectionValidity.h` free functions
(extracted from `ConnectionManager` during this branch's own Phase 3) instead of
`ConnectionManager::isConnectionAllowed()`, which would compile but fail to link. Deliberately
skipped: the original branch's own throwaway local `QUndoCommand` stand-in for drag-to-move --
building one now just to delete it once Phase 6's real `Commands.h`/id-registry lands would be
exactly the double-work the plan's reordering exists to avoid.

**Texture atlas + full gate-family parity**: `TextureAtlas` (single 2048x2048 page, shelf
packing) copied verbatim, also already Scene-free. Every `GraphicElement` subclass now
renders its own real, unmodified `paint(QPainter*)` offscreen and caches the result --
free-inheritance majority, the `Mux`/`Demux`/`TruthTable` vector-body trio, the
`Display7`/`Display14`/`Display16` segment-compositing trio (needs one extra cache-key
dimension: each input port's live status, since their segment overlays paint on top of an
otherwise-unchanged base pixmap), `InputRotary`/`InputButton`/`Text`, and `IC` (body paint was
already atlas-compatible for free; this phase added its hover-preview signal chain and
verified its internal simulation graph end to end with a real loaded sub-circuit). Found and
fixed a real gap while doing this: `InputButton` was never actually wired up to its momentary
press/release behavior. `GraphicElement` gained one new public accessor,
`appearanceCacheKey()`.

**Phase 7.5 (rendering parity), folded into this phase**: wires are real cubic Bézier S-curves
(tessellated to match `Connection::updatePath()`'s exact curve) with a selection color (from
the now-confirmed-portable `ThemeManager::attributes()`) and a highlight halo on wires
touching a selected element; every port's own circle/triangle status glyph and every
element's label (plus `Text`'s empty-state hint) are baked into the same per-element atlas
tile the body uses. Found and fixed a real correctness bug in the immediately preceding
port-glyph work during this same phase: `Port::currentPen()/currentBrush()` were assumed
stale for a scene-less port (copied from the original branch's own reasoning without
re-verifying against this branch's actual code) -- reading `InputPort::setStatus()`/
`OutputPort::setStatus()` directly showed they're always kept live via `updateTheme()`, so
the hand-rolled pen/brush computation was replaced with a direct `port->paint()` call,
matching `ICRenderer.cpp`'s own established technique for the exact same problem.

**Port-hover peer highlighting + Show Gates/Wires**, closing out the phase: hovering a port
computes it and every port connected to it via a wire and emits the set via
`portHoverChanged(QVariantList)` (screen position/name/real theme colors) for a future chrome
layer to render -- no `QuickAppController`/QML overlay exists yet, so only the signal
emission is in scope here, the same split already used for the IC hover-preview signals.
`setGatesVisible()`/`setWiresVisible()` recreate `VisibilityManager`'s Widgets-only feature
(a real, plan-flagged feature gap, not a test-coverage gap): both toggles are consulted by
`rebuildSpatialIndex()` (hidden items stop being hit-testable, not just stop drawing) and by
the atlas-lookup/rendering path, with a new per-port cache-key dimension so a toggle
correctly invalidates every affected tile. Verified by temporarily wiring both toggles into
`Main.qml`'s `Component.onCompleted` to exercise every changed code path for real before
reverting the QML hook.

**Deliberate simplifications from the original branch's later, more-evolved history**, applied
because this branch's own CanvasItem is much simpler at this point (no id/registry, no
pan/zoom, no `ThemeManager`-driven chrome, no `QuickAppController`) and building any of that
early would just be redone once its own real phase lands:
- `appearanceKeyFor()` was written with `QString::number()` from the start, not `.arg()` --
  the original branch profiled `.arg(qreal)/.arg(int)` as a real ~15-19% hotspot on a
  continuously-clocking circuit and fixed it in a separate later perf commit; recreating that
  mistake here just to fix it again later would be exactly the double work the plan's
  reordering exists to avoid. (Display7/14/16-classification memoization and the port-status
  dirty-flag tracking from that same later perf history were *not* front-loaded, though --
  those are real added infrastructure the plan deliberately bundles into its own later
  engine-perf-hardening phase, not a zero-risk pick between equally-simple options.)
- Hover-port tracking is a plain `Port*` (`m_hoveredPort`), not an id/registry-resolved
  element-id+port-index pair -- no delete gesture exists yet for the dangling-pointer race
  that scheme guards against.
- Chip/wire-selection screen positions are plain `scenePos()` -- no pan/zoom transform exists
  yet, so "screen" and "world" coincide 1:1.
- `connectedPeers()` is a small local free function, not a call into
  `ConnectionManager::connectedPeers()` (`WIDGETS_ONLY_SOURCES`).

## Phase 6 detail (DONE — core commands; IC-double-click fix deferred)

All 7 sub-steps from the plan's "Phase 3 in depth" breakdown, 8 commits:

1. **Id/registry layer**: `SceneItemRegistry` (already portable, unmodified) mounted on
   `CanvasItem`; `itemById()`/`nextId()`/`updateItemId()` public; `addItem()`/`removeItem()`
   typed overloads for `GraphicElement*`/`Connection*` (id-assignment/registry-membership
   only at this point — the m_elements/m_connections membership half was added in sub-step 4
   once Add/Delete needed it). `elementId()`/`wireId()` (SpatialIndex tagging) switched from a
   loop index to the real `ItemWithId::id()`. `deserializationContext()` deliberately deferred
   until Add/Delete became its first real consumer.
2. **keyPressEvent()/keyReleaseEvent()**: ported `Scene::keyPressEvent()`/`keyReleaseEvent()`'s
   auto-repeat guard and keyboard-triggered InputSwitch/InputButton dispatch verbatim; added
   `focus: true` to `Main.qml`'s `CanvasItem` so real interactive use actually routes key
   events to it.
3. **Move/Rotate/Flip/Align/Distribute**: new `App/QuickShell/Canvas/CanvasCommands.h/.cpp`
   with `CanvasElementsCommand`/`CanvasMoveCommand`/`CanvasRotateCommand`/`CanvasFlipCommand`,
   a real `QUndoStack` (`m_undoStack`) replacing what had been a documented "no undo entry
   yet" gap in Phase 5's drag gesture. `rotateRight()`/`rotateLeft()`/`flipHorizontally()`/
   `flipVertically()` and all 8 align/distribute methods; arrow-key nudge; Ctrl+R/Ctrl+Shift+R/
   Ctrl+H shortcuts matched directly in `keyPressEvent()` (no chrome QAction layer yet).
4. **Add/Delete/Update/ChangePortSize**: `CanvasCommandUtils` (namespace port of `Commands.h`'s
   `CommandUtils`) plus `CanvasAddItemsCommand`/`CanvasDeleteItemsCommand`/
   `CanvasUpdateCommand`/`CanvasChangePortSizeCommand`. Written directly against `ItemWithId*`
   from the start (not `QGraphicsItem*`) — unlike the original branch, which wrote this
   namespace before its own later domain-decoupling refactor and had to retrofit it via a
   dedicated ~20-file commit (`7fab41310`); that commit's diff was used here as the reference
   for the already-correct final shape. Fixed two real, latent bugs this surfaced:
   `ListSimulationHost` held a stale snapshot copy of `m_elements` instead of a live reference;
   `addItem()`/`removeItem()` needed to own `m_elements`/`m_connections` membership, not just
   id registration. Added `deleteSelected()` (Del key), `simulation()`/`restartSimulation()`
   accessors, and the deferred `deserializationContext()`.
5. **Clipboard**: `copyAction()`/`cutAction()`/`pasteAction()`/`duplicateAction()` +
   `mute()`/`selectAll()`, `CanvasCommandUtils::serializeItems()`, `deserializeAndAdd()`.
   Ctrl+A/C/X/V/D shortcuts. Not ported: `cloneDrag()` (Ctrl+drag ghost image — needs
   `QGraphicsScene::render()`, no fallback available) and blob-registry inclusion (needs
   IC embedding, sub-step 6).
6. **IC embedding**: `App/QuickShell/Canvas/CanvasICRegistry.h/.cpp`, scoped to the
   blob-storage core MCP's `embed_ic`/`instantiate_ic` handlers actually exercise (confirmed
   by reading `ICHandler.cpp`) — `hasBlob`/`blob`/`setBlob`/`registerBlob`/`removeBlob`/
   `renameBlob`/`findICsByBlobName`/`initEmbeddedIC`/`uniqueBlobName`/`createEmbeddedIC`.
   `CanvasRegisterBlobCommand`/`CanvasRemoveBlobCommand`/`CanvasRenameBlobCommand`/
   `CanvasUpdateBlobCommand`. `deserializationContext()`'s `blobRegistry` now points at the
   real blob map; `contextDir` stays empty — a real, confirmed gap (crashes on a nested
   relative-path file dependency round-tripping through this context), documented in the
   method's own doc comment, not papered over.
7. **Split/Morph/property-cycling**: `CanvasSplitCommand` (double-click wire-splitting, via
   new `mouseDoubleClickEvent()`), `CanvasMorphCommand` (in-place type swap with full
   port-connection migration), `CanvasToggleTruthTableOutputCommand` (logic only — its real
   trigger is chrome, a later phase). `nextElm()`/`prevElm()`/`next`/`prevMainPropShortcut()`/
   `next`/`prevSecndPropShortcut()`, ported from `PropertyShortcutHandler.cpp`, re-selecting by
   id rather than position (this canvas has no `itemAt()` equivalent anyway). "[" / "]" / "{" /
   "}" / "<" / ">" keyboard shortcuts matched directly.

**Verification discipline**: every command-family sub-step (3 onward) got, in addition to the
standing build/smoke/qmllint/full-ctest pass, a temporary self-test (`WP_QUICK_SELFTEST` env
guard, removed before commit) exercising real redo/undo/macro round-trips against the actual
command classes — not just crash-freedom. This caught one real self-test bug (holding a fixed
`m_elements` vector index across a morph, which reshuffles the vector — not a bug in the
ported code, exactly the "don't hold a raw index across a mutating command" property the
class's own doc comments warn about) but zero bugs in the ported production code itself.

**Deliberately not ported** (see the plan's "Phase 3 in depth" section and the feature-gap
tracking below): IC file-watching, `registerBlob()`'s recursive self-containment,
`renameBlob()`'s recursive nested-reference rewrite, and the IC-double-click-opens-tab fix
(needs Phase 7's tab/workspace concept and an inline-label editor, neither of which exists
yet).

## Phase 7 detail (DONE)

All 3 sub-steps from the plan's "Phase 4 sub-steps 1-3" scope, 7 commits:

1. **Message-dialog abstraction**: `App/QuickShell/Chrome/DialogProvider.h/.cpp` (abstract
   `DialogButton` enum + `DialogProvider` interface + `Dialogs::provider()/setProvider()`) and
   `QuickDialogProvider.h/.cpp` (backed by new `ChoiceDialog.qml`/`TextPromptDialog.qml`, plus a
   `dialogOpened(QObject*)` signal as a self-test hook). Verified via a deferred
   `QTimer::singleShot(0, ...)` self-test — calling `accept()` synchronously inside
   `dialogOpened()` ran before `execModal()`'s own `open()` call, so `close()`'s `!m_visible`
   guard silently no-op'd and hung the nested `QEventLoop` forever; fixed by deferring.
2. **File-dialog provider split**: `QuickDialogProvider` extended to also implement
   `FileDialogProvider` (backed by new `QuickFileDialog.qml`, using `QtQuick.Dialogs`'
   `QuickDialogs2` component, added to the root `CMakeLists.txt`). `App/UI/FileDialogProvider.h/.cpp`
   split so the abstract interface + registry storage moved to portable HEADERS/SOURCES in
   `CMakeSources.cmake` (new `FileDialogProviderRegistry.cpp`), while `RealFileDialogProvider`'s
   `QFileDialog`-based bodies stay WIDGETS_ONLY. `FileDialogs::provider()` now asserts a
   provider was explicitly registered instead of implicitly defaulting to
   `RealFileDialogProvider`.
3. **QuickWorkSpace**: new `App/QuickShell/Chrome/QuickWorkSpace.h/.cpp` (844 lines), the
   CanvasItem-side port of `WorkSpace` — `load()`/`save()`/`autosave()`/`loadFromBlob()`/
   `onChildICBlobSaved()`/`removeEmbeddedIC()`/`setCurrentFile()`, owning one `CanvasItem` via
   `std::unique_ptr`. A real file-dependency bug surfaced and got fixed here, not just in the
   self-test: `contextDir` (added to `CanvasItem` this sub-step) is what makes a nested
   relative-path IC dependency (e.g. `jkflipflop.panda` referencing `dflipflop.panda`) actually
   resolve — the very gap Phase 6's IC-embedding sub-step had left open and documented.
4. **QuickWorkspaceManager**: new `App/QuickShell/Chrome/QuickWorkspaceManager.h/.cpp`
   (694 lines), owning `std::vector<std::unique_ptr<QuickWorkSpace>> m_tabs` (not
   `QVector`/`QList` — Qt's containers require copy-constructible elements even for the
   never-hit COW detach path, which `unique_ptr` can't satisfy). `createNewTab()`/`closeTab()`/
   `save()`/`loadPandaFile()`/`openICInTab()`/`confirmSave()`/`hasModifiedFiles()`/
   `closeFiles()`.
5. **Real menu/toolbar/action shell**: `QuickAppController` (QML-facing root object,
   `QuickMainWindowHost` implementer) + `Main.qml`'s full File/Edit/Transform/Align/Distribute/
   Simulation/Export/Help `MenuBar`, `TabBar`, and canvas-hosting `Item`. Exposed as the
   `AppController` QML singleton via the `QML_FOREIGN`+`QML_SINGLETON` pattern
   (`AppControllerForeign`) rather than a `QQmlContext` context property — front-loaded from the
   original branch's own later fix for this exact pattern (`project_qml_context_property_root_cause_fixes.md`),
   since context properties are invisible to qmllint/the QML Language Server. Surfaced and fixed
   **3 real production bugs**, none guessed at — root-caused via GDB and, for the third, a
   from-scratch AddressSanitizer build (`cmake --preset asan`) after GDB alone wasn't
   conclusive:
   - `ThemeManager::instance()`'s GUI-thread constructor assertion firing from Qt Quick's
     render thread the first time a real (non-offscreen) scene got rich enough to trigger the
     threaded render loop — fixed via an early `ThemeManager::setTheme(ThemeManager::theme())`
     re-apply in `main()`, before the engine loads any QML (mirrors `App/Main.cpp`'s identical
     guard).
   - `static` `appController`/`dialogProvider` locals outliving `QGuiApplication` at real
     process-exit teardown (a function-local `static`'s destructor runs via `atexit`, after
     `main()`'s own stack locals — including `app` — are already destroyed) — fixed by making
     both plain automatic locals in the correct declaration order.
   - A real heap-use-after-free: `QuickWorkSpace` instances double-freed between QML's GC
     (`QV4::QObjectWrapper::destroyObject()`, since an unparented `QObject` first touched by
     QML defaults to `JavaScriptOwnership`) and `m_tabs`' own `unique_ptr` destructor — fixed
     via `QQmlEngine::setObjectOwnership(workspace.get(), QQmlEngine::CppOwnership)` in
     `createNewTab()`.
6. **Recent Files/Examples/geometry/close-confirm**: `AppController.recentFiles`
   (`App/IO/RecentFiles`, already portable), `examplesList()` (mirrors
   `MainWindow::setupExamplesMenu()`'s title-prettification exactly), `Settings::quickWindowGeometry()/
   setQuickWindowGeometry()` (new `QRect` pair in `Settings`), `confirmClose()` (mirrors
   `MainWindow::closeEvent()`'s modified-vs-clean branch).
7. **Play/pause and window-activate simulation sync**: `AppController.simulationRunning`
   (Play menu checkbox) and `backgroundSimulationEnabled` (Background Simulation menu
   checkbox), plus `handleWindowActiveChanged()` wired to `ApplicationWindow.onActiveChanged`.
   `bindCurrentTab()` now stops the previously-bound canvas's simulation and starts/stops the
   newly-bound one to match `simulationRunning` — symmetric in both directions (unlike a naive
   port of the original branch's fix), since `CanvasItem`'s constructor unconditionally
   auto-starts its `Simulation` (unlike `Scene`, which starts paused), so a freshly-created tab
   switched to while paused must be actively stopped, not just left alone. `QPointer<CanvasItem>
   m_boundCanvas` handles `closeTab()` erasing (and destroying) the previous tab before
   `currentTabChanged` fires for the new one.

**Verification discipline**: every sub-step got the standing build/smoke(offscreen+real
xcb)/qmllint/self-test(`WP_QUICK_SELFTEST`, removed before commit) pass. No full `ctest`
re-run this phase — none of the 7 commits touched any file the domain/CPU/Level Quick test
suite depends on (all changes confined to new `App/QuickShell/Chrome/*` files plus
`Main.qml`/`Main.cpp`/`Settings.{h,cpp}`/`CMakeSources.cmake`/`CMakeLists.txt`); confirmed via
`git diff --stat` against Phase 6's `HEAD` before starting.

**IC-double-click fix still not done** (see Feature-gap tracking below) — Phase 7 supplied the
tab/workspace half (`QuickWorkspaceManager::openICInTab()` exists and is exercised by
`QuickWorkSpace`'s own load path), but the inline-label-editor half doesn't exist yet
(original branch's Phase 4 sub-step 6, mapped to v2 Phase 9). Re-derive there, writing the
IC-vs-labelable-element branch in from the start rather than shipping the original branch's own
gap (which it never actually fixed — confirmed by reading its own audit doc to `HEAD`).

## Phase 8 detail (DONE)

All of Phase 4 sub-steps 4-5 from the plan, plus one foundational fix, 3 commits:

1. **Element palette** (`QuickElementPalette` + `ElementPalette.qml` + `PaletteItemDelegate.qml`):
   a QML-native data model backing the category grids/search/drag/double-click-to-add --
   unlike every earlier copy-and-adapt port in this plan, there's no widget class to adapt
   (QML's `GridView`/`Repeater` over a `QVariantList` replaces
   `ElementPalette::populateMenu()`/`updateICList()`'s widget-construction loops outright).
   `CanvasItem::addElementFromPalette()` mirrors `SceneDropHandler::addFromMimeData()`'s
   element-construction logic (built-in / file-based IC / embedded IC) without the
   `QMimeData` round-trip Widgets' native drag-and-drop needs. `App/UI/ElementPalette.cpp` is
   `WIDGETS_ONLY` (pulls in `QLayout`/`QScrollArea`/`MainWindowUI.h`), so its static
   `nameMatchesSearch()` is duplicated (a self-contained four-line regex match) rather than
   called, and only the one real, working search pass is reproduced -- the original's other
   two passes match `QObject::objectName()` values no code path ever actually sets, confirmed
   dead via a full grep and via `TestElementPalette.cpp`. Front-loads two fixes the original
   branch's own later qmllint sweep had to retrofit: the drag delegate is its own
   `PaletteItemDelegate.qml` component with typed `canvasWidth`/`canvasHeight` properties
   instead of an implicit ancestor-id reach-through, and `Main.qml`'s `DropArea` casts
   `drop.source` to `PaletteItemDelegate` (`as`) instead of reading `.modelData` off
   `DragEvent`'s plain `QObject`-typed source.
2. **Plain click-to-select + `selectionChanged`** (prerequisite fix, not itself in the
   original phase numbering but needed before the editor could exist): `CanvasItem` had no
   selection-change notification and, more seriously, a real, previously-unnoticed bug in
   this branch's own canvas -- a plain (non-Ctrl) click never selected anything at all, only
   Ctrl+click and rubber-band did (`mousePressEvent()` only implemented the Ctrl+click toggle
   branch; nothing built the "plain click selects, deselecting the rest" default `Widgets`
   gets for free from `QGraphicsScene`). Found and fixed while porting the original branch's
   own `36f05f854`, which had already found and fixed this same bug in its own chronology --
   confirmed via a temporary self-test using a small local subclass exposing the protected
   `mousePressEvent()`, feeding real `QMouseEvent`s.
3. **Element editor property panel** (`QuickElementEditor` + `ElementEditor.qml`): pulls
   `ElementEditor`'s property-read/apply logic into `Q_PROPERTY`s the QML panel binds to
   declaratively, with per-section visibility computed from `SelectionCapabilities` exactly as
   `applyCapabilitiesToUi()` does. Covers Label, Color, Frequency, Delay, Audio, Volume,
   Trigger, WirelessModeSelector, plus input/output port size, the locked checkbox, and a
   latched input's output value (which route through separate command paths, not
   `applyProperty()`'s switch). Simplifies production's fragile "does the widget's current
   text still equal a magic sentinel string" mixed-selection tracking into an explicit
   per-field dirty bool. Closes the "element-editor rebinding" deferral named back in Phase 7:
   `QuickAppController::bindCurrentTab()` now calls `QuickElementEditor::setCanvas()` on every
   tab switch. Not built in this pass, named rather than silently dropped: Appearance (file
   dialog + per-state tile grid), AudioBox (file dialog), TruthTable (the editor grid dialog),
   and embedded-IC blob rename -- each needs its own QML dialog surface.

**Verification discipline**: every sub-step got the standing build/smoke(offscreen+real
xcb)/qmllint/self-test(`WP_QUICK_SELFTEST`, removed before commit) pass, plus a full
114-target Quick CTest re-run for the two commits touching `CanvasItem.{h,cpp}`. The element
editor's own self-test tripped over `Led::updatePortsProperties()`'s real, deliberate
behavior (`hasColors()` is only true while `inputSize() == 1`) while testing setColor() after
setInputSize() in the wrong order -- confirmed as correct production behavior by reading the
source, not a bug, and fixed the self-test's ordering rather than the production code.

**IC-double-click fix still not done** (unchanged from Phase 7 -- see Feature-gap tracking
below): still needs the inline-label-editor half, scheduled for Phase 9.

## Phase 9 detail (DONE)

IC hover preview, misc widgets, theme/i18n, and export wiring from Phase 4 sub-steps 6-8
(the original branch's own numbering), plus one retroactive fix to Phase 8's classes, 5
commits:

1. **Retroactive fix**: `QuickElementPalette`/`QuickElementEditor`/`QuickAppController`'s
   Phase 8 `Q_PROPERTY`s were missing `FINAL` (the plan's own standing authoring rule), and
   `QuickAppController`'s constructor was missing the `QQmlEngine::setObjectOwnership(CppOwnership)`
   guard for `m_palette`/`m_elementEditor` (the same `JavaScriptOwnership`-on-first-touch hazard
   `QuickWorkSpace::m_canvas` needed it for). Both retroactively applied before starting new
   Phase 9 work.
2. **Context menu, inline label editing, IC hover preview** (one commit, since the
   `CanvasItem.h/.cpp` changes ended up interleaved): right-click context menu
   (`handleRightClick()`, `elementContextMenuRequested`/`emptyContextMenuRequested`,
   `QuickElementEditor::canMorph`/`morphCandidates`/`morphSelectionTo()`); inline label
   editing (`mouseDoubleClickEvent()` now checks `hasLabel()` and emits
   `inlineEditRequested`); IC hover preview (`renderICPreviewImage()` +
   `QuickICPreview`, a line-for-line port of `ICPreviewPopup`'s show/hide-delay state
   machine, encoding the rendered preview as a `data:` URL). **Front-loads the IC-double-click
   fix** tracked since Phase 6/7/8: IC is explicitly excluded from the `hasLabel()` inline-edit
   branch (confirmed by reading the original branch to its own HEAD that it never actually
   fixed this -- it stayed a permanently-broken, tracked gap the whole time). Opening the
   sub-circuit tab instead is still deferred (needs file-backed-vs-embedded blob resolution),
   so a double-click on an IC is a deliberate no-op for now, not either behavior. **Found and
   fixed a second real, latent bug**: `QuickElementEditor::m_canvas`/`QuickICPreview::m_canvas`
   were plain `CanvasItem*`, but `QuickWorkspaceManager::closeTab()` erases (and destroys) the
   current tab's `CanvasItem` *before* `currentTabChanged` fires for the new one -- confirmed
   as a real, reproducible SEGV (not theoretical) by reverting to a plain pointer and rebuilding
   under `cmake --preset asan`: closing the current tab crashed inside `QObject::disconnect()`
   dereferencing already-freed memory. Fixed with `QPointer<CanvasItem>` in both classes,
   reran clean under ASan.
3. **LabeledSlider + ICDropZone/TrashButton**: fraction-of-clock-period tick labels under the
   delay slider (`ElementEditor.qml`); two `DropArea`s on `ElementPalette.qml`'s IC tab
   (embed a file-based IC by dropping it on the embedded section, extract by dropping the
   reverse) plus a trash drop target, backed by new `QuickICController`
   (`embedICByFile`/`extractICByBlobName`/`removeICFile`/`removeEmbeddedIC`) and two new
   `CanvasICRegistry` methods (`embedICsByFile`/`extractToFile`) -- both already anticipated
   and named as deferred gaps in `CanvasICRegistry`'s own doc comment back when it was first
   written in Phase 6. `QuickMainWindowHost` gained `palette()`/`requestSave()`.
4. **Theme and language switching**: `QuickAppController.theme`/`languages`/`currentLanguage`/
   `switchLanguage()`, backed by a directly-owned `LanguageManager`. `LanguageManager.{cpp,h}`
   moved from `WIDGETS_ONLY_SOURCES`/`HEADERS` to the portable lists -- reading both files
   start to finish found zero actual Widgets/QApplication dependency (it was simply
   miscategorized, unlike the original branch's contemporaneous `LanguageManager`, which still
   had a real `Application::instance()` crash bug at this point in its own chronology).
   `ThemeManager.cpp`'s `qApp` usage was checked the same way and left as-is: empirically
   confirmed (via a throwaway compile with this file's exact include order) that `qApp`
   already resolves to `QGuiApplication*` here, since this file never transitively includes
   `<QApplication>` -- v2's earlier domain-decoupling refactor already avoided the coupling
   that caused the original branch's bug. **Found and fixed a real, empirically-confirmed
   gap**: `wiredpanda_quick` had zero embedded translation catalogs. The root cause differed
   from the original branch's own fix (which just added a second `qt_add_translations()`
   call): v2's `qt_add_translations(wiredpanda ...)` lives inside the root `CMakeLists.txt`'s
   `if(WIREDPANDA_BUILD_WIDGETS_APP)` block, which defaults OFF, but `wiredpanda_quick` builds
   unconditionally -- so a naive second call placed inside that same block would never run in
   the normal dev configuration. Fixed with a second, ungated block (guarded only on
   `Qt6LinguistTools_FOUND`) right after `add_subdirectory(App/QuickShell)`.
5. **CircuitExporter port**: `CanvasItem::elementsBoundingRect()`/`renderExportImage()`/
   `exportToImage()`/`exportToPdf()`/`clearSelection()`, generalizing
   `renderICPreviewImage()`'s offscreen-QPainter-plus-real-paint() technique from a single
   IC's `internalElements()` to the whole canvas's element+connection set.
   `QuickExportController` gained `exportPdfDialog()`/`exportImageDialog()`.
   `wiredpanda_quick` now links `Qt6::PrintSupport` for `QPrinter`/`QPageLayout`/`QPageSize`
   (confirmed PrintSupport itself has no QtWidgets coupling for this API surface).

**Verification discipline**: every sub-step got the standing build/smoke(offscreen+real
xcb)/qmllint/self-test(`WP_QUICK_SELFTEST`, removed before commit) pass, plus a full
114-target Quick CTest re-run for every commit touching `CanvasItem.{h,cpp}` or shared CMake
wiring. The context-menu/inline-edit/IC-preview self-test additionally verified the QPointer
fix by closing the tab currently bound to `elementEditor`/`icPreview` (no crash, selection
cleared) and, separately, by reverting the fix and reproducing the SEGV under ASan before
restoring it. The theme/language self-test verified the CMake fix directly via
`QResource(":/i18n/wpanda_pt_BR.qm").isValid()`, not just via `languages()`'s count, so a
regression there fails loudly. The CircuitExporter self-test ran the full export flow against
a real bundled example (`Examples/dflipflop.panda`, 25 elements/33 connections): real
non-empty bounding rect, a non-null image with real painted pixels, a reloadable PNG, and a
PDF with a valid `%PDF-` header.

**IC-double-click fix still not done** (unchanged from Phase 7/8 -- see Feature-gap tracking
below): the tab/workspace half has existed since Phase 7, and the "IC excluded from inline
editing" half landed in this phase's first commit, but the actual "open the sub-circuit tab"
behavior is still deferred, now for the same reason it always was: needs file-backed-vs-
embedded blob resolution and `QuickWorkspaceManager::openICInTab()` wiring, real, separately-
scoped work.

## Phase 10 detail (DONE)

Pan/zoom + minimap from Phase 4 sub-step 7 (the original branch's own numbering), 2 commits:

1. **Pan/zoom** (`CanvasItem`): introduces an explicit world/screen coordinate split
   (`screenToWorld()`/`worldToScreen()`) mirroring `GraphicsView`'s discrete zoom ladder
   (`1.25`/`0.8` reciprocal steps, `-9`..`7` level range) and pan gestures, since `CanvasItem`
   has no `QGraphicsView` underneath it to do this implicitly. Every element/port/wire position
   stays in WORLD coordinates, unaffected by pan/zoom, exactly as before; every mouse/hover
   event handler converts the incoming screen position via `screenToWorld()` before any spatial
   reasoning (hit-testing via `m_index`, drag deltas, wire routing). `updatePaintNode()` wraps
   its existing (already 5-node, by this point in v2's own history: wire-halo/wire/hover/
   gate/overlay) geometry in one new `QSGTransformNode`, so none of the vertex-building code
   itself needed to change. Signals that position QML overlays
   (`elementContextMenuRequested`/`emptyContextMenuRequested`/`inlineEditRequested`) keep
   emitting SCREEN coordinates via `worldToScreen()` at the point of emission, since Main.qml
   consumes them as canvasHost-local pixels. Adds wheel-zoom (`wheelEvent()`, anchored on the
   cursor), middle-drag/space-drag pan, keyboard shortcuts (`Ctrl+=`/`Ctrl+-`/`Ctrl+0`/
   `Ctrl+Shift+F`), and a `View > Zoom` menu (Zoom In/Out/Reset/Fit).
2. **Minimap** (`QuickMinimap` + `Minimap.qml`): Widgets-free port of `MinimapWidget` — a
   thumbnail image, viewport-rect overlay, content aspect ratio, and click/drag-to-navigate
   presenter (`QuickMinimap`), plus `Minimap.qml`'s own background/border chrome, click/
   drag-to-navigate, a move strip, and 8 aspect-locked resize handles — QML-native `MouseArea`
   drag math (mirroring `MinimapWidget::applyResize()`'s dominant-axis/anchor-preserving math
   exactly, just in QML/JS) rather than a port of `MinimapWidget`'s C++ `ResizeMode` machinery,
   since this is interactive UI chrome, not domain logic. `CanvasItem` gains `centerOn()`/
   `visibleWorldRect()`/`minimapContentRect()`/`renderMinimapImage()`, reusing the same
   offscreen-`QPainter`-plus-real-`paint()` technique `renderICPreviewImage()`/
   `renderExportImage()` already established. Content regeneration is throttled 200ms on
   *both* content changes (`undoStack()->indexChanged`) and view changes (`zoomChanged()`) —
   a deliberate divergence from `MinimapWidget`, which only throttles content changes and
   re-renders unthrottled on every view-changing paint — justified because
   `renderMinimapImage()` is a real per-element CPU paint, not `QGraphicsScene::render()`'s
   cheap downscale of an already-built scene graph; eager re-render on every pan/zoom step
   would regress. Visibility/geometry are one shared global instance
   (`AppController.minimap`), not per-tab — a legitimate simplification, since production's
   own per-tab `MinimapWidget` instances already all end up showing the same
   `Settings`-persisted visibility/geometry in practice. `QuickMinimap` has no `QQuickItem` of
   its own — `Minimap.qml` reports its actual on-screen `minimapWidth`/`minimapHeight` to it
   (mirrors `ElementPalette.qml`'s `canvasWidth`/`canvasHeight`-reported-by-`Main.qml`
   precedent). Adds a `View > Show Minimap` toggle to `Main.qml`.

**Verification discipline**: both commits got the standing build/smoke(offscreen+real
xcb)/qmllint/self-test(`WP_QUICK_SELFTEST`, removed before commit)/full-114-target-CTest pass.
The pan/zoom self-test verified `screenToWorld()`/`worldToScreen()` are true inverses; `zoomIn()`/
`zoomOut()`/`resetZoom()` step/undo by the exact `1.25`/`0.8` reciprocal factors with no residual
pan; a real `wheelEvent()` (not just calling `zoomIn()`/`zoomOut()` directly) dispatches on
`angleDelta().y()`'s sign; a simulated middle-drag pan shifted the pan offset by exactly the
expected screen-delta-over-scale; `zoomToFit()` against a real (non-demo — this tab's canvas
starts genuinely empty, so two real elements were added via `addElementFromPalette()` first)
circuit brought its whole bounding rect inside the viewport; and hit-testing (`mousePressEvent()`
→ `screenToWorld()` → `m_index`) still correctly selects a real element after zooming. The
minimap self-test verified `centerOn()` maps its world point to the item's exact screen center;
`visibleWorldRect()` tracks live pan/zoom; `minimapContentRect()` is grown to exactly the
requested target aspect ratio and contains both the real content and the current viewport;
`renderMinimapImage()` produces a correctly-sized, non-blank (real opaque pixels, not an
all-transparent canvas) thumbnail; `QuickMinimap`'s throttled regen actually fires (spun a short
local `QEventLoop` past the 200ms single-shot timer) and produces a valid `data:image/png;base64,`
URL plus a non-empty viewport rect; `navigateTo()` centers the viewport on the world point under
the given minimap-local pixel (captured against the content rect from *before* the navigate,
since `centerOn()` itself shifts `visibleWorldRect()` and therefore `minimapContentRect()`'s own
union term — a subtlety the self-test's first draft got wrong and had to fix); and
`setVisible()` round-trips through `Settings::minimapVisible()`.

## Phase 11 detail (DONE)

Startup polish + CLI + perf pass 1 (the post-chrome perf pass + Phase 4 deferrals + CLI, per the
plan's own reordering), 7 commits (6 landed from the original branch's own sequence, plus 1
bugfix this phase's own testing found):

1. **Startup debug spam + canvas clipping** (`fix(quick)`): `Comment::setVerbosity(-1)` was
   never called in `main()`, so Qt's `qCDebug` categories spammed element-registration/
   simulation-lifecycle noise on every tab open (confirmed: thousands of lines dropped to 2 in a
   normal offscreen run). `CanvasItem` also never called `setClip(true)`, so panned/dragged
   content painted over the SplitView's left pane instead of stopping at its own bounds. Applied
   directly (not as a historical-shaped port) since the plan folds this into `main()`'s setup
   and the canvas-foundation commit, both long since landed in this recreation.
2. **The seven named Phase 4 deferrals** (`feat(quick)`): embedded-IC blob rename (a plain
   `TextField`), `AudioBox` file picking (non-undoable, matching production), Appearance (file
   picker + a multi-state tile grid via base64 preview images — `TruthTableRow`/
   `AppearanceStateOption` added as real `Q_GADGET`/`QML_VALUE_TYPE` classes, matching the
   plan's standing authoring rule, even though this phase's own `colorOptions()`/
   `morphCandidates()` predate that rule and still use `QVariantMap` — a known, not-yet-fixed
   pre-existing gap, noted here rather than silently carried forward), a real TruthTable editor
   dialog (`TruthTableDialog.qml`), mute state now going through `Simulation::setUserMuted()`
   with per-tab resync, and a status bar message strip. **Found and fixed one real bug**: Ctrl+
   click and Shift+click were swapped relative to production (`Scene::eventFilter()` intercepts
   Ctrl+Left-click unconditionally for clone-drag before `SceneInteraction::mousePress()` ever
   runs, so production's real multi-select keybinding is Shift+click) — fixed by swapping the
   keybinding and adding a real Ctrl+drag clone-and-move gesture (duplicate the selection in
   place, drive the existing drag machinery directly). The original branch's own third bug
   (element editor rendering at zero height) does not apply here — v2's chrome is still a plain
   `RowLayout` of `ElementPalette`/`canvasHost`/`ElementEditor` (already has `Layout.fillHeight`),
   not yet the `SplitView`-with-stacked-leftPane restructuring that caused that regression
   originally; that restructuring is Phase 15 scope in this recreation.
3. **CLI file-loading** (`feat(quick)`): a scoped-down `QCommandLineParser` positional-argument
   handler mirroring `App/Main.cpp`'s, resolving to an absolute path and loading through
   `QuickAppController::openRecentFile()` — `wiredpanda_quick` had zero command-line
   file-loading before this, a real permanent capability gap, not just profiling scaffolding.
   `Scripts/profile_load.sh` gains a `BINARY` environment-variable override.
4. **Memoize per-frame render state** (`perf(quick)`): `updatePaintNode()` rebuilt every
   element's texture-atlas cache key and every port's `scenePos()` from scratch on every single
   frame. Adds `m_elementRenderCache` (per-element bounding rect + atlas tile, keyed on a
   fingerprint of exactly what `appearanceKeyFor()`/the gate loop's own `localRect` already
   read) and `m_portScenePosCache` (per-port position, cleared by `rebuildSpatialIndex()`). v2's
   `appearanceKeyFor()` already carried its *complete* final key shape (per-port status/hidden,
   label text, `.arg()`→`QString::number()`) from Phase 5's front-loaded rendering-completeness
   work — dimensions the original branch's own history only accumulated piecemeal across several
   later commits (one of which had to go back and extend this exact key a second time) — so this
   memoization's fingerprint only ever needed writing once, against the complete key, unlike the
   original's own two-pass history.
5. **Stop the blind repaint poll and per-drag full spatial-index rebuild** (`perf(quick)`):
   replaced the unconditional 16ms `m_refreshTimer` poll with a real `Simulation::
   visualStateChanged` signal (emitted only on a genuine throttled visual flush); replaced
   per-drag-sample `rebuildSpatialIndex()` (every element/port/wire) with `updateSpatialIndexFor()`
   (only the dragged element's own port/wire fan-out, via `Port::connections()`), requiring
   stable per-`Port*` spatial ids (`spatialIdFor()`) instead of positional ones. Also fixes a
   latent gap the old timer was masking: keyboard-triggered elements never called `update()` and
   would have gone invisible while the simulation is paused. **Found and fixed one real bug**:
   the original branch's own `updateSpatialIndexFor()` used `boundingRect().translated(pos())`
   for an element's world box, while `rebuildSpatialIndex()` uses `sceneBoundingRect()` (which
   additionally applies rotation/flip) — a real, never-fixed inconsistency there that would leave
   a rotated element's hit-test box wrong after a drag until some other event forced a full
   rebuild. This port uses `sceneBoundingRect()` in both places from the start.
6. **Reuse `QSGGeometry` buffers** (`perf(quick)`): `updatePaintNode()`'s five geometry nodes
   each allocated a fresh `QSGGeometry` every repaint even when the vertex count hadn't changed;
   `geometryFor()` now reuses the existing buffer in place via `QSGGeometry::allocate()`. The
   original branch's companion fix (skip `Port`/`Connection` pen-and-brush work when `scene()`
   is null) does **not** apply to this recreation and was **not** ported: `Connection`/`Port`
   stopped being `QGraphicsItem` subclasses entirely back in Phase 2's domain-decoupling
   refactor (confirmed via both classes' own doc comments — no `scene()` concept remains to gate
   on), and `CanvasItem`'s wire loop already reads `startPort()->status()`/`colorForStatus()`
   directly, never touching `Connection::statusPen()` at all — the waste this targeted doesn't
   exist here in the same shape.
7. **`QuickMinimap::m_canvas` QPointer fix** (`fix(quick)`, found while testing #6 above, but a
   Phase 10 regression): a plain `CanvasItem*` (not `QPointer`) left `QuickMinimap::setCanvas()`
   vulnerable to the exact same dangling-pointer pattern `QuickElementEditor`/`QuickICPreview`
   were already fixed for in Phase 9 — `QuickWorkspaceManager::closeTab()` destroys the current
   tab's `CanvasItem` before `currentTabChanged` fires. Confirmed as a real, reproducible SEGV
   (not theoretical) via a `WP_QUICK_SELFTEST` `closeTab()` sequence and a GDB backtrace landing
   exactly in `QObject::disconnect()` from `QuickMinimap::setCanvas()`. Missed in Phase 10 since
   `QuickMinimap` was written after the QPointer fix's own phase; fixed with `QPointer<CanvasItem>`,
   same as the other two.

**Verification discipline**: every commit got the standing build/smoke(offscreen+real
xcb)/qmllint(where QML changed)/self-test(`WP_QUICK_SELFTEST`, removed before commit)/
full-114-target-CTest pass. Commit 2's self-test covered plain-click/Shift-click/Ctrl-drag-clone
selection behavior (including real undo entries for the clone-then-move), mute round-tripping
through `Simulation::isUserMuted()` with per-tab resync on tab switch, status-message auto-clear,
and AudioBox/Appearance/TruthTable/blob-rename wiring against real domain objects (a bundled
`:/Components/Output/Audio/wiredpanda.wav` resource path for the AudioBox case, since a fake
path throws inside `setAudio()`'s real file-exists check). Commit 3's self-test loaded a real
bundled example via the CLI path and confirmed `currentFile()`/element count matched. Commit 4's
self-test used the `DiagCanvasItem::updatePaintNode()` technique (project memory
`project_inprocess_screenshot_technique.md`'s no-display-needed fallback) to prove a real
element's cached tile UV rect stays identical across an unchanged repaint, then genuinely
changes on rotate and again on deselect, and restabilizes afterward. Commit 5's self-test drove
real synthesized Qt events against a real two-element, one-wire circuit with no full
`rebuildSpatialIndex()` call after the initial one, proving a plain element, a rotated element,
and the wire between two elements all hit-test/render correctly at their post-drag position (not
the pre-drag one), and that a keyboard trigger while the simulation is stopped still flips state
and reaches a subsequent repaint. Commit 6's self-test drove a real add/hover/rubber-band(grow+
shrink)/delete sequence, confirming the gate/hover/overlay nodes' `QSGGeometry` objects keep
their identity across every transition (reused, never replaced) while still resizing to the
exactly correct vertex count with no stale leftover data.

## Phase 12a detail (DONE)

Exercise/Tour overlays + MCP server hosting, ported from the original branch's `40635c751`, 1
commit (landed as one atomic unit, matching that source commit's own scope: three peripheral
systems whose real coupling only shows up in each other's CMake wiring, not worth artificially
splitting).

1. **Exercise overlay** (`ExerciseOverlay.qml`, `QuickExerciseController`): ports `ExerciseEngine`'s
   Scene-coupled `setScene()` to a Quick-side `setCanvas()` via the type-erased `std::function`
   seam already declared on `ExerciseEngine.h` (`m_connectFn`/`m_disconnectFn`/`m_elementsFn`),
   with `setCanvas()`'s body living in its own Quick-only TU (`QuickExerciseEngineBinding.cpp`) so
   `ExerciseEngine.cpp` itself never needs `CanvasItem.h`. Verified end to end via a
   `WP_QUICK_SELFTEST` run driving the real bundled `basic-and-gate` exercise through all 5 steps
   by adding real elements/connections through `CanvasItem`'s command API and confirming
   `stepCounterText()` auto-advances after each one, reaches the final observe step with `Next`
   enabled, and completes correctly on `advanceStep()`.
2. **Tour overlay** (`TourOverlay.qml`, `QuickTourController`): `TourEngine` has zero Scene
   coupling at all (confirmed by reading it in full), so this owns its `TourEngine` directly, no
   `setCanvas()` split needed. Target resolution (`resolveTarget()`) and click dispatch
   (`dispatchClick()`) live in `TourOverlay.qml` against typed `required property` bindings
   (`paletteItem`/`editorItem`/`canvasAreaItem`, wired from `Main.qml`), not reached for through
   the `Window` attached property — qmllint can't see custom QML functions through its generic
   `QQuickWindow` static type. Verified via self-test: start the bundled `ui-overview` tour,
   read `currentTarget()` without crashing, advance twice, close.
3. **MCP server hosting** (`QuickBaseHandler` + `QuickConnectionHandler`/`QuickElementHandler`/
   `QuickFileHandler`/`QuickHistoryHandler`/`QuickICHandler`/`QuickSimulationHandler` +
   `QuickMCPProcessor`, `--mcp`/`--mcp-gui` CLI flags in `Main.cpp`). `QuickBaseHandler` inherits
   `BaseHandler` directly rather than copying it — only `validateElementId()`/`validatedElement()`
   needed reimplementing against `currentCanvas()` instead of `currentScene()`, since those are
   the only two `BaseHandler` methods that touch `Scene`/`MainWindow` at all (confirmed by
   reading `BaseHandler.cpp` in full). `ServerInfoHandler`/`ThemeHandler` need no Scene/MainWindow
   access and are reused completely as-is. `create_waveform`/`export_waveform` return a clean
   "not yet available" error (BeWavedDolphin isn't ported until Phase 12b), matching
   `QuickExportController`'s identical waveform-export deferral.

**Real architectural gap found and fixed, not present in the original branch's own history**:
v2's `CMakeSources.cmake` already splits a `WIDGETS_ONLY_SOURCES`/`WIDGETS_ONLY_HEADERS` pair
from the shared, Widgets-free `SOURCES`/`HEADERS` lists — a split that didn't exist yet when
`40635c751` was authored (confirmed by diffing that commit's own parent's `CMakeSources.cmake`:
everything MCP-related was still in one shared list back then, since `wiredpanda_quick` and
`wiredpanda` both linked the same not-yet-decoupled library). In v2 today, `BaseHandler.cpp`,
`MCPProcessor.cpp`, and `ExerciseEngine.cpp` had each been filed as Widgets-only (or, for
`ExerciseEngine.cpp`, kept whole in `WIDGETS_ONLY_SOURCES` even though only one of its methods
needed `Scene.h`) — mechanically porting `QuickBaseHandler`/`QuickMCPProcessor`/
`QuickExerciseController` to link against them as originally written would have forced
`wiredpanda_quick` (zero `Qt6::Widgets` linkage by design, confirmed via
`target_link_libraries`) to either newly link Widgets — undoing the whole point of this
rewrite — or silently fail to link. Root-caused (not guessed) via the actual `mold` "undefined
symbol"/GCC compile errors from a real build attempt, then fixed by splitting each of the three
files into a portable half (kept under the original name, moved to shared `SOURCES`) and a new
Scene/MainWindow-dependent half (`BaseHandlerScene.cpp`, `MCPStdinReader.cpp`,
`ExerciseEngineScene.cpp`, added to `WIDGETS_ONLY_SOURCES`) — pure code motion, zero behavior
change on either side. Verified both ways: `wiredpanda_quick` links clean with no Widgets
dependency, and a separate one-off `-DWIREDPANDA_BUILD_WIDGETS_APP=ON` configure+build confirmed
the `wiredpanda` target's own remaining compile errors are 100% pre-existing `QGraphicsItem`-era
signature mismatches from Phase 2's domain-decoupling refactor (`AddItemsCommand`/
`DeleteItemsCommand` still taking `QList<QGraphicsItem*>`, `qgraphicsitem_cast<Connection*>`),
already known-broken and explicitly accepted per the plan's own Setup section — unrelated to,
and not made any worse by, this split.

**Two real bugs found and fixed via a live MCP round-trip** (a real Python client script driving
`wiredpanda_quick --mcp` over actual stdin/stdout — element creation/wiring, `list_elements`/
`list_connections`, `simulation_control`, `set_input_value`/`get_output_value`, `undo`/`redo`,
`get_theme`, `get_tab_count`, `close_circuit`, then closing stdin):

- `get_output_value`'s response omitted `status`, which the *current* `schema-mcp.json`
  (evolved since `40635c751` was authored) requires alongside `value`
  (`get_output_value_response`'s `required: ["value", "status"]`, `additionalProperties: false`)
  — every call failed schema validation. Fixed by adding the same four-state `statusName()`
  mapping (`"low"`/`"high"`/`"unknown"`/`"error"`) the current Widgets-side `ElementHandler.cpp`
  already uses for its own two non-input branches, duplicated locally (not shared — `ElementHandler.cpp`
  is Widgets-only) — and also applied to the `GraphicElementInput` branch, which even the
  *current* Widgets-side handler still doesn't set (a latent, undiscovered bug there too, since
  nothing exercises a live MCP round-trip against an input element in that suite either; left
  as-is on the Widgets side, out of scope here).
- `confirmClose()` had no `Application::interactiveMode` gate: `QCoreApplication::quit()` (fired
  on stdin EOF, from `QuickMCPProcessor::onStdinReadable()`) closes every top-level window,
  reaching `Main.qml`'s `onClosing` → `confirmClose()` → a real modal confirmation dialog with no
  user present to answer it — reproduced directly (process hung past a 10s timeout before the
  fix). Fixed with the same `Application::interactiveMode` early-return `App/Main.cpp` already
  uses for this exact class of problem. The `close_circuit` segfault `40635c751` also fixed is
  already moot in v2: `QuickMinimap`/`QuickElementEditor`/`QuickICPreview` were already converted
  to `QPointer<CanvasItem>` in earlier phases (9, 10) for the identical dangling-pointer reason,
  confirmed by reading their current headers before assuming the fix was still needed.

**Incidental fix, caught while touching the same menu-population code**: `examplesList()`
converted from `QVariantList`-of-`QVariantMap` to a real `ExampleEntry` `Q_GADGET`/
`QML_VALUE_TYPE`, matching the new `LearnEntry` type added alongside it for
`exercisesList()`/`toursList()` — closes a pre-existing gap against this project's own
standing "no `QVariantMap`" authoring rule (Phase 7 predates that rule being consistently
applied here; `Main.qml`'s `Examples` submenu now binds `required property exampleEntry
modelData` instead of an untyped `var`).

**Verification discipline**: build clean (`wiredpanda_quick` target); qmllint clean (0
warnings); full 114-target Quick CTest suite green (2267 individual test functions, 0 failed)
both before and after the self-test scaffolding was added and removed; a real MCP client
round-trip under both `QT_QPA_PLATFORM=offscreen` and real `DISPLAY=:0 QT_QPA_PLATFORM=xcb`; the
Exercise/Tour `WP_QUICK_SELFTEST` block removed before commit (confirmed via `git diff --stat
App/QuickShell/Main.cpp` showing only the intended `--mcp`/`--mcp-gui` flag additions).

## Phase 12b detail (DONE)

BeWavedDolphin waveform editor port, ported from the original branch's `dc94754ee`, 1 commit:
table shell (`SignalTable.qml` + `QuickDolphinController`), window shell (`DolphinWindow.qml` +
`RangeDialog.qml`), and integration (Waveform menu action, Tour `bwd:`/`actionWaveform`
targets, MCP `create_waveform`/`export_waveform`).

1. **`WaveformSimulator`/`DolphinModelBuilder::collect()` decoupled from `Scene*`**: both now
   take a plain `QVector<GraphicElement *>` (`Scene::elements()`/`CanvasItem::elements()` both
   already produce it) -- the same generalization the original commit made, but adapted to v2's
   own already-more-evolved `DolphinModelBuilder::Row`-based `sweep()` signature (a
   `(element, port, kind, label)` descriptor list this branch already had before this phase,
   which the original diff's `(inputs, outputs, inputPorts)` tuple shape didn't need to account
   for). `BeWavedDolphin.cpp`'s three call sites and `Tests/Unit/Serialization/
   TestDolphinModelBuilder.cpp`'s five `collect(scene)` calls updated to `collect(scene->
   elements())` accordingly -- a mechanical, behavior-preserving adaptation (`collect()`'s body
   only ever called `scene->elements()` once, at its very top).
2. **`QuickDolphinController`/`QuickDolphinExporter`/`QuickDolphinZoom`** (new, Widgets-free):
   mirror `BewavedDolphin`'s data/simulation half (`SignalModel`/`WaveformSimulator` reused
   as-is) and its window-action layer (undo/clipboard/edit commands, file I/O, PNG/PDF export).
   `QuickDolphinExporter` reimplements `DolphinExporter`'s pixmap rendering via direct
   `QPainter` calls instead of a throwaway `QTableView` -- the same `QApplication`-needs-Widgets
   substitution `CanvasItem`'s own export renderers already established for
   `CircuitExporter`/`ICRenderer`.

**Real architectural gap found and fixed, continuing Phase 12a's pattern**: `SignalModel`,
`DolphinClipboard`, `DolphinCommands`, `DolphinEdits`, `DolphinFile`, and `Serializer` were all
filed under `WIDGETS_ONLY_SOURCES` despite having zero actual Qt Widgets dependency (confirmed
by reading every `#include` in each -- `DolphinClipboard.cpp`'s one real Widgets include,
`<QApplication>`, was itself just an unnecessary alias for `QGuiApplication::clipboard()` and
switched over). All six moved to the shared, Widgets-free `SOURCES`/`HEADERS` lists -- pure
historical grouping, not real coupling, same root cause as Phase 12a's `BaseHandler`/
`MCPProcessor`/`ExerciseEngine` split. `SignalDelegate.h` needed a genuinely new split, not just
reclassification: it bundled a real `QItemDelegate`-derived class together with the
`PlotType`/`WaveSegment` enums and `segmentFor()`'s pure logic in one header, so no
reclassification alone could make the vocabulary reachable without pulling in Widgets. Extracted
the enums/logic into a new `WaveSegment.h`/`.cpp` (shared); `SignalDelegate::segmentFor()` now
forwards to the free function, so no existing Widgets-side call site (`DolphinExporter.cpp`,
`TestSignalDelegate`) needed to change. `DolphinExporter.cpp`'s Widgets-free
`writeTruthTableText()`/`csvText()` (needed by `QuickDolphinController::saveToTxt()`) were
similarly split into a new `DolphinExporterText.cpp` (shared), leaving the genuinely
`QTableView`/`QHeaderView`-based `renderToPixmap()`/`exportToPng()`/`exportToPdf()` in the
original, Widgets-only file (its header, `DolphinExporter.h`, moved to shared HEADERS too, now
that it only includes the new `WaveSegment.h`). All splits are pure code motion -- zero behavior
change on the Widgets side, confirmed via a clean build throughout.

**Real, empirically-confirmed qmllint gap found and fixed**: `QuickDolphinController::model()`'s
`Q_PROPERTY` needed declaring as `QObject*` rather than `SignalModel*` -- `SignalModel` has no
QML registration at all (it's part of `wiredpanda_lib`, and `qt_add_qml_module`'s type scanner
only walks `wiredpanda_quick`'s own listed sources, confirmed directly by inspecting the
generated `.qmltypes`, not assumed), and qmllint's static Q_PROPERTY-type resolution warned
(`unresolved-type`) on it even though nothing in QML ever reads a member off the property
(`SignalTable.qml` only ever assigns it straight into `TableView.model`, which accepts any
`QAbstractItemModel`-derived `QObject` opaquely at runtime). Fixed by declaring the property as
`QObject*` (the implicit `SignalModel*`->`QObject*` upcast in the `READ` call is what moc's
generated code relies on) rather than leaving a permanently-accepted warning -- this branch's
own standing discipline has been 0 qmllint warnings after every commit since Phase 5.

**Known, deliberately accepted gap, documented not silently dropped**: this chrome has no
toolbar yet (a separate, already-tracked gap predating this phase), so the Tour's
`"toolbar"`/`waveformToolButton` spotlight target stays unresolvable, and every `bwd:`-prefixed
spotlight target stays unresolvable too -- `DolphinWindow` is a genuinely separate top-level
`QQuickWindow`, and Qt Quick has no cross-window item-to-item coordinate mapping the way
Widgets' `mapToGlobal()`-based overlay-reparenting trick relied on. A real fix would need a
second `TourOverlay` instance living inside `DolphinWindow.qml` itself -- both gaps documented
directly in `TourOverlay.qml`'s `resolveTarget()` rather than silently dropped. Click dispatch
(`actionWaveform`/`bwd:actionCombinational`) has no such constraint and is real now, since both
just call through `AppController`/`AppController.dolphin` regardless of which window is
currently focused.

**Verification discipline**: build clean; qmllint clean (0 warnings); full 114-target/
2267-function Quick CTest suite green both before and after the self-test scaffolding; a
`WP_QUICK_SELFTEST` run exercising the full waveform lifecycle (build a 2-switch/And/Led
circuit -> `createWaveform()` -> `setCellValue()` -> `snapshot()` -> `applyClockWave()`'s
undo/redo -> zoom -> `setLength()` -> `openWaveform()`/`notifyWaveformClosed()`) under both
offscreen and real xcb; a real MCP client round-trip (`create_waveform` with explicit
`input_patterns`, verifying the AND-gate truth table came back correct in the response;
`export_waveform` to both `.txt` and `.png`, confirming non-empty files). Full
`ctest --preset debug` run at the end of Phase 12 (both 12a and 12b), per the plan's own gate
list for this phase.

## Phase 13 detail (DONE)

Scoping this phase surfaced that the plan's own "already covered" notes for two Phase 7
sub-steps were wrong for v2's actual state: neither `TestCanvasItemSmoke` (7a) nor
`TestCanvasCommands` (7b) had ever been ported, despite several already-landed test classes'
doc comments assuming `TestCanvasCommands` existed. Both were true gaps, not scoping notes —
confirmed by `find`/`grep` turning up zero matches for either class anywhere in the tree before
this phase started. Filled in first, in dependency order, before continuing to the sub-steps
the plan did anticipate. All 14 of the original branch's real Phase 7 sub-commits are now
ported; only `TestArduino`/`TestICInline` (never wired into the original branch's own Quick
target either — confirmed via `git show qtquick-rewrite:CMakeSources.cmake`) remain, and need
original test-by-test authoring rather than porting.

**Commits landed, in order**:
- **7a + 7b** (`bc9f1734b`'s predecessor batch, one combined commit): `TestCanvasItemSmoke` (3
  tests, proves `CanvasItem`/`QuickAppController`/the Quick MCP handlers link into
  `test_wiredpanda_quick` a second time) + `TestCanvasCommands` (16 tests, add/delete/move/
  rotate/flip/update/change-port-size/morph/split, each with undo/redo). Found and fixed a real,
  pre-existing bug: `CanvasAddItemsCommand`'s constructor captured every item's id — including
  auto-discovered wires — via `loadList()` *before* those wires had been through `addItem()`, so
  a freshly wired `Connection`'s id was captured as `-1`; `undo()` could never resolve that back
  via `findItems()`, leaving the connection a permanent zombie once its endpoints were deleted
  around it — a real, reproducible crash on the next `rebuildSpatialIndex()` call. Fixed by
  re-deriving `m_ids`/`m_otherIds` via `storeIds()`/`storeOtherIds()` after every item (wires
  included) has its real assigned id. Also fixed two unrelated unity-build self-containment
  gaps the new files' batch-boundary shift exposed (`TestDisplays.cpp` used `Simulation`
  without including its header; `TestPriorities.cpp` used `OutputPort::connections()` without
  including `Port.h`).
- **7c** — MCP handler parity: `TestQuickElementHandler`/`TestQuickFileHandlerSecurity`/
  `TestQuickICHandlerSecurity` (Clock-delay range validation, PNG export dimension/padding
  clamps, all four IC path-traversal/confinement guards). Required real CMake surgery:
  `test_wiredpanda_quick` (stood up in Phase 3, before `CanvasItem` existed) only ever linked
  `wiredpanda_lib` — none of the Quick-shell classes, compiled directly into the `wiredpanda_quick`
  executable via `qt_add_qml_module` rather than a shared library, were reachable from it.
  Extracted that source list into `App/QuickShell/CMakeLists.txt`'s `QUICK_SHELL_SOURCES`
  variable, consumed by both `wiredpanda_quick`'s `qt_add_qml_module()` and a new
  `target_sources()`/`target_include_directories()`/`target_link_libraries()` trio wiring the
  same sources into `test_wiredpanda_quick`.
- **7d** — `QuickDolphinController` parity: 40 tests, all passed on the first attempt (a useful
  negative result confirming Phase 12b's port held up under permanent test pressure). Locks in
  one real, confirmed behavioral difference from Widgets: `setCellValue()` (the double-click
  entry point) is deliberately not undoable in the Quick port.
- **7e-1** — `QuickAppController` tab/file/sim lifecycle + Edit/Transform delegation: 20 tests.
  Surfaced the same-named-anonymous-namespace-helper unity-build collision pattern for the
  first time (`QuickAppController.cpp`/`QuickElementPalette.cpp` both define `toQmlUrl()`) —
  fixed via `SKIP_UNITY_BUILD_INCLUSION` scoped to `QUICK_SHELL_SOURCES` (not a wholesale
  `UNITY_BUILD OFF`, which would cost every routine rebuild of the ~150 pre-existing domain
  test files).
- **7e-2** (backfilled, discovered while reading 7e-5's `StubDialogProvider.h` dependency) —
  `TestCanvasEmbeddedIC`: 12 tests giving `CanvasICRegistry` real coverage for the first time
  (copy/cut/paste, delete+undo, rotate, simulation-after-delete-undo, multi-blob selective
  delete, cross-tab paste, `QuickICController`'s embed/extract/remove commands). Found and
  fixed a real bug matching the original branch's own: `CanvasItem::copyAction()`/`cutAction()`/
  `pasteAction()` never round-tripped an embedded IC's blob bytes through the clipboard, so
  pasting into a different `CanvasItem` threw (no blob in the target registry, no real file to
  fall back to). Fixed by porting `ClipboardManager`'s exact dual-format
  `MimeType::BlobRegistry`/`BlobRegistryV2` bundling/import logic. Added
  `Tests/QuickShell/StubDialogProvider.h` (no stub existed yet for `Dialogs::provider()`).
- **7e-3** — inline-IC-tab lifecycle: `TestCanvasInlineIC` (4 tests) + a real fix landed
  directly rather than reproduced-then-fixed: `CanvasICRegistry` gained a `blobRenamed` Qt
  signal (it was a plain non-`QObject` class with no way to notify `QuickWorkspaceManager` of a
  rename), and `QuickWorkspaceManager` connects to it per-tab in `createNewTab()`, retitling any
  open inline-IC tab tracking the renamed blob.
- **7e-4** — `CanvasItem`'s protected event handlers: `TestCanvasItemInteraction` (20 tests) via
  real `QMouseEvent`/`QKeyEvent`/`QWheelEvent` through `QCoreApplication::sendEvent()`. Found
  and fixed a real bug: `startSelectionRect()` deselected via the write-only `m_selectedIds`
  cache instead of real `isSelected()` state, so clicking empty canvas after a paste/embed/
  palette-add never actually deselected the prior selection — fixed by routing through the
  already-correct `clearSelection()`.
- **7e-5** — `QuickElementEditor`: `TestQuickElementEditor` (22 tests, later 27 after 7f),
  covering the full property-panel surface plus the right-click-context-menu-backing surface
  (`prepareContextMenu()`/`morphCandidates()`/`morphSelectionTo()`). Needed one real adaptation:
  v2's `morphCandidates()` returns `QVariantMap` entries (`{"type": ..., "name": ...,
  "iconSource": ...}`), not a `Q_GADGET` with its own `.type()` accessor.
- **7e-6** — `QuickElementPalette` + `addElementToCurrentTab()`: `TestQuickElementPalette` (13
  tests, later 20 after 7f/7h). Same `QVariantMap`-not-gadget adaptation as 7e-5.
- **7e-7** — `testInlineICDropSaveCloseReopen`: the full drop-a-file-IC / auto-embed-on-save /
  close / reopen scenario, added to `TestCanvasInlineIC`. No new bug — a pure coverage gap for
  `QuickWorkSpace::save()`'s inline auto-embed block.
- **7e-8** — `QuickAppController` window-title/theme/language/Learn-menu: 11 tests added to the
  same class (now 33). Covers `windowTitle()`'s four real states, theme switching through the
  real process-global `ThemeManager` (restored after each test), language switching,
  `shortcutsHelpHtml()`'s generated content, `exercisesList()`/`toursList()`/`examplesList()`'s
  real bundled content.
- **7f** (four commits): `TestQuickCanvasZoom` (4 tests, the real -9..7 zoom-ladder clamp +
  `zoomToFit()`); `TestQuickMinimap` (12 tests, `minimapContentRect()`'s aspect-fit math,
  `navigateTo()`'s click-to-pan math, the Settings-backed geometry round trip, one real
  `QSignalSpy`-backed wait on the 200ms `regenerate()` throttle); 2 more wireless-Tx-label tests
  added to `TestQuickElementEditor`; 4 more tests added to `TestQuickElementPalette` closing
  `TestICInline`'s "Batch 8 — UI widgets" disposition (most of that batch tests either pure
  domain logic already covered elsewhere, or Widgets' `QMimeData` drag-and-drop transport,
  confirmed absent on the Quick side — `DropArea.onDropped` reads `drop.source.modelData`
  directly, no serialization involved).
- **7h** — `TestElementPalette`'s tooltip gap: one test confirming each palette entry's
  `"tooltip"` field is non-empty and genuinely element-specific.

**Real, confirmed features/gaps with no Quick-side equivalent, documented rather than silently
dropped** (all found via full reads of the relevant production file, not assumed): `CanvasItem`'s
GPU-composited rendering has no render-hint toggle to test (`testFastMode`); pan is
gesture-driven, unconditional state, not a settable drag mode (`testDragModeToggle`); zero
`Accessible.*` usage anywhere in `App/QuickShell/` (a real, currently-unported accessibility
gap, tracked here, not fixed — there is nothing yet to test); port-hover peer-label highlighting
is a real missing feature (`CanvasItem::hoverMoveEvent()` has no port-level hover/peer-highlight
concept at all).

**`TestArduino` DONE (2026-08-31)**: ported to `Tests/QuickShell/TestArduino.{h,cpp}`, 94 of 95
tests passing (1 graceful `QSKIP`, matching the original's own arduino-cli/simavr-availability
guard) — no dropped tests. Correction to the note above: on actually reading the file, the
`WorkSpace`/`Scene`/`CircuitBuilder`/`ICRegistry` coupling was real but far narrower than
assumed — only 5 of ~70 test methods touched them at all (the other ~65 build `GraphicElement`/
`IC` objects directly via `ElementFactory::buildElement()`/`new` and call the fully Scene-free
`generateFromElements()` helper, zero adaptation needed beyond the include path). Of the 5:
`testEmbeddedICGeneration`/`testEmbeddedICLabelWithNewlineDoesNotInjectCode` needed a real IC
registry to embed a blob into — adapted to `CanvasItem`/`CanvasICRegistry` (`icRegistry()`);
`testArduinoExportHelper`/`testArduinoSequentialMultiCycleCpu8Bit`/
`testGeneratedSketchMatchesEngineOnRippleCounter` just needed a flat, simulation-capable element
list — adapted to `QuickCircuitBuilder`. `TestUtils::arduinoExpectedDir()` (golden-fixture
directory path) ported into `QuickTestUtils.h` alongside its existing `cpuComponentsDir()`/
`systemVerilogExpectedDir()` siblings. `TestUtils::setupTestEnvironment()`'s per-class call in
`initTestCase()` was dropped, not ported: the Quick test runner already does the equivalent
setup once, globally, in `Tests/QuickShell/Runners/QuickRunnerUtils.h`, confirmed by every other
already-ported Quick test class passing without a call of its own. Notably,
`testArduinoSequentialMultiCycleCpu8Bit` (compiles and runs a generated sketch under `simavr`)
and the full 22-fixture `testArduinoExport` golden-comparison sweep both actually ran (not
skipped) in this dev container, giving real functional-equivalence coverage, not just a
structural check. Also fixed, found via this batch's Unity-build reshuffle (same recurring
self-containment-gap pattern as every prior batch): 7 more headers
(`TestComponents.h`/`TestAudioBox.h`/`TestBuzzer.h`/`TestConnectionSerialization.h`/
`TestElementProperties.h`/`TestFiles.h`/`TestGraphicElementSerializer.h`/`TestSerialization.h`)
missing their own `#include <QTest>`. Full 163-target Quick CTest suite green after both fixes.

**`TestICInline` DONE (2026-08-31)**: ported to `Tests/QuickShell/TestICInline.{h,cpp}`, 142 of
164 tests passing, 0 failed. 22 dropped, all confirmed genuinely Widgets-only with no Quick-side
equivalent (matching Phase 13 sub-step 7f's own already-reached disposition for this exact
bucket): `testElementLabelMimeRoundTrip`/`testElementLabelMimeFileBacked` (`ElementLabel` is a
`QFrame`-based Widgets palette-icon widget, same disposition as `TestElementLabel`'s own earlier
drop); `testICDropZoneMimeAcceptance`/`testICDropZoneWiredInUI`/
`testICDropZoneRejectsWrongDirectionDrop`/`testTrashButtonDragAcceptance` (`ICDropZone`/
`TrashButton` are Widgets-only drag-and-drop targets); `testElementPaletteUpdateEmbeddedICList`
(+Dedup/+Null)/`testElementPaletteSearchFindsEmbeddedIC`/`testElementPaletteRefreshAfterRemoveAll`
(need the Widgets-only `PaletteTestHarness`/`ElementPalette` -- `QuickElementPalette`'s own
embedded-IC-list coverage lives in `TestQuickElementPalette` instead);
`testSceneAddItemMimeDataThrowCleansUp`/6 `testSceneDropEmbedded*`/4 `testSceneDropEvent*`
(`Scene::addItem(QMimeData*)`/`Scene::dropEvent()`'s raw-MIME-bytes parsing has no Quick
equivalent -- `CanvasItem::addElementFromPalette()` takes already-decoded fields directly, never
a serialized `QMimeData` payload); `testBlobNameCollisionDuringRename` (drives the Widgets-only
`ElementEditor`'s `QLineEdit` blob-name field directly -- the registry-level guard it backs up is
already independently covered by the kept `testRenameBlobCollisionRejected`). The pure-domain
`SelectionCapabilities::computeCapabilities()`-based quarter of "Batch 8" (4 tests) ported
cleanly with zero adaptation.

Every one of the other 140 ported tests needed the same mechanical, 1:1-API substitution
`TestCanvasEmbeddedIC.cpp` already established (`Scene`/`WorkSpace`/`ICRegistry`/`CircuitBuilder`
-> `CanvasItem`/`QuickWorkSpace`/`CanvasICRegistry`/a small file-local `CircuitBuilder` compat
shim wrapping `CanvasItem`; `Commands.h`'s `UpdateCommand`/`RegisterBlobCommand`/
`RemoveBlobCommand`/`RenameBlobCommand`/`UpdateBlobCommand` -> `CanvasCommands.h`'s identically-
shaped `Canvas*` counterparts) -- confirmed by reading every one of these classes' constructor
signatures directly rather than assuming parity. Two real, confirmed bugs found and fixed, both
genuine use-after-frees (not test artifacts) surfaced only because this port ran the actual test
bodies rather than just getting them to compile:

- **A stack-allocated-element double-free**, same class as the earlier `TestConnectionSerialization`
  finding: 8 tests declared `InputSwitch swA, swB; Led led;` as locals *after* a `QuickWorkSpace ws;`
  local, then registered their addresses with `ws`'s `CanvasItem` -- `CanvasItem::~CanvasItem()`
  unconditionally `qDeleteAll()`s every registered element, so by the time `ws` (declared first,
  destructs last, per reverse-declaration-order) tried to delete them, `swA`/`swB`/`led` had
  already destructed as ordinary locals. Fixed by heap-allocating them (`new InputSwitch()` etc.),
  matching the same fix `TestConnectionSerialization` already used.
- **A real, previously-unknown production use-after-free** in the IC-blob-reload path, found via
  a genuine crash (SIGSEGV inside `rebuildSpatialIndex()`'s wire-stroke pass, called from the very
  next `restartSimulation()`) while running
  `testOnChildICBlobSavedPortCountWithConnections()`: an IC's `load()`/`loadFromBlob()` tears down
  and rebuilds its own ports, and each old `Port`'s destructor drains (deletes) whatever
  `Connection` is still attached to it -- exactly like `CanvasChangePortSizeCommand`'s own
  port-shrink path, which is why *that* command pre-drains via
  `CanvasCommandUtils::drainPortConnections()` before ever touching a port count.
  `CanvasUpdateBlobCommand::loadData()` and `QuickWorkSpace::onChildICBlobSaved()` both called
  `elm->load()`/`ic->loadFromBlob()` directly with no equivalent step, so any connection to a
  port the new blob's IC doesn't have anymore got deleted out from under
  `m_canvas->connections()` without the canvas ever being told -- a dangling pointer that
  crashed on its *next* access, not immediately (which is why the crash surfaced specifically on
  the test's own explicit `undo()` call, one command later, rather than at the mutation site
  itself). Fixed by extracting a new shared `CanvasCommandUtils::drainAllConnections(elm, canvas)`
  helper (removes+deletes every connection on `elm`'s ports, `canvas->removeItem()` first) and
  calling it from both real call sites plus the one test
  (`testReconnectConnectionsSkipsDeletedElement`) that also re-embeds an already-wired IC
  directly, before the reload, in all three places.

Full 164-target Quick CTest suite green after both fixes. **This closes Phase 13's declared
"port `TestArduino`/`TestICInline`" step entirely.**

## Feature-gap tracking (see plan's "Feature-gap decisions")

- [x] Show Gates / Show Wires — canvas-level logic implemented in Phase 5
      (`CanvasItem::setGatesVisible()`/`setWiresVisible()`), but that was only half the decision:
      no chrome menu trigger and zero test coverage existed until the Phase 13 wrap-up
      re-audit (2026-09-02) found both gaps and closed them -- `QuickAppController` gained
      per-tab `gatesVisible`/`wiresVisible` properties (mirrors `muted`'s resync-on-tab-switch
      pattern) wired to two checkable `Main.qml` View-menu items, plus 2
      `TestCanvasItemInteraction` + 1 `TestQuickAppController` tests. Full 173-target Quick
      CTest suite green. Now genuinely fully done.
- [x] IC double-click opens sub-circuit tab, not inline editor — fixed post-Phase-13, once
      `openICInTab()`'s exact resolution pattern had been proven repeatedly in tests (7e-2/
      7e-3/7e-7): `IC::handleDoubleClick()` (Widgets-free, part of `wiredpanda_lib`) already
      emitted `requestOpenSubCircuit(id(), blobName(), file())` fully resolved -- nothing on the
      Quick side had ever connected to it. `CanvasItem::mouseDoubleClickEvent()`'s IC branch now
      emits its own `icOpenRequested(elementId, blobName, filePath)`, and
      `QuickWorkspaceManager` connects to it per-tab in `createNewTab()`, resolving it exactly
      the way `SceneUiBinder.cpp`'s identical `Scene::icOpenRequested` connection does: an
      embedded IC's blob (read from that tab's own `icRegistry()`) opens inline via
      `openICInTab()`; a file-backed IC's path opens as a new top-level tab via
      `loadPandaFile()`. Two new `TestCanvasInlineIC` tests drive the real double-click gesture
      via `QCoreApplication::sendEvent()` end to end for both cases.
- [x] IC file-watching/hot-reload — ported. `CanvasICRegistry` gained a `QFileSystemWatcher`
      (`watchFile()`/`onFileChanged()`), and `CanvasItem::addItem()` calls `watchFile(ic->file())`
      for every file-backed IC it registers, mirroring `Scene::registerItem()`'s identical hook.
      `onFileChanged()` mirrors `ICRegistry::onFileChanged()` exactly (same `Application::
      guardedSlot()` wrapping, same `reloadTargetsAtomically()`/`CanvasUpdateBlobCommand` reuse)
      except it doesn't port the unused `cachedFileBytes()`/`m_fileCache` cache -- confirmed
      nothing else in the codebase (Widgets included) ever calls `cachedFileBytes()`, so it was
      dead weight in the original, not behavior worth preserving. Two new `TestCanvasEmbeddedIC`
      tests exercise it with a real file rewrite + real `QFileSystemWatcher` notification
      (`QSignalSpy::wait()`), not a direct `onFileChanged()` call: one confirms
      `definitionChanged()` fires even with no IC referencing the file, the other confirms a
      real file-backed IC reloads (port count actually changes) and a `CanvasUpdateBlobCommand`
      is pushed. Undo restoring the *pre-edit* port count is deliberately not asserted --
      confirmed by reading `IC::save()` that a file-backed IC's own snapshot stores only a path
      reference, never port/sub-circuit data, so "undo" after an external edit reloads from the
      very same (already-rewritten) file and observes the *new* content again; there is no old
      copy anywhere to restore. This is the same disposition already established for
      `testRemoveICFileIsUndoableA14`, and matches the original `TestICInline::
      testOnFileChangedPushesUndoCommandC5`'s own real scope (verified by reading it directly):
      it never asserts a port-count revert either, only that exactly one command is pushed and
      undo walks the stack back without throwing on stale ids.
- [x] `registerBlob()` recursive `makeBlobSelfContained()` — ported verbatim (reading file-backed
      dependencies relative to `CanvasItem::contextDir()` instead of `Scene::contextDir()`;
      everything else, including the recursion-depth guard and cycle detection, is identical).
      `registerBlob()` now calls it exactly like `ICRegistry::registerBlob()` does. Regression:
      `TestCanvasEmbeddedIC::testRegisterBlobInlinesFileBackedDependency()`.
- [x] `renameBlob()` recursive nested-reference rewrite — `renameBlobReference()` ported verbatim
      (a pure `QByteArray` transform with no `Scene`/`CanvasItem` dependency at all); `renameBlob()`
      now loops over every other stored blob and rewrites its nested reference, exactly like
      `ICRegistry::renameBlob()` does. Regression:
      `TestCanvasEmbeddedIC::testRenameBlobUpdatesNestedMetadataReference()`.
- [x] Ctrl+click-swallow at the element/wire level — confirmed already resolved by Phase 6's own
      gesture design, not a live gap: `CanvasItem::mousePressEvent()`'s `Qt::ControlModifier`
      branch (its own comment cites `Scene::eventFilter()` by name) intercepts Ctrl+click on an
      element *before* any per-item click-select logic runs, driving clone-drag directly --
      exactly the same "intercept before default per-item handling" shape the old `sceneEvent()`
      overrides existed for, just via a fresh, deliberate redesign rather than a literal port.
      The original checklist wording ("let canvas-level rubber-band/multi-select win") doesn't
      map cleanly onto the new architecture: Quick has no separate per-item event handler for a
      canvas-level gesture to have to "win" against in the first place -- `CanvasItem` is the
      sole, unified recipient of every mouse event, so the whole class of problem the swallow
      mechanism solved doesn't exist anymore. Element-level Ctrl+click is real, deliberate, and
      already covered by `TestCanvasItemInteraction::testCtrlClickClonesSelectionInPlace()`.
      Wire-level Ctrl+click has no separate behavior to re-derive: wires aren't independently
      selectable at all in this port (`SpatialIndex::queryRect()`'s rubber-band excludes wire
      ids, a single click on a wire is a no-op past `cancelEditedWire()`) -- an already-documented
      Phase 5 selection-model limitation (see `CanvasItem.h`'s own Clipboard-section doc comment),
      not part of this item.
- [x] `TestICInline` sub-tests blocked on the above — unblocked: all three underlying features
      (file-watching, `renameBlob()` nested-reference rewrite, `registerBlob()` self-containment)
      are now ported, each with its own `TestCanvasEmbeddedIC` regression covering the real
      intent. `testOnFileChangedPushesUndoCommandC5`, `testRenameBlobUpdatesNestedMetadata`, and
      the `makeBlobSelfContained()`-recursion tests should be ported for real when
      `TestICInline`'s batch lands, not dropped.
- [x] `ICController`'s toolbar/menu-triggered methods (`makeSelfContained()`, `addICFromFile()`,
      `embedSelectedIC()`, `extractSelectedIC()`, `addEmbeddedICFromFile()`) — **DONE
      (2026-09-01)**: all five ported to `QuickICController`/`QuickAppController`, wired to real
      toolbar UI — `ElementPalette.qml` grew "Add IC..." `ToolButton`s on the file-based/embedded
      sections and a "Make Self-Contained" `Button` below the trash target; `ElementEditor.qml`
      grew "Embed"/"Extract to file..." `Button`s for a selected IC (new `embedIcVisible`/
      `extractIcVisible` properties on `QuickElementEditor`, computed from the same
      `SelectionCapabilities.isFileBacked`/`isEmbedded` flags `blobNameVisible` already uses).
      `showRemoveICHint()` (a modal "Drag here to remove" popup) is deliberately NOT ported:
      `ElementPalette.qml`'s trash drop target already shows that hint as an always-visible
      inline label, so a redundant click-triggered popup would only duplicate it — a real design
      simplification, not a dropped feature. 16 new tests in `TestCanvasEmbeddedIC` (core
      success/guard paths for all five, not every one of the ~25 corresponding Widgets tests,
      since most of those exercise QMessageBox/QInputDialog wording this port's `DialogProvider`
      abstraction already covers once, generically, elsewhere). Full 172-target Quick CTest
      suite green (`TestCanvasEmbeddedIC` alone: 29/29). See plan's Feature-gap decision #6.
- [x] `PropertyShortcutHandler`'s keyboard-driven property cycling (input count, clock frequency,
      color) — **was already DONE, this checklist entry was simply stale**: rereading
      `CanvasItem.cpp` (2026-09-01) found `adjustMainProperty()`/`adjustSecondaryProperty()`/
      `nextElm()`/`prevElm()` and their six `prevMainPropShortcut()`/`nextMainPropShortcut()`/
      `prevSecndPropShortcut()`/`nextSecndPropShortcut()`/`nextElm()`/`prevElm()` wrappers fully
      ported and dispatched from `keyPressEvent()`'s `Key_BracketLeft`/`Key_BracketRight`/
      `Key_BraceLeft`/`Key_BraceRight`/`Key_Less`/`Key_Greater` branches — 1:1 with
      `PropertyShortcutHandler.cpp`, landed as part of Phase 6's own original commits (see this
      doc's own Phase 6 detail section above, "Split/Morph/property-cycling"). Test coverage
      already existed too (`TestCanvasItemInteraction.cpp`). The audit that opened this item
      (2026-08-31) grepped for the wrong names or didn't check `CanvasItem.cpp` directly; no
      code change was needed. See plan's Feature-gap decision #7.
- [x] No update-check feature exists anywhere in Quick (`UpdateChecker`/`UpdateController` have
      zero Quick-side presence) — see plan's Feature-gap decision #8. **Assigned to Phase 11**
      (startup polish), post-hoc. **DONE (2026-09-01)**: new
      `App/QuickShell/Chrome/QuickUpdateController.{h,cpp}` wraps the same, unchanged
      `App/Core/UpdateChecker.h` (`checkForUpdates()` relayed via a plain
      `updateAvailable(...)` signal, `downloadUpdate()`/`cancelDownload()` reimplementing
      `UpdateController::downloadUpdate()`'s body with `downloadProgress`/`downloadFinished`
      signals instead of a `QProgressDialog` — it owns no dialog of its own, unlike the
      `DialogProvider`-routed confirm/prompt dialogs elsewhere in this chrome, since a
      checkbox + conditional buttons + a progress bar don't fit that abstraction's generic
      shapes). New `App/QuickShell/UpdateDialog.qml` (a single `Dialog` with three content
      states — available/downloading/finished — rather than mirroring `UpdateController`'s
      two separate `QDialog`/`QProgressDialog` instances), opened via a `Main.qml` `Connections`
      on `updateAvailable`, same `open()`-by-id precedent as `shortcutsDialog`/
      `truthTableDialog`. `Main.qml`'s Help menu grew the checkable "Automatically Check for
      Updates" toggle (`autoCheckEnabled` property, Settings-backed) — mirroring
      `actionCheckForUpdates` exactly: Widgets has no manual "check now" action either, only
      this toggle; the actual check always runs once at startup
      (`QuickAppController::updateController()->checkForUpdates()`, called from `Main.cpp`
      right before `app.exec()`), gated internally by this same setting. 8 new tests in
      `TestQuickUpdateController` (auto-check round-trip, skip-version persistence,
      non-interactive no-op, download success/no-op-while-in-flight/cancel, via the same local
      `QTcpServer` mock-HTTP-server technique `Tests/Unit/Ui/TestUpdateController.cpp`/
      `Tests/Unit/Core/TestUpdateChecker.cpp` already use) — not a full port of
      `TestUpdateController.cpp`'s dialog-driven tests, since this class owns no dialog for
      those techniques to apply to, and the underlying `UpdateChecker`/network/file-write logic
      is already covered by the existing, unchanged tests for the class it wraps. Full
      173-target Quick CTest suite green.

## Post-Phase-13 audit findings (2026-08-31)

Full audit against current `master` (216 registered Widgets test classes, up from the 185 the
original branch's own audit doc was written against) vs. v2's 128-target Quick suite. See the
plan file's own "Post-Phase-13 audit findings" section for full detail on each item. Each is
assigned to the existing phase whose scope it belongs to (Phase 4 for zero-`CanvasItem`-coupled
domain logic, Phase 13 for `CanvasItem`/chrome-coupled UI test parity — Phase 13's own row
already named "MCP handler parity"/"AppController lifecycle" as in scope, so these are gaps in
that already-declared scope, not new scope):

- **DONE (2026-08-31)**: MCP handler functional test coverage was mostly missing: only
  `TestQuickElementHandler`/`TestQuickFileHandlerSecurity`/`TestQuickICHandlerSecurity` existed,
  despite all six Quick handlers being real, built production code (Phase 12a). Fixed: all 6
  remaining classes ported —
  `Tests/QuickShell/TestQuickBaseHandler.{h,cpp}` (42 tests, exercises `QuickBaseHandler` via a
  local `StubHandler`, same technique the Widgets original uses; everything except
  `currentCanvas()`/`validateElementId()`/`validatedElement()` is inherited unmodified from
  `BaseHandler`),
  `Tests/QuickShell/TestQuickConnectionHandler.{h,cpp}` (30 tests; the two split-connection
  item-count assertions needed a real adaptation, not just a rename — Widgets'
  `scene->items().size()` flattens the whole `QGraphicsItem` tree, so `CanvasSplitCommand::redo()`'s
  real effect is asserted as `elements().size()`/`connections().size()` each growing by exactly
  1, not a "+5" figure with no `CanvasItem`-side equivalent),
  `Tests/QuickShell/TestQuickHistoryHandler.{h,cpp}` (11 tests),
  `Tests/QuickShell/TestQuickICHandler.{h,cpp}` (47 tests, distinct from the already-existing
  `TestQuickICHandlerSecurity`; general `create_ic`/`instantiate_ic`/`list_ics`/`embed_ic`/
  `extract_ic` validation-chain and success-path coverage),
  `Tests/QuickShell/TestQuickSimulationHandler.{h,cpp}` (26 tests; `create_waveform`/
  `export_waveform` mirror the original's `m_persistentDolphin` design against
  `QuickDolphinController` instead of `BewavedDolphin` — needed a new `friend class` declaration
  on `QuickSimulationHandler.h`, matching `SimulationHandler.h`'s own),
  `Tests/QuickShell/TestThemeHandler.{h,cpp}` (9 tests, kept the original class name since
  `ThemeHandler` itself is already fully shared/portable code, reused as-is — not subclassed —
  by `QuickMCPProcessor`).

  Found and fixed two real bugs, neither a test artifact:
  - **A double-free** in `TestQuickBaseHandler`'s non-graphic-element test: `std::make_unique`-managed
    elements registered with a `CanvasItem` declared later in scope, same class already fixed in
    `TestConnectionSerialization`/`TestICInline` — heap-allocated instead.
  - **A pointer-identity test-methodology flaw**, not a production bug: `QuickDolphinController`
    (a plain `QObject`) very reliably gets the exact same heap address back from `delete`+`new`
    immediately after, unlike `BewavedDolphin`'s much larger `QWidget`-based construction — a raw
    pointer comparison would have passed even if the handler leaked the old instance. Fixed by
    tracking the old instance with a `QPointer` and asserting it was actually destroyed instead.
  - **A real, previously-undiscovered production use-after-free**: `CanvasItem`'s
    `strokeShapeFor()` (used by `rebuildSpatialIndex()`/`updateSpatialIndexFor()` to compute a
    wire's click-target stroke for the spatial index) unconditionally dereferenced
    `connection->startPort()`/`endPort()`, with no guard for an in-progress connection (freshly
    `new Connection()`'d, neither endpoint attached yet — exactly what
    `ConnectionHandler`'s own "skip it, don't crash" tests exercise, and a real reachable state
    mid-drag too). Found via `testDisconnectElementsSkipsInProgressConnection`'s real crash.
    Fixed by returning an empty `QPainterPath` when either port is null.

  Full 170-target Quick CTest suite green after all three fixes.
- **DONE (2026-09-01)**: Exercise/Tour Quick controllers (`QuickExerciseController`/
  `QuickTourController`) had zero permanent test coverage — Phase 12a verified them only via a
  manual self-test block removed before commit. Fixed via original test authoring (no direct
  Widgets-side unit test exists for `ExerciseOverlay`'s logic to port from):
  `Tests/QuickShell/TestQuickExerciseController.{h,cpp}` (12 tests, driving the real bundled
  `:/Exercises/basic-and-gate.json` fixture end to end against a real `CanvasItem` via
  `CanvasAddItemsCommand`/`Connection`, not synthetic step data) and
  `Tests/QuickShell/TestQuickTourController.{h,cpp}` (7 tests, `:/Tours/ui-overview.json`).
  Two real bugs found and fixed along the way, both via genuine cross-run reproduction, not
  guesswork:
  - **Test-methodology bug** (not production): `ExerciseEngine`/`TourEngine::start()` resume from
    real, `QSettings`-backed persisted progress (`StepEngineCore::start()`'s
    `m_persistence.getProgress(m_id)`) — correct, intended "resume where you left off" behavior,
    sandboxed per test *process* (`QuickRunnerUtils.h`), not per test function. Every test in each
    class shares one bundled exercise/tour id, so a prior test's `advance()` calls leaked into the
    next test's `start()`. Fixed with an `init()` slot in each class resetting
    `Settings::setExerciseProgress()`/`setTourProgress()` and the completed-list before every test.
    A second, unrelated test bug in the same investigation: three tests drove wire connections via
    a bare `canvas->addItem(connection)` instead of through the undo stack
    (`CanvasAddItemsCommand`) — since `ExerciseEngine::onCircuitChanged()` is wired to
    `CanvasItem::undoStack()->indexChanged`, a connection added outside the undo stack never
    triggers re-validation, so the exercise silently never advanced past the "connect" step. Fixed
    by routing every connection through `CanvasAddItemsCommand` like every other circuit edit.
  - **Real, previously-undiscovered production bug**: closing a *completed* exercise's overlay in
    the Quick chrome did nothing — `QuickExerciseController::close()` calls `ExerciseEngine::stop()`,
    which is a no-op once `markCompleted()` has already deactivated the engine
    (`StepEngineCore::stop()`'s own "already inactive" guard silently drops the call, so
    `exerciseStopped` never fires and `onExerciseStopped()` — the only place that resets the
    completed flag and notifies QML — never runs). The Widgets equivalent
    (`ExerciseOverlay`/`MainWindow.cpp`) never hit this because closing there unconditionally
    destroys the overlay widget regardless of what `stop()` decided internally — Quick's chrome
    has no such fallback since it never destroys anything, only toggles bindable state. Fixed in
    `App/QuickShell/Chrome/QuickExerciseController.cpp`'s `close()`: always resets `m_completed`
    and emits `activeChanged()`/`completedChanged()` itself when the exercise was already
    completed, instead of depending solely on the engine's guarded signal.

  All 21 new tests passed; full 172-target Quick CTest suite green after.
- **DONE (2026-08-31)**: `QuickWorkSpace` (autosave debounce/naming, external-dependency-copy-on-save)
  had no dedicated test class of its own — only exercised incidentally as a fixture-builder
  inside other tests. Fixed: `Tests/QuickShell/TestQuickWorkSpace.{h,cpp}`, 39 tests merging the
  Widgets-side `TestWorkspace` (autosave triggering/naming/cleanup) and `TestWorkspaceFileops`
  (save/load round-trips, external-dependency-copy-on-save, file-state management, migration)
  against `QuickWorkSpace` instead of `WorkSpace` — both source classes test one production
  class, so their coverage merges into this one Quick-side class. Near-duplicate assertions
  across the two source files were consolidated rather than ported 1:1 (e.g. three separate
  "two workspaces get distinct autosave files" tests became one); `testRecoveredFileRemovedFromSettings`
  was dropped as never having asserted anything about Settings/autosave removal in the first
  place, once its actual body was read rather than just its name. All 39 tests passed on first
  build/run; full 129-target Quick CTest suite green.
- **DONE (2026-08-31)**: no Quick equivalent of `TestFiles::testFiles()` (data-driven "every
  bundled example .panda loads cleanly" sweep) existed. Fixed: `Tests/QuickShell/TestFiles.{h,cpp}`
  (2 data-driven tests, 329 rows total: 21 bundled examples + every `.panda` fixture across
  every historical version under `Tests/BackwardCompatibility/`), ported against
  `QuickWorkSpace`/`CanvasItem` instead of `WorkSpace`/`Scene` -- the source's mixed
  `scene()->items()` + `qgraphicsitem_cast<Connection*>()` walk becomes `CanvasItem`'s own
  already-separate `elements()`/`connections()` accessors, same intent, no `QGraphicsItem`
  concept left to cast through. All 329 rows passed on first real run; full 134-target Quick
  CTest suite green.
- **DONE (2026-08-31)**: `LanguageManager`, `ExerciseTourResources`, install-relative-paths
  `candidates()`, and `ExternalFilePath::resolve()`'s edge cases — shared/portable Core classes
  already used in production by Quick — had no or only incidental Quick-side test reference.
  Fixed: `Tests/QuickShell/TestLanguageManager.{h,cpp}` (12 tests), `TestExerciseTourResources.{h,cpp}`
  (23 tests), `TestInstallRelativePaths.{h,cpp}` (8 tests), `TestExternalFilePath.{h,cpp}` (16
  tests) — all four verbatim copies of their Widgets-side originals (same class names, following
  the existing `TestSerialization`/`TestConnections`/`TestNotifyCatch` convention for classes
  already fully shared/portable, not "TestQuick"-prefixed), only the #include path for each
  file's own header changed. `TestLanguageManager.cpp` dropped an unused
  `Tests/Common/TestUtils.h` include carried over from the Widgets original — pulling it in
  breaks the Quick build (`TestUtils.h` drags in the still-`QGraphicsItem`-era `App/Scene/Scene.h`),
  and nothing in the ported test bodies actually calls `TestUtils::` anything, confirmed by
  grepping the original file too. Adding these four files' Unity-build batch also exposed
  `Tests/QuickShell/IC/QuickTestUtils.h` using `Led*` without including `Led.h` itself (relying
  on a batch-mate to provide it) — same recurring Unity-collision shape as Phase 13's earlier
  finds, fixed at the point of use. All 4 classes passed on first real run after those two fixes;
  full 133-target Quick CTest suite green.
- **IN PROGRESS (2026-08-31)**: a whole bucket of pure-domain `Tests/Unit/Elements`/`Logic`/
  `Wiring`/`Factory`/`CodeGen`/`Common` classes were never brought into Phase 4's scope — most
  predate the original `qtquick-rewrite` branch's own start, so its audit doc never saw them.
  Likely the second-largest remaining test-porting bucket after `TestICInline`.
  **First batch landed**: `TestCodeGenUtils` (4), `TestCommon` (6), `TestDisplay` (8),
  `TestDragDropPayload` (6), `TestEnums` (28), `TestFeatures` (13), `TestGeometry` (11),
  `TestIcons` (33), `TestMultiplexing` (12) — 9 classes, 121 tests, verbatim copies (only each
  file's own `#include` path changed; several also carried an unused `Tests/Common/TestUtils.h`
  include from the Widgets original, dropped the same way `TestLanguageManager.cpp` was).
  **Real finding, not ported**: `TestElementLabel` looked like a pure-domain class from its
  `Tests/Unit/Elements/` directory and an empty `scene_coupled` grep result, but its production
  class `App/Element/ElementLabel.h` is `class ElementLabel : public QFrame` — the Widgets
  element-palette's draggable icon+name widget, unrelated to on-canvas `GraphicElement` text
  labels despite the confusable name. Confirmed via a real link failure (undefined `ElementLabel::`
  symbols), not just a header include. Same disposition as `TestLabeledSlider`/`TestTrashButton`/
  `TestICDropZone` — a genuine Widgets-only class with no Quick equivalent (`QuickElementPalette`/
  `ElementPalette.qml`'s own drag mechanism already covers the intent, Phase 8/9). **Also found**:
  `TestQuickWorkSpace.h`/`TestFeatures.h` (this session's own files) were missing their own
  `#include <QTest>`, surfaced only once new files shifted Unity-batch membership and produced a
  real, hard-to-read GCC error ("specialization of qCompare(long long, int) after instantiation")
  — root-caused to Qt's `qtest.h` explicit `qCompare(qint64,qint32)` specialization needing to be
  visible before any implicit generic-template instantiation for that exact pair in the same
  Unity TU. Fixed at 11 headers total (both new and pre-existing) that relied on a batch-mate to
  provide `<QTest>`, not by reordering files.
  **Second batch landed**: `TestElementLogic` (71), `TestElementLogicErrors` (14),
  `TestElementAppearance` (9), `TestStatusOps` (38), `TestNodeLogic` (2) — 5 classes, 134 tests.
  Needed `initSrc()`/`initElm()` added to `QuickTestUtils` (`pixmapHasInk()` already existed
  there, added for an earlier phase — duplicating it caused a real redefinition build error,
  caught and fixed by removing the duplicate, keeping the pre-existing one). `TestElementAppearance`
  additionally needed a real signature adaptation, not just an include-path fix: its three
  `elm.paint(&painter, &option, nullptr)` calls used the pre-Phase-2 `QGraphicsItem::paint()`
  signature (`QStyleOptionGraphicsItem*` + `QWidget*`); `GraphicElement::paint()` is now
  `paint(QPainter*)` only, matching the pattern already used by `TestDemux`/`TestMux`/
  `TestConnection`. All 5 classes passed on first real run after those two fixes; full
  148-target Quick CTest suite green.
  **Third batch landed**: `TestTruthTable` (17 of 23 tests; the 6 dropped —
  `testRotatedIconStaysUpright`, `testPaintingSelected`, `testMouseDoubleClickRequestsEditor`,
  `testBigPivotsAtBoundingRectCenter`, `testBigRotationDoesNotDriftInScene`,
  `testBigFlipDoesNotDriftInScene` — all construct a `QGraphicsScene` and/or call
  `mapToScene()`/`transformOriginPoint()`/`setSelected()`/the old `paint(QPainter*,
  QStyleOptionGraphicsItem*, QWidget*)` signature/`mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)`,
  none of which exist post-Phase-2; same disposition already established for
  `TestDemux`/`TestMux`'s identical "Big/rotation/pivot/drift" drops, deferred to the single
  already-tracked `TestIc` 8-test rotation/pivot gap) and `TestGraphicelementAdvanced` (25 of 27
  tests; the 2 dropped — `testLabelStaysUprightWhenRotatedOrFlipped`,
  `testLabelPositionStaysFixedWhenRotatedOrFlipped` — walk `elem->childItems()` for a
  `QGraphicsSimpleTextItem` label child, a QGraphicsItem-era concept with no Quick-side
  equivalent; a real, currently-untracked "label upright/position-on-rotation has no test
  technique yet" gap, same "one Phase item, not several" disposition). 44 tests total. All
  passed on first real run; full 150-target Quick CTest suite green.
  **Fourth batch landed**: `TestAudioBox`, `TestBuzzer`, `TestComponents`,
  `TestConnectionSerialization`, `TestDisplay7`, `TestElementFactory`, `TestElementProperties`,
  `TestGraphicElementSerializer`, `TestWirelessNode` — 9 classes, ~216 tests, each needing real
  `WorkSpace`/`Scene`/`CircuitBuilder` → `QuickWorkSpace`/`CanvasItem`/`QuickCircuitBuilder`
  adaptation (not a verbatim copy). Two real, confirmed bugs found and fixed during this batch,
  not just mechanical translation:
  - **`GraphicElement::paint()`'s pre-Phase-2 signature**: `TestDisplay7`'s `disp->paint(&painter,
    &option, nullptr)` used the old `QGraphicsItem::paint()` 3-arg signature; `paint(QPainter*)`
    is now the only overload (same fix already applied to `TestElementAppearance`/`TestDemux`/
    `TestMux`).
  - **A real double-free, caught by actually running the test, not just building it**:
    `TestConnectionSerialization`'s 8 test methods construct elements via
    `std::make_unique<And>()`/`Or`/`InputSwitch`/`Led` and add them to a `CanvasItem`.
    `CanvasItem::~CanvasItem()` `qDeleteAll()`s every still-registered element on destruction —
    so with the element also owned by a `unique_ptr` declared *after* the canvas, the element
    got deleted twice (once by its own unique_ptr's normal stack unwind, once more by the
    canvas's destructor, since locals destruct in reverse declaration order). Confirmed via a
    real crash (`./test_wiredpanda_quick TestConnectionSerialization` produced a stack trace),
    not caught at compile time. Fixed by dropping `unique_ptr` for every element added to a
    canvas (`auto *x = new Type()` instead, letting the canvas be sole owner) — the same fix
    independently applied to `TestGraphicElementSerializer`'s one affected test *before* this
    batch was ever run, from reading `CanvasItem::~CanvasItem()`'s body directly rather than
    assuming Widgets' `Scene` ownership semantics carried over unchanged. Every other file in
    this and earlier batches was swept for the same `make_unique<...>()`-added-to-canvas pattern
    afterward; none of the other matches were actually registered with a canvas (standalone
    save/load elements only), confirmed safe.
  All 9 classes' own tests plus the full 159-target Quick CTest suite green after the fix.
  **Fifth (final) batch landed, this bucket now CLOSED**: `TestGraphicElement` (17 of 21 tests;
  the 3 dropped — `testInlineDoubleClickRenamesLabelWithUndo`/
  `testInlineDoubleClickEscapeCancelsWithoutUndo`/`testInlineDoubleClickIgnoredWithoutLabel` —
  drive a `QLineEdit` hosted by a `QGraphicsProxyWidget`, a Widgets-only inline-edit mechanism
  already covered on the Quick side by `TestCanvasItemInteraction` (Phase 6/9); 1 more —
  `testSceneEventSwallowsCtrlClickPress` — calls the now-gone `GraphicElement::sceneEvent()`,
  already confirmed covered by `TestCanvasItemInteraction::testCtrlClickClonesSelectionInPlace()`),
  `TestInputRotary` (38 of 38), `TestInputElements` (34 of 34) — 3 classes, 89 tests. All three
  needed real `Scene`/`QGraphicsSceneMouseEvent` → `CanvasItem`/`QMouseEvent`+
  `QCoreApplication::sendEvent()` interaction-test adaptation (the technique
  `TestCanvasInlineIC.cpp`'s `sendDoubleClick()` helper already established), not a per-test drop
  — every mouse-driven test in all three classes ported successfully. Two more real, confirmed
  production bugs found and fixed by porting these tests faithfully against the real dispatch
  path, both the same missing-lock-guard class as each other:
  - **`CanvasItem::activateOnPress()` never checked `isLocked()` for `InputSwitch`/`InputRotary`**,
    unlike their Widgets `mousePressEvent()` originals, which both guard on `!m_locked` — a
    locked switch/rotary could still be toggled/advanced by a plain click in the live Quick app.
    Found while porting `TestInputRotary::testMousePressWhileLockedDoesNotAdvance()`: an initial
    attempt called the (protected, inaccessible) `InputRotary::handleClick()` directly, which
    would have silently passed regardless of the guard's presence — switching to a real
    `QMouseEvent` sent through the actual dispatch path (the only way to reach a `protected`
    method from an external test) is what surfaced the missing guard. Fixed directly in
    `CanvasItem::activateOnPress()` (`App/QuickShell/Canvas/CanvasItem.cpp`).
  - **`CanvasItem::mousePressEvent()`/`mouseReleaseEvent()`'s `InputButton` branches had the same
    gap**: real `InputButton::mousePressEvent()`/`mouseReleaseEvent()` both guard their entire
    body on `!m_locked`, so a locked button neither turns on on press nor tracks
    `m_pressedInputButton` for release; this dispatcher pressed it unconditionally. Found by
    `TestInputElements::testInputButtonMousePressReleaseWhileLockedDoesNotToggle()` failing on
    first real run (not assumed from the `InputRotary` fix — independently confirmed against
    `InputButton.cpp`'s own source). Fixed by guarding both the press-time `setOn()`/
    `m_pressedInputButton` assignment and the release-time `setOff()` on `isLocked()`.
  A real hit-testing detail also had to be corrected in the new mouse-press tests themselves (not
  a production bug): `m_index.queryPoint()` — the spatial index `mousePressEvent()`'s hit test
  reads — is only populated by `rebuildSpatialIndex()`, which a bare `CanvasItem::addItem()` does
  *not* call (it only registers ids); real interactive adds always go through a command
  (`CanvasAddItemsCommand`), which does rebuild it. `TestCanvasItemInteraction.cpp`'s own
  click-driven tests already use this exact pattern — the first attempt's plain `addItem()` calls
  were corrected to match. All 3 classes' own tests plus the full 162-target Quick CTest suite
  green after both fixes. **This closes the ~28-class pure-domain audit bucket entirely** — every
  class named in the original audit list is now ported (or explicitly dropped/deferred with a
  named reason), none left untriaged.
- Confirmed NOT a gap: the seven `TestDolphin*` serialization classes are already referenced from
  `TestSerialization`/`TestQuickDolphinController` (Phase 12b).
- **Phase 13 — `TestScene`/`TestSceneState` audit CLOSED (2026-09-02)**: every test in both
  classes cross-referenced against the existing Quick suite by direct code reading (not
  name-matching alone, which fails here since almost nothing was ported under an identical
  name). See "resolved below" for `TestScene`'s eight real gaps found and closed.
  `TestSceneState`'s 25 tests needed zero new ports: 4 (`testSceneRect*`/
  `testResizeSceneExpandsBounds`) test a `QGraphicsScene`-specific auto-expanding `sceneRect()`
  with no `CanvasItem` equivalent at all (a fixed-size `QQuickItem`, sized by `setSize()`, not
  auto-expanded by its children) -- genuinely N/A, not a gap. 3 (`testHoverPort*`) test
  `ConnectionManager::showHoverLabels()`'s scene-item-based hover labels (`PortHoverLabel`
  `QGraphicsItem`s) -- superseded by this canvas's signal-based `portHoverChanged()` mechanism,
  already covered by this session's own new port-hover tests (confirmed via
  `buildPortHoverChips()`'s identical "hovered port + `connectedPeers()`" logic). The
  remaining 18 (item/element queries, selection filtering, connection count/state tracking,
  Z-order, position-after-move) are confirmed already exercised, under different names, by the
  dozens of existing tests across `TestCanvasCommands`/`TestCanvasItemInteraction`/
  `TestCanvasEmbeddedIC` that constantly add/remove/select/connect elements and check
  `elements()`/`connections()`/`selectedElements()` -- including `testConnectionZOrderBehindElements`,
  verified correct by reading `updatePaintNode()` directly: `wireHaloNode`/`wireNode` are
  appended as earlier `QSGNode` children than `gateNode`, so wires paint behind element bodies
  by construction, the scene-graph-node-order equivalent of `Connection`'s fixed `zValue() == -1`.
  - **`TestScene` — partial pass (2026-09-02), 3 real gaps found and closed, most of the rest
    confirmed covered or architecturally superseded**: cross-referenced `TestScene.h`'s full
    test list against every existing Quick test file (not just the originally-estimated "~9
    remaining"). The bulk (add/remove/query, select/clear, copy/cut/paste, rotate/flip/delete/
    undo/redo, morph/property-cycle, edge-case no-selection guards) are already covered under
    different names by `TestCanvasCommands`/`TestCanvasItemInteraction`/`TestCanvasEmbeddedIC`.
    Several categories are confirmed N/A (Widgets-only event types or a deliberately-changed
    interaction model, not gaps): `testCtrlClickTogglesElementSelection` (Ctrl+click means
    something different now -- clone-drag, not toggle-select; Shift+click is this canvas's only
    toggle gesture, already covered), `testEventFilterRemapsShiftClickToCtrlClick` (no
    `eventFilter()` concept exists here), `testHelpEventShowsPortHoverLabelsOverPort`/
    `testHelpEventFallsBackForNonPortItem` (QGraphicsSceneHelpEvent has no equivalent -- but see
    the real port-hover gap this uncovered, below), `testClipboardCanPasteMatchesPasteFormats`/
    `testBuildDragImageClampsExtremeSelectionExtent` (`QDrag` ghost-image building doesn't
    apply -- Ctrl+drag clones directly, no drag image), the `WireAntialiasing*`/
    `CheckWireIdleRestore*`/`RestoreWireAntialiasingIsNoOp` family (a `QPainter`-antialiasing
    perf throttle specific to Widgets' immediate-mode rendering; this canvas's batched
    `QSGGeometryNode` renderer has its own, different, already-addressed perf-hardening history
    instead), `DragEnterEvent*`/`DragMoveEvent*`/`DropEventHandlesCloneDragFormat` (Quick's
    canvas has no drag-proposed-action-accept concept the way `QGraphicsScene`'s
    `dragEnterEvent()`/`dragMoveEvent()` do -- a `DropArea` accepts any drag by default; only
    the CloneDrag-format-specific half is genuinely N/A, since Ctrl+drag clones directly, no
    `QMimeData` round trip), `testSortSimpleChain`/`testSortMultipleChains`/
    `testSortCycleDetection`/`testSortDisconnectedComponents` (`Scene::sortByTopology()` is a
    thin wrapper around `ElementGraphUtils::sortByTopology()`, an already-shared, unchanged,
    portable free function -- the simple-chain shape is already covered by
    `TestSimulation::testTopologicalSorting()`, and the cycle-detection/disconnected-component
    edge cases test the same unmodified algorithm the still-running Widgets-side `TestScene`
    already validates, so duplicating them into the Quick suite would just be testing identical
    code twice, not new coverage), `testGeometryChangeKeepsSceneIndexConsistentKS` (a
    `QGraphicsScene` BSP-tree reindexing invariant -- this canvas's own equivalent,
    `SpatialIndex`/`rebuildSpatialIndex()`, already has its hit-testing invariant exercised
    indirectly by every mouse-interaction test that adds/resizes/rotates an element then clicks
    it, e.g. `TestCanvasCommands::testChangePortSizeIncreaseInputSize()` immediately followed by
    other tests' hit-testing). Eight real gaps found and closed:
    1. **Port-hover peer labels had zero test coverage.** `CanvasItem::updatePortHover()`/
       `portHoverChanged()` (the real Quick port of the hover-label feature) existed and was
       wired into `hoverMoveEvent()`/`hoverLeaveEvent()`, but nothing exercised it (confirmed by
       grep). 3 new `TestCanvasItemInteraction` tests
       (`testHoverOverPortEmitsPortHoverChangedWithPeers`/`testHoverOverNonPortEmitsNoPortHoverChips`/
       `testHoverLeaveClearsPortHover`).
    2. **`mouseDoubleClickEvent()`'s empty-label bounding-rect fallback had zero coverage.**
       `testInlineLabelEditorEmptyLabelUsesElementBoundingRectFallback()` drives the Widgets-only
       `InlineLabelEditor` class directly (no Quick equivalent -- the inline editor is a plain
       QML `TextField` positioned by `Main.qml`), but the *fallback rect computation itself*
       ("`labelSceneBoundingRect()` is empty when the label has no text yet, fall back to
       `sceneBoundingRect()`") lives in `CanvasItem.cpp`, and the existing
       `testDoubleClickLabeledElementEmitsInlineEditRequested()` never inspected the emitted
       rect argument at all. Closed with `testDoubleClickEmptyLabelFallsBackToElementBoundingRect`.
    3. **A real, previously-undiscovered production bug, found while investigating
       `testUpdateThemeUpdatesConnectionTheme()`: live theme switching never refreshed the
       canvas.** `Scene` connects `ThemeManager::themeChanged` to `Scene::updateTheme()`
       directly in its own constructor (confirmed by reading `Scene.cpp`); `CanvasItem` had no
       such connection, and no `updateTheme()` method to call one even if it did (confirmed by
       grep). Worse: `appearanceKeyFor()`'s atlas cache key has no theme dimension at all, so
       even a manual refresh wouldn't have invalidated already-cached element tiles --
       `TextureAtlas::clear()`'s own doc comment literally anticipates this exact scenario
       ("e.g. on a theme change that invalidates every cached appearance at once") but nothing
       ever called it. Net effect: switching Light/Dark/System while a circuit was open left
       every already-rendered element/port/connection frozen at whatever theme was active when
       it was last drawn, until an unrelated state change (rotation, status, selection)
       happened to invalidate that one element's own cached tile. Fixed: new
       `CanvasItem::updateTheme()` (element/connection loops + `m_atlas.clear()` + `update()`),
       connected to `ThemeManager::themeChanged` in the constructor, mirroring `Scene`'s own
       wiring exactly. New regression test:
       `TestCanvasCommands::testLiveThemeSwitchRefreshesElementsAndConnections` (had to check
       the port's *brush*, not its pen -- `ThemeManager.cpp`'s own comment confirms port pens
       are deliberately theme-invariant, only the status brush varies by theme, mirroring
       connection colors).
    4. **Dropping a `.panda` file from the file manager onto the canvas did nothing --
       worse, it would have thrown a QML runtime error.** `Main.qml`'s canvas `DropArea` casts
       `drop.source` straight to `PaletteItemDelegate` with no null check, so any non-palette
       drop (an OS-level file drag has no QML `Drag.source` Item at all) would have dereferenced
       `item.modelData` on `null`. Fixed: `QuickAppController::openDroppedPandaFile(QList<QUrl>)`
       (mirrors `Scene::droppedPandaFile()`'s validation -- local file, case-insensitive
       `.panda` suffix, first match wins) plus a `null` guard before the palette-cast branch in
       `Main.qml`. 3 new `TestQuickAppController` tests.

    5. **Wire-creation's two reversed/completion port-press paths had zero coverage.** The
       existing `testWireCreationByDraggingPortToPort()` only exercises one continuous drag
       from an output port to an input port; `mousePressEvent()`'s own port-press branch
       (confirmed by reading `CanvasItem.cpp`) has two more real, distinct paths: completing an
       in-progress wire via a second port *press* (not a drag/release), and starting a wire from
       an *input* port instead of an output. 2 new tests
       (`testPressOnPortWhileWireInProgressCompletesConnection`/
       `testPressOnEmptyInputPortStartsWireFromInput`).
    6. **Detaching an already-wired input port by pressing it had zero coverage.**
       `mousePressEvent()`'s `detachWire()` branch (pressing an occupied input port removes the
       existing connection and re-starts an edited wire from the same output) had never been
       exercised. New `testPressOnOccupiedInputPortDetachesWire`.
    7. **Right-click context menu had zero test coverage at all.**
       `CanvasItem::handleRightClick()` (select-then-`elementContextMenuRequested()` for a hit
       element, `emptyContextMenuRequested()` otherwise -- wired to real `Main.qml` `Menu`
       popups, confirmed present) had no test anywhere (confirmed by grep). 2 new tests
       (`testRightClickOnElementOpensContextMenu`/
       `testRightClickOnEmptyCanvasEmitsEmptyContextMenuRequested`) plus
       `testCtrlDoubleClickOnWireDoesNotInsertSplitNode` for `mouseDoubleClickEvent()`'s
       Ctrl-guard (mirrors `TestScene::testCtrlDoubleClickOnWireDoesNotInsertNode`).
    8. **`keyPressEvent()`/`keyReleaseEvent()`'s auto-repeat guards and the nudge-selection
       Ctrl/Alt-ignore guard had zero coverage.** All three guards existed and matched
       `Scene`'s identical originals (confirmed by reading `CanvasItem.cpp`), but
       `testKeyboardTriggerFiresInputSwitch()`/`testArrowKeyNudgesSelection()` never exercised
       the auto-repeat-suppression, momentary-InputButton-release, or Ctrl/Alt-arrow-ignored
       cases. 3 new tests (`testAutoRepeatKeyPressDoesNotRetoggleTrigger`/
       `testKeyReleaseTurnsOffMomentaryInputButton`/`testCtrlAndAltArrowKeysDoNotNudgeSelection`).

    16 new tests total (3 port-hover, 1 inline-label fallback, 1 theme-refresh regression, 3
    dropped-file-open, 2+1 wire-press edge cases, 3 right-click/double-click, 3 keyboard-guard).
    Full 173-target Quick CTest suite green.
  - **`TestSceneDropHandler` — RESOLVED (2026-09-01)**: all 7 tests accounted for, none needed
    a literal port. `testIsSupportedDropFormat*` (2 tests) don't apply -- Quick's canvas has no
    `QMimeData`/MIME-type-gated drop path at all. `testHandleNewElementDropAddsElementAtScenePosition`/
    `testAddFromMimeData*` (3 tests) are already covered by `TestQuickElementPalette`'s
    `testAddElementToCurrentTab*` tests: `PaletteItemDelegate`'s QML drag calls
    `AppController.addElementToCurrentTab()` directly with an already-known type/position, no
    `QMimeData` payload serialize/deserialize round-trip needed (confirmed by reading both).
    `testHandleCloneDragClonesSelectionAtOffsetPosition`/`testHandleCloneDragAcceptsLegacyMimeFormat`
    (2 tests) -- investigated and confirmed real, but architecturally superseded: Ctrl+drag on
    the canvas clones the selection via `CanvasCommandUtils::serializeItems()` +
    `deserializeAndAdd()` directly (`CanvasItem::mousePressEvent()`'s own doc comment explains
    why: driving the existing drag-to-move machinery on live duplicated elements gives the
    identical net effect without a real `QDrag`), not a `QGraphicsSceneDragDropEvent`/MIME-type
    round trip -- so there's no "legacy MIME format" concept to test either. This *was* a real
    coverage gap though: `TestCanvasItemInteraction::testCtrlClickClonesSelectionInPlace()` only
    exercised the zero-distance case, never the "clone lands at original + drag delta" behavior
    `testHandleCloneDragClonesSelectionAtOffsetPosition` checks for. Closed with a new
    `testCtrlDragClonesSelectionAtOffsetPosition`. Full 173-target Quick CTest suite green.
  - **`TestDialogs` — RESOLVED (2026-09-01)**: its `testElementEditor*` (5 tests, wireless
    Tx-node duplicate-label rejection) are already fully ported under different names in
    `TestQuickElementEditor.cpp` (`testWirelessModeRejectsDuplicateTxLabel`/
    `testWirelessModeAllowsUniqueTxLabelRename`/`testWirelessModeAllowsRxNodeToShareLabelWithTx`),
    confirmed by reading both files. Its `ClockDialog`/`LengthDialog` range-clamp tests (10
    tests) have no meaningful Quick port: the Quick equivalent (`RangeDialog.qml`, backing both
    the clock-period and simulation-length prompts in `DolphinWindow.qml`) hardcodes the same
    ranges (2-1024, 2-2048, confirmed by reading it directly) but the clamping itself is stock
    `QtQuick.Controls` `Slider`/`SpinBox` framework behavior, not custom app logic -- unlike the
    Minimap gap below, there is no real custom algorithm here to leave untested, so this isn't
    treated as an open gap.
  - **`TestSceneUiBinder` — RESOLVED (2026-09-01), architecturally superseded, not a gap**:
    `SceneUiBinder` binds `Scene`'s signals to `MainWindowUi`'s QWidget menu/status-bar/element-
    editor components (`menuEdit->insertAction()`, etc.) -- Quick's chrome has no equivalent
    class at all, because the same signal relays (`icOpenRequested`, IC preview show/cancel,
    truth-table-requested, file-drop) are wired directly as declarative `Connections{}` blocks
    in `Main.qml` (confirmed present there). Nothing left to port.
  - **`TestElementContextMenu`'s two open tests — RESOLVED (2026-09-01), one real bug/gap found
    and closed**: `testRotateRightAction` needs no separate port -- `elementContextMenu`'s
    "Rotate right" `MenuItem` (`Main.qml`) just calls the same `AppController.rotateRight()`
    already covered by `TestCanvasCommands`/`TestCanvasItemInteraction`'s keyboard/direct-API
    tests; the QML wiring itself has no distinct logic to unit-test. `testICSubcircuitAction`
    uncovered a real, previously-untested-but-already-implemented feature:
    `CanvasItem::mouseDoubleClickEvent()` already special-cases `IC` (emitting
    `icOpenRequested()` to open its sub-circuit tab, confirmed by reading `CanvasItem.cpp`
    directly) instead of the labelable-element inline-edit path -- contradicting this progress
    file's own Phase 7 row, which had never been updated to reflect it (see corrected Phase 7
    row above). No context-menu-specific "Open sub-circuit" item exists (only double-click
    reaches it), so the literal menu-item test doesn't port, but the underlying feature did have
    zero test coverage until now: `TestCanvasItemInteraction::testDoubleClickICEmitsIcOpenRequested`
    (1 new test) closes that. Full 173-target Quick CTest suite green.
  - **`TestFileDialogProvider` — DONE (2026-09-02)**: the "needs a live `QQuickWindow`+
    `QQmlEngine`" framing from the earlier (2026-09-01) pass turned out to overstate the
    gap — `test_wiredpanda_quick` just needed a private `qt6_add_resources()` copy of
    `ChoiceDialog.qml`/`TextPromptDialog.qml`/`QuickFileDialog.qml` (none of them `import
    QuickShell`, so no module-registration machinery was needed), plus a minimal inline
    `import QtQuick.Window; Window {}` component for a real, shown `QQuickWindow`. New
    `Tests/QuickShell/TestQuickDialogProvider.cpp` (10 tests) drives a real
    `QuickDialogProvider` via its own `dialogOpened(QObject*)` test hook (the same technique
    Phase 7 already proved for `choice()`/`textPrompt()` via a since-removed
    `WP_QUICK_SELFTEST` block, never given a permanent test until now) — closing the 4
    literal `testReal*`-equivalent tests (`getOpenFileName`/`getSaveFileName`
    success+cancel), plus permanent coverage for `choice()`/`textPrompt()` themselves (a
    second, previously-untracked gap), plus 2 bonus tests for `getSaveFileName()`'s own
    dir-vs-full-path branch (no Widgets-side equivalent exists to have ever had a test for
    it). Found and worked around two real, non-obvious `QtQuick.Dialogs` behaviors while
    making this reliable, confirmed via `QT_LOGGING_RULES` tracing and reading Qt's own
    source rather than guessed: (1) `QQuickFileDialog::accept()` overwrites `selectedFiles`
    from the real platform helper's own internal selection, silently discarding anything set
    on the outer QML wrapper afterwards — a pre-selected file must be set *synchronously*
    inside the `dialogOpened()` handler itself (before `execModal()`'s `open()` call runs),
    not deferred alongside `accept()`; (2) `selectedNameFilter.name` is documented to strip
    everything from `'('` onward, so `getSaveFileName()`'s returned `selectedFilter` never
    contains the full `"Label (*.ext)"` string the Widgets-side `RealFileDialogProvider`
    equivalent returns for the same input — confirmed as a real, single-filter-call-site-only
    behavior (13 of 14 production call sites never read `.selectedFilter` at all; only
    `QuickDolphinController::saveAs()` does, and it passes a `;;`-joined *multi*-filter
    string `getSaveFileName()`'s own `nameFilters` handling was never designed for — flagged
    separately below, not fixed here, since it's Dolphin/Phase-12b scope, not this gap).
    `testFileDialogCreation` stayed out of scope (trivially true, already implied by every
    Quick test that calls `FileDialogs::provider()`). Full 174-target Quick CTest suite green.
    **Separately flagged, not fixed**: `QuickDialogProvider::getSaveFileName()`'s own
    `nameFilters` handling (`dialog->setProperty("nameFilters", QVariant::fromValue(QStringList{filter}))`)
    wraps its entire `filter` argument as a single list entry, which is correct for the 13
    single-filter call sites but wrong for `QuickDolphinController::saveAs()`'s `;;`-joined
    3-filter string (the Widgets `QFileDialog` convention) — `QtQuick.Dialogs`' `FileDialog.
    nameFilters` expects one filter string per list entry, not a `;;`-joined blob. The
    practical effect: `saveAs()`'s own `result.selectedFilter.contains("dolphin")` check
    (used to infer which extension to append to an extension-less typed filename) reads
    `extractName()`'s stripped name of whichever filter happens to be first in the
    mis-joined blob, not the filter the user actually picked — a real, reachable
    misclassification bug (a `.dolphin`-format extension could get silently mis-appended as
    `.csv` or vice versa). Confirmed via direct code + Qt-source reading, not fixed as part
    of this gap (out of scope for `TestFileDialogProvider`/Phase 13; belongs with
    `QuickDialogProvider`/`QuickDolphinController`, Phase 7/12b).
  - **`TestWorkspaceUnit` — DONE (2026-09-01)**: 18 of its 28 tests ported into
    `TestQuickWorkSpace.cpp` (version-warning/read-only/dialog-driven save/load/autosave paths
    not already covered by that class's existing 41 tests), confirmed 1:1 against
    `QuickWorkSpace::save()`/`load()`/`autosave()` by reading `QuickWorkSpace.cpp` directly
    (`Dialogs::provider()->choice()` standing in for `QMessageBox`). Found and fixed a real,
    previously-unknown production bug along the way: `QuickWorkSpace::autosave()` was missing
    `WorkSpace::autosave()`'s "don't write into the bundled Examples directory" guard
    (`InstallRelativePaths::isCandidate("Examples", ...)`) entirely — autosaving a circuit
    opened from a writable dev-checkout `Examples/` directory would have written a hidden
    `.name.uuid.panda` file straight into it (exactly the file shape `.gitignore` explicitly
    un-ignores to catch). Fixed by adding the same guard. The remaining 10 of 28 were not
    ported, each for a specific documented reason: 5 trivial non-null-accessor smoke tests
    (`testWorkspaceCreation`/`testTabManagement`/`testModificationTracking`/`testICEditing`/
    `testFullScreenHandling`) with no behavior of their own, already implied by every other
    test in the file; `testResizeRepositionsVisibleExerciseOverlay` is architecturally
    superseded (`ExerciseOverlay.qml` positions itself via declarative `anchors.fill: parent`
    + margins, so there is no imperative C++ resize-repositioning logic left to test). Of the
    original 6 minimap-geometry tests: `testOnMinimapGeometryChangeFinishedPersistsSettings`
    was already covered verbatim by `TestQuickMinimap::testGeometryPersistsViaSettings()`;
    `testMinimapDefaultPositionWithoutPersistedGeometry` and
    `testMinimapIgnoresPreShowResizeThenRestoresOnShow` are architecturally superseded (a
    plain declarative binding and a QWidget-specific show/resize-event race, same disposition
    class as `testResizeRepositionsVisibleExerciseOverlay` above). **The remaining 3 — DONE
    (2026-09-02)**: rather than needing a new QML-level test harness (the framing from the
    earlier 2026-09-01 pass), `Minimap.qml`'s `clampToParent()` arithmetic was extracted into
    a new, pure `Q_INVOKABLE QuickMinimap::clampGeometry(candidate, parentWidth,
    parentHeight)` (mirrors `WorkSpace::applyMinimapGeometry()`'s shared clamp block, using
    `qreal`/`qBound<qreal>` throughout — not `WorkSpace`'s `int` — to match
    `Item::x/y/width/height`'s own type, and preserving the same
    width/height-clamped-before-x/y-clamped ordering both rely on) — plain deterministic
    geometry math has no interactivity of its own, unlike the mouse-drag resize/move math
    `Minimap.qml` legitimately keeps. `Minimap.qml`'s `clampToParent()` now just calls it and
    assigns the result, behavior-preserving. 3 new tests added directly to the existing
    `Tests/QuickShell/TestQuickMinimap.cpp` (`testClampGeometryReturnsUnchangedWhenAlreadyWithinBounds`/
    `testClampGeometryReclampsWhenParentShrinks`/`testClampGeometryClampsOversizedCandidate`),
    hand-verified numeric scenarios mirroring the 3 original Widgets tests' exact shapes, all
    passing on first run. Full 174-target Quick CTest suite green. Local `ScopedTinyFsizeLimit`
    duplicated into the test file rather than
    reused from `Tests/Common/TestUtils.h`, which also pulls in `App/Scene/Scene.h` (Widgets-only,
    and stale against the current `SimulationHost` interface — confirmed the hard way:
    including it broke the `wiredpanda_quick` test build). Full 173-target Quick CTest suite
    green; `TestQuickWorkSpace` alone: 59/59.
  - **`TestSceneUndoredo`'s align/distribute methods — DONE (2026-09-01)**: 10 tests ported
    into `Tests/QuickShell/TestCanvasCommands.cpp` (same file already covering
    move/rotate/flip/morph/split as direct API calls, not event-driven — the same shape as
    align/distribute), against `CanvasItem::alignLeft()`/etc. directly. All eight methods
    confirmed 1:1 with `Scene::alignLeft()`/etc. (same `sceneBoundingRect()`-based edge/center
    math, reusing `moveElementsTo()`/`CanvasMoveCommand`, no new command class) by reading
    `CanvasItem.cpp` directly. All 10 tests passed on the first build/run; full 173-target
    Quick CTest suite green.

All of the above was assigned to Phase 4 or Phase 13 (both rows amended above to say so) rather
than left unscheduled. **Both are now fully DONE (2026-09-02)**: Phase 4's pure-domain bucket
closed 2026-08-31; Phase 13's full residual scope (MCP handler parity, Exercise/Tour Quick
controller tests, `QuickWorkSpace`, the `TestScene`/`TestSceneState` audit, and finally
`TestFileDialogProvider`/`TestWorkspaceUnit`'s minimap-clamp tests) closed across 2026-08-31
through 2026-09-02 — see each row's own detail section above for what was actually done versus
explicitly, permanently deferred (IC file-watching/self-containment/rename-propagation; the
`QuickDolphinController::saveAs()` multi-filter `selectedFilter` bug flagged but not fixed under
`TestFileDialogProvider` above). Phase 14 (engine perf hardening) closed 2026-09-02 (see its own
detail section below). Only Phase 15 (real stock-Fusion pass) remains TODO in the overall plan.

## Phase 14 detail (DONE)

Direct verification against v2's current code (not assumed from the plan's own phase-table
parenthetical or from memory) found the real scope was **6 items, not 8**:

- **`appearanceKeyFor()`'s `QString::arg()` removal — already done.** Front-loaded during
  Phase 5 specifically to avoid this exact redo. Confirmed via direct grep before starting.
- **Display7/14/16 classification memoization — moot, not just superseded.** The original
  commit that added this was itself made obsolete two commits later in the same source
  history (the port-status dirty-flag commit below removed it as "now-unnecessary"); v2's
  `ElementRenderCache` never had a separate Display-family field to begin with, since its
  per-port segment status already rode the same generic fingerprint every other element used.
  Skipped entirely; the dirty-flag item was written directly against its final, already-generic
  shape.

**6 commits, one per item, each independently built/tested (full 174-target Quick CTest suite
green after every one) before the next started**, per the plan's own file-collision caution
(`CanvasItem.{h,cpp}` is touched by items 1/3/4/5/6):

1. **Minimap thumbnail caching** (`77fc7f2f1`) — `renderMinimapImage()` gained an image+content-
   rect cache, skipped whenever the current viewport still fits inside what's cached, invalidated
   by `rebuildSpatialIndex()`. Above `kMinimapSimplifiedThreshold` elements, a real rebuild falls
   back to flat rects/lines. Threshold re-measured directly against this branch's own fixtures
   (not the original branch's number, copied unverified) via a temporary `QElapsedTimer` probe,
   removed before commit: v2's cost curve (~25ms@1000 through ~358ms@20000 elements, debug build)
   is roughly linear with no severe blowup, unlike the original branch's own measurement — kept
   the threshold at 2000 anyway as a deliberately conservative choice, not because the numbers
   demanded it. 3 new `TestCanvasItemSmoke` tests via a new `m_minimapRebuildCount` counter
   (the first `friend class` declaration in `CanvasItem.h`).
   **Found, documented, not fixed** (pre-existing, not introduced by this item): the minimap
   thumbnail already didn't refresh its colors on a bare theme switch before this item existed
   either (no regen trigger for `updateTheme()`) — adding a cache doesn't make this any worse.
2. **`Port::attachConnection()`/`detachConnection()` cascade fix** (`9f3f67599`) — split
   `updateConnections()`'s position-refresh loop from its status/validity tail
   (`revalidateStatus()`); attach/detach/`setRequired()` now call only the latter, since none of
   them move the port or its siblings. Verified safe by tracing `Connection::setStartPort()`/
   `setEndPort()` (which set a newly-attached connection's own position directly) and both the
   plain-file-load and IC-loading paths.
3. **Port-status dirty flag** (`1a42f0b06`) — replaced `updatePaintNode()`'s per-repaint
   `(status, isPortHidden)` fingerprint rebuild with `GraphicElement::isRenderDirty()`, pushed by
   `InputPort`/`OutputPort::setStatus()` on a genuine *final-value* change (not just a requested
   one — the `isValid()`-override edge case, where a required/unconnected `InputPort` asked to
   go Active stays Error, must not mark dirty; regression-tested explicitly). `ElementRenderCache`
   gained explicit `inputSize`/`outputSize` fields: the removed fingerprint's vector length was
   the only thing implicitly catching a `ChangePortSize` edit before, so dropping it without a
   replacement would have silently broken render-cache invalidation on port-count changes — found
   during planning by pulling the source commit's own full diff, not left to be discovered as a
   live bug. Show Gates/Wires' cache invalidation (a v2-only feature the fingerprint used to cover
   reactively) moved to an explicit `m_elementRenderCache.clear()` in `setGatesVisible()`/
   `setWiresVisible()`.
4. **`ElementPorts::allPorts()` removal** (`6fcc503bf`) — found **13 real call sites**, not the
   9-10 estimated during planning: an initial grep pattern (`x.allPorts()`/`x->allPorts()`) missed
   bare `allPorts()` calls from within `GraphicElement`'s own methods (`portsBoundingRect()`,
   `setPos()`, `highlight()`) and `IC::isCursorOverPort()`. A clean build after converting every
   site found by a corrected full-repo grep (`grep allPorts()`, no receiver pattern) confirmed
   none were missed — deleting the method would have failed to compile otherwise. Multi-line
   bodies got a small local lambda called from both `inputs()`/`outputs()` loops; one-line bodies
   were duplicated directly. `IC::isCursorOverPort()`'s `std::any_of` split into two calls (one
   per port vector).
5. **Deadline-based simulation scheduling** (`41e6b18d5`) — the biggest item.
   `Clock::nextDeadline()`, `Simulation::wakeSoon()`/`rescheduleTimer()`, a single-shot
   `m_timer` retargeted to the soonest clock deadline (or stopped entirely with nothing to wait
   for) at both of `update()`'s exit paths and in `start()`. `wakeSoon()` is called **once**,
   internally, from `restart()` — covering all of its callers for free — plus 6 real
   interactive-input sites in `CanvasItem` (mouse clicks, keyboard triggers) whose downstream
   propagation would otherwise wait for the next clock deadline or forever with none.
   `isRunning()` and **two** `m_timer.isActive()` sites (not one) switched to an explicit
   `m_running` flag: `update()`'s clock-advance gate (present in the source), and `stop()`'s
   `m_pausedAt`-recording guard for the pause/resume phase-shift feature — a v2-only addition
   genuinely absent from the original commit's own diff, found only by checking every
   `m_timer.isActive()` site in v2 rather than trusting the source's change list as exhaustive;
   left unconverted, a `stop()` landing between deadlines would have silently skipped recording
   the pause and made the next `start()` incorrectly `resetClock()` instead of `shiftClock()`.
   Verified via `/proc` polling (not GDB, which distorts a hot loop): a clock-free circuit's CPU
   drops to ~0% after settling (the timer stops entirely) where the old fixed-1ms timer would
   have kept waking 1000×/sec forever; `clocked_8000.panda`'s ~86% CPU is legitimate real work
   from 7200 actively-toggling flip-flops, not a scheduling regression.
   **Found, documented, not fixed** (inherited from the original branch's own shipped design):
   `QuickElementEditor::apply()`'s editor-driven input toggle only calls `rescheduleTimer()`,
   never `wakeSoon()`/`update()` — in a zero-clock circuit, propagation beyond the toggled
   element's immediate neighbors could stall indefinitely. `QuickElementHandler::
   handleSetInputValue()`'s MCP path already sidesteps this by calling `update()` directly.
6. **`CanvasItem::addItem()` O(n²) scan fix** (`02355ee6c`) — `QSet<GraphicElement*>`/
   `QSet<Connection*>` as O(1) membership companions to `m_elements`/`m_connections`, kept in
   lockstep at all 4 add/remove sites (confirmed to be all of them via a full-file grep for any
   direct mutation of either vector).

**Verification discipline**: every item's own commit built and ran the full 174-target Quick
CTest suite clean before the next item started (not batched), matching this plan's own
file-collision caution for `CanvasItem.{h,cpp}`. Design was adversarially validated across 6
passes before implementation began (see the plan file this phase executed,
`~/.claude/plans/before-doing-any-of-wondrous-music.md`, for the full validation record) — one
real regression risk (item 3's `inputSize`/`outputSize`) and one real design error (item 5's
`wakeSoon()` placement) were found and fixed before any code was written, plus one genuinely new
v2-only bug (item 5's `stop()` guard) neither the plan nor the source commit had anticipated
until checked directly.

## Post-Phase-14 profiling pass (2026-09-02)

A follow-up profiling session on `clocked_8000.panda` (8001 elements, 7200 clocked flip-flops;
`perf record -F 500 --call-graph dwarf` against `wiredpanda_quick`) went past Phase 14's own
scope, root-causing the top 5 remaining self-time hotspots via `perf annotate`
(instruction-level, not just call-graph text) rather than accepting them at face value. Two were
real, fixable bugs — **landed**, each its own commit, full 174-target Quick CTest suite green
after both:

- **`Connection::applyStatusPen()`/`Port::updateTheme()` rebuilt a fresh `QPen`/`QBrush` from a
  `QColor` on every status change** (`67d526e80`) — root-caused via a GDB breakpoint on
  `QPen::detach()` after an initial partial fix (Connection alone) made things *worse*, not
  better; Qt 6.9's `QPen::operator=(QColor)` unconditionally detaches. Both classes now
  precompute the 4 status pens/brushes once per theme refresh.
- **`QQmlApplicationEngine`'s auto-attached `QQmlFileSelector` checked every asset load against
  platform/locale selector variants** (`4fdff4f16`) this project has never shipped and never
  will (Exercise/Tour and UI strings both go through their own catalog mechanisms, not
  per-locale/per-platform asset variants) — fixed via `QT_NO_BUILTIN_SELECTORS=1` in `Main.cpp`,
  verified empirically (`selectionHelper` no longer appears in the profile at all).

The remaining 3 hotspots (`Simulation::update()`'s topological sweep, `Connection::setStatus()`'s
`endPort()` dispatch, `ElementSimState::updateInputs()`'s un-inlined `Port::connections()` call)
are **not bugs** — root-caused to (a) cache misses from iterating ~8000 individually
heap-allocated, heterogeneous `GraphicElement*`/`Port*`, and (b) a trivial accessor landing in a
different Unity translation unit than its hot caller. A full plan to address these — inlining 3
cross-TU accessors, marking provably-leaf classes `final`, a sized-bucketed pool allocator for
`Port`/`Connection`/`GraphicElement`, and software prefetching in `Simulation::update()`'s hot
loops — was designed and adversarially validated (6 passes) but **not yet executed**; it lives at
`~/.claude/plans/before-doing-any-of-wondrous-music.md` for whenever this is picked back up.
That plan's own "Future follow-ups" section also flags the most complete possible fix (a
handle/ID-based object-model rewrite so memory layout can actually match topological iteration
order) as a deliberately out-of-scope, multi-week undertaking of its own — not something to
attempt as part of a targeted perf pass.

## Notes

- Toolchain: Qt 6.9.3 (verified present), Ninja generator, `cmake --preset debug` /
  `ctest --preset debug`.
- `App/QuickShell/Canvas/CanvasItem.{h,cpp}` was flagged in the plan as the highest cross-phase
  file-collision risk — Phase 6 confirmed this in practice: every sub-step's commit needed the
  original's `QGraphicsItem*`-era signatures re-derived against `ItemWithId*` (Phase 2 already
  removed that inheritance), and the id/registry sub-step's own doc comment had explicitly
  pre-anticipated exactly this hand-off point ("that belongs to the commands phase that follows
  this one") before Phase 6 started.
- `App/Main.cpp` / `App/UI/*` / `App/Scene/*` (Widgets-only) stay on disk, unreferenced, until the
  Phase 15 rename commit — matches the original branch's own end state.
- **Known, tracked style-rule gap**: `QuickElementEditor::colorOptions()`/`morphCandidates()`
  (Phase 8) use `QVariantList`-of-`QVariantMap`, not the plan's standing `Q_GADGET`/
  `QML_VALUE_TYPE` authoring rule for list model data — predates that rule being consistently
  applied (Phase 11's `TruthTableRow`/`AppearanceStateOption` do follow it). Not retroactively
  fixed (out of scope for the phases that touched this file since); worth a small follow-up pass
  if a later phase touches `QuickElementEditor` again.
- **`WIDGETS_ONLY_SOURCES`/`SOURCES` split, and what it means for future ports**: as of Phase
  12a, `CMakeSources.cmake` cleanly separates Widgets-free code (linked into `wiredpanda_lib`,
  reachable from `wiredpanda_quick` via `link_wiredpanda_whole_archive`) from genuinely
  Scene/MainWindow-coupled code (`WIDGETS_ONLY_SOURCES`, `wiredpanda`/`test_wiredpanda` only).
  Any future port of a class currently filed under `WIDGETS_ONLY_SOURCES` needs the same check
  Phase 12a had to do the hard way: read the `.cpp`'s own `#include`s for `Scene.h`/
  `Workspace.h`/`MainWindow.h`/anything Widgets, and if only *some* of its methods actually need
  them, split those into a sibling `*Scene.cpp` file (kept `WIDGETS_ONLY_SOURCES`) so the rest
  can move to shared `SOURCES` — don't assume "it's in `WIDGETS_ONLY_SOURCES`" means the whole
  file is actually Widgets-coupled. `BaseHandler`/`MCPProcessor`/`ExerciseEngine` are now split
  this way; `ConnectionHandler`/`ElementHandler`/`FileHandler`/`HistoryHandler`/`ICHandler`/
  `SimulationHandler` (the remaining Widgets-side MCP handlers) were NOT split since Quick has
  its own complete `Quick*Handler` replacements for all six — there was nothing to reuse from
  them.

## Phase 16 detail (DONE, 2026-09-03)

User-directed ("remove all remaining Widgets code"), ahead of Phase 15's own stock-Fusion pass
-- explicitly not blocked on it since a sibling `wiredpanda` worktree stays available as the
Widgets visual reference this phase would otherwise have needed. Two sequential asks in the
same session: first the removal itself, then (once packaging turned up as a casualty) renaming
`wiredpanda_quick`/`test_wiredpanda_quick` back to `wiredpanda`/`test_wiredpanda` -- matching
this file's own long-standing framing of that rename as "the original branch's own end state."

**Removed**: every file listed in `WIDGETS_ONLY_SOURCES`/`WIDGETS_ONLY_HEADERS` and
`TEST_WIREDPANDA_SOURCES`/`TEST_WIREDPANDA_HEADERS` (547 files: `App/UI`, `App/Scene`, the
Widgets-only slices of `App/BeWavedDolphin`/`App/Element`/`App/Exercise`/`App/Tour`/`MCP/Server`,
`App/Main.cpp`, and the entire Widgets-only Cpu/Integration/Resources/Runners/System/
TestsWithoutPanda/Unit test-source lists under `Tests/`), plus 4 Widgets-only `Tests/Common/`
files, minus the `WIREDPANDA_BUILD_WIDGETS_APP`-gated ~750-line CMake block that built them
(`wiredpanda`/`test_wiredpanda` targets, their `add_test()` entries, install/packaging). Fixture
data (`Tests/Integration/IC/Components/*.panda`, `SystemVerilog/*.sv`, `Arduino/*.ino`,
`Generators/*.py`) was deliberately *not* touched -- confirmed still live runtime dependencies
of the Quick test suite (`Tests/QuickShell/IC/QuickTestUtils.h`'s `CURRENTDIR`-relative paths).

**3 real Widgets-only-but-still-needed symbols** had to move rather than just disappear (found
by rebuilding after each deletion batch, not assumed complete beforehand):
`StdinReader`/`kMaxStdinLineBytes`/`extractStdinLines()` (moved from the deleted `MCPProcessor`
onto `QuickMCPProcessor`, which already called into them across the Widgets/Quick divide) and
`CircuitExporter::kMaxImageDimension` (moved onto `CanvasItem` as a public
`static constexpr`, since `CanvasItem::renderExportImage()` and
`TestQuickFileHandlerSecurity` both needed the same cap `CircuitExporter.h` used to provide).

**4 `TestDanglingPointer` regression guards were pure source-text checks against the
now-deleted Widgets files** (`App/Scene/Commands.cpp`/`ConnectionManager.cpp`/`ICRegistry.cpp`,
already flagged in their own doc comments as "still present in the tree" pending exactly this
removal). All 4 redirected cleanly to their real `Canvas*`-prefixed Quick equivalents. The
first 3 (bug6/bug7/hardening_icRegistryReloadHelpers) confirmed each already carries the same
`SimulationBlocker`/`reloadTargetsAtomically`/`restartSimulation` protection the original bug
fix required. `hardening_deleteEditedConnectionMustUseSimulationBlocker`
(`ConnectionManager::deleteEditedConnection`'s Quick analog,
`CanvasItem::cancelEditedWire()`) initially had no settled equivalent -- `cancelEditedWire()`
opens no `SimulationBlocker` -- and was dropped rather than silently weakened, flagged as an
open question. **Resolved 2026-09-03**: the sibling `wiredpanda-quick` worktree's own,
independent recreation of this exact test (a different branch, not an ancestor of this one --
`git merge-base --is-ancestor` confirmed) had already answered this behaviorally rather than by
source-text check: `m_editedConnection` is never registered via `addItem()`/`simulationItems()`
until the wire completes (confirmed by reading `startWireFromOutput()`/`cancelEditedWire()`
directly), so there is nothing for the simulation timer to dangle a reference to --
structurally safe by construction, not merely untested. Ported that sibling branch's real
behavioral test (start a wire, cancel it, verify the simulation survives and a real connection
still propagates) rather than re-deriving it from scratch; needed one new `friend class
TestDanglingPointer;` on `CanvasItem` to reach the private wire-drag API directly. 17/17
`TestDanglingPointer` tests green (up from 16).

**Rename**: `wiredpanda_quick`→`wiredpanda`, `test_wiredpanda_quick`→`test_wiredpanda`
(`App/QuickShell/CMakeLists.txt`, root `CMakeLists.txt`), `TEST_WIREDPANDA_QUICK_SOURCES/HEADERS`
→`TEST_WIREDPANDA_SOURCES/HEADERS` (`CMakeSources.cmake`), and all ~175 `add_test()` NAME entries
that carried a `Quick` suffix for Widgets-suite disambiguation (`TestFooQuick`→`TestFoo`) --
~24 already-bare names (Quick-only classes like `TestQuickAppController`) were untouched. Every
prose mention of the old names across comments/doc-comments was swept too (`sed`, ~30 files),
not just the functional CMake identifiers.

**Packaging casualty, found and restored**: the only `install()`/packaging code in the whole
repo (desktop entry, FreeDesktop icons, shared-mime-info, macOS bundle `Info.plist`, Windows
`windeployqt`/VERSIONINFO) lived entirely inside the deleted `WIREDPANDA_BUILD_WIDGETS_APP`
block, hardcoded to the `wiredpanda` (Widgets) target. Moved into
`App/QuickShell/CMakeLists.txt`, re-targeted at the renamed Quick `wiredpanda` executable --
none of the resource files themselves (`.desktop`'s `Exec=wiredpanda`, `Info.plist.in`'s
`CFBundleExecutable`) needed edits, since they already hardcoded the plain `wiredpanda` name.
Also restored (found missing, not just moved): `update_translations`'s Exercise/Tour catalog
regeneration wiring (`Scripts/generate_exercise_tour_catalog.py` via `update_exercise_tour_catalog`)
had *only* ever existed inside the same doomed block, meaning `update_translations` never
actually regenerated that catalog on this branch at all until now, despite CLAUDE.md's own i18n
section documenting it as already wired in. `copy_mcp_schema_quick`/`schema-mcp-quick.stamp`
renamed to `copy_mcp_schema`/`schema-mcp.stamp` (no longer a second, disambiguated copy of a
still-shipping `copy_mcp_schema` -- the Widgets one it needed disambiguating from is gone).
Verified via `cmake --install build --prefix <tmp>` (binary, Examples, `.desktop`, all 5 icon
sizes, mime XML all land correctly) and by launching the installed binary and screenshotting it.

**Also cleaned, not by request but to avoid leaving dead references**: `.github/workflows/
build.yml`/`sanitizers.yml`/`coverage.yml`'s `-L gui`/`-LE gui` `ctest` label filters and their
paired "Test (GUI, ...)" steps (the Widgets-only `ALL_TESTS`/`GUI_TESTS`/`LABELS "gui"` block
they depended on no longer exists -- these steps were already matching zero tests before today,
since `WIREDPANDA_BUILD_WIDGETS_APP` was already `OFF` everywhere in CI; not a regression this
phase introduced). Crash-diagnostic steps repointed at `test_wiredpanda` instead of the deleted
`test_wiredpanda`(Widgets)/`test_wiredpanda_quick` names.

**Follow-up pass, 2026-09-03 -- every item below closed except one new finding:**

- **`Tests/Fuzz/*.cpp` (all 12 libFuzzer harnesses) -- FIXED.** The Widgets-removal commit had
  also accidentally deleted the entire `ENABLE_FUZZER` CMake block (physically nested inside the
  deleted line range, despite being logically independent). Restored it, then discovered
  building with `-DENABLE_FUZZER=ON -DCMAKE_CXX_COMPILER=clang++` that all 12 harnesses --not
  just the 9 using `Scene.h`/`Workspace.h`/`Commands.h`/`ICRegistry.h`-- were already broken
  independent of the CMake accident: every harness constructs a real `QApplication`, and
  `wiredpanda_lib` was never linked against `Qt6::Widgets`. 3 needed only
  `QApplication`→`QGuiApplication`; the other 9 also needed porting to
  `CanvasItem`/`QuickWorkSpace`/`CanvasCommands`/`CanvasICRegistry` (a new
  `add_quick_fuzzer_harness()` macro compiles `QUICK_SHELL_SOURCES` directly into these targets).
  Making `fuzz_undo` buildable for the first time surfaced a real, LeakSanitizer-confirmed leak
  (`QUndoStack::push()` calls `cmd->redo()` *before* taking ownership, so a command whose
  `redo()` throws leaks) -- confirmed the identical shape already existed unchanged in the
  deleted Widgets `RegisterBlobCommand::redo()`, so a pre-existing exposure the port inherited,
  not introduced; hardened the harness's own push sites for the two commands with
  side-effect-free constructors. All 12 harnesses verified: clean build, and a real 10-20s
  fuzzing run each (`LSAN_OPTIONS=suppressions=Tests/lsan_suppressions.txt`) with no
  crashes/leaks and real coverage growth. See `fix(fuzz)` commit.
- **Debian i386 CI job missing `qt6-declarative-dev` -- FIXED**, verified via a real
  `i386/debian:trixie` Docker container (the job's own image): that one package supplies every
  needed CMake config file (`Qt6Quick`/`Qt6QuickControls2`/`Qt6QuickDialogs2` all ship under it
  on Debian), confirmed by a full clean configure + build inside the container, not a
  package-list guess. **New finding, NOT fixed**: a full `ctest` run in that same container
  found 5 of 175 tests (`TestElementAppearance`, `TestExerciseTourResources`,
  `TestGraphicElementSerializer`, `TestInputRotary`, `TestICFixtureLayout`) fail specifically on
  i386 while passing on 64-bit -- a genuine 32-bit-specific issue (likely pointer-size or
  endianness-sensitive) needing its own dedicated root-cause investigation, unrelated to the
  missing-package problem this pass fixed. See `fix(ci)` commit.
- **`.github/workflows/deploy.yml` (release/packaging pipeline) -- structurally fixed by the
  rename, still not independently verified end-to-end.** It never set
  `WIREDPANDA_BUILD_WIDGETS_APP=ON` anywhere, yet already assumed a `wiredpanda`-named installed
  binary with `.desktop`/icons/mime files (confirmed by reading its own smoke-test assertions)
  -- meaning release packaging was silently non-functional (installing nothing usable) for this
  branch's entire prior existence. The `wiredpanda_quick`→`wiredpanda` rename + restored
  `install()` is what actually makes it structurally correct again; no AppImage/DMG/MSI build
  was attempted, so this is not yet independently verified end-to-end.
- **`TestDanglingPointer`'s dropped `hardening_deleteEditedConnectionMustUseSimulationBlocker`
  -- RESOLVED**, see the `TestDanglingPointer` paragraph above.
