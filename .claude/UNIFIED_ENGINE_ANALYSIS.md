# Unified Simulation Engine — Deep Design

## Decision: Non-Blocking Assignment (Two-Phase Delta Cycles) in Temporal Mode

Non-blocking delta cycles applied to the **temporal event-driven engine only**. Functional mode and flip-flop logic remain unchanged.

---

## 1. The Core Problem (Recap)

```
Source S → NOT → D input of flip-flop
Source S → directly → CLK input of flip-flop
```

When S changes 0→1, both NOT and CLK-path schedule the flip-flop simultaneously (zero delay). Without non-blocking, the result depends on which event fires first within the delta cycle — the flip-flop may see old-D or new-D depending on evaluation order.

---

## 2. The Mechanism: Prepare / Defer / Commit

Three new methods on `LogicElement` implement non-blocking without changing any gate logic:

```cpp
void LogicElement::prepareForEvaluation()
{
    m_savedOutputs = m_outputValues;   // snapshot pre-evaluation state
    m_outputChanged = false;
}

void LogicElement::deferOutputs()
{
    m_pendingOutputs = m_outputValues;  // capture what updateLogic() computed
    m_outputValues = m_savedOutputs;    // restore pre-evaluation values for other elements
}

bool LogicElement::commitOutputs()
{
    m_outputChanged = false;
    for (int i = 0; i < m_outputValues.size(); ++i) {
        if (m_outputValues[i] != m_pendingOutputs[i]) {
            m_outputValues[i] = m_pendingOutputs[i];
            m_outputChanged = true;
        }
    }
    return m_outputChanged;
}
```

**Why `setOutputValue()` is unchanged**: Gate logic calls `setOutputValue()` which writes to `m_outputValues` as it always has. The engine wraps each evaluation in prepare/defer to capture the result in `m_pendingOutputs` and restore `m_outputValues` to its pre-evaluation snapshot. No gate code changes.

**Why `updateInputs()` is unchanged**: It reads `predecessor->outputValue(port)` which reads `m_outputValues`. Since we restore `m_outputValues` after each evaluation (deferOutputs), every element in the same delta cycle sees the pre-commit values. Order-independent by construction.

---

## 3. Why m_lastValue Is Kept

### The non-blocking symmetry problem

Applying non-blocking to functional mode's `updateWithIterativeSettling()` was attempted and failed. Gate-level feedback circuits (SR latches built from NAND gates) start from an **all-zero initial state** (every LogicElement output defaults to `Status::Inactive`). With non-blocking:

- Both cross-coupled NAND gates evaluate simultaneously, both reading each other's old output (0).
- Both compute NAND(input, 0) = 1. Both commit to 1.
- Next iteration: both compute NAND(input, 1). Both commit to 0.
- **Oscillation**: the symmetric evaluation can't break the tie.

With blocking (topological order), one gate evaluates first, breaking the symmetry and converging to a stable state in 1-2 iterations.

This affects IC-based tests (e.g., `TestLevel1JKFlipFlop`) where flip-flops are built from gate-level .panda files with internal feedback.

### The m_lastValue cascade problem

Removing `m_lastValue` from built-in flip-flops and using blocking iterative settling breaks multi-flip-flop state machines (e.g., `TestCPUControlUnit`). Without `m_lastValue`, each flip-flop captures current D (computed earlier in the same iteration). In a state machine with multiple flip-flops, flip-flop A's new Q propagates instantly to flip-flop B's D input within the same iteration, causing the state to advance by multiple steps per clock edge.

### Resolution

| Configuration | Gate-level ICs | Multi-FF state machines |
|--------------|:-:|:-:|
| Blocking + m_lastValue (current) | Pass | Pass |
| Blocking + no m_lastValue | Pass | **Fail** (cascade) |
| Non-blocking + no m_lastValue | **Fail** (oscillation) | Pass |
| **Non-blocking temporal only + m_lastValue kept** | **Pass** | **Pass** |

Keep `m_lastValue` in all flip-flops. Apply non-blocking only to `updateTemporal()`. Functional mode stays completely unchanged.

In temporal mode, non-blocking ensures that `m_lastValue` is always correct: predecessors commit before the flip-flop evaluates (in separate delta cycles), so `m_lastValue` holds the value from the previous evaluation — which is always the most recent D.

---

## 4. Implementation (as shipped)

### Files changed (3 files)

| File | Change |
|------|--------|
| `LogicElement.h` | Add `m_pendingOutputs`, `m_savedOutputs`, 3 new public methods |
| `LogicElement.cpp` | Constructor initializes new vectors; implement prepare/defer/commit |
| `Simulation.cpp` | `updateTemporal()` uses two-phase delta cycles with deduplication |

