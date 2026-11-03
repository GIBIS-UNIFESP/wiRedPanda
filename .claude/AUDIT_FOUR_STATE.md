# Four-state audit

Scope: the four-state value (`Status`: Unknown, Inactive, Active, Error) and every place the app computes with it,
checked against plan 5 §2 (`.claude/PLAN_05_semantics.md`), mainly §2.2 (wiring, defaults), §2.3 (what each node
computes: read both ways, Unknown where an Unknown input decides, else Error), §2.4 (power-on) and §2.6 (memory).

Method: read the operators and kernels; probed the app's own engine (`master`, through `makeMasterTraceEngine`) on
every four-value vector for the gates, and on hand-built circuits for the rest; compared each with the reference
(`Tests/Common/ReferenceKernels`). The probes were throwaway and are not committed: each result below is reproducible
with the snippet described under it. Nothing was changed in the app.

Values: `Status::Unknown = -1`, `Inactive = 0`, `Active = 1`, `Error = 2` (`App/Core/Enums.h`).

## Findings, by severity

### F1. Error outranks Unknown in the combiners (high; rule conflict)

- Code: `App/Core/StatusOps.h` folds: `statusAnd` (26), `statusOr` (41), `statusXor` (68), `statusAndAll` (81),
  `statusOrAll` (100), `statusXorAll` (120). Each returns Error before Unknown when both are present (for example
  `statusAndAll`: `if (hasError) return Error;` before the Unknown check).
- Pinned by tests: `Tests/Unit/Logic/TestStatusOps.cpp` lines 44-49, 136-143, 172-175, 224, 398 ("Error > Unknown",
  "Error takes priority over Unknown"). The tests lock in the behaviour, so they must change with it.
