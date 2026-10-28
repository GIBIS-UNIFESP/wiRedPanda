# The model-cost round

Once plan 9 has landed, the canvas draws from a model of plain objects with spatial grids, one change funnel and a
document listener, and its CPU backend paints with sprites; what an edit still costs on a large circuit is the
model's own work. This round measures selection, move and load on that model, on plan 1's named fixtures, and
optimises what the measurement shows. Its starting list is the `perf/load-hotspots` prototype's 37 load and paint
optimisations, each re-derived on the new model from its intent where the new model still pays its cost, never
cherry-picked, and each measured first, since plans 2, 3, 4, 6 and 9 may already have removed it. It comes right after
plan 9 and before plan 10, so plan 10's GPU mirror is built on the model as the round leaves it.

- **Depends on:** plans 1 to 9, the Qt floor's and the exception boundaries' plans, landed, since the plans run one
  at a time, in their order (the plans' rules, §1): plan 9's model (the document with its spatial indices and
  stacking sequence, the change funnel and the listener, the editing session's selection, ports, wires and labels as
  plain objects, and the CPU backend with its sprite cache), and its measurement of the CPU backend against `master`
  (P9.25); plan 1's perf fixtures and probe harness (its §4.2 and §4.3); plan 6's core, whose netlist a load builds;
  plan 4's shared IC definitions; plan 3's descriptors and presenters.
- **Lands before:** plan 10, whose mirror follows the listener's events.
- **Branch and ledger:** `09a-model-cost`, from `09-editor` once plan 9 has landed, in slices (the plans' rules, §1
  and §3). `LEDGER_model_cost.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing", its
  profiling rules among them. This file states the round's own: a cost is measured before it is optimised, by
  instructions under callgrind through the probe's phases on a relwithdebinfo build, never by one wall-clock run; an
  optimisation stays only where the counted cost drops on the named fixtures; a saved file's bytes do not change; and
  a cache's key names everything the cached value depends on. Its decisions are numbered MC.D1 on.

## 1. What the round starts from

### 1.1 The model's costs, as the spikes measured them

The GPU spikes rendered `master`'s `Scene` as a headless model, the mouse forwarded to it as scene events, on the
clocked 8,000-element circuit (`spike/gpu-scene`). With rendering cheap, the edits cost the model's own work:

| Edit | Cost | Notes |
|---|---|---|
| Rubber band (press, move, release) | 158 ms of the scene's event handling | 9 items selected |
| Dragging the clock (7,200 wires) | about 87 ms per moved frame | about 17 ms the renderer refilling 7,200 wire slots, the rest (about 70 ms) the scene's own move |
| A wire drag between two ports | 6.5 ms of event handling | the scene created the wire; the spike's renderer then rebuilt everything (30 ms) |
| Polling for moves, scanning statuses (combinational 20,000) | 2.7 ms and 1.2 ms per frame | neither is needed with change notifications |

Those were `master`'s `Scene` and `QGraphicsScene`'s rubber band; plan 9 replaces both (the editor's selection over
the document's grids, the listener instead of polling, the runner's changed slots instead of a status scan), so each
is measured again on the new model before anything is changed (MC.3, MC.10).

### 1.2 The prototype branch

`perf/load-hotspots` holds 93 commits on `master` as of 2026-08-30: 59 `perf`, 14 `docs`, 11 `test`, 5 `fix`, 3
`refactor` and 1 `build`, so 79 that change code. Its characterization tests and tools went to plan 1, the element
geometry and the plain ports, wires and labels to plan 9, the IC definitions to plan 4 and the dense graph's intents to
plan 6's builder; its new file format (Rev102) is not adopted (MC.D3). What it leaves for this round is its load and
paint work: 36 `perf` commits and the `build` one, below.

### 1.3 The 37, by kind

Each is a starting point: the round looks for its cost in the new model's profile first, and re-derives it only where
the cost is there. A hash is the prototype's, on `perf/load-hotspots`, kept read-only.

| Kind | Optimisation | Commit |
|---|---|---|
| Measuring | Callgrind modes scoped to the load and to start-up (`--toggle-collect`), every run with an isolated configuration and no update check | `b3de9bca0` |
| Once, not per item | A port's attach and detach without re-walking its sibling wires (done: plan 2's P2.38 and P2.51) | `36cf4c433` |
| Once, not per item | A port's attach without scanning its connections (done: plan 2's P2.66) | `dabe62ca4` |
| Once, not per item | A moved port's scene position computed once, not once per wire | `3e05cd106` |
| Once, not per item | Element asset paths resolved once and cached | `f0b494053` |
| Once, not per item | A port's glyph and hit-area paths built once | `3ed0d5211` |
| Once, not per item | Ports themed once, not again in the element's constructor | `9b1923b0a` |
| Once, not per item | A new element's ports laid out once instead of three times | `d4fc723f9` |
| Once, not per item | No tooltip set when a port's name is unchanged | `22e125076` |
| Once, not per item | Orientation work skipped when it leaves the element unchanged | `6a843eedb` |
| Once, not per item | A shared skin instead of one built per element | `4e2805ff4` |
| Once, not per item | An element's label item only when there is a label | `25e295093` |
| Once, not per item | Both ends of a wire connected in one step | `29321ec59` |
| Once, not per item | The bounds of an element's ports cached where they can change | `74744bc4b` |
| Lookups and allocation | Items by id in a table indexed by id instead of a `std::map` | `04b0b5923` |
| Lookups and allocation | The elements kept in evaluation order instead of sorted for every simulation build | `e2f59b49f` |
| Lookups and allocation | The scene's element accessors split by what the caller needs | `473ec3301` |
| Lookups and allocation | Ports from a slab pool instead of one heap allocation each | `8fb356e61` |
| Lookups and allocation | Wires from the same slab pool | `a0b09db50` |
| Lookups and allocation | A logic node's input and output values inside the node instead of six heap vectors | `a0f80678c` |
| Lookups and allocation | An IC instance's nodes in one allocation | `000d183aa` |
| Lookups and allocation | Each IC file resolved once per circuit load, not once per instance | `ee3f7a33e` |
| Lookups and allocation | An IC's ports created at the definition's counts and laid out once | `b50563b89` |
| Lookups and allocation | An IC's ports re-registered from the loader's keys instead of a scan of the whole port map (done: plan 2's P2.73) | `5f0f931bf` |
| Lookups and allocation | A repeated metadata key's string decoded once and shared | `79b22cbc7` |
| Lazy interface | The Language menu built when first shown | `3a208ee56` |
| Lazy interface | The palette's icons loaded when first shown | `cc4b0a360` |
| Lazy interface | The element editor's colour combo filled on first use | `24c08f876` |
| Painting | Elements, ports, wires and labels at reduced detail when tiny | `e32860a94` |
| Painting | The visible part of a long wire's curve found, and only it sampled | `31ae7b70b` |
| Painting | A long wire stroked as a polyline over the part that shows | `06297bdc5` |
| Painting | Wires at reduced detail grouped by status, leaving out those inside one device pixel | `ae5f11acb` |
| Painting | The wire layer drawn from cached device-pixel tiles | `13e50ece6` |
| Painting | Port glyphs in one path per status and kind instead of six painter calls each | `f1fb2d544` |
| Painting | Repaint requests skipped for areas no view shows | `92f004df6` |
| Painting | The circuit's bounds without listing every element's child items | `0859881c0` |
| Painting | Wires drawn without antialiasing | `d96d90f0a` |

The last is not re-derived (MC.D4). Three are done on `master` by plan 2, each with a test of its growth that the plans
rewriting the code keep (P2.38 and P2.51, P2.66, P2.73), so the round re-derives none of them. Several others name code
the plans before this one replace: the simulation's items are plan 6's netlist, the scene's accessors and the label
items are plan 9's document and label layer, a port's glyphs are plan 9's port painter, an IC's ports and files are plan
4's definitions. For those the round asks whether the replacement pays the same cost, not whether the old code is still
there.

### 1.4 The prototype's other `perf` commits

They carry intents other plans hold, or the format the restart does not adopt, and are not this round's:

- **Plan 9:** ports, wires and labels as plain objects drawn by layers (`88fd7094a`, `e35627ab3`, `3f32125f5`), a wire
  listed in the grid cells its curve touches (`2a9244971`), the element owning its ports' orientation (`83a8fa634`),
  items kept in id order (`793e93239`), the circuit's bounds from the registry (`4b15787a5`), and pens and brushes from
  the theme (`9c80d4b47`, `fe05c3fdb`, `d8f582ee6`), which plan 9's painters take from the theme they are handed.
- **Plan 6:** the dense dependency graph (`78c7e02e6`), the IC netlist compiled once per definition (`c58d195f8`), the
  simulation handed elements instead of every item (`ebd4b54ce`), and the one-byte status (part of `e3455886e`, whose
  inline wire lists and reserved port lists are plan 9's ports).
- **Plan 4:** an IC's file or blob opened once per definition (`4765a2f81`) and its preview once per definition
  (`d1706d323`).
- **Plan 3:** names and tooltips on demand (`9445ff4d4`, `9c1690beb`), a footprint size instead of a sizing picture
  (`d6b9ce650`), and the mute state in the audio service (`1939ad679`).
- **Not adopted:** the Rev102 records (`833e91e7f`, `0faf2539f`) and resolving a wire's ports through the element it
  names (`8a4a25dc4`), which works only on Rev102's keys.

## 2. What the round does

### 2.1 Measuring first

- **The callgrind modes** (MC.1), re-derived first, since every later number is counted with them. On `master`,
  `Scripts/profile_load.sh` has `callgrind`, `perf` and their attaching forms, each over a wall-clock window that holds
  a varying amount of post-load idle and simulation work, so two totals are not comparable. Modes scoped to the load
  and to the main window's construction count only that code (`--toggle-collect`), and every profiling run uses an
  isolated configuration directory with the update check off: on the prototype the update check was 4.8% of the load
  profile, with most of the unresolved cryptography time.
- **The probe's new phases** (MC.2), added to plan 1's harness beside its load, frames, idle tick, wire, undo, move,
  delete and pan: a rubber band over the whole circuit and over a few elements, select all, a click selection, moving
  one element and moving a selection of many, dragging the clock, and a wire drag, each through the editor's own path
  as the application's input takes it, on the named fixtures (clocked 8,000, combinational 20,000, IC-heavy). Each
  phase asserts the state it is meant to reach (a rubber band selects what it covers, the clock moves) before its
  count is read.

### 2.2 The baseline

Every phase is counted on `master` as plan 9 leaves it, and every optimisation of §1.3 is looked for in those profiles:
its cost found, gone, or moved to the code that replaced it (MC.3). The baseline is the ledger's, phase by phase, with
plan 9's own measurement of the CPU backend against the old view (P9.25) beside it.

### 2.3 Selection and move

Rubber band, click selection and select all over the document's grids and the session's selection; moving one element,
many and the clock, each move reaching the indices, the wires' curves and the listener once per element and wire, not
once per frame per item; a wire drag that adds one wire to the document and the listener, not a rebuild. Each change
comes from the baseline's profile, and the listener's events keep their meaning (MC.11).

### 2.4 Load

The load's share of the 37 where the baseline finds its cost: a port's attach and wires connected once, ports laid out
and themed once, paths and assets resolved once, lookups by id in a table, the elements in evaluation order without a
sort per build, pooled ports and wires, the core's node values without heap vectors, an IC instance's nodes in one
allocation, its file resolved once per load, repeated metadata keys decoded once. Every file reads and saves as
before: plan 1's payload goldens and the backward-compatibility files hold.

### 2.5 Painting on the CPU backend

The CPU backend stays the fallback and draws exports, the minimap, the IC preview and MCP's screenshots, so its
painting costs count. The painting part of the 37 where the baseline finds its cost: reduced detail when tiny, the
visible part of a long wire sampled and stroked as a polyline, wires at reduced detail grouped by status and those
inside one device pixel left out, port glyphs in one path per status and kind, repaints skipped where no view looks,
cached tiles only if the sprite cache does not already cover them. Wires keep `master`'s adaptive antialiasing
(MC.D4). Plan 1's wire-pixel goldens and the CPU backend's parity renders hold; a reduced-detail drawing that changes
pixels at a zoom the goldens cover is shown to the user first.

### 2.6 Start-up

The Language menu, the palette's icons and the editor's colour combo built when first shown, where the start-up
profile finds them.

### 2.7 Measured after

Every phase counted again against the baseline, on the named fixtures, and the CPU profile of a settled application
on the 8,000-element circuit compared with the baseline's, as `CLAUDE.md`'s profiling rules say (MC.9). Each of
the 37 is listed as re-derived (with its drop), not needed (the new model had no such cost), or dropped (its drop too
small to keep).

## 3. Pitfalls

- **Wall-clock numbers on the development machine varied 12 to 54% between runs**, so a single run proves nothing;
  instructions are counted, or runs interleaved in pairs and only a direction that holds across all of them reported.
- **Under callgrind every paint pass exceeds the 50 ms budget** after which the CPU backend turns wire antialiasing
  off, so the probe holds antialiasing on before each frame, as plan 1's harness does.
- **A probe that does not reach its state answers nothing**: a rubber band that selects nothing, or a move that the
  snap swallows, counts the wrong work. Each phase asserts its state first.
- **A cache keyed on less than it depends on draws stale**: the theme, the pixel ratio and the zoom belong in every
  painting cache's key, as plan 9's sprite key shows.
- **A `ps` CPU figure is an average since launch**, falling for a minute or more as start-up dilutes; a running
  application is measured settled, attached after the load (`perf stat -e task-clock -p`), over matched windows.
- **An optimisation can change a file's bytes** through the order it writes items in; plan 1's payload goldens catch
  it, and the round changes no format.

## 4. Steps and slices

| Row | Step | Slice |
|---|---|---|
| MC.1 | The callgrind modes scoped to the load and to start-up, with an isolated configuration (§2.1) | 1 |
| MC.2 | The probe's selection, move, clock and wire-drag phases on the named fixtures (§2.1) | 1 |
| MC.3 | The baseline: every phase counted, every one of the 37 looked for in the profiles (§2.2) | 1 |
| MC.4 | Selection's costs (§2.3) | 2 |
| MC.5 | Move's costs: one element, many, the clock, a wire drag (§2.3) | 2 |
| MC.6 | Load's costs (§2.4) | 3 |
| MC.7 | Painting's costs on the CPU backend (§2.5) | 4 |
| MC.8 | Start-up's costs (§2.6) | 4 |
| MC.9 | The measurement after, each of the 37 accounted for (§2.7) | after |

The checks and open questions of §6 are rows too (MC.10 and MC.11).

## 5. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **MC.D1. A round on the model's own costs, after plan 9,** re-deriving the prototype's load and paint work on the new
  model from its intent, each optimisation measured first. Basis: measured in the spikes (a rubber band 158 ms and
  moving the clock about 70 ms in the model, whatever the renderer); the user's rulings of 2026-10-03/04 (the round)
  and 2026-10-04 (the prototype's work re-derived, not merged).
- **MC.D2. Right after plan 9, before plan 10,** with a plan file and a ledger of its own: plan 10's mirror follows the
  model's events, so it is built on the model as the round leaves it. Basis: the user's choice of 2026-10-21, once the
  plans were set to run one at a time.
- **MC.D3. No new file format:** the prototype's Rev102 records, and what works only on their keys, are not adopted;
  every file reads and saves as before. Basis: the user's ruling of 2026-10-20 that each addition to a document extends
  the current format, never replaces it.
- **MC.D4. Wires keep their antialiasing on the CPU backend:** the prototype's wires without antialiasing are not
  re-derived. Basis: plan 9 keeps `master`'s adaptive antialiasing on the CPU backend (P9.D13), and plan 10's GPU draws
  wires with 4× multisampling (P10.D4), so the prototype's reason, the cost of antialiased wires on a large circuit, is
  answered by the GPU, not by a cruder picture.

## 6. Claims to check and open questions

- **Whether the new model still pays the spikes' costs** (MC.10): the rubber band's 158 ms and the clock's 70 ms were
  `master`'s `Scene`; plan 9's selection over grids and its listener may have removed them. The baseline answers
  before anything is optimised; a cost that is gone closes its step with the measurement.
- **Whether an optimisation changes what the listener reports** (MC.11): coalescing a move's changes, for one, changes
  the events plan 10's mirror reads, which plan 10's P10.19 asks about; any such change is settled with that question
  before plan 10 starts, and the listener's coverage test stays green.

## 7. Risks

| Risk | Mitigation |
|---|---|
| An optimisation re-derived for a cost the new model no longer has | The baseline first (MC.3); each of the 37 kept only with its measured drop |
| A cache or a reduced drawing shows stale or different pixels | Keys that name everything they depend on; plan 1's wire-pixel goldens and the parity renders; pixel changes shown to the user |
| A load optimisation changes a file's bytes | Plan 1's payload goldens; no format change (MC.D3) |
| Measurements that do not compare | Instruction counts in the scoped modes with an isolated configuration (MC.1); settled, attached CPU profiles |
| A change to the listener's events breaks plan 10's mirror before it exists | Settled with plan 10's P10.19 (MC.11) |

## 8. What other plans take from it

- **Plan 10:** the model as the round leaves it, with the listener's events settled for its mirror (MC.11), and the
  probe's selection and move phases, which time the GPU canvas's edits too.

## 9. Evidence

A hash is its branch's, kept read-only.

| What | Where |
|---|---|
| The 37 load and paint optimisations and the other `perf` commits | `perf/load-hotspots`: the hashes of §1.3 and §1.4 |
| A headless `Scene` rendered through the Qt Quick scene graph, its frames and its edits measured | `spike/gpu-scene`: `b214546bf`, the edit-gesture test in `4c7906614` |
