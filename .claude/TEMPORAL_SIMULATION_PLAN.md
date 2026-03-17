# Temporal Simulation Plan

## Goal

Add an event-driven temporal simulation mode to wiRedPanda with configurable propagation delays, while preserving the current zero-delay behavior as the default mode.

---

## Current Architecture Summary

The existing simulation (`App/Simulation/Simulation.cpp`) is a **synchronous cycle-based engine**:

- **1ms QTimer** drives `update()` at ~1000 steps/sec
- **Phase 1**: Clock elements toggle based on wall-clock time (`steady_clock`)
- **Phase 2**: User inputs (`InputSwitch`, `InputButton`, `InputRotary`) snapshot state to logic layer via `GraphicElementInput::updateOutputs()`
- **Phase 3**: All `LogicElement::updateLogic()` called in topological order (single pass), or iterative settling (up to 10 passes) if feedback loops exist
- **Phase 4**: Wire visuals updated from logic outputs via `updatePort(QNEOutputPort*)`
- **Phase 5**: Output elements (LEDs, buzzers, displays) refreshed via `updatePort(QNEInputPort*)`

Key files:
- `App/Simulation/Simulation.h/.cpp` — main loop (360 lines)
- `App/Simulation/ElementMapping.h/.cpp` — graph builder, topological sort (162 lines)
- `App/Core/Priorities.h` — DFS priority + Tarjan SCC algorithms
- `App/Element/LogicElements/LogicElement.h/.cpp` — base class (`updateLogic()` interface, 73 lines)
- `App/Element/GraphicElements/Clock.h/.cpp` — wall-clock-driven toggle

### Critical Architecture Details

**Connection model**: Predecessor-only. Each `LogicElement` stores `QVector<InputPair> m_inputPairs` where `InputPair = {LogicElement*, int port}`. There is NO successor tracking — successors are computed locally in `sortLogicElements()` as a temp variable and discarded after priority calculation.

**Input value resolution**: Live traversal at runtime. `inputValue(index)` calls `m_inputPairs[index].logic->outputValue(port)` — reads predecessor output directly each cycle. The `updateInputs()` method snapshots all predecessors into `m_inputValues` cache for the current `updateLogic()` call.

**IC flattening**: ICs are fully flattened. `IC::createLogicElements()` creates a nested `ElementMapping` for internal elements and returns all internal `LogicElement` shared_ptrs. These are merged into the parent's flat `m_logicElms` vector and participate in the same topological sort. Boundary elements are `LogicNode` (passthrough) instances bridging external ports to internal circuit.

**19 LogicElement subclasses**: LogicAnd, LogicOr, LogicNot, LogicNand, LogicNor, LogicXor, LogicXnor, LogicMux, LogicDemux, LogicTruthTable, LogicDFlipFlop, LogicJKFlipFlop, LogicTFlipFlop, LogicSRFlipFlop, LogicDLatch, LogicSRLatch, LogicNode, LogicSink, LogicSource.

**Visual refresh chain**: `LogicElement::setOutputValue()` → `updatePort(QNEOutputPort*)` reads logic and calls `port->setStatus()` → `QNEConnection::setStatus()` propagates to wire visuals → `updatePort(QNEInputPort*)` reads connection status → calls `element->refresh()` for output elements (LEDs call `setPixmap()`, Displays call `update()` triggering `paint()`, Buzzers control audio).

---

## Design

### Simulation Modes

| Mode | Behavior | Use case |
|------|----------|----------|
| **Functional** (current default) | Zero-delay, topological-sort single pass | Teach boolean logic, combinational/sequential concepts |
| **Temporal** (new) | Event-driven with per-element propagation delays | Teach timing, hazards, glitches, race conditions |

The user selects the mode from the UI. Functional mode preserves 100% backward compatibility.

### Core Concept: Event-Driven Simulation

Replace the "evaluate all elements every tick" model with a **priority queue of timestamped events**:

```
Event { time: SimTime, target: LogicElement*, inputIndex: int, value: Status }
```

When an element's input changes, it schedules its output change at `now + propagation_delay`. The simulator processes events in chronological order.

### Zero-Delay Mode in Temporal Engine

