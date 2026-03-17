# Temporal Simulation Plan

## Goal

Add an event-driven temporal simulation mode to wiRedPanda with configurable propagation delays, while preserving the current zero-delay behavior as the default mode.

---

## Current Architecture Summary

The existing simulation (`App/Simulation/Simulation.cpp`) is a **synchronous cycle-based engine**:

- **1ms QTimer** drives `update()` at ~1000 steps/sec
- **Phase 1**: Clock elements toggle based on wall-clock time (`steady_clock`)
- **Phase 2**: User inputs (`InputSwitch`, `InputButton`) snapshot state to logic layer
- **Phase 3**: All `LogicElement::updateLogic()` called in topological order (single pass), or iterative settling if feedback loops exist
- **Phase 4**: Wire visuals updated from logic outputs
- **Phase 5**: Output elements (LEDs, buzzers) refreshed

Key files:
- `App/Simulation/Simulation.h/.cpp` — main loop
- `App/Simulation/ElementMapping.h/.cpp` — graph builder, topological sort
- `App/Core/Priorities.h` — DFS priority + Tarjan SCC algorithms
- `App/Element/LogicElements/LogicElement.h/.cpp` — base class (`updateLogic()` interface)
- `App/Element/GraphicElements/Clock.h/.cpp` — wall-clock-driven toggle

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

When an element's input changes → it schedules its output change at `now + propagation_delay`. The simulator processes events in chronological order.

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
| **Clock elements** | Wall-clock driven | Simulation-time driven |
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
    // Min-heap comparison: earliest time first
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

**Estimated scope:** ~150 lines, 2 new files

### Phase 2: LogicElement Delay Support

**Modified files:**
- `App/Element/LogicElements/LogicElement.h/.cpp`

Add propagation delay to the base class:

```cpp
class LogicElement {
public:
    // New
    SimTime propagationDelay() const { return m_delay; }
    void setPropagationDelay(SimTime ns) { m_delay = ns; }

private:
    SimTime m_delay = 0;  // 0 = instant (functional mode behavior)
};
```

Each element subclass gets a default delay constant (only used in temporal mode):

| Element | Default delay |
|---------|--------------|
| NOT gate | 5 ns |
| AND, OR, NAND, NOR | 10 ns |
| XOR, XNOR | 15 ns |
| MUX, DEMUX | 20 ns |
| Flip-flops | 10 ns (clk-to-q) |
| Buffers | 3 ns |
| IC (subcircuit) | Sum of internal path |

**Estimated scope:** ~50 lines changed, touch ~20 files (1-2 lines each)

### Phase 3: Temporal Simulation Engine

**New file:**
- `App/Simulation/TemporalSimulation.h/.cpp`

Or, refactor `Simulation` to support both modes internally. The cleaner approach is a **strategy pattern**:

```cpp
// In Simulation.h
enum class SimulationMode { Functional, Temporal };

class Simulation {
public:
    void setMode(SimulationMode mode);
    SimulationMode mode() const;

    // Temporal mode accessors
    SimTime currentTime() const;
    void advanceTo(SimTime time);

private:
    // Existing members unchanged
    SimulationMode m_mode = SimulationMode::Functional;
    EventQueue m_eventQueue;
    SimTime m_currentTime = 0;

    // Dispatch
    void updateFunctional();  // current update() body
    void updateTemporal();    // new event-driven loop
};
```

**Temporal update loop:**

```cpp
void Simulation::updateTemporal() {
    if (!m_initialized && !initialize()) return;

    // Determine how much sim-time to advance this wall-clock tick
    SimTime targetTime = m_currentTime + m_timePerTick;  // e.g., 1000ns per 1ms tick at 1x speed

    // Process clocks: schedule next edge events
    for (auto *clock : m_clocks) {
        clock->scheduleNextEdge(m_eventQueue, m_currentTime, targetTime);
    }

    // Process user inputs: schedule immediate events for changed inputs
    for (auto *input : m_inputs) {
        input->scheduleIfChanged(m_eventQueue, m_currentTime);
    }

    // Event processing loop
    int deltaCount = 0;
    const int maxDelta = 1000;

    while (!m_eventQueue.empty() && m_eventQueue.nextTime() <= targetTime) {
        SimTime eventTime = m_eventQueue.nextTime();
        m_currentTime = eventTime;

        // Process all events at this exact time (delta cycle)
        while (!m_eventQueue.empty() && m_eventQueue.nextTime() == eventTime) {
            auto event = m_eventQueue.pop();
            // Apply input change to target element
            event.target->applyInput(event.inputIndex, event.value);
            // Re-evaluate element
            event.target->updateLogic();
            // Schedule output changes to successors
            scheduleSuccessors(event.target);
        }

        if (++deltaCount > maxDelta) {
            emit simulationWarning(tr("Temporal simulation: delta cycle limit exceeded — possible oscillation."));
            break;
        }
    }

    m_currentTime = targetTime;

    // Visual refresh (same as functional mode phases 3-4)
    refreshVisuals();
}
```

**Estimated scope:** ~300 lines, primarily in Simulation.cpp

### Phase 4: Clock Element Temporal Mode

**Modified files:**
- `App/Element/GraphicElements/Clock.h/.cpp`

In temporal mode, clocks schedule edge events into the event queue instead of reading wall time:

```cpp
void Clock::scheduleNextEdge(EventQueue &queue, SimTime from, SimTime to) {
    SimTime nextEdge = m_nextEdgeTime;
    while (nextEdge <= to) {
        queue.schedule({nextEdge, logic(), 0, m_nextValue});
        m_nextValue = !m_nextValue;
        nextEdge += m_halfPeriodNs;
    }
    m_nextEdgeTime = nextEdge;
}
```