### Files unchanged

- All flip-flop logic (D, JK, T, SR) — `m_lastValue`/`m_lastJ`/`m_lastK`/`m_lastS`/`m_lastR` kept
- All latch logic (DLatch, SRLatch) — no m_lastValue to begin with
- All combinational gate logic — unchanged
- `updateFunctional()` — unchanged
- `updateWithIterativeSettling()` — unchanged (blocking)
- `SimEvent.h`, `EventQueue`, `ElementMapping` — unchanged
- All test files — pass as-is

### Delta cycle event loop (updateTemporal)

```cpp
while (anyChanged && events_at_same_time && deltaCycles < cap) {
    // ── Phase 1: Evaluate ──
    QVector<LogicElement *> evaluated;
    QSet<LogicElement *> seen;
    while (events at eventTime) {
        event = pop();
        if (seen.contains(event.target)) continue;  // deduplicate
        seen.insert(event.target);

        event.target->prepareForEvaluation();
        // clock toggle or updateLogic()
        event.target->deferOutputs();
        evaluated.append(event.target);
    }

    // ── Phase 2: Commit & schedule ──
    anyChanged = false;
    for (auto *elm : evaluated) {
        if (elm->commitOutputs()) {
            scheduleSuccessors(elm);
            anyChanged = true;
        }
    }
    ++deltaCycles;
}
```

### Event deduplication

Within a single delta cycle, an element may receive multiple events (e.g., two predecessors changed in the previous delta cycle). Without deduplication, a flip-flop evaluates twice:

1. First evaluation: detects CLK edge, updates m_lastClk = Active.
2. Second evaluation: CLK=Active, m_lastClk=Active → no edge. Different result.

The `m_lastClk` update is a side effect that persists across evaluations. Deduplication via `QSet<LogicElement *>` prevents this — each element evaluates at most once per delta cycle.

---

## 5. Trace-through: D + CLK simultaneous change (temporal mode)

Initial: S=0, NOT_out=1 (D=1), CLK_buf=0. FF: Q=0, m_lastClk=Inactive, m_lastValue=1.

**S changes to 1** → scheduleSuccessors(S) → NOT and CLK_buf scheduled at time T.

**Delta cycle 1 at T**:
- Evaluate NOT: reads S=1, computes 0. Defer: pending=0, restore=1.
- Evaluate CLK_buf: reads S=1, computes 1. Defer: pending=1, restore=0.
- FF not scheduled yet.
- **Commit**: NOT=0, CLK_buf=1. Schedule successors → FF scheduled at T.

**Delta cycle 2 at T**:
- Evaluate FF: reads D=0 (committed NOT), CLK=1 (committed CLK_buf).
- Edge: CLK=Active, m_lastClk=Inactive → EDGE.
- Captures m_lastValue=1 (D from before S changed, set during previous evaluation).
- Q=1. m_lastClk=Active. m_lastValue=0 (current D).
- Defer: pending=Q=1, restore=Q=0.
- **Commit**: Q=1.

**Result**: Q=1 (captures old D via m_lastValue). The non-blocking commit boundary ensures the flip-flop evaluates AFTER its predecessors commit, and m_lastValue correctly holds the pre-change D value.

---

## 6. Combined Plan: Event-Driven Engine + 4-State Logic

### 6.1 Why they must ship together

Implementation attempts revealed that 4-state logic and event-driven evaluation are **codependent**:

| Approach | Result | Root cause |
|----------|--------|-----------|
| 4-state + blocking settling | Invalid/Error stuck as fixed point | Blocking evaluates all gates in same pass; known values from inputs can't propagate level-by-level through feedback |
| Event-driven + Inactive (0) initial | Oscillation in feedback circuits | Symmetric NAND(1,0)=1 for both gates → NAND(1,1)=0 → repeat |
| **Event-driven + Invalid initial** | **Correct convergence** | Invalid doesn't flip-flop (it's a stable non-value); event-driven propagates known values one delta cycle at a time, resolving Invalid level by level |

Logisim avoids this coupling because their engine is already event-driven. For wiRedPanda, both changes must land simultaneously.

### 6.2 The 4 states

```cpp
enum class Status { Invalid = -1, Inactive = 0, Active = 1, Error = 2 };
```

