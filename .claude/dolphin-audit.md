# beWavedDolphin codebase audit — refactor / improvement opportunities

## Context

Audit of `App/BeWavedDolphin/` (≈2,600 LOC across 12 files) requested because the
module feels messy — notably its use of a `QTableView` embedded in a
`QGraphicsScene`. This is a findings list, not an implementation plan. Items are
grouped by theme and roughly ordered by impact. No fixes have been applied yet.

Scope read in full: all 12 `App/BeWavedDolphin/` files (`BeWavedDolphin.{cpp,h}`
1409/242, `BeWavedDolphinUI.{cpp,h}`, `Serializer.{cpp,h}`, `SignalDelegate.{cpp,h}`,
`SignalModel.{cpp,h}`, `WaveformView.{cpp,h}`); the dialogs it drives
(`App/UI/ClockDialog.{cpp,h}`, `App/UI/LengthDialog.{cpp,h}`); its lifecycle/creation
sites (`App/UI/MainWindow.cpp:1488-1497,1824-1826`); the shared `GraphicsView` base;
`Serialization::{read,write}DolphinHeader` and `loadDolphinFileName`; the `Workspace`
dolphin-filename plumbing; `Settings::dolphinGeometry`; `SimulationBlocker` /
`SimulationThrottleDisabler`; and every dolphin-referencing test/fuzz file
(`TestBewavedDolphinGui` 35 cases, `TestWaveform`, `TestDolphinSerializer`,
`TestWorkspaceFileops`, `TestSerialization`, `TestSettings`, `TestMainWindowGui`,
and the 11 Fuzz harnesses).

## 1. Headline: QTableView-inside-QGraphicsScene + the zoom system (the core mess)

The table is a real `QTableView` reparented into a `QGraphicsScene` via
`m_scene->addWidget()` (`BeWavedDolphin.cpp:72`), shown through a `WaveformView`
(a `QGraphicsView`). The scene/proxy exists almost entirely to enable one
feature — Fit Screen's smooth transform scaling — and it imposes a high tax:

- **Three different, conflicting zoom mechanisms** that must be kept mentally in sync:
  - Discrete Zoom In/Out (`on_actionZoomIn/Out_triggered`): leaves the view transform
    at identity and instead *resizes columns* (`adjustColumnWidths`, ×/÷ `kZoomStep`);
    `m_scale` stays `1.0`.
  - Fit Screen (`on_actionFitScreen_triggered`): applies an *absolute* view transform
    (`setTransform`) **and** sets `m_scale` **and** inversely resizes the inner table
    (`setTableViewSize` divides widget size by `m_scale`).
  - Reset Zoom (`on_actionResetZoom_triggered`): resets the transform to identity but
    sets `m_scale = kZoomStep` (i.e. 1.25, not 1.0) — a value that contradicts the
    member's own doc comment ("Mirrors the view's accumulated scale transform").
- **`m_scale` conflates two unrelated ideas** (the view transform factor *and* an
  inner-table counter-sizing factor). The quirk is now frozen into a regression test
  (`testZoomScaleTrackingA26` asserts `m_scale == kZoomStep` while `transform.m11() == 1.0`),
  so the confusion is load-bearing rather than incidental.
- **No scrollbars** — both policies are forced `AlwaysOff` (`:76-77`). A waveform longer
  than the window can't be scrolled, only zoomed/fit. A plain table gives this for free.
- **Manual scene-rect bookkeeping** on every resize (`resizeScene` → `setSceneRect(itemsBoundingRect)`),
  plus the `-2px` / `+10px` / `-64px` magic offsets scattered around (`:469`, `:875`, `:1380`).

**Direction:** make the `QTableView` (or a custom-painted `QAbstractScrollArea`) the
central widget directly, with native scrollbars. Implement zoom by scaling the
delegate/row-column metrics (already half-done — discrete zoom does exactly this).
Then delete the `QGraphicsScene`, the proxy, `WaveformView`, and `m_scale`. Export
(see §6) renders the model+delegate offscreen, so it no longer needs the scene either.
This is the big win and removes the most code.

