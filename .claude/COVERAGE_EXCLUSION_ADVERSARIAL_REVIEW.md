# Adversarial validation of gcov/lcov exclusion markers

Re-derives every `LCOV_EXCL_LINE`/`LCOV_EXCL_START`/`LCOV_EXCL_STOP` justification
in `.claude/COVERAGE_100_PLAN.md` against current source, directory by directory,
following the plan at `~/.claude/plans/do-adversarial-validation-of-buzzing-scone.md`.
Only actionable findings are listed — directories/files with no findings say so
in one line.

**Scope covered:** all 422 `LCOV_EXCL_*` markers across all 88 files that carry
one (verified the count and file list are unchanged from scoping time), the
~150-row documented-exclusions log, the 45-item recurring-pattern glossary, both
cross-cutting deferred templates (`Application.h`'s `guardedSlot<Body>`,
`BaseHandler.h`'s `tryCommand<Fn>`), and the mechanical layer (`codecov.yml`,
`coverage.yml`, `Scripts/coverage.sh`).

**Bottom line:** the original sweep holds up very well under adversarial
re-derivation. One real reasoning gap (not fully resolved either way — see
below), three cosmetic/informational inaccuracies in stated justifications
(conclusions still correct), and one local-tooling/CI divergence worth being
aware of. No exclusion was found to be hiding a genuinely reachable, untested
bug.

---

## Finding: `QSaveFile::commit()` cluster — reasoning gap, not fully resolved

**Files:** `App/BeWavedDolphin/DolphinFile.cpp:44`, `App/Element/ICLoader.cpp`'s
`migrateFile()` (:252), `App/Scene/Workspace.cpp`'s `save()`/`autosave()`
(:392, :721), `App/Scene/ICRegistry.cpp`'s `extractToFile()` (:284) — five sites,
one shared justification.

The justification says `QSaveFile::commit()` can only fail, once `open()`
already succeeded, via a scenario needing "a second OS user or root" (e.g.
losing rename permission on a sticky-bit directory). But Qt's `QSaveFile`
documented behavior defers *write* errors to `commit()`: if any `write()` call
during the preceding `saveBinary()`/`saveCSV()`/stream-write fails, `commit()`
itself returns `false` without attempting the rename — and an ordinary
unprivileged process can force a write failure via
`setrlimit(RLIMIT_FSIZE, ...)` (+ `signal(SIGXFSZ, SIG_IGN)` so the failure
surfaces as `EFBIG` instead of killing the process), no second user or root
needed. That's a different reachability path than the one the exclusion's
comment considers, so the stated justification is incomplete as written.

I tried to empirically confirm this with a standalone Qt probe
(`QSaveFile::write()` under a tight `RLIMIT_FSIZE`), but got inconsistent,
non-reproducible results in this sandbox's Bash tool across identical runs
(consistent with the project's known `feedback_test_writes_sandboxed.md`
finding that this Bash tool interferes with subprocess file-write semantics)
— a plain Python `open()`+`write()` under the same rlimit *did* fail
deterministically with `EFBIG`, so the OS-level limit itself works in this
sandbox, but the Qt/C++ probe's behavior wasn't trustworthy enough to call
this either confirmed or refuted.

**Recommendation:** re-run the same probe outside this sandbox (or as a real
CTest) before deciding whether these 5 lines are truly unreachable or whether
an `RLIMIT_FSIZE`-based test could close them for real.

---

## Finding: local `Scripts/coverage.sh` diverges from CI's coverage view

`Scripts/coverage.sh` (the local lcov-based report a developer runs by hand)
explicitly `lcov --remove`s `*/MCP/*` from its output and never runs
`MCP/Client/run_tests.py` (the Python integration suite that drives the
instrumented binary to actually exercise `MCP/Server/**`). CI's
`.github/workflows/coverage.yml`, by contrast, runs that integration suite
specifically to collect `MCP/Server` coverage, and `codecov.yml` defines
`mcp_server` (`MCP/Server/**`) as its own tracked, gated component — not
ignored.