| State | Logisim equiv. | Meaning | Visual |
|-------|---------------|---------|--------|
| `Invalid` | `UNKNOWN` | Undriven / floating / high-Z | Red (existing) |
| `Inactive` | `FALSE` | Logic low (0) | Gray (existing) |
| `Active` | `TRUE` | Logic high (1) | Green (existing) |
| `Error` | `ERROR` | Bus conflict | Orange (new) |

### 6.3 Domination rules (Logisim `Value.java`)

```
AND:  Inactive dominates   — AND(Inactive, X) = Inactive  for any X
      Otherwise            — AND(Active, Active) = Active
                             AND(Active, Invalid) = Invalid (soft propagation)
                             AND(Active, Error) = Error
                             AND(Invalid, Invalid) = Invalid

OR:   Active dominates     — OR(Active, X) = Active  for any X
      Otherwise            — OR(Inactive, Inactive) = Inactive
                             OR(Inactive, Invalid) = Invalid
                             OR(Inactive, Error) = Error

NOT:  Inverts known        — NOT(Active) = Inactive, NOT(Inactive) = Active
      Propagates unknown   — NOT(Invalid) = Invalid, NOT(Error) = Error

XOR:  Requires all known   — any Invalid → Invalid, any Error → Error
```

Invalid propagates "softly" — it means "not yet resolved" and resolves once a dominant value arrives through event-driven propagation. Error is reserved for actual bus conflicts.

### 6.4 Event-driven trace: SR latch from NANDs

S=0 (active set), R=1 (hold). All outputs start Invalid.

```
Input switches set S=Inactive, R=Active via updateOutputs().
scheduleSuccessors → NAND1 and NAND2 scheduled at time 0.

Delta cycle 1:
  Evaluate NAND1: AND(S=Inactive, Q'=Invalid) = Inactive (dominates). NOT = Active.
    Defer: pending=Active, restore=Invalid.
  Evaluate NAND2: AND(R=Active, Q=Invalid) = Invalid. NOT(Invalid) = Invalid.
    Defer: pending=Invalid, restore=Invalid.
  Commit: NAND1 → Active (changed). NAND2 → Invalid (no change).
  Schedule successors of NAND1 → NAND2.

Delta cycle 2:
  Evaluate NAND2: AND(R=Active, Q=Active) = Active. NOT = Inactive.
    Defer: pending=Inactive, restore=Invalid.
  Commit: NAND2 → Inactive (changed).
  Schedule successors of NAND2 → NAND1.

Delta cycle 3:
  Evaluate NAND1: AND(S=Inactive, Q'=Inactive) = Inactive. NOT = Active.
    No change. Converged.

Result: Q=Active, Q'=Inactive. Correct!
```

Hold mode (S=1, R=1) from Invalid: both NANDs compute `AND(Active, Invalid) = Invalid → NOT = Invalid`. No change, stays Invalid. **No oscillation.** The circuit correctly indicates "needs a SET or RESET pulse."

### 6.5 Unified event-driven engine (implemented)

