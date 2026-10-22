# Plan 8: export fidelity

The SystemVerilog export is synthesizable (the tests run it through yosys); the Arduino export is a sketch that runs in
real time. This plan puts both exporters on plan 6's netlist builder, so they read the circuit the simulator reads;
makes the SystemVerilog export start and compute as the simulator does, in both modes; keeps the Arduino export
functional and has each exporter say where its output differs from what the application shows; and checks the exports
against the simulator on every run, in CI, with a differential whose oracle cannot pass by checking nothing. External
corpora run through the same differential for development.

- **Depends on:** plans 1 to 7 and the Qt floor's pull request, landed, since the plans run one at a time, in their
  order (the plans' rules, §1): plan 6 (the netlist builder, with its order, its wireless resolution, its compiled
  definitions and its list of undriven inputs; and the core, which the differential compares with, in lockstep with
  plan 5's reference); plan 7 (each element's delay, which the temporal SystemVerilog export reads and the Arduino
  export's note names); plan 5 (the spec's start, which the export follows); plan 4 (one shared definition per IC, from
  which the exporters read an instance's circuit); plan 3 (each port's untranslated source name); plan 2's exporter
  fixes, which this plan keeps (truth tables that compile in SystemVerilog, identifiers from untranslated names, both
  exporters following the Tx into an Rx); plan 1's text goldens and translation tests.
- **Lands before:** plan 9, and any release that follows plan 6's switch: from the switch until this plan lands, the
  exports and the simulation follow different rules. The IC instances' internal elements go as its last step (§2.11).
- **Branch and ledger:** its own branch from `master`, started once plan 7 has landed, landing in slices (the plans'
  rules, §3). `LEDGER_08_export.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing". This
  file states plan 8's own: an export reads the circuit only through the builder; a divergence between an export and
  the simulator is either fixed or explained, and an explanation that no longer occurs fails; a check counts once it
  has failed, which for a testbench means it checked readings. Its decisions are numbered P8.D1 on.

## 1. Where `master` stands

### 1.1 The exporters

`ArduinoCodeGen` (1,443 lines) and `SystemVerilogCodeGen` (1,519) take the scene's elements in the priority walk's
order (`Scene::elements()` returns `Scene::sortByTopology`), find wireless drivers with `Scene::wirelessTxInputPorts`,
and walk each IC instance's internal elements, which the Arduino exporter sorts with the same walk. The Arduino
exporter writes the old engine's settle bound (`Simulation::kMaxSettleIterations`, 10) into its sketch's settle loop,
times clocks with `elapsedMillis`, has no notion of delays, and prints `// WARNING` comments (and a `qWarning`) for
inputs and outputs it drops; its one disclosure of a difference from the simulator, inputs nothing drives whose
default is Unknown, which the sketch reads as LOW, is a comment. An export shows a status message for 4 seconds.

The SystemVerilog exporter is a structural translator: every IC exports from its gates, with no template picked by
port signature, and cross-coupled feedback gates become a `reg` with `always @(*)` and a seed, inside
`/* verilator lint_off UNOPTFLAT */`. A change to one component's emission therefore reaches every embedder: the
level-1 D flip-flop sits in 10 components, and changing its emission changed 15 goldens. Memory is
`always @(posedge clk ...)` with `if (~prst)`, so a clock going from 0 to x is a rising edge that stores D, and an x on
~Preset takes the `else` branch; `fourState` writes Unknown and Error alike as `1'bx`; yosys's `read_verilog` defines
`SYNTHESIS` unless given `-nosynthesis`. Truth tables do not compile (plan 2 fixes it).

