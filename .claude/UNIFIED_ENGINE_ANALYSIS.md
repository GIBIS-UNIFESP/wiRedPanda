# Unified Simulation Engine — Deep Design

## Decision: Non-Blocking Assignment (Two-Phase Delta Cycles)

This is how VHDL/Verilog simulators achieve order-independent evaluation. Within each delta cycle, elements evaluate and write outputs to a **pending buffer**. After all events fire, pending values are **committed** simultaneously. Changed outputs schedule successors in the **next** delta cycle.

---

## 1. The Core Problem (Recap)

```
Source S → NOT → D input of flip-flop
Source S → directly → CLK input of flip-flop
```

When S changes 0→1, both NOT and CLK-path schedule the flip-flop simultaneously (zero delay). Without `m_lastValue`, the result depends on which event fires first within the delta cycle — the flip-flop may see old-D or new-D depending on evaluation order.

The `m_lastValue` pattern fixed this for the tick-based functional engine (where every element evaluates every tick), but breaks in event-driven mode where elements only evaluate on events.

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

### Delta cycle event loop

```cpp
while (!m_eventQueue.empty() && m_eventQueue.nextTime() == eventTime) {
    // ── Phase 1: Evaluate ──
    QSet<LogicElement *> evaluated;
    while (!m_eventQueue.empty() && m_eventQueue.nextTime() == eventTime) {
        auto event = m_eventQueue.pop();
        if (!event.target || evaluated.contains(event.target)) {
            continue;   // deduplicate: evaluate each element at most once per delta cycle
        }

        event.target->prepareForEvaluation();

        if (isClockSource(event.target)) {
            // Clock: toggle directly (LogicSource::updateLogic would reset to default)
            const auto cur = event.target->outputValue();
            event.target->setOutputValue(cur == Status::Active ? Status::Inactive : Status::Active);
        } else {
            event.target->updateLogic();
        }

        event.target->deferOutputs();
        evaluated.insert(event.target);
    }

    // ── Phase 2: Commit & schedule ──
    bool anyChanged = false;
    for (auto *elm : evaluated) {
        if (elm->commitOutputs()) {
            scheduleSuccessors(elm);
            anyChanged = true;
        }
    }

    if (!anyChanged) break;   // settled
    ++deltaCycles;
}
```

### Trace-through: D + CLK simultaneous change

Initial: S=0, NOT_out=1 (D=1), CLK_buf=0. FF: Q=0, m_lastClk=Inactive.

**S changes to 1** → scheduleSuccessors(S) → NOT and CLK_buf scheduled at time T.

**Delta cycle 1 at T**:
- Evaluate NOT: reads S.m_outputValues=1, computes NOT(1)=0, writes m_outputValues=0. Defer: pending=0, restore m_outputValues=1 (old).
- Evaluate CLK_buf: reads S=1, outputs 1, writes m_outputValues=1. Defer: pending=1, restore m_outputValues=0 (old).
- FF has no event yet.
- **Commit**: NOT.m_outputValues=0 (changed). CLK_buf.m_outputValues=1 (changed). Schedule both successors → FF scheduled at T.

**Delta cycle 2 at T**:
- Evaluate FF: `updateInputs()` reads NOT.m_outputValues=0 (D=0, committed), CLK_buf.m_outputValues=1 (CLK=Active, committed).
- Edge detected: CLK=Active, m_lastClk=Inactive → EDGE.
- Captures current D = 0. Sets Q=0. Defer: pending=Q=0, restore Q=old.
- m_lastClk = Active.
- **Commit**: FF.m_outputValues = Q=0. (No change from initial Q=0, so no further events.)

**Result**: Q=0 (captures current D at clock edge). This matches VHDL semantics — see section 3.

---

## 3. The Semantic Change: VHDL vs m_lastValue

| Aspect | Current (m_lastValue) | Non-blocking (VHDL) |
|--------|----------------------|---------------------|
| What the flip-flop captures | D from the **previous** tick | D at the **clock edge** |
| Setup-time model | Implicit 1-tick setup | Zero setup (signal-level) |
| Order-dependent? | No (but only works with every-tick evaluation) | No (by construction) |
| Industry standard | No (wiRedPanda-specific) | Yes (VHDL/Verilog) |

**Why this is correct**: In a zero-delay model, there are no propagation delays and therefore no meaningful "setup time." The `m_lastValue` pattern was an artifact of the tick-based engine forcing a 1-tick artificial delay. VHDL resolves this with the delta cycle commit boundary — if D changes in the same delta cycle as CLK, they're committed together, and the flip-flop evaluates in the next delta cycle seeing both new values. If D changes in a *different* delta cycle than CLK, the flip-flop correctly sees whichever D was committed at the time of the clock edge.