## 2. Leaky abstraction: dolphin zoom plumbing lives in the shared GraphicsView base

`WaveformView` (used only by dolphin) relies on `m_redirectZoom`, `m_zoomLevel`,
`setRedirectZoom()`, and the `scaleIn()/scaleOut()` signals **declared on the base
`GraphicsView`** (`App/Scene/GraphicsView.h:57-91`) — the same class that backs the
main circuit canvas. So a dolphin-specific concern (redirect wheel-zoom to the parent
so it can resize columns) is baked into a general-purpose base class. If §1 is done,
this plumbing can be pulled out of `GraphicsView` entirely.

Worse: `WaveformView` re-declares `zoomIn/zoomOut/resetZoom/canZoomIn/canZoomOut`, which
**non-virtually shadow** the base `GraphicsView` methods of the same names but with
*different semantics* (base scales the view transform; `WaveformView` just bumps a 0–6
counter and re-flows columns). Calling them through a `GraphicsView*` would silently run
the wrong implementation — a latent footgun masked only because callers use the concrete type.

## 3. `BewavedDolphin` is a god-class (1409 LOC) mixing ~6 responsibilities

A single `QMainWindow` subclass owns: model/table lifecycle, **driving the simulation**
sweep, file-IO orchestration, clipboard copy/cut/paste, PNG/PDF/txt/print export, the
zoom/view system, and all menu/toolbar slots. Natural seams to extract:

- **A simulation-sweep driver** (`run()`, `restoreInputs()`, `loadSignals()` snapshot,
  `resetSimState` loop) — see §4; it's the part with the deepest circuit coupling and
  the part most worth unit-testing in isolation.
- **A clipboard helper** for `copy/cut/paste` + `sectionFirstRow/Column` (`:670-1055`).
- **Export** (`exportToPng`, `exportWaveformToPng`, PDF slot, `saveToTxt`, `print`) — see §6.

This also keeps the view/UI class free of non-UI logic.

## 4. Simulation-driving coupling and special-cases in `run()`

`run()` (`:377-437`) reaches across the boundary into circuit internals: it iterates
`m_externalScene->elements()`, calls `resetSimState()` on each, pokes input ports with
`setOn()`, calls `m_simulation->update()` per column, and reads back `inputPort()->status()`.
Embedded smells:

- **Rotary special-case** inline (`isRotary` branch, `:403-412`) — `qobject_cast<InputRotary*>`
  per input per column. Belongs behind a polymorphic "apply value to port" on the input element.
- **`restoreInputs()` relies on a documented `setOn` toggle subtlety** (single-port
  `setOn(bool)` vs multi-port `setOn(value, port)`); see the known-gotcha that
  `InputSwitch::setOff()`/no-arg `setOn()` *toggle*. Fragile; worth a single explicit
  "set port to level" API.
- The reset-all-elements loop duplicates the `type() == GraphicElement::Type` filter that
  also appears in `loadElements()`.

## 4b. Dialogs: semantic mismatch + side-effect getters

- **Period/frequency confusion across files.** `on_actionSetClockWave_triggered`
  (`:712-722`) does `ClockDialog dialog(m_clockPeriod, ...)` then
  `clockPeriod = dialog.frequency()` and uses it as a **period** (`halfClockPeriod =
  clockPeriod / 2`, modulo over columns). But `ClockDialog`'s UI is titled "Clock
  Frequency Selection" with a `frequencySpinBox` and `frequency()` returns Hz. The same
  integer means "frequency" in the dialog and "period (columns)" in dolphin — genuinely
  wrong labeling, confusing for users and maintainers.
- **Side-effect in a getter.** `ClockDialog::frequency()` and `LengthDialog::length()`
  call `exec()` internally and return `-1` on cancel. So a const-looking accessor actually
  pops a modal dialog and blocks. Rename to `exec()`-style (`promptFor…()`) or split
  show-vs-read.