Of the places where `master` reads a status as a level, the exporters hold two of the 74 `== Status::Active` reads
(each exporter's `highLow`) and 15 reads of `defaultValue()` (11 in the Arduino exporter, 4 in the SystemVerilog one).

### 1.2 Their tests and CI

A normal run compares the generated text and runs two hand-written iverilog testbenches of fixed values; iverilog,
yosys and verilator, the stimulus and the generated testbenches run only with `GENERATE_EXPECTED_SYSTEMVERILOG` set,
which overwrites a golden only when every testbench passes, and skip on a machine without the tools. The two
hand-written tests skip silently without iverilog; iverilog and vvp time out after 30 s. `TestArduino` compiles a
ripple counter's sketch with `g++` against stubs of the Arduino API and compares it with the engine, skipping without
`g++`. CI runs `ctest -LE gui` on Linux, but no workflow installs iverilog, yosys or verilator (iverilog is a 2.2 MB
download, 6.8 MB installed), so CI has never run the differential.

### 1.3 What the differential caught before

The text goldens cannot see a behavioural divergence; the differential can, and in the component library it caught
two, each now a rule for the generators (P8.D11):

- **Gated clocks.** A clock gated by a condition (`Clock AND phase`) becomes the trigger of `always @(posedge gated)`;
  when the condition changes while the clock is high, the wire glitches and iverilog takes the glitch as an edge, which
  the simulator does not, so a program counter ran one increment ahead while the cycle counter on the main clock
  matched. The divergence showed only once the data path changed: a multi-cycle CPU's hid until the CPU was made
  programmable.
- **A structural flip-flop behind a hold on its own bit.** The gate-built D flip-flop inside the 1-bit register, whose
  hold path feeds Q back through a Mux to D, made a combinational self-loop through the transparent master that
  glitches under iverilog's event-driven run while the simulator does not; the differential failed for the register and
  its six embedders while the simulator-only tests passed, so the register keeps the native flip-flop. A counter that
  embeds the same component exports cleanly, since its flip-flops' D comes from other flip-flops.

## 2. What plan 8 builds

### 2.1 Identifiers

Plan 2's fix is kept: element names from the factory's untranslated name; port names from source names, which plan
3's descriptors hold; IC pins derived through the boundary element they stand for; the file header's pin
names from source names, so the output does not depend on the interface language. Plan 1's golden under a tagging
translator pins both generators, with the three displays (7, 14 and 16 segments, the 16-segment sketch taking the
Mega), whose port names are the case a translated name would reach first.

### 2.2 On the builder

- The builder's dependency order and its map of wireless transmitters replace `Scene::sortByTopology` and
  `Scene::wirelessTxInputPorts`, which go with their tests, and `Scene::elements()` takes the builder's order (on the
  branch 22 payload goldens then listed file-backed ICs in that order, named by this cause); the priority walk
  (`Priorities.h`), which plan 6's switch kept for the exporters, goes with them.
- An IC instance's circuit is read from its definition, through the builder's compiled definitions, never from the
  instance's internal elements; no exporter includes the scene or the simulation.
- Both exporters refuse a net with more than one driver (P8.D8), which the simulator reads as Error; plan 2's
  exporters followed the Tx until then. Path-based inlining refuses a loop it cannot map, which never becomes a
  constant.
- The level reads of §1.1 take the builder's values: the SystemVerilog export writes a non-level as x, and the Arduino
  sketch, two-state, reads it as LOW and notes it (§2.6). An unconnected IC input reads the level of the input element
  it stands for, as the builder reads it, so no default copies a live status.

### 2.3 SystemVerilog that starts and computes as the simulator does

- Gates in loops are unseeded (P8.D1), since the simulator starts them Unknown: seeding the feedback registers, as an
  earlier finding asked when the old engine seeded them, made the export print 1 where the simulator reads Unknown.
- Muxes, demuxes and truth tables are `?:` trees, which merge x as the simulator merges Unknown; a padded Mux or a
  Demux's select past its outputs exports `1'bx` in its padding leaves, where the export wrote `1'b0` that the
  differential could not see.
- Clocks go through the same simple-signal wrapping as ~Preset and ~Clear: one rule for every event-control signal.
- The reset source is a register asserted at time 0 and released at time 1.
- `` `timescale 1ns/1ps `` heads every file; the commit that adds it regenerates every golden it changes (on the branch
  the line left six goldens stale).
- **Memory's Unknown in simulation-only code** (P8.D2): inside `` `ifndef SYNTHESIS ``, a memory input that is neither
  level is read both ways, as the kernels read it, so a simulation of the export reads x where the simulator stores
  Unknown, and the synthesized hardware does not change.

