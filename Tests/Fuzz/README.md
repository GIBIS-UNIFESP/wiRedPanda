# Fuzzing wiRedPanda

A libFuzzer harness suite targeting wiRedPanda's binary parsers (`.panda`,
`.dolphin`, the clipboard stream, and embedded/file-backed IC blobs) and the
stateful subsystems reachable from a loaded circuit (code generation,
undo/redo). Every harness is Clang-only and built under AddressSanitizer +
UndefinedBehaviorSanitizer so that memory-safety and UB bugs abort the run on
inputs no human would hand-write.

## Harnesses

Twelve harnesses, registered in `CMakeLists.txt` under `if(ENABLE_FUZZER)`:

| Binary             | Source                | Target |
|--------------------|-----------------------|--------|
| `fuzz_deserialize` | `FuzzDeserialize.cpp` | The full `File > Open` path: `readPreamble` → `deserializeBlobRegistry` → `deserialize` element/connection loop. Ships a structure-aware custom mutator that keeps the corpus parseable. |
| `fuzz_ic_blob`     | `FuzzICBlob.cpp`      | `IC::loadFromBlob` directly (raw bytes + a structured path that builds a valid IC with boundary elements). |
| `fuzz_structured`  | `FuzzStructured.cpp`  | `deserialize` again, but `FuzzedDataProvider` synthesises valid `.panda` structure so the budget goes to semantic content, not recovering magic bytes. |
| `fuzz_waveform`    | `FuzzWaveform.cpp`    | beWavedDolphin `.dolphin` files: `readDolphinHeader` + `DolphinSerializer::loadBinary`. |
| `fuzz_clipboard`   | `FuzzClipboard.cpp`   | A clipboard pasted, undone and redone; a `.panda` file copied, pasted, duplicated and cut. |
| `fuzz_old_format`  | `FuzzOldFormat.cpp`   | The pre-4.1 flat binary path (`GraphicElement::loadOldFormat`) and its per-version / per-element-type branches. |
| `fuzz_ic_registry` | `FuzzICRegistry.cpp`  | `ICRegistry`: a `.panda` input as an IC's definition, placed from its file, reloaded when the file changes, embedded, renamed, extracted and created, each step undone and redone; other inputs as raw blob bytes through the blob API and `embeddedICs` metadata loading. |
| `fuzz_codegen`     | `FuzzCodeGen.cpp`     | The Arduino + SystemVerilog exporters (only reachable via `File > Export`), plus `generateTestbench`, on loaded `.panda` files and on circuits other inputs describe (every element type the exporters take, embedded and file-backed ICs). |
| `fuzz_round_trip`  | `FuzzRoundTrip.cpp`   | The save side: `WorkSpace::save()` and every element `save()`, then a reload, a second save and load: the application's own file must save to the same bytes, which surfaces save/load asymmetries. |
| `fuzz_ic_file`     | `FuzzICFile.cpp`      | File-backed IC loading (`IC::loadFromFile` → `loadFileDirectly`): a `.panda` input as an IC's file, beside every `.panda` file of the repository's examples and test circuits so the ICs it names resolve, loaded with migration on or off, reloaded through the registry's cache, simulated and saved; including cycle detection and old-format migration. |
| `fuzz_copy_panda`  | `FuzzCopyPanda.cpp`   | `copyPandaFile` + `readPreamble` and `File > Save As`: a `.panda` input beside the repository's circuits copied into a new folder (read-only for half the inputs) with the files its `fileBackedICs` name, named twice and at the end of chains up to the depth limit, then loaded and saved into that folder. |
| `fuzz_undo`        | `FuzzUndo.cpp`        | The undo/redo command system (`Commands.cpp`): a `.panda` input loaded beside the repository's circuits, or a small built circuit, given a random sequence of the editor's commands on its scene's undo stack (property edits merging in a session, blob registration, renaming, embedding and extraction among them), then the stack wound fully back and forward. |

Each `Fuzz*.cpp` opens with a `\file \brief` block describing its strategy and
input layout — read that before triaging a finding from it.

## Build

Clang is required (libFuzzer is a Clang-only feature):

```bash
cmake --preset fuzzer
cmake --build --preset fuzzer
```

This produces `build-fuzzer/fuzz_*`, each instrumented with libFuzzer +
AddressSanitizer + UndefinedBehaviorSanitizer. `ENABLE_FUZZER` puts libFuzzer's
coverage instrumentation (`-fsanitize=fuzzer-no-link`) on every target, so the
fuzzer is steered by the edges of the code a harness drives, not only the
harness's own; the callbacks come from each harness's `-fsanitize=fuzzer` link,
so the fuzzer presets build the harnesses alone.

## Seed corpus

