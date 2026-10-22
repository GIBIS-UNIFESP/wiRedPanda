# The plans' shared rules

The rules every plan file shares: the plans and their order (§1), the principles each inherits (§2), how each works
(§3), and what is kept as evidence (§4). A plan file states only the rules that are its own. What applies to any work
on this code base, the build's and the tests' pitfalls among it, is in `CLAUDE.md`, under "Building and testing".

The plans restart the work of the `gpu-canvas` branch from `master`. That branch set out to reach three outcomes in one
plan: a canvas drawn on the GPU, whose cost does not grow with the circuit; a model that does not know how it is drawn
or edited, in layers that are each a build target; and one simulation with a functional and a temporal mode, written as
a data-oriented core whose state the renderers read. It finished five of its eleven phases and part of the sixth, then
held 574 commits since `master`, 259 of them code, none landed. On 2026-10-20 the user split the work into the plans
below, each recreated from `master` by intent, and kept the branch as evidence.

## 1. The plans and their order

| Order | Plan | File |
|---|---|---|
| 1 | Characterization on `master` | `PLAN_01_characterization.md` |
| 2 | Fixes to `master` | `PLAN_02_master_fixes.md` |
| 3 | The Qt 6.8 floor's pull request | `PLAN_qt_floor.md` |
| 4 | Element types as data | `PLAN_03_element_types.md` |
| 5 | ICs and the document format | `PLAN_04_ics_and_format.md` |
| 6 | Simulation semantics and its oracles | `PLAN_05_semantics.md` |
| 7 | The simulation core | `PLAN_06_core.md` |
| 8 | Temporal mode in the application | `PLAN_07_temporal_mode.md` |
| 9 | Export fidelity | `PLAN_08_export.md` |
| 10 | Editor architecture | `PLAN_09_editor.md` |
| 11 | The model-cost round | `PLAN_model_cost.md` |
| 12 | GPU canvas | `PLAN_10_gpu_canvas.md` |

Each has a ledger beside it (`LEDGER_<same suffix>.md`). The plans run one at a time, in this order, each starting once
the one before it has landed; no plan runs beside another, so no plan carries a rule for whichever of two lands first
(the user, 2026-10-21: "we arent going to run plans in parallel. we are going to follow the order"). On the branch,
rendering, the editor, the element model, the simulation, temporal mode, both exporters and the file format shared one
branch and one order of phases, so an open question in one area held up the rest, and none of it reached `master`.

## 2. Principles

- **Ground truth first.** An oracle written apart from the implementation, exhaustive where it can be, comes before the
  code it checks. On the branch the core was compared with a reference simulator on every slot after every tick, and
  both implemented the same faulty rules for pulses and for Unknowns merged from orders that disagree, so the comparison
  found nothing; an exact oracle written apart from both found 94 unsound outputs in 1,500 random feedback circuits and
  6,522 contradicted slots in the walks of 102 shipped circuits.
- **The semantics before the code.** Plan 5 writes the simulation's spec, its oracles and the reference before plan 6
  writes the core. On the branch the rules were amended three times after the core was written (2026-10-16, -19 and
  -20), and each amendment had to be carried into code already written.
- **Measure, then decide.** A claim is checked before a decision rests on it, and a decision says what evidence it has;
  one that rests on an observation stays unverified, with a ledger row, until a measurement backs it. A count is taken
  where a plan acts on it, on `master`. On the branch the decision to redraw the glows without blur filters rested on
  one element type's picture, and a review of 395 commits found 8 claims in the plan or in commit messages that the
  code did not bear out; the restart's first draft repeated four claims that were wrong when checked (a commit count,
  six defects said to be on `master` that were the branch's own, a count of affected types, and a count taken on the
  branch instead of `master`).
- **Small and landed.** A plan's work reaches `master` in slices that each leave the application working; nothing waits
  on a branch for weeks.
- **Nothing in prose only.** An open item, a user choice or an unchecked claim written anywhere gets a ledger row in the
  same change, and every commit is listed in its row. On 2026-10-20 an audit of the branch's ledger gave 19 items the
  row they lacked and found 41 code commits that no row listed.