### 2.4 SystemVerilog in temporal mode

The export takes the exporting tab's mode (P8.D3); functional output stays byte for byte as it was. In temporal mode:

- a combinational node is `assign #d`, `d` its element's delay, whose inertial behaviour matches the core's: checked
  with iverilog 12.0, `assign #5` suppresses a 2-unit pulse, passes an 8-unit one, and restarts a replaced change;
- a memory node is a pending slot guarded by a token, not `q <= #d d`, which is transport-style (sampled twice 4 units
  apart it glitches for 4 units, which the core does not): a changed staged value increments the token and schedules an
  apply at t + d, which runs only if its token is still current; a staged value equal to the published one increments
  the token without scheduling; one equal to the current staged value changes nothing. Memory elements carry 15 to
  20 ns by default, so default circuits need the token;
- clocks follow the core's schedule: HIGH at an even edge count, edges at the shift plus k half periods for k ≥ 1,
  clamped, negative shifts included.

The differential compares temporal runs at the core's instants.

### 2.5 The Arduino export stays functional

No temporal Arduino mode (P8.D4); clocks stay on `elapsedMillis`. Each loop runs rounds of sample, commit and settle, so
a ripple counter ripples one stage per round as the core does; flip-flops sample after the combinational settle
converges, within a bound derived from the circuit instead of the old engine's 10 (on `temporal-engine` the exporter
disagreed with the engine on sequential timing); the reset source is a flag held through the first settle.

### 2.6 Exporter notes

Each exporter returns, as data, the ways its output differs from what the application shows (P8.D5); the export writes
them into the generated file's header and shows them to the user when there are any. The Arduino export's notes: the
inputs nothing drives whose default is Unknown, which the sketch reads as LOW, found through the builder's list of
undriven inputs, so an IC's internal inputs count too (`temporal-engine`'s disclosure scanned the top level only); a
zero-delay loop that does not settle, Unknown in the application and the last of its passes in the sketch; and, when
the exporting tab runs in temporal mode, that the sketch runs without propagation delays, naming each element whose
delay differs from its type's default, read from the netlist (plan 7 makes the delay each element's parameter). There
is no such note in functional mode, and naming every element with a delay would fire on nearly every circuit, since
most logic types default to 5 to 20 ns (P8.D4).

### 2.7 The differential

- **One harness:** one runner for every SystemVerilog testbench, returning its output; one native `g++` harness for
  the sketch, its host shim and Arduino stubs in one copy (`master`'s ripple-counter test and the branch's sequential
  test each had their own).
- **Expected values** come from plan 6's core through the lockstep harness, the reference agreeing in-domain. Each
  holds 1, 0 or no level for each output:
  recording a reading of Unknown or Error as 0 made six sequential circuits fail (the PISO and SIPO shift registers,
  the loadable counter, the stack pointer, the stack memory interface and the flag register) where the export
  correctly printed x.
- **The oracle** (P8.D2): steps outside the simulator's exact domain are recorded per settle with their cause: a race
  (orders disagreeing, with two or more causes, at a loop or memory node), an approximation (a loop past its work
  limit) or a trip; the race needs a counter in the core and the reference, beside the loop's work-limit exits, which
  this plan adds with a test that a raced latch counts one and an unraced one none, and which plan 6's lockstep
  compares. In-domain steps compare 1, 0 or x per output, x as x (the combinational vectors expected 0 for a
  non-level); out-of-domain steps go to a per-circuit oracle file with their cause, checked both ways as plan 5's
  causes file is: every difference is explained, and an explanation that no longer occurs fails. An Error reading
  failing the test was considered and withdrawn: Error exports as x, as Unknown does.
- **A floor** of in-domain readings per output, recorded and compared exactly: a testbench that checked nothing passed
  (`PASS: 0 steps`).
