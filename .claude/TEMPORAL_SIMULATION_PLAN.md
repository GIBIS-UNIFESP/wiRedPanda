# Temporal Simulation Plan

## Goal

Add an event-driven temporal simulation mode to wiRedPanda with configurable propagation delays, while preserving the current zero-delay behavior as the default mode.

---

## Implementation Status

All phases complete.

| Phase | Description | Status | Commit |
|-------|-------------|--------|--------|
| Pre-work | Output change tracking in LogicElement | **Done** | `a9643d665` |
| 1 | Event Queue Infrastructure | **Done** | `711cf5c6b` |
| 2 | LogicElement Delay Support + per-type defaults | **Done** | `711cf5c6b`, `6721d093f` |
| 6 | Successor Scheduling | **Done** | `711cf5c6b` |
| 3 | Temporal Simulation Engine | **Done** | `711cf5c6b` |
| 4 | Clock Element Temporal Mode | **Done** | `711cf5c6b` |
| 5 | Input Element Temporal Mode | **Done** | `711cf5c6b` |
| 7 | Waveform Visualization (standalone widget) | **Done** | `863e4184e` |
| 8 | UI Integration (toolbar + waveform dock) | **Done** | `b5212c10c`, `85ae797ba` |
| Tests | 22 tests in TestTemporalSimulation | **Done** | `6721d093f`, `863e4184e` |
| Bug fixes | Clock icon, tab sync, code review (10 bugs) | **Done** | `e64b8c60e`, `2a435fdf2`, `cf9be05be`, `c32c57ffe` |

---

## Architecture (as implemented)

### Simulation Modes

```cpp
enum class SimulationMode { Functional, Temporal };
```

- **Functional** (default): Zero-delay, topological-sort single pass — original behaviour, untouched.
- **Temporal**: Event-driven with per-element propagation delays via inertial delay model.

Mode is selected via toolbar ComboBox or `Simulation::setMode()`. Switching mode resets simulation time, clears the event queue, and invalidates the waveform recorder.

### Key Files

| File | Role |
|------|------|
| `App/Simulation/SimTime.h` | `using SimTime = uint64_t` (nanoseconds) |
| `App/Simulation/SimEvent.h` | `SimEvent{time, target}` struct + `EventQueue` min-heap |
| `App/Simulation/Simulation.h/.cpp` | Mode dispatch, `updateFunctional()`, `updateTemporal()`, `refreshVisuals()` |
| `App/Simulation/WaveformRecorder.h` | `SignalTrace` + `WaveformRecorder` (per-signal transition recording) |
| `App/Simulation/ElementMapping.cpp` | Builds port-granular successor lists in `sortLogicElements()` |
| `App/Element/LogicElements/LogicElement.h` | `InputPair`, `OutputPair`, `m_propagationDelay`, `m_successors`, `m_outputChanged` |
| `App/Element/GraphicElements/Clock.h/.cpp` | `scheduleEdges()`, `resetTemporalClock()` |
| `App/Element/GraphicElementInput.h` | `scheduleIfChanged()`, `resetScheduledState()`, `m_lastScheduled` |
| `App/UI/TemporalWaveformWidget.h/.cpp` | Timing diagram widget with custom `paintEvent()`, zoom, time ruler |
| `App/UI/MainWindowUI.h/.cpp` | Mode ComboBox, speed ComboBox, sim time label in toolbar |
| `App/UI/MainWindow.h/.cpp` | Mode switching, waveform dock, Watch All / Clear, tab sync |

### Delay Model: Inertial Delay (Option B)

Events carry only `{time, target}` — no value. When an event fires, the target calls `updateLogic()` which reads live predecessor outputs via `updateInputs()`. This implements **inertial delay**: short pulses (shorter than gate delay) are naturally absorbed because the predecessor's output has already settled by the time the successor evaluates.

**Consequence**: Glitches shorter than a gate's propagation delay are invisible. Transport delay (carrying values in events) is a future enhancement for glitch-visible simulation.

### Temporal Update Loop