- **A check counts once it has failed.** A golden, a test or a probe is broken on purpose once (an order reversed, a
  render skipped, a type removed from a list, a fixed defect put back) and must fail before it is trusted, and a probe
  is shown to reach the state it asks about before its answer counts. On the branch these passed while pinning nothing:
  clock tests that never called `update()` after their command, where the rebuild resets the clock; an ink check on an
  opaque picture; a port-count edit on an element whose count cannot change; wire geometry read with rendering off,
  where a wire builds no path; a translatable-strings golden compared with a stale extraction; display goldens pinned
  unlit, since orienting an element reset its unwired inputs; and an analysis whose probe circuits left a Mux data
  input unwired, so the clock never left Unknown and the probes asked nothing.
- **Name what goes.** A rewrite lists what the code it replaces does and keeps or drops each behaviour by name. An
  earlier engine rewrite dropped `master`'s idle-tick skip as an undocumented side effect, and made the test that
  visuals flush on a later idle tick vacuous; it was found only by asking what `master` had that the rewrite removed.
- **Enumerate a pattern, not its instances.** Where one defect shows a pattern (a status read as a level, a cast that
  hides a non-level), the repository is searched for every instance at once. On an earlier branch six such boundaries
  each picked a different wrong answer for the same Unknown (`false`, a raw −1, a clean high, a −1 that broke a column
  count, `1'b0`, and `true` written back into live state); they were found by enumerating `== Status::Active`,
  `static_cast<bool>` and `defaultValue()` across the repository.
- **The document format is extended, never broken.** A plan that changes what a document holds extends the current
  format: older files read as before, a file the application writes reads back and saves again byte for byte, and each
  extension is tested by reading files written before it. Plan 1's saved-payload golden pins the format first.