**Estimated scope:** ~60 lines

### Phase 5: Input Element Temporal Mode

**Modified files:**
- `App/Element/GraphicElementInput.h/.cpp`

Track previous output state; only schedule events when user toggles:

```cpp
void GraphicElementInput::scheduleIfChanged(EventQueue &queue, SimTime now) {
    for (int port = 0; port < outputSize(); ++port) {
        Status newVal = isOn(port) ? Status::Active : Status::Inactive;
        if (newVal != m_lastScheduled[port]) {
            queue.schedule({now, logic(), port, newVal});
            m_lastScheduled[port] = newVal;
        }
    }
}
```

**Estimated scope:** ~40 lines

### Phase 6: Successor Scheduling

**Modified files:**
- `App/Simulation/Simulation.cpp`

When an element's output changes, schedule events for all connected successor elements:

```cpp
void Simulation::scheduleSuccessors(LogicElement *source) {
    for (int outIdx = 0; outIdx < source->outputSize(); ++outIdx) {
        Status val = source->outputValue(outIdx);
        for (auto &[successor, inIdx] : source->successorPairs(outIdx)) {
            SimTime delay = successor->propagationDelay();
            m_eventQueue.schedule({m_currentTime + delay, successor, inIdx, val});
        }
    }
}
```

This requires adding a **successor adjacency list** to `LogicElement` (currently only predecessors are tracked). This is built during `ElementMapping::connectElements()`.

**Modified files:**
- `App/Element/LogicElements/LogicElement.h/.cpp` — add `m_successorPairs`
- `App/Simulation/ElementMapping.cpp` — populate successor lists

**Estimated scope:** ~80 lines

### Phase 7: Waveform Visualization (Optional, Future)

Add a timing diagram view that records signal transitions over simulation time:

- `App/Widgets/WaveformView.h/.cpp` — QWidget showing horizontal traces
- Records `(SimTime, Status)` pairs per watched signal
- Scrollable time axis with zoom
- Cursor for reading exact timestamps

This is a significant UI feature and can be deferred to a later milestone.

### Phase 8: UI Integration

**Modified files:**
- `App/UI/MainWindow.h/.cpp` or toolbar

Add simulation mode selector:

- Toolbar dropdown or toggle: "Functional" / "Temporal"
- Speed control for temporal mode (1x, 10x, 100x, max)
- Current simulation time display (ns/us/ms counter)
- Per-element delay editor in ElementEditor (optional, for advanced users)

**Estimated scope:** ~100 lines UI code

---

## Migration Strategy

### Backward Compatibility

- **Default mode = Functional** — existing behavior unchanged
- **Temporal mode is opt-in** — user must explicitly switch
- **Zero-delay temporal** — if user switches to temporal but doesn't set delays, behavior matches functional mode via delta cycles
- **File format** — propagation delays stored per-element in `.panda` files; older files default to 0

### Testing Strategy

1. **Unit tests**: Event queue ordering, delta cycle convergence, delay arithmetic
2. **Equivalence tests**: Run existing test circuits in both modes with zero delays — outputs must match
3. **Temporal-specific tests**: Glitch detection, hazard circuits, setup/hold violation detection
4. **Performance tests**: Ensure temporal mode handles large circuits without excessive event generation

---

## Risk Assessment

| Risk | Mitigation |
|------|-----------|
| Performance degradation from event queue overhead | Event queue only active in temporal mode; functional mode unchanged |
| Delta cycle explosion (oscillating circuits) | Hard cap at 1000 delta cycles per time step with user warning |
| Sequential element edge detection semantics differ | Zero-delay temporal must use same `m_lastClk` pattern; add regression tests |
| IC flattening interacts with delay calculation | IC internal delays sum along critical path; or treat IC as atomic with single delay |
| Breaking existing simulation behavior | Functional mode code path is untouched; temporal is a parallel path |

---

## File Change Summary

| File | Change type | Phase |
|------|------------|-------|
| `App/Simulation/SimEvent.h` | **New** | 1 |
| `App/Simulation/SimTime.h` | **New** | 1 |
| `App/Element/LogicElements/LogicElement.h/.cpp` | Modified (delay + successors) | 2, 6 |
| `App/Simulation/Simulation.h/.cpp` | Modified (mode dispatch, temporal loop) | 3 |
| `App/Element/GraphicElements/Clock.h/.cpp` | Modified (temporal scheduling) | 4 |
| `App/Element/GraphicElementInput.h/.cpp` | Modified (change detection) | 5 |
| `App/Simulation/ElementMapping.cpp` | Modified (successor lists) | 6 |
| `App/Widgets/WaveformView.h/.cpp` | **New** (optional) | 7 |
| UI files (MainWindow/toolbar) | Modified | 8 |
| ~20 LogicElement subclasses | Modified (default delay constants, 1-2 lines each) | 2 |

---

## Open Questions

1. **Unified vs separate engines?** — Should temporal mode be a refactored `Simulation` class, or a separate `TemporalSimulation` class? Unified is cleaner but increases complexity of `Simulation.cpp`.

2. **Per-element delay editing?** — Should users be able to customize individual gate delays, or only use presets (ideal/typical/worst-case)?

3. **Waveform viewer priority?** — Temporal simulation is much less useful without a waveform viewer. Should Phase 7 be promoted to a core phase?

4. **IC delay model?** — Should ICs expose a single lumped delay, or should the engine trace through internal paths? Lumped is simpler; path tracing is more accurate.

5. **Clock model in temporal mode?** — Should clocks still support real-time mode (wall clock) in temporal mode for interactive use, or be purely simulation-time driven?