- **Stimuli:** data changes are serialised on both sides, the generator changing one input per settle, the testbench
  one per delay, the clock a change of its own; the simultaneous-change vectors stay beside them. Stimuli move the
  data path, since a divergence can hide while outputs stay constant.
- **Text:** comments and test text that describe a seeded export or the deleted engine are written for the new
  exporters, and a test comment's "as the core does", which does not hold for Error, goes.

### 2.8 CI

iverilog joins the Linux jobs that run the suite (P8.D6); the hand-written iverilog tests and the differential fail
instead of skipping where CI requires the tool, and the differential runs on every run, not only when goldens are
generated. Whether the runner image already carries iverilog was never checked, and does not matter once CI installs
it. Whether yosys and verilator join it too is settled here (P8.16), once the run time of all validation is measured
(P8.17).

### 2.9 The IC generators' rules

Every memory element is clocked from the main clock, with enables on the data side (write enable, load, increment)
instead of a gated clock; no gate-built flip-flop sits behind a hold on its own bit (P8.D11). Both are written in the
generators' notes, and the differential runs after any change to a generator, which CI makes automatic.

### 2.10 External corpora, for development

Kept outside the repository at pinned versions, never committed or redistributed, so their licences do not limit the
choice (P8.D7); run through the differential in both modes, with the simulator's race, approximation and trip counters
reported per corpus. In order: VerilogEval's netlist-shaped problems; the ITC'99 benchmarks b01, b02, b03 and b06,
from the I99T release itself (`cad-polito-it/I99T`, commit `8a2c3b50`, 2025-05-14; `b01.bench` 1,369 bytes,
`b02.bench` 923, `b03.bench` 4,714 and `b06.bench` 1,669), since a mirror's copies come from an older converter and
differ in body and gate count; Digital's single-bit test circuits (28 by the selection rule); then Icarus's and
Verilator's tests, and ISCAS'85 and '89 from a mirror, their origin site not resolving. The corpora measure what the
suite cannot: the share of out-of-domain settles per circuit (P8.18), and whether the export's x matches the
simulator's Unknowns that no race causes, from zero-delay loops (P8.19).

### 2.11 The IC instances' internal elements

Plan 6's switch left them for the exporters; once the exporters read the builder, nothing reads them, and this plan's
last step deletes them (P8.D10): the IC's definition reference becomes a parameter read through the registry, the
IC's own `save()` and `loadState()` go, the definitions move into the document's registry and the IC folds into the
one element class (plan 3's P3.26, plan 4's P4.11); tests that edit an instance's internals (a label or blob reload
inside an inline IC, moving an internal element) are rewritten, and IC tests read the designed circuit from the
definition.

## 3. Pitfalls

- A change to one component's emission reaches every embedder; find the embedders in the generators, which name their
  sub-components in plain text.
- A line added to every exported file changes every golden; the commit that adds it regenerates them all.
- A truth table driven through `setOutputValue()` tests only power-on on the new core, which reads an input element's
  on state; drive inputs with `setOn(bool)`.
- A gated-clock divergence shows only once the data path changes; a stimulus that holds the data still hides it.
- The ITC'99 files differ between the I99T release and a mirror; measurements made on one do not carry over.
- Lint the Python generators as the repository's linters do, and keep them in `Tests/Integration/IC/Generators/`.

## 4. Steps and slices