Bootstrap a corpus from the project's example circuits — real `.panda` files
spanning every file-format version we still support, which gives the fuzzer a
wide, well-formed starting point to mutate from:

```bash
./Tests/Fuzz/build-seed-corpus.sh build-fuzzer/seed-corpus
```

`fuzz_ic_blob` runs against a second corpus of the inner sub-circuit `.panda`
files under `Tests/Integration/IC` — the run scripts build it automatically at
`build-fuzzer/seed-corpus-ic` on first use. `panda.dict` feeds libFuzzer the
magic header, version tokens, QVariant type IDs, and map keys so mutation
threads the format's length-prefixed fields.

The harnesses whose input is a whole `.panda` file share one structure-aware
mutator, `PandaMutator::mutate()` (`PandaMutator.h`): it splices known keys,
type ids and element headers, and mutates a current-format file in its
decompressed body, writing it back compressed, so mutations reach the reader's
structure instead of breaking zlib's stream. One draw in ten hands that body
to libFuzzer's own mutation, `panda.dict`'s entries included, and one the
file's raw bytes, so the decompression guards still see corrupt streams.

A `.panda` file names its file-backed ICs' files, which resolve against its
own directory, so an input loaded alone loads none of them: 180 of the seed
corpus's 410 files name one. Every harness that loads a whole `.panda` input
(`clipboard`, `codegen`, `copy_panda`, `deserialize`, `ic_file`,
`ic_registry`, `round_trip`, `undo`) loads it beside every `.panda` file of the
repository's examples and test circuits, copied at start-up by
`FuzzCircuitDir::fill()` (`FuzzCircuitDir.h`); all 410 then load. Some are
processors whose few kilobytes name thousands of elements through nested ICs,
and a load costs about 1.2 ms per element under ASan, so a harness that loads
its input several times in a unit (`clipboard`, `ic_file`'s reload,
`ic_registry`, `round_trip`) goes on only with a circuit whose
`FuzzCircuitDir::loadedElementCount()` keeps the unit near 10 s, well inside a
round's 30 s timeout (`ic_registry`, about 25 loads a unit, near 1.5 s, so
that reading its corpus does not take the round).

Four harnesses (`codegen`, `ic_registry`, `structured`, `undo`) also build
circuits of their own at start-up, written by `FuzzSeed::pandaFile()`
(`FuzzSeed.h`) as `WorkSpace::save()` writes a file. Each checks them with
`FuzzSeed::requireAccepted()` before fuzzing: a circuit whose header, payload or
metadata the reader rejects, or that loads with no elements, aborts the harness
with its name and the reason, since its target catches every rejection and the
run would otherwise look clean (`ic_blob` builds its circuit per input with the
same writer, so it has none to check at start-up). A smoke test after changing a
harness: `./build-fuzzer/fuzz_<harness> -runs=1`.

## Run

Two wrappers cover the common cases; both wire up the corpus, dictionary, RSS
limit, and the LSan suppressions file for you.

```bash
# Campaign: all harnesses in the background, findings logged not fatal.
# Args: [build-dir] [workers] [harness]   (harness defaults to "all")
./Tests/Fuzz/run-unattended.sh build-fuzzer 4 all
tail -f build-fuzzer/fuzz_deserialize.log     # watch
pkill -f fuzz_                                 # stop all

# Triage: one harness, single-process, ASan/UBSan stack traces inline,
# exits on the first crash. Args: [harness] [iterations] [max-time] [build-dir]
./Tests/Fuzz/run-debug.sh deserialize 5 1800
```

To drive a single harness by hand instead:

```bash
ASAN_OPTIONS=abort_on_error=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
LSAN_OPTIONS=suppressions=Tests/lsan_suppressions.txt \
./build-fuzzer/fuzz_deserialize \
    -max_total_time=600 -rss_limit_mb=2048 -dict=Tests/Fuzz/panda.dict \
    -workers=4 -jobs=4 \
    build-fuzzer/seed-corpus
```

Findings land in `build-fuzzer/findings/` (or the current directory for a
hand-run harness):
- `crash-<sha1>` — ASan/UBSan abort (real memory or UB bug, must fix).
- `oom-<sha1>` — `-rss_limit_mb` exceeded. An unbounded allocation path
  (e.g. an attacker-controlled count with no upper bound). Not a memory-safety
  bug per se, but a denial-of-service vector worth reviewing.
- `leak-<sha1>` — LSan-detected leak that survived the harness teardown.
- `timeout-<sha1>` — `-timeout` exceeded (e.g. a pathological undo/redo loop).

## Triage a crash

To re-run a single crashing input under inline stack traces:

```bash
./build-fuzzer/fuzz_deserialize build-fuzzer/findings/crash-<sha1>
```