**Why existing tests still pass**: Every flip-flop test follows this pattern:

```cpp
swD.setOn(true);                      // Set D
sim->update();                         // Propagate D (no clock edge)
TestUtils::clockCycle(sim, &swClk);    // Rising edge in a SEPARATE update()
```

D is always set in a **preceding** `update()` call before the clock edge. When CLK rises, D is already stable at its new value. Both models capture the same D:
- m_lastValue: captured D=1 from the previous update → Q=1
- Non-blocking: reads current D=1 at clock edge → Q=1

The only case where results differ is D and CLK changing in the **same** `update()` from the **same** source (e.g., S→NOT→D and S→CLK). No existing test does this because it's a degenerate circuit.

---

## 4. LogicElement Changes

### New members

```cpp
private:
    QVector<Status> m_pendingOutputs;   // deferred output buffer
    QVector<Status> m_savedOutputs;     // snapshot for restore during defer
```

### Constructor change

```cpp
LogicElement::LogicElement(const int inputSize, const int outputSize)
    : m_inputPairs(inputSize, {})
    , m_inputValues(inputSize, Status::Inactive)
    , m_outputValues(outputSize, Status::Inactive)
    , m_pendingOutputs(outputSize, Status::Inactive)   // NEW
    , m_savedOutputs(outputSize, Status::Inactive)      // NEW
{
}
```

### New public methods

```cpp
void prepareForEvaluation();   // save + clear
void deferOutputs();           // pending = computed, restore saved
bool commitOutputs();          // apply pending → live, return changed
```

### Unchanged

- `setOutputValue()` — writes to `m_outputValues` as before
- `updateInputs()` — reads `predecessor->outputValue()` as before
- `outputValue()` — reads `m_outputValues` as before
- `clearOutputChanged()` — still used (inside prepareForEvaluation)
- `outputChanged()` — still used (after commitOutputs)

---

## 5. Flip-Flop Changes

### What's removed

| Element | Removed members |
|---------|----------------|
| LogicDFlipFlop | `m_lastValue` |
| LogicJKFlipFlop | `m_lastJ`, `m_lastK` |
| LogicTFlipFlop | `m_lastValue` |
| LogicSRFlipFlop | `m_lastS`, `m_lastR` |

### What's kept

All flip-flops keep `m_lastClk` — edge detection requires comparing current CLK with previous CLK. This is equivalent to VHDL's `rising_edge(CLK)` which internally stores the previous clock value.

### New updateLogic() — D Flip-Flop example

```cpp
void LogicDFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    Status q0 = outputs().at(0);
    Status q1 = outputs().at(1);
    const Status D = inputs().at(0);      // read CURRENT D (not m_lastValue)
    const Status clk = inputs().at(1);
    const Status prst = inputs().at(2);
    const Status clr = inputs().at(3);

    if (clk == Status::Active && m_lastClk != Status::Active) {
        q0 = D;                            // capture current D
        q1 = (D == Status::Active) ? Status::Inactive : Status::Active;
    }

    if (prst != Status::Active || clr != Status::Active) {
        q0 = (prst != Status::Active) ? Status::Active : Status::Inactive;
        q1 = (clr != Status::Active) ? Status::Active : Status::Inactive;
    }

    m_lastClk = clk;
    // NO m_lastValue update

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
```

### JK Flip-Flop

```cpp
if (clk == Status::Active && m_lastClk != Status::Active) {
    const Status j = inputs().at(0);   // read current J (not m_lastJ)
    const Status k = inputs().at(2);   // read current K (not m_lastK)

    if (j == Status::Active && k == Status::Active) {
        std::swap(q0, q1);
    } else if (j == Status::Active) {
        q0 = Status::Active; q1 = Status::Inactive;
    } else if (k == Status::Active) {
        q0 = Status::Inactive; q1 = Status::Active;
    }
}
m_lastClk = clk;
```

T and SR flip-flops follow the same pattern: read current inputs at clock edge, remove m_last* members.

---

## 6. Event Deduplication

Within a single delta cycle, an element may receive multiple events (e.g., two predecessors changed in the previous delta cycle). Without deduplication, the flip-flop evaluates twice:

1. First evaluation: detects CLK edge, updates m_lastClk = Active.
2. Second evaluation: CLK=Active, m_lastClk=Active → no edge. Different result.

The `m_lastClk` update is a **side effect** that persists across evaluations. Deduplication via `QSet<LogicElement *> evaluated` prevents this — each element evaluates at most once per delta cycle.

---

## 7. Edge Cases

### 7.1 Clock heuristic in temporal mode

The current clock detection heuristic (`delay==0 && outputSize==1 && inputPairs.empty()`) still works. Clock toggle events go through prepareForEvaluation/deferOutputs like any other element:

```cpp
target->prepareForEvaluation();
const auto cur = target->outputValue();  // reads m_outputValues (= saved, same thing)
target->setOutputValue(cur == Active ? Inactive : Active);
target->deferOutputs();   // pending = toggled, live = restored
```

On commit, the clock output updates and schedules successors (flip-flops) in the next delta cycle. The flip-flop then sees the committed clock value. Correct.

### 7.2 Initialization

Flip-flop constructors call `setOutputValue()` to set power-on state (Q=0, Q'=1). These write to `m_outputValues`. Since `m_pendingOutputs` is initialized to the same default (`Status::Inactive`), we need constructors to also initialize pending:

**Option A**: Initialize `m_pendingOutputs` in the base constructor to match `m_outputValues`:
```cpp
LogicElement::LogicElement(int inputSize, int outputSize)
    : m_outputValues(outputSize, Status::Inactive)
    , m_pendingOutputs(outputSize, Status::Inactive)  // matches
```
Then derived constructors that call `setOutputValue(1, Status::Active)` must also sync pending. Cleanest: add a `syncPending()` call in the derived constructors, or just accept that pending starts at Inactive and only matters during event processing (first commitOutputs will sync correctly since the engine evaluates elements before committing).

**Option B** (simpler): Don't worry about it. The first `prepareForEvaluation()` call saves m_outputValues (which includes the constructor-set values). After `deferOutputs()`, m_pendingOutputs gets the computed result. The initial pending value is never read before being overwritten.

**Verdict**: Option B — no initialization changes needed.

### 7.3 Feedback circuits (SR latches)

With non-blocking, cross-coupled NOR gates in an SR latch evaluate simultaneously within the same delta cycle, both seeing each other's pre-evaluation outputs. On commit, both update. Next delta cycle, both re-evaluate with committed values. This naturally converges:

- Delta 1: Both NOR gates compute based on old state → defer → commit.
- Delta 2: Both re-evaluate with committed values. If converged, done.
- Repeat until stable (same cap as current delta cycle limit).

This is actually **better** than functional mode's iterative settling, which depends on topological order within each iteration. Non-blocking makes it fully order-independent.

### 7.4 Async preset/clear on flip-flops

Preset and clear are **level-sensitive** (active-low) and override the clock. They work unchanged with non-blocking because they read current inputs (via `updateInputs()`) and the non-blocking mechanism ensures consistency within each delta cycle. When preset/clear changes, the flip-flop is scheduled as a successor, evaluates, and applies the override.

### 7.5 updateInputs() Invalid handling

When `updateInputs()` encounters an Invalid (unconnected) predecessor, it sets all outputs to Invalid directly on `m_outputValues` (line 22-27 of LogicElement.cpp). With non-blocking, this write to `m_outputValues` is captured by `deferOutputs()` into `m_pendingOutputs` and then restored. On commit, the Invalid values are applied. Correct behavior.

### 7.6 Elements evaluated but output unchanged

If an element evaluates but produces the same outputs (e.g., an AND gate with one input changed but the output still 0), `commitOutputs()` returns false and no successors are scheduled. No unnecessary propagation.

---

## 8. Functional Mode Integration

### Stage 1: Temporal mode only (non-blocking)

Keep `updateFunctional()` unchanged. Only modify `updateTemporal()` to use two-phase delta cycles. This is the minimal change that fixes the temporal engine.

Functional mode continues to use topological sort + `m_lastValue`. The flip-flops need `m_lastValue` removed — but they're shared between modes. **Resolution**: The flip-flops read current inputs (no m_lastValue). In functional mode with topological sort, the flip-flop reads current D which was computed earlier in the same pass. Since topological order ensures D's predecessors evaluated before the flip-flop, D is the value computed *this* tick. This is equivalent to `m_lastValue` when D hasn't changed since the previous tick, and diverges only when D and CLK change in the same tick (which the current test pattern avoids).

**Functional mode trace (after m_lastValue removal):**
```
Tick N-1: D=0, CLK=0. FF evaluates: no edge. m_lastClk = Inactive.
Tick N:   D=1, CLK=1. Topological order: D's combinational logic evaluates first.
          FF evaluates: D=1 (current), CLK=Active, m_lastClk=Inactive → EDGE.
          Q = D = 1. m_lastClk = Active.
```
vs old behavior:
```
Tick N-1: D=0, CLK=0. FF: m_lastValue=0, m_lastClk=Inactive.
Tick N:   D=1, CLK=1. FF: edge detected. Q = m_lastValue = 0.
          m_lastValue=1.
Tick N+1: CLK=0 (falling). No edge. m_lastValue=1.
Tick N+2: CLK=1 (rising). Edge! Q = m_lastValue = 1.
```

So with m_lastValue, the flip-flop captures D **one tick late** (captures 0 instead of 1 when D and CLK rise on the same tick). Without m_lastValue, it captures D immediately. For the pathological case of D and CLK from the same source, m_lastValue gives the "old D" — but this is a 1-tick artifact, not real setup-time modeling.

In practice, clock elements toggle on their own real-time schedule (independent of data). The clock never changes at the exact same `update()` call as a user-toggled InputSwitch, so D and CLK never change simultaneously in normal usage.

### Stage 2: Full unification (future)

Replace `updateFunctional()` with the event-driven engine at zero delays:
- Clocks and input changes generate events.
- Delta cycles propagate through the circuit (one level per delta cycle).
- No topological sort needed — event propagation handles ordering.
- Iterative settling for feedback is handled by delta cycle convergence.

This eliminates the dual-engine architecture entirely. Can be done as a follow-up.

---

## 9. Test Impact Analysis

### Tests that PASS unchanged

All existing flip-flop tests follow the pattern "set data → update → clock edge → update":

| Test | Pattern | Safe? |
|------|---------|:-----:|
| TestSequentialLogic::testDFlipFlopBehavior | swD.setOn → update → clockCycle | Yes |
| TestSequentialLogic::testJKFlipFlopBehavior | set J/K → update → clockCycle | Yes |
| TestSequentialLogic::testSRFlipFlopBehavior | set S/R → update → clockCycle | Yes |
| TestSequentialLogic::testTFlipFlopBehavior | set T → update → clockCycle | Yes |
| TestSequentialLogic::testDLatchBehavior | No m_lastValue (level-sensitive) | Yes |
| TestSequentialLogic::testSRLatchBehavior | No m_lastValue (level-sensitive) | Yes |
| TestLevel1DFlipFlop (all 8 steps) | Data set before clock edge | Yes |
| TestLevel1JkFlipFlop (6 tests) | clockPulse() after data setup | Yes |
| TestLevel3Register1bit | clockCycle after data setup | Yes |
| TestLevel4RippleAdder (all) | Combinational, no flip-flops | Yes |
| TestTemporalSimulation::testZeroDelay* | Combinational gates, SR latch | Yes |

### Tests that may need updating

| Test | Risk | Reason |
|------|------|--------|
| TestTemporalSimulation (propagation delay tests) | Low | Test NOT/AND delays — no flip-flops involved |
| Integration IC tests with nested flip-flops | Low | clockCycle helper separates D and CLK updates |

**Expected result**: All 136 tests pass with no changes.

---

## 10. Implementation Plan

### Files changed

| File | Change |
|------|--------|
| `App/Element/LogicElements/LogicElement.h` | Add `m_pendingOutputs`, `m_savedOutputs`, 3 new methods |
| `App/Element/LogicElements/LogicElement.cpp` | Implement `prepareForEvaluation()`, `deferOutputs()`, `commitOutputs()` |
| `App/Element/LogicElements/LogicDFlipFlop.h` | Remove `m_lastValue` |
| `App/Element/LogicElements/LogicDFlipFlop.cpp` | Read current D instead of m_lastValue |
| `App/Element/LogicElements/LogicJKFlipFlop.h` | Remove `m_lastJ`, `m_lastK` |
| `App/Element/LogicElements/LogicJKFlipFlop.cpp` | Read current J/K instead of m_lastJ/m_lastK |
| `App/Element/LogicElements/LogicTFlipFlop.h` | Remove `m_lastValue` |
| `App/Element/LogicElements/LogicTFlipFlop.cpp` | Read current T instead of m_lastValue |
| `App/Element/LogicElements/LogicSRFlipFlop.h` | Remove `m_lastS`, `m_lastR` |
| `App/Element/LogicElements/LogicSRFlipFlop.cpp` | Read current S/R instead of m_lastS/m_lastR |
| `App/Simulation/Simulation.cpp` | Two-phase delta cycles in `updateTemporal()` |

### Files unchanged

- `SimEvent.h` — no changes to event struct
- `EventQueue` — no changes
- `ElementMapping.cpp` — successor building unchanged
- All combinational gate logic (AND, OR, NOT, etc.) — unchanged
- All latch logic (DLatch, SRLatch) — unchanged (no m_lastValue)
- `updateFunctional()` — unchanged in stage 1
- All test files — expected to pass as-is

### Implementation order

1. Add `m_pendingOutputs`, `m_savedOutputs`, and 3 methods to `LogicElement`
2. Remove `m_lastValue`/`m_lastJ`/`m_lastK`/`m_lastS`/`m_lastR` from flip-flops
3. Simplify flip-flop `updateLogic()` to read current inputs
4. Refactor `updateTemporal()` to use two-phase delta cycles with deduplication
5. Run all 136 tests — verify no regressions
6. Add targeted tests for the D+CLK simultaneous change scenario