When all elements have delay=0, events scheduled at the same time form **delta cycles** — the engine processes all time-0 events, which may generate more time-0 events, repeating until quiescent. This is functionally equivalent to the current topological-sort approach:

- Combinational logic: converges to same steady state
- Sequential logic: edge detection unchanged (still uses `m_lastClk` pattern)
- Feedback loops: delta-cycle cap replaces iterative settling (same concept, cleaner)

This means the temporal engine with all-zero delays **can be the unified engine** if desired, or can coexist alongside the current engine.

### Simulation Time vs Wall Time

| Concept | Functional mode | Temporal mode |
|---------|----------------|---------------|
| **Simulation time** | Not tracked (implicit) | Explicit `SimTime` counter in nanoseconds |
| **Clock elements** | Wall-clock driven (`steady_clock`) | Simulation-time driven |
| **Advancement** | 1ms QTimer ticks | Process next event from queue |
| **Speed** | Real-time locked | Configurable ratio (1x, 10x, max-speed) |

---

## Implementation Phases

### Phase 1: Event Queue Infrastructure

**New files:**
- `App/Simulation/SimEvent.h` — event struct and priority queue
- `App/Simulation/SimTime.h` — simulation time type (uint64 nanoseconds)

```cpp
// SimTime.h
using SimTime = uint64_t;  // nanoseconds
constexpr SimTime SIM_TIME_UNSET = UINT64_MAX;

// SimEvent.h
struct SimEvent {
    SimTime time;
    LogicElement *target;
    int inputIndex;
    Status value;
    bool operator>(const SimEvent &other) const { return time > other.time; }
};

class EventQueue {
public:
    void schedule(SimEvent event);
    SimEvent pop();
    bool empty() const;
    SimTime nextTime() const;
    void clear();
private:
    std::priority_queue<SimEvent, std::vector<SimEvent>, std::greater<>> m_queue;
};
```

#### Investigation findings

**No surprises expected.** This is a self-contained data structure with no dependencies on existing code. The only design decision is the event struct layout.

**Concern — event deduplication**: When multiple inputs of the same element change at the same time, multiple events target the same element. The engine must handle this correctly — it should NOT re-evaluate the element once per event. Instead, apply all same-time events to the element, then evaluate once.

**Decision needed**: Should the queue support event cancellation? When an element's output changes before a previously scheduled event fires (e.g., a gate's input changes again within the propagation delay), the old event is stale. Options:
1. **Lazy invalidation**: Let stale events fire but detect "no actual change" and skip successor scheduling. Simpler, slightly wasteful.
2. **Eager cancellation**: Remove superseded events from queue. Complex (requires addressable heap or tombstones).

**Recommendation**: Lazy invalidation. Compare element output before/after `updateLogic()` — only schedule successors if output actually changed. This naturally handles both deduplication and cancellation.

**Estimated scope:** ~100 lines, 2 new files

---

### Phase 2: LogicElement Delay Support

**Modified files:**
- `App/Element/LogicElements/LogicElement.h/.cpp`

Add propagation delay to the base class:

```cpp
class LogicElement {
public:
    SimTime propagationDelay() const { return m_delay; }
    void setPropagationDelay(SimTime ns) { m_delay = ns; }
private:
    SimTime m_delay = 0;  // 0 = instant (functional mode behavior)
};
```

#### Investigation findings

**Concern — where are delays assigned?** The `LogicElement` is created by `ElementMetadataRegistry::logicCreator` functions (one per element type). These are lambdas registered in each `ElementInfo<T>` specialization. The delay must be set either:
- (a) In the `logicCreator` lambda (per-type default), or
- (b) In `GraphicElement::createLogicElements()` after creation (per-instance from file)

**Recommendation**: Both. The `logicCreator` sets a type-specific default. The `GraphicElement` can override from serialized per-instance values.

**Concern — IC internal delays**: IC elements are flattened, so their internal gates already have individual delays. The IC itself (as a `GraphicElement`) does NOT need a lumped delay. The flattened internal `LogicElement` instances carry their own delays naturally. **No special handling needed** — this simplifies the design significantly.

**Concern — LogicNode (passthrough buffer)**: Used as IC boundary elements. Should have delay=0 to avoid adding artificial latency at IC boundaries. This is the default, so no action needed.

