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

## 6. Next: 4-State Logic (Logisim Approach)

Applying non-blocking to functional mode requires solving the **initial-state symmetry problem**. Logisim Evolution solves this with a 4-state value system. Analysing their `Value.java` reveals the mechanism.

### 6.1 The 4 states

| Logisim | wiRedPanda (current) | wiRedPanda (proposed) | Meaning |
|---------|---------------------|----------------------|---------|
| `FALSE` | `Status::Inactive` | `Status::Inactive` | Logic low (0) |
| `TRUE` | `Status::Active` | `Status::Active` | Logic high (1) |
| `UNKNOWN` | `Status::Invalid` | `Status::Invalid` | Undriven / floating / high-Z |
| `ERROR` | *(missing)* | **`Status::Error`** | Conflict / indeterminate |

### 6.2 How it breaks symmetry

Logisim's gate operations follow **domination rules**:

**AND**: FALSE dominates — `AND(FALSE, UNKNOWN) = FALSE` (a single 0 forces output to 0).
**OR**: TRUE dominates — `OR(TRUE, UNKNOWN) = TRUE` (a single 1 forces output to 1).
**NOT**: only inverts known values — `NOT(UNKNOWN) = ERROR`, `NOT(ERROR) = ERROR`.
**XOR**: requires all inputs known — any UNKNOWN/ERROR input → ERROR.

Full Logisim single-bit truth tables (from `Value.java` lines 329–531):

```
AND:  F∧F=F  F∧T=F  F∧?=F  F∧E=F     (FALSE dominates)
      T∧F=F  T∧T=T  T∧?=E  T∧E=E
      ?∧F=F  ?∧T=E  ?∧?=E  ?∧E=E
      E∧F=F  E∧T=E  E∧?=E  E∧E=E

OR:   F∨F=F  F∨T=T  F∨?=E  F∨E=E     (TRUE dominates)
      T∨F=T  T∨T=T  T∨?=T  T∨E=T
      ?∨F=E  ?∨T=T  ?∨?=E  ?∨E=E
      E∨F=E  E∨T=T  E∨?=E  E∨E=E

NOT:  ¬F=T   ¬T=F   ¬?=E   ¬E=E
```

NAND = NOT(AND(...)). For an SR latch from NANDs, starting from all-UNKNOWN:

```
S=0 (active set), R=1 (hold).  Q=?, Q'=?

Iteration 1:
  NAND1 = NOT(AND(S=0, Q'=?)) = NOT(FALSE) = TRUE   → Q = TRUE   ← 0 dominates
  NAND2 = NOT(AND(R=1, Q=?))  = NOT(ERROR) = ERROR   → Q' = ERROR

Iteration 2:
  NAND1 = NOT(AND(0, ERROR)) = NOT(FALSE) = TRUE     → Q = TRUE   (stable)
  NAND2 = NOT(AND(1, TRUE))  = NOT(TRUE)  = FALSE    → Q' = FALSE ← resolved!

Converged: Q=1, Q'=0. Correct!
```

Hold mode (S=1, R=1) from UNKNOWN stays at ERROR — no oscillation because ERROR is a fixed point under these operations. The circuit correctly indicates "indeterminate until externally driven."

### 6.3 What changes in wiRedPanda

#### Enum change (1 file)

`App/Core/Enums.h`:
```cpp
enum class Status { Invalid = -1, Inactive = 0, Active = 1, Error = 2 };
```

#### Initial output value (1 file)

`App/Element/LogicElements/LogicElement.cpp` — constructor initialises outputs to `Status::Invalid` instead of `Status::Inactive`:
```cpp
LogicElement::LogicElement(const int inputSize, const int outputSize)
    : m_outputValues(outputSize, Status::Invalid)   // was Status::Inactive
```

#### updateInputs() change (1 file)

`App/Element/LogicElements/LogicElement.cpp` — stop bailing out on Invalid. Let each gate decide:
```cpp
bool LogicElement::updateInputs()
{
    for (int index = 0; index < m_inputPairs.size(); ++index) {
        const Status val = inputValue(index);
        if (val == Status::Invalid && !m_inputPairs.at(index).logic) {
            // Truly unconnected (null predecessor) — keep Invalid propagation
            for (auto &out : m_outputValues) {
                if (out != Status::Invalid) m_outputChanged = true;
                out = Status::Invalid;
            }
            return false;
        }
        m_inputValues[index] = val;
    }
    return true;
}
```

#### Helper functions (1 file, new)

Add `StatusOps` free functions or static methods for gate operations:

```cpp
// AND with domination: FALSE dominates Invalid/Error
Status statusAnd(Status a, Status b);
// OR with domination: TRUE dominates Invalid/Error
Status statusOr(Status a, Status b);
// NOT: only inverts known values
Status statusNot(Status a);
// Fold AND/OR over a range
Status statusAndAll(const QVector<Status> &inputs);
Status statusOrAll(const QVector<Status> &inputs);
```

