# Plan 6: the simulation core

The data-oriented core that implements plan 5's spec in both modes, in the model; its netlist builder, the first
landing, checked against plan 5's reference before the core is built on it, which plan 8's exporters use too; a runner
in the application that drives the core from a timer, holds the mode and the speed per tab and publishes changed slots
at the display rate; the reset source as a whole element type; and the switch, which moves every caller of `master`'s
engine to the runner and deletes that engine, every changed golden named by plan 5's list. Temporal mode runs in the
core and the runner, and the application offers it with plan 7.

- **Depends on:** plans 1 to 5, the Qt floor's and the exception boundaries' plans, landed, since the plans run one
  at a time, in their order (the plans' rules, §1): plan 5 (the spec, the reference simulator, the oracles, the
  random circuits, the spec's traces and the list of differences from `master`); plan 3 (each descriptor's kernel and an
  input type's source level, and the include check of the model's direction, which this plan extends to the runner);
  plan 4 (one shared definition per IC file or blob, which the builder compiles once); plan 1's goldens; plan 2's fixes
  to `master`'s engine, which the core keeps.
- **Lands before:** plan 7, which offers temporal mode in the application; then plan 8, whose exporters move onto this
  plan's netlist builder and compare with its core, and which lands before any release that follows this plan's
  switch; then plan 9, which starts from a `master` holding the core.
- **Branch and ledger:** `06-core`, from `05-semantics` once plan 5 has landed, in slices (the plans' rules, §1 and
  §3).
  `LEDGER_06_core.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing". This
  file states plan 6's own: the core implements plan 5's spec and nothing else, so a question about a rule goes to the
  spec first; every part of the core runs in lockstep with the reference from its first commit; and the switch names
  every behaviour of `master`'s engine it keeps or drops. Its decisions are numbered P6.D1 on.

## 1. Where `master` stands

### 1.1 The engine and its callers

`Simulation` (`App/Simulation/Simulation.cpp`, 693 lines; its header 212) runs a 1 ms timer, whose slot runs the tick
inside the application's guarded slot: an exception thrown across Qt's dispatch would reach `std::terminate` on macOS,
while direct callers (BeWavedDolphin's sweep, MCP, tests) still see it. Each element computes in its own
`updateLogic()` with its state in `ElementSimState`, in the order of the priority walk (`App/Core/Priorities.h`);
feedback settles by up to `kMaxSettleIterations` (10) passes, warning once when they do not converge; each IC instance
simulates its own internal elements (`ICSimulation`, behind the `SimulationHost` interface). Latches and flip-flops
commit their outputs after the sweep, like SystemVerilog's non-blocking assignments, and sample the values their inputs
had at their previous evaluation (`m_simLastValue`, `m_simLastJ` and the like), which gives `master` its zero setup
time. A tick at a fixed point with no input change is skipped, and a structural edit clears the fixed point. After a
tick the statuses are pushed into every port, wire and output element: the visual push, run at the display's refresh
rate (60 Hz when it cannot be read), not at every tick. A throttle disabler forces the push on every tick for sweeps
that read outputs after each step; its destructor re-enables the throttle rather than restoring it, so nested disablers
would undo the outer one (no caller nests them).

Who reaches the engine: the scene, the workspace, the main window, the scene-to-interface binder, the export and IC
controllers, BeWavedDolphin's waveform simulator (which drives each element's simulation-state hooks), the element, file
and simulation MCP handlers, 98 test files through `initSimulation()`, and the fuzz harnesses that load circuits.
`SimulationBlocker` stops the timer from 16 places in the commands, and from the IC registry, the clipboard, the wire
routing, the workspace (three), the export controller (two) and BeWavedDolphin. `buildConnectionGraph` serves the engine
and two tests (`TestArduino`, `TestSimulation`). Plan 5's §1 describes what the engine computes; its list of differences
is what this plan's switch changes.

### 1.2 What the exporters take from the engine

Both exporters take the scene's elements in the priority walk's order (`Scene::elements()` returns
`Scene::sortByTopology`, which runs the walk), find wireless drivers with `Scene::wirelessTxInputPorts`, and walk each
IC instance's internal elements, which the Arduino exporter sorts with the same walk; the Arduino exporter also writes
`Simulation::kMaxSettleIterations` into its sketch as the bound of its settle loop. Plan 8 moves the exporters onto the
netlist builder; until it lands, the switch leaves them what they read (§2.13).

### 1.3 Where a status is read as a level

`== Status::Active` appears 74 times in 19 files of `App` and `MCP`. 27 of them go with the old engine at the switch:
the six memory elements' and the truth table's `updateLogic()` (18), the select decoding of `ElementSimState`, the
rotary's sweep hook, and the status operations (7), which follow plan 5's rules or go with the Error-first folds. The
other 47 read what the core will publish: the 7-, 14- and 16-segment displays' painting (8, 15 and 17), the LED's
colour, the audio output's refresh, MCP's output-value query (2), BeWavedDolphin's restore of the inputs it captured
(which already puts a non-level back as it was), and both exporters' `highLow` (plan 8's). `defaultValue()` is read by
both exporters, the element simulation state, the engine and the port; `static_cast<bool>` turns no status into a
level on `master` (the element editor's casts a number). Under `master`'s rules Unknown is rare once a circuit runs;
once combinational outputs power on Unknown, it reaches every one of these. On an earlier branch, six such boundaries
each picked a different wrong answer for the same Unknown (`false`, a raw −1, a clean high, a −1 that broke a column
count, `1'b0`, and `true` written back into live state); they were found by enumerating `== Status::Active`,
`static_cast<bool>` and `defaultValue()` across the repository, not one at a time, and the canvas, which had drawn
three states correctly for years, already held the answer.

### 1.4 The data-oriented spike

