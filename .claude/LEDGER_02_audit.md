# Ledger: audit of plan 2

Tracks the gaps a full audit of plan 2 found on 2026-10-27, at `02-master-fixes`'s tip `4b113b957`, before the plan
is called done. The audit read `.claude/PLAN_02_master_fixes.md` and `.claude/LEDGER_02_master_fixes.md` in full and
checked every commit on the branch against the ledger, CI, the tests named and the fuzz rounds' outputs. Plan 2 lands
once every row here is `done` or `dropped`.

**Rules** (the plans' rules, §3): one row per gap; a task found along the way gets the next free row and a line in
the log. Status: `todo`, `doing`, `done` (committed), `blocked` (on what, in the notes) or `dropped` (why, with the
user's decision). The Commits column lists every commit of the row. Every status change gets a dated line in the log.
Fixes to the plan and its ledger rewrite the commits that were wrong (the plans' rules: misfiled records are fixed by
rewriting history, not by appending); a fix to code is a new commit with its own row in plan 2's ledger.

**What held:** every code commit on the branch is in a row of plan 2's ledger, and every hash there is on the branch;
every test the plan and ledger name exists, except two named as renamed (`testDragOfTopOverlappingElementPushesNoMove`,
`testOnlyTheLoadedICRendersAPreview`); every fix's commit message says how its test failed on `master`'s code; CI is
green at `7d595508b` (Build 40363055159, Sanitizers 40363055578, API docs 40363055135), the last push changing
application code, and every later commit changes only `Tests/Fuzz/` and `.claude/`.

**Open decisions (the user's):** P2.59, `blocked` in plan 2's ledger on dropping it as no defect; EB.D2 and EB.D3 in
plan 2b's ledger; the P2.D4 wording, AU.12 here.

| ID | Gap | Status | Commits | Notes |
|---|---|---|---|---|
| AU.1 | Every fuzz finding's input kept under `Tests/Fuzz/regressions/`, with a README row, as P2.D10 and the README require | todo | | None of the eleven is kept. Inputs on disk under `build-fuzzer/rounds/<harness>/round-N/`: P2.57 `clipboard/round-2/leak-6eeb308f…`; P2.58 the slow unit `slow-unit-fdf9d860…`; P2.61 `clipboard/round-3/crash-908608b1…`; P2.62 `clipboard/round-4/crash-f09f8306…`; P2.66 `ic_registry/round-2/timeout-fdf9d860…`; P2.71 `ic_file/round-1/crash-278a53ed…`; P2.72 `ic_file/round-2/oom-cbe929f3…`; P2.73 `ic_file/round-4/timeout-d5319413…` (a mutated `ic_heavy.panda`); P2.77 `undo/round-2/oom-d5319413…`; EB.4 `undo/round-4/leak-a68276f9…`; P2.79 `/var/tmp/wiredpanda-scratch/p263b/codegen/oom-b15f0fad…`. Each minimised, then replayed with its harness on the fixed code (and on `master`'s, failing) |
| AU.2 | P2.62's defect pinned on GCC | todo | | Its unit test passes on `master`'s code under GCC; only the fuzz input shows it. Pin it with a test that fails on `master` under GCC, or say in its row why none can (AU.1 keeps the input either way) |
| AU.3 | `Tests/Fuzz/coverage-round.sh` reports each round's peak memory | todo | | libFuzzer's status lines carry `rss: NNNMb`; the script prints none, so a harness running near `-rss_limit_mb=2048` went unseen (AU.4, AU.5) |
| AU.4 | P2.79's late finding explained in its row | todo | | `fuzz_codegen`'s rounds 1 to 6 peaked at 1,948 to 2,037 MB against the 2,048 MB limit; P2.63's changes pushed it over and P2.79 was found. Rounds 7 and 8, fixed, about 1,030 MB. The row now says only "P2.68's rounds did not report it" |
| AU.5 | `fuzz_copy_panda`'s 2,022 MB peak explained | todo | | Its last round (round 3) ran before P2.77, which stopped every nested IC rendering a preview as it loads; the likely cause, not checked. A confirming round on the current code, its peak recorded; `fuzz_ic_file` fell from 2,037 MB to 1,072 MB with P2.72 and P2.77 |
| AU.6 | Seed corpus rebuilt and one confirming round per harness | todo | | `build-fuzzer/seed-corpus` was built 2026-10-03: 76 of its 410 files differ from the repository's (P2.42's 72 regenerated components and 4 performance fixtures, e.g. `combinational_1000.panda` 19,106 against 18,650 bytes), and every round seeded from it. Rebuilt with `Tests/Fuzz/build-seed-corpus.sh`, then one 600 s round of each of the twelve harnesses with AU.3's peak, `fuzz_copy_panda` first (AU.5); a finding becomes a row of the plan whose code it reaches |
| AU.7 | `ArduinoCodeGen.cpp:243`, the generator running out of pins, reached by `fuzz_codegen` | todo | | P2.68 noted it reachable and the rounds stopped without it. A seed or dictionary entry with more inputs and outputs than the board's pins, then a round; or the row says why it cannot be reached |
| AU.8 | A throw inside an undo macro checked at every `beginMacro` | todo | | P2.57 (`ClipboardManager.cpp:182`) and P2.76 (`ICRegistry.cpp:253`) were one defect: a throw leaves the macro open. Not checked: `Workspace.cpp:914`, `PropertyShortcutHandler.cpp:44`, `:145`, `:221`, `:252` (morph, port size, delete and blob removal pushed inside with no guard), `ElementEditor.cpp:725`, and `:250` (excluded from coverage as unreachable). Each one that can throw becomes a row in plan 2's ledger with a test failing on `master`; plan 2b's ledger records the class |
| AU.9 | Plan 2's §2.13 to §2.25 filed by the plan that rewrites their code | todo | | They sit under §2, "In code no plan rewrites": P2.57, P2.58, P2.60, P2.61, P2.62, P2.66, P2.69, P2.71, P2.72, P2.73, P2.76, P2.77, P2.79. Clipboard, commands and ports are plan 9's; IC loading, the registry and the preview plan 4's; the serializer plan 3's or 4's; the SystemVerilog exporter plan 8's (§6). Each moved to its plan's section, the ledger's section references with it |
| AU.10 | Plan 2's §13 and plans 3, 4, 8 and 9 list the fixes they keep | todo | | Follows AU.9. Plan 8's line on plan 2's exporter fixes (lines 16 to 18) lacks P2.79, every gate a declared variable and the module linear in size |
| AU.11 | Plan 2's §9 complete | todo | | Its table lacks P2.18a, P2.38, P2.39, P2.40, P2.50, P2.51, P2.52, P2.57, P2.58, P2.60, P2.61, P2.62, P2.66, P2.69, P2.71, P2.72, P2.73, P2.76, P2.77 and P2.79, the fixes added since it was written. The other rows it lacks (P2.34 to P2.37, P2.59, and the harness changes P2.53 to P2.56, P2.63 to P2.65, P2.67, P2.68, P2.70, P2.74, P2.75, P2.78, P2.80) each checked for a golden it touches. Its heading still says "Pull requests" |
| AU.12 | The P2.D4 wording confirmed by the user | blocked | | On the user. Asked 2026-10-24 with P2.18: P2.D4 said a refusal is explained "in the status bar" and that duplicates from a file or paste "only warn"; reworded to: the inline editor says why in the status bar, the property editor keeps `master`'s dialog, and nothing on `master` warns of duplicates from a file or paste (plan 6's netlist builder will). Never answered and never given a row |
| AU.13 | Plan 2's ledger header and §12 name every open item | todo | | "Open decisions" omits P2.59 and P2.D4 (AU.12); §12 does not list P2.59 |
| AU.14 | A row in plan 2's ledger lists the branch's docs commits | todo | | About 86 `docs:` commits are in no Commits column; plan 1's P1.25 lists its own |
| AU.15 | Stale text in plan 2 and its ledger | todo | | §2.14 ends "P2.59, to be counted" (counted 2026-10-26); §2.1 says six harnesses check their seed, four since P2.64 and P2.74; P2.52's notes end "CI pending" (green); P2.1, P2.2 and P2.37 speak of the user's overnight run, replaced by P2.D10's amendment |

## Log

- 2026-10-27: ledger written from the audit of plan 2 at `4b113b957`: AU.1 to AU.15, AU.12 `blocked` on the user, the
  rest `todo`.
