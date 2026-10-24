# Plan 5: simulation semantics and its oracles

One spec of the simulation's rules, for both modes; oracles that check those rules exactly without sharing them, the
temporal one new; a test-only reference simulator that implements the spec by definition and agrees with the oracles
on random circuits, ICs included, and on every step of every shipped walk; and the spec's traces of plan 1's walks,
each difference from `master`'s engine named by its cause, for plan 6's switch to match. It is documentation and test
code only: the application keeps `master`'s engine until plan 6.

- **Depends on:** plans 1 to 4, the Qt floor's and the exception boundaries' plans, landed, since the plans run one
  at a time, in their order (the plans' rules, §1): plan 1's simulation traces (every logic type, the six fixtures,
  the 73 IC components and the two perf circuits) and its test readers, which return a port's `Status`, so a test tells
  a non-level from 0; plan 2's fixes, which `master`'s traces hold before this plan starts; plan 3's one element class,
  whose descriptors, ports and typed parameters the reference reads in one place; and plan 4's shared IC definitions,
  which the reference flattens, so it never reads an instance's internal elements, which plan 8's last step deletes.
- **Lands before:** plan 6, whose core implements the spec and is checked against the reference and the oracles; then
  plan 7, which offers temporal mode once the exact oracle with delays agrees with the reference; and plan 8, whose
  differential compares the exports with plan 6's core, which runs in lockstep with the reference.
- **Branch and ledger:** `05-semantics`, from `04-ics-and-format` once plan 4 has landed, in slices (the plans'
  rules, §1 and §3).
  `LEDGER_05_semantics.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing"; their
  first principle, ground truth first, is this plan's subject. This file states plan 5's own: the spec comes before the
  code that implements it; a rule changes in the spec first, then in the oracles and the reference (and, from plan 6,
  the core); and every trace a change moves is named by its cause. Its decisions are numbered P5.D1 on.

## 1. Where `master` stands

`master` simulates in `App/Simulation/Simulation.cpp`; each element computes itself (`updateLogic()`) from the state it
keeps (`ElementSimState`). A 1 ms timer ticks, and each tick:

- toggles each clock at most once, when more than its half period has passed since its reference, which then advances
  by exactly a half period; after a lag of more than five seconds a clock forces itself HIGH and restarts its phase
  (`Clock::updateClock()`, `resetClock()`), so a clock above 500 Hz falls behind and, at 1 kHz, resynchronises about
  every 10 s; a locked clock keeps its reference frozen and, once unlocked, pays its owed edges one per tick, or
  resynchronises if it was locked for more than five seconds;
- lets the input elements publish, and skips the rest when the circuit is at a fixed point and no source changed;
- has the latches and flip-flops, across the IC hierarchy, stage their outputs, so the logic between them reads the
  state before the tick, and commit them together after the pass;
- runs the elements once in the order of the priority walk (`App/Core/Priorities.h`), or, when the circuit has
  feedback, up to `kMaxSettleIterations` (10) passes, warning "feedback circuit did not converge" when the last still
  changes;
- after a commit that changed memory, settles the elements other than memory again, in one pass or up to 10.

What that engine does, which the spec changes:

- **Power-on.** Every output starts at its port's default, Unknown read as Inactive, so a gate-built latch starts from
  levels its inputs do not give it. Clocks start HIGH, shifted by their phase. The first update after a structural
  edit rebuilds and starts every clock HIGH again, so a flip-flop whose last clock was low sees an edge (plan 2 fixes
  the rebuild).
- **Non-levels.** An unconnected input reads its port's default, and an input that more than one wire reaches reads
  Error. Gates fold Unknown and Error with closed forms that rank Error above Unknown (`StatusOps`: the AND of Unknown
  and Error is Error), and an unconnected input whose default is Unknown makes every output Unknown. Every other kind,
  memory among them, makes every output Unknown when an input is not a level, by writing the published outputs
  directly (`ElementSimState::updateInputs()`): inside a flip-flop's staging window, elements later in the tick read
  that Unknown, and the commit then restores the staged value. So the engine contradicts itself: an SR flip-flop with S
  and R open gives Q Unknown when `updateLogic()` is called directly, and holds 0 in the running simulation. Plan 2
  makes an open S, R or T read Inactive, which removes that example; read on `master`, the same path runs for any input
  that is not a level.
- **Races.** Independent changes in one tick are applied in the engine's evaluation order, so that order decides a
  race between them, a gate-built latch released from both sides at once included, and a ring oscillator's passes
  never agree.
- **Wireless.** `connectWirelessElements()` points an Rx at its Tx's driver even when a wire also reaches the Rx, and
  `TestSimulation::testWirelessOverridesPhysicalWire` pins it. The editor refuses a new wire into an Rx node ("This
  element receives wirelessly — no input wire needed."), so such a net comes from elsewhere, a file for one.
- **IC boundaries.** Each instance builds its internal elements, and the loader gives each boundary pin, as default
  and as status, a copy of the status the inner input element's port had at load (`ICLoader`), a clock's pin required:
  an unconnected pin reads whatever that status was, and plan 1's scene goldens hold a connected clock pin's default as
  Error.
- **Time.** One mode: clocks on wall time and no propagation delays.

## 2. The rules, as they stand

The rules the branch arrived at after two corrections, stated here as plan 5 starts from them; it re-examines each
against its oracles (§4) before the spec states it. They hold in both modes: functional mode is the same rules with
every delay zero and the clocks on wall time.

### 2.1 Values and time

- Four values, one byte each: Inactive, Active, Unknown and Error. In their order Unknown is below the two levels and
  Error, which are unordered among themselves (the flat order): an input lowered to Unknown may lower an output to
  Unknown, never move it to another value.
- Time is simulated nanoseconds in temporal mode. In functional mode every delay is zero, and the clocks and the user's
  changes run on wall time. An instant is one time, settled in rounds.

### 2.2 Wiring

- One node per element that computes: logic, memory, sources and clocks. A junction, an IC pin and a wireless Tx or Rx
  alias the slot that drives them, inside loops too, and cost no time; ICs are flattened into their nodes.
- An input nothing drives reads its port's default, and a combinational node with such an input whose default is
  Unknown outputs Unknown. A loop of aliases with no node in it reads Unknown.
- A net with more than one driver reads Error and warns once: two wires into an input, a label on more than one Tx in
  a scope, or an Rx that a wire also reaches.
- Wireless labels resolve once per scope (the top level and each IC instance), after the IC boundaries are mapped: an
  Rx reads its scope's Tx's driver.
- A node with outputs has at most 11 inputs (a Mux's 8 data and 3 select lines) and a memory node at most 5, which
  bounds a node's readings at 2^11; plan 6's builder asserts both.

### 2.3 What each node computes

- Every kernel and every memory sample reads an Unknown or Error input both as Inactive and as Active, and outputs what
  every reading agrees on; where the readings disagree, Unknown if a reading is Unknown or an Unknown input decides it,
  else Error (IEEE 1164 ranks Unknown first). Mux, Demux, truth tables and memory samples read both ways explicitly;
  the gates keep closed forms (a dominating value, parity) that give the same result.
- So every kernel is monotone in the flat order, with one exception the edge rule makes: a flip-flop's clock kept at
  Unknown has no edge.
- A flip-flop stores on a rising edge, its clock having gone from Inactive to Active since it last sampled; a clock
  that kept its value since then, Unknown both times included, does not rise. ~Preset and ~Clear are active low and
  override; latches are level-sensitive. So a falling clock with Unknown data holds; an asserted ~Preset sets Q
  whatever the data; a JK flip-flop with J Unknown, K low and Q high stays high; and one cause taking a clock from
  Active to Unknown makes no edge, since no reading of it rises.

### 2.4 Power-on

- Sources at their levels, clocks HIGH shifted by their phase, memory at its power-on state (Q low, ~Q high), and
  combinational outputs Unknown until their inputs decide them. A reset source is Active through the power-on instant
  (§2.13).
- A node whose outputs change with no changing input, as at power-on, is the cause of that change.
- The power-on settle cannot oscillate: a monotone gate changes at most once from Unknown.

### 2.5 Rounds read by cause

- A change's cause is the node that made it, and changes with different causes happen in any order. An instant runs
  rounds: the publications due apply, each changed slot with its publisher as its cause; the combinational nodes that
  read a change run in topological order; the memory nodes whose inputs changed sample, once their round's logic has
  settled; the zero-delay loops settle; memory nodes and loops publish after the round, which starts the next; and the
  instant ends when a round publishes nothing.
- A node whose changing inputs follow several causes is read over every order of them. Where every order takes its
  output once from the old value to the new one, that is an event of its own; where every order keeps it, it does not
  change; otherwise (a third value, or the old value after the new) it may pulse. A node also may pulse if, in some set
  of the events, the levels of its pulsing inputs, or of a divergent input that did not change (§2.8), change its
  output.
- All memory nodes read before any publishes, so a shift register shifts by one per edge, and a ripple counter ripples
  within one instant, a stage per round.
- Nothing is canonicalised. The topological order only orders evaluation within a round, and nothing observable
  depends on it or on where an element sits on the canvas; a ring oscillator or a raced latch reads Unknown because its
  phase or its outcome is.

### 2.6 Memory

- A memory node stores what every order of its input changes in the round agrees on, Unknown where they disagree, an
  input that may pulse taking every value at every step, after the last event too; a clock that may pulse has not kept
  its value.
- So a flip-flop whose data changes in its edge's round stores Unknown where the old and the new data differ, and so
  do a latch closing as its data changes, a preset or clear released at an edge, and both released together.
- A fresh memory node, new at power-on or at a rebuild, samples once a round publishes nothing, its last control levels
  taken from its inputs: preset, clear and an enabled latch apply, and nothing fabricates an edge.

### 2.7 Zero-delay loops

- A zero-delay loop, a set of combinational nodes that read each other, settles by ternary iteration: Eichelberger's
  algorithm A (the changing inputs read Unknown, iterated to a fixed point) then B (their new values) after each event,
  an input that may pulse taking every value at every step, and B with the pulses' actual values once every event has
  happened.
- Every order of the events is followed: up to five events by enumerating the orders, past five by a search over
  subsets of the events that keeps the distinct states each reaches, within a work limit of 2^18 member evaluations,
  named apart from the round budget (its value is provisional, P5.24). Past that limit, the orders again with A alone
  between events, within a limit of its own; past that, or past four inputs that move at will (3^4 combinations), a
  fallback in which every changed input reads Unknown in A, and what A makes Unknown pulses. Each settle past the work
  limit is counted.
- A ternary loop cannot oscillate: a ring that cannot settle settles at Unknown.

### 2.8 Pulses and divergent Unknowns

- A loop, and a memory node without delay, follow each order's path of every output. An output that some order moves
  twice, or moves while the merge keeps its published Unknown, publishes as a pulse after the round; a delay swallows
  a pulse, as it does a gate's.
- A pulse moves at will at every step, after the last event too, and memory and loops take every value of a pulsing
  input.
- An Unknown merged within an instant from orders, branches or runs that disagree is divergent: it stands for one value
  per order until the instant ends. Loops set it (every new Unknown, since ternary iteration does not tell a race from
  an Unknown input), and so do memory nodes, gates (an Unknown output that some level of their divergent inputs makes a
  level) and the temporal tie merge (§2.10). Memory and loops take each of its values for a whole branch, and gates
  read a divergent input that did not change at each of its levels. An output that stays Unknown while its divergent
  mark turns on or off moved in some order, so it publishes as a pulse. A delayed publication lands in a later instant
  as a plain value.
- An event is a change of the four-state value, as in Verilog, where a net that stays `x` has no edge: a slot that
  stays Unknown while its inputs change wakes nothing, and from one instant to the next an Unknown is a value.

### 2.9 Trips and the budget

- When an instant's rounds repeat a state (memory clocking itself), the memory nodes that cycled store Unknown, and the
  user is warned once ("oscillation"). The state includes the pulses the round's publications made, and a memory node
  that published a pulse counts as cycled. A round that repeats an earlier state does not read its pulses again, and a
  pulse that changed its slot's value becomes that change, an event of its publisher.
- A budget of 1,000 rounds stops an instant, with a warning of its own.
- A trip never makes memory sample a cone that has not settled.
- No fixture, example or IC component reaches the budget or trips unless it is meant to, so a budget set too small
  shows in the suite.

### 2.10 Delays and ties

- A node with a delay evaluates at its inputs' new values and schedules its result at `t + d`, superseding any
  publication it has pending (inertial delay). A loop with a delay in it runs in simulated time and may oscillate,
  which is behaviour, not a trip.
- A delayed node whose publication falls due in an instant's first round, at the time one of its inputs changes from
  another cause, publishes the merge of its old value and the due one (Unknown, or a pulse where the old value was
  Unknown), and its readers run again; a tie that the first round's pass finds undoes the pass, which runs again from
  the round's start with the merge.
- A covering rule (P5.25): a memory node whose clock reaches Unknown through a divergent Unknown, one level in each
  order, may have seen an edge, so it stores Unknown rather than holding. The rule that a clock taken from a level to
  Unknown by one cause holds stays for an Unknown that is no level. The exact oracle with delays showed the rule is
  needed (§4.10).

### 2.11 Clocks

- Functional mode, on wall time: an edge once more than a half period has passed since the clock's reference, which
  then advances by exactly a half period, so drift does not skew the frequency; a tick runs every edge due since the
  last one, each at its time (§2.12).
- Temporal mode: a clock's edges are events at `phase + k · half period` of simulated time (k ≥ 1); the speed only maps
  simulated time to wall time, so it changes nothing a clock does.
- A locked clock stops. Unlocked, it shows the level its schedule gives, the missed edges dropped, not replayed: at the
  rebuild that unlocks it, the grid edges since its stored reference are counted, the reference advances by that many
  half periods, and the output flips if the count is odd. It then matches a never-locked clock at every later tick,
  with no state of its own while locked.
- A settle holds the temporal clocks for the simulated time it took, a locked one like the others: a lock changes what
  a clock shows, not where its edges fall.

### 2.12 Ticks and settles

- A functional tick runs everything due since the last tick in time order, each distinct time an instant of its own:
  each clock edge at its due time and each user change at the time it was made. A temporal tick runs the user's changes
  first, as an instant of their own, then drains a window of simulated time; a temporal start with no change due runs
  no instant of its own. Both check one wall-time deadline between instants: a tick cut by it stops time at the next
  instant due and the next tick goes on from there, so what remains runs later, in the same order, and nothing a tick
  shows is invented.
- A timed change waits for its time; an untimed one is seeded at the start of the tick that sees it, so the runner
  stamps each user change with the time it was made. A parameter or structural edit takes effect at the next tick's
  first instant. Only the changed sources are seeded (every input after a rebuild), a source seeded back to its
  published level drops the publication it had pending, and a tick with nothing due does nothing.
- A settle runs instants while one is due, each at the earliest time something is due, within a budget of instants;
  it seeds no clock and holds the temporal clocks (§2.11).

### 2.13 ICs and the reset source

- An unconnected IC input reads the level that the input element it stands for has inside the IC (a switch's, a
  button's or a rotary's); a clock pin, which is required, reads Unknown.
- A reset source is Active through the power-on instant in functional mode, released at time 1 (the second tick, or a
  settle that runs the power-on instant), and for its width in temporal mode (1,000 ns by default), its release an
  instant of its own. A reset that survives a rebuild keeps its state; one a rebuild adds reads Inactive at once. Plan
  6 lands its element type, and with it the reference's reading of it (P5.27).

### 2.14 Rebuilds, mode switches and snapshots

- A rebuild keeps the outputs and memory state of the nodes that survive it, and fabricates no edge. A node whose
  element keeps its id and output count keeps its published outputs whatever its inputs did, so an edit that changes no
  output changes none (an And that loses an input keeps its value). A memory node also keeps its stored state, last
  control levels and clock phase when its kind and port counts are unchanged, and otherwise starts fresh (Q low, its
  control levels from the published inputs). A new combinational node starts at its result against the published
  values, not at its power-on Unknown. A new element, or one morphed to another type, is a new node; loading a file is
  power-on.
- An IC's nodes survive a rebuild when the IC keeps the definitions it was built from, each node placed by its
  top-level element and its index path through the ICs nested in it.
- A switch between functional and temporal mode keeps every value, Unknown included, and seeds nothing.
- A snapshot holds the run's state with its publications in flight, so a sweep resumes a live run exactly.

### 2.15 What the model promises, and what it does not

The direct changes of one tick's causes happen together in the first round, in any order among themselves, and memory
nodes and loops respond after the round. So a cause that reaches a memory node or a loop through another memory node or
loop comes after the direct ones: the round barrier. The alternative, each cause settling fully before the next in some
order, would read Unknown wherever a change through memory could arrive first; it needs a different engine, assumes any
skew between inputs, and makes a ripple stage that reads a switch changing in the clock's tick read Unknown in every
BeWavedDolphin waveform. Several causes in one tick come from a waveform column, a clock edge with a switch, or a
script, all meaning "these change together"; two hand toggles never share a 1 ms tick.

So the hazard rule is scoped: independent changes that meet in one round read Unknown where they reach memory or a
loop; outside a zero-delay loop, a hazard that only some delays produce (one change reaching a gate by two paths) does
not pulse; inside one, ternary iteration reads it Unknown where it can latch.

The price is Unknown where every order agrees: on the branch, 306 of 4,540 memory outputs (6.7%) of a random acyclic
draw, against 136 (3.0%) under the rules before the pulse model's correction, and 267 values at 26 steps of the
shipped walks. 262 of those trace to one unit reading another's merge, which is the rounds' rule as written (87 gates
reading two or more disagreeing slots whose levels move together in every order, the slots they feed, and loop members
reading another unit's merge); five, at one step of the 8-bit stack pointer's walk, were classed with them without
being decoded (P5.17). Following A then B after each loop event had already removed 228 of 490 such values (102 of
them in the stack pointer's NOR latches, where all 362,880 orders of the step agree).

## 3. Why the rules are these

### 3.1 The first rules, measured

The first rules (delta cycles, every node starting from its ports' defaults) were measured on the traces of 104
circuits, on 2026-10-16:

- 21 of the 104 tripped, oscillated or stayed Unknown: every power-on trip, every temporal settle that failed, and two
  counters Unknown for good after an all-off power-on.
- A gate-built flip-flop raced at power-on: its closed latch's NORs both computed 1 from the ports' defaults, then both
  0. HDL simulators start signals at Unknown (VHDL and Verilog both), where NOR(0, X) stays X, and their oscillation is
  the answer to a simultaneous release from a defined state; a free-running ring never starts at Unknown either.
  Logisim and Digital break a latch's symmetry at random or by their settle order, and Digital adds a reset gate for a
  defined start, which `master` lacks (hence the reset source).
- Starting the combinational nodes at Unknown removed every power-on trip and oscillation without changing a
  component; six circuits then power on Unknown, and a free-running ring needs an enable.
- The trips after power-on were setup violations inside gate-built latches (a click and a clock edge in one tick), not
  races of one input; a tick that runs the user's changes before the clock edges has none (hence time-ordered ticks).
  The examples' temporal oscillation was the first JK's inner flip-flop: the functional glitch of a Not's start value
  became a loop under a 5 ns delay.
- A junction or an IC pin that cost a delta step let the drawing decide a race (hence aliases). A delta step also
  turned a static hazard into a pulse that a gate-built latch caught, where with the default delays in temporal mode
  the And never pulses, and zero-delay logic took the clock-to-output delay out of a gate-built flip-flop; rounds
  restore it.
- Inside a loop, ternary iteration gives the every-order answer and delta steps one order's, and a ternary loop cannot
  oscillate (102 circuits). Reading logic without feedback the ternary way too made counters race through, one change
  read as two; read by cause, the race-through went and every race between inputs stayed, and an event form equalled it
  in every slot of all 102 circuits. The rounds changed one shipped circuit (a gate-built register driving memory
  elements); memory storing what every order agrees on changed 17 under the walk, and none when one input changes at a
  time.
- Three kernels locked at Unknown under the new start, and Unknown met Error the wrong way round. Data changing at a
  flip-flop's edge got three answers from the core's parts; reading it both ways is the answer no implementation
  contradicts. A duplicate Tx label and an input reached by two wires were one fault under two rules; both read Error.
  A functional tick toggled each due clock at most once, so a clock above 500 Hz ran slow.
- Rules more pessimistic than these (ternary reading per instant; every element starting at Unknown) made counters and
  shift registers Unknown after their first edge.
- An earlier event-driven engine, on another branch, measured two more: at one time, evaluating and then publishing
  was not enough, since a flip-flop sampled a cone still rippling (an 8-bit program counter preset to 0xFF and clocked
  once latched 0x1C), so memory samples only after the combinational nodes have settled; and staging every node's
  result, not only memory's, was load-bearing: letting zero-delay gates publish in place, as `master` does, took its
  failures from 16 to 21.
- The review of `temporal-engine` measured the clocks: on wall time in temporal mode, a 1 kHz clock into a T
  flip-flop made 100 transitions at normal speed and none at 1 ns per tick, so the speed changed behaviour; and a mode
  switch reset the circuit, as did visiting another tab afterwards. It also found an oscillation trip that made
  flip-flops sample cones that had not settled, and publications at one instant applied one at a time, so canvas
  placement decided latch races in both modes.

Rejected: defining every power-on state instead (seven new asynchronous resets, every embedding circuit regenerated,
shipped circuits held in reset, and a user's gate-built latch still oscillating), and checking for trips only after
power-on, which fixes nothing. Every rule that yields more definite values breaks a symmetric latch's tie by order, by
ids or by chance; every rule that yields fewer reports Unknown where circuits work.

### 3.2 The pulse model, corrected

The rules of §2 are the second correction of the branch's pulse model. Before it, the core and the reference shared
three defects that the lockstep between them could not see: a pulse could rise only before any event, so a pulsing
clock missed edges that some order of the events gives; memory and loops read a pulsing input as Unknown at one step
instead of every value at every step; and an Unknown merged from orders that disagree was then read as one value, as
if every order had made it. Checks written apart from both found them:

- On 1,500 random acyclic circuits (3 or 4 switches, 3 to 6 gates, 1 or 2 memories), the every-order oracle found 158
  unsound outputs of 4,500; on a larger draw (4 or 5 switches, up to 10 gates with Xnor and two-output Demux, SR
  flip-flops), 200 of 4,540, and none after the correction. Each part of the fix was needed: without moving pulses
  after the last event, 68 unsound outputs came back; without reading pulses in every set of events, 80.
- The exact-rounds oracle found 94 unsound outputs in 1,500 random feedback circuits before the fix and none after
  (216 imprecise).
- Over the 102 shipped walks (12,144 steps, 2,419,104 slot values: the 73 IC components, 21 examples, 6 fixtures and 2
  perf circuits), the old rules contradicted every order of the step's changes at 6,522 values in 18 walks (17
  components and the `serialize` example). The corrected rules contradict none, proven at the 9,775 steps where every
  order runs; none was found at the 2,369 where 400 orders are sampled.
- The typical unsound case was a register's hold Mux (data, Q, and a select from the write enable through a Not): when
  data, clock and write enable change in one tick, the Mux's output pulses while the clock rises, and the old rules
  read that pulse at rest at the clock's event, storing a level where some order stores the other.
- The parts of the fix depend on each other: publishing a change as a pulse drops the cause an event keeps, so that
  part alone was worse than the old rules at three of four components (on the 8-bit CPU's program counter, where every
  order reads 24 values Unknown, it gave a level for each); it is sound only together with reading pulses in every set
  of events.
- A trace observes only outputs and embedded ICs, so two rule sets can differ in memory for steps before a trace shows
  it: in five components a storage flip-flop held a level that the outputs show only when a later read selects its
  register, up to eight steps later. Over 17 walks, up to each one's first changed trace step, 2,147 slot values
  differed, each a level under the old rules and Unknown under the corrected ones, never the other level. So rule
  changes are compared slot by slot, with every order checked over every slot, not by trace.
- Error values need the values' order. A sweep of the kernels found the old combiner non-monotone in stored state (a D
  latch with D 0 and its enable Error stored 0 from (0, U) and Error from (U, U)), and, in a chain order with Error
  below the levels, Mux and Demux shapes whose select passes their inputs non-monotone with an Error select; under
  §2.3's rule both are gone, and only the kept clock's pairs remain in the flat order.
- Turning the corrected rules from flags into code, in the core and the reference, found five more defects, each a test
  since: a loop settle with branched inputs followed its sets with no work limit up to five events, so a loop with
  several pulses could stall the instant (each branch settle is now bounded by states reached × value combinations ×
  members against the 2^18 limit, counted the same way in core and reference); a tie merge found in the first round's
  pass woke readers that had already run, which re-ran with the slot's old value set to their first result and could
  not take the change back (the pass is now undone and run again from the round's start); a tie merge's pulse was
  missing from the repeated-state key; a rebuild dropped the staged pulse marks of surviving nodes; and a re-staged node
  could keep a stale pulse mark.

What these checks did not reach is plan 5's to build (P5.D20): loop settles that start from or read an Unknown (no
exhaustive check over the levels an Unknown stands for: 38,338 one-event settles in the walks start from or read one,
against 42,993 checked exact, and 254 more were too big to check); temporal mode (no exact oracle with delays existed,
and the delayed every-order oracle's last three unsound circuits were explained on paper only); steps where more than
seven inputs change (only sampled); random circuits with ICs; the five Unknowns the corrected rules add at one step of
the 8-bit stack pointer's walk, classed with a known group rather than each traced to its cause; and loops larger or
more numerous than the shipped ones, timed at the work limit. The every-step sweep and the per-event loop check ran on
a build with each rule behind a flag, which equals the committed code on the shipped walks only; plan 5's oracles run
on the reference itself.

## 4. What plan 5 builds

### 4.1 The traces behind an engine

Plan 1's trace walks move into the test library behind an engine: start a wired circuit, have an input's new levels
take effect, run one step, stop, and read what an element drives, or what it receives when it drives nothing.
`master`'s tick engine is the first engine, its goldens unchanged by the move; the reference is the second (§4.11), and
plan 6 adds the core. The branch made this move in one commit that changed no golden.

### 4.2 The spec

The rules of §2, each with its reason, in a section of the developer guide of their own, written before the reference
(P5.D2). The section says that the reference simulator in `Tests/Common` implements them, that the application's
engine, which the guide's section on the simulation engine describes, does not yet, and that the causes file lists
every trace in which they differ. A rule changes there first, then in the oracles and the reference, and every trace
it moves is named by its cause. Each rule is re-examined against the oracles as they land; a rule an oracle refutes
changes, with the user's ruling where the change alters what a circuit shows.

### 4.3 The random circuits

One generator, driven by a source of choices (a seeded generator, or a fuzzer's bytes in plan 6), builds the circuits
the oracles and plan 6's lockstep run: switches, buttons, rotaries, VCC, GND, clocks (random frequency, phase and
lock), every logic and memory type at random port counts, truth tables with random bits, wireless nodes on three
labels, eight building-block ICs (one a nested counter, one with an internal wireless channel), LEDs, feedback, 8% of
inputs open and 4% doubly driven, and in temporal mode delays of 0 or 1 to 30 ns. A memory clock is its own output 15%
of the time and an input 40%, and about 2% of circuits carry a ripple chain of budget + 1 to budget + 4 toggle
flip-flops, so the round budget is reached.

A walk changes inputs and ticks (functional: long wall-time jumps, or short steps onto a 25 µs grid, so a slow clock
lands exactly on a toggle; temporal: windows of 1 to 5, 1 to 100 or 1 to 1,000 ns), settles with a budget of 0 to 7,
switches mode, or edits (adds or removes an element or a wire, changes a delay, a clock, a gate's input count or a
wireless label) and rebuilds. A failure names the step, the last ten operations and what differed. Seeds are FNV-1a of
a name, and two draws in one expression are sequenced explicitly (`CLAUDE.md`, "Building and testing"). The reset
source joins the draws with plan 6 (P5.27), and the sweeps inside a guard, which need the core's snapshots, are plan
6's.

On the branch a strict-clock mutant (an edge at exactly a half period) survived the walks until the steps onto the
25 µs grid were added.

### 4.4 The reference simulator

Test code in `Tests/Common` that implements the spec by definition, written to be read, not to be fast:

- It reads the elements (plan 3's one element class, its descriptors and typed parameters), ports and wires directly,
  in one place, flattening ICs from the definitions plan 4 shares and mapping their pins itself, so an error of plan
  6's netlist builder cannot be shared. It finds the zero-delay loops with Kosaraju's
  algorithm (plan 6's core uses Tarjan's) and has its own cycle finder for the rounds' repeated states.
- In every round it visits every unit, not only those whose inputs changed; a node has at most one pending
  publication, which a newer result supersedes (the inertial delay), so it needs no event queue, and no priorities
  order anything.
- It is a value type: a copy is a snapshot that resumes exactly, its publications in flight included.
- It keeps an IC instance's internal state keyed by the top-level element and the index path through the ICs nested in
  it, and keeps that state across a rebuild only when the IC keeps the definitions it had: a reload of a file-backed IC
  that brings the same definition keeps it, as plan 6's core will.
- Wireless per scope: an Rx reads its scope's Tx's driver; a label on several Txs, or an Rx a wire also reaches, reads
  Error, warned once per net.
- In temporal mode it takes each element's delay from its caller, since `master`'s elements carry none; plan 6 gives
  each logic type its delay, and plan 7 each element its own, which the reference then reads.
- `master`'s five-second resynchronisation of a lagging clock is not in the spec, and not in the reference.
- Its rounds' cycle search is quadratic in rounds, which the random circuits' ripple chains past the budget make
  costly.

Its hand-written tests, each derived from the spec by hand and each run on plan 6's core too:

- **Wiring:** an IC flattened behind its pins; an alias loop reads Unknown; two wires into an input read Error; an Rx
  reads its Tx's driver; a duplicated channel reads Error; the result does not depend on the order of the elements.
- **Power-on and kernels:** power-on fabricates no edge; an invalid input is read both ways; one cause taking a clock
  from a level to Unknown holds; two causes that each take a clock to Unknown keep it.
- **Latches and loops:** a latch released from both sides at once settles to Unknown; a cross-coupled latch sets and
  holds; a free-running ring settles to Unknown; a latch feeding itself trips through its samples; a flip-flop driven
  by its own outputs trips (through ~Preset from Q and ~Clear from ~Q: a clock alone cannot trip in zero delay, since
  a flip-flop's Q falls only at an edge its own change ends); a loop that every order settles to a level keeps it; one
  event reaching a loop member by two paths reads Unknown; a loop over the work limit, and one with too many branched
  inputs, takes its fallback, counted.
- **Memory and rounds:** a flip-flop samples the value settled at its edge; a shift register shifts by one per edge; a
  ripple counter ripples within one instant; a node evaluates once per order of its events; two switches released at
  once race into a NOR latch; one switch reaching an And by two paths makes no pulse; a gate-built register clocking
  an element samples its old output; the level-1 JK component follows its textbook over clock pulses; a gate-built
  counter counts on clean pulses; a gate-built register with a hold Mux holds on clean edges.
- **Pulses and divergence:** a pulsing clock may rise at any step; a data pulse meets an edge in its tick; an And
  passes a pulse once its other input rises; a pulsing clock rises while its data falls; a pulse that moves a gate makes
  it pulse; a loop's transient reaches a flip-flop; a latch passes a pulse on to a flip-flop; a latched pulse that
  comes back trips; a divergent Unknown is one level per order in later rounds.
- **Time:** a ring with delays oscillates in simulated time; a pulse shorter than the delay is swallowed; a newer
  result supersedes a pending one; a delayed data path misses the edge; a functional clock toggles after more than its
  half period; a temporal clock's edges fall on its phase grid; the speed does not change what a clock does; a
  functional change takes its time between edges; a temporal tick runs the user's changes before the edges due at its
  start; a temporal start with no change due runs no instant; only the changed sources are seeded; a delayed
  publication tied with an input change shows their merge; a tie the first pass makes runs the pass again.
- **Settles, locks and rebuilds:** a settle runs while an instant is due and holds the clocks, stops at its budget and
  samples a fresh memory node; a settle's hold moves a locked clock's grid like an unlocked one's; an unlock discards
  an odd and an even number of missed edges; a rebuild keeps the state of surviving nodes and fabricates no edge; a
  gate that loses an input keeps its output, in both modes (in temporal mode the `temporal-engine` review's shrunk gate
  fabricated an edge); an IC instance keeps its state across a rebuild, and across a reload that keeps its definition;
  a copy resumes with its publications in flight; a mode switch keeps every value.

Checked by mutation: on the branch an immediate-publication mutant made rings hang, so every test of a loop or an
oscillator has a timeout (`CLAUDE.md`, "Building and testing").

### 4.5 The kernels, checked three ways

What each node computes is checked apart from the rounds:

- **Against `master` on levels.** Every combinational kernel, and the IC flattening, on every mix of levels and of
  open inputs whose defaults are levels, exhaustively up to three inputs and sampled beyond, equals `master`'s engine,
  where the two rule sets agree. On the branch the same check ran under the first rules, which read non-levels as
  `master` does.
- **Against the rule on non-levels.** Every kernel on every input vector of the four values equals the merge of its
  readings at levels, computed by enumerating them, so the gates' closed forms are checked against §2.3's rule written
  out.
- **The sweep.** Every combinational and memory kernel over every input vector of the four values, with the clock's
  last level and the stored outputs as dimensions for memory, every covering pair checked in the flat order: monotone,
  but for the kept clock's pairs.

### 4.6 The exact rounds

An interpreter of the rounds written apart from the reference, the ground truth for the model of §2.15. From the
state before a tick, each toggled input is a cause of its own:

- in a round the causes' steps come in every order, and a step's changes reach the gates outside zero-delay loops at
  once, in topological order;
- a memory node samples at every step that changes one of its inputs, levels and values as they are, Unknown a value;
- a loop settles at every such step under every run of its members' delays, the general multiple-winner model: from
  each state, any nonempty set of the members whose outputs are not what their inputs give takes those outputs at
  once; each sink component of the runs' graph is an outcome (its one state, or the merge of its states where the runs
  cycle), and each outcome is a branch;
- after the round, each memory node and loop member that moved publishes every output it moved to, one step each, as
  a cause of its own;
- branches are kept once per state; a branch past 64 rounds (memory clocking itself) or past 2^17 states skips the
  circuit, and the skips are counted.

Each output's set of final values is compared with the reference: a single value must be matched or read Unknown;
several must read Unknown. With the reference it shares only the kernels, which §4.5 checks, and the flattening, which
plan 6's builder is checked against; it takes the circuit and the state as plain data, which plan 6's core gives too,
so it checks both. It runs on 1,500 random feedback circuits on every run: NOR and NAND latches, a Mux holding its own
output, memory feeding logic and, through inputs wired last, itself, a driven Unknown and an Error at times, and, new,
the building-block ICs of §4.3. On the branch, written against the core, it ran in 5.5 s for 1,500 circuits.

### 4.7 The every-order oracles and the detector

Random circuits run one tick with two or more of their switches toggled, then every order of those toggles, one per
tick; one toggle per tick is one cause, so nothing pulses and the rules are exact there. The orders' merge (the value
they all give, else Unknown) is the answer, and a one-tick result is sound where it equals it or is Unknown. Four
draws, 1,500 circuits each, ICs among their parts:

- acyclic: 3 to 5 switches, a driven Unknown in a third of them, 3 to 10 gates (Xnor and a two-output Demux among
  them), and 1 or 2 memories (D, T, JK and SR flip-flops, D latches) whose outputs feed nothing;
- feedback: NOR and NAND latches, a Mux holding its own output, memory feeding logic and, through inputs wired last,
  itself, a driven Unknown and an Error at times;
- temporal: the same parts without the late wiring, every gate and memory element at a delay of 0 or 16 to 80 ns,
  loop members at 0, the one tick making the toggles at one time and each order 1 ns apart;
- delayed: every memory element at 16 to 80 ns and every loop output read only through an Or of itself at 16 to 80
  ns, so no memory node or loop reads another's change in the instant it is made.

The orders let one toggle's response through memory or a loop come before another toggle, which the rounds do not
(the round barrier), so they check a stronger model and are ground truth only where no cause reaches memory through
other memory. A detector in the reference marks each slot change with the causes it descends from; a memory node or
loop that reads, within one instant, changes in more than one round with two or more causes among them is exposed, and
an unsound output counts only where neither it nor anything upstream of it was exposed. On the branch these oracles and
the detector were probes on the flagged build, never committed; plan 5 builds them as tests. After the correction
there, the acyclic draw had no unsound output; the feedback and temporal draws had none unexposed (179 and 106 exposed:
the barrier); and the delayed draw kept 9 unsound outputs in three circuits, explained on paper only as the barrier at
a delayed node's pending publication and as an Unknown that is a value from one instant to the next (§2.8). That count
was against every order of the toggles 1 ns apart; the exact oracle with delays, run on the branch afterwards, checks
simultaneous toggles directly and found 7 unsound outputs in two circuits (§4.10).

### 4.8 Memory over every order

Each of the six memory kinds (the D and SR latches; the D, T, JK and SR flip-flops, with their presets and clears),
wired to switches, is taken from every pattern of its inputs to every other in one instant, so several inputs change
together, the clock and the presets among them; it must store the merge of every order of those changes, each order's
state carried on.

### 4.9 The levels an Unknown stands for

A loop settle that starts from, or reads from outside the loop, an Unknown is checked by running the exhaustive loop
search of §4.6 at every assignment of levels to those Unknowns and merging the outcomes; the reference's settle must
cover that merge. On the branch, 38,338 one-event settles in the walks were of this kind and unchecked, and 254 more
exceeded the search's caps; the check runs on the walks (§4.12) and on the random circuits.

### 4.10 The exact oracle with delays

The exact rounds of §4.6 extended to temporal mode: each delayed node's publication falls due at its time, a newer
result superseding it in every order of the round's causes, and the ties at a time (a publication falling due as an
input changes from another cause) run in every order; each instant starts from four-state values, as §2.8 says. It
runs on the temporal and delayed draws of §4.7 and agrees with the reference before plan 7 offers temporal mode.

On the branch it ran as a probe (2026-10-21, on `gpu-canvas` at `58636e047`), sharing only the kernels with the core
and the reference: from the quiescent state after power-on, 2 to 5 switches toggle at one time; an output is sound when
the simulator reads Unknown or the one value every final state holds. On a release build both draws (1,500 circuits
each) ran with no cap reached, 41 instants at most:

| Draw | Outputs | Unsound (core and reference alike) | Circuits | Unknown where every final holds one level |
|---|---|---|---|---|
| temporal | 22,772 | 0 | 0 | 180 |
| delayed | 27,639 | 7 | 2 | 284 |

The core and the reference agreed on every unsound output, so lockstep could not see them. One circuit, traced by
hand: a NOR latch released from both sides at once settles at (1, 0) in one order and (0, 1) in the other; read only
through delayed buffers, it decides whether a mux takes a T flip-flop's clock from 1 to 0 and back to 1 (it toggles)
or leaves it at 0 (it holds). Both simulators read the latch as Unknown, the clock as going from a level to Unknown,
and hold, so Q is 0 where the flip-flop may be 0 or 1. The other circuit pairs a level with Unknown in its finals, the
same shape, not traced. This decided P5.25: the spec takes the covering rule of §2.10, and the test of P5.13 requires
no unsound output on both draws. The 180 and 284 outputs read Unknown where one level is certain: sound, and the
measure of the semantics' precision that P5.13 reports.

### 4.11 The spec's traces and their causes

- The reference runs plan 1's walks: every element run, and the 81 circuits from power-on through 120 steps, one tick
  per step at wall time 0, so a clock keeps the level its schedule gives at power-on. Its traces are pinned in goldens
  of their own, the spec's traces, which plan 6's core must match exactly.
- One causes file lists every trace in which they differ from `master`'s goldens, with its causes and nothing else:
  `power-on` (storage starts at its power-on state, where `master` starts it at its port's default, and a
  combinational output starts Unknown; plan 6 adds a reset source asserted at power-on) and `race` (a latch, flip-flop
  or gate-built storage whose inputs change in one round reads Unknown where the orders of those changes disagree,
  where `master` settles that instant in one evaluation order). Two rules are checked, one-sided: a difference at the
  first step lists power-on, and an Unknown after the first step lists race. A difference not listed, a listed trace
  that no longer differs, a malformed line and a unit named twice each fail. Deriving the causes automatically was
  rejected, since a labelling heuristic would become the test's answer.
- On the branch, against the core, 11 element runs and 37 circuits differed, each by power-on, race or both. The setup
  cause it first had (with zero setup time `master`'s elements sample the previous tick's data) folded into race once
  rounds read by cause. The modulo and up/down counters stay Unknown for good after an all-off power-on, having no
  reset input under any power-on rule; plan 6 gives them one.
- The 21 examples are walked too, without goldens: no fixture, example or component trips or reaches the round budget
  unless it is meant to, and no example takes a loop's work-limit fallback.
- Plan 2's fixes land before this plan starts, so `master`'s trace goldens hold them and the causes file names none of
  them. A later fix to `master` that changes one of its trace goldens after plan 5 has landed updates the causes file
  in the same commit, since the check fails otherwise.

### 4.12 Every step of every shipped walk

- Before each step of the 102 walks (the 81 traced circuits and the 21 examples), the walk copies the reference, runs
  every order of the step's input changes from the copy, one change per tick after the inputs it leaves as they were,
  and checks every slot of the one tick against their merge: a slot is contradicted where it holds a level, or Error,
  that not every order ends at.
- Up to seven changed inputs, every order is enumerated. Past seven (2,369 of the branch's 12,144 steps, where 400
  orders were sampled), a search over subsets of the changed inputs that keeps the distinct states each reaches gives
  every order's end state in 2^n subsets instead of n! orders; it is checked against the enumeration up to seven, and a
  step it cannot finish within a bound is listed by name rather than sampled silently.
- Each loop settle the walks make is checked against the exhaustive loop search of §4.6, and, where it starts from or
  reads an Unknown, by §4.9. On the branch all 42,993 checkable settles were exact.
- It runs when `WIREDPANDA_EVERY_ORDER_WALKS` is set, and in CI on a release build, never a debug one: on every push
  if it fits, else in a scheduled job (P5.26). On the branch, on the core, it took 518 s in one process, or 135 s on
  8 cores (533 s of CPU), in relwithdebinfo; the reference visits every unit in every round, so it is slower.

### 4.13 A decoded case

The five values the corrected rules add at step 87 of the 8-bit stack pointer's walk, each traced to its cause and
pinned as a test: on the branch they were classed with the units that read another's merge, not decoded.

### 4.14 Larger loops at the work limit

Circuits with zero-delay loops larger or more numerous than the shipped ones (whose settles have two to five events):
which tier answers each settle, checked by the exact search where it can run, with the fallbacks counted. Plan 6 times
them on its core and sets the work limit's value from that (P5.24). The branch's worst cases, on its core, 50
functional ticks each: a Mux with all 11 inputs on switches (2^11 sets of events) 0.57 to 0.76 ms per tick; a ring of
six NOR latches, one 18-member loop with 8 events, 4.0 to 4.3 ms; the ring with four pulsing set inputs 4.2 to 4.4 ms.

### 4.15 The differences from `master`'s engine

A list kept beside the causes file, each difference with its cause, for plan 6's switch to match and its release notes
to state:

- combinational outputs Unknown at power-on until their inputs decide them, and memory at its power-on state;
- non-levels read both ways instead of every output going Unknown, and Unknown ranked over Error;
- the tick engine's contradiction on invalid inputs, gone with the switch;
- races read Unknown: a simultaneous latch release, data changing at its clock's edge, a waveform column or an MCP
  batch changing a clock and its data together, a function hazard into a latch;
- a ring oscillator settles at Unknown where `master`'s passes never agree;
- no wire time: junctions, IC pins and wireless links are aliases;
- two drivers read Error, an Rx that a wire also reaches included, where `master` follows the Tx;
- an unconnected IC input reads its input element's level, and a connected clock pin's default is Unknown (ten scene
  goldens changed on the branch, each a clock pin's default from Error to Unknown);
- time-ordered ticks: a clock above 500 Hz keeps its rate, and there is no five-second resynchronisation;
- an unlocked clock shows the level its schedule gives, where `master` pays its owed edges one per tick, or after a
  lock of more than five seconds starts HIGH again.

Plan 2's fixes to the engine (rebuilds keep state and fabricate no edge; an open S, R or T reads Inactive; the priority
walk) are on `master` before this plan starts, so they are not in the list.

## 5. Steps and slices

| Row | Step | Slice |
|---|---|---|
| P5.1 | The trace walks in the test library behind an engine; plan 1's goldens unchanged (§4.1) | 1 |
| P5.2 | The spec, in the developer guide (§4.2) | 1 |
| P5.3 | The random circuits and their walks (§4.3) | 2 |
| P5.4 | The reference: reading the circuit, flattening, aliases, wireless, two drivers, IC boundaries, kernels (§4.4) | 2 |
| P5.5 | The kernels checked three ways (§4.5) | 2 |
| P5.6 | The reference: rounds by cause, memory, loops, pulses and divergent Unknowns, trips, the budget (§4.4) | 2 |
| P5.7 | The reference: clocks, ticks, delays and ties, settles, copies (§4.4) | 2 |
| P5.8 | The reference: rebuilds, an IC's state, the mode switch (§4.4) | 2 |
| P5.9 | The exact rounds, on random feedback circuits with ICs, on every run (§4.6) | 3 |
| P5.10 | The every-order oracles, four draws, and the detector (§4.7) | 3 |
| P5.11 | Memory over every order (§4.8) | 3 |
| P5.12 | The levels an Unknown stands for, at the loop settles that start from or read one (§4.9) | 3 |
| P5.13 | The exact oracle with delays (§4.10) | 4 |
| P5.14 | The spec's traces, pinned; the causes file; the examples walked (§4.11) | 4 |
| P5.15 | The list of differences from `master`'s engine (§4.15) | 4 |
| P5.16 | Every step of every shipped walk against every order, each loop settle checked, in CI (§4.12) | 5 |
| P5.17 | The five values at step 87 of the 8-bit stack pointer's walk, decoded (§4.13) | 5 |
| P5.18 | Larger and more numerous loops at the work limit (§4.14) | 5 |
| P5.19 | The developer guide's section on the oracles and how to run them; the tests' README | 5 |

The checks and open questions of §7 are rows too (P5.20 to P5.27), and the covering rule that P5.25 decided is P5.25a,
in slice 4 with the oracle that shows it. New test classes are registered in the four places `CLAUDE.md`'s "Building
and testing" lists.

## 6. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **P5.D1. One spec for both modes, and the simulation split by layer:** the semantics and their oracles (this plan),
  the core (plan 6), temporal mode in the application (plan 7). Functional mode is the spec with every delay zero and
  the clocks on wall time. The spec starts from the rules of `temporal-engine` with its review's corrections and the
  amendments since, each re-examined here, and the core is written once, data-oriented, with no object engine first.
  Basis: the user's rulings of 2026-10-04 (the corrected rules, written once) and 2026-10-21 (the split by layer, with
  the restart's write-up).
- **P5.D2. The spec is a section of the developer guide, written before the reference;** a rule changes there first,
  then in the oracles, the reference and the core. Basis: read (the developer guide is where `master` documents its
  engine and its four-state logic); the plans' rule that the semantics come before the code (`PLAN_rules.md`, §2).
- **P5.D3. A test-only reference simulator, itself checked against exact oracles written apart from it,** not a
  second engine in the product. Basis: measured on the branch (the reference shared three of the core's pulse-model
  defects, which the lockstep could not see; an exact-rounds oracle written apart found 94 unsound outputs in 1,500
  random feedback circuits, and checking every order of every step of the 102 shipped walks found 6,522 contradicted
  values in 18 of them); the user's ruling of 2026-10-04.
- **P5.D4. Rounds read by cause, as §2.5 to §2.8 state them:** where the orders of independent changes disagree the
  result is Unknown, a simultaneous latch release included; nothing depends on evaluation order or canvas placement;
  pulses move at will; memory and loops take every value of a pulsing input; a merged Unknown stays divergent until the
  instant ends; a temporal tie publishes its merge. Basis: measured on the branch (§3.1 on 104 circuits; §3.2 for the
  pulse model, with the price of §2.15); the user's rulings of 2026-10-16 and 2026-10-20.
- **P5.D5. Combinational nodes power on Unknown;** sources, clocks and memory as before. Basis: measured on the
  branch (21 of 104 circuits tripped, oscillated or stayed Unknown under the old start; Unknown removed every power-on
  trip and oscillation without changing a component); VHDL and Verilog start at Unknown; the user's ruling of
  2026-10-16.
- **P5.D6. Every kernel reads a non-level both ways:** Unknown where a reading is Unknown or an Unknown input decides
  the disagreement, else Error. Basis: tested on the branch (the sweep of §4.5 found the old combiner and two Mux and
  Demux shapes non-monotone; under this rule only the kept clock's pairs remain); the user's rulings of 2026-10-16 and
  2026-10-20.
- **P5.D7. Junctions, IC pins and wireless links alias the slot that drives them,** inside loops too. Basis: measured
  on the branch (a junction or an IC pin that cost a delta step let the drawing decide a race); the user's ruling of
  2026-10-16.
- **P5.D8. A net with more than one driver reads Error and warns:** a Tx label duplicated by a file or a paste, an Rx
  that a wire also reaches, two wires into an input. Basis: read (a duplicated label and an input reached by two wires
  were one fault under two rules; the review of `temporal-engine` found four places that resolved wireless channels and
  disagreed); the user's ruling of 2026-10-16. Plan 8 makes the exporters refuse such a net.
- **P5.D9. An instant trips only when its rounds repeat a state,** the memory that cycled storing Unknown, the state
  including the round's pulses; a budget of 1,000 rounds with its own warning. Basis: measured on the branch (without
  the pulses in the state, a transparent latch passing its own pulse back ran all 1,000 rounds and ended at 0 where
  every order latches 1; with them it tripped at round 3; under the old cap, flip-flops sampled a cone that had not
  settled); the user's ruling of 2026-10-16.
- **P5.D10. Memory stores what every order of its input changes agrees on,** Unknown where they disagree, an input
  that may pulse taking every value at every step. Basis: measured on the branch (data changing at a flip-flop's edge
  got three answers from the core's parts; every order changed 17 shipped circuits under the walk and none when one
  input changes at a time); the user's rulings of 2026-10-16 and 2026-10-20.
- **P5.D11. A zero-delay loop follows every order of its events, within a work limit** named apart from the round
  budget, with counted fallbacks past it. Basis: measured on the branch with temporary checks (the search over subsets
  equalled the permutations at six and seven events on every tick of 20 random loops, and agreed with the old subset
  merge on about 106,000 values; no shipped example reaches the limit, its loop settles having two to five events);
  the user's ruling of 2026-10-19. The limit's value is provisional (P5.24), and eight events and more are
  unchecked against permutations (P5.23).
- **P5.D12. Time-ordered ticks:** a functional tick runs user changes and clock edges in time order, each distinct
  time an instant; a temporal tick runs the user's changes first. Basis: measured on the branch (the trips after
  power-on were setup violations from a click and a clock edge in one tick; `master` toggles each due clock at most
  once per tick, so a clock above 500 Hz runs slow); the user's ruling of 2026-10-16.
- **P5.D13. In temporal mode clocks are event sources in simulated time;** the speed only maps simulated time to wall
  time. Basis: measured on `temporal-engine` (a 1 kHz clock into a T flip-flop: 100 transitions at normal speed, none
  at 1 ns per tick); the user's ruling of 2026-10-03/04.
- **P5.D14. A settle's hold moves a locked temporal clock's phase,** as it does every clock's. Basis: tested on the
  branch (a clock locked across a settle and unlocked after a rebuild matches a never-locked twin); the user's ruling
  of 2026-10-17.
- **P5.D15. An unlock drops the missed edges:** the clock shows the level its schedule gives. Replaying them was
  rejected (a 1 kHz clock locked for an hour owes 7.2 million), and so was a pause, which would shift only the locked
  clock and change temporal mode. Basis: tested on the branch against a never-locked twin, after an odd and an even
  count of missed edges; the user's ruling of 2026-10-17.
- **P5.D16. A mode switch keeps every value,** Unknown included, and seeds nothing. Basis: measured on
  `temporal-engine` (a mode switch reset the circuit, and so did visiting another tab afterwards); the user's ruling of
  2026-10-16.
- **P5.D17. An unconnected IC input reads the level of the input element it stands for;** a clock pin, required,
  reads Unknown. Basis: tested on the branch (the switch, button and rotary boundaries; ten scene goldens changed, a
  connected clock pin's default from Error to Unknown); read (copying the declared default instead would leave an
  unconnected switch pin Unknown, since switches declare none); the user's ruling of 2026-10-19.
- **P5.D18. A power-on reset source,** Active through the power-on instant in functional mode and for its width in
  temporal mode, not asserted again by a rebuild. Basis: measured on the branch (two shipped counters stayed Unknown
  for good after an all-off power-on) and read (Logisim and Digital break a latch's symmetry at random or by their
  settle order; Digital adds a reset gate); defining every power-on state instead was rejected (§3.1); the user's
  ruling of 2026-10-16. Plan 6 lands the element whole.
- **P5.D19. Rebuilds and power-on fabricate no edge:** survivors keep their outputs and state, a new combinational
  node takes its result, a fresh memory node samples without an edge. Basis: tested on the branch (the review's shrunk
  gate fabricated an edge in temporal mode; its test, turned round, passes on the branch's core and reference); among
  the amended rules the user approved on 2026-10-16.
- **P5.D20. Tests for what the evidence did not reach,** each built here: the exact oracle with delays, before
  temporal mode is offered; the search over the levels an Unknown stands for; every order of the walk steps where more
  than seven inputs change; random circuits with ICs; the decoded case; larger loops at the work limit; and the
  every-order walks in CI on a release build, in a scheduled job if they are too slow for every push. Basis: the pulse
  analysis's own list of what its evidence did not reach (§3.2); the user's ruling of 2026-10-20; the release build at
  the user's word of 2026-10-21 ("as long as it built on release mode it should be fine to run them in CI").
- **P5.D21. A clock reaching a divergent Unknown may have seen an edge,** so its memory node stores Unknown (§2.10).
  Basis: the exact oracle with delays, run as a probe on the branch (its findings in `b28bb61ad` on `gpu-canvas`),
  found the core and the reference holding where the flip-flop may be 0 or 1 (§4.10); the user's "yes" of 2026-10-21.

## 7. Claims to check and open questions

- **Algorithm B's termination with Error values** (P5.20) was argued on the branch, not proven. The proof the spec
  states: A starts from a state that the old inputs hold fixed and lowers the changing inputs to Unknown, so with
  monotone kernels each member moves at most once, down to Unknown; B raises them to their new values from A's fixed
  point, so each member moves at most once, up from Unknown. Its premise, monotone kernels in the flat order, is what
  the sweep of §4.5 checks; the reference asserts that no member moves twice within A or within B.
- **No subset mask wider than its type** (P5.21): `1 << count` above 31 is undefined, and on the branch nothing bounded
  the count but the cost. A settle whose events the work limit could never cover goes to the next tier before any mask
  is formed, asserted.
- **Each cap at its exact value** (P5.22): the round budget, the loop's work limit and its tier's own limit, the four
  inputs that move at will, and the exact oracle's 64 rounds and 2^17 states, each tested one below, at and one above.
  Random transients rarely meet a cap exactly, and plan 6's lockstep does not compare work counters.
- **The search over subsets against every order past seven events** (P5.23): checked at six and seven events only, on
  the branch, since the reference's permutations stop there; past seven, sampled orders.
- **The work limit's value** (P5.24): 2^18 member evaluations is provisional; no shipped settle reaches it. Plan 6 times
  the loops of §4.14 on its core; if the limit moves, it moves in the spec first.
- **A covering rule for a delayed node's pending level under a divergent change** (P5.25): decided, the rules need
  it; the exact oracle with delays found it missing on the branch (§4.10), and the spec takes the rule of §2.10.
- **The every-order walks' run time on the reference** (P5.26), measured on a release build, which decides whether
  they run on every push or in a scheduled job; if the reference is too slow even for that, a reference that visits
  only the units that read a change is measured against the current one, slot by slot, before it replaces it.
- **The reset source in the reference** (P5.27): its reading, its draws in the random circuits and its tests land with
  plan 6's element type; the spec states its rule now.

## 8. Risks

| Risk | Mitigation |
|---|---|
| The reference and plan 6's core share a misreading of the spec | The oracles, written apart from both and exhaustive where they can be; the traces with every difference named; the `temporal-engine` review's measured cases as tests |
| Gate-built latches change for users: a simultaneous release reads Unknown, gates power on Unknown | Certain; the two counters get a reset input and the examples a reset source or an enable (plan 6); the release notes |
| Unknown where independent changes race: a waveform column changing a clock and its data, an MCP batch, a function hazard into a latch | Certain; the release notes and the developer guide (plans 6 and 7) |
| Reading by cause costs more where independent changes meet | Bounded per node (2^11 readings) and per loop (the work limit); timed on plan 6's core |
| Algorithm B does not terminate with Error values, or a subset mask overflows | The proof and its assertion (P5.20); the tier before any mask (P5.21) |
| A cap is off by one | Each cap tested at its exact value (P5.22) |
| A zero-delay ring of more than about 500 memory nodes spends the round budget every instant | The budget bounds each instant; a deadline between instants bounds a tick in both modes (plan 6), which times the worst single instants |
| The reference is too slow for the every-order walks | The scheduled job; a faster reference only once it equals the current one slot by slot (P5.26) |
| A test of a loop or an oscillator hangs on a mutant | Every such test has a timeout |
| A later fix to `master` moves its traces after plan 5 lands | The causes check fails until the fix's commit updates the file (§4.11) |

## 9. What other plans take from plan 5

- **Plan 6:** the spec its core implements; the reference and the oracles its core is checked against (the lockstep
  on the random circuits, the exact rounds from the core's state, the kernel checks on its own kernels, the every-order
  walks); the spec's trace goldens, which the core must match, and the causes file and the list of differences, which
  its switch matches and its release notes take; the random circuits, for its lockstep and its fuzzer; the hand-written
  cases, run in lockstep; the reset source in the reference (P5.27); each logic type's delay for the reference to
  read; the timing that sets the work limit (P5.24); the two counters' reset input.
- **Plan 7:** the exact oracle with delays, which agrees with the reference before temporal mode is offered; the
  examples' lessons: a free-running ring needs an enable (its ring oscillator is saved with the enable off), and the
  first JK's inner flip-flop oscillated under a 5 ns delay from a Not's start value.
- **Plan 8:** the spec's start, which the SystemVerilog export follows (gates in loops unseeded), and the two drivers
  that read Error, which the exporters refuse; its differential compares with plan 6's core, which runs in lockstep
  with the reference.

## 10. The branch's work, as evidence

Each step was built on `gpu-canvas`, where its commits show the change and its tests; plan 5 writes its own on
`master`, as test code, with no core beside it. A hash is the branch's, kept read-only.

| Step | Commits on the branch |
|---|---|
| The reference simulator, first form | `0f02feb1e` |
| Invalid inputs read both ways | `3df5cd3d6` |
| The random circuits and the lockstep | `bd525357a`, `e1403720c`, `7bce09d33`, `0150f6a2e` |
| The traces behind an engine, a display's trace, the causes | `49da1eb9a`, `c0681583a`, `075d1fab3`, `10d345a9b`, `e497e80e5`, `81e6b174a` |
| Power-on Unknown and both-way reading | `4df8fb80a` |
| Aliases and two drivers | `b1995103f` |
| Rounds by cause | `47b421d94` (the reference), `a047d38e1`, `24972f49e`, `4c9811bc2` |
| Time-ordered ticks; a temporal start with no change due | `2985a993a`, `805b8b7d8`, `3dd7a4d15`, `0cdec44a0` |
| The reset source | `b663552d1` |
| Rebuilds keep outputs; an IC's state in the reference | `b5351b44a`, `799cdb274` |
| Locked clocks | `487fd6c60`, `1fbc8fbe6` |
| The IC boundary | `f3537ad80`, `712dcf1bf` |
| Loops in every order; memory in every order; no example at the fallback | `3f2d4cf3f`, `c484f14d6`, `c7a4a52bc` |
| The input limits | `cd9054327` |
| The pulse model's correction, with the exact rounds | `1b1f81e76` |
| The kernel sweep | `574d2db44` |
| Every step of every walk against every order | `0d4e6b9c3` |
| No walk trips or reaches the budget | `fbaabc14b`, `7da9e861b` |

The every-order oracles of §4.7, the detector, the per-event loop check and the decoded circuits were scratch probes
of the pulse analysis, never committed. The branch never built the exact oracle with delays, the search over the
levels an Unknown stands for, the subset search over a step's changed inputs, random circuits with ICs in the oracles,
or the larger loops.