Before the branch's core, a spike (`soa_spike` on `spike/gpu-scene`) asked whether a structure-of-arrays layout pays. It
flattened a circuit into one status byte per output slot (constants first, so an unconnected input is just a slot
index), per-node kind, input range and output base, inputs as slot indices, a topological order, and memory staged and
committed after the sweep, for the fixtures' seven types (switch, clock, LED, And, Or, Xor, D flip-flop), and compared
every output of every node after every tick with `master`'s engine (non-interactive, so its push ran every tick): no
mismatch over 500 to 1,000 ticks at 1, 8 and 64 input flips per tick. Both evaluated the whole circuit every tick.

| | clocked, 8,001 nodes | combinational, 20,000 nodes |
|---|---|---|
| `master`'s tick (logic and visual push), wall | 4.0–4.5 ms | 8.1–8.2 ms |
| Array tick (logic only), wall | 0.09 ms | 0.14 ms |
| Instructions per tick, `master` / arrays | 3.3 M / 1.03 M | 4.0 M / 1.48 M |
| L1 data read misses per tick | 187 K / 6.1 K | 317 K / 5.3 K |
| Last-level misses per tick (callgrind's cache model) | 24 K / about 0 | 154 K / about 0 |
| Heap: object model after load / arrays | 46.6 MB / 0.24 MB | 78.9 MB / 0.43 MB |
| Status palette gathered from objects / from arrays | 0.20 / 0.036 ms | 0.60 / 0.056 ms |

`master`'s instructions went 44% (clocked) and 49% (combinational) to the visual push (`setStatus` on every output
port, its fan-out to wires, the port layer's change notification, the LED's refresh), with 40 to 66% of the cache
misses; 33% and 44% to the logic sweep; the rest to per-tick vector copies and the staging of memory. The win is mostly
memory behaviour. What it did not show: the other kernels, real clock timing, wireless, snapshots, feedback and ICs,
which are more kernels rather than another layout; the comparison is not like for like, since the arrays have no push
(renderers read them, which the palette row measures); the timings came from a virtual machine; a steady tick costs
`master` 21 K instructions, so idle circuits gain nothing; and propagating only changed cones helps either layout.

## 2. What plan 6 builds

### 2.1 Status in one byte

`Status` takes `qint8` as its underlying type: the core keeps a status per output slot of every node and a staged copy
of each. No file holds a `Status`, and code that compares one with a number converts it explicitly; a test checks the
size, and plan 1's payload goldens show that no byte of a file moves.

### 2.2 Each logic type's delay

The descriptor of each of the 16 logic types gives its propagation delay in nanoseconds, from `temporal-engine`: Not 5;
Nand and Nor 8; And, Or and the truth table 10; Xor, Xnor, Mux and Demux 12; latches 15; flip-flops 20 (P6.D5). Having
a delay is having the entry (a zero default is possible); every other type has none. The netlist takes each node's
delay from its type until plan 7 makes the delay a parameter of each element, and plan 5's reference reads it from the
same place instead of from its caller.

### 2.3 The netlist builder, the first landing

From the registry's elements in ascending id, or from a given list in its order, never in a positional order:

- a node per element with outputs: its kind (the descriptor's kernel), delay and truth table; its inputs as slot
  indices, with four constant slots first, so an open input reads its default's constant slot with no branch; the
  power-on value of every slot (a source's level, a clock HIGH, a combinational output Unknown, memory at its power-on
  state); the elements that own the nodes; the successors, inverted from the inputs, deduplicated, in compressed sparse
  rows; a flag per node for an open input whose default is Unknown, decided by the input's own wires, not by the value
  it reads (a gate fed through a junction takes no shortcut);
- junctions, IC pins and wireless Tx and Rx as aliases, walked to the driving slot; a net with more than one driver
  reads Error and warns once, the warnings sorted; a loop of aliases with no node reads Unknown; wireless labels
  resolved once per scope (the top level and each IC instance), after the boundaries are mapped;
- an unconnected IC input reads the level of the input element it stands for, which the definition holds, and a clock
  pin reads Unknown: the builder reads the level, so the loader no longer copies a live status into the pin (§2.13);
- each IC definition compiled once, from plan 4's shared definition, into a node range each instance fills, its pins
  reading the instance's input ports; compiled definitions cached by the definition and every nested one, depth first,
  kept only while a build uses them, so a changed nested file recompiles its users; nesting deeper than 16 refused (the
  limit a constructor parameter, for tests);
- one iterative Tarjan pass that numbers the components in topological order, the members in compressed rows, checked
  by a test against a reachability oracle;
- the input limits asserted: 11 inputs for a node with outputs, 5 for a memory node (P6.D4);
- a failed build (nesting, a wire from outside the circuit, an IC output with nothing behind it) keeps the previous
  netlist, its cache and the ports' slots; a successful one sets every top-level port's slot (−1 until then, reading
  its default).

Its tests, from the branch: the condensation against a reachability oracle on 400 random graphs; nodes against their
elements on random circuits and on the IC fixtures up to the multi-cycle CPU; inputs reading the slots their wires give;
kernels, delays and tables from the elements, a table inside an IC included; power-on values; successors; an Rx reading
its scope's Tx; a duplicated label and two wires reading Error; an alias loop reading Unknown; instances sharing one
compiled definition; a nested change recompiling its users; the nesting limit; a failed build keeping the netlist; the
registry, not the positions, ordering the nodes; the switch, button and rotary boundaries reading their levels. Its
nodes are compared with plan 5's reference, which flattens the same circuits itself.

### 2.4 The kernels

Free functions over the netlist and an array of slot values, reading through slot indices without allocating:
combinational kinds evaluate into an output buffer, the open-Unknown flag first; memory kinds keep a small state (the
stored Q and ~Q, the last control levels, whether the node is fresh), one function deciding whether to sample and one
storing; sources give a constant's power-on value (an IC's constants have no element), an input's level read when
asked, and a clock's timing from its frequency, phase and lock, with its level and next edge (both clamping a negative
quotient). Every reading follows plan 5's §2.3. They are checked as plan 5 checks the reference's: against `master` on
levels, against the rule on non-levels, the flat-order sweep; and against the reference's kernels on every input
vector of the four values. On the branch every kernel ran exhaustively up to three inputs over open, low, high, driven
Unknown and driven Error, 300 samples beyond, and a Mux select past its data inputs; each memory kind runs over 1,500
random ticks that move one switch a tick, so every change has one cause and the test needs no every-order merge.