The blocking event-driven engine (Logisim's Propagator approach) replaces both `updateFunctional()` and `updateWithIterativeSettling()`:

```cpp
void Simulation::processEvents()
{
    QVector<LogicElement *> queue;
    // Seed from m_pendingSchedule successors
    for (auto *source : m_pendingSchedule) {
        for (int outIdx = 0; outIdx < source->outputSize(); ++outIdx)
            for (const auto &succ : source->successors(outIdx))
                queue.append(succ.logic);
    }
    m_pendingSchedule.clear();

    const int maxEvents = 100000;
    int processed = 0;
    while (!queue.isEmpty() && processed < maxEvents) {
        auto *elm = queue.takeFirst();
        if (!elm) continue;
        elm->clearOutputChanged();
        elm->updateLogic();
        ++processed;
        if (elm->outputChanged()) {
            for (int outIdx = 0; outIdx < elm->outputSize(); ++outIdx)
                for (const auto &succ : elm->successors(outIdx))
                    queue.append(succ.logic);
        }
    }
}
```

Each element evaluates and **immediately** updates its outputs (blocking). If an output changed, its successors are appended to the FIFO queue. FIFO ordering naturally breaks symmetry in feedback circuits — whichever gate is scheduled first evaluates first and "wins."

### 6.6 Why blocking, not non-blocking

The initial implementation used **non-blocking delta cycles** (prepare/defer/commit, sections 2–4) because that's the textbook VHDL/Verilog approach. This was a mistake — non-blocking is designed for **timed simulation** where propagation delays naturally separate events across delta cycles. wiRedPanda's functional mode is **zero-delay**: all events are at the same timestamp.

With zero delay + non-blocking:
- Both NAND gates in an SR latch evaluate **simultaneously**, both reading each other's **old** (pre-commit) outputs
- Both see Invalid → both compute Invalid → both commit Invalid → nothing changes
- The latch is permanently stuck because non-blocking is **order-independent by design** — that's its whole point, and it's exactly the property that prevents symmetry breaking

With zero delay + blocking (Logisim's approach):
- Events are processed one at a time in FIFO order
- The first NAND gate evaluates, immediately writes its output
- When the second NAND gate evaluates, it sees the first gate's **new** output
- Symmetry broken — the latch resolves in 2–3 evaluations

**"Event-driven" does not mean "non-blocking."** They're independent concepts. Logisim is event-driven AND blocking — it processes a FIFO queue one element at a time with immediate output updates. That's the correct approach for zero-delay simulation.

Non-blocking remains correct for the future **temporal mode** (with propagation delays), where time separation between events naturally creates delta cycle boundaries. That work is preserved on the `tests-temporal` branch.

| Property | Non-blocking (VHDL/Verilog) | Blocking FIFO (Logisim) |
|----------|---------------------------|------------------------|
| Order-independent within delta cycle | Yes (by design) | No (FIFO order matters) |
| Breaks feedback symmetry | No — needs time separation | Yes — evaluation order breaks ties |
| Correct for zero-delay | **No** | **Yes** |
| Correct for timed simulation | **Yes** | Depends on implementation |

### 6.6.1 Why temporal mode cannot simulate functional via zero delay

An earlier design assumption was that temporal mode could subsume functional mode by setting all propagation delays to zero. This is **wrong** because the two modes require **opposite evaluation strategies**:

- **Functional (zero-delay)** needs **blocking** — FIFO order breaks feedback symmetry when all events share the same timestamp.
- **Temporal (with delays)** needs **non-blocking** — order-independence within a delta cycle is required for correct VHDL semantics when multiple signals update simultaneously.

Setting temporal delays to zero puts every event at the same timestamp within the same delta cycle. Non-blocking at zero delay can't break feedback symmetry — the exact problem that blocking solves. So "temporal with zero delay" would reproduce the stuck-Invalid bug, not emulate functional mode.

**Consequence**: functional and temporal modes must use **different evaluation strategies**, not a single unified code path. They can share event queue infrastructure (scheduling, successor tracking, convergence detection) but the inner evaluation loop must be:
- **Blocking FIFO** for functional mode (zero-delay, Logisim approach)
- **Non-blocking delta cycles** for temporal mode (with delays, VHDL approach)

This is not a limitation — it reflects a fundamental difference in what the two modes simulate. Zero-delay simulation models **logical correctness** (what the circuit computes). Timed simulation models **physical behavior** (when signals change). These require different invariants.

### 6.7 GND/VCC as shared graph elements (completed)

**Problem**: Each `ElementMapping` created its own `m_globalGND`/`m_globalVCC` as stack-allocated members. When `IC::generateMap()` created a nested `ElementMapping`, its GND/VCC were separate instances invisible to the top-level simulation. Unconnected IC ports (e.g., D flip-flop Preset tied to VCC) remained at Invalid because the top-level simulation never scheduled the nested GND/VCC.

**Fix**: Changed `m_globalGND`/`m_globalVCC` to `shared_ptr<LogicSource>`. The top-level mapping creates them and adds them to `m_logicElms`. Nested IC mappings receive the parent's GND/VCC via `IC::setGlobalSources()` and `ElementMapping`'s `gnd`/`vcc` constructor parameters. Only the top-level (owning) mapping adds them to `m_logicElms` — nested mappings just reference the same instances.

**How it works**:
1. Top-level `ElementMapping` creates GND/VCC as `make_shared<LogicSource>`, appends to `m_logicElms`
2. `ElementMapping::generateMap()` calls `IC::setGlobalSources(m_globalGND, m_globalVCC)` before `createLogicElements()`
3. `IC::generateMap()` passes them to its internal `ElementMapping(elements, gnd, vcc)`
4. Internal mapping uses `gnd ? move(gnd) : make_shared(...)` — receives parent's instance, doesn't create new ones
5. Ownership detection: `m_globalGND.use_count() == 1` means top-level (sole owner), else shared
6. At init, simulation seeds `m_pendingSchedule` with any element whose output differs from Invalid — this naturally includes the GND/VCC LogicSources

**Result**: One GND + one VCC for the entire simulation graph, regardless of IC nesting depth.

### 6.8 IC circuit design fixes (completed)

#### D flip-flop and JK flip-flop ICs (Level 1)

Both had three structural errors vs textbook designs:

1. **Master/slave clock polarity swapped** — master should be transparent when CLK matches the desired edge sensitivity
2. **JK feedback from master instead of slave** — slave outputs are stable during master-transparent phase
3. **Clear/preset only routed to slave** — master NOR gates had no async inputs, leaving master at Invalid

Both generators rewritten to match Mano "Digital Design":
- JK: negative-edge triggered. Master transparent CLK=1, slave CLK=0. Feedback from slave.
- D: positive-edge triggered. Master transparent CLK=0, slave CLK=1.
- Both: clear/preset injected into master NOR gates (3-input) AND slave (via OR gates).

#### Counter ICs (Level 4–5)

Binary counter, modulo counter, and up/down counter ICs had **no reset pin**. With 4-state logic, D flip-flop NOR latches cannot resolve from Invalid without an explicit reset signal. Real counters always have reset.

**Fix**: Added active-LOW `RST` input to all three counter IC generators. Connected to DFF `Clear` pins (Preset stays tied to VCC). Tests updated to pulse reset before counting.

#### Stack memory interface (Level 6)

Test was not pulsing the existing `SP_Reset` signal. Added reset pulse during initialization to set stack pointer to 0xFF.

#### Multi-cycle CPU clock gating race (Level 9)

**Problem**: The 2-bit cycle counter and the stage clock gates both triggered on the same rising clock edge. When the clock rose, the fetch stage gate saw the OLD counter value (00 = fetch), fired the fetch stage clock, AND the counter advanced to 01 simultaneously. This caused the PC to increment one cycle early.

**Root cause**: Clock gating depends on counter outputs, but the counter clocks on the same edge. Race condition — the gate sees stale counter values when the clock rises.

**Fix**: Inverted the clock for the cycle counter so it advances on the **falling edge** of the system clock. Counter outputs are then stable before the next rising edge fires the stage gates. Timing:
- Falling edge: counter advances (e.g., 00→01)
- Rising edge: gates see settled counter values (01 = decode, fetch gate disabled)

Updated test expectations: PC increments during each Fetch phase (counter=00). First fetch fires on clock 1, so PC=1 after 1 clock.

#### Arduino codegen (expected output files)

D flip-flop, JK flip-flop, and binary counter IC structures changed from the above fixes. Regenerated expected `.ino` files to match the new circuit structures. No codegen code changes needed — the codegen works on `GraphicElement*` from the scene, not `LogicElement*` from the simulation graph.

### 6.9 Final test results

**135/135 tests pass (100%).**

| Phase | Tests passing | Key fix |
|-------|:------------:|---------|
| Before 4-state + event-driven | 135/135 | Baseline (old engine) |
| After 4-state + event-driven (initial) | 100/135 (74%) | Engine works, IC designs broken |
| After Level 1 FF IC rewrites | 100/135 (74%) | JK/D flip-flop ICs fixed |
| After GND/VCC as graph elements | 129/135 (96%) | Nested IC port propagation fixed |
| After counter IC reset pins | 132/135 (98%) | Counter ICs have proper reset |
| After CPU clock gating fix | 135/135 (100%) | All tests pass |

### 6.10 Lessons learned

1. **4-state logic and event-driven simulation are codependent** — neither works alone for circuits with feedback.

2. **GND/VCC must be first-class graph elements** — not side-channel members. Every unconnected port in the graph needs a proper source element that participates in event scheduling.

3. **Nested ICs must share a single GND/VCC pair** — each IC creating its own pair means the top-level simulation can't schedule them. Pass down via constructor parameters.

4. **Real hardware needs reset** — NOR SR latches starting from Invalid in hold mode are a fixed point. Without an asymmetric input (set, reset, or clear), they cannot resolve. Every sequential IC must have a reset mechanism. This matches real hardware practice.

5. **Clock gating requires edge separation** — if a counter and its gated logic share the same clock edge, race conditions occur. The counter must advance on the opposite edge so its outputs are stable when the gates evaluate.

6. **Domination rules enable incremental resolution** — AND(Inactive, Invalid)=Inactive and OR(Active, Invalid)=Active allow known values to propagate through circuits with partially-resolved nodes. XOR has no dominant value (correct — XOR genuinely can't determine output from partial information).

7. **Blocking FIFO breaks feedback symmetry** — unlike non-blocking delta cycles (which are order-independent and can't break ties), blocking evaluation in FIFO order means whichever gate is scheduled first "wins." This is how Logisim's Propagator works.