So a developer running the local script gets a report that silently omits an
entire directory Codecov actually gates on. This is arguably a deliberate,
reasonable choice (the local script's data would be incomplete/misleading for
MCP/Server anyway, since it never drives that code path) rather than an
oversight — flagging it per the plan's instruction to surface this divergence
rather than silently "fix" it. If a developer wants a local signal for
MCP/Server before pushing, `Scripts/coverage.sh` doesn't currently provide
one.

---

## Minor/informational notes (conclusions correct, stated reasoning imprecise) — fixed

All 3 comments below have been corrected in source to match the more precise
reasoning found during this review.

- **`App/CodeGen/ArduinoCodeGen.cpp:765-782` and
  `App/CodeGen/SystemVerilogCodeGen.cpp:1319-1351`/`:1385-1417`** — both
  "unsupported element type" dead-switch-arm blocks justify every listed type
  as dead because it's "already filtered" upstream (IC handled separately,
  gate types intercepted earlier, everything else caught by an
  inputs()/outputs()-emptiness check). True for every type in the list
  **except** `ElementType::JKLatch`, which is dead for an unrelated and
  stronger reason: it's a **deprecated enum value with no live
  `GraphicElement` implementation at all**
  (`ElementFactory::hasCreator(JKLatch)` is `false` —
  `Tests/Unit/Factory/TestElementFactory.cpp:376`, `App/Core/Enums.h:59`). No
  real `elm` can ever report this type, so the "filtered by inputs/outputs
  emptiness" reasoning doesn't actually apply to it. Not a functional
  problem — just a blanket justification that could confuse a future reader
  auditing this one case specifically.

- **`App/Core/ItemWithId.cpp:12-17`** — the exclusion comment frames this
  virtual destructor's gap as "the same inlined-special-member-function
  false-miss as `= default` destructors elsewhere in this sweep, just with a
  real body here" (pattern 3, trivial-dtor inlining). That doesn't really
  fit: this destructor has real logic
  (`if (m_registry) m_registry->forget(this);`) and `ItemWithId` isn't
  abstract, so it isn't a candidate for full inlining-away. The actual
  mechanism is much more likely the ABI-variant one already used elsewhere in
  the same doc for `App/Wiring/Port.h` (pattern 35's sibling reasoning):
  grepped every `ItemWithId *` use in the repo and found none that `delete`s
  through that exact base-class pointer type — every real deletion goes
  through the concrete `GraphicElement`/`Connection` type's own D0, which
  then invokes `ItemWithId`'s D2 (base-object destructor) as part of that
  unwind, never `ItemWithId`'s own D0. Same conclusion, more precise
  mechanism than what's written.

- **`App/Element/ElementAppearance.cpp:437-441`** — the guard comment lists
  `AudioBox (hardcoded 0)` as a real caller of `setAlternativeAppearanceAt()`.
  Grepped every call site in the repo: `AudioBox.cpp` never calls this method
  at all (it refreshes purely via `setPixmap()` in the shared
  `AudioOutputElement` base). The 5 real callers are `GraphicElementInput`,
  `GraphicElementSerializer` (×2), `InputRotary`, and `Led`. A non-existent
  caller in the list is harmless here (one fewer real caller to worry about,
  not one more), likely stale copy-paste — worth fixing the comment so a
  future reader doesn't go looking for a call site that isn't there.

---

## Everything else, by directory (no findings beyond the above)

**App/BeWavedDolphin/:** `checkSave()`'s `default:` (QMessageBox::question
with exactly Save/Discard/Cancel can't return anything else — Cancel wins on
Escape/close since it's the only Cancel-role button present); the
compiler-generated-cleanup braces in `snapshot()`, `allCellIndexes()`,
`buildLabels()`, `loadBinary()`, `captureInputs()` (all return
non-trivial-destructor values by value, no throwing call sites);
`exportToPng()`'s `catch(...)` (confirmed `DolphinExporter::renderToPixmap()`
only touches non-throwing Qt widget/paint APIs, `QPixmap::save()` returns
bool); `DolphinModelBuilder::collect()`'s null/wrong-type guard (its one
caller passes a `Scene*`, and `collect()` itself sources `scene->elements()`,
independently confirmed at `Scene.cpp:336-349` to pre-filter via
`item->type() == GraphicElement::Type` before the cast); `DolphinHost.h`'s
defaulted virtual destructor (both derivers — `MainWindow` and the
`TestBewavedDolphinGui.cpp` stub — are ordinary derived-class destructions).

