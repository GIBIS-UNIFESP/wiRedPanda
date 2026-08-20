# Test Quality Audit

Companion effort to `.claude/COVERAGE_100_PLAN.md`, once that sweep reached literal 100% line
coverage for `App/`. Coverage only proves a line *ran* during some test — it says nothing
about whether the test that ran it actually asserted the right thing. Tracked here per the
approved plan (`/home/torres/.claude/plans/now-we-have-to-inherited-riddle.md`): a
weak-assertion audit (this doc), flaky-test stabilization, a clang-tidy pass, and a
mutation-testing (Mull) spike.

## Phase A — Weak-assertion / test-smell audit

Grepped for `at minimum`, `no crash`, `may or may not`, and bare `QVERIFY(true)` across
`Tests/*.cpp`: 32 comment hits + 7 bare `QVERIFY(true)`, out of ~3661 test methods. Read every
hit in context; most were false positives (comment framing followed by real assertions).
Genuine gaps found and fixed:

| File | Test | What was weak | Fix |
|------|------|----------------|-----|
| `Tests/Unit/Elements/TestElementAppearance.cpp` | `testRotatedMalformedSvgFallsBackToBasePixmap`, `...Content` | `paint()` called with zero assertion on the result | Reused the existing `TestUtils::pixmapHasInk()` helper (already used elsewhere in the same file) to assert real ink was drawn. **Caught a real test-fixture bug in the process** (see below), not a production bug. |
| `Tests/Unit/Wiring/TestConnections.cpp` | `testConnectionPositionUpdate` | Only checked ports were still non-null after `updatePosFromPorts()`; never checked the connection's own geometry actually moved | Added `boundingRect()` before/after comparison. Required toggling `Application::renderingEnabled = true` around the test (off by default in the test binary — `updatePath()` is a no-op otherwise), mirroring `TestConnection.cpp`'s existing save/restore idiom. |
| `Tests/Unit/Scene/TestScene.cpp` | `testRotateWithConnections` | Named "WithConnections" but never created any real `Connection` — the before/after connection-count check was vacuously 0==0 | Added two real `Connection`s (input→gate, gate→output) via the same manual-construction idiom already used elsewhere in this file (`testSelectedElements`), then asserted both survive rotation with their ports intact. |
| `Tests/Integration/TestICInline.cpp` | `testSetInlineDataRollbackSimulationWorks` | Comment said "IC should still be fully functional" but never checked the actual simulated output | Added `TestUtils::inputStatus(&led)` check (simple_and.panda's AND-gate contract with both inputs high). **First attempt used `outputStatus()`, which QFATAL'd — `Led` has no output port; fixed to `inputStatus()`, matching the established idiom used elsewhere in the same file.** |
| `Tests/Integration/IC/Tests/TestLevel3Bcd7segmentDecoder.cpp` | `testBCD7SegmentDecoder` (invalid BCD rows 10-15) | Placeholder `-1` meant "any output 0x00-0x7F is acceptable" — a vacuous range check, since the value is mathematically guaranteed to be in that range by construction | Read `create_level3_bcd_7segment_decoder.py`: exactly 10 digit-detector AND gates (one per digit 0-9), each requiring an exact 4-bit match; no pattern for 10-15 matches any of them, so every segment output is provably 0x00 for all six invalid codes — not implementation-defined, a structural consequence of this specific circuit. Pinned the exact expected value; test still runs against the real fixture (would have failed if the analysis were wrong). |
| `Tests/Integration/IC/Tests/TestsWithoutPanda/TestSequential.cpp` | `testShiftRegisterAsyncClear` | Same vacuous 4-bit-range check as the BCD case | Derived the exact expected pre-clear value analytically (bit0 = most recently shifted-in bit, bit1 = the one before it, chain starts all-zero) and confirmed empirically via a temporary debug probe against all 4 data rows before pinning it as `QCOMPARE`. |
| `Tests/Unit/Serialization/TestSerialization.cpp` | `testCopyPandaFileTerminatesOnCircularMetadata` | Only proved the call *returned* (didn't hang) — termination alone doesn't prove the copy succeeded | Added `QFile::exists()` check on the real output path, matching the sibling test's pattern one function below it. |
| `Tests/Unit/Ui/TestCircuitExporter.cpp` | `testExporterCreation` | `QVERIFY(true)` after `renderToImage()` — only proved it didn't throw | Added `QFile::exists()` + non-zero-size checks on the real output PNG. |
| `Tests/Integration/TestMainWindowGui.cpp` | `testLanguageChange` | Explicitly said "may or may not change... at minimum verify no crash" | `LanguageManager::loadTranslation()` persists `Settings::language()` unconditionally before the `.qm`-resource lookup even happens — that's real and observable regardless of whether the resource exists (it doesn't, in the test binary). Asserted on `Settings::language()` transitions instead of the unreliable visual title. |
| `Tests/Integration/TestMainWindowGui.cpp` | `testUndoPastStack` | `QVERIFY(scene() != nullptr)` — near-tautological | Asserted the undo stack stays at index 0 / `canUndo() == false` through 5 no-op undo attempts, then proved the scene is still genuinely usable (add + undo a real command afterward). |
| `Tests/Integration/TestMainWindowGui.cpp` | `testStartExerciseDrivesClickTargetsAndOverlayParenting`'s guard scenario, and the Tour equivalent | `QVERIFY(isVisible())` after forcing the "overlay already gone" guard — only proved survival, not that the rest of the handler still worked | Added `currentStep()` before/after checks (both `ExerciseEngine`/`TourEngine` expose it) proving the guard only skips the overlay-touching code, not the whole handler. |
| `Tests/Integration/TestMainWindowGui.cpp` | `testClickTargetDrivesEachKnownId`'s unknown-id case | `clickTarget("not-a-real-target")` with zero assertion after | Asserted `scene->elements().size()` is unchanged — a real no-op check. |
| `Tests/Integration/TestMainWindowGui.cpp` | `testFullscreenToggle` | Hedged "may not apply in headless CI... no-crash is sufficient", with the untaken branch asserting nothing | Verified empirically (temporary debug probe) that F11 genuinely toggles `isFullScreen()` under offscreen QPA on Linux — the primary dev/CI target. Kept the graceful fallback structure for other platforms' CI (can't verify those from here) but gave the previously-empty branch a real assertion (`isVisible()`) instead of nothing. |