Once the bug is understood and fixed, minimise the reproducer
(`-minimize_crash=1`), drop it under `Tests/Fuzz/regressions/`, and pin it with
a regression test in `Tests/Unit/Serialization/TestSerialization.cpp` (see the
"libFuzzer Regressions" section there for the pattern).

## Corpus maintenance

After a campaign, cross-pollinate and minimise the corpora so each harness
keeps only the inputs that add coverage for it:

```bash
./Tests/Fuzz/merge-corpora.sh build-fuzzer
```

## Coverage

A harness is improved in rounds, not by long blind runs: a short run of the
harness grows its corpus, the corpus is replayed through the coverage build,
the code the harness is meant to reach and does not is read off the report,
and the harness, its seeds or `panda.dict` are changed to reach it; the
rounds stop when one adds nothing. With both presets built, one round is:

```bash
./Tests/Fuzz/coverage-round.sh structured 300   # harness, seconds
```

It grows the harness's own corpus in `build-fuzzer/rounds/<harness>/corpus`,
which every round reseeds with the repository's circuits as they are now (the
seed corpus rebuilt and copied over the corpus's copies of the same names, so
a regenerated circuit replaces its old copy), and writes the round's coverage
of `App/` per file and per function to
`build-fuzzer/rounds/<harness>/round-<n>/`, printing it next to the previous
round's. It prints the round's peak memory and slowest input against the
limits (2,048 MB, 30 s) and warns past three quarters of either: a harness
running near a limit hides what crosses it, as `fuzz_codegen`'s rounds at up
to 2,037 MB hid plan 2's P2.79 until a change to the harness pushed it over.
`llvm-cov show build-fuzzer-coverage/fuzz_<harness>
-instr-profile=<round>/coverage.profdata <source>` then shows a file's lines
with their counts.

`fuzz_clipboard` pastes clipboards, which no file in the repository holds: a
whole `.panda` input is copied and pasted, and with `FUZZ_CLIPBOARD_SEED_DIR`
set every clipboard it copies is written to that directory in the harness's
clipboard layout (a flags byte, the blob registry's 2-byte little-endian
length, the registry, then the items). Every round does this pass over the
seed corpus itself, so the corpus holds real clipboards to mutate.
`fuzz_codegen` builds a circuit from any input that is not a `.panda` file,
which the seed corpus holds none of, so `coverage-round.sh` adds 64 inputs of
fixed pseudo-random bytes (`structured-seed-*`) to its corpus.

To see which `App/` lines the seed corpus never reaches across all the
harnesses at once, build the coverage preset (source-based instrumentation,
no sanitizers) and render an HTML report:

```bash
cmake --preset fuzzer-coverage
cmake --build --preset fuzzer-coverage
./Tests/Fuzz/coverage-report.sh build-fuzzer-coverage
# open build-fuzzer-coverage/coverage-report/index.html
```

## Regression corpus

`regressions/` holds minimised reproducers for crashes/OOMs/timeouts this suite
has found. The contract under a fix: `load()` must throw cleanly or succeed —
never abort. Under ASan a reintroduced UAF aborts the test process, which is the
failure signal.

All reproducers have been re-verified clean against the current tree (no
crash/OOM/leak/timeout under ASan+UBSan).

`Status`:
- **pinned** — guarded by a dedicated regression test that loads the fixture and
  asserts it is rejected before allocation (OOM) or loads/throws without an ASan
  abort (UAF/UB). Tests live in `TestSerialization.cpp` (`testFuzzRegression*`)
  and `TestDolphinSerializer.cpp` (`.dolphin` header), run on every sanitizer CI
  lane via `ctest`.
- **harness** — the input is in a harness's own layout (a clipboard, an IC's
  file), so no test loads it: it is replayed through its harness, failing on
  the code without the fix and clean with it
  (`./build-fuzzer/fuzz_<harness> Tests/Fuzz/regressions/<file>`), and the
  defect is pinned by the unit test named in its row.
- **retired** — the reproducer no longer reaches its defect, and is kept as its record:
  - `bugB`: its embedded IC's blob is a compressed payload whose bytes no longer decompress, so the load stops in
    `qUncompress` before the IC shrinks its ports. `testFuzzRegressionICBlobShrink` builds the case instead, a real
    save whose IC's circuit is swapped for one with fewer inputs than the host wires, and crashes with `IC::load()`'s
    eviction of the stale `portMap` keys taken out.
  - `bugJ`: a timeout of `FuzzUndo` on a truth table, for which the harness once left `TruthTable` out of the types it
    adds, on the ground that its 2048-bit key made `UpdateCommand` serialization take seconds. The harness now adds,
    edits and toggles truth tables, and the slowest unit of its coverage rounds is 12 s, a large circuit's load. The
    reproducer was written for the harness's earlier input layout and no longer reaches a truth table (it runs in 2 ms).

