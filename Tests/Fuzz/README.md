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
| `fuzz_clipboard`   | `FuzzClipboard.cpp`   | Clipboard MIME slots + the copy/paste serialize→deserialize round-trip. |
| `fuzz_old_format`  | `FuzzOldFormat.cpp`   | The pre-4.1 flat binary path (`GraphicElement::loadOldFormat`) and its per-version / per-element-type branches. |
| `fuzz_ic_registry` | `FuzzICRegistry.cpp`  | `ICRegistry` blob lifecycle: `setBlob`/`hasBlob`/`blob`, `initEmbeddedIC`, and `embeddedICs` metadata loading. |
| `fuzz_codegen`     | `FuzzCodeGen.cpp`     | The Arduino + SystemVerilog exporters (only reachable via `File > Export`), plus `generateTestbench`. |
| `fuzz_round_trip`  | `FuzzRoundTrip.cpp`   | The save side: `Serialization::serialize` + every element `save()`, then re-loads the output to surface save/load asymmetries. |
| `fuzz_ic_file`     | `FuzzICFile.cpp`      | File-backed IC loading (`IC::loadFromFile` → `loadFileDirectly`), including cycle detection and old-format migration. |
| `fuzz_copy_panda`  | `FuzzCopyPanda.cpp`   | The `Save As` helpers `copyPandaFile` + `readPreamble`, including the `fileBackedICs` recursive copy + cycle guard. |
| `fuzz_undo`        | `FuzzUndo.cpp`        | The undo/redo command system (`Commands.cpp`): applies a random command sequence, then winds the stack fully back and forward. |

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

Six harnesses (`clipboard`, `codegen`, `ic_file`, `ic_registry`, `structured`,
`undo`) also build circuits of their own at start-up, written by
`FuzzSeed::pandaFile()` (`FuzzSeed.h`) as `WorkSpace::save()` writes a file.
Each checks them with `FuzzSeed::requireAccepted()` before fuzzing: a circuit
whose header, payload or metadata the reader rejects, or that loads with no
elements, aborts the harness with its name and the reason, since its target
catches every rejection and the run would otherwise look clean (`ic_blob` builds
its circuit per input with the same writer, so it has none to check at
start-up). A smoke test after changing a harness:
`./build-fuzzer/fuzz_<harness> -runs=1`.

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

It grows the harness's own corpus in `build-fuzzer/rounds/<harness>/corpus`
and writes the round's coverage of `App/` per file and per function to
`build-fuzzer/rounds/<harness>/round-<n>/`, printing it next to the previous
round's. `llvm-cov show build-fuzzer-coverage/fuzz_<harness>
-instr-profile=<round>/coverage.profdata <source>` then shows a file's lines
with their counts.

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
- **harness-mitigated** — not a memory/correctness bug, so there is no product
  invariant to pin. `bugJ` is the only one: `TruthTable`'s 2048-bit key makes
  `UpdateCommand` serialization inherently multi-second, so `FuzzUndo` excludes
  `TruthTable` from its add/morph types. The reproducer is retained as
  documentation of why that exclusion exists.

| Reproducer | Class | Status | What it reproduces |
|------------|-------|--------|--------------------|
| `bugA_cleanup_uaf`                  | UAF     | pinned            | `qScopeGuard` cleanup after a mid-loop throw in `deserialize` |
| `bugB_ic_blob_shrink`               | UAF     | pinned            | IC declares more outer ports than its blob exposes; stale `portMap` |
| `bugC_unbounded_portlist`           | OOM     | pinned            | implausible port-list count reserved before validation |
| `bugD_oom_qneconnection_stream_map` | OOM     | pinned            | unbounded map read in the `Connection` stream |
| `bugE_uaf_ic_boundary_orphan_conns` | UAF     | pinned            | orphan connections at an IC boundary |
| `bugF_oom_unknown_typeid_metadata`  | OOM     | pinned            | unknown QVariant type ID in the metadata map |
| `bugG_oom_element_stream_map`       | OOM     | pinned            | unbounded map read in the element stream |
| `bugH_old_format_v11_elements`      | crash   | pinned            | pre-4.1 v1.1 element parsing |
| `bugI_old_format_ic_skin_ref`       | crash   | pinned            | pre-4.1 IC skin reference |
| `bugJ_undo_truthtable_timeout`      | timeout | harness-mitigated | pathological undo/redo on a truth table |
| `oom_dolphin_filename_large`        | OOM     | pinned            | oversized `.dolphin` filename |
| `oom_dolphin_header_large_appname`  | OOM     | pinned            | oversized app-name string in the `.dolphin` header |
| `oom_panda_header_large_appname`    | OOM     | pinned            | oversized app-name string in the `.panda` header |
| `ic_nonfinite_position`             | abort   | pinned            | NaN/inf element position aborts IC preview via `qSaturateRound` assert (`fuzz_ic_file`) |
| `ic_nonfinite_rotation`             | abort   | pinned            | NaN element rotation aborts IC preview via the same assert (`fuzz_ic_file`) |