### 2.5 The core

`SimulationCore`, in the model: the arrays of §2.3 and per-node state, run by plan 5's rules (its §2.5 to §2.12):

- an instant of rounds read by cause: every order of a node's changing inputs' causes; pulses, whose every value
  memory and loops take, and Unknowns merged from orders that disagree, divergent until the instant ends; memory
  sampling once its round's logic has settled, over every order of its input changes, so a trip never samples early;
  zero-delay loops by A then B over every order of their events with the subset search, the work limit (named apart
  from the round budget), its tier and the fallback, each exit counted; trips only on a repeated state, the round's
  pulses in it; the budget of 1,000 rounds; publications after the round;
- delays as inertial publications at `t + d`, the temporal tie merged in the first round, a tie the first pass finds
  undoing the pass;
- clocks: functional ones on wall time, an edge once more than a half period has passed, every edge due in a tick run
  at its time; temporal ones as events on their phase grid; a lock that stops a clock and an unlock that drops the
  missed edges; a settle's hold moving every temporal clock's phase, a locked one's included;
- ticks: a functional tick runs everything due since the last in time order, each distinct time an instant; a temporal
  tick runs the user's changes as an instant of their own, then drains `[T, T + w)`; both check one wall-time deadline
  after each completed instant (P6.D11), stopping time at the next instant due, and the next tick goes on from there;
  timed source changes wait for their time, untimed ones are seeded at the start of the tick that sees them; only the
  changed sources are seeded, a source seeded back to its published level drops its pending publication, and an
  instant with nothing forced, flagged or due does not run;
- per-node state with no list searches, so sampling stays linear in the flip-flops on one clock (`temporal-engine`
  sampled on pointer-keyed hashes, quadratic: 609 ms per edge pair at 8,000 flip-flops).

A check that must hold in release is not an assertion: a snapshot restored onto another build is a fatal error in every
build (an assertion compiles out in release and would corrupt silently).

### 2.6 Snapshots, the sweep guard and settle

A snapshot copies the slots, the node state, the queue, the clocks, the time, the trips and the warnings, shared
implicitly until either side writes; the work counters keep counting across a restore. A sweep guard snapshots when made
and restores when destroyed, and guards nest. `settle(instants)` runs instants while one is due (a source changed, a
publication pending, a build or a trip left nodes to evaluate), each at the earliest time due, within a budget of
instants; it seeds no clock, holds the temporal clocks for the simulated time it took (their pending edges and phase,
per node, surviving rebuilds) and leaves the functional clocks to wall time. Found on the branch: a source or a clock
seeded back to its published level left its pending publication live when a zero budget left the seed unapplied; it
is superseded.

### 2.7 Rebuilds and the mode switch

A node keeps its published outputs, staged values, memory state, clock and pending publications when its element
survives with the same kind and port counts, or sits at the same place in an IC instance with the same compiled
definition; new nodes power on. A surviving combinational node keeps its outputs through an input-count change (a
memory node needs its ports unchanged), and a fresh combinational node takes its result in passes against the published
values until none changes, capped at one pass more than the fresh nodes, so a chain of new gates settles at the
rebuild. A mode switch keeps every value and seeds nothing. Plan 5's rebuild cases run in lockstep: an And that loses an
input while feeding a D flip-flop keeps its output in temporal mode, where it fabricated an edge on the branch, and in
functional mode, a twin the branch never wrote; and a reload of a file-backed IC that keeps its definition keeps its
state.

### 2.8 What the rules cost

On the 73 component walks, optimized: 2.48 s of CPU before the pulse model's correction; 2.55 s with pulses moving and
read in every set; 3.36 s with A then B after each loop event; 4.19 s with transients as pulses and the pulse trip; and
18.2 s once memory and loops branch on pulses, 82% of that in two gate-built register circuits (the 8-bit stack
pointer and the stack memory interface, each a NOR-latch master-slave flip-flop in one zero-delay loop with its adder
bit and Mux). Two implementation changes, which alter no value and no fallback, brought it to about 1.07 times the
original, and the core is written with them from the start:

- a flat loop state: one preallocated array, each outside input resolved once per settle, no allocation per evaluation,
  the states reached in a hash set;
- a settle step made again is reused: A then B from a state, at a subset of the events, under a combination of pulses
  and divergent inputs, is a function of those alone. It still counts against the work limit.

Worst cases, 50 functional ticks each: a Mux with all 11 inputs on switches (2^11 sets of events) 0.57 to 0.76 ms per
tick; a ring of six NOR latches, one 18-member loop with 8 events, 4.0 to 4.3 ms; the ring with four pulsing set inputs
(the cap on branched inputs) 4.2 to 4.4 ms, against 63 ms before the two changes. The deadline is checked only between
instants (P6.D11), so the round budget alone bounds one instant; P6.30 times the worst instants on the core.
Further levers, if the measurement after the plan asks for them: re-evaluate only the members whose inputs moved within
A and B, and skip B's first evaluation after A. With the two changes the branch's suite ran 249 tests in 36 s optimized
(49 s before), and the every-step check passed all 102 walks in 518 s in one process, or 135 s on 8 cores.

### 2.9 Equivalence

- **The lockstep:** a harness runs a circuit in the core and in plan 5's reference and reports what differs after each
  operation: every port, every slot inside each IC compared node by node by its place (the top-level element and the
  index path through the ICs nested in it), trips, warnings, time and the settle's result. On the branch IC internals
  were compared as a sorted multiset of published outputs per IC, so two nodes swapping values inside one IC went
  unseen; the reference already keys each node by its place, which makes the exact comparison possible.