```
updateTemporal():
  1. Schedule clock edge events within [currentTime, targetTime]
     - Advance past stale edges before scheduling (prevents burst after pause)
  2. Detect user input changes → schedule immediate successor events
  3. Process events from queue up to targetTime:
     - All events at same timestamp = one delta cycle
     - Delta cycles repeat at same timestamp until no new same-time events
     - Clock events: clearOutputChanged → toggle LogicSource directly
     - Other events: clearOutputChanged → updateLogic
     - If output changed: schedule successors at currentTime + successor.delay
     - Delta cycle cap: 1000 per timestamp, 100000 total events per tick
  4. Record waveform transitions after each timestamp settles
  5. Sync clock graphic state (m_isOn + pixmap) from logic output
  6. Advance currentTime to targetTime
  7. refreshVisuals() — same as functional mode
```

### Clock Events in Temporal Mode

Clock elements use `LogicSource` which resets to its initial value on `updateLogic()`. The temporal engine detects clock events by checking `propagationDelay() == 0 && outputSize() == 1 && inputPairs().isEmpty()` and toggles the output directly instead of calling `updateLogic()`. After the event loop, `Clock::setOn()` syncs the graphic pixmap from the logic state.

### Successor Tracking

Built in `ElementMapping::sortLogicElements()` using two passes:
1. **Init pass**: `initSuccessors(outputSize())` for all elements in `m_logicElms`, plus `m_globalVCC`/`m_globalGND`.
2. **Build pass**: For each element's input pair `{predecessor, outPort}`, call `predecessor->addSuccessor(outPort, {element, inIdx})`. Predecessors not in `m_logicElms` (e.g., IC-internal globalVCC/GND) get lazy-initialized.

### Waveform Recording

`WaveformRecorder` stores `SignalTrace` objects, each with a name, `LogicElement*` + port index, and a deduplicating `QVector<QPair<SimTime, Status>>`. Recording is hooked into `updateTemporal()` after each timestamp settles. The recorder is cleared on `restart()` and `setMode()` to prevent dangling pointers when `ElementMapping` is rebuilt.

### UI Integration

- **Toolbar**: Mode ComboBox (Functional/Temporal), speed ComboBox (0.1x–100x, hidden in Functional), sim time label (auto-formatted ns/µs/ms, updated every 100ms)
- **Waveform dock**: Bottom dock widget toggled via Simulation → "Temporal Waveform" menu. Contains Watch All / Clear buttons and +/−/Fit zoom controls around a `TemporalWaveformWidget` in a QScrollArea.
- **Tab sync**: `connectTab()` applies the current ComboBox mode/speed to new tabs so opening files respects the UI state.

---

## Pitfalls Discovered During Development

1. **IC-internal VCC/GND predecessors**: Flattened ICs create internal `m_globalVCC`/`m_globalGND` that aren't in `m_logicElms`. Successor-building must lazy-init their storage.

2. **QToolBar widget visibility**: `QToolBar::addWidget()` wraps widgets in `QWidgetAction`. Must toggle the action's visibility, not the widget's.

3. **Clock graphic desync**: Temporal event loop toggles `LogicSource` directly, bypassing `Clock::setOn()`. Must sync `m_isOn` + pixmap after the event loop.

4. **New tab mode desync**: New `Simulation` objects default to Functional. Must apply ComboBox state in `connectTab()`.

5. **Dangling recorder pointers**: `WaveformRecorder` stores raw `LogicElement*` that become invalid when `ElementMapping` is rebuilt. Must clear recorder on restart/mode-switch. Watches must be set up AFTER `setMode()` + `update()`.

6. **Clock negative phase delay**: `m_delay` can be negative. `resetTemporalClock` must use signed arithmetic to avoid unsigned underflow.

7. **Stale m_lastScheduled**: `GraphicElementInput::m_lastScheduled` persists across restarts. Must call `resetScheduledState()` during temporal init.

---

## Per-Type Default Delays

Each element's `logicCreator` lambda calls `setPropagationDelay()` after creation. 16 element types have non-zero defaults.

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

## Test Coverage

`Tests/Unit/Simulation/TestTemporalSimulation.cpp` — 22 tests:

- **EventQueue** (3): ordering, same-time FIFO, clear
- **LogicElement delay** (2): default delays assigned, zero-delay elements
- **Successor tracking** (1): lists built correctly after init
- **Mode switching** (2): default is Functional, setMode restarts
- **Zero-delay equivalence** (3): AND gate, cascaded gates, SR latch feedback
- **Propagation delays** (2): NOT gate delay, chained cumulative delay
- **Clock scheduling** (1): clock edges scheduled and toggle correctly
- **Input change detection** (1): InputSwitch toggle propagates in temporal mode
- **Waveform recorder** (4): watch/record, deduplication, statusAt binary search, full integration
- **Waveform widget** (1): sizeHint and paint without crash
- **Existing tests**: All 135 pre-existing tests pass unmodified (functional mode untouched)

---

## BeWavedDolphin Alternative (documented, not used)

A full waveform viewer already exists: **BeWavedDolphin** (`App/BeWavedDolphin/`, 2,452 lines, 12 source files). It's a mature waveform editor/analyzer with signal rendering, zoom/scroll, export (PNG/PDF/CSV), and `.dolphin` file persistence.

However, BeWavedDolphin is an **active simulation driver** — it paints input waveforms, then sweeps column-by-column calling `sim->update()`. Temporal mode needs **passive recording** during live simulation.

| Aspect | BeWavedDolphin (current) | Temporal mode needs |
|--------|-------------------------|-------------------|
| Time model | Discrete columns (user-defined steps) | Continuous nanoseconds (`SimTime`) |
| Simulation | Drives `sim->update()` per column | Passively records during live `updateTemporal()` |
| Input control | User paints input waveforms, then runs | Live circuit interaction |
| Resolution | 1 column = 1 sim cycle | Nanosecond-level transitions |
| Display | 8 SVG pixmaps (high/low/edges) | Similar, but time-proportional spacing |

A standalone `TemporalWaveformWidget` was chosen to avoid coupling two unrelated time models. If BeWavedDolphin integration is desired in the future, the reusable components are: `SignalDelegate`, `WaveformView`, `DolphinSerializer`, and the table-based UI layout.

---

## Future Enhancements

1. **Unified engine** (attempted, deferred): Replacing `updateFunctional()` with the event engine at zero delays was attempted but failed. The `m_lastValue` pattern in flip-flops (DFlipFlop, JKFlipFlop, TFlipFlop, SRFlipFlop) captures data from the *previous* `updateLogic()` call to model setup time. This assumes every-tick evaluation. In the event engine, `m_lastValue` becomes stale when no events target the flip-flop. Removing `m_lastValue` and reading current inputs breaks circuits where D and CLK change in the same delta cycle (zero delay), because the flip-flop sees the new D instead of the pre-edge D. Proper fix options:
   - **Two-phase event processing**: Process data-path events before clock-path events within each delta cycle, so `m_lastValue` is fresh when the clock edge fires.
   - **Minimum clock-path delay**: Add a 1ns delay to clock inputs so the flip-flop always evaluates D before CLK, even at "zero" delay.
   - **Transport delay on data inputs**: Carry values in events so the flip-flop sees the D value at scheduling time.
   All options require significant refactoring of either the event loop or the flip-flop logic. The current dual-engine approach (functional = topological sort, temporal = events) works correctly for both modes.

2. **Transport delay**: Add `inputIndex` + `value` fields to `SimEvent` so elements see the value at scheduling time rather than evaluation time. Enables glitch-visible simulation.

3. **LogicClock subclass**: Replace the clock detection heuristic (`delay==0 && outputSize==1 && inputPairs.empty()`) with a proper `LogicClock` that handles toggling in `updateLogic()`.

4. **Per-element delay editing**: Add a delay field to `ElementEditor` (visible in temporal mode) so users can customize individual gate delays.

5. **Waveform export**: Add PNG/CSV export to the waveform dock (can reuse `DolphinSerializer` patterns).

6. **Signal selection via context menu**: Right-click on element → "Watch signal" / "Unwatch signal" for fine-grained waveform control instead of Watch All.