Reviewed and left as-is (false positives — real assertions already follow, or the "no crash"
framing is an accurate, precise statement of the actual contract, not a hedge):
`TestFeatures.cpp:141`, `TestSimulationBlocker.cpp:91`, `TestScene.cpp:443,733→now fixed,1563`,
`TestICInline.cpp:1358`, `TestSimulation.cpp:962,1514`, `TestMainWindowGui.cpp` (`testMinZoom`),
`TestDialogs.cpp:127`, all `TestMultiplexing.cpp` "at minimum size" hits (describe port-count
semantics, not test confidence), `TestICController.cpp:700` (`removeEmbeddedIC()` with no
current tab genuinely has no other observable side effect — confirmed by reading the
production guard), and all 7 `QVERIFY(true)` in `TestDanglingPointer.cpp` /
`TestSerialization.cpp:1747` (legitimate "reaching here without crashing/hanging *is* the pass
condition" death-tests for the Sentry WIREDPANDA-H2 regression cluster and a
cycle-termination guard respectively — explicitly documented as such in the surrounding code).

No production bugs found this phase — every fix was a test-quality gap, confirmed against
real production behavior (one fixture mistake in `TestICInline.cpp`'s first pass — wrong
helper function, `outputStatus()` vs `inputStatus()` — caught by the resulting QFATAL and
fixed before landing).

Full 216-test suite green across 2 consecutive `ctest --preset debug` runs.