**Concern — LogicSource (VCC/GND)**: `ElementMapping::m_globalVCC` and `m_globalGND` are shared singletons. They should have delay=0 (default). Since they never change value, they never generate events anyway.

**Concern — LogicSink (output elements)**: These are terminal — no successors. They don't need delay since they don't produce outputs. The visual refresh happens separately.

Default delays per element type (only meaningful in temporal mode):

| Element | Default delay | Rationale |
|---------|--------------|-----------|
| LogicNot | 5 ns | Single transistor stage |
| LogicAnd, LogicOr | 10 ns | Two-level logic |
| LogicNand, LogicNor | 8 ns | Single-level complementary |
| LogicXor, LogicXnor | 15 ns | Multi-level path |
| LogicMux | 20 ns | Selection + routing |
| LogicDemux | 20 ns | Decoding + routing |
| LogicTruthTable | 25 ns | Lookup + decoding |
| LogicDFlipFlop (clk-to-q) | 10 ns | Register propagation |
| LogicJKFlipFlop (clk-to-q) | 12 ns | More complex internal logic |
| LogicTFlipFlop (clk-to-q) | 10 ns | Similar to D |
| LogicSRFlipFlop (clk-to-q) | 10 ns | Similar to D |
| LogicDLatch (enable-to-q) | 8 ns | Transparent path |
| LogicSRLatch | 8 ns | Cross-coupled gate delay |
| LogicNode | 0 ns | Wire passthrough |
| LogicSource | 0 ns | Constant |
| LogicSink | 0 ns | Terminal |

**Estimated scope:** ~30 lines in LogicElement.h/.cpp, 1-2 lines in each of 19 subclass registrations

---

### Phase 3: Temporal Simulation Engine

Refactor `Simulation` to support both modes internally via mode dispatch:

```cpp
enum class SimulationMode { Functional, Temporal };

class Simulation {
public:
    void setMode(SimulationMode mode);
    SimulationMode mode() const;
    SimTime currentTime() const;

private:
    SimulationMode m_mode = SimulationMode::Functional;
    EventQueue m_eventQueue;
    SimTime m_currentTime = 0;
    SimTime m_timePerTick = 1000000;  // 1ms in ns (1x speed)

    void updateFunctional();  // current update() body, untouched
    void updateTemporal();    // new event-driven loop
};
```

#### Investigation findings

**CRITICAL — `updateLogic()` reads live predecessor outputs via `inputValue()`**: In the current model, `updateInputs()` traverses `m_inputPairs` and reads each predecessor's current `outputValue()`. This works because topological order guarantees predecessors are already updated.

In the temporal model, events deliver values to specific input slots. But `updateLogic()` calls `updateInputs()` which reads live predecessor outputs — it does NOT read from a delivered event value. This means we have two options:

**Option A — Bypass `updateInputs()`, use event-delivered values**: Add a separate input cache for event-driven mode where `applyInput(index, value)` writes directly to `m_inputValues[index]`. Then `updateLogic()` reads from `m_inputValues` without calling `updateInputs()`. Problem: `updateInputs()` also handles the Invalid check (returns false if any input is Invalid). We'd need to replicate this.

**Option B — Keep `updateInputs()`, let it read live predecessors**: Events change predecessor outputs before the target evaluates. Since the event loop processes all same-time events and then evaluates, by the time a target element's `updateLogic()` runs, its predecessor's `outputValue()` reflects the latest state. This works naturally — no change to `updateLogic()` or `updateInputs()`.

**Recommendation: Option B.** The event loop doesn't need to deliver values to input slots. Instead:
1. Event fires for element X
2. X calls `updateLogic()` which calls `updateInputs()` which reads live predecessor outputs
3. X computes new outputs via `setOutputValue()`
4. If outputs changed, schedule successor events

This means the event struct simplifies to: `{ time, target }` — no need for inputIndex or value. The event just says "re-evaluate this element at this time." Predecessor outputs are always live.

**Wait — this changes the event model fundamentally.** If events are just "re-evaluate element X at time T", then:
- When element A's output changes, we schedule events for all successors at `now + successor.delay`
- When successor B is evaluated, it reads A's output (which may have changed again since the event was scheduled)
- This is actually correct for zero-delay mode (delta cycles) but **incorrect for nonzero delays** because B should see A's output at the time the event was scheduled, not the current value

