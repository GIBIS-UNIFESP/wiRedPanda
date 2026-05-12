# Fuzzing the .panda deserializer

`FuzzDeserialize.cpp` is a libFuzzer harness for `Serialization::deserialize`.
It exercises the same code path as `WorkSpace::load` (the File > Open code
path that triggered the H2/HC bug cluster):

1. `Serialization::readPreamble` — magic header + metadata blob
2. `Serialization::deserializeBlobRegistry` — embedded-IC registry parsing
3. `Serialization::deserialize` — main element/connection stream loop

## Build

Clang is required (libFuzzer is a Clang-only feature):

```bash
cmake --preset fuzzer
cmake --build --preset fuzzer
```

This produces `build-fuzzer/fuzz_deserialize`, instrumented with libFuzzer +
AddressSanitizer + UndefinedBehaviorSanitizer.

## Seed corpus

Bootstrap a corpus from the project's example circuits — they are real
`.panda` files spanning every file-format version we still support, which
gives the fuzzer a wide and well-formed starting point to mutate from:

```bash
./Tests/Fuzz/build-seed-corpus.sh build-fuzzer/seed-corpus
```

## Run

```bash
# 10-minute smoke run, 4 workers, ASan abort-on-error
ASAN_OPTIONS=abort_on_error=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
LSAN_OPTIONS=suppressions=Tests/lsan_suppressions.txt \
./build-fuzzer/fuzz_deserialize \
    -max_total_time=600 \
    -rss_limit_mb=2048 \
    -workers=4 -jobs=4 \
    build-fuzzer/seed-corpus
```

Findings are written to the current directory:
- `crash-<sha1>` — ASan/UBSan abort (real memory or UB bug, must fix).
- `oom-<sha1>` — `-rss_limit_mb` exceeded.  Indicates an unbounded allocation
  path in the deserializer (e.g. an attacker-controlled element count with no
  upper bound).  Not a memory-safety bug per se, but a denial-of-service
  vector worth reviewing.
- `leak-<sha1>` — LSan-detected leak that survived `~WorkSpace`.  Real bug.

Drop new findings under `Tests/Unit/Serialization/` as regression tests once
the bug is understood and fixed.

## Triage a crash

To re-run a single crashing input:

```bash
./build-fuzzer/fuzz_deserialize crash-<sha1>
```

Drop it as a regression test under `Tests/Unit/Serialization/` once the bug
is understood and fixed.