| Row | Step | Slice |
|---|---|---|
| P8.1 | Identifiers from untranslated names and source names; the three displays under a translator (§2.1) | 1 |
| P8.2 | Both exporters on the builder's order, Tx map and definitions; the scene's helpers go (§2.2) | 1 |
| P8.3 | Two drivers and an unmappable loop are export errors; the four level reads on the builder's values (§2.2) | 1 |
| P8.4 | SystemVerilog starts and computes as the simulator: unseeded loops, `?:` trees, clocks wrapped, the reset source, the timescale (§2.3) | 2 |
| P8.5 | Memory's Unknown in simulation-only code (§2.3) | 2 |
| P8.6 | The Arduino sketch: rounds, a derived settle bound, the reset flag (§2.5) | 3 |
| P8.7 | Exporter notes as data, in the header and shown to the user, the Arduino export's note on dropped delays among them (§2.6) | 3 |
| P8.8 | One testbench runner, one `g++` harness with one copy of its stubs (§2.7) | 4 |
| P8.9 | The oracle: values with no level, x as x, a floor per output, steps out of domain by cause with the race counter, explanations both ways, serialised stimuli (§2.7) | 4 |
| P8.10 | Comments and test text written for the new exporters (§2.7) | 4 |
| P8.11 | iverilog in CI; the tests fail instead of skipping; the differential on every run (§2.8) | 5 |
| P8.12 | SystemVerilog in temporal mode (§2.4) | 6 |
| P8.13 | The IC generators' two rules in their notes (§2.9) | 6 |
| P8.14 | The external corpora through the differential, for development (§2.10) | after |
| P8.15 | The IC instances' internal elements deleted, the IC folded into the one element class (§2.11) | 7 |

The checks and open questions of §6 are rows too (P8.16 to P8.19).

## 5. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P8.D1. The SystemVerilog export starts and computes as the simulator does:** gates in loops unseeded; muxes,
  demuxes and truth tables as `?:` trees, which merge x as the simulator merges Unknown; the reset source asserted at
  time 0 and released at time 1. Basis: measured on the branch (seeding the feedback registers made the export print
  1 where the simulator reads Unknown; with the expected values recorded as 1, 0 or no level, the export printed x
  where the simulator read Unknown in six sequential circuits that had failed when an Unknown was recorded as 0); the
  user's ruling of 2026-10-16.
- **P8.D2. Memory's Unknown in simulation-only code, and the differential's oracle:** x compared as well as levels,
  except where a race causes the Unknown, recorded by cause; every difference explained, and an explanation that no
  longer occurs fails; a floor of checked readings per output; one data input changed per settle, the
  simultaneous-change vectors kept. Basis: read on `master` (a clock from 0 to x is a rising edge that stores D, and
  an x on ~Preset takes the `else` branch, where the simulator stores Unknown unless both readings agree) and on the
  branch (a testbench that checks no reading passes; the combinational vectors expect 0 for a non-level); the user's
  ruling of 2026-10-20.
- **P8.D3. The SystemVerilog export takes the simulation mode:** in temporal mode `assign #d` and a token-guarded
  pending slot; clocks on the core's schedule; functional output byte for byte as before. Basis: measured with
  iverilog 12.0 on 2026-10-19 (§2.4); the user's ruling of 2026-10-19.
- **P8.D4. The Arduino export stays functional,** its clocks on `elapsedMillis`; in a temporal tab it notes the delays
  it drops, naming each element whose delay differs from its type's default. Basis: read on 2026-10-19 (most logic
  types carry 5 to 20 ns by default, so a sketch could not honour them, and a note naming every delayed element would
  fire on nearly every circuit); the user's rulings of 2026-10-19, of 2026-10-20 for the note's condition, and of
  2026-10-21 for the plans' order, which puts the note here, after plan 7.