- **A refactor preserves behaviour, and every golden change is named by its cause.** Plan 1's goldens of `master`'s
  behaviour land first, so a structural change that silently alters a picture, a hit result, a trace or a file's bytes
  fails one. The application works after every commit, a replacement running beside what it replaces until it is
  proven. Layering is enforced by build targets, not described. A string keeps its translation context when its code
  moves, so the catalogs keep matching. Each rule lives in one place (a parameter's type, default, range, file key and
  editor in its schema; the simulation's semantics in one spec that the core and the reference implement). No
  compatibility code for unreleased work. Comments state live constraints, with no history.

## 3. How every plan works

- **Recreated by intent.** Nothing is cherry-picked, and no file is copied or merged from `gpu-canvas` or the other
  branches of §4: they show what to build and how to test it, not the bytes.
- **A plan file, a ledger and a branch per plan.** Each plan has a plan file, a ledger and decision numbers of its own
  (`P1.D1`, `QF.D1`, `MC.D1`, ...). The ledger has a row for every item, and for every open item, user choice and
  unchecked claim, written in the change that finds it; a task found along the way gets the next free sub-ID of its item
  (`P1.4a`) and a log line. A row's status is `todo`, `doing`, `done` (committed), `blocked` (on what, in its notes) or
  `dropped` (why, with the user's decision); every commit is listed in its row; every status change gets a dated line
  in the ledger's log. The plan is implemented on its own branch from `master`, and it lands before the next plan
  starts.
- **Slices.** A plan lands in slices: each slice is a pull request of the branch's oldest finished commits, small enough
  for rebase-merge, the only merge the repository allows (well under 100 commits: in June 2026 GitHub refused to rebase
  a pull request of 114, though it was up to date), and leaves the application building and working. After each slice
  lands, the branch is rebased onto `master`.
- **Each commit correct from its first line; nothing landed is rewritten.** A fix to a commit of a slice that has not
  landed is folded into that commit, with the user's approval, by an autosquash rebase. A fixup folds only if everything
  its added lines use exists at its target: a name lookup is not proof, a build at the target is. On the branch a
  squash folded a check that used `values` into a function whose scope has no `values` but whose class has `values()`,
  so the lookup passed and 21 commits did not compile; and hand-resolved conflicts left a brace as `>}` and a blank
  line that a fix had removed, which no conflict marker showed. So after the fold the tip's tree is compared with a
  backup branch's. A fix to code already on `master`, or one unrelated to the slice, is a commit of its own, placed
  before the change that needs it.
- **Checked before it lands.** Before a slice lands, after its last fold, every commit in it that changes code is built
  on the relwithdebinfo preset with its suite run, and its changed sources are compiled alone, outside their unity batch
  and without the precompiled header (`CLAUDE.md`, "Building and testing"). The pull request then runs CI, which covers
  Qt 6.8 and 6.9, the sanitizers and Apple clang. On the branch each phase was written without building it and built
  at its gate, which then found what the writing had missed: a debug gate passed a null dereference that GCC reports
  only at `-O2`, Qt 6.7.3's defects showed only once a gate ran it, and includes that a unity batch or the precompiled
  header supplied stayed hidden until a change shifted the batches.
- **The user's standing rulings.** No clang checking at all: ASan, UBSan and clang are left to CI (its Sanitizers
  workflow and its macOS jobs' Apple clang build). Validation runs on the relwithdebinfo preset. Callgrind and
  performance baselines stay out of the checks before a slice lands; performance is measured on the named fixtures
  after a plan lands. The fuzz harnesses run on the user's machine, overnight, not in CI, and a slice that adds or
  changes a harness lands after that run.
- **Rules for every new test.** No absolute wall-time bounds (ratios or counts instead); CTest labels that match what
  the test does; no history or defect IDs in comments; the tests of the model, presentation, controller, mirror,
  simulation core and CPU backend run under ASan and UBSan in CI.
- **Not carried from the branch:** squashing a long branch's fixes into earlier commits at phase gates, checked only at
  the tip; one order for all the work; and everything about landing `gpu-canvas` itself.

## 4. Kept as evidence

Read-only, none merged. A hash a plan cites is on the branch it names.

- `gpu-canvas`, with the backup branches of its squashes (`gpu-canvas-pre-squash`, `gpu-canvas-pre-squash-5v`,
  `gpu-canvas-pre-r14`, `gpu-canvas-pre-squash-goldens`, `gpu-canvas-pre-squash-includes`, `gpu-canvas-pre-fold-review`,
  `gpu-canvas-pre-fold-rs9`). Its `.claude/` holds the restart's write-up (`RESTART_gpu_canvas.md`), the lessons note,
  the decisions note with the old plan's decision numbers, the inventory mapping every old item to a plan, the old plan
  and its ledger, the squash record and the analysis notes.
- `temporal-engine`: 20 code commits on the `master` of 2026-08-29, then five commits of its review: an event-driven
  engine with a zero-delay and a temporal mode, a timed-run bracket for sweeps, BeWavedDolphin's temporal waveforms, a
  per-element delay editor, MCP delays and sweeps, temporal example circuits, a mode selector, and the fixes found on
  the way. The older `unified-event-engine` (55 commits on the same base) mixes the engine, temporal mode, a live
  analyzer in BeWavedDolphin, a step debugger and unrelated fixes.
- `perf/load-hotspots`: 93 commits on the `master` of 2026-08-30, load and paint optimisations of `master`'s view,
  measured on the 8,000-element circuit, and the characterization tests and tools plan 1 recreates.
- `spike/gpu-scene`: five spike commits on `perf/load-hotspots`, behind the CMake option `WIREDPANDA_GPU_SPIKE`:
  renderer experiments (Qt Quick's scene graph, a `QRhiWidget` wire renderer and viewport, raster and OpenGL
  comparisons) and a spike that flattens the simulation into arrays. Both reach private state through friend structs
  and build with `-Wno-error`.
- `qtquick-rewrite` and `qtquick-rewrite-v2`: the interface rewritten in Qt Quick, out of scope, but for one lesson its
  canvas taught: drawn on the GPU, yet rebuilding every element's cache key, bounds and port positions on the CPU each
  frame, it first cost 54% more CPU than the widgets view on a 2,000-element circuit.
