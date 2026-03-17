# Temporal Simulation Plan

## Goal

Add an event-driven temporal simulation mode to wiRedPanda with configurable propagation delays, while preserving the current zero-delay behavior as the default mode.

---

## Implementation Status

| Phase | Description | Status | Commit |
|-------|-------------|--------|--------|
| Pre-work | Output change tracking in LogicElement | **Done** | `a9643d665` |
| 1 | Event Queue Infrastructure | **Done** | `711cf5c6b` |
| 2 | LogicElement Delay Support | **Done** (field added, defaults not yet assigned) | `711cf5c6b` |
| 6 | Successor Scheduling | **Done** | `711cf5c6b` |
| 3 | Temporal Simulation Engine | **Done** | `711cf5c6b` |
| 4 | Clock Element Temporal Mode | **Done** | `711cf5c6b` |
| 5 | Input Element Temporal Mode | **Done** | `711cf5c6b` |
| 8 | UI Integration | Not started | — |
| 7 | Waveform Visualization | Not started | — |

---

## Architecture (as implemented)

### Simulation Modes

```cpp
enum class SimulationMode { Functional, Temporal };
```

- **Functional** (default): Zero-delay, topological-sort single pass — original behaviour, untouched.
- **Temporal**: Event-driven with per-element propagation delays via inertial delay model.

Mode is selected via `Simulation::setMode()`. Switching mode resets simulation time and clears the event queue.

### Key Files

| File | Role |
|------|------|
| `App/Simulation/SimTime.h` | `using SimTime = uint64_t` (nanoseconds) |
| `App/Simulation/SimEvent.h` | `SimEvent{time, target}` struct + `EventQueue` min-heap |
| `App/Simulation/Simulation.h/.cpp` | Mode dispatch, `updateFunctional()`, `updateTemporal()`, `refreshVisuals()` |
| `App/Element/LogicElements/LogicElement.h` | `InputPair`, `OutputPair`, `m_propagationDelay`, `m_successors`, `m_outputChanged` |
| `App/Simulation/ElementMapping.cpp` | Builds port-granular successor lists in `sortLogicElements()` |
| `App/Element/GraphicElements/Clock.h/.cpp` | `scheduleEdges()`, `resetTemporalClock()` |
| `App/Element/GraphicElementInput.h` | `scheduleIfChanged()` with `m_lastScheduled` tracking |

### Delay Model: Inertial Delay (Option B)

Events carry only `{time, target}` — no value. When an event fires, the target calls `updateLogic()` which reads live predecessor outputs via `updateInputs()`. This implements **inertial delay**: short pulses (shorter than gate delay) are naturally absorbed because the predecessor's output has already settled by the time the successor evaluates.

**Consequence**: Glitches shorter than a gate's propagation delay are invisible. Transport delay (carrying values in events) is a future enhancement for glitch-visible simulation.

### Temporal Update Loop

```
updateTemporal():
  1. Schedule clock edge events within [currentTime, targetTime]
  2. Detect user input changes → schedule immediate successor events
  3. Process events from queue up to targetTime:
     - All events at same timestamp = one delta cycle
     - Clock events: toggle LogicSource output directly (bypass updateLogic)
     - Other events: clearOutputChanged → updateLogic
     - If output changed: schedule successors at currentTime + successor.delay
     - Delta cycle cap: 1000 (warns on overflow)
  4. Advance currentTime to targetTime
  5. refreshVisuals() — same as functional mode
```

### Clock Events in Temporal Mode

Clock elements use `LogicSource` which resets to its initial value on `updateLogic()`. The temporal engine detects clock events by checking `propagationDelay() == 0 && outputSize() == 1 && inputPairs().isEmpty()` and toggles the output directly instead of calling `updateLogic()`.

### Successor Tracking

Built in `ElementMapping::sortLogicElements()` using two passes:
1. **Init pass**: `initSuccessors(outputSize())` for all elements in `m_logicElms`, plus `m_globalVCC`/`m_globalGND`.
2. **Build pass**: For each element's input pair `{predecessor, outPort}`, call `predecessor->addSuccessor(outPort, {element, inIdx})`. Predecessors not in `m_logicElms` (e.g., IC-internal globalVCC/GND) get lazy-initialized.

### Implementation Pitfall Found During Development

**IC-internal VCC/GND predecessors**: When ICs are flattened, their internal `ElementMapping` creates its own `m_globalVCC`/`m_globalGND` LogicSource instances. These end up as predecessors of elements in the outer `m_logicElms`, but they are NOT in `m_logicElms` themselves. The successor-building code must lazily initialize their successor storage when encountered, or the `addSuccessor()` call will index into an empty vector and crash.

---

## Remaining Work

### Phase 8: UI Integration (not started)

**What's needed:**
- Mode selector in toolbar: ComboBox or toggle "Functional" / "Temporal"
- Speed control for temporal mode: 1x, 2x, 5x, 10x, 50x, 100x, Max
- SimTime display label (visible in temporal mode): "Sim: 1.234 µs"
- Per-element delay editing in `ElementEditor` (optional, advanced)

**Where to integrate:**
- `MainWindow` has play/pause/stop buttons; `Scene` owns `Simulation`
- Mode stored in application settings (`QSettings`), not in `.panda` files
- `setMode()` already handles restart; UI just calls it

### Phase 7: Waveform Visualization (not started, future)

Timing diagram view recording `QVector<QPair<SimTime, Status>>` per watched signal. ~500 lines. This is the primary user-facing value of temporal simulation.

### Per-Type Default Delays (not yet assigned)

The `m_propagationDelay` field exists but defaults to 0 for all elements. Setting type-specific defaults can be done in the `logicCreator` lambda of each `ElementInfo<T>` specialization:

| Element | Default delay | Rationale |
|---------|--------------|-----------|
| LogicNot | 5 ns | Single transistor stage |
| LogicAnd, LogicOr | 10 ns | Two-level logic |
| LogicNand, LogicNor | 8 ns | Single-level complementary |
| LogicXor, LogicXnor | 15 ns | Multi-level path |
| LogicMux | 20 ns | Selection + routing |
| LogicDemux | 20 ns | Decoding + routing |
| LogicTruthTable | 25 ns | Lookup + decoding |
| LogicDFlipFlop | 10 ns | Clk-to-q propagation |
| LogicJKFlipFlop | 12 ns | More complex internal logic |
| LogicTFlipFlop | 10 ns | Similar to D |
| LogicSRFlipFlop | 10 ns | Similar to D |
| LogicDLatch | 8 ns | Transparent path |
| LogicSRLatch | 8 ns | Cross-coupled gate delay |
| LogicNode | 0 ns | Wire passthrough (IC boundary) |
| LogicSource | 0 ns | Constant |
| LogicSink | 0 ns | Terminal |

---

## Open Questions

1. **Transport delay**: Should we add `inputIndex` + `value` fields to `SimEvent` for future transport delay support, even if unused by inertial delay?

2. **Clock detection heuristic**: Current temporal engine detects clock LogicSource events by `delay==0 && outputSize==1 && inputPairs.empty()`. Would a `LogicClock` subclass be cleaner?

3. **Waveform viewer timing**: Should it be implemented alongside the UI integration, or deferred further?

4. **Per-element delay editing**: Should users be able to customize individual gate delays in temporal mode, or only use type defaults?