**App/CodeGen/:** `otherPortName()`/`otherPortNameImpl()`'s null-port/
null-connection/cycle-detection guards in both files (confirmed `highLow()`
only ever returns `"HIGH"`/`"LOW"`, the SV equivalent only `"1'b1"`/`"1'b0"`,
both non-empty; confirmed every call site of `otherPortName()` in both files
passes an `inputPort(...)` result, so the SV file's "`visited` can only ever
hold InputPorts" cycle-proof holds structurally, by the type system); the
`isArduinoReserved(varName)` guards (`varName` always embeds
`elm->objectName() + QString::number(counter)` before any reserved-word
check, so it can never collide with a purely-alphabetic reserved word); the
`assignLogicOperator()`/`generateLogicExpressionImpl()` dispatch-set claims
(re-traced both switches' single call sites, case lists match exactly); the
two `LCOV_EXCL_START`/`STOP` blocks in `SystemVerilogCodeGen.cpp` (scoping is
exact, no live code caught inside); the testbench-generation `catch (...)`
block (whole try body re-read — only `QTextStream` writes and
already-proven-non-throwing helper calls).

**App/Core/:** `App/Core/Priorities.h`'s `legacyCalculatePriorities()`
"already resolved" recheck (independently re-derived from the algorithm's own
code: a node is pushed only when absent from both `outPriorities`/`inStack`,
and priority assignment always happens atomically with its
`stack.pop()`/`inStack.remove()` — `stack.top()` genuinely can never already
hold a priority). `App/Core/ThemeManager.cpp`'s Meyer's-singleton Light/Dark
branches (confirmed `Tests/Common/TestUtils.cpp:43-51` redirects `QSettings`
to a per-process temp directory before the first touch, and since each CTest
class runs as its own OS process, every process's first-ever
`ThemeManager::instance()` really does see an empty settings store).
`Application.cpp`'s font-registration branch, `UpdateChecker.cpp`'s
backpressure-abort branch, and the remaining `= default`/Meyer's-singleton
exclusions (`Common.cpp`, `ContextDirProvider.h`, `SimulationHost.h`,
`ElementFactory.h`) all match well-established mechanical patterns.
`Application.h`'s `guardedSlot<Body>` cross-cutting item is confirmed to
carry zero `LCOV_EXCL` markers — genuinely left open, not hidden.

**App/Element/ (34 files, 68 markers):** all ~25 identical
`ElementMetadata`-return pattern-1 braces across `GraphicElements/*.cpp`;
`AudioBox::startAudio()`'s source-empty guard (traced `m_hasOutputDevice`
gating through `AudioOutputElement::play()`/the constructor — `startAudio()`
is only reached once the constructor already allocated `m_player` and ran
`setAudio()` to completion, no null-`m_player` path); `Led.cpp`'s two
`default:` switch arms (confirmed `GraphicElement::setPortSize()` at
`GraphicElement.cpp:673-693` unconditionally clamps/no-ops out-of-range —
note the doc's own session log records this exact class of assumption being
caught wrong once already, for `InputRotary`, and fixed with a real test
instead of an exclusion); `ToneGenerator.h`'s `writeData()` (confirmed
`start()` always opens `QIODevice::ReadOnly`); `TruthTable.cpp`'s two
`rotatesGraphic()` guards (confirmed its `ElementInfo` omits
`.rotatesGraphic`, keeping the struct default `true`, while 9 sibling types
explicitly set it `false` and do reach the equivalent code); `ICLoader.cpp`'s
null-owner guards, `processLoadedItems()`'s post-Connection `continue`, and
its `deserializeAndLoad()` itemsGuard (independently traced both named
non-throwing steps: `IC::resetInternalState()` only clears containers/asserts
invariants, `ICRenderer::generatePreviewPixmap()` has no `throw` in its body);
`ElementLabel.cpp`'s mouse-move guard; `ElementPorts.cpp`'s empty-vector
guards; `ElementFactory.h`'s Meyer's-singleton static; and
`GraphicElementSerializer.cpp`'s "redundant" stream-status check (confirmed
`readPortList()` throws immediately on any stream error in both the count
read and the per-entry loop, so it can never return with a bad status for
that check to catch).