- **Its walks**, from plan 5's random circuits: change inputs and tick (functional: long wall-time jumps, or short
  steps onto a 25 µs grid; temporal: windows of 1 to 5, 1 to 100 or 1 to 1,000 ns), settle with a budget of 0 to 7,
  sweep inside a guard (after which values, time, trips and warnings are as before and the lockstep agrees), switch
  mode, or edit (add or remove an element or a wire, change a delay, a clock, a gate's input count or a wireless label)
  and rebuild. A failure names the step, the last ten operations and what differed. 60 seeded circuits of 120 steps per
  mode run in the suite, which asserts that trips, cut-short settles, sweeps, edits and warnings happen. On the branch a
  throwaway run of 1,500 circuits per mode (about 217,000 ticks, 64,000 settles, 25,000 sweeps and 54,000 rebuilds, with
  1,057 trips) agreed.
- **Plan 5's checks on the core:** every hand-written case of plan 5's §4.4 in lockstep; the exact rounds from the
  core's state; the every-order oracles; memory over every order; the every-step walks, which add the core.
- **The fuzzer:** a libFuzzer harness takes one byte for the mode and every other choice of the random circuit from the
  input until it runs out, up to 400 steps; any difference aborts. It is registered in both fuzzer presets, the
  unattended-run script and the coverage report, with its own corpus directory, and checks its seed at start-up
  (`CLAUDE.md`, "Building and testing"). Its coverage feedback needs the fuzzer flag on the whole project, which plan
  2 gives it.
- **Every fixture, example and IC component** runs on the core at power-on and clocked; none trips or reaches the
  budget unless it is meant to, and each one meant to is named in its test.
- **The review of `temporal-engine`, its core-level cases as regression tests:** a 1 kHz clock into a T flip-flop makes
  the same transitions in the same simulated time at every speed (with clocks on wall time it made 100 at normal speed
  and none at 1 ns per tick);
  sampling stays linear in the flip-flops on one clock; no latch race is decided by canvas placement; a trip never
  samples a cone that has not settled; and the trip rule, which replaced that branch's untested canonicalisation, has
  its tests.
- On the branch, 14 of the 17 mutants of the queue, the snapshots and settle failed the lockstep alone; the others were
  an off-by-one at a cap, which a random transient rarely meets exactly, and the work counters, which the lockstep does
  not compare, so the caps have unit tests of their own (plan 5's P5.22, on the core too). A strict-clock mutant
  survived until the walks' short steps landed exactly on the 25 µs grid.

### 2.10 The reset source, whole

A new element type: a source kernel, a width in nanoseconds (default 1,000, range 0 to 1,000,000) in the parameter
schema, its descriptor, presenter, picture and own icon, its palette entry, its entries in MCP's schema and in the
Python client's element types (with the client's type-coverage test), and plan 5's reference reading it, with its draws
in the random circuits (P5.27). In functional mode it is Active through the power-on instant and released at time 1
(the second tick, or a settle that runs the power-on instant); in temporal mode Active for its width, its release an
instant of its own, run by a tick or a settle. A reset that survives a rebuild keeps its state; one a rebuild adds reads
Inactive at once. On the branch its icon was the VCC's, since the palette did not offer it yet; here its picture is
drawn for it and shown to the user before its slice is finished (P6.31). It lands no later than the switch, whose
examples use it. Pitfalls from the branch: a settle must run a reset's release when it falls due, in both engines; and
the reset source has a row in every element golden.

### 2.11 The runner

`SimulationRunner`, in the application, one per tab:

- **Its timer wakes when the core next has something due** (P6.D8): a clock edge, a publication, a release, the next
  display-rate publication of changed slots, or a tick cut by its deadline; a user change or an edit wakes it
  at once; it wakes no more often than once a millisecond, since a functional tick runs every edge due since the last at
  its own time; an idle circuit keeps it stopped. On `qtquick-rewrite`, replacing `master`'s fixed 1 ms timer with such
  a timer cut the scheduler's share of a profile of the 8,000-flip-flop circuit from 20–40% to 9.58%, and the scheduled
  CPU time of an equal-length capture by about three to five times. Its slot runs the tick inside the application's
  guarded slot, as `master`'s does, while a direct call still sees an exception; the branch's runner connected the
  timer to its tick directly, dropping the guard.
- **Running time:** it accrues only while the timer runs, so a pause keeps every clock's phase and a stop or a
  suspension neither injects an edge nor loses one; functional clocks read it as their wall time, and each user change
  is stamped with it when made (on the branch the application polled flags and gave no time, so the runner had to stamp
  them); in temporal mode the simulated time is running time × speed. A tick in either mode yields after 4 ms of wall
  time and continues at the next wake (P6.D11); while the core is behind, running time does not run ahead of it, so an
  overloaded circuit runs slower than wall time, its clocks' edges neither dropped nor bunched, and no backlog grows.
- **Mode and speed per tab** (P6.D7): a mode switch keeps every value and seeds nothing; a speed change keeps the
  simulated time elapsed. Plan 7 adds the selectors.
- **Publication:** the output slots that changed since the last publication, at the display rate (60 Hz until the
  display's rate is known), or at every update when the visual throttle is off; ports take the new values, the output
  elements refresh, and a signal carries the changed slots, which plan 3's audio service and plan 9's renderers read.
  A test's `update()` runs the core up to the present and publishes at once.
- **Warnings:** a circuit that does not build stops the runner with its error; each warning the core raises shows once
  per build.
- **Suspension:** the runner honours the model's suspend count (§2.12) and the user's mute; the audio is muted while it
  is stopped or suspended.
- **Sweeps:** `sweep(body)` runs a body from power-on inside the core's sweep guard and publishes once the state is
  back, so BeWavedDolphin asks its question of the circuit and changes none of its state.
- **The throttle disabler** wraps the runner and restores the state it found, so disablers nest.

Its tests, headless, with a manual clock: start and stop follow the timer; the user's mute survives a stop; the audio
mute follows the run state; a suspension holds the timer and the last resume restores it; a stop and a start during a
suspension apply on resume; a suspension does not change a functional run; a pause keeps a clock's phase step by step;
a temporal run follows running time at its speed; a speed change keeps the elapsed simulated time; output ports take the
core's values; only changed slots publish; the display rate paces publications; the throttle off publishes at once; a
restart rebuilds the edited circuit; a duplicated Tx warns once per build; a circuit that does not build warns and
stops; the timer wakes at the next due edge, at once on a user change, and not at all on an idle circuit.

### 2.12 The suspend count, in the model

The core's owner in the model keeps a suspend count, which a blocker holds: the commands, the IC registry, the
clipboard, the wire routing, the workspace, the export controller and BeWavedDolphin hold it as they held
`SimulationBlocker`, and the runner, in the application, reads it. So no model code includes the runner (P6.D2), which
plan 3's include check enforces once this plan adds the runner's header to what a model file may not include; plan 9,
after this plan, extends the check to every layer. On the branch the blocker stayed in the simulation directory
while including the runner's header from the interface, the one layering defect the switch left. Until plan 9 splits
the scene into a document and an editing session, the scene owns the core and the count as it owns `master`'s engine,
and creates its tab's runner; plan 9 moves them. No model code restarts the simulation either: an IC reload, like any
structural edit, leaves the netlist to be rebuilt at the next update, keeping state by §2.7 (on `temporal-engine` an IC
reached into the scene's simulation to restart it).

### 2.13 The switch

One change moves the application onto the core, the old engine running until then:

- **Callers.** Every caller of §1.1 reaches the runner, the tests and the fuzzers included (`initSimulation()` first;
  `buildConnectionGraph`'s two tests ask the builder); the counts are taken again on `master` when the switch starts.
  The blockers hold the suspend count. BeWavedDolphin's sweep runs through the runner and the sweep guard instead of
  each element's reset, restore and waveform hooks. Output ports show what the runner publishes, an element's output
  value reads its port, and plan 3's audio service plays from the runner's notifications instead of the visual push.
- **Deleted:** `Simulation`, `SimulationHost`, `updateLogic()`, `ElementSimState`, `ICSimulation`, the iterative
  settle, the visual push, the status operations that rank Error first (`statusAnd`, `statusOr`, `statusXor` and their
  three `*All` forms; no caller is left but their tests, and `statusNot` stays), and the loader's copies of a live
  status into an IC's pins, with the tests of that machinery, each rewritten onto the core or deleted by name: a null
  guard in `IC::updateLogic`, a body test that drove a node through its simulation vectors, the rotary's reset in the
  sweep, the element logic error tests, the status operations' tests and the old trace snapshot. The priority walk
  stays, since the exporters read it until plan 8 (below).
- **Tests on the core:** the element logic, IC, feedback and integration tests move off `updateLogic`,
  `simUpdateInputs`, `resetSimState`, `kMaxSettleIterations` and the internal elements, to plan 5's harness or the IC's
  definition; each keeps its expectation unless plan 5's list changes it, named by cause. Tests `master` lacked come on
  the core: the truth table's and the Demux's disconnected inputs, sequential behaviour, a ring oscillator settling at
  Unknown, an unloaded IC and its disconnected input, a D flip-flop's edge with an Unknown input.
- **Every place a status is read as a level** (§1.3) is decided at once (P6.D10), each told what it does with Unknown
  and Error, its test reading the port's `Status` through plan 1's readers. The 27 reads inside the old engine go with
  it. Of the 47 that stay, the proposal: the displays and the LED draw a non-level as the canvas draws one, which the
  user sees before the switch lands (P6.31); the audio plays only on Active; MCP's output-value query reports the
  status itself; BeWavedDolphin keeps putting a captured non-level back as it was; the exporters' reads are plan 8's,
  and until it lands each keeps `master`'s reading, listed. The search is run again across the repository when the
  switch starts, `static_cast<bool>` and `defaultValue()` included.
- **`master`'s behaviours, kept or dropped by name**, since an earlier engine rewrite dropped the idle-tick skip as an
  undocumented side effect and made the test of a later idle flush vacuous:

| Behaviour of `master`'s engine | At the switch |
|---|---|
| A tick at a fixed point with no change is skipped | Kept: an empty queue after a drain is the idle signal, and the timer sleeps (§2.11) |
| Visuals flushed on a later idle tick | Kept: changed slots wait for the next display-rate publication, which a wake brings |
| The visual push at the display's rate, 60 Hz when it cannot be read | Kept as the publication of changed slots |
| The throttle disabler | Kept, wrapping the runner; it now restores the state it found |
| A pause keeps every clock's phase | Kept: running time accrues only while the timer runs |
| The timer's slot runs the tick inside the application's guarded slot | Kept: the runner's slot runs inside it (§2.11) |
| The convergence warning, once | Replaced by the spec's oscillation and budget warnings, once each per build |
| A clock pays owed edges one per tick, so a clock above 500 Hz runs slow | Dropped: every due edge runs at its own time |
| A clock lagging over five seconds forces itself HIGH and restarts its phase | Dropped: no clock lags |
| An unlocked clock pays the edges owed since its lock, one per tick | Dropped: an unlock drops the missed edges |
| Memory samples the previous evaluation's inputs (zero setup time) | Dropped: data changing at an edge reads by plan 5's rules |
| A non-level input writes Unknown into a memory element's published outputs, which its commit then overwrites, so `updateLogic()` called directly and a running simulation disagree | Dropped: non-levels read both ways, one answer on every path |

- **Goldens:** plan 5's spec traces become the traces, `master`'s frozen ones and the causes file going with the old
  engine's trace test; every other golden that changes (the scene goldens that record a connected IC clock pin's
  default, now Unknown, ten on the branch; the counters' files and previews, §2.14) is named by plan 5's list.
- **The exporters until plan 8,** which lands after plan 7: the switch keeps what they read, the priority walk behind
  `Scene::sortByTopology` and `Scene::elements()`, `Scene::wirelessTxInputPorts`, and the Arduino sketch's settle
  bound, which becomes the exporter's own constant of 10, so the generated text is unchanged; plan 8 removes them.
- **The lock's tooltip** gains the sentence on unlocking: a locked clock, unlocked, shows the level its schedule gives
  (plan 2 wrote the rest of the tooltip on `master`).

### 2.14 Fixtures, examples and documentation

- **The counters.** The modulo and up/down counters, which no input brings to a defined state under any power-on rule,
  gain a reset input through their generators: the modulo counter takes it in its modulo mode only, the up/down counter
  always, and the loadable counter none (its Load defines its state). Every circuit that embeds them is regenerated in
  dependency order (on the branch nothing else embedded them, which plan 4's §1.3 says to check in the generators), and
  their SystemVerilog goldens, trace goldens, scene snapshots, previews and payloads change by this cause.
- **The level-1 fixtures.** The SR latch, D latch and D flip-flop suites run on the core unchanged; the gate-built JK's
  reset helper gains an asynchronous clear, since its reset term stays Unknown from power-on and J and K pulses never
  leave it.
- **The 4-bit display counter example** gets its J and K wired and a reset source.
- **Documentation:** the project's `CLAUDE.md` simulation section and the developer guide's chapter on the engine
  describe the core and the runner (the spec stays plan 5's section); the old engine's descriptions go; the release
  notes' list of changes is plan 5's list, written at the release.

### 2.15 The IC instances' internal elements

The switch removes the simulation's use of them, but they stay (P6.D9): the exporters walk them until plan 8 moves
them onto the builder, and plan 8 lands after plan 7. Plan 8's last step deletes them (its P8.15): the IC's
definition reference becomes a parameter read through the registry, the IC's own `save()` and `loadState()` go, the
definitions move into the document's registry, and the IC folds into the one element class (plan 3's P3.26, plan 4's
P4.11). The core and the switch's tests read nothing through them: the builder compiles plan 4's definitions, and the
tests move onto the core or the IC's definition (§2.13).

### 2.16 Measured after it lands

Performance is measured once the plan has landed, on plan 1's named fixtures (the clocked 8,000, the combinational
20,000, the IC-heavy circuit) in both modes against `master`'s engine: a tick, a rebuild, gathering the statuses,
memory, and sampling, which must stay linear in the flip-flops on one clock. A netlist rebuild visible on edits of the
large fixtures is answered by patching node ranges, if the measurement shows it.

## 3. Pitfalls

- A test that drives an input through `setOutputValue()` tests nothing on the core, which reads an input element's on
  state: a truth table's every row read its power-on value that way. Drive inputs with `setOn(bool)`.
- The first update after a structural edit builds the netlist, so a test of a fast path builds first.
- Building only the test target leaves out the target that copies the MCP schema, and twelve tests fail; build all
  targets.
- A check that must hold in release is not `Q_ASSERT`.
- A fold that moves a check between functions needs a build at its target: on the branch a squash folded an
  equal-value check into `setPending()`, whose scope has no `values` but whose class has `values()`, so the name lookup
  passed and 21 commits did not compile (the plans' rules, §3).
- Times on the branch: the lockstep suite 86 s in a debug build and 16 s in relwithdebinfo; the core's traces 70 s and
  18 s; 1,500 seeds of 120 steps per mode 302 s in relwithdebinfo, which is why long runs use an optimized build.
- A test of a loop or an oscillator has a timeout: an immediate-publication mutant made rings hang.

## 4. Steps and slices

| Row | Step | Slice |
|---|---|---|
| P6.1 | Status in one byte (§2.1) | 1 |
| P6.2 | Each logic type's delay in its descriptor (§2.2) | 1 |
| P6.3 | The netlist builder and its tests (§2.3) | 1 |
| P6.4 | The kernels, checked as plan 5 checks the reference's and against them (§2.4) | 2 |
| P6.5 | The core: rounds, memory, loops, pulses and divergence, trips and the budget (§2.5) | 3 |
| P6.6 | The core: delays and ties, clocks, ticks and the deadline (§2.5) | 3 |
| P6.7 | Snapshots, the sweep guard and settle (§2.6) | 3 |
| P6.8 | Rebuilds and the mode switch (§2.7) | 3 |
| P6.9 | The lockstep, IC internals by place, and plan 5's hand-written cases on it (§2.9) | 3 |
| P6.10 | Plan 5's oracles and walks on the core (§2.9) | 3 |
| P6.11 | The fuzzer (§2.9) | 3 |
| P6.12 | Every fixture, example and IC component at power-on and clocked, nothing tripping unless meant to (§2.9) | 3 |
| P6.13 | The review of `temporal-engine`, its core-level cases as regression tests (§2.9) | 3 |
| P6.14 | Plan 5's larger loops timed on the core; the work limit set, in the spec first (§2.8) | 3 |
| P6.15 | The reset source, whole (§2.10) | 4 |
| P6.16 | The suspend count in the model (§2.12) | 5 |
| P6.17 | The runner (§2.11) | 5 |
| P6.18 | The switch: callers, blockers, the sweep, ports and outputs from the runner, the audio (§2.13) | 6 |
| P6.19 | The old engine deleted, its tests rewritten or deleted by name; the tests moved onto the core (§2.13) | 6 |
| P6.20 | Every place a status is read as a level, decided (§2.13) | 6 |
| P6.21 | `master`'s behaviours kept or dropped by name (§2.13) | 6 |
| P6.22 | The goldens: the spec's traces, every changed golden named (§2.13) | 6 |
| P6.23 | What the exporters read, kept until plan 8 (§2.13) | 6 |
| P6.24 | The lock tooltip's sentence on unlocking (§2.13) | 6 |
| P6.25 | The counters' reset input, their embedders regenerated; the JK's reset helper (§2.14) | 7 |
| P6.26 | The 4-bit display counter example (§2.14) | 7 |
| P6.27 | `CLAUDE.md` and the developer guide on the core and the runner (§2.14) | 7 |
| P6.28 | The measurement after the plan (§2.16) | after |

The checks and open questions of §6 are rows too (P6.29 to P6.32). New test classes are registered in the four places
`CLAUDE.md`'s "Building and testing" lists, and the fuzzer where it says.

## 5. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P6.D1. A data-oriented core, written once, with no object engine first:** per node its kind, input range, output
  base, delay and component, whose number is its priority; inputs as slot indices; one status byte per output slot;
  kernels per kind; a queue of timed publications and clock edges. The arrays are what the renderers and presentation
  read, through an interface this plan defines; no push of every status into every port and wire follows a tick, the
  runner publishing only the changed slots. Basis: measured by the spike (§1.4); the user's rulings of 2026-10-03/04
  (the arrays) and 2026-10-04 (written once, from `temporal-engine`'s corrected rules), and the user's agreement of
  2026-10-20 that the renderers read the arrays through an interface this plan defines.
- **P6.D2. The core in the model, the runner in the application, the state owned with the netlist; the suspend count
  in the model,** so no model code includes the runner. Basis: the user's ruling of 2026-10-03/04 for the split; read
  for the count: the commands and the IC registry, which hold the blocker, are model code in plan 9's layers and the
  runner is the application's, so a blocker that reaches the runner crosses the layers, as it did on the branch.
- **P6.D3. The netlist builder, from the prototype's intent, is this plan's first landing:** each IC definition
  compiled once into node ranges, one wireless resolution per scope, one Tarjan pass; plan 8's exporters use it. The
  prototype's `LogicNode`, dense graph and `ICNetlist` on the old engine are not recreated. Basis: measured by the
  review of `temporal-engine` (wireless channels resolved in four places that disagreed on IC scope, duplicate labels
  and a wire into an Rx; sampling on pointer-keyed hashes quadratic in the flip-flops on one clock); the user's rulings
  of 2026-10-04 and of 2026-10-21 (the first landing, and no release between this plan's switch and plan 8's landing).
- **P6.D4. The input limits asserted:** 11 inputs for a node with outputs (a Mux's 8 data and 3 select lines), 5 for a
  memory node; no code handles more. Basis: read on the branch (the core's branches for twelve or more causes were
  unreachable under these limits, and deleting them changed no circuit); the user's ruling of 2026-10-19.
- **P6.D5. Each logic type's delay in its descriptor,** the values of §2.2; the delay becomes an element's parameter
  with plan 7, where it is first offered. Basis: the user's ruling of 2026-10-21 (each piece of the delay arrives where
  it is first used); the values are `temporal-engine`'s.
- **P6.D6. The reset source lands whole, no later than the switch:** type, picture, icon, palette entry, MCP and the
  reference's reading. Basis: the switch's examples use it, so users must be able to place it; on the branch it drew
  the VCC's icon until a later phase offered it; the user's acceptance of the restart's write-up, 2026-10-21.
- **P6.D7. Mode and speed per tab, held by the runner;** a mode switch keeps every value; plan 7's selectors show the
  current tab's. Basis: measured on `temporal-engine` (a mode switch reset the circuit, and so did visiting another tab
  afterwards); the user's ruling of 2026-10-16.
- **P6.D8. The runner's timer wakes when the core next has something due,** not every millisecond, and an idle circuit
  keeps it stopped. Basis: measured on `qtquick-rewrite` (`b068b4397`: the scheduler's share of a profile of the
  8,000-flip-flop circuit from 20–40% to 9.58%); the user's choice there, 2026-07-21, of all three options offered
  (retargeting the timer to the next deadline, stopping it when idle, scheduling the next event), which compose into
  this one design.
- **P6.D9. The IC instances' internal elements stay through the switch, for the exporters,** and go as plan 8's last
  step, with the IC's own serialization, the definitions' move into the document's registry and the IC's fold into the
  one element class. Basis: the core stops reading them at the switch and the exporters when plan 8 lands; the user's
  rulings of 2026-10-21 (their removal once neither reads them, and the plans' order, which puts plan 8 after plan 7).
- **P6.D10. Every place a status is read as a level is decided at the switch, all at once,** each told what it does
  with Unknown and Error. Basis: read on `master` (74 reads in 19 files, §1.3); measured on an earlier branch (six
  boundaries, six wrong answers); the user's acceptance of the risk and its mitigation with the restart's write-up,
  2026-10-21.
- **P6.D11. One deadline for both modes:** a functional tick, like a temporal one, checks a 4 ms wall-time deadline
  after each completed instant, stops time at the next instant due and goes on at the next tick; while the core is
  behind, the runner's running time does not run ahead of it. A cap on instants per tick was the other option: it cuts
  at a count that suits neither a small circuit nor a large one. Every state shown is a real state at its time, and an
  overloaded circuit runs slower than wall time. Basis: read (functional clocks already run on the runner's running
  time; a zero-delay ring of more than about 500 memory nodes can spend the round budget at every instant, which with
  no deadline stalls the application for the whole tick); the user's ruling of 2026-10-21.

## 6. Claims to check and open questions

- **Whether an IC's first reload in a scene always brings another definition** (P6.29), as a test comment on the
  branch assumed: plan 4 counts definitions across a first reload (its P4.9); this plan checks what the core's node
  ranges do across it.
- **A tick's bound** (P6.30): decided as one deadline for both modes (P6.D11). What it leaves is one instant, which
  only the round budget bounds: the worst instants (the ring of NOR latches with pulsing inputs, a zero-delay ring of
  more than 500 memory nodes) are timed on the core, and if one instant alone stalls the application, a lower budget
  goes into the spec first.
- **What the user sees new** (P6.31): the reset source's picture and icon, and how a display or an LED draws a
  non-level, shown to the user before their slices are finished.
- **No release between this plan's switch and plan 8's landing** (P6.32), plan 7 landing in between: until plan 8
  lands, the exporters follow the Tx and read `master`'s order and Unknown, where the simulator reads Error and its own
  rules; a release in between would ship that disagreement. Closed when plan 8 lands.

## 7. Risks

| Risk | Mitigation |
|---|---|
| The core and the reference share a misreading of the spec | Plan 5's oracles on the core too; the `temporal-engine` review's measured cases as regression tests (P6.13); the traces with every difference named |
| Tests construct and drive what goes: the old engine, through `initSimulation()` in 98 test files | The shared helpers in `Tests/Common` move first; every deletion compiler-checked, never a silent no-op |
| Functional mode changes beyond races: power-on Unknown, no wire time, two drivers read Error, time-ordered ticks, rebuilds keep state | Certain; each change listed by cause (plan 5's list); the release notes |
| Gate-built latches change: a simultaneous release reads Unknown, gates power on Unknown | Certain; the counters get a reset input, the examples a reset source or an enable; the release notes |
| Unknown, now common at power-on, reaches every place a status is read as a level | The places enumerated at once (P6.20), each told what it does with Unknown and Error; the test readers that tell a non-level from 0 |
| Reading by cause costs more where independent changes meet | Bounded per node (2^11 readings) and per loop (the work limit); the two cost changes from the start; measured |
| A tick stalls the application | One deadline between instants in both modes (P6.D11); the worst single instants timed (P6.30) |
| The switch drops a behaviour of `master` unnoticed | The table of §2.13, each behaviour kept or dropped by name |
| The next-event timer misses an edge, a release or a publication | The runner's tests, with a manual clock, step by step |
| A netlist rebuild is visible on edits of the large fixtures | Measured after the plan; patching node ranges if needed |

## 8. What other plans take from plan 6

- **Plan 5:** the core joins its oracles and walks; its work limit is set from P6.14's timing; its reference reads each
  type's delay and the reset source from here.
- **Plan 7:** the core's temporal mode and the runner's mode, speed and simulated time per tab, for its selectors and
  readout; the settle, which MCP's input changes go through; the sweep guard, through which BeWavedDolphin's temporal
  sweeps run; each type's delay, which it makes a parameter of each element.
- **Plan 8:** the netlist builder (slice 1), its wireless resolution, its order and its compiled definitions, which the
  exporters read instead of walking IC instances; the reset source, which the exporters treat as a register released
  at time 1 (SystemVerilog) and a flag held through the first settle (Arduino); the core, which its differential
  compares with; the internal elements, which the switch leaves for the exporters and plan 8's last step deletes
  (§2.15).
- **Plan 9:** the core and its suspend count, which its document takes, and the runner, which its application layer
  takes; the changed slots its renderers read.
- **Plan 10:** the status arrays its GPU renderer reads, through the interface of P6.D1.

## 9. The branch's work, as evidence

Each step was built on `gpu-canvas`, where its commits show the change and its tests; plan 6 writes its own on
`master`. A hash is the branch's, kept read-only.

| Step | Commits on the branch |
|---|---|
| Status in one byte; kernels named by the descriptors | `f22237560`, `e3ca5c15c` |
| The netlist builder | `4440ea5c6` |
| The kernels | `eebb9a429`, `4b90c1394`, `3df5cd3d6` |
| The queue and drain, first form | `c0048cdb6` |
| Snapshots, the sweep guard and settle; the restore's check | `beef3ba54`, `ebf72a66c` |
| The lockstep, the random circuits, IC internals compared | `bd525357a`, `e1403720c`, `7bce09d33`, `8ac090ac9`, `0150f6a2e` |
| The fuzzer, and the fuzzer flag on the whole project | `bd94044b6`, `d2b0c4ae2` |
| Power-on Unknown and both-way reading; aliases | `4df8fb80a`, `b1995103f` |
| Rounds by cause | `a047d38e1`, `24972f49e`, `4c9811bc2` |
| Time-ordered ticks; a temporal start with no change due | `2985a993a`, `805b8b7d8`, `3dd7a4d15`, `0cdec44a0` |
| The reset source | `b663552d1`, `3d5a81a8f`, `5a86134f6` |
| Rebuilds keep outputs; locked clocks | `b5351b44a`, `487fd6c60`, `1fbc8fbe6` |
| The IC boundary in the builder | `f3537ad80`, `712dcf1bf` |
| Loops in every order; the input limits | `3f2d4cf3f`, `c484f14d6`, `c7a4a52bc`, `cd9054327` |
| The pulse model's correction, with the two cost changes | `1b1f81e76` |
| No example, fixture or component walk trips or reaches the budget | `fbaabc14b`, `7da9e861b` |
| The runner | `9be7ffe59`, `65a26d84b`, `357fd7e56` |
| The level-1 fixtures and the counters' reset | `9d5e7ed28`, `4f5d11058`, `b2f3261d5`, `c39a90fff`, `03630c8f5` |
| Tests on the core | `6348b3689`, `9e3128d75`, `98e86d997`, `e1b19139d`, `3b877f737`, `b03dd04fb`, `bfbab19ec`, `84c5188ea` |
| The switch; the Error-first folds removed | `c0877969c`, `8ca89f093` |
| `CLAUDE.md` and the developer guide on the core | `cf2ec8fd5` |

The branch never built the reset source's palette entry, picture, icon or MCP, the 4-bit display counter's change, the
runner's next-event timer, the suspend count in the model, the comparison of IC internals by place, the functional twin
of the rebuild's edge test, or the removal of the internal elements; its switch left `SimulationBlocker` including the
runner's header, and its runner's timer without `master`'s guarded slot.