**CRITICAL ISSUE: Transport delay vs inertial delay**

- **Transport delay**: The output value at time T arrives at the successor at time T+d. The successor sees the value that was present when the event was created.
- **Inertial delay**: Short pulses (shorter than the delay) are absorbed. The successor sees the final stable value.

For educational purposes, **inertial delay** is more realistic and simpler to implement. With inertial delay, reading live predecessor outputs (Option B) is actually correct — a gate with 10ns delay that is re-evaluated at T+10 will read its predecessors' current outputs, which represent the stable state after any transients shorter than 10ns have been absorbed.

**However**, inertial delay means glitches shorter than the gate delay are invisible — which defeats one purpose of temporal simulation (showing glitches). For showing glitches, we need transport delay, which requires delivering the scheduled value.

**Revised recommendation**: Support both, but start with **inertial delay** (Option B, simpler). Add transport delay as a future enhancement. Inertial delay still shows hazards on paths with different delays — it just filters out sub-gate-delay glitches.

**Concern — sequential element `m_lastClk` pattern**: Flip-flops cache `m_lastClk` from the previous `updateLogic()` call. In the current model, this is the previous 1ms cycle. In temporal mode, "previous call" is the last time the element was evaluated (could be any time). Edge detection still works: if clock changes from Inactive→Active between evaluations, the flip-flop detects the rising edge. The key is that `m_lastClk` persists between evaluations regardless of timing.

**Concern — flip-flop setup time**: Current flip-flops use `m_lastValue` (D value from previous cycle) to model setup time. In temporal mode, this becomes "D value from the previous evaluation," which is semantically different. With inertial delay, the predecessor's output at evaluation time is stable for at least one gate delay, so using the current D value (not `m_lastValue`) would be more accurate.