| Reproducer | Class | Status | What it reproduces |
|------------|-------|--------|--------------------|
| `bugA_cleanup_uaf`                  | UAF     | pinned            | `qScopeGuard` cleanup after a mid-loop throw in `deserialize` |
| `bugB_ic_blob_shrink`               | UAF     | retired           | IC declares more outer ports than its blob exposes; stale `portMap` |
| `bugC_unbounded_portlist`           | OOM     | pinned            | implausible port-list count reserved before validation |
| `bugD_oom_qneconnection_stream_map` | OOM     | pinned            | unbounded map read in the `Connection` stream |
| `bugE_uaf_ic_boundary_orphan_conns` | UAF     | pinned            | orphan connections at an IC boundary |
| `bugF_oom_unknown_typeid_metadata`  | OOM     | pinned            | unknown QVariant type ID in the metadata map |
| `bugG_oom_element_stream_map`       | OOM     | pinned            | unbounded map read in the element stream |
| `bugH_old_format_v11_elements`      | crash   | pinned            | pre-4.1 v1.1 element parsing |
| `bugI_old_format_ic_skin_ref`       | crash   | pinned            | pre-4.1 IC skin reference |
| `bugJ_undo_truthtable_timeout`      | timeout | retired           | pathological undo/redo on a truth table |
| `oom_dolphin_filename_large`        | OOM     | pinned            | oversized `.dolphin` filename |
| `oom_dolphin_header_large_appname`  | OOM     | pinned            | oversized app-name string in the `.dolphin` header |
| `oom_panda_header_large_appname`    | OOM     | pinned            | oversized app-name string in the `.panda` header |
| `ic_nonfinite_position`             | abort   | pinned            | NaN/inf element position aborts IC preview via `qSaturateRound` assert (`fuzz_ic_file`) |
| `ic_nonfinite_rotation`             | abort   | pinned            | NaN element rotation aborts IC preview via the same assert (`fuzz_ic_file`) |
| `clipboard_paste_unreadable_items_leak` | leak | harness | a paste whose items do not read leaves its undo macro open (`fuzz_clipboard`; `TestScene::testPasteOfUnreadableItemsChangesNothing`) |
| `clipboard_snap_position_beyond_int` | UB     | harness           | snapping a position past `int`'s range (`fuzz_clipboard`; `TestGraphicElement::testSnapToGridKeepsAPositionBeyondIntRange`) |
| `clipboard_drag_image_extent_beyond_int` | UB | harness           | the clone-drag image of an extent past `int`'s range (`fuzz_clipboard`; `TestScene::testBuildDragImageClampsAnExtentBeyondIntRange`) |
| `clipboard_grid_beyond_int`         | UB      | harness           | the grid of a render reaching past `int`'s range, the drag image's fix kept (`fuzz_clipboard`; `TestScene::testGridIsNotDrawnBeyondIntRange`) |
| `ic_file_preview_extent_beyond_int` | UB      | harness           | an IC's preview of a body past `int`'s range (`fuzz_ic_file`; `TestICUnit::testGeneratePreviewPixmapScalesAnExtentBeyondIntRange`) |

Findings with no reproducer kept, each pinned by its unit test alone:
- The out-of-memory errors, timeouts and slow units of plan 2's coverage rounds
  (the rows of plan 2's ledger P2.58, P2.60, P2.66, P2.72, P2.73, P2.77 and
  P2.79), each found on one of the repository's large circuits
  (`Tests/Fixtures/Perf/clocked_2000.panda`, `ic_heavy.panda`,
  `combinational_1000.panda`) or a mutation of one. They are thresholds crossed
  in a round, not failures of one input: libFuzzer's memory limit compares the
  process's peak since it started. Replayed alone through the current harnesses
  on the code without each fix, every one stays under the limits, the closest
  `ic_heavy.panda` through `fuzz_undo` at 1,957 MB against the 2,048 MB limit.
  Their tests measure the growth itself:
  `TestScene::testNoItemMakesEveryMoveWalkTheScene`,
  `TestCommands::testLoadListGrowsLinearly`,
  `TestPort::testFanOutGrowsLinearly`,
  `TestICUnit::testAPreviewRendersOnlyWhenShown`,
  `TestICUnit::testLoadGrowsLinearlyInICs` and
  `TestSystemVerilogCodeGenUnit::testModuleGrowsLinearlyWithDepth`.
- The leak of a command whose first `redo()` throws (plan 2b's EB.4):
  `fuzz_undo` reached it by removing a blob still in use, which the
  application never does, and no longer does so; its test is EB.4's.