- Spec: §2.3 says a disagreement is Unknown if an Unknown input decides it, else Error ("IEEE 1164 ranks Unknown
  first").
- Reproduced: an AND with one input wired from a gate whose output is Unknown and the other wired from a gate whose
  output is Error. `master` gives Error, the rule gives Unknown.
- Mismatch count: for the gates through `master` with the inputs driven by defaults, Error-only vectors have 0
  mismatches. The mismatches that include Unknown come from F2 (default Unknown) and are not this finding. The
  wired probe above is this finding alone.
- Real world: a bus conflict is not a known value. Verilog resolves conflicting drivers to X, which argues for Unknown
  ranking first, as the spec says.
- Decision needed: which ranking is the rule. The reference follows §2.3 (Unknown first).

### F2. A default-Unknown input decides nothing, and wires treat the same Unknown differently (high; spec conflict + inconsistency)

- Code: `App/Element/ElementSimState.cpp:78-87` (`shouldFail`): an unconnected input whose default is Unknown makes the
  element output Unknown for every output and skip the kernel, even when another input is a dominating 0 or 1.
- Spec: §2.2 says a combinational node with such an input "outputs Unknown". §2.3 says a dominating level decides the
  gate (FALSE AND X = FALSE). The two parts of the spec contradict each other for this case.
- The app is also inconsistent with itself. The same Unknown, arriving by a wire from a gate that outputs Unknown, beside
  a wired 0 gives 0 (the fold's dominance). Arriving as an unconnected default beside a wired 0 gives Unknown.
- Mismatch counts against the §2.3 rule (drivers by default, every vector, `master` vs rule): AND with 2, 3 and 4
  inputs: 2, 18 and 110; OR with 2, 3 and 4 inputs: 2, 18 and 110; NAND with 2 inputs: 2; NOR with 3 and 4 inputs: 18
  and 110; XOR and XNOR with 2, 3 and 4 inputs: 0. Every mismatch contains an Unknown input and none is Error-only.
  (NAND and NOR were probed only at the arities listed.)
- Decision needed: §2.2 literal (Unknown) or §2.3 (dominance). The reference follows §2.3.

### F3. Mux, demux and truth tables force their output to a select's or an input's status (high; spec violation)

- Code: `App/Simulation/KernelLogic.cpp`. `mux` (251-257): a select line Unknown or Error sets the output to that status,
  whatever the data, so the readings are not taken. `demux` (270-278): a select line Unknown or Error sets every output
  to it. `truthTable` (297-305): the first Unknown or Error input sets every output to that input's status.
- Spec: §2.3, "Mux, Demux, truth tables and memory samples read both ways explicitly".
- Swept (wired Unknown, from a Not with an unconnected Unknown default, and wired Error, from two drivers):
  - mux, 3 / 4 / 5 / 6 inputs: 11 of 56, 91 of 240, 344 of 992, 1,141 of 4,032 four-valued steps differ.
  - demux, 2 / 3 / 4 / 8 outputs: 3 of 12, 32 of 56, 40 of 56, 210 of 240.
  - truth table, 2 inputs with 1 / 2 outputs: 5 of 12, 7 of 12; 3 inputs with 1 / 2 outputs: 19 of 56, 29 of 56.
- Examples: mux, data 0 and 0, select Unknown: `master` `U`, the rule `0`. Mux, data 0 and 0, select Error: `master`
  `E`, the rule `0`. Demux, data 0, select Unknown: `master` `UUU` on three outputs, the rule `000`. Truth table, A
  Unknown, B Active: `master` `U`, the rule `1` (the row for B = 1 is 1 whichever A is).
- The first-found rule also picks the wrong status: a truth table with A Error and B Unknown gives `master` Error, and
  the rule Unknown, since B's readings decide it.
- The sweep shows no mux mismatch with a defined select (all 11 mux 3 mismatches have a Unknown or Error select). So an
  unselected wired Unknown does not reach the output. The default case is different: an unconnected input whose default
  is Unknown makes any combinational node output Unknown whatever its select (F2).
- Decision: the kernels should read both ways, as the gates do. The spec is explicit, so this is a code change.

### F4. Unknown preset and clear are ignored in the flip-flops (high; spec violation)

- Code: `App/Simulation/KernelLogic.cpp:79-84` (`applyOverrides`) acts only when the preset or clear is Inactive. An
  Unknown or Error preset or clear is treated as not asserted.
- Spec: §2.3: "~Preset and ~Clear are active low and override"; every kernel reads Unknown inputs both ways.
- Reproduced: a D flip-flop with Q at power-on (Inactive), no clock edge, and an Unknown preset. `master` leaves Q at
  Inactive. The rule gives Unknown: one reading sets Q, the other leaves it.
- Swept (memory sweep, below): every step with an Unknown or Error preset or clear mismatches, in each flip-flop kind
  (D flip-flop: 1,053 of 1,053 steps with the preset Unknown, and the same with it Error; T, JK and SR the same).
  Example: D flip-flop, data Unknown, preset asserted (Inactive), clear Active, Q 0. `master` keeps Q 0 (`01`); the rule
  gives Q 1 (`10`). §2.3 says "an asserted ~Preset sets Q whatever the data", so this is the case the spec names.
  The cause is F7: the kernel returns before the overrides are applied.

### F5. Combinational outputs start Inactive, not Unknown (medium; spec deviation, acknowledged in code)

- Code: `App/Element/ElementSimState.cpp:18` and `:52`: an output whose port default is Unknown starts Inactive. The
  comment says this is deliberate, so gate-level feedback loops can settle.
- Spec: §2.4: "combinational outputs Unknown until their inputs decide them".
- Real world: a circuit's undetermined outputs at power-up are not 0; treating them as 0 asserts a value nothing
  supports.
- Decision needed: keep the settle-friendly start and say so in the spec, or start Unknown as §2.4 says.

### F6. A failed input sets every output to Unknown, whatever the cause (low; not reproduced to a violation)

- Code: `ElementSimState.cpp:78-87`: when `shouldFail`, every output is set to Unknown, including when the failing value
  is Error (the `!allowUnknown` path). So an element with a failing Error input outputs Unknown, not Error.
- Probed: a D flip-flop with two drivers on its data and no clock edge gives Inactive, which agrees with the rule. So the
  Error-to-Unknown conversion did not show there. The case is still open for a flip-flop with a clock edge, and for the
  combiners under `!allowUnknown`.
- Swept: for a memory node, an Error input does not become Unknown. The memory kernels return early and keep their
  outputs (F7). The combinational path is not swept here.

### F7. Memory nodes keep their outputs whenever an input is Unknown or Error (high; spec violation)

- Code: `App/Simulation/KernelLogic.cpp`. `dFlipFlop` (90-95), `tFlipFlop` (115-118), `jkFlipFlop` (138-141) and
  `srFlipFlop` (170-173) return when `simUpdateInputs()` fails, after setting only the kept clock to Unknown. `dLatch`
  (199-201) and `srLatch` (217-219) return on the same test. `ElementSimState::updateInputs` (78-87) does write Unknown
  into the element's outputs when an input fails, but a sequential element publishes its outputs from the staging
  buffer taken before the tick (`commitDeferredOutputs`, `Simulation.cpp:185-193`). That buffer holds the old values,
  so the Unknown is overwritten and the outputs hold. The gates instead use `simUpdateInputsAllowUnknown()` and read
  both ways.
- Spec: §2.3, "every kernel and every memory sample reads an Unknown or Error input both as Inactive and as Active".
- Swept (memory sweep, after power-on, four-valued steps): D flip-flop 15,605 of 21,216 steps differ from the rule; T
  the same; JK 81,363 of 112,398; SR flip-flop the same as JK; D latch 393 of 624; SR latch 515 of 624.
- Examples: D latch, data Unknown, enable Active, Q 0: `master` `01` (held), the rule `UU`. SR latch, set Unknown,
  reset Inactive, Q 0 and ~Q 1: `master` `01`, the rule `UU`.
- Consequence: an enabled latch does not follow its Unknown data, a rising edge with Unknown data is dropped, and a
  flip-flop keeps its old value where §2.3 gives Unknown.
- Decision: the spec is explicit, so the code should read memory inputs both ways as the gates do. Tests that pin the
  freeze were not searched for.

### F8. A flip-flop whose clock is High at power-on loads a value no input gave (high; spec violation)

- Code: the power-on kernel history is set in `App/Element/GraphicElements/DFlipFlop.cpp:53`,
  `App/Element/GraphicElements/TFlipFlop.cpp:53`, `App/Element/GraphicElements/JKFlipFlop.cpp:56` and
  `App/Element/GraphicElements/SRFlipFlop.cpp:55`. D and T start with last clock Inactive and last data Active. JK
  starts with last clock Inactive and both last J and last K Active. SR starts with all three Inactive. With the clock
  High at the first evaluation, the edge test in `dFlipFlop` (101-104) fires against that history: D stores the stale
  Active, and T and JK toggle Q from 0 to 1.
- Spec: §2.6, "a fresh memory node ... samples once a round publishes nothing, its last control levels taken from its
  inputs ... and nothing fabricates an edge". §2.4 says clocks start HIGH, shifted by their phase, so a High clock at
  power-on is the ordinary case for a real circuit.
- Swept (power-on step only): D flip-flop 8 of 625 configurations, T 8 of 625, JK 30 of 3,125. SR flip-flop and both
  latches 0, which fits SR's Inactive history. All 46 give `master` Q 1 and the rule Q 0, whatever the data.
- Consequence: a flip-flop whose clock starts High can begin with Q 1 that no input supplied.
- Decision: the fix that §2.6 describes is to take the fresh node's last clock from its first level, so no edge is
  fabricated. This is a code change to the kernel history, which I have not made.

### F9. JK toggle from both outputs Active (spec silent)

- Code: `jkFlipFlop`, toggle branch (149-150), swaps Q and ~Q. The rule complements each output. They agree in every
  state where Q and ~Q differ, so the disagreement is only in a state where both are Active.
- That state arises when both ~Preset and ~Clear are asserted (§2.3's override gives Q 1 and ~Q 1), and §2.3 does not
  say what a clock does from it.
- Sample: JK with J and K Active, clock rising, stored (1, 1): `master` `11`; complement `00`.
- Count: 1 of 9,477 defined steps in the JK sweep.
- Decision: what a JK does from Q = ~Q = 1 when clocked with J = K = 1.

### F10. Out-of-range select under an Error select (spec silent)

- Code: `mux` (259-260) and `demux` (280-287) treat a select value that addresses no data input or output as Unknown
  (the comment at 239-242 says so). A select line that is Error is handled first and gives Error (`mux` 252-256).
- A select with a defined value, out of range, gives Unknown in both `master` and the rule: no mismatch in the sweep.
- With an Error select line and a reading that is out of range, `master` gives Error and the rule gives Unknown. The
  rule's Unknown follows §2.3's "Unknown if a reading is Unknown", which the spec does not extend to a reading that
  addresses nothing.
- Examples: mux with 4 inputs (two data, two select), data 0 and 0, select lines 0 and Error: `master` `E`, the rule
  `U`. The second select line read as 1 addresses input 2, which does not exist, so one reading is Unknown. Of the 91
  mux-4 mismatches, 87 are Error from `master` against Unknown from the rule. I did not split those 87 between this case
  and a wired Unknown data input that the select reaches.
- Decision: under an Error select that can reach an out-of-range value, should the output be Error or Unknown?

### Conventions taken from `master` (spec silent)

- SR flip-flop with S and R both Active at a rising edge: Q 1 and ~Q 1 (`master`). SR latch with both Active: Q 0 and
  ~Q 0 (`master`). The reference uses these, so the sweep does not check them against the spec. §2 needs a ruling.

## Memory sweep (this round)

Method, in `Tests/Unit/Simulation/TestReferenceKernels.cpp` as a throwaway probe (not committed; kept at
`/tmp/claude-1000/-home-torres-wiredpanda/e445d050-0180-4a3b-b949-b79465438de0/scratchpad/probe/`):

- Each input is one of five kinds: a switch that changes over time, an unconnected default Inactive, an unconnected
  default Active, an unconnected default Unknown, or two switches driving it (an Error). Every combination of kinds is
  swept: 5^4 for the D and T flip-flops, 5^5 for JK and SR, 5^2 for the latches.
- The Unknown kind is an unconnected default. For a memory node a wired Unknown takes the same path, since
  `updateInputs(false)` fails on any Unknown whatever its source.
- Each combination runs 40 steps, the first being the power-on step. Random switch levels come from a fixed seed.
  A switch on a clock or enable does not change at its own edge, and the other switches hold their level at that step,
  so no data changes in an edge's round. This keeps the §2.6 changed-data case out of the sweep; that case waits for
  the sampling ruling.
- The reference is fed master's previous output as the stored value at each step. At power-on it is fed Q 0 and ~Q 1,
  with the last clock equal to the clock, so no edge is fabricated.

| Kind | Steps after power-on | Defined-input mismatches | Power-on mismatches | Four-valued mismatches |
|---|---|---|---|---|
| D flip-flop | 24,375 (3,159 defined) | 0 | 8 of 625 | 15,605 of 21,216 |
| T flip-flop | 24,375 (3,159 defined) | 0 | 8 of 625 | 15,605 of 21,216 |
| JK flip-flop | 121,875 (9,477 defined) | 1 (F9) | 30 of 3,125 | 81,363 of 112,398 |
| SR flip-flop | 121,875 (9,477 defined) | 0 | 0 of 3,125 | 81,363 of 112,398 |
| D latch | 975 (351 defined) | 0 | 0 of 25 | 393 of 624 |
| SR latch | 975 (351 defined) | 0 | 0 of 25 | 515 of 624 |

The defined-input column is the control: where no input is Unknown or Error, master and the reference agree, apart from
the one F9 case. The four-valued column is where the rule and master part ways, and its causes are F4, F7 and F8.

## Routing and truth-table sweep (this round)

Method, as the memory sweep, with a throwaway probe (kept at the scratchpad path above). Each input is a switch, a
wired Unknown (a Not with an unconnected Unknown default drives it), or a wired Error (two drivers). Every combination of
kinds is swept, with every level for the switches: 4^n steps for n inputs. The truth tables take random tables, fixed
by a seed. A mux's select lines are its last inputs; a demux's are the lines after its data input.

| Kind | Steps (defined) | Defined-input mismatches | Four-valued mismatches |
|---|---|---|---|
| mux, 3 inputs | 64 (8) | 0 | 11 of 56 |
| mux, 4 inputs | 256 (16) | 0 | 91 of 240 |
| mux, 5 inputs | 1,024 (32) | 0 | 344 of 992 |
| mux, 6 inputs | 4,096 (64) | 0 | 1,141 of 4,032 |
| demux, 2 outputs | 16 (4) | 0 | 3 of 12 |
| demux, 3 outputs | 64 (8) | 0 | 32 of 56 |
| demux, 4 outputs | 64 (8) | 0 | 40 of 56 |
| demux, 8 outputs | 256 (16) | 0 | 210 of 240 |
| truth table, 2 in / 1 out | 16 (4) | 0 | 5 of 12 |
| truth table, 2 in / 2 out | 16 (4) | 0 | 7 of 12 |
| truth table, 3 in / 1 out | 64 (8) | 0 | 19 of 56 |
| truth table, 3 in / 2 out | 64 (8) | 0 | 29 of 56 |

The defined-input column is the control: where no input is Unknown or Error, `master` and the reference agree. The
four-valued column is F3 and F10. The earlier run used an unconnected Unknown default for its Unknown input, which
makes every output Unknown through F2 and inflated the counts (mux 6: 908 of 4,032 that way); the table above uses a
wired Unknown.

## Checked and agreeing

- Dominance and levels in every fold and `statusNot`: correct for Inactive and Active inputs, and for Inactive or Active
  dominating Unknown and Error in AND and OR.
- Two drivers on one input: `master` gives Error (probed), matching §2.2. The multi-driver branch is at
  `ElementSimState.cpp:69`.
- Error-only vectors through the gates: 0 mismatches against the rule.
- Flip-flop edge rule and the kept clock (`dFlipFlop`, lines 87-110): matches the reference's stable-data cases.
- Memory with defined inputs (the sweep above): D, T and SR flip-flops and both latches agree with the reference on
  every step after power-on, with presets and clears defined. JK agrees except for F9.
- Routing and truth tables with defined inputs (the sweep above): mux, demux and truth tables agree with the reference in
  every case.
- Exporters: `DolphinExporter.cpp:88-89` writes Unknown as `x` and Error as `E`, one character each, as the comment
  requires. `WaveformSimulator.cpp:52-53` restores four-state values rather than a `bool`, which would conflate them.
- Presentation: `PicturePresenters.cpp:337-340` and `Connection.cpp:232-235` colour all four values.

## Not covered

- `Element::isValid` (`Element.cpp:850-853`) sends Error down the outputs of an element with an invalid input. §2 does
  not say what an invalid element outputs, so this is a gap in the spec rather than a defect in the code.
- Memory kernels: T, JK and SR flip-flops and the SR latch are swept now (above). The sweep holds data fixed at each edge
  and does not cover the §2.6 changed-data case, which waits for the sampling ruling. A whole-circuit comparison is not
  run: `ReferenceNetlist` has no reference kernels for those kinds yet (plan 5, P5.5).
- Whole-circuit sweeps: not run. The sweeps here are single elements, exhaustive over kinds and levels, with random
  sequences for memory. `ReferenceNetlist` has no reference kernels for T, JK, SR, mux, demux or truth tables yet (plan
  5, P5.5), so a circuit-level comparison has to wait for that.
- Node, Tx and Rx, ICs, clocks and probes are not swept for four-state behaviour.
- ASan and UBSan were not run, per the project's rules.

## Decisions for the user, in order of effect

Code changes the spec already settles (no ruling needed; each moves code and, where pinned, tests):

1. F7 and F4: memory kernels read Unknown and Error inputs both ways, and their presets and clears are read too. The
   outputs must not hold through an Unknown or Error input.
2. F8: a fresh flip-flop takes its last clock from its first level, so no edge is fabricated at power-on.
3. F3: mux, demux and truth tables read both ways, as the gates do.

Rulings (the spec is silent or contradicts itself):

4. F1: the combiners rank Error over Unknown; the spec ranks Unknown first.
5. F2: a default-Unknown input: outputs Unknown (§2.2) or is decided by a dominating level (§2.3).
6. F9 and the SR conventions: a JK clocked with J = K = 1 from Q = ~Q = 1 (complement or swap), and an SR flip-flop or
   latch with S and R both Active (master's (1, 1) and (0, 0)).
7. F10: an Error select that can reach an out-of-range value: Error (master) or Unknown (the rule).
8. F5: combinational outputs start Unknown (§2.4) or Inactive (the code).
9. The two sampling readings (ledger, PENDING).

Until these are decided, the reference follows the spec (§2.3 Unknown first; dominance), and the master divergences
above stay recorded as divergences, not as defects to match.