- **P8.D5. Exporter notes as data,** in the file's header and shown to the user when there are any, the note on
  dropped delays among them. Basis: read on `master` (an export shows a status message for 4 seconds, and the Arduino
  export's one disclosure is a comment); the user's ruling of 2026-10-20.
- **P8.D6. iverilog in CI, and the differential on every run;** the iverilog tests fail, not skip, where CI requires
  the tool. Basis: read (no workflow installs iverilog, yosys or verilator, the runner image's README lists none of
  them, and a normal run compares only the generated text, so CI has never run the differential); the user's ruling of
  2026-10-19.
- **P8.D7. External corpora, for development only,** outside the repository at pinned versions and never committed.
  Basis: read on 2026-10-19 and 2026-10-21 (the ITC'99 files from the I99T release, a mirror's differing); the user's
  rulings of 2026-10-19, and of 2026-10-21 for development only, which makes the licences moot.
- **P8.D8. A net with two drivers is an export error.** Basis: the user's ruling of 2026-10-16; the simulator reads
  it as Error (plan 5), where plan 2's exporters follow the Tx.
- **P8.D9. This plan starts once plan 7 has landed,** compares with plan 6's core, and lands before any release that
  follows plan 6's switch. Basis: the user's rulings of 2026-10-21 (no release in between, and the plans' order, one
  at a time); until it lands the exports and the simulation follow different rules.
- **P8.D10. The IC instances' internal elements go as this plan's last step.** Basis: the user's rulings of
  2026-10-21 (their removal once neither the core nor the exporters read them, and the plans' order); plan 6's switch
  removes the simulation's use of them, and the exporters walk them until this plan.
- **P8.D11. Two rules for the IC generators:** every memory element clocked from the main clock, with enables on the
  data side; no gate-built flip-flop behind a hold on its own bit. Basis: measured by the differential (§1.3); the
  user's acceptance of the restart's write-up, 2026-10-21.

## 6. Claims to check and open questions

- **Whether CI installs yosys and verilator too** (P8.16): iverilog is decided; the other two follow the run time.
- **The run time of all validation on every run** (P8.17): the differential, yosys and verilator, measured on CI.
- **The share of out-of-domain settles per circuit** (P8.18), on the suite and the corpora, which decides whether an
  exact-domain oracle covers enough.
- **Whether the export's x matches the simulator's Unknowns that no race causes, from zero-delay loops** (P8.19).

## 7. Risks

| Risk | Mitigation |
|---|---|
| An export diverges from the simulator only once a data path moves, so constant outputs hide it | The differential on every run; a floor of in-domain readings per output; stimuli that move the data path |
| Unknown, now common at power-on, reaches the exporters' level reads | Those reads take the builder's values: x in SystemVerilog, LOW with a note in the sketch (§2.2) |
| A release ships exports and a simulation that follow different rules | No release between plan 6's switch and this plan's landing (P8.D9) |
| A change to one component's emission breaks its embedders | The differential on every run, after any generator change; embedders found in the generators |
| A testbench passes having checked nothing | The floor per output |
| Validation's run time grows past what CI affords | Measured (P8.17); the corpora stay out of CI unless cheap |

## 8. What other plans take from plan 8

- **Plan 9:** exporters that no longer include the scene, so its layers place them in the model; the IC as one
  element class, with no internal elements, so its document's elements are the circuit's own; the priority walk,
  `Scene::sortByTopology`, `Scene::wirelessTxInputPorts` and the old engine's settle bound gone, which plan 6's switch
  kept for the exporters.

## 9. The branch's work, as evidence

Each step was built on `gpu-canvas`, where its commits show the change and its tests; plan 8 writes its own on
`master`. A hash is the branch's, kept read-only.

| Step | Commits on the branch |
|---|---|
| Identifiers from untranslated names; the displays under a tagging translator | `bea9b2e95`, `e187dad16`, `d533ea57c`, `e2425e062` |
| The exporters on the builder; refusing what the core does not define; payload goldens in the builder's order | `1b37acc62`, `eed93afca` |
| The reset source in both exports | `5489e75e8` |
| The Arduino disclosure of open Unknown inputs | `981667f35` |
| One testbench runner; the sequential sketch run natively; the truth table driven through `setOn` | `5b816b796`, `03ef85147`, `eb85df837` |
| The Arduino sketch ripples a stage per round | `f520738f1` |
| SystemVerilog starts and computes as the core; a padded select exports x | `3a61967ec`, `3a462d8ba` |
| The differential's vectors from the lockstep harness; an output with no level not checked | `21e8ed583`, `737b6ed4a` |

The branch never built the SystemVerilog export in temporal mode, memory's Unknown in simulation-only code, the
exporter notes, the floor, x checked as x, the race counter, iverilog in CI or the corpora runs.
