# Temporal Simulation Plan

## Goal

Add an event-driven temporal simulation mode to wiRedPanda with configurable propagation delays, while preserving the current zero-delay behavior as the default mode.

---

## Implementation Status

| Phase | Description | Status | Commit |
|-------|-------------|--------|--------|
| Pre-work | Output change tracking in LogicElement | **Done** | `a9643d665` |
| 1 | Event Queue Infrastructure | **Done** | `711cf5c6b` |
| 2 | LogicElement Delay Support | **Done** | `711cf5c6b`, delays assigned next commit |
| 6 | Successor Scheduling | **Done** | `711cf5c6b` |
| 3 | Temporal Simulation Engine | **Done** | `711cf5c6b` |
| 4 | Clock Element Temporal Mode | **Done** | `711cf5c6b` |
| 5 | Input Element Temporal Mode | **Done** | `711cf5c6b` |
| Tests | Temporal simulation test suite (17 tests) | **Done** | next commit |
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

#### Approach: Extend BeWavedDolphin

A full waveform viewer already exists: **BeWavedDolphin** (`App/BeWavedDolphin/`, 2,452 lines, 12 source files). It's a mature waveform editor/analyzer with signal rendering, zoom/scroll, export (PNG/PDF/CSV), and `.dolphin` file persistence.

However, BeWavedDolphin is an **active simulation driver** — it paints input waveforms, then sweeps column-by-column calling `sim->update()`. Temporal mode needs **passive recording** during live simulation.

| Aspect | BeWavedDolphin (current) | Temporal mode needs |
|--------|-------------------------|-------------------|
| Time model | Discrete columns (user-defined steps) | Continuous nanoseconds (`SimTime`) |
| Simulation | Drives `sim->update()` per column | Passively records during live `updateTemporal()` |
| Input control | User paints input waveforms, then runs | Live circuit interaction |
| Resolution | 1 column = 1 sim cycle | Nanosecond-level transitions |
| Display | 8 SVG pixmaps (high/low/edges) | Similar, but time-proportional spacing |

**Reusable components:**
- `SignalDelegate` — waveform pixmap rendering (blue inputs, green outputs, edge detection)
- `WaveformView` — QGraphicsView with zoom/scroll
- `DolphinSerializer` — export to `.dolphin`/`.csv`/PNG/PDF
- Table-based UI layout (rows = signals, columns = time)
- `BeWavedDolphinUI` — toolbar, menus, keyboard shortcuts

**New code needed (~200 lines):**
1. `recordFromTemporalSim()` — hook into the temporal engine to capture `(SimTime, Status)` transitions per watched signal during live simulation
2. Time quantization — convert continuous `SimTime` transitions to discrete column indices for the existing `SignalModel` (pixels-per-nanosecond scaling)
3. Live append — as simulation advances, append new columns to the right edge of the table (ring buffer or auto-scroll)
4. Mode toggle — keep existing manual-input mode for functional simulation; add passive-recording mode for temporal simulation

**Key files to modify:**
- `App/BeWavedDolphin/BeWavedDolphin.h/.cpp` — add recording mode and temporal hooks
- `App/BeWavedDolphin/SignalModel.h/.cpp` — support dynamic column append
- `App/Simulation/Simulation.cpp` — emit signal on transition (or callback) for recording

This approach is ~200 lines vs ~500 for a new widget from scratch, and inherits all existing display/export/persistence infrastructure.

#### Chosen approach: Standalone TemporalWaveformWidget

A new purpose-built widget for temporal simulation, independent of BeWavedDolphin. Rationale: BeWavedDolphin is an active simulation driver with a fundamentally different time model (discrete columns). Adapting it would couple two unrelated time models. A standalone widget is cleaner and avoids polluting BeWavedDolphin's existing (working) codebase.

**New files:**
- `App/Simulation/WaveformRecorder.h` — per-signal transition recording (`QVector<QPair<SimTime, Status>>`)
- `App/Widgets/TemporalWaveformWidget.h/.cpp` — QWidget with custom `paintEvent()` drawing timing diagrams

**Recording mechanism:**
- `WaveformRecorder` stores a list of `SignalTrace` objects, each with a name and a vector of `(SimTime, Status)` transitions
- Hooked into `Simulation::updateTemporal()` — after each event where `outputChanged()`, record new output values for watched signals
- Watched signals selected by `LogicElement*` + port index

**Display:**
- Horizontal time axis (nanoseconds), vertical signal traces
- Each trace: horizontal lines at HIGH/LOW with vertical edges at transitions
- Scrollable (QScrollArea) and zoomable (pixels-per-nanosecond scale factor)
- Signal names on the left, time ruler on top

### Per-Type Default Delays (assigned)

Each element's `logicCreator` lambda in `ElementInfo<T>` calls `setPropagationDelay()` after creation. 16 element types have non-zero defaults; sources, sinks, and nodes remain at 0.

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

### Test Coverage

`Tests/Unit/Simulation/TestTemporalSimulation.cpp` — 17 tests covering:

- **EventQueue** (3): ordering, same-time FIFO, clear
- **LogicElement delay** (2): default delays assigned, zero-delay elements
- **Successor tracking** (1): lists built correctly after init
- **Mode switching** (2): default is Functional, setMode restarts
- **Zero-delay equivalence** (3): AND gate, cascaded gates, SR latch feedback
- **Propagation delays** (2): NOT gate delay, chained cumulative delay
- **Clock scheduling** (1): clock edges scheduled and toggle correctly
- **Input change detection** (1): InputSwitch toggle propagates in temporal mode

---

## Open Questions

1. **Transport delay**: Should we add `inputIndex` + `value` fields to `SimEvent` for future transport delay support, even if unused by inertial delay?

2. **Clock detection heuristic**: Current temporal engine detects clock LogicSource events by `delay==0 && outputSize==1 && inputPairs.empty()`. Would a `LogicClock` subclass be cleaner?

3. **Waveform viewer timing**: Should it be implemented alongside the UI integration, or deferred further?

4. **Per-element delay editing**: Should users be able to customize individual gate delays in temporal mode, or only use type defaults?
