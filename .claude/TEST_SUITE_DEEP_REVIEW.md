# Test Suite Deep Review

Exhaustive, line-by-line review of the entire `Tests/` tree (234 files, ~100k lines),
requested as a follow-up to the lighter, grep-seeded `TEST_QUALITY_AUDIT.md` Phase A pass.
Run via a 69-agent Workflow (`wf_c5e7e759-b5b`): the suite was split into 34 file groups, each
fully read (no sampling, no git access) by a Find agent, then every raised finding was
independently re-read and re-verified by a second, skeptical agent before being kept. 198
findings survived verification out of 199 raised (1 refuted). Raw per-finding data (all 198,
with verifier notes) is preserved in the workflow's journal
(`/home/torres/.claude/projects/-home-torres-wiredpanda-coverage/c49f630d-52f7-411b-b72a-d5896b1662f3/subagents/workflows/wf_c5e7e759-b5b/journal.jsonl`)
if a machine-readable form is ever needed again.

Severity: **22 high**, **90 medium**, **66 low**, 20 untagged (coverage-gap/weak-assertion
class). None of these were applied as fixes yet — this is the findings document; fixes should
be picked up as their own follow-up work, most likely triaged by the "Systemic Patterns"
groupings below since several apply to many files at once.

One item is a real bug rather than a test-quality issue: `Tests/Fuzz/FuzzUndo.cpp:101` has a
genuine use-after-free (a cached `Connection*` dangles after `DeleteItemsCommand::redo()` frees
it, then gets dereferenced by the very guard meant to check it's still alive) — worth fixing
ahead of the rest, since it could be silently producing spurious ASan aborts misattributed to
`SplitCommand`.

---

<!-- BEGIN GENERATED REPORT (workflow wf_c5e7e759-b5b, run 2026-08-20) -->

# wiRedPanda Test Suite -- Deep Review

This review covers `Tests/` in full (≈234 files, ~100k lines) via 34 independent reviewers, each assigned a slice of the suite for a complete line-by-line read, with no git-blame or history access -- every claim had to be substantiated against the checked-out source as it stands today. Every finding a reviewer raised was then re-read and independently re-verified by a second, skeptical reviewer against the actual production and test code before being kept; findings that didn't hold up were dropped entirely, and findings that were real but partially overstated were kept with an `adjusted` verdict and a corrected description.

**Topline:** 198 findings survived verification (182 `confirmed`, 16 `adjusted` -- kept, but narrowed or corrected by the second reviewer). Severity: **22 high**, **90 medium**, **66 low**, 20 without an explicit severity tag (all coverage-gap/weak-assertion class, effectively low-medium). By category (where tagged): 30 coverage-gap, 26 weak-assertion, 15 test-smell, 10 misleading-test-name, 7 redundant-test, 5 test-isolation, 3 correctness-bug, 3 test-coverage, 2 each of flaky-prone-pattern / silent-skip-overuse / no-op-assertion / resource-safety, plus single instances of correctness, tautological-assertion, dead-code, resource-leak and test-correctness. (Many findings did not carry an explicit category tag but fall naturally into one of the systemic patterns below.)

## Systemic Patterns

### 1. Global/static test state mutated without an RAII guard -- restore is skipped if an assertion fails mid-test, leaking into every later test
The project already has an RAII idiom for this (`ScopedFileDialogStub`, `ScopedTinyFsizeLimit`, `ScopedStdinRedirect`), but it isn't used consistently. A `QVERIFY`/`QCOMPARE` failure between the mutation and the plain restore statement causes Qt Test's early-return semantics to skip the restore entirely, leaking the mutated value to every subsequent test in the run:
- `Tests/Unit/Exercise/TestExerciseEngine.cpp:135` -- `Settings::language()` set to `"en"`, restored only as the last statement.
- `Tests/Unit/Exercise/TestExerciseEngine.cpp:767` -- `ThemeManager` theme hardcoded back to `Light` unconditionally (wrong even on success, since it ignores the theme that was actually in effect at entry).
- `Tests/Unit/Simulation/TestSimulation.cpp:266` -- `Application::interactiveMode` set `true`, restored only after two intervening `QVERIFY2`s.
- `Tests/System/TestBewavedDolphinGui.cpp:1767` and `:1976` -- same `Application::interactiveMode` pattern, twice more, in a file that otherwise uses the RAII stub idiom correctly elsewhere.
- `Tests/System/TestBewavedDolphinGui.cpp:1539` -- real process **stdin** redirected via `testUtilsFreopen()` directly instead of the project's own `ScopedStdinRedirect` (which exists specifically to dup/dup2-save-and-restore stdin); never restored at all.
- `Tests/Unit/MCP/TestICHandler.cpp:216` -- a temp **directory's** permissions are narrowed to read-only between assertions with no guard; a failure there leaves a directory `QTemporaryDir` cannot even delete (POSIX requires parent-dir write permission to unlink). (The three sibling tests in the same file narrow only a *file's* permissions, which does not block cleanup -- so this risk is real for exactly one test in that file, not four.)
- `Tests/Common/TestUtils.h:364` -- the RAII helpers meant to fix exactly this class of bug (`ScopedStdinRedirect`/`ScopedStdoutCapture`) don't check `fopen()`'s return value before passing it to `fileno()`, so a bad path crashes the whole binary with UB instead of a legible `qFatal`.

**Fix pattern:** wrap every one of these in a scope guard (`qScopeGuard`, or the existing `ScopedFileDialogStub`/`ScopedStdinRedirect` idiom) that restores on any exit path, and add a null check to the two RAII helpers themselves so they fail loudly instead of undefined-behaving.