**App/IO/, App/Scene/, App/Simulation/, App/Tour/, App/Wiring/:** leans
heavily on invariants already re-derived above (`ElementPorts::addPort()`
always setting a port's owner, `Scene::elements()`'s `GraphicElement::Type`
pre-filter) plus novel claims checked directly: `Commands.cpp:501`
(`wirelessStateDiffers()` — confirmed `hasWirelessMode()` is purely
type-based virtual dispatch, `GraphicElement` always `false`, `Node` always
`true`); `Commands.cpp:830,838` (`hasAudio`/`hasDelay` — re-grepped every
`GraphicElements/*.cpp`, zero set `hasAudio=true`, only `Clock.cpp` sets
`hasDelay=true`); `Connection.cpp:168` (confirmed `m_endPort` is declared
`InputPort *` and `InputPort::isOutput()` is hardcoded `false`); `Port.h:66`
(confirmed `Port` is genuinely abstract via its 4 pure virtuals);
`TourOverlay.cpp:253-254` (independently re-derived the `qMin` algebra — both
arms give `cx+cw+pad <= ow`); `Workspace.cpp:619,682` (confirmed
`QStandardPaths::setTestModeEnabled(true)` removes the real per-developer
`AppDataLocation`, so the autosave directory is unavoidably pre-created by an
earlier `WorkSpace` before this branch's own test could see it missing).

**App/UI/:** the recurring "value member address, never null" family
(confirmed `WorkSpace::m_scene` and `Scene::m_undoStack` are plain value
members, not pointers, backing guards in `MainWindow.cpp`,
`SceneUiBinder.cpp`, `WorkspaceManager.cpp` all at once); `ElementEditor.cpp`'s
`hasTruthTable`/`hasAudio` guards; `MinimapWidget.cpp`'s "never
empty/invalid" union claim (traced `SceneInteraction`'s `m_selectionRect` — a
value-member `QGraphicsRectItem` unconditionally added to the scene in the
constructor); `ElementContextMenu.cpp`'s `default:` (confirmed `ElementGroup`
is an `enum class`, all 9 values handled by name); `MainWindowHost.h`'s
destructor (confirmed genuinely abstract). Also confirmed the 3 items the
original doc marks **"deferred, not excluded"** really carry zero
`LCOV_EXCL` markers: `setupLanguage()`'s locale auto-detect branches
(`MainWindow.cpp:181-188`), `populateContentMenu()`'s folder-failure branch
(`MainWindow.cpp:283-286`), and `UpdateController.cpp` as a whole.

**MCP/Server/:** most markers are the "pattern 45" `tryCommand([&]{...})`
lambda-entry misattribution, repeated across every handler. Verified the one
claim that's both load-bearing and independently checkable without running
gcov: `MCPProcessor.cpp:321-322`'s "Unknown method" branch, justified because
`addRoutes()`'s registered method set exactly matches `schema-mcp.json`'s
defined commands. Extracted both lists (8 `addRoutes()` calls vs. the
schema's `properties.commands.properties` keys) and diffed them — **43 names
each side, zero difference.** `BaseHandler.h` (home of the other
cross-cutting deferred item, `tryCommand<Fn>`) confirmed to carry zero
`LCOV_EXCL` markers — genuinely left open. `ElementHandler.cpp`'s
`hasAudio`-gated blocks, `BaseHandler.cpp:285`'s null-port fallback, and the
handler `catch`/`createErrorResponse` blocks all match invariants already
re-derived elsewhere in this review.

**Tests/ harness (`TestUtils.cpp`, `RunnerUtils.h`):** the
`inputStatus()`/`outputStatus()`/`clockCycle()`/`clockToggle()` guards all
funnel into `qFatal()` (calls `abort()`) — unreachable-without-crashing by
construction, not just plausible. `RunnerUtils.h:91-96` (the no-class-filter
"run everything in one process" path) is the one exclusion that documents its
own falsification attempt (a direct probe hit a cross-test `QTest` timeout) —
this also retroactively confirms an assumption this review relied on more
than once elsewhere, that every real CTest invocation takes the per-class
`argc > 1` branch instead.
