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

## Phase B — Flaky-test stabilization

`project_flaky_parallel_tests` memory (41 days old) reported `TestAudioBox`/`TestComponents`
flaking under full parallel `ctest`. Re-verified rather than trusted: ran `ctest --preset
debug` (the project's own `-j 8` default) 7 times consecutively, plus 3 more oversubscribed at
`-j 16` to increase contention pressure — all 10 runs clean, 216/216, 0 failures. Could not
reproduce at all on the current tree (test count has grown from 192 to 216 since that memory
was written, with a full coverage sweep landing in between — plausible something incidental
along the way fixed it, or the original observation was tied to a since-changed environment
characteristic).

Deliberately did **not** add a speculative `RESOURCE_LOCK` or other serialization fix: there
was nothing currently observable to fix, and inventing one on faith would be exactly the kind
of unverified patch this project's own conventions warn against. Memory updated
(`project_flaky_parallel_tests.md`) to record the non-reproduction rather than silently
carrying a stale claim forward. If this resurfaces, the next step is a real failing repro with
`--output-on-failure` before touching `CMakeLists.txt`'s test scheduling.

## Phase C — clang-tidy static pass

`CMAKE_EXPORT_COMPILE_COMMANDS` was already `ON`; no CMake change needed there. First blocker:
the default `debug`/`build` compile database only has per-*unity-batch* entries (8 files
merged per translation unit), which clang-tidy can't parse per-file (tried it: 25k+ bogus
warnings and `QObject` "file not found" errors from a broken fallback command). `build-coverage`
already configures `ENABLE_UNITY=OFF`/`ENABLE_PCH=OFF` for its own reasons, giving a real
per-file `compile_commands.json` for free — reused it via `-p build-coverage` rather than
standing up a new preset. GCC's compile flags needed `--extra-arg=-Wno-unknown-warning-option`
(clang doesn't recognize `-Wduplicated-branches` etc.) but otherwise worked directly; no
separate Clang toolchain build was needed for this phase (unlike the Phase D spike, which
needs Clang for Mull itself).

Added `.clang-tidy` (`bugprone-*`, `performance-*`, a curated `readability-*` subset), scoped
to `Tests/` via `HeaderFilterRegex`. First full run (`run-clang-tidy -p build-coverage`, 218
files): 372 raw findings across 17 check categories. Read every category's findings in
context — most concentrated in a handful of files/patterns rather than 372 independent issues.

**Fixed** (all verified via rebuild + 2 clean full-suite runs):
- `bugprone-reserved-identifier` (184 duplicate reports, 1 real site): `TestUtils.h`'s
  `_QUOTE` helper macro renamed to `QUOTE_IMPL`.
- `readability-container-size-empty` (24 sites, 15 files): `.size() > 0`/`>= 1` → `!...isEmpty()`,
  matching this project's established Qt idiom (not bare `.empty()`).
- `readability-redundant-casting` (9 sites, 1 file — `TestConnectionSerialization.cpp`):
  removed no-op `dynamic_cast<OutputPort*>(outputPort)`-shaped casts where the source
  expression was already exactly that type. Left the *other*, superficially similar casts in
  the same file untouched (clang-tidy didn't flag them — different declared types there, real
  casts).
- `performance-unnecessary-copy-initialization` (8 sites, 2 files): local `QByteArray`/`QString`
  copies that are never modified afterward → `const &`, after checking each downstream call
  site takes its argument by `const &` too.
- `performance-implicit-conversion-in-loop` (7 sites, 4 files, all MCP handler tests):
  `for (const QJsonValue &v : x)` → `for (const auto &v : x)`.
- `performance-no-automatic-move` (5 sites, 3 files): local fixture-writer helpers' `const
  QString path` return values de-const'd so NRVO/move applies on `return path;`.
- `bugprone-integer-division`, `bugprone-implicit-widening-of-multiplication-result` (2 of 4
  total instances; the other 2 below): `TestTruthTable.cpp`'s `key.setBit(i * 100, ...)` given
  an explicit `static_cast<qsizetype>(i) * 100` (real, if currently-unreachable, overflow
  hardening for a small loop counter).
- `readability-simplify-boolean-expr`, `readability-redundant-member-init`,
  `readability-duplicate-include` (1 each): simplified an `if/return false; return true;` pair
  to a direct `return`, dropped a redundant `{}` on an already-default-constructing member,
  removed a genuine duplicate `#include`.

**Reviewed and excluded via `.clang-tidy` with documented reasoning** (not blanket-suppressed
without cause — each spot-checked against real code first; full reasoning inline in the config
file's comments):
- `bugprone-empty-catch` (12 sites): this suite's established "reaching here without an ASan
  abort is the pass condition" pattern (`TestDanglingPointer.cpp` and several fuzzed-input
  survival tests) plus exception-to-return-value adapter helpers — every spot-checked instance
  was this shape, not a real hidden bug.
- `bugprone-pointer-arithmetic-on-polymorphic-object` (7 sites, 1 file): indexing into real,
  plain arrays of a polymorphic element type (`InputSwitch addressBits[6];`) — always safe
  since the array's static and dynamic type can never differ; the check can't distinguish this
  from indexing through a base-class pointer.
- `bugprone-unchecked-optional-access` (3 sites): doesn't recognize `QVERIFY(x.has_value())`
  as a narrowing guard the way it would `if`/`assert` — confirmed all 3 sites have the QVERIFY
  immediately before the dereference.
- `performance-enum-size` (60 duplicate reports, 5 real sites): real but negligible for
  test-only CPU-ISA helper enums; not worth the churn for a test-*quality* pass.
- `readability-redundant-access-specifiers` (45 duplicate reports, ~22 real sites): flags
  `private slots:` immediately after a plain `private:` as redundant since both are
  C++-level `private` — but they are **not** redundant in Qt code (`slots` registers the
  following methods with moc); blindly "fixing" this would silently break slot registration.

**2 remaining false positives NOLINT'd individually** (not worth disabling the whole check for
one instance each): `TestSerialization.cpp`'s deliberate grid-layout integer division, and
`TestUpdateChecker.cpp`'s compile-time-constant `64 * 1024` (provably can't overflow).
`TestWorkspaceManager.cpp`'s `bugprone-branch-clone` (two distinct dialog texts that both need
the same "click Yes" response — not a copy-paste bug) also NOLINT'd.

No production bugs found this phase either — every fix was test-code quality/perf, and the
false-positive categories were genuinely false, not disguised real issues (confirmed by
reading the actual code at each site, not assumed from the check name).

Final state: a clean `run-clang-tidy -p build-coverage` pass over all of `Tests/` reports
**zero** findings. Full 216-test suite green across 2 consecutive `ctest --preset debug` runs
after the fixes landed.