### 2. Loose/OR/tolerance assertions where an exact expected value is known and trivially computable
A recurring shortcut: instead of computing the one correct value (often derivable from the test's own setup or its own comment) and `QCOMPARE`-ing it, the assertion accepts a wide band or an `||` of unrelated conditions, which a real regression can slip through:
- `Tests/Unit/CodeGen/TestSystemVerilogCodeGenUnit.cpp:187` -- `contains("case") || contains("assign")`; the `assign` half is emitted unconditionally by the LED regardless of the Mux, making the whole check pass even if the Mux's `case` logic is deleted.
- `Tests/Unit/Common/TestEnums.cpp:97` -- `contains("MULTIPLE", CI) || !isEmpty()`; second clause makes the first meaningless.
- `Tests/Unit/Serialization/TestSerialization.cpp:1265` -- 7-way `||` chain over every label used in the fixture, satisfied regardless of load order/correctness.
- `Tests/Integration/TestWorkspace.cpp:282` -- `contains("autosaves") || contains(".panda")`; every autosave entry always ends in `.panda` (proven by a sibling test), so the directory-location half never actually gets tested.
- `Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.cpp:898` -- `QVERIFY((readValue == X) || (readValue == Y))` in three more branches (also `:648`, `:833`, and `TestLevel5RegisterFile4x4.cpp:1326` in the sibling data-driven test) with no comment justifying the ambiguity and no re-sample to confirm determinism, unlike `TestSimulation.cpp`'s analogous (and correctly justified) pattern.
- `Tests/Unit/Elements/TestGeometry.cpp:80`, `:124`, `:161` -- three port-position tests all use `pos.y() >= lo && pos.y() <= hi` bands (16px-32px wide) where the test's own comment already derives the single exact expected y (32.0, 24.0, 8/24/40/56).
- `Tests/Unit/Elements/TestClock.cpp:515` -- `contains("1")`/`contains("5")` single-digit substring checks for formatted frequency text.
- `Tests/Unit/Scene/TestSceneState.cpp:731` -- position after `setPos()` checked with `>=` and a hedging comment, even though the grid-snap math (8px) makes the exact post-snap value fully deterministic.
- `Tests/Integration/TestWorkspace.cpp:782` -- `autosavesAfter.size() <= countBefore` where both sides are provably 0, a trivial `0<=0`.
- `Tests/Unit/MCP/TestConnectionHandler.cpp:452` (x2 tests) -- split-connection success checked only via `items().size() > itemCountBefore`, when the real delta is a known, fixed `+2`.

**Fix pattern:** replace with `QCOMPARE` against the value the test's own comment/setup already derives.

### 3. Assertions that check a structural proxy (count, non-null, tautology) instead of the actual behavior the test name promises
The single most common failure mode in the suite: a test builds a real, correct scenario, then asserts something that would be identical for a broken implementation --  `elements().size()` unchanged, a pointer non-null, or a value compared to itself:
- `Tests/Unit/Scene/TestScene.cpp:669` (flip) and `:970` (morph-with-connections) -- only `elements().size()` checked; `Scene::elements()` structurally excludes `Connection`s and doesn't reflect `isFlippedX/Y()`.
- `Tests/Unit/Scene/TestConnectionManager.cpp:24,38,67` -- `testConnectionCreation`/`testConnectionDeletion`/`testConnectionValidation`/`testMultiPortConnection` all check only `!inputs().isEmpty()` or `items().contains(...)`, none of which reflects whether a `Connection` was ever actually made.
- `Tests/Unit/Wiring/TestConnections.cpp:182,169,131,373` -- "reject duplicate/self/output-to-output" tests never attempt the rejected operation at all; `testRejectOutputToOutput`/`testRejectInputToInput` only prove C++'s `dynamic_cast` semantics, guaranteed true at compile time regardless of app logic.
- `Tests/Unit/Commands/TestCommands.cpp:496` -- post-undo connection check only verifies `startPort()`/`endPort()` non-null, not which ports.
- `Tests/Integration/TestIc.cpp:146` -- `inputSize() >= 0` / `outputSize() >= 0`, both always true for an `int`.
- `Tests/Unit/Serialization/TestSerialization.cpp:966` -- `WorkSpace::scene() != nullptr` repeated as the main positive-path assertion in 5 different tests (`:966,1129,1341,1380,1423`); `scene()` returns `&m_scene`, a value member -- can never be null.
- `Tests/Unit/Elements/TestFeatures.cpp:212` -- compares two freshly-built same-type instances against each other, true by construction.
- `Tests/Unit/Scene/TestSceneConnections.cpp:257` -- re-reads the exact ports it just set two lines above.
- `Tests/Integration/IC/Tests/TestLevel6Register8bit.cpp:486` -- `QCOMPARE(pass_count, total_scenarios)`, two counters always incremented together, guaranteed equal by construction.
- `Tests/Integration/TestMainWindowGui.cpp:3326`, `Tests/Integration/TestWorkspace.cpp:891` and `:563` -- similar "nothing was ever populated so the after-check is trivially satisfied" shape.

**Fix pattern:** assert the specific state the test name promises (a port identity, a color/rotation/flip flag, a connection's endpoints), not a size or a pointer.

### 4. Control/enable signals wired to real, independently-settable switches, but pinned to one fixed value in every test in the file
Concentrated in the `Integration/IC` CPU-building-block tests: a fixture wires two genuinely independent control inputs, but the shared helper (or every data row) always drives the same combination, leaving reachable input combinations entirely untested:
- `Tests/Integration/IC/Tests/TestLevel4RippleAlu4bit.cpp:73` -- `CarryIn`/`SubCarryIn` hardcoded false/true on every call to `setOperands()`, the sole write path.
- `Tests/Integration/IC/Tests/TestLevel4RippleAdder4bit.cpp:122` -- `CarryIn` driven `false` before all 10 data rows; carry-in propagation never tested.
- `Tests/Integration/IC/Tests/Cpu/TestCPURegisters.cpp:236` -- `writeEnable=0` while `decoderOut=1` never exercised (a decoder-only gate bug would pass unnoticed).
- `Tests/Integration/IC/Tests/TestLevel6Ram8x8.cpp:136` -- no write-inhibited (`we=false` during an attempted write) or address-isolation test, unlike sibling `TestLevel4Ram4x1.cpp`/`TestLevel4Ram8x1.cpp`.
- `Tests/Integration/IC/Tests/TestLevel9RegisterFile32x16.cpp:67` -- `writeAt()` always drives `we=true`; no hold-while-`we=0` test exists in the class.
- `Tests/Integration/IC/Tests/TestLevel8MemoryStage.cpp:98` and `Tests/Integration/IC/Tests/TestLevel9MemoryStage16bit.cpp:102` -- both `MemRead`/`MemWrite` control-switch data tables cover 3 of the 4 combinations, never `(true,true)`.

**Fix pattern:** add a data row / dedicated test per file that drives the pinned signal to its untested value and asserts the resulting (in)action.

### 5. Hand-rolled test-case loop with an unused diagnostic `name` field instead of a real QTest data table
The same copy-pasted shape appears three times, each time carrying a `name` string per case purely for human readability that is never actually surfaced on failure (so a failing row only reports file/line, not which of the N cases failed):
- `Tests/Integration/IC/Tests/TestLevel3AluSelector5way.cpp:122` (12 rows)
- `Tests/Integration/IC/Tests/TestLevel2PriorityMux3to1.cpp:108` (9 rows)
- `Tests/Integration/IC/Tests/Cpu/TestCPUDecoders.cpp:107` (16 rows)

**Fix pattern:** convert to `QTest::newRow(test.name)` / a real `_data()` function, or at minimum include `test.name` in a `QVERIFY2` failure message.

### 6. Giant single test function bundling many unrelated, order-dependent scenarios with no failure isolation
An early `QCOMPARE`/`QVERIFY` failure inside these aborts the whole function per Qt Test semantics, silently hiding every later scenario's result for that run:
- `Tests/Integration/IC/Tests/TestLevel6Register8bit.cpp:103` -- `test8BitRegister()`, 175 lines, 6+ scenarios (reset, write, hold, re-enable, individual-bit mapping, mid-range sweep).
- `Tests/Integration/IC/Tests/TestLevel3Register1bit.cpp:85` -- `test1BitRegister()`, 6 sequential state-dependent scenarios in one slot (contrast: sibling `TestLevel4Register4bit.cpp` already splits this pattern into separate slots).
- `Tests/System/TestBewavedDolphinGui.cpp:1680` -- `testTrivialAccessors` bundles 5 largely independent concerns (toolbar/view accessors, combinational-mode trigger, port-label lookup, snapshot) in one chain.

**Fix pattern:** split into one `QTest` slot per scenario, matching the pattern the same codebase already uses elsewhere (`test8BitRegisterSequential`/`test8BitRegisterEdgeCases`, `TestLevel4Register4bit.cpp`).

### 7. Near-duplicate / copy-pasted test bodies that add no coverage beyond an existing test, or should be one data-driven test
**Redundant (no unique coverage beyond a sibling test):**
`Tests/Unit/Scene/TestConnectionValidity.cpp:142`, `Tests/Integration/TestICInline.cpp:3764`, `Tests/Integration/TestMainWindowGui.cpp:2179`, `Tests/Integration/IC/Tests/Cpu/TestCPUAlu.cpp:166`, `Tests/Integration/IC/Tests/TestLevel2Decoder2to4.cpp:139`, `Tests/Integration/IC/Tests/TestLevel2Decoder5to32.cpp:115`, `Tests/Integration/IC/Tests/TestsWithoutPanda/TestRamCell1bit.cpp:78`, `Tests/Integration/IC/Tests/TestsWithoutPanda/TestSequential.cpp:473`, `Tests/Unit/Logic/TestElementLogicErrors.cpp:252`, `Tests/Unit/Wiring/TestConnections.cpp:373`, `Tests/Integration/TestFeedback.cpp:225`, `Tests/Unit/CodeGen/TestArduinoCodeGenUnit.cpp:130` and `:160` (byte-for-byte identical to `testEmptyScene`).

**Should be consolidated / data-driven (near-identical setup boilerplate copy-pasted N times):**
`Tests/Unit/Elements/TestInputRotary.cpp:232` (8 functions), `Tests/Integration/TestArduino.cpp:2523` (24 one-liner wrappers), `Tests/Integration/TestSimulation.cpp:326` (5 determinism tests), `Tests/Unit/Ui/TestICController.cpp:800` (3 functions), `Tests/Unit/Ui/TestWorkspaceManager.cpp:180` (2 functions), `Tests/Unit/Factory/TestElementFactory.cpp:141` (list duplicated verbatim between two functions), `Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.cpp:418` (22-way positional if/else chain instead of a named dispatch).

### 8. Platform-gating granularity: whole-class skip where a per-test skip would do, and the inverse (missing platform guard where a sibling file has one)
- `Tests/Integration/TestSystemVerilogExport.cpp:115` -- `initTestCase()` `QSKIP`s the **entire class** (70+ tests) on non-Linux, even though the actual `iverilog`/`yosys`/`verilator` process calls are confined to a `generateMode`-only branch that doesn't run by default; most of the 76 tests, including 3 that touch no `QProcess` at all, never needed the skip.
- `Tests/Unit/MCP/TestFileHandler.cpp:316` -- the inverse problem: relies on Unix-only `/dev/null` semantics with **no** `Q_OS_WIN`/`QSKIP` guard, unlike 4 sibling tests in `TestICHandler.cpp` that do guard the identical scenario, and Windows is a real CI matrix target.
- Related: `Tests/Integration/TestArduino.cpp:2467` -- functional (simavr) validation for all Level 1-4 IC export tests exists but only runs behind a manual `GENERATE_EXPECTED_ARDUINO` env var, never in normal CI; the default path is a byte-diff against a golden file that can't catch a functional regression baked into that golden file.

## Findings by Area

### Unit/CodeGen (5)
- **high** `Tests/Unit/CodeGen/TestArduinoCodeGenUnit.cpp:130` -- `testMuxCircuit`: only assertion (`contains("void setup()")`) is identical to `testEmptyScene`'s; a broken Mux lowering would pass. **Fix:** assert Mux-specific emitted code (select/data routing).
- **high** `Tests/Unit/CodeGen/TestArduinoCodeGenUnit.cpp:160` -- `testDemuxCircuit`: same issue, identical to `testEmptyScene`. **Fix:** assert select-conditioned routing text, as `testDemuxWithFourOutputsUsesTwoSelectLines` already does elsewhere in the file.
- **medium** `Tests/Unit/CodeGen/TestSystemVerilogCodeGenUnit.cpp:187` -- Mux-specific check degenerates to near-tautology via `contains("case") || contains("assign")`. **Fix:** drop the OR, assert `case`/`endcase` specifically.
- **medium** `Tests/Unit/CodeGen/TestArduinoCodeGenUnit.cpp:332` -- no test exercises `ArduinoCodeGen`'s main-sketch constructor on an unwritable path (sibling SV test does, for the same regression class). **Fix:** add `testArduinoCodeGenConstructorUnwritablePathThrows`.
- **low** `Tests/Unit/CodeGen/TestArduinoCodeGenUnit.cpp:67` -- no assertion in the file ever checks the emitted `&&`/`||`/`!` operator text for And/Or/Not. **Fix:** add operator-text checks like the SV sibling file's `testNandGateCircuit`.

### Unit/Commands (5)
- **medium** `Tests/Unit/Commands/TestCommands.cpp:430` -- `ChangePortSizeCommand` only tested growing (2→4, 1→4 inputs/outputs), never shrinking with connections attached; `redo()`'s connection-drop path is untested. **Fix:** add a shrink test with pre-existing wires on the removed ports.
- **medium** `Tests/Unit/Commands/TestCommands.cpp:496` -- `testConnectionPreservation` post-undo check only verifies non-null ports, not correct wiring. **Fix:** re-check `startPort()`/`endPort()` identities.
- **medium** `Tests/Unit/Commands/TestCommands.cpp:220` -- `FlipCommand` test's own comment computes exact expected positions, but the assertion is a loose `||` on "did anything change". **Fix:** `QCOMPARE` against the derived `(200,100)`/`(100,100)`.
- **medium** `Tests/Unit/Commands/TestCommands.cpp:1189` -- `testFlipCommandWithEmptyItemsIsNoOp` never calls `redo()`/`undo()`, unlike its two sibling no-op tests. **Fix:** call `redo()`/`undo()` and assert no crash/no-op state.
- **low** `Tests/Unit/Commands/TestCommands.cpp:1022` -- `testToggleTruthTableOutputCommandBounds` never tests the top-edge boundary (2047), only 5 and the invalid values above 2048. **Fix:** add a must-succeed case at 2047.

### Unit/Common (5)
- **high** `Tests/Unit/Common/TestEnums.cpp:97` -- `testTypeToTitleText`: `contains("MULTIPLE") || !isEmpty()` is vacuous; `tr()`'s literal output is stable enough for an exact `QCOMPARE`. **Fix:** `QCOMPARE(unknownTitle, "MULTIPLE TYPES")`.
- **medium** `Tests/Unit/Common/TestThemeManager.cpp:183` -- `testLightThemeSceneColors`: comments claim dots-darkness/cream-color checks the assertions never perform (only `isValid()` / red&green>200, blue never checked). **Fix:** mirror `testDarkThemeSceneColors`'s full-RGB + exact-color pattern.
- **low** `Tests/Unit/Common/TestEnums.cpp:378` -- `testStatusEnumToString`/`testElementGroupToString` only check non-emptiness/pairwise-inequality, never exact key strings. **Fix:** `QCOMPARE` against exact strings, as `testAllEnumsExposeMetaEnum` already does.
- **low** `Tests/Unit/Common/TestEnums.cpp:112` -- `testElementTypeIncrement`: integer-equality checks written as `QVERIFY` instead of `QCOMPARE`, losing diagnostic value on failure.
- **low** `Tests/Unit/Common/TestSettings.cpp:149` -- `testTypedTheme` round-trips only `Light`/`Dark`, skips `Theme::System` (a normal in-range value, not just an error fallback). **Fix:** add an explicit `setTheme(Theme::System)` round-trip.

### Unit/Core (5)
- **medium** `Tests/Unit/Core/TestApplication.cpp:13` -- `testApplicationNotify()`/`testExceptionHandling()` are byte-for-byte identical stubs; neither invokes `Application::notify()` or exercises exception handling. **Fix:** rename/delete, or add real coverage of `notify()`'s catch path.
- **medium** `Tests/Unit/Core/TestExerciseTourResources.cpp:302` -- `discover()`'s merge of `managedContentDir()` entries is never exercised end-to-end, unlike the analogous install-relative/Documents-fallback merge tests. **Fix:** write a JSON entry into `managedContentDir()` and assert it surfaces via `discover()`.
- **low** `Tests/Unit/Core/TestSettings.cpp:9` -- `testSettingsSave()`/`testSettingsLoad()` fully subsumed by `testSettingsRoundTrip()`. **Fix:** delete the two smaller tests.
- **low** `Tests/Unit/Core/TestSettings.cpp:47` -- `Settings::theme()`'s upper-bound (`Theme::System`) round-trip via `setTheme()` and `hideV4Warning()`'s remove-vs-store-falsy asymmetry (`QSettings::contains()` after `setHideV4Warning(false)`) are genuinely unverified anywhere in the suite (narrower than first reported -- most of this file's ground is already covered by `Tests/Unit/Common/TestSettings.cpp`). **Fix:** add the two narrow round-trip checks described.
- **low** `Tests/Unit/Core/TestExerciseTourResources.cpp:109` -- `translateFromCatalog()`'s empty-string-value branch (as opposed to key-absent) is untested. **Fix:** add a catalog entry with `"title": ""` and assert fallback-to-English.

### Unit/Elements (22)
- **high** `Tests/Unit/Elements/TestMux.cpp:22` -- `testMuxSelection` is a byte-for-byte duplicate of `testMuxInputSize`; no test anywhere in the file drives select+data and checks output routing. **Fix:** add a real select→data-routing test.
- **high** `Tests/Unit/Elements/TestDemux.cpp:24` -- `testDemuxRouting` only checks `inputSize()>0`; no test verifies data actually routes to the selected output. **Fix:** drive a valid select index, assert the addressed output tracks input and others stay Inactive.
- **medium** `Tests/Unit/Elements/TestIC.cpp:40` -- `testICLoadFromFile`/`testICPortLabelResolution`/`testICNestedSaveLoad`/`testICInvalidFile` all operate on an unloaded/empty IC and don't exercise what their names promise. **Fix:** actually `load()` a real IC file in each.
- **medium** `Tests/Unit/Elements/TestLed.cpp:187` -- `testColorIndexTwoInputs` never drives the input ports it claims to set high before checking `color()=="White"`. **Fix:** set both ports Active before `refresh()`. (Downgraded from high: `testAppearanceStates` already independently verifies the real index-25 branch.)
- **medium** `Tests/Unit/Elements/TestLed.cpp:213` -- `testColorIndexThreeInputs`/`testColorIndexFourInputs`: same pattern, no port ever driven; only `testAppearanceStates` actually verifies these index formulas.
- **medium** `Tests/Unit/Elements/TestLed.cpp:400` -- `testLoadColorOldVersion` loads a v2.3 fixture but never checks any LED's loaded `.color()`. **Fix:** cast and check color post-load.
- **medium** `Tests/Unit/Elements/TestInputRotary.cpp:553` -- locked-state section sets `setLocked(true)` then never asserts the lock's effect on `setOn()`. **Fix:** assert output value unchanged while locked.
- **medium** `Tests/Unit/Elements/TestInputElements.cpp:204` -- `testInputSwitchLockingMechanism`/`testInputButtonLockingMechanism` never call `setLocked()` at all, despite the name. **Fix:** actually lock and assert state can't change.
- **medium** `Tests/Unit/Elements/TestICRegistry.cpp:109` -- `testICFileWatcher()` never rewrites the watched file or triggers `onFileChanged()`; cache-invalidation-on-change is untested. **Fix:** modify the file, wait for the queued signal, re-check `cachedFileBytes()`.
- **medium** `Tests/Unit/Elements/TestElementLabel.cpp:175` -- `testMimeDataContent`/`testMimeDataWithIC` only check payload byte count, never decode/verify the serialized fields (`elementType`, `icFileName`, `isEmbedded`). **Fix:** deserialize and `QCOMPARE` each field.
- **low** `Tests/Unit/Elements/TestInputRotary.cpp:70` -- `testPreviousPortWrapping` doesn't test backward navigation at all; duplicates the forward-wrap tests. **Fix:** exercise the actual previous-port API.
- **low** `Tests/Unit/Elements/TestInputRotary.cpp:232` -- 8 near-identical `testPortConfigurationWithXPorts` functions should be one data-driven test.
- **low** `Tests/Unit/Elements/TestElementProperties.cpp:385` -- `testFrequencyValidation`'s high-frequency case branches on the outcome instead of asserting the (deterministically-always-rejected) one true outcome.
- **medium** `Tests/Unit/Elements/TestMux.cpp` / `TestDemux.cpp` -- see high entries above.
- **medium** `Tests/Unit/Elements/TestDisplay7.cpp:30` -- `testDisplay7SegmentRendering` never sets input ports, calls `refresh()`, or renders a pixmap despite its name; weaker duplicate of `testDisplay7ColorSetting`. **Fix:** delete or fold into `testDisplay7Paint`'s real render check.
- **medium** `Tests/Unit/Elements/TestFeatures.cpp:212` -- `testFeatureConsistency` compares two fresh same-type instances against each other -- tautological. **Fix:** compare against concrete expected booleans, as `testMultipleElementsFeatures` already does.
- **medium** `Tests/Unit/Elements/TestGeometry.cpp:80` -- `testInputPortPositionSinglePort` actually builds an Or gate (2 ports), never a single-port element, and uses a 32px-wide tolerance where 32.0 exact is known.
- **medium** `Tests/Unit/Elements/TestGeometry.cpp:161` -- `testPortCenteringAroundY32` only checks the average of 4 port y-positions, insensitive to symmetric-but-wrong per-port spacing.
- **low** `Tests/Unit/Elements/TestGeometry.cpp:124` -- `testOutputPortPositionSinglePort` uses a loose range where 32.0 exact is known and the element (Not) is correctly single-port.
- **low** `Tests/Unit/Elements/TestGeometry.cpp:35` -- `testBoundingRectNoPortsCalculation` never tests a zero-ports element at all; comment admits it.
- **low** `Tests/Unit/Elements/TestClock.cpp:515` -- `testGenericProperties()` uses single-digit substring checks (`contains("1")`) instead of exact formatted text.
- **low** `Tests/Unit/Elements/TestWirelessNode.cpp:384` -- dead leftover `corruptBuffer` block in `testLoadCorruptedWirelessModeClampsToNone`, never read.
- **low** `Tests/Unit/Elements/TestComponents.cpp:122` -- `testIC` repeats an unvarying 4-phase stimulus in an unexplained 10-iteration loop.

### Unit/Exercise (5)
- **medium** `Tests/Unit/Exercise/TestExerciseEngine.cpp:767` -- `testOverlayAppliesThemeReactsToThemeChange` hardcodes global `ThemeManager` back to `Light` instead of the original theme. (See systemic pattern #1.)
- **medium** `Tests/Unit/Exercise/TestExerciseEngine.cpp:135` -- `Settings::language()` restore skipped on assertion failure between `setLanguage("en")` and the trailing restore. (See systemic pattern #1.)
- **medium** `Tests/Unit/Exercise/TestExerciseEngine.cpp:489` -- 5 tests sharing `kExerciseFixtureId` assume step-0 progress without resetting it themselves, unlike 3 sibling tests that do; hidden declaration-order dependency.
- **low** `Tests/Unit/Exercise/TestExerciseEngine.cpp:310` -- `testLoadFromResourceMissingIdOrTitleFails` only exercises the missing-**id** half of `m_id.isEmpty() || rawTitle.isEmpty()`, never missing-title.
- **low** `Tests/Unit/Exercise/TestExerciseEngine.cpp:516` -- `stepChanged` emission on the non-terminal `advanceStep()` path is genuinely unverified anywhere in the file (narrower than first reported -- `stepCompleted` emission on the same shared code path is already covered by `testOnCircuitChangedAutoAdvancesWhenElementRequirementSatisfied`). **Fix:** add a `stepChangedSpy` assertion to that same test.

### Unit/Factory (2)
- **medium** `Tests/Unit/Factory/TestElementFactory.cpp:28` -- `testBuildElement_data()` covers only 17 of 32 real buildable `ElementType` values with the full build-time invariant check; `testAllElementTypesRegistered()` walks all types but never builds an instance or checks per-instance port invariants. 15 types (AudioBox, Buzzer, DLatch, Display7/14/16, IC, InputGnd, InputVcc, Line, SRFlipFlop, SRLatch, TFlipFlop, Text, TruthTable) get zero build-time invariant coverage. **Fix:** extend `testBuildElement_data()` to walk the same metaEnum set `testAllElementTypesRegistered()` already walks.
- **low** `Tests/Unit/Factory/TestElementFactory.cpp:141` -- the 17-type list is duplicated verbatim (not shared) between `testBuildElement_data()` and `testTypeToTextBidirectional()`. **Fix:** extract to a shared constant.

### Unit/Logic (7)
- **high** `Tests/Unit/Logic/TestStatusOps.cpp:721` -- `testDFlipFlopUnknownClockNoEdge` never actually sets the clock to `Unknown`/`Error`; body is an ordinary two-cycle edge sequence despite its name and the file's "4-state behavior" section header. **Fix:** drive `clk.setOutputValue(Status::Unknown)` and assert no spurious edge/state change.
- **medium** `Tests/Unit/Logic/TestElementLogicErrors.cpp:24` -- `testInvalidInputPortIndex` only connects in-bounds indices; never tests an actual out-of-bounds index (that case is covered elsewhere by `testConnectPredecessorOutOfBoundsIsNoOp`, just not here despite the name).
- **medium** `Tests/Unit/Logic/TestElementLogic.cpp:281` -- `testDFlipFlop`/`_data` never varies the previous-Q state (both "maintain" rows force `lastClk=true`), unlike its JK/SR/T/latch siblings which explicitly pair both prior states.
- **medium** `Tests/Unit/Logic/TestElementLogic.cpp:1040` -- `test3InputXor`/`test3InputXnor` cascade two 2-input gates instead of calling `setInputSize(3+)` on the real N-input Xor/Xnor (`minInputSize=2, maxInputSize=8`); the native multi-input path is untested anywhere in the file.
- **medium** `Tests/Unit/Logic/TestElementLogic.cpp:824` -- `testSRLatch_data` never tests the S=1,R=1 contention/forbidden state, unlike `testSRFlipFlop_data`'s explicit "not permitted" rows.
- **low** `Tests/Unit/Logic/TestElementLogic.cpp:686` -- `testNotGateInAnd_data`'s `expectedNotInput0` column is mislabeled (actually stores `NOT(input1)`) and dead -- discarded via `Q_UNUSED`, recomputed inline instead.
- **low** `Tests/Unit/Logic/TestElementLogicErrors.cpp:252` -- `testInputValueBoundary` duplicates `test4InputAnd`'s exhaustive 16-row truth table under a misleading "boundary" name; no port-index/Status boundary is actually exercised.

### Unit/MCP (9)
- **medium** `Tests/Unit/MCP/TestElementHandler.cpp:544` -- Node `wireless_mode=Rx` (input-port severing) branch never exercised; only Tx(1)/None(0)/invalid(99) are driven. **Fix:** add a `wireless_mode=2` (Rx) test.
- **medium** `Tests/Unit/MCP/TestConnectionHandler.cpp:452` -- `split_connection` success verified only via `items().size() > itemCountBefore`; real delta is a known, fixed `+2`. **Fix:** `QCOMPARE(itemCountAfter, itemCountBefore + 2)`.
- **medium** `Tests/Unit/MCP/TestFileHandler.cpp:316` -- `testHandleExportImageRejectsSvgWriteFailure` relies on Unix-only `/dev/null` semantics with no Windows guard, unlike 4 analogous `TestICHandler.cpp` tests; Windows is a real CI target.
- **medium** `Tests/Unit/MCP/TestSimulationHandler.cpp:144` -- no test exercises a successful `create_waveform` with valid `input_patterns`; the label→row resolution and per-cell write loop is only ever exercised on the failure paths.
- **medium** `Tests/Unit/MCP/TestThemeHandler.cpp:74` -- `get_effective_theme` test never sets `Theme::System`, so `effectiveTheme()`'s only interesting branch (`Theme::System → resolveSystemTheme()`) is untested; only the trivial passthrough is covered.
- **low** `Tests/Unit/MCP/TestICHandler.cpp:216` -- narrowed temp-**directory** permission restoration skipped on assertion failure in `testHandleCreateIcFailsOnUnwritableTarget` specifically (the 3 other similarly-shaped tests narrow file, not directory, permissions and don't share the leak risk). (See systemic pattern #1.)
- **low** `Tests/Unit/MCP/TestFileHandlerSecurity.cpp:41` -- no coverage for negative/malformed padding in `export_image`, though production code's existing `std::clamp(..., 0, kMaxExportPadding)` already handles it safely today.
- **low** `Tests/Unit/MCP/TestSimulationHandler.cpp:113` -- `testCreateWaveformRejectsInvalidDuration` never exercises the exact `kMaxColumns` (2048) boundary, only 0/-1/999999.
- **low** `Tests/Unit/MCP/TestHistoryHandler.cpp:130` -- `redo_text` field, returned by every history handler success response, is never asserted anywhere in the file.

### Unit/Scene (19)
- **high** `Tests/Unit/Scene/TestScene.cpp:1113` -- six show/hide-gates/wires tests (`testShowGatesToggle`, `testShowWiresToggle`, `testGateVisibilityStateTracking`, `testWireVisibilityStateTracking`, `testShowGatesWithMultipleElements`, `testShowWiresWithMultipleConnections`) never call `isVisible()` -- the one thing they claim to test. **Fix:** mirror the sibling `testShowWiresTogglesNodeVisibility`, which does check it correctly.
- **high** `Tests/Unit/Scene/TestScene.cpp:815` -- `testDeleteWithConnections` never creates any connections; the connection-cleanup assertion (`finalConnections <= initialConnections`) is an unconditional `0<=0`.
- **high** `Tests/Unit/Scene/TestSceneState.cpp:647` -- entire "Z-Order and Rendering" section (`testElementZOrderOnSelection`, `testConnectionZOrderBehindElements`, `testBringToFrontAfterSelection`) never reads a `zValue()`.
- **medium** `Tests/Unit/Scene/TestSceneState.cpp:216` -- `testEditedConnectionSetGet` never touches an edited connection, only checks `simulation()` non-null.
- **medium** `Tests/Unit/Scene/TestSceneState.cpp:445` -- three "Hover Port" tests never call any hover-port API.
- **medium** `Tests/Unit/Scene/TestScene.cpp:669` -- `testFlipHorizontal`/`testFlipVertically` only check element count survives; `isFlippedX()`/`isFlippedY()` never checked.
- **medium** `Tests/Unit/Scene/TestScene.cpp:1024` -- `testPropertyCycleFrequency`/`testPropertyCycleColor`/`testPropertyCycleAudio` capture the initial value but never re-read it after cycling.
- **medium** `Tests/Unit/Scene/TestScene.cpp:970` -- `testElementMorphingWithConnections` checks element count only; `Scene::elements()` structurally excludes `Connection`s so it can't reflect wire survival.
- **medium** `Tests/Unit/Scene/TestConnectionManager.cpp:24` -- `testConnectionCreation`/`testConnectionDeletion` byte-for-byte identical, tautological (`GraphicElement::inputs()` is a fixed structural vector unaffected by `connect()` success).
- **medium** `Tests/Unit/Scene/TestConnectionManager.cpp:38` -- `testConnectionValidation` never exercises `ConnectionManager`'s validation logic at all.
- **medium** `Tests/Unit/Scene/TestConnectionManager.cpp:67` -- `testMultiPortConnection` never creates any connection, let alone a fan-out.
- **medium** `Tests/Unit/Scene/TestSceneDropHandler.cpp:19` -- `handleNewElementDrop()` and `addFromMimeData()` are entirely untested anywhere in the suite.
- **medium** `Tests/Unit/Scene/TestConnectionValidity.cpp:44` -- `testConnectionStatusValid` never verifies the required-input port itself recovers from `Status::Error` once wired, even though the test's own setup demonstrates the recovery.
- **medium** `Tests/Unit/Scene/TestConnectionValidity.cpp:85` -- `testPortDeletionDeletesConnection` never verifies the surviving input port reverts to `Status::Error` after its sole connection is torn down.
- **low** `Tests/Unit/Scene/TestConnectionValidity.cpp:142` -- `testConnectionWithDisconnectedPorts` duplicates `testPortValidityWithConnections` with no differentiating assertion.
- **low** `Tests/Unit/Scene/TestSceneConnections.cpp:257` -- `testValidConnectionBetweenInputOutput` only re-checks values it just set itself.
- **low** `Tests/Unit/Scene/TestScene.cpp:1593` -- comment claims an ordering check (`orIndex < andIndex`) the code never performs; only presence (`>=0`) is checked.
- **low** `Tests/Unit/Scene/TestSceneState.cpp:731` -- position assertions loosened to `>=` where the grid-snap result is exactly computable (`(50,50)` unsnapped, `(152,152)` snapped).
- **low** `Tests/Unit/Scene/TestScene.cpp:2399` -- `testCheckWireIdleRestoreRestoresAfterBothWindowsElapse` waits 3100ms against a binding 3000ms threshold -- only ~3.3% margin, flaky-prone on a loaded CI runner.

### Unit/Serialization (7)
- **medium** `Tests/Unit/Serialization/TestSerialization.cpp:1210` -- `testMismatchedElementCount` never calls `workspace.load(...)`; the count-mismatch code path is never actually exercised.
- **medium** `Tests/Unit/Serialization/TestSignalDelegate.cpp:27` -- `SignalDelegate::segmentFor()`'s Low/Rising/Falling branches and the input-vs-output color path are never exercised (both tests only ever hit the High branch on an input row).
- **medium** `Tests/Unit/Serialization/TestDolphinZoom.cpp:1` -- only `fitScreen()`'s two degenerate-geometry no-op branches are tested; `zoomIn/Out/reset/apply/canZoomIn/canZoomOut` are entirely untested.
- **low** `Tests/Unit/Serialization/TestSerialization.cpp:1265` -- `testMultiGateChainSerialization`'s final check passes for any of the 7 fixture labels regardless of load order.
- **low** `Tests/Unit/Serialization/TestSerialization.cpp:1348` -- `testConnectionWithDeletedPorts` never actually deletes any port; misleading name, redundant with `testConnectionPersistence`.
- **low** `Tests/Unit/Serialization/TestSerialization.cpp:966` -- `WorkSpace::scene() != nullptr` repeated as the main check in 5 tests; structurally can never be null.
- **low** `Tests/Unit/Serialization/TestSerialization.cpp:952` -- comment claims precise control over which field is corrupted by flipping specific byte offsets, but the payload is `qCompress()`-ed, so the offsets land inside the compressed blob, not the claimed field.

### Unit/Simulation (5)
- **medium** `Tests/Unit/Simulation/TestSimulation.cpp:266` -- `Application::interactiveMode` leaks to every later test if either of two `QVERIFY2`s fails before the plain restore statement. (See systemic pattern #1.)
- **medium** `Tests/Unit/Simulation/TestDanglingPointer.h:103` -- the file's own docstring/design guarantee ("this test sits last so a crash only affects it") is violated: two more test slots (`hcDrainConnectionsMustCleanRegistry`, `jd_cancelledWireMustNotLeaveDanglingPointer`) are declared after `integration_simulationTickAfterResetMustNotCrash`.
- **medium** `Tests/Unit/Simulation/TestSimulationBlocker.cpp:18` -- `testRAIIPatternStopsSimulation`/`testRAIIPatternRestartsSimulation` run the identical sequence; the genuinely untested branch (a blocker constructed on an already-stopped simulation, standalone) is never covered.
- **low** `Tests/Unit/Simulation/TestSimulation.cpp:291` -- throttle-interval test never checks the flush is still *pending* on intermediate ticks, only the final state -- a bug that fires early (e.g. tick 2 instead of 5) would still pass.
- **low** `Tests/Unit/Simulation/TestDanglingPointer.cpp:224` -- balanced-brace function-body extraction regex is copy-pasted 4 times with diverging correctness; a later copy fixed a nested-paren bug that was never propagated back to the earlier 3.

### Unit/Tour (4)
- **medium** `Tests/Unit/Tour/TestTourEngine.cpp:342` -- `fillTourStepFields()`'s `target`/`click` JSON parsing is never exercised through `TourEngine::loadFromResource()`; every `TourStep.target` used in the file is hand-set, not round-tripped.
- **medium** `Tests/Unit/Tour/TestTourEngine.cpp:671` -- test name/comment promise a horizontal "shifted right" callout check; only `y()` is asserted, `x()` never read.
- **medium** `Tests/Unit/Tour/TestTourEngine.cpp:313` -- "MissingIdOrTitleFails" only exercises the missing-**id** half of the combined `||` guard, never missing-title.
- **low** `Tests/Unit/Tour/TestTourEngine.cpp:235` -- `testGoToPreviousStepAtFirstStepIsNoOp` relies on a leftover persisted `Settings::tourProgress` from the preceding test instead of resetting it itself; declaration-order fragile.

### Unit/Ui (5)
- **high** `Tests/Unit/Ui/TestSelectionCapabilities.cpp:11` -- `computeCapabilities()` (~30 output fields, dozens of branches) is almost entirely untested: only an empty-selection check and a trivial two-identical-Demux check exist. AND-reduction, mixed-type/mixed-value consensus flags, mixed embedded/file-backed IC, TruthTable, and InputButton selections are all untested. **Fix:** add mixed-value, mixed-type, mixed-IC, TruthTable, and InputButton selection tests.
- **medium** `Tests/Unit/Ui/TestElementContextMenu.cpp:201` -- flip-action test only checks undo-stack count, never `isFlippedX()`/`isFlippedY()` or that the correct axis flipped.
- **medium** `Tests/Unit/Ui/TestUpdateController.cpp:259` -- `testDownloadUpdateCanceledSkipsWarning` never verifies the progress dialog/cancel action actually happened before asserting the negative outcome; would pass identically if the dialog never appeared.
- **low** `Tests/Unit/Ui/TestICController.cpp:800` -- three ~45-line near-identical conflict-resolution test bodies differing only in which button is clicked.
- **low** `Tests/Unit/Ui/TestWorkspaceManager.cpp:180` -- two ~40-line near-identical "close tab anyway" test bodies, same pattern.

### Unit/Wiring (6)
- **medium** `Tests/Unit/Wiring/TestConnections.cpp:182` -- `testRejectDuplicateConnection` never creates a duplicate connection or checks any rejection; `connectionRejectionReason()`'s "already connected" guard is never invoked.
- **medium** `Tests/Unit/Wiring/TestConnections.cpp:169` -- `testRejectSelfConnection` never attempts a self-connection; the real self-connection guard is never called.
- **medium** `Tests/Unit/Wiring/TestConnections.cpp:131` -- `testRejectOutputToOutput`/`testRejectInputToInput` only demonstrate C++ `dynamic_cast` semantics (compile-time-guaranteed), not application-level rejection.
- **medium** `Tests/Unit/Wiring/TestConnectionSerialization.cpp:152` -- `portMap` keyed by a raw pointer address, not the project's packed serial-ID format (`Port::makeSerialId`), so the "known" port entry can never be matched by `load()` -- both ports end up null, not just the intentionally-missing one. **Fix:** key with `Port::makeSerialId(...)`.
- **medium** `Tests/Unit/Wiring/TestPort.cpp:1` -- large parts of `Port`'s public API (`name()`, `setName()`, `hoverEnter/Leave()`, `updateConnections()`, `itemChange()`, `setDefaultStatus()`, `defaultValue()`, `makeSerialId()`, `globalIndex()`) have zero direct test coverage; file has only 4 test functions.
- **low** `Tests/Unit/Wiring/TestConnections.cpp:373` -- `testMultiConnectionStatus` is a strict subset of `testOutputFanOut`'s coverage with Led substituted for And.

### Integration (42)
- **high** `Tests/Integration/TestMainWindowGui.cpp:889` -- `testFlipHorizontallyViaKeyboard` checks `rotation()`, a property horizontal flip never touches (`setFlippedX()` never calls `setRotation()`); a completely dead flip feature would pass undetected. **Fix:** assert `isFlippedX()`.
- **high** `Tests/Integration/TestIc.cpp:523` -- `testICNestedCircuitSignalPropagation` never runs a simulation or drives any input despite its name/docstring; only proves an unconnected IC output starts `Unknown`.
- **high** `Tests/Integration/TestSystemVerilogExport.cpp:115` -- `initTestCase()` skips the entire 76-test class on non-Linux, even though 70+ tests never touch the gated `iverilog`/`yosys`/`verilator` calls by default. (Systemic #8.)
- **high** `Tests/Integration/TestWorkspace.cpp:74` -- `testAutosaveSkippedWhenClean` never checks autosave state at all; structurally cannot fail regardless of correctness.
- **high** `Tests/Integration/TestWorkspace.cpp:144` -- `testAutosaveAfterElementModify` reuses the exact bare-`QUndoCommand` anti-pattern this same file documents as a fixed bug elsewhere; a bare `QUndoCommand`'s no-op `redo()` never triggers real autosave logic.
- **high** `Tests/Integration/TestWorkspace.cpp:163` -- `testAutosaveAfterElementDelete`, same bare-`QUndoCommand` gap.
- **high** `Tests/Integration/TestWorkspace.cpp:563` -- `testAutosaveInCurrentDirForSavedProject` performs a save and asserts nothing about the outcome.
- **high** `Tests/Integration/TestFeedback.cpp:183` -- `testRingOscillatorWarningAfterMaxIterations` has literally zero meaningful assertions (`scene != nullptr` is the only check).
- **medium** `Tests/Integration/TestICInline.cpp:3176` -- `testFlipRotateEmbeddedIC` never exercises `RotateCommand` at all, and its inline comments swap the two `FlipCommand` axes (labels "Flip horizontally" on axis 1, "Rotate" on a second flip).
- **medium** `Tests/Integration/TestICInline.cpp:4081` -- `testEmbedICByDropCollisionAutoSuffix` never calls `ICRegistry::uniqueBlobName()`; hardcodes the expected `"simple_and_2"` literal instead of asserting the real generator's output.
- **medium** `Tests/Integration/TestMainWindowGui.cpp:1431` -- "Default skin" test never checks the skin actually reverted; only checks `isSelected()`.
- **medium** `Tests/Integration/TestMainWindowGui.cpp:928` -- secondary-property-cycle test only checks selection, never a property value.
- **medium** `Tests/Integration/TestMainWindowGui.cpp:2902` -- "Context menu" tests (`testContextMenuRotateLeft/Right/Copy/Cut/Delete/MorphTo`) never open a context menu at all; drive keyboard shortcuts identical to (and weaker than) other named tests.
- **medium** `Tests/Integration/TestIc.cpp:551` -- "File dependency resolution" test loads `jkflipflop.panda`, a fixture explicitly documented elsewhere in the same file as non-nested.
- **medium** `Tests/Integration/TestIc.cpp:146` -- `inputSize()>=0`/`outputSize()>=0` tautological.
- **medium** `Tests/Integration/TestIc.cpp:417` -- required/optional port classification is tallied but no specific port's classification is checked against an expected value.
- **medium** `Tests/Integration/TestIc.cpp:310` -- claims to verify "input changes result in output changes" but the switch-ON transition's effect on the LED is never captured or asserted (only OFF-vs-OFF idempotency is checked).
- **medium** `Tests/Integration/TestIc.cpp:206` -- temp filename uniqueness relies on wall-clock milliseconds inside the real, shared source-tree `Examples/` directory -- flaky-prone under concurrent runs.
- **medium** `Tests/Integration/TestArduino.cpp:2467` -- functional simavr validation for all Level 1-4 IC export tests gated entirely behind a manual env var, never in normal CI. (Systemic #8.)
- **medium** `Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.cpp:273` -- "VERIFY WRITE" loop's comment promises a check the body never performs -- no read/assert at all.
- **medium** `Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.cpp:898` -- OR-based "violation" assertions accept two outcomes with no justification or re-sample; one row's name is also mislabeled (`we_setup_violation`, not `we_hold_violation`). (Systemic #2.)
- **medium** `Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.cpp:418` -- 22-way if/else chain dispatched by positional integer index, fragile and hard to audit. (Systemic #7.)
- **medium** `Tests/Integration/TestSystemVerilogExport.cpp:188` -- `testWirelessNodeGeneration` only rules out one previously-seen bug's output (`1'b0`), never verifies the actually-correct generated variable name.
- **medium** `Tests/Integration/TestWorkspace.cpp:891` -- `testAutosaveListCorrectAfterCleanup` asserts a condition trivially true regardless of correctness (compares against a path that could never have been an autosave entry).
- **medium** `Tests/Integration/TestWorkspace.cpp:282` -- `contains("autosaves") || contains(".panda")` trivially defeated by the `.panda` half. (Systemic #2.)
- **medium** `Tests/Integration/TestWorkspace.cpp:782` -- `<=` comparison plus a non-command element add means the test can pass without ever exercising autosave removal.
- **medium** `Tests/Integration/IC/Tests/TestsWithoutPanda/TestSequential.cpp:116` -- `testRegisterAsyncClear` never verifies the register held the loaded value before clearing (no "beforeClear" check, unlike sibling tests).
- **medium** `Tests/Integration/Logic/TestMuxDemuxComprehensive.cpp:754` -- chained Mux→Demux test only exercises the diagonal case where data index, mux select, and demux select are all equal.
- **medium** `Tests/Integration/TestWorkspaceFileops.cpp:343` -- `testLoadCorruptedFileHandling` contains zero assertions; always passes.
- **medium** `Tests/Integration/TestFeedback.cpp:51` -- `testSetResetPriorityInSRLatch` never drives S/R differently or reads Q/Q'; `verifyStableState` is only an idempotency check.
- **medium** `Tests/Integration/TestFeedback.cpp:67` -- `testInitialStateDependency` never gives its two scenes different initial states.
- **medium** `Tests/Integration/TestFeedback.cpp:161` -- `testConvergenceSpeedVariation` never reads or compares any iteration count despite the name.
- **medium** `Tests/Integration/TestFeedback.cpp:225` -- `testWarningMessageContent` never inspects any message; duplicates 3 other tests via a different code path.
- **medium** `Tests/Integration/IC/Tests/TestLevel6Register8bit.cpp:486` -- `QCOMPARE(pass_count, total_scenarios)` tautological. (Systemic #3.)
- **medium** `Tests/Integration/IC/Tests/TestLevel6Register8bit.cpp:103` -- `test8BitRegister()`, 175-line, 6-scenario giant function. (Systemic #6.)
- **low** `Tests/Integration/TestICInline.cpp:1961` -- `testOnChildICBlobSavedCorruptBlob` (and multi-target sibling) hedge with if/else instead of asserting the deterministic throw, unlike `testChildBlobSavedPartialFailureRollback`'s unconditional `QVERIFY2(threw, ...)` on the same input.
- **low** `Tests/Integration/TestICInline.cpp:3764` -- `testICDropZoneDropEventSignals` duplicates two-thirds of `testICDropZoneWiredInUI`.
- **low** `Tests/Integration/TestMainWindowGui.cpp:2429` -- closing the last tab never checked for its actual resulting tab count.
- **low** `Tests/Integration/TestMainWindowGui.cpp:2179` -- near-duplicate of `testEmbeddedICCopyPaste` with no added coverage.
- **low** `Tests/Integration/TestMainWindowGui.cpp:3326` -- empty-project `makeSelfContained` test ends on a near-tautological assertion.
- **low** `Tests/Integration/TestArduino.cpp:2523` -- 24 copy-pasted one-line test functions should be one data-driven test. (Systemic #7.)
- **low** `Tests/Integration/TestArduino.cpp:1731` -- `createConnection()`'s possible null return never checked (`unique_ptr` wraps it directly, no `QVERIFY`).
- **low** `Tests/Integration/TestArduino.cpp:1771` -- embedded-IC test doesn't verify the IC's internal logic (`^`/`&&` for a half-adder) actually appears inlined; only checks the function signature exists.
- **low** `Tests/Integration/TestSimulation.cpp:326` -- near-identical circuit-construction boilerplate copy-pasted across 5 determinism tests. (Systemic #7.)
- **low** `Tests/Integration/TestWorkspaceFileops.cpp:623` -- `QVERIFY2` failure message copy-pasted from an unrelated autosave test.
- **low** `Tests/Integration/TestWorkspaceFileops.cpp:111` -- duplicate identical assertion; comment describes a size-diff check that was never implemented.
- **low** `Tests/Integration/TestWorkspaceFileops.cpp:340` -- "scene elements should be cleared" assertion is trivially true since nothing was ever populated.
- **low** `Tests/Integration/IC/Tests/TestsWithoutPanda/TestSequential.cpp:473` -- two data rows ("release_to_idle"/"second_trigger") are misleadingly-named duplicates of two earlier rows.
- **low** `Tests/Integration/IC/Tests/TestsWithoutPanda/TestSequential.cpp:75` -- `advanceFsm()` helper defined but never called anywhere in the file.
- **low** `Tests/Integration/TestFeedback.cpp:688` -- `verifyConvergence`'s `shouldConverge=false` branch is dead code; no caller ever passes `false`.
- **low** `Tests/Integration/IC/Tests/TestLevel9SingleCycleCpu8bit.cpp:205` -- `cleanup()` is a no-op unlike every sibling IC test file, leaving several tests order-dependent.
- **low** `Tests/Integration/IC/Tests/TestLevel9FetchStage16bit.cpp:377` -- `testSrcBitsMatchInstruction`'s expected `SrcBits` is derived from the IC's own `Instruction` output rather than the ground-truth programmed word.

### Integration/IC (34)
- **high** `Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.cpp:1266` -- "address_change_midwrite" data row asserts nothing at all (no read, no `QCOMPARE`); passes unconditionally regardless of correctness.
- **high** `Tests/Integration/IC/Tests/Cpu/TestCPUInstructionExecute.cpp:51` -- ALU status flags (`zeroFlag`/`signFlag`/`carryFlag`/`overflowFlag`) are built and wired but never asserted anywhere in the file, including the `exec_flags_carry` row.
- **high** `Tests/Integration/IC/Tests/TestLevel4RippleAdder4bit.cpp:122` -- `CarryIn` input wired but always driven `false`; carry-propagation path genuinely untested. (Systemic #4.)
- **medium** `Tests/Integration/IC/Tests/TestLevel4RippleAlu4bit.cpp:73` -- `CarryIn`/`SubCarryIn` pinned to fixed values across every test in the file. (Systemic #4.)
- **medium** `Tests/Integration/IC/Tests/Cpu/TestCPURegisters.cpp:236` -- `writeEnable=0` while `decoderOut=1` never exercised. (Systemic #4.)
- **medium** `Tests/Integration/IC/Tests/TestLevel6Ram8x8.cpp:136` -- no WriteEnable-gating or address-isolation test. (Systemic #4.)
- **medium** `Tests/Integration/IC/Tests/TestLevel9RegisterFile32x16.cpp:67` -- `writeEnable=0` (write-inhibited) path never exercised. (Systemic #4.)
- **medium** `Tests/Integration/IC/Tests/TestsWithoutPanda/TestRamCell1bit.cpp:78` -- `testWriteOneHoldWeDisabled` is a byte-for-byte duplicate of `testDataChangeWithoutWe`.
- **medium** `Tests/Integration/IC/Tests/Cpu/TestCPUMemoryInterface.cpp:20` -- `writeAndVerify()`'s doc comment claims it's shared by both tests to avoid duplication; only one test calls it, the other reimplements the identical body inline.
- **medium** `Tests/Integration/IC/Tests/TestLevel3Register1bit.cpp:85` -- `test1BitRegister()`, 6-scenario giant function with cascading-failure risk. (Systemic #6.)
- **low** `Tests/Integration/IC/Tests/TestLevel8MemoryStage.cpp:98` -- `MemRead`+`MemWrite` both-asserted combination never tested. (Systemic #4.)
- **low** `Tests/Integration/IC/Tests/TestLevel9MemoryStage16bit.cpp:102` -- same gap, 16-bit sibling. (Systemic #4.)
- **low** `Tests/Integration/IC/Tests/TestLevel6StackPointer8bit.cpp:144` -- push/pop wraparound at the 0x00/0xFF boundary never tested.
- **low** `Tests/Integration/IC/Tests/TestLevel2Decoder2to4.cpp:139` -- `testMutualExclusivity` is a strict subset of `testSequentialScan`.
- **low** `Tests/Integration/IC/Tests/TestLevel2Decoder5to32.cpp:115` -- `test5to32DecoderMaxAddress` duplicates rows already covered by the exhaustive data-driven test.
- **low** `Tests/Integration/IC/Tests/TestLevel4JohnsonCounter4bit.cpp:114` -- `testJohnsonCounter_data` stops one cycle short of the documented full-sequence wrap.
- **low** `Tests/Integration/IC/Tests/TestLevel5LoadableCounter4bit.cpp:227` -- `testBinaryCounterHoldBehavior_data` declares a `description` column that's never fetched/used.
- **low** `Tests/Integration/IC/Tests/TestLevel2Mux4to1.cpp:110` -- `'sequential'`/`'pattern_propagation'` data rows execute byte-for-byte identical code.
- **low** `Tests/Integration/IC/Tests/TestLevel5Controller4bit.cpp:132` -- unexplained 20-iteration magic-number settle loop, no comment, contrasts with every other purely-combinational IC test's single `sim->update()`.
- **low** `Tests/Integration/IC/Tests/Cpu/TestCPUAlu.cpp:166` -- two rows in `testALU8bit_data()` duplicate rows already better-covered by `testALU8bitFlags_data()`.
- **low** `Tests/Integration/IC/Tests/TestLevel3AluSelector5way.cpp:122`, `TestLevel2PriorityMux3to1.cpp:108`, `Cpu/TestCPUDecoders.cpp:107` -- hand-rolled test-case loops with unused diagnostic `name` field. (Systemic #5.)
- **medium** `Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.cpp:418` -- 22-way positional if/else dispatch, fragile/hard to audit. (Systemic #7.)
- **medium** `Tests/Integration/IC/Tests/TestLevel6Register8bit.cpp:486`/`:103` -- see Integration section (tautological pass_count / giant function).
- (Remaining Level5RegisterFile4x4/TestSequential/TestMuxDemuxComprehensive entries listed under Integration above, sharing this directory.)

### System (8)
- **medium** `Tests/System/TestBewavedDolphinGui.cpp:640` -- `saveToTxt()` output checked only via `contains('0')`/`contains('1')` character presence, not the "exact pattern" comments claim.
- **medium** `Tests/System/TestBewavedDolphinGui.cpp:691` -- `testSaveAndLoadWaveform` never actually loads anything back; only content check is a line-count floor.
- **medium** `Tests/System/TestBewavedDolphinGui.cpp:1539` -- real process stdin redirected via `testUtilsFreopen()`, never restored. (Systemic #1.)
- **medium** `Tests/System/TestBewavedDolphinGui.cpp:497` -- `testShowNumbers`/`testShowWaveforms` only assert `isEnabled()`, a tautology since nothing ever disables these actions.
- **medium** `Tests/System/TestBewavedDolphinGui.cpp:1767` -- `Application::interactiveMode` mutated without RAII guard, leaks on assertion failure. (Systemic #1.)
- **medium** `Tests/System/TestBewavedDolphinGui.cpp:1976` -- same unguarded mutation, second site. (Systemic #1.)
- **low** `Tests/System/TestBewavedDolphinGui.cpp:1170` -- 3 raw `createDolphin()` pointers (`:1170,1209,1575`) never deleted/wrapped in `unique_ptr`, unlike ~56 other call sites in the same file.
- **low** `Tests/System/TestBewavedDolphinGui.cpp:1680` -- `testTrivialAccessors` bundles 5 unrelated behaviors into one function. (Systemic #6.)

### Fuzz (1)
- **high** `Tests/Fuzz/FuzzUndo.cpp:101` -- `splitableConn` becomes a dangling pointer after a `DeleteItemsCommand` frees the `Connection` it points to (via `m_ids`, resolved and deleted in `DeleteItemsCommand::redo()`), then is dereferenced by the `if (!splitableConn || !splitableConn->scene())` guard itself -- a genuine use-after-free in the fuzz harness that would plausibly produce spurious ASan aborts misattributed to `SplitCommand`/`Connection`. **Fix:** re-resolve the connection from the scene by id before each use instead of caching a raw pointer across command executions, or clear the cached pointer whenever a delete targeting it is issued.

### Common (harness) (2)
- **medium** `Tests/Common/TestUtils.h:364` -- `ScopedStdinRedirect`/`ScopedStdoutCapture` don't check `fopen()`'s result before passing it to `fileno()`; a bad fixture path is UB (crash) instead of a legible `qFatal`, inconsistent with the rest of the file's guard style. (Systemic #1.)
- **low** `Tests/Common/TestUtils.cpp:266` -- `AutoDismisser::clickMessageBoxButton()` does an exact-text match with no mnemonic-stripping fallback; would break for any future `"Yes"`/`"No"`-style call (Qt renders those with an `&` mnemonic), though neither of the two current real call sites (`"Switch to Tab"`, `"Save"`) is actually affected today.

## Verification Notes

- Several findings were **narrowed rather than dropped** on re-verification, and the narrowing itself is useful context for maintainers: `TestSettings.cpp:47` (Core) initially read as a broad Settings-untested-everywhere gap, but a sibling file (`Unit/Common/TestSettings.cpp`) already covers most of that ground -- only the `Theme::System` upper-bound and the `hideV4Warning` remove-vs-store asymmetry are genuinely open. Similarly `TestExerciseEngine.cpp:516`'s "signals never spied on" claim turned out to be half-covered by a shared code path (`performAdvance()`) already exercised by another test; only the `stepChanged` half of the gap is real.
- A few findings' **illustrative examples were factually wrong** even though the underlying issue was real -- worth a note so nobody re-derives the same wrong example: `TestUtils.cpp:266`'s "OK"/"&OK" mnemonic scenario doesn't occur in Qt 6.9.3 (only Yes/No-family buttons get mnemonics, empirically confirmed); `TestIC.cpp:40`'s claim about a `CircuitBuilder::inputPortByLabel` API was fabricated (no such class exists); `TestConnectionSerialization.cpp:152`'s root-cause mechanism was subtly mis-described (the connection's id lands in `m_ids`, not `m_otherIds`).
- Several **counts were off by one or more** and were corrected without changing severity: `TestElementFactory.cpp:28` (32 real buildable types, not 33), `TestArduino.cpp:2467`/`:2523` (24 one-liner tests, not ~23), `TestBewavedDolphinGui.cpp:1170` (~56-57 correctly-wrapped call sites, not "~70"), `TestLevel5RegisterFile4x4.cpp:898` (one violation row is actually named `we_setup_violation`, not `we_hold_violation`).
- One finding flagged a **live-looking vulnerability that turned out not to be one**: `TestFileHandlerSecurity.cpp:41`'s negative-padding gap is a real coverage gap, but production code already clamps the value safely today (`std::clamp(..., 0, kMaxExportPadding)`), so there's no active exploit path -- just a missing regression test for that safety net.
- A pattern that **looks wrong but is intentional** and might deserve a clarifying comment: `Tests/Integration/TestSimulation.cpp`'s `testWirelessDuplicateTxIgnored` deliberately accepts either of two outcomes via `QVERIFY(a || b)` -- but unlike the `TestLevel5RegisterFile4x4.cpp` OR-assertions flagged above, it documents *why* either is acceptable and re-samples to confirm the observed value is stable. That's the right way to write a legitimately-nondeterministic assertion; the flagged instances elsewhere in the suite use the same shape without either safeguard.
- `TestICHandler.cpp:216`'s finding was narrowed from "4 tests share this resource-leak risk" to just 1: on POSIX, a *file's* own permission bits don't gate its removal, only the *containing directory's* write bit does -- so the 3 sibling tests that narrow file (not directory) permissions don't actually share the leak.
<!-- END GENERATED REPORT -->