#### Gate logic changes (11 combinational gate files)

Each gate replaces its current bool-based logic with 4-state-aware operations:

**LogicAnd** (before → after):
```cpp
// Before:
const bool result = std::all_of(inputs().cbegin(), inputs().cend(),
                                [](Status s) { return s == Status::Active; });
setOutputValue(result ? Status::Active : Status::Inactive);

// After:
setOutputValue(statusAndAll(inputs()));
```

**LogicOr**:
```cpp
setOutputValue(statusOrAll(inputs()));
```

**LogicNand**:
```cpp
setOutputValue(statusNot(statusAndAll(inputs())));
```

**LogicNor**:
```cpp
setOutputValue(statusNot(statusOrAll(inputs())));
```

**LogicNot**:
```cpp
setOutputValue(statusNot(inputs().at(0)));
```

**LogicXor / LogicXnor**: XOR has no dominant value. Any Invalid/Error input → Error:
```cpp
// If any input is Invalid or Error → output is Error
// Otherwise: count Active inputs, odd = Active, even = Inactive
```

**LogicMux**: Select lines with Invalid/Error → Error. Data input passes through (including Invalid/Error).

**LogicDemux**: Select lines with Invalid/Error → all outputs Error.

**LogicNode**: Pass-through unchanged (already forwards any Status including Invalid).

**LogicTruthTable**: Any Invalid/Error input → Error output.

#### Sequential elements (6 files)

**Flip-flops** (D, JK, T, SR): Invalid/Error on CLK → no edge detection (treat as unchanged). Invalid/Error on data inputs → propagate to output on edge. Invalid/Error on async preset/clear → Error output.

**Latches** (D, SR): Invalid/Error on enable → Error output. Invalid/Error on data while transparent → Error output.

#### Source/Sink (2 files)

**LogicSource**: No change — output set externally, updateLogic() is a no-op.

**LogicSink**: No change — terminal element.

#### UI / Visual layer (3 files)

**QNEConnection.cpp**: Add `case Status::Error:` → error color (e.g., orange) + thick pen.

**QNEPort.cpp**: Add `case Status::Error:` → error pen/brush.

**ThemeManager.h**: Add `m_connectionError`, `m_portErrorPen`, `m_portErrorBrush` colors.

#### Output elements (3-5 files)

**Led.cpp**: Error → dim/off + optional error indicator (red border?). Or treat Error like Inactive.

**Display7/14/16.cpp**: Error on any segment input → show "E" or blank.

**Buzzer/AudioBox**: Error → silent (treat like Inactive).

#### Waveform (1 file)

**TemporalWaveformWidget.cpp**: Add Error color for waveform traces.

#### Codegen (2 files)

**ArduinoCodeGen.cpp**: Error doesn't apply — codegen works on circuit structure, not simulation state.

**SystemVerilogCodeGen.cpp**: Same — no runtime Status in generated code.

### 6.4 What does NOT change

- **SimEvent.h / EventQueue** — events don't carry Status values
- **ElementMapping.cpp** — successor building is topology, not values
- **Simulation.cpp** — update loops unchanged (non-blocking temporal already done)
- **Serialization** — Status is not serialized; simulation recomputes it
- **Input elements** — switches/buttons produce Active/Inactive only (bool boundary)
- **Flip-flop m_lastValue pattern** — kept for functional mode

### 6.5 Implementation order

| Phase | Scope | Files | Tests |
|-------|-------|:-----:|:-----:|
| 1 | Add `Status::Error` enum + switch/case defaults | 3 | Compile-only |
| 2 | Add `statusAnd/Or/Not` helpers + unit tests | 2 | New tests |
| 3 | Change initial output to `Status::Invalid` + fix `updateInputs()` | 1 | Existing tests will fail |
| 4 | Update 11 combinational gates to use helpers | 11 | Fix failing tests |
| 5 | Update 6 sequential elements | 6 | Fix remaining tests |
| 6 | UI: connection/port/theme Error rendering | 3 | Visual verification |
| 7 | Output elements: LED, displays, buzzer | 3-5 | Integration tests |
| 8 | Waveform Error color | 1 | Visual verification |
| 9 | **Enable non-blocking in functional mode** | 1 | Full regression |

Phase 9 is the payoff — once Error breaks the symmetry, non-blocking iterative settling converges correctly for gate-level feedback circuits. Combined with the existing non-blocking temporal engine, this completes the unified engine.

### 6.6 Test impact

- **Phase 3 will break most existing tests** — they assume outputs start at Inactive (0), not Invalid. Tests that check initial LED/output state will fail.
- **Phases 4-5 restore test parity** — gates now compute correct outputs from Invalid inputs using domination rules. Circuits with all inputs driven will settle to the same Active/Inactive values as before.
- **New tests needed** for Error propagation, domination rules, and the SR latch symmetry-breaking scenario.
- **Expected final result**: All 136 existing tests pass + new Error-state tests.
