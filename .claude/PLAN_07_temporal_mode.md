# Plan 7: temporal mode in the application

Plan 6's core and runner run temporal mode; this plan offers it. It adds the main window's mode and speed selectors
for the current tab with a simulated-time readout; the propagation delay as a parameter of each element, with its file
key, its editor field and its MCP property; BeWavedDolphin's temporal sweeps, in nanoseconds per column; MCP's mode,
speed, waveform resolution and input changes that settle; and the temporal examples with their generators. The
Arduino export's note on the delays it drops in temporal mode is plan 8's, among its exporter notes, since plan 8
lands after this plan. Temporal mode is offered only once plan 5's exact oracle with delays agrees with the reference.

- **Depends on:** plans 1 to 6 and the Qt floor's pull request, landed, since the plans run one at a time, in their
  order (the plans' rules, §1): plan 6 (the core's temporal mode; the runner's mode, speed and simulated time per
  tab; the settle; the sweep guard; each logic type's delay in its descriptor); plan 5 (its exact oracle with delays,
  P5.13, agreeing with the reference, and the covering rule it decides, P5.25); plan 3 (the parameter schema and store,
  the property command on the schema, the editor's fields from the schema); plan 2's fixes this plan's code builds on
  (one undo entry per edit, a morph's lossless undo, MCP validating every value before changing anything, the Python
  client's clock delay bounds, BeWavedDolphin's text export with its rows by port).
- **Lands before:** plan 8, whose temporal SystemVerilog export reads each element's delay from the netlist, and whose
  exporter notes include the Arduino export's note on the delays it drops in a temporal tab. No release comes between
  plan 6's switch and plan 8's landing (plan 6's P6.32), so none offers temporal mode without that note.
- **Branch and ledger:** its own branch from `master`, started once plan 6 has landed, landing in slices (the plans'
  rules, §3).
  `LEDGER_07_temporal_mode.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing", the
  format's among them: the document format is extended, never broken. This file states plan 7's own: temporal mode
  follows plan 5's spec, which this plan never changes; every control that sets the delay, the mode, the speed or a
  sweep's resolution goes through one rule, stated once. Its decisions are numbered P7.D1 on.

## 1. Where `master` stands

### 1.1 No temporal mode

`master` simulates with zero delay only. No element carries a propagation delay (the clock's `delay` is a phase, a
fraction of its period), the main window has no mode or speed control, and nothing shows simulated time.
BeWavedDolphin sweeps its columns as settles of the circuit, one input combination per column, and draws four states
(Unknown grey and Error red, as the canvas draws wires). MCP's `simulation_control` takes the actions start, stop,
restart and update; `create_waveform` takes a duration and input patterns; `set_input_value` sets the input, runs one
update and answers with an empty result, whose schema allows no field (`additionalProperties: false`). The 21
examples in `Examples/` are installed verbatim and preloaded into the WebAssembly build; none shows timing.

### 1.2 What the earlier branches built

The temporal interface was built twice before: on `unified-event-engine`, then on `temporal-engine` by way of a branch
that recreated its waveform half. `temporal-engine` added, on its event-driven engine:

- status-bar selectors: Functional or Temporal, and a speed from 0.000001x to 1x (1 ns to 1 ms of simulated time per
  1 ms tick, labels written with a `.` decimal separator), the speed and a simulated-time readout (5.00 µs rather than
  5000 ns) shown only in temporal mode, the readout refreshed by a 100 ms timer that ran for the whole session;
- a mode switch that re-initialised the circuit, because a ring that functional mode had settled at Unknown stays
  there in temporal mode (NOT of Unknown is a fixed point), and so did visiting another tab afterwards;
- a per-element delay editor, the delay in files, and MCP's `propagation_delay` property, apart from the clock's
  `delay`;
- BeWavedDolphin's own sweep mode and a resolution in nanoseconds per column (presets, 2 ns by default), each column
  advancing that much simulated time, so a delay shows as a lag of columns, with a message that a pulse shorter than a
  gate's delay is absorbed;
- MCP's `create_waveform` with `temporal` and `ns_per_column` (1 to 1,000,000), and a `set_input_value` that stepped
  until the circuit stopped changing, up to 256 ticks, and reported `settled` and `ticks`;
- three examples with generators: a gate delay chain, a ring oscillator and a static hazard.

### 1.3 What its review found here

Its review read every touched file and measured every behavioural claim. For this plan:

- clocks stayed on wall time in temporal mode, so the speed changed behaviour: a 1 kHz clock into a T flip-flop made
  100 transitions at normal speed and none at 1 ns per tick;
- a mode switch reset the circuit, and so did visiting another tab afterwards;
- the propagation delay had a false refusal (a delay-free type stored one, and every later save wrote it back), two
  meanings of the type's default (the editor cleared the override at the default, MCP stored it), two writers (the
  element and the engine's own table, which a rebuild overwrote) and a morph whose undo lost it;
- its bound lived in five places (the setter, the editor's spin box, the MCP handler, the JSON schema and the Python
  client), and one property touched eleven;
- a waveform sweep in temporal mode dropped the live run's in-flight captures; the text export's sweep ran outside the
  timed-run bracket, which was opt-in and asymmetric, so with the live window temporal a stale publication committed
  the sweep's last capture over the restored live state (Q from 0 to 1 with no live edge), and the sweep rewound the
  live clock (live time 7, then 2 after the export);
- BeWavedDolphin's resolution box showed a preset while the sweep ran at another value (`setTemporalMode(true, 7)`
  left it showing 2 ns per column), its labels were not translatable, its default of 2 ns lived in three places, and
  MCP accepted `ns_per_column` without `temporal` and ignored it;
- MCP settled an input change by looping the update;
- the readout's timer ran for the whole session;
- the MCP client's text and the examples' descriptions were stale or wrong.

## 2. What plan 7 builds

### 2.1 Offered once its oracle agrees

The selectors land only after plan 5's exact oracle with delays agrees with the reference on its temporal and delayed
draws (P5.13), and the covering rule it decides is in the spec (P5.25). Until then the core's temporal mode is
reachable only from tests.

### 2.2 The delay parameter

An int parameter in nanoseconds, common to the 16 logic types, in plan 3's schema: its default the type's delay from
plan 6's descriptor (Not 5; Nand and Nor 8; And, Or and the truth table 10; Xor, Xnor, Mux and Demux 12; latches 15;
flip-flops 20), its range [0, 1,000,000], its accept rule clamping; every other type refuses it. Its file key,
`propagationDelay`, is written in the element's common map only where the value differs from the type's default, in
undo snapshots too, and read as the default where absent, so files and snapshots of elements at their default keep
their bytes and a file written before this plan reads as before; undoing the first delay edit restores the default. A
key on a delay-free type is ignored and not saved again; a file's out-of-range integer saturates instead of wrapping;
a morph carries a delay set off its default, while one at its default takes the new type's, and its undo restores it.
The schema's setter is the only writer, which the editor, MCP, files and morphs all go through; the netlist reads each
node's delay from its element, and a delay change, which plan 3's change funnel reports as a parameter change, leaves
the netlist to be rebuilt at the next update, keeping state by plan 6's rebuild rule. On
`gpu-canvas` this parameter was built and tested, not offered (`3d7d5a621`: files and snapshots at the default kept
their bytes, 10 of 10 mutants caught).

### 2.3 The delay in the editor

The property editor offers the delay from the schema on every logic type, in nanoseconds, within the schema's range,
and on a selection only when every selected type has it, as for any parameter (plan 3). It applies as every field does
after plan 2's fix, one undo entry per edit rather than per keystroke. A test opens the editor on a gate and finds the
field (on two earlier branches it was invisible for every type: the timing group showed only for a type with a
frequency or a delay, which held for the clock and the buzzer alone); another edits a delay and checks that the timing
changes, and that its undo changes it back (an edited delay once never reached the engine, since the property-only
refresh after an update command did not rebuild).

### 2.4 The mode and speed selectors

- **The selectors show the current tab's** mode and speed, which the runner holds per tab (plan 6); switching tabs
  shows the new tab's. A mode switch keeps every value, Unknown included, and seeds nothing; a ring that functional
  mode settled at Unknown stays there in temporal mode, which is why the ring example has an enable (§2.7). A speed
  change keeps the simulated time elapsed.
- **The speed** maps simulated time to running time and nothing else (P7.D1): presets from 1 ns of simulated time per
  100 ms to 1 ms per millisecond (a hundred-millionth of real time to real time): `temporal-engine`'s, from 1 ns to
  1 ms per millisecond, and two slower ones, 1 ns per 10 ms and 1 ns per 100 ms, so a delay can be seen (P7.D5);
  labels translatable and formatted for the locale.
- **The readout** shows the tab's simulated time in the unit that keeps it at or above 1, formatted for the locale; its
  timer runs only while the current tab is in temporal mode.
- **Their place:** the status bar, as on `temporal-engine`, the speed and the readout shown only in temporal mode;
  shown to the user before they land (P7.15).

Tests, headless: the selectors follow the current tab; a mode switch keeps every slot, Unknown included; the readout's
timer is stopped in functional mode; a speed change keeps the elapsed time; a 1 kHz clock into a T flip-flop makes
the same transitions over the same simulated time at every speed.

### 2.5 BeWavedDolphin's temporal sweeps

BeWavedDolphin keeps its own sweep mode, functional or temporal, starting at the tab's mode when it opens (P7.15). In
temporal mode each column advances a number of nanoseconds of simulated time, so a delay shows as a lag of columns:

- every value from 1 to 1,000,000 is shown as it is, a value off the presets included; the default and the bound live
  in one place, which the dialog, MCP's handler and schema and the Python client read;
- every sweep, the text export's and MCP's included, runs through the runner's sweep inside plan 6's guard, so the live
  run's in-flight state and its time survive it, and the sweep cannot write into the live circuit;
- choosing temporal mode says that a pulse shorter than a gate's delay is absorbed, so an input toggling every column
  can leave the outputs flat;
- the labels are translatable.

Tests: a temporal sweep of a Not shows its delay as columns of lag (about five at 1 ns per column for 5 ns); a sweep
during a live temporal run leaves the run's pending captures and time as they were (a D flip-flop's capture in
flight survives); a value off the presets shows as itself; the text export follows the same sweep.

### 2.6 MCP

- **Mode and speed:** MCP reads the current tab's mode, speed and simulated time and sets the mode and the speed,
  declared in the schema with the Python client's models.
- **Input changes settle:** `set_input_value` changes the input and runs the core's settle, then reports whether the
  circuit settled and the instants it ran, both declared in its response schema; a free-running clock never settles,
  so the answer says so instead of looping. Measured on `temporal-engine`: one tick in temporal mode answered
  `{"status":"low","value":false}` where the settled value was 1. `simulation_control`'s `update` stays one tick: a
  client that asks for a tick gets one.
- **Waveforms:** the waveform commands take `temporal` and `ns_per_column`; `ns_per_column` without `temporal` is
  refused, not ignored; its bound and default come from §2.5's one place.
- **The delay:** offered through the schema as an element property, its name apart from the clock's phase `delay`,
  refused on a type without it, validated with every other value before anything changes (plan 2).
- **Text:** the MCP README and the client's descriptions are written for these commands.

### 2.7 The temporal examples

Three examples, each from a generator in `Tests/Integration/IC/Generators/` (not `Examples/`, which is installed
verbatim and preloaded into the WebAssembly build, nor `Scripts/`, which `pylint Scripts/` walks as a package,
breaking the generators' shared import), with a script that runs every example generator:

- **A gate delay chain:** a switch into four inverters, an LED on every stage; in temporal mode the edge marches one
  inverter's delay per stage, where functional mode changes all four at once.
- **A static hazard:** F = A OR (NOT A), with the Not's delay set to 10 ns and the Or's to 3 ns, so a falling A dips F
  in temporal mode; functional mode shows F constant at 1.
- **A ring oscillator with an enable saved off:** a Nand closing a ring of inverters, its enable a switch saved off,
  so the ring powers on at levels; switched on, it oscillates in temporal mode and reads Unknown in functional mode. A
  ring with no enable powers on Unknown and stays there, since NOT of Unknown is a fixed point.

Their descriptions are written for the new rules (on `temporal-engine` the ring's text said functional mode
"canonicalises" it). Every example, these three and `master`'s, also runs in temporal mode in a test, at its types'
delays, and nothing trips or oscillates unless it is meant to: under the branch's first rules one did, a gate-built JK
whose inner flip-flop turned the functional glitch of a Not's start value into a loop under a 5 ns delay, which
power-on Unknown removed.

### 2.8 Documentation

The developer guide's chapters on the simulation and on BeWavedDolphin, the project's `CLAUDE.md` (which says the
application does not offer temporal mode yet) and the READMEs' feature lists in their three languages describe
temporal mode as offered; the new strings go through `tr()`, and the translation catalogs follow at the release. The
release notes name what users see: the selectors, the delay, BeWavedDolphin's temporal sweeps and the examples.

### 2.9 The manual pass

Before its last slice lands, a pass by hand over what this plan changes: simulation in both modes at several speeds and
per tab, a mode switch in the middle of a run, a temporal sweep during a live temporal run, the delay field on a
selection, the three examples, and the readout across tabs.

## 3. Pitfalls

- A delay saved only off its default needs the reader to set a missing key back to the default: on an earlier branch,
  restoring a snapshot from before the first change left the change in place.
- A response schema with `additionalProperties: false` rejects a field it does not declare, so every caller fails
  validation until the field is declared.
- A ring held at Unknown stays there under a timed run, so a sweep or a test that starts from that state shows a flat
  Unknown: start from levels.
- A one-column pulse narrower than a gate's delay is absorbed by the inertial delay; a waveform test of a delay holds
  its inputs for several columns.
- Lint the Python client with ruff, pyright and mypy before pushing it (`CLAUDE.md`, "Building and testing").

## 4. Steps and slices

| Row | Step | Slice |
|---|---|---|
| P7.1 | The delay parameter: schema, file key, snapshots, morphs, one setter; the netlist reads it (§2.2) | 1 |
| P7.2 | The delay in the property editor (§2.3) | 1 |
| P7.3 | MCP's delay property and the Python client's models (§2.6) | 1 |
| P7.4 | The gate: plan 5's exact oracle with delays agrees, and its covering rule is in the spec (§2.1) | 2 |
| P7.5 | The mode and speed selectors and the readout (§2.4) | 2 |
| P7.6 | MCP's mode, speed and simulated time (§2.6) | 2 |
| P7.7 | `set_input_value` through the settle, its response declared (§2.6) | 2 |
| P7.8 | BeWavedDolphin's temporal sweeps through the runner's sweep (§2.5) | 3 |
| P7.9 | MCP's waveform commands: `temporal` and `ns_per_column` (§2.6) | 3 |
| P7.10 | The three temporal examples, their generators and the script that runs them (§2.7) | 4 |
| P7.11 | Every example run in temporal mode, nothing tripping or oscillating unless meant to (§2.7) | 4 |
| P7.12 | The developer guide, `CLAUDE.md`, the READMEs and the MCP text (§2.6, §2.8) | 5 |
| P7.13 | The manual pass (§2.9) | 5 |

The checks and open questions of §6 are rows too (P7.14 to P7.16).

## 5. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P7.D1. Clocks are event sources in simulated time in temporal mode; the speed only maps simulated time to wall
  time.** Basis: measured on `temporal-engine` (with clocks on wall time a 1 kHz clock into a T flip-flop made 100
  transitions at normal speed and none at 1 ns per tick); the user's ruling of 2026-10-03/04.
- **P7.D2. Mode and speed per tab, the selectors showing the current tab's; a mode switch keeps every value,** Unknown
  included, and seeds nothing. Basis: measured on `temporal-engine` (a mode switch reset the circuit, and so did
  visiting another tab afterwards); the user's ruling of 2026-10-16.
- **P7.D3. The delay becomes each element's parameter here, where it is first offered:** its file key, editor field
  and MCP property, on the type's delay plan 6 gives. Basis: the user's ruling of 2026-10-21 (the delay arrives where
  it is first used); `master` has none, and plan 3 preserves behaviour.
- **P7.D4. The delay's file key is written only where it differs from the type's default,** read as the default where
  absent. Basis: the format rule (the user's ruling of 2026-10-20, each addition to a document an extension); tested
  on `gpu-canvas` (`3d7d5a621`).
- **P7.D5. Two speed presets slower than `temporal-engine`'s: 1 ns of simulated time per 10 ms and per 100 ms.** The
  runner shows changes at the display rate, 60 Hz, about 16.7 ms a frame. At `temporal-engine`'s slowest, 1 ns per
  millisecond, plan 6's delays last 5 ms (Not) to 20 ms (a flip-flop), so no gate's delay shows as a frame of its own
  and a chain of gates changes in one frame. At 1 ns per 10 ms a Not's delay spans three frames and a flip-flop's
  twelve, so a chain visibly steps; at 1 ns per 100 ms a Not spans thirty, slow enough to follow one gate at a time.
  Basis: read (the arithmetic, from the delays of plan 6's §2.2 and the display rate); the user's ruling of 2026-10-21.

## 6. Claims to check and open questions

- **The Python client reads the bounds from the schema file** (P7.14), not from constants of its own: on
  `temporal-engine` the delay's bound lived in the client too. Checked by a test that changes the schema's bound and
  sees the client follow, or the client keeps a copy and a test compares it with the schema.
- **What the user sees new** (P7.15): the selectors' place and labels, the readout, BeWavedDolphin's controls and its
  sweep mode starting at the tab's, shown to the user before their slices land.
- **Whether the slowest speed shows a delay on the canvas** (P7.16): decided by arithmetic, two slower presets
  (P7.D5). The gate delay chain and the ring are still watched at each preset, by hand, to confirm that the slower two
  step visibly and the faster ones do not stall.

## 7. Risks

| Risk | Mitigation |
|---|---|
| Unknown where independent changes race: a waveform column changing a clock and its data, an MCP batch, a function hazard into a latch | Certain; the release notes and the developer guide say why |
| Unknown, now common at power-on, reaches every place a status is read as a level | Plan 6 decides the places; this plan's new readers (the settle's report, the temporal columns) read the `Status` and are tested with a non-level |
| A user's free-running ring never leaves Unknown | The ring example saved with its enable off, and its description says why |
| A temporal sweep's inputs toggling every column leave the outputs flat | The dialog says so when temporal mode is chosen; tests hold inputs for several columns |
| Temporal mode offered before its oracle agrees | The selectors wait on P7.4 |
| A rule restated in several places drifts: the delay's bound, the resolution's default | One place each, read by the editor, BeWavedDolphin, MCP and the client (P7.14) |

## 8. What other plans take from plan 7

- **Plan 8:** each element's delay, through the netlist, for the temporal SystemVerilog export and for the Arduino
  export's note on the delays it drops, which plan 8 writes among its exporter notes.
- **Plan 9:** the selectors and the readout, in the application's layer, and BeWavedDolphin's controls, which its
  editor split leaves where they are.
- **Plan 5:** its exact oracle with delays, whose agreement this plan waits on.

## 9. The branches' work, as evidence

A hash is its branch's, kept read-only; `temporal-engine`'s commits are on that branch, not on `gpu-canvas`.

| Step | Commits |
|---|---|
| The delay parameter, built and not offered | `3d7d5a621` on `gpu-canvas` |
| The per-element delay editor | `3d90ae143` on `temporal-engine` |
| The Functional and Temporal selector | `5fedb6f3e` on `temporal-engine` |
| BeWavedDolphin's temporal waveform mode; the timed-run bracket the review found asymmetric | `0f2bf5524`, `e11d17d62` on `temporal-engine` |
| MCP's delay and temporal sweeps; refusing writes that cannot take effect | `8611ee34d`, `9b42cb140` on `temporal-engine` |
| `set_input_value` presenting a settled circuit | `47b05e4bd` on `temporal-engine` |
| The temporal examples and their generators | `7b1c4c0b1` on `temporal-engine` |
| The review, with its measured findings | `a7c82468b`, `45764238a`, `9e502da41`, `57f54382d`, `e3f71feed` on `temporal-engine` |

None of `temporal-engine`'s interface is copied: it sat on an engine the restart replaces, and its review's findings
above are what this plan answers.