**Decision needed**: In temporal mode, should flip-flops use `m_lastValue` (previous evaluation's D) or current D? The current behavior exists for a specific reason in the synchronous model — it prevents a change in D from being captured by a clock edge in the same cycle. In temporal mode, events ensure D changes propagate with proper timing, so the setup-time hack becomes unnecessary. However, changing this per-mode adds complexity.

**Recommendation**: Keep `m_lastValue` pattern unchanged for now. It's conservative and won't cause incorrect behavior — at worst it adds one extra gate-delay of latency to the D-to-Q path, which is educationally acceptable.

**Concern — `updateWithIterativeSettling()` in temporal mode**: Not needed. The event loop with delta cycles handles feedback naturally. When temporal mode is active, the settling code is simply not called.

**Concern — visual refresh frequency**: In temporal mode, simulation time may advance much faster than wall time (e.g., 100x speed). Visual refresh should still happen once per QTimer tick (1ms wall time), showing the state at the end of the simulated time window. The existing Phase 3-4 visual update code works unchanged — just call it after the event loop completes.

**Concern — QTimer interval**: The 1ms timer remains the visual refresh rate. In temporal mode, each tick advances simulation time by `m_timePerTick` (configurable). At 1x speed: 1ms sim time per tick. At 100x: 100ms sim time per tick. At max speed: process all pending events regardless of sim time.

**Estimated scope:** ~200 lines in Simulation.h/.cpp (the temporal loop is simpler than initially estimated since we use Option B)

---

### Phase 4: Clock Element Temporal Mode

**Modified files:**
- `App/Element/GraphicElements/Clock.h/.cpp`

#### Investigation findings

**Current clock architecture**: `Clock::updateClock(steady_clock::time_point)` is called every 1ms tick. It compares elapsed wall time against `m_interval` (half-period in microseconds). On toggle, advances `m_startTime` by exactly `m_interval` (anti-drift).

**Temporal mode change**: Instead of reading `steady_clock::now()`, clocks schedule toggle events based on simulation time.

**Concern — Clock is a `GraphicElementInput`**: In the current model, `Simulation::update()` calls `clock->updateClock()` separately, then `inputElm->updateOutputs()` for all inputs (including clocks). The clock's `isOn()` method reads `m_isOn` which was set by `updateClock()`. In temporal mode, the clock needs to inject events into the queue rather than directly setting `m_isOn`.

**Approach**: Add `scheduleNextEdge(EventQueue&, SimTime from, SimTime to)` to Clock. This method calculates all toggle points within the time window and schedules events. Each event targets the clock's own `LogicSource` element.

```cpp
void Clock::scheduleNextEdge(EventQueue &queue, SimTime from, SimTime to) {
    while (m_nextEdgeSimTime <= to) {
        queue.schedule({m_nextEdgeSimTime, logic()});
        m_nextEdgeSimTime += m_halfPeriodNs;
    }
}
```

**Concern — Clock frequency to SimTime conversion**: `m_interval` is `std::chrono::microseconds`. For temporal mode, need `m_halfPeriodNs` in nanoseconds. Conversion: `m_halfPeriodNs = m_interval.count() * 1000`. This should be computed once when frequency changes.

**Concern — Clock initial state**: `resetClock()` currently sets `m_isOn = true` and applies delay offset. In temporal mode, `m_nextEdgeSimTime` should be initialized to `delay_fraction * period_ns`.

**Concern — Clock `updateOutputs()`**: In temporal mode, when a clock event fires, the clock's `LogicSource` output must toggle. But `LogicSource` is a static element (`updateLogic()` is a no-op for sources). The event needs to call `logic()->setOutputValue(!logic()->outputValue())` directly, or Clock needs a custom `LogicElement` subclass that supports toggling.

**CRITICAL ISSUE**: `LogicSource::updateLogic()` just sets its output to the fixed value:
```cpp
void LogicSource::updateLogic() { setOutputValue(m_isActive ? Active : Inactive); }
```
If we call `updateLogic()` on a clock's LogicSource, it will reset to the initial value. The clock toggle must bypass `updateLogic()` and directly set the output.

**Solution**: When a clock event fires in the temporal engine, don't call `updateLogic()` on the clock's logic element. Instead, directly toggle the output:
```cpp
// In temporal event loop, special handling for clock events:
if (isClock(event.target)) {
    auto status = event.target->outputValue();
    event.target->setOutputValue(status == Active ? Inactive : Active);
} else {
    event.target->updateLogic();
}
```

Or better: give Clock its own `LogicClock` subclass that maintains toggle state internally. This avoids special-casing in the event loop.

**Recommendation**: Create `LogicClock` subclass of `LogicElement` that stores `m_isOn` and toggles on `updateLogic()`. In functional mode, `Clock::updateClock()` sets `m_isOn` and calls `logic()->setOutputValue()` as before. In temporal mode, each event calls `logic()->updateLogic()` which toggles.

Wait — that would change behavior in functional mode where `updateLogic()` is called every cycle. The toggle should only happen on clock events, not every cycle.

**Revised approach**: Keep `LogicSource` for functional mode. For temporal mode, the event loop handles clock events specially:
1. Toggle the source output directly: `source->setOutputValue(source->outputValue() == Active ? Inactive : Active)`
2. Schedule successors (gates connected to clock output)

This keeps the LogicSource class unchanged and puts the clock-specific logic in the event loop dispatcher where it belongs.

**Estimated scope:** ~80 lines (more than initially estimated due to SimTime conversion and temporal state)

---

### Phase 5: Input Element Temporal Mode

**Modified files:**
- `App/Element/GraphicElementInput.h/.cpp`

#### Investigation findings

**Current input types that need temporal support:**
1. `InputSwitch` — bistable, `isOn()` returns persistent bool
2. `InputButton` — momentary, `isOn()` returns press state
3. `InputRotary` — multi-output, `isOn(port)` returns true for selected port only
4. `Clock` — handled separately in Phase 4

**Note**: `InputVCC` and `InputGND` do NOT inherit from `GraphicElementInput`. They inherit directly from `GraphicElement`. Their logic is `LogicSource` (constant). They never change and never generate events.

**Concern — `updateOutputs()` is inline in the header**: `GraphicElementInput::updateOutputs()` is defined inline in `GraphicElementInput.h`. Adding temporal-mode logic here requires either:
- (a) Moving implementation to a .cpp file (currently there is no GraphicElementInput.cpp)
- (b) Adding a conditional inline

**Note**: There IS no `GraphicElementInput.cpp` file. The entire class is header-only. Creating a .cpp file for this class would be a larger refactor.

**Recommendation**: Keep `updateOutputs()` unchanged for functional mode. For temporal mode, add `scheduleIfChanged()` as a new method. The `Simulation::updateTemporal()` loop calls `scheduleIfChanged()` instead of `updateOutputs()`.

**Concern — InputRotary has multiple outputs**: `InputRotary::outputSize()` returns 2-16. When the user clicks to advance position, one output goes HIGH and the previous goes LOW — two changes in one click. `scheduleIfChanged()` must iterate all ports and schedule events for each changed port.

**Concern — where to store `m_lastScheduled`**: Need a `QVector<Status>` tracking last-scheduled values. This must be initialized during `Simulation::initialize()` and sized to `outputSize()`. Options:
- Store in `GraphicElementInput` (adds member to base class)
- Store in `Simulation` parallel array (keeps element class clean)

**Recommendation**: Store in `GraphicElementInput`. It's a small addition and the class already has state (`m_locked`). Initialize in `scheduleIfChanged()` on first call (lazy init, no coupling to Simulation::initialize).

**Estimated scope:** ~50 lines (including creating state tracking)

---

### Phase 6: Successor Scheduling

**Modified files:**
- `App/Element/LogicElements/LogicElement.h/.cpp` — add successor tracking
- `App/Simulation/ElementMapping.cpp` — populate successor lists

#### Investigation findings

**Current state**: Successors are computed locally in `ElementMapping::sortLogicElements()` as `QHash<LogicElement*, QVector<LogicElement*>>` and discarded after priority calculation. They are never stored persistently.

**What we need**: For each output port of element A, which (element, input_index) pairs are connected? This is the reverse of `InputPair`.

**Proposed structure:**
```cpp
struct OutputPair {
    LogicElement *logic = nullptr;  // Successor element
    int inputPort = 0;              // Input port index on the successor
};

// In LogicElement:
QVector<QVector<OutputPair>> m_successorPairs;  // [outputIndex] -> list of successors
```

**Concern — building the successor list**: Must be built from `m_inputPairs` of all elements. For each element B with `m_inputPairs[i] = {A, outPort}`, add `{B, i}` to `A->m_successorPairs[outPort]`.

**Where to build**: In `ElementMapping::sortLogicElements()`, right after building the local `successors` hash (which already does this but without port-level granularity).

**CRITICAL — the existing successor computation loses port information**: The current code builds `QHash<LogicElement*, QVector<LogicElement*>>` which only tracks "A has successor B" — it doesn't record WHICH output port of A connects to WHICH input port of B. For the temporal engine, we need the full `(outputPort, successor, inputPort)` triple.

**Solution**: Build the output-port-granular successor list during `sortLogicElements()`:

```cpp
// In sortLogicElements():
for (const auto &logic : m_logicElms) {
    // Initialize successor vectors
    logic->initSuccessors(logic->outputSize());

    for (int inIdx = 0; inIdx < logic->inputPairs().size(); ++inIdx) {
        const auto &pair = logic->inputPairs()[inIdx];
        if (pair.logic) {
            pair.logic->addSuccessor(pair.port, {logic.get(), inIdx});
        }
    }
}
```

**Concern — successor list lifetime**: Successors are invalidated when `Simulation::restart()` is called (clears `m_initialized`, next `initialize()` rebuilds everything). Since `LogicElement` instances are owned by `shared_ptr` in `ElementMapping::m_logicElms`, and `ElementMapping` is rebuilt on restart, the successor lists are naturally cleared and rebuilt. No stale pointer risk.

**Concern — `LogicSource` successors (VCC/GND)**: `m_globalVCC` and `m_globalGND` in `ElementMapping` are NOT in the `m_logicElms` vector — they're direct members. They can have many successors (every unconnected optional port defaults to VCC or GND). These should also have successor lists, but since they never change value, they'll never generate events. Safe to track but never used.

**Concern — successor deduplication**: Same element can appear multiple times as successor of the same output port (if multiple inputs of B connect to the same output of A). The current code already checks `!successors[pair.logic].contains(logic.get())` for dedup. For port-granular tracking, we should dedup by `(successor, inputPort)` pair — but in practice, two inputs of the same element connecting to the same output port is rare. Skip dedup for simplicity; duplicate events are harmless with the lazy invalidation approach from Phase 1.

**Estimated scope:** ~60 lines

---

### Phase 7: Waveform Visualization (Future Phase)

Add a timing diagram view that records signal transitions over simulation time.

#### Investigation findings

**This phase is independent** of the temporal engine and can be deferred. However, it is the primary user-facing value of temporal simulation. Without it, users can only observe final state, not timing behavior.

**Minimal viable waveform viewer:**
- `App/Widgets/WaveformView.h/.cpp` — QWidget subclass
- Records `QVector<QPair<SimTime, Status>>` per watched signal
- Horizontal time axis, vertical signal traces
- User selects signals by clicking wires/ports in the scene
- Scrollable and zoomable

**Concern — signal recording overhead**: Recording every transition for every signal is expensive. Should only record explicitly "watched" signals. The user adds/removes signals from the waveform view.

**Concern — memory for long simulations**: Need a ring buffer or automatic pruning (keep last N transitions or last T simulation time).

**Estimated scope:** ~500 lines (significant standalone feature)

---

### Phase 8: UI Integration

**Modified files:**
- `App/UI/MainWindow.h/.cpp` or toolbar
- `App/Scene/Scene.h/.cpp` (mode storage)

#### Investigation findings

**Where simulation is controlled**: `MainWindow` has play/pause/stop buttons. The `Scene` owns the `Simulation` instance via `scene->simulation()`. Starting/stopping is done via `simulation()->start()`/`stop()`.

**Concern — mode must survive restart**: `Simulation::restart()` sets `m_initialized = false` but does NOT destroy the Simulation object. The mode setting persists across restarts. Good.

**Concern — mode change while running**: Switching from Functional to Temporal (or vice versa) while simulation is running requires:
1. Stop simulation
2. Change mode
3. Restart simulation (rebuilds ElementMapping, now with/without successor lists)

**Concern — per-file mode setting**: Should the simulation mode be stored in the `.panda` file? Probably not — it's a runtime preference, not a circuit property. Store it in application settings (QSettings).

**Minimal UI additions:**
- ComboBox or toggle button in toolbar: "Functional" / "Temporal"
- Speed slider (only enabled in Temporal mode): 1x, 2x, 5x, 10x, 50x, 100x, Max
- SimTime display label (only visible in Temporal mode): "Sim: 1.234 us"

**Concern — ElementEditor delay field**: In temporal mode, users might want to edit per-element delays. This can be added to `ElementEditor` as an optional field (visible only in temporal mode). However, this is a nice-to-have, not essential for MVP.

**Estimated scope:** ~100 lines for minimal UI

---

## Revised Phase Order and Dependencies

```
Phase 1: Event Queue Infrastructure        (no dependencies)
Phase 2: LogicElement Delay Support         (no dependencies, can parallel with 1)
Phase 6: Successor Scheduling               (depends on 2 for OutputPair struct)
Phase 3: Temporal Simulation Engine         (depends on 1, 2, 6)
Phase 4: Clock Element Temporal Mode        (depends on 3)
Phase 5: Input Element Temporal Mode        (depends on 3)
Phase 8: UI Integration                     (depends on 3)
Phase 7: Waveform Visualization             (depends on 3, can be deferred)
```

Phases 1 and 2 can be implemented and tested independently. Phase 6 should be done before Phase 3 (the engine needs successors). Phases 4, 5, and 8 can be done in parallel after Phase 3.

---

## Detailed Risk Assessment

| Risk | Severity | Likelihood | Mitigation |
|------|----------|-----------|-----------|
| **Inertial delay hides glitches** | Medium | Certain | Document as known limitation; transport delay is a future enhancement |
| **Delta cycle explosion** | High | Medium | Hard cap at 1000 delta cycles per time step + user warning |
| **Performance: large circuits with many events** | Medium | Medium | Profile with real circuits; consider event batching or spatial partitioning |
| **Flip-flop `m_lastValue` semantics change** | Low | Low | Keep unchanged; adds at most one gate delay of D-to-Q latency |
| **Clock LogicSource toggle bypass** | Medium | Certain | Handle as special case in event loop; document clearly |
| **InputRotary multi-port events** | Low | Low | scheduleIfChanged iterates all ports; tested |
| **IC boundary delay accumulation** | Low | Low | LogicNode has delay=0; IC flattening works naturally |
| **No GraphicElementInput.cpp exists** | Low | Certain | Add new methods carefully; could create .cpp if needed |
| **GlobalVCC/GND not in m_logicElms** | Low | Certain | Never changes, never generates events; safe to ignore |
| **Visual refresh at high sim speeds** | Medium | Medium | Always refresh once per QTimer tick regardless of sim speed |

---

## Testing Strategy

### Unit Tests (Phase 1-2)
- EventQueue ordering (events sorted by time)
- EventQueue with same-time events (FIFO within same time)
- LogicElement delay getter/setter
- SimTime arithmetic (overflow at ~584 years, not a concern)

### Integration Tests (Phase 3-6)
- **Equivalence tests**: Run all 136 existing tests in temporal mode with zero delays — all must pass identically
- **Simple delay test**: NOT gate with 5ns delay; input changes at T=0, output changes at T=5ns
- **Chain delay test**: AND → NOT → OR chain; verify cumulative delay
- **Feedback convergence**: SR latch settles within delta cycle limit
- **Clock test**: Clock at 1MHz in temporal mode; verify edge timing
- **InputSwitch test**: Toggle switch, verify event generation and propagation

### Edge Case Tests
- Empty circuit in temporal mode
- Circuit with only VCC/GND (no events generated)
- Circuit with disconnected elements
- Rapid input changes (multiple toggles within one QTimer tick)

---

## File Change Summary

| File | Change type | Phase | Lines |
|------|------------|-------|-------|
| `App/Simulation/SimTime.h` | **New** | 1 | ~10 |
| `App/Simulation/SimEvent.h` | **New** | 1 | ~50 |
| `App/Element/LogicElements/LogicElement.h` | Modified (delay + successors) | 2, 6 | ~20 |
| `App/Element/LogicElements/LogicElement.cpp` | Modified (successor init) | 6 | ~15 |
| `App/Simulation/ElementMapping.cpp` | Modified (build successor lists) | 6 | ~20 |
| `App/Simulation/Simulation.h` | Modified (mode, event queue, temporal state) | 3 | ~25 |
| `App/Simulation/Simulation.cpp` | Modified (updateTemporal, mode dispatch) | 3 | ~150 |
| `App/Element/GraphicElements/Clock.h` | Modified (temporal scheduling state) | 4 | ~15 |
| `App/Element/GraphicElements/Clock.cpp` | Modified (scheduleNextEdge, SimTime conversion) | 4 | ~40 |
| `App/Element/GraphicElementInput.h` | Modified (scheduleIfChanged, m_lastScheduled) | 5 | ~25 |
| UI files (MainWindow/toolbar) | Modified | 8 | ~100 |
| ~16 LogicElement subclasses | Modified (default delay in registration) | 2 | ~1-2 each |

**Total new code:** ~500-600 lines (excluding tests and waveform viewer)

---

## Open Questions

1. **Transport vs inertial delay?** — Investigation recommends starting with inertial delay (simpler, Option B — `updateLogic()` reads live predecessor outputs). Transport delay requires delivering event values to input slots, which is a larger refactor of the `updateLogic()` pattern. Should we plan for transport delay in the architecture even if we don't implement it initially?

2. **Clock LogicElement subclass?** — Should we create a `LogicClock` subclass that handles toggling, or special-case clock events in the temporal engine loop? The former is cleaner OOP; the latter avoids a new class for a narrow use case.

3. **Per-element delay editing in UI?** — Should the MVP include per-element delay editing in `ElementEditor`, or just use type-specific defaults? Editing adds complexity but is essential for advanced users.

4. **Waveform viewer timing?** — Should Phase 7 be implemented alongside the temporal engine (Phases 3-6), or deferred to a follow-up? The temporal engine is much less useful without visual timing feedback.

5. **Event struct design?** — With inertial delay (Option B), events only need `{time, target}`. Should we still include `inputIndex` and `value` fields for future transport delay support, even if unused initially?
