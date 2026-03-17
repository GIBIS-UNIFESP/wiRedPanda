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

## 6. Future: Full Engine Unification

Applying non-blocking to functional mode requires solving the **initial-state symmetry problem** for gate-level feedback circuits. Possible approaches:

1. **Asymmetric initialization**: Initialize one output of each feedback pair to break symmetry. Requires circuit analysis to identify feedback pairs.

2. **Hybrid settling**: Use blocking for the first iteration (to break symmetry), then switch to non-blocking for subsequent iterations (to prevent cascade).

3. **Event-driven functional mode**: Replace `updateFunctional()` with the temporal event engine at zero delays. Events naturally order evaluation (predecessors fire before successors), avoiding both the symmetry and cascade problems. This is the cleanest long-term solution but the largest change.

These are deferred to a future effort.