## 5. Duplication and dead/odd indirection

- **`load(QDataStream&)` vs `load(QFile&)`** (`:1241-1269`) are byte-for-byte identical
  except the serializer call — collapse into one templated/lambda helper.
- **`on_actionShowNumbers` vs `on_actionShowWaveforms`** (`:1271-1316`) share the same
  re-apply-all-input-cells-then-`run()` body; only the mode flag + clear-loop differ.
- **`createElement()` is a pure pass-through to `setCellValue()`** (`:520-523`) with
  identical args — exists only for MCP naming. Either rename `setCellValue` and drop one,
  or document why both survive.
- **`copy()` / `cut()`**: `cut()` just calls `copy()` then `on_actionSetTo0_triggered()`,
  which re-runs the whole sim — fine, but the naming hides that cut mutates + simulates.
- `sectionFirstRow/Column` are two copies of the same min-over-ranges scan.

## 6. Export mutates live UI state

`exportToPng()` (`:612-630`) resizes the **on-screen** inner table and resets the scene
rect as a side effect of exporting, so exporting visibly disturbs the view. `saveToTxt()`
(`:581-610`) is worse: it calls `on_actionCombinational_triggered()`, **destroying the
user's current waveform** (forces combinational mode, sets `m_edited`, re-runs) just to
produce text output. Export/serialize paths should render/read from the model into a
throwaway target without touching live widgets or user data.

## 7. Smaller correctness / consistency / hygiene nits

- **Magic `2048` hardcoded** in `Serializer.cpp` column checks (`:41`, `:103`) instead of a
  shared `kMaxSimLength`; and the min column count disagrees across entry points
  (serializer demands `>= 2`, `loadFromTerminal` allows `>= 1`).
- **`print()` uses `std::cout`/`<iostream>`** (`:565-579`) in an otherwise all-Qt module;
  prefer `QTextStream`. Also dereferences `m_model->item(row,col)` without null-checks.
- **Per-cell `setData` storm**: every edit path sets 2–4 roles per cell and many call
  `run()` afterward; with the proxy-in-scene each `setData` repaints through the graphics
  proxy. For long sweeps (up to 2048 cols) this is O(rows·cols) repaints. Batch updates
  / block signals during bulk fills.
- **`m_model` is re-`new`ed in `loadNewTable()`** each `prepare()` and parented to `this`
  but never explicitly replaced/deleted — old models accumulate for the window's lifetime.
- **Leaked dolphin in BOTH export paths (confirmed by reading the call sites)**:
  `exportToWaveFormFile` (`MainWindow.cpp:1488`) and `exportToWaveFormTerminal` (`:1495`)
  each do `new BewavedDolphin(...)` passing `this` as the *`MainWindow*` arg* (stored as
  `m_mainWindow`), **not** the Qt parent — the ctor is `QMainWindow(nullptr)`. They call
  `createWaveform()` + `print()` but never `show()`/`close()`/`delete`, so `WA_DeleteOnClose`
  never fires and both instances leak (harmless only because these are one-shot CLI exits).
  The interactive `on_actionWaveform_triggered` (`:1824`) calls `show()`, so it self-deletes
  correctly — only the two export paths leak.
- **UI placeholder string**: `retranslateUi` sets the window title to `"MainWindow"`
  (`:245`) before the constructor overrides it — translatable noise.
- **Unused/idempotent `Settings::dolphinGeometry` restore** ordering and the `resize()`
  calls in both `setupUi` and the constructor (800×600 then 800×500) are redundant.

## 8. Testability and test quality (from reading the test suite in full)

The scene-embedding architecture (§1) directly degrades the tests, and a few tests are weak:

- **Tests fight the proxy widget.** `TestBewavedDolphinGui` needs a `findTableView()` helper
  that walks `scene->items()` and unwraps each `QGraphicsProxyWidget` just to reach the table
  (`:73-89`); `testDoubleClickToggle` can't use `QTest::mouseDClick` because the view is inside
  the proxy, so it `emit tv->doubleClicked(...)` directly (`:511-518`). A plain central widget
  removes both hacks.
- **Hard dependency on a non-null `MainWindow`.** `testSaveAsAction` admits it *cannot* trigger
  Save As because the handler dereferences `m_mainWindow` (`:767-769`); so the entire Save-As /
  load-from-file / associate path is GUI-untested. `BewavedDolphin` reaches into
  `m_mainWindow->currentFile()/currentDir()/dolphinFileName()/save()` in many places — injecting a
  small interface (or passing the needed paths in) would make these paths testable.
- **`saveToTxt`'s destructive side effect is depended upon by a golden test.** `TestWaveform::testDisplay4Bits`
  works only because `saveToTxt` forces combinational mode and regenerates the full truth table —
  cementing the §6 conflation of "export text" with "rewrite the user's waveform."
- **The zoom quirk is frozen in tests.** `testZoomScaleTrackingA26` / `testFitScreenClampsAndGuardsA26`
  assert the muddled `m_scale`/transform behavior via `friend`-access to privates, so the §1/§7
  cleanup must rewrite these, not just keep them green.
- **The dolphin-filename save→load round-trip is effectively untested.** `TestWorkspaceFileops::testDolphinFileNameStorage`
  only checks the in-memory getter/setter, and `TestSerialization::testDolphinFilenamePreserved`
  saves+reloads but then asserts only `!scene.elements().isEmpty()` — it never `QCOMPARE`s the
  reloaded `dolphinFileName()`, so its name overpromises. Persistence is exercised by the fuzzers
  but not asserted by a unit test.
- **Stale comment in `testWaveformOnEmptySceneShowsWarningHQ`** (`TestMainWindowGui.cpp:3157`):
  claims `on_actionWaveform_triggered` "pre-checks the empty case," but the slot (`MainWindow.cpp:1815-1827`)
  has no such pre-check — the empty-circuit `PANDACEPTION` from `loadElements()` is caught by
  `Application::guardedSlot`. Code/comment drift to fix when that area is touched.
- **`TestDolphinSerializer::testCorruptedDataHandling` is tautological** (`QVERIFY(true)` regardless),
  and `testOpenWaveformEditor` has to hunt the dolphin window among `topLevelWidgets()` and force-clear
  `WA_DeleteOnClose` — another symptom of the lifecycle/embedding issues above.

## Not problems (leave as-is)

- **Hand-written `BewavedDolphinUi`** is the *project-wide convention* — the app has zero
  `.ui` files. Not a smell.
- **`Serializer` namespace** is already a clean pure-IO extraction; only the magic-number
  nit (§7) applies.
- **`SignalModel` / `SignalDelegate`** are small and focused (delegate just got the
  SVG→QPainter refactor); fine as-is.
- **`SimulationBlocker` / `SimulationThrottleDisabler`** are clean RAII guards; the dolphin
  binding to them in `run()` is correct.
- **`Workspace` dolphin-filename plumbing** and `Settings::dolphinGeometry` are trivial,
  correct getters/setters; `Serialization::{read,write}DolphinHeader` is already fuzz-hardened.

## Verification (for any item later acted on)

- Build `cmake --build --preset debug`; run `./build/test_wiredpanda TestBewavedDolphinGui`
  (35 cases covering length/autocrop/clock/zoom/show-modes/export) plus `TestFiles` for
  `.dolphin`/`.csv` round-trips.
- For the §1 architecture change specifically: add/keep tests asserting horizontal
  scrolling works, Fit/Reset/discrete zoom produce expected column widths, and PNG/PDF
  export is unchanged pixel-wise; manually open dolphin on a clocked circuit and exercise
  all zoom paths + Show Numbers/Waveforms.
