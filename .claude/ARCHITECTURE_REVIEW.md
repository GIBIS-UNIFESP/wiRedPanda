# Architecture Review

Deep review of 2025-04-05. Every header and key implementation file read directly.

---

## Overall Structure

```
App/
├── Core/          — Application, Enums, Settings, ThemeManager, StatusOps
├── Element/       — GraphicElement base, IC, factory, metadata, registry
│   └── GraphicElements/  — 29 concrete element types
├── Nodes/         — QNEPort, QNEConnection (wires)
├── Scene/         — Scene, Commands, Workspace, managers (Connection/Clipboard/Visibility/PropertyShortcut)
├── Simulation/    — Simulation engine, SimulationBlocker
├── IO/            — Serialization, VersionInfo, FileUtils, RecentFiles
├── UI/            — MainWindow, ElementEditor, ElementPalette, dialogs
├── BeWavedDolphin/ — Waveform viewer (standalone sub-application)
├── CodeGen/       — Arduino + SystemVerilog code generators
└── Main.cpp
MCP/
├── Server/        — C++ JSON-RPC handlers bridging to App/
└── Client/        — Python test client
```

---

## What's Good

### 1. Scene delegates well

Scene owns 5 focused managers as value members with `{this}` initialization:
- `ConnectionManager` — wire creation, deletion, hover feedback
- `ClipboardManager` — copy/cut/paste/clone-drag
- `PropertyShortcutHandler` — keyboard property cycling
- `VisibilityManager` — gate/wire show/hide
- `ICRegistry` — IC definitions, file watching, blob storage

Each handles a distinct concern. Scene routes operations to them rather than implementing everything itself.

### 2. Workspace composes cleanly

`WorkSpace` owns `Scene` + `GraphicsView` as value members. The inline-IC-tab design (parent/child WorkSpaces linked via `QPointer<WorkSpace> m_parentWorkspace` and the `icBlobSaved` signal) is well structured — child tabs save blobs back to the parent's registry via signals rather than reaching up through the object tree.

### 3. Element metadata system is compile-time safe

Each element .cpp defines an `ElementInfo<T>` template specialization with:
- `static constexpr ElementConstraints` validated by `static_assert(validate(constraints))`
- `static ElementMetadata metadata()` providing display strings, pixmap paths, feature flags
- Self-registration via `static inline const bool registered = []() { ... }()` lambda

This guarantees every element type is registered before `main()` runs, and constraint violations are caught at compile time.

### 4. Serialization is version-aware and forward-compatible

The `VersionInfo` namespace (16 named predicates like `hasQMapFormat()`, `hasMetadata()`) makes load code self-documenting. `SerializationContext` bundles per-load state (portMap, version, contextDir, blobRegistry) without globals. The magic-header detection in `readPandaHeader()` gracefully handles 4 different legacy formats.

### 5. ID system is scene-local

`ItemWithId` provides a stable integer ID. `Scene` owns the registry (`QMap<int, ItemWithId*> m_elementRegistry`) and counter (`m_lastId`). IDs are assigned on `addItem()`, used by undo/redo commands to find elements after deserialization, and by port serial IDs (`elementId << 16 | portIndex`) for cross-referencing during save/load.

### 6. Graph algorithms are templated and reusable

`Priorities.h` provides `calculatePriorities<T>()` (iterative DFS priority assignment) and `findFeedbackNodes<T>()` (Tarjan's SCC) as free function templates. Both `Simulation::sortSimElements()` and `IC::initializeSimulation()` use them with `GraphicElement*`, but they could work with any node type.

### 7. Simulation model is correct for the educational purpose

1ms QTimer tick → update clocks → update inputs → topological-sort logic update → visual refresh (throttled to monitor rate). Feedback loops handled by iterative settling with max 10 iterations. Wireless Tx→Rx edges are injected into both the predecessor graph (for value routing) and the successor graph (for topological ordering). IC sub-circuits run their own internal simulation graph per `updateLogic()` call.

### 8. MCP architecture separates concerns

`MCPProcessor` handles JSON-RPC parsing/validation, `HandlerFactory` maps command names to handler types, 5 `BaseHandler` subclasses implement domain logic. The `IValidator` interface allows swapping validation strategies. Handlers access the App layer through `MainWindow*` (which gives access to WorkSpace → Scene → elements).

---

## Architectural Problems

### 1. `Core` → `UI` circular dependency

**Chain:** `Application.h` includes `MainWindow.h` → `MainWindowUI.h` → `ElementEditor.h` → `Scene.h` → (everything). `ThemeManager.h` includes `Application.h`. `Settings.h` includes `ThemeManager.h`. So any file needing `Settings` or `ThemeManager` transitively pulls in the entire UI + Scene + Element + Nodes + Simulation layer.

**Impact:** Slow compilation; conceptual violation — Core should be foundational.

**Fix:** Forward-declare `MainWindow` in Application.h; move include to Application.cpp. The `mainWindow()` return type becomes `MainWindow*` with an incomplete type in the header, which is fine since callers must include MainWindow.h themselves.

### 2. `Element` depends on `Scene` — inverted dependency

5 Element/ .cpp files include `Scene.h`:
- `GraphicElement.cpp` — needs `Scene::resolveContextDir()` and `Scene::gridSize`
- `GraphicElementSerializer.cpp` — needs `Scene::deserializationContext()`
- `IC.cpp` — needs Scene for deserialization context, and MainWindow.h for opening IC tabs
- `ICRegistry.cpp` — needs Scene for undo stack and element queries
- `ICDefinition.cpp` — needs Scene (though `Q_UNUSED(contextDir)`)

**Impact:** Bidirectional dependency makes it impossible to test Element/ independently.

**Fix:** `gridSize` should be a constant in a Core header. Context directory resolution needs an interface (`ISceneContext` with `contextDir()`, `deserializationContext()`, `itemById()`). IC double-click → open tab should use a signal, not `#include "MainWindow.h"`.

### 3. `Scene.h` is a transitive include bomb

`Scene.h` directly includes 7 internal headers: `ICRegistry.h`, `QNEPort.h`, `ClipboardManager.h`, `ConnectionManager.h`, `PropertyShortcutHandler.h`, `VisibilityManager.h`, `Simulation.h`. Since `Commands.h` includes `Scene.h`, and ~53 classes reference `Scene*`, this expands into a massive transitive closure.

**Impact:** Any change to any manager header triggers recompilation of almost everything.

**Fix:** The 5 managers and Simulation are stored as value members, which requires complete types. Either:
- Move to `std::unique_ptr` members (allows forward declaration, adds indirection)
- Or move manager includes to Scene.cpp and expose only forward-declared pointers via accessors (but this changes ownership semantics)

The cleanest approach: keep ConnectionManager/ClipboardManager as value members (they're frequently accessed in hot paths) but forward-declare VisibilityManager, PropertyShortcutHandler, and ICRegistry since they're accessed less often.

### 4. `GraphicElement` is a fat base class (655 lines, ~100 methods)

Responsibilities:
- Port management (add/remove/access input/output ports)
- Pixmap/appearance rendering with multi-state support
- Versioned binary serialization (save/load with 10+ version-aware helpers)
- Grid-snapping and wire-update callbacks (itemChange)
- Label and keyboard-trigger display
- Theme-aware selection highlight painting
- Virtual no-op hooks for: audio, color, frequency, delay, truth table, volume, wireless mode

Only ~8 of the 29 subclasses use audio/color/frequency/delay/volume. The rest inherit empty virtual methods.

**Impact:** Every element pays for capabilities it doesn't use (vtable entries, conceptual surface area). Adding a new capability means modifying the base class.

**Fix:** Not urgent — the no-op virtuals have negligible runtime cost. If the element count grows, consider property interfaces or a capability-query pattern. But for 29 elements the current design is pragmatic.

### 5. Flat element hierarchy — no intermediate bases for element families

Class tree:
```
GraphicElement
├── GraphicElementInput (4 subclasses: Clock, InputButton, InputSwitch, InputRotary)
├── IC
└── 24 direct subclasses (gates, flip-flops, latches, displays, audio, decorative)
```

Missing intermediate bases:
- **LogicGate** — And/Or/Not/Nand/Nor/Xor/Xnor are structurally identical (ElementInfo template + 2-line constructor + 4-line updateLogic)
- **FlipFlop** — DFlipFlop/JKFlipFlop/SRFlipFlop/TFlipFlop share: clock edge detection (`clk == Active && m_simLastClk == Inactive`), async preset/clear override (identical 4-line block), Q/Q-bar complementary output initialization, and identical `updateTheme()` implementation
- **Latch** — DLatch/SRLatch share the same pattern minus the clock edge
- **SegmentDisplay** — Display7/Display14/Display16 share: color-to-index mapping (identical `setColor()`), port-driven segment painting, and serialization pattern
- **AudioOutput** — AudioBox/Buzzer share: hardware detection, play/stop state machine, volume/mute management, identical `refresh()`
- **ToggleInput** — InputSwitch/InputButton share: `setOn(bool)`, `setAppearance()`, `appearanceStates()` (identical implementations)

**Impact:** ~500+ lines of duplication across element files.

**Fix:** Introduce 4-5 intermediate bases. The flip-flop base alone would save ~80 lines and make the clock-edge/preset-clear contract explicit.

### 6. `Commands.h/cpp` — no shared base for 10+ undo commands

Every command independently stores `QList<int> m_ids` + `Scene *m_scene` and calls `CommandUtils::findElements(m_scene, m_ids)`. Every `undo()` and `redo()` starts with `qCDebug(zero) << text()`. Two commands (ChangeInputSize/ChangeOutputSize) are near-clones (~180 lines).

**Impact:** ~150 lines of duplication; inconsistency risk if one command changes the ID-lookup pattern.

**Fix:** `ElementsCommand : public QUndoCommand` with `m_ids`, `m_scene`, `elements()`, `storeIds()`, and logging-wrapper `undoImpl()`/`redoImpl()`.

### 7. CodeGen has no shared base

ArduinoCodeGen and SystemVerilogCodeGen duplicate:
- `stripAccents()` — byte-for-byte identical
- `removeForbiddenChars()` — near-identical
- `otherPortNameImpl()` — same cycle-detection/port-following algorithm (~60 lines each)
- Gate type checking — same 8-type list repeated 4 times across both generators

**Impact:** ~200 lines of duplication; bug fixes must be applied in two places.

**Fix:** `BaseCodeGen` with shared utilities. Subclasses provide `defaultValue()`, `logicOperatorFor()`, and language-specific emission.

### 8. `MainWindow` does too much (1961 lines, ~60 slots)

Mixes: tab lifecycle, file open/save/export, undo/redo menu wiring, IC embed/extract, translations, theme, zoom, palette updates, 30+ action slots, BeWavedDolphin integration, autosave recovery, recent files, update checking.

**Impact:** Hard to find things; long compilation unit; any change risks touching unrelated code.

**Fix:** Low urgency since methods are well-named and organized in clear groups. If it grows further, extract: `TabController` (tab create/close/switch), `ICEmbedController` (embed/extract/self-contained), `ExportController` (Arduino/Verilog/image/PDF).

### 9. IC sub-circuit loading couples IC to the entire system

`IC::loadFile()` and `IC::loadFromBlob()` both go through `Serialization::deserialize()` then `processLoadedItems()`, which:
- Classifies each loaded element as input, output, or internal
- Creates proxy Node elements for boundary ports
- Deletes the original input/output elements
- Sorts and labels ports

`IC::mouseDoubleClickEvent()` directly calls `Application::instance()->mainWindow()->openICInTab()`, coupling Element/ to both Core/ and UI/.

**Impact:** IC can't be tested without a full Application + MainWindow stack.

**Fix:** The double-click should emit a signal (`requestOpenSubCircuit(blobName, elementId)`) caught by the Workspace or MainWindow. This is a one-method change.

### 10. Simulation graph is built twice — once for connections, once for wireless

`connectWirelessElements()` builds a `QHash<QString, GraphicElement*> txMap` by scanning all elements for Tx labels. Then `sortSimElements()` builds the **same txMap again** independently (lines 424-430) to add wireless edges to the successor graph.

**Impact:** Duplication (already in refactoring report #23). But also an architectural issue: the wireless dependency should be captured once during graph building, not re-derived during sorting.

**Fix:** `buildTxMap()` helper called once, result passed to both `connectWirelessElements()` and `sortSimElements()`.

### 11. MCP handlers access App layer through MainWindow — tight coupling

All 5 handlers hold `MainWindow*` and use it to get `Scene*` via `getCurrentScene()`. This means MCP can't work without a MainWindow instance. In headless/CLI mode (MCP's primary use case), a full UI stack must be constructed.

**Impact:** MCP testing requires constructing QApplication + MainWindow even in headless mode.

**Fix:** Handlers should depend on `Scene*` (or an `ICircuitAccess` interface) rather than `MainWindow*`. The MCPProcessor or factory can resolve the current scene at dispatch time.

### 12. HandlerFactory creates a new handler instance per command

`createHandler()` calls the factory lambda on every command, creating a fresh handler object. The handler is stateless — it just holds `MainWindow*` and `MCPValidator*` pointers. This means 25+ object allocations per second during heavy MCP traffic.

**Impact:** Minor performance concern; more importantly, it prevents handlers from caching per-session state.

**Fix:** Either cache handler instances per type (since they're stateless), or just store one instance per handler class.

---

## Subsystem Deep-Dive

### Serialization Flow

```
Save:  MainWindow → Workspace::save() → Serialization::writePandaHeader()
                                       → metadata (dolphin, rect, blobs)
                                       → Serialization::serialize(items)
                                         → element.save(stream) for each element
                                         → connection.save(stream) for each wire

Load:  MainWindow → Workspace::load() → Serialization::readPreamble()
                                       → deserializeBlobRegistry(metadata)
                                       → Serialization::deserialize(stream, context)
                                         → ElementFactory::buildElement(type)
                                         → element.load(stream, context)
                                         → QNEConnection::load(stream, context)
                                       → Scene::addItem() for each item
```

The `SerializationContext` carries `portMap` (maps serial IDs to live port pointers), `version`, `contextDir`, and `blobRegistry`. Port serial IDs are `(elementId << 16) | portIndex` — assumes max 65536 ports per element and max 2^48 element IDs.

Version compatibility spans V1.1 through V4.7 (~16 format versions). Legacy detection in `readPandaHeader()` handles: magic header (modern), "wiRedPanda X.Y" string header (mid-era), and headerless clipboard format (old).

### Simulation Lifecycle

```
Scene change → setCircuitUpdateRequired() → next timer tick:
  1. checkUpdateRequest() → initialize()
     a. Scan scene items → categorize into clocks/inputs/outputs/elements/connections
     b. initSimulationVectors() for each element (allocate input/output value arrays)
     c. buildConnectionGraph() — wire each element's inputs to predecessor outputs
     d. connectWirelessElements() — override Rx inputs with matching Tx outputs
     e. IC::initializeSimulation() — recursively build sub-circuit graphs
     f. sortSimElements() — topological sort with Tarjan SCC for feedback detection
  2. update() runs on 1ms QTimer:
     a. Update clocks (real-time frequency-driven)
     b. Update inputs (InputSwitch/Button state → output ports)
     c. Update logic (topologically sorted; iterative settling if feedback)
     d. Update visual ports (throttled to monitor refresh rate)
     e. Update output element visuals (LEDs, displays, buzzers)
```

IC simulation is nested: `IC::updateLogic()` runs the internal element graph to convergence, then copies boundary output values to the IC's external output ports.

### IC System

Three loading paths:
1. **File-backed:** `IC::loadFile()` → `ICRegistry::definition()` → `ICDefinition::fromFile()` reads metadata → `IC::loadFromDefinition()` → `IC::loadFromBlob()` deserializes elements
2. **Embedded (blob):** `IC::loadFromBlob()` directly deserializes from in-memory bytes
3. **Drop from palette:** `IC::loadFromDrop()` dispatches to `loadFile()` or `loadFromBlob()` based on whether a blob name is set

`ICRegistry` manages:
- Definition cache (`QMap<QString, ICDefinition>`) — shared metadata across instances
- Blob storage (`QMap<QString, QByteArray>`) — embedded IC .panda bytes
- File watcher (`QFileSystemWatcher`) — reloads IC instances on external change
- Cycle detection (`QSet<QString> m_loadingFiles`) — prevents A→B→A recursion
- Batch operations (`embedICsByFile`, `extractToFile`) with atomic rollback and undo

### MCP Bridge

```
stdin JSON-RPC → MCPProcessor::processCommand()
  → parse JSON, validate jsonrpc version
  → MCPValidator::validateCommand() (schema check)
  → Route: if-else chain matching method → handler.handleCommand()
    → Handler validates params, accesses Scene via MainWindow, executes operation
    → Returns QJsonObject response
  → MCPValidator::validateResponse() (schema check)
  → stdout JSON response
```

HandlerFactory registers 25 commands across 5 categories. Each command creates a fresh handler instance. The Python client (`mcp_infrastructure.py`) communicates via subprocess stdin/stdout, with optional schema validation using Pydantic models.

---

## Dependency Diagrams

### Current (verified)

```
    Core ←──────── UI (circular via Application.h → MainWindow.h)
     ↑              ↑
     │              │
   Nodes          Scene ←──── Element (bidirectional: 5 .cpp files include Scene.h)
     ↑            ↗ ↑ ↖         ↑
     │           /  │  \        │
  Simulation ──/   IO   \── BeWavedDolphin
                    ↑     \── CodeGen
                    │
                    └── MCP Server (via MainWindow*)
```

### Ideal (acyclic, layered)

```
    Core  (no UI dependency; forward-declare MainWindow)
     ↑
   Nodes  (depends only on Core/Enums)
     ↑
   Element  (depends on Core, Nodes, IO; uses ISceneContext interface instead of Scene)
     ↑
   IO  (depends on Core, Nodes, Element)
     ↑
   Scene  (depends on all above; forward-declares managers in header)
     ↑
   Simulation  (depends on Scene, Element, Nodes)
     ↑
   UI / BeWavedDolphin / CodeGen / MCP  (top-level consumers)
```

---

## Priority

1. **Fix Core → UI circular dependency** — forward-declare MainWindow in Application.h. One-line change, big conceptual win.
2. **IC double-click signal** — replace `MainWindow::openICInTab()` call with signal emission. Removes Element → UI coupling.
3. **Extract `gridSize` to Core** — removes one of Element's reasons to include Scene.h.
4. **Break Element → Scene with interface** — extract `ISceneContext`. Medium effort, enables independent Element testing.
5. **Reduce Scene.h transitive includes** — forward-declare infrequently-accessed managers. Improves compile times.
6. **MCP handlers depend on Scene, not MainWindow** — decouple MCP from UI stack.
7. **Add intermediate element base classes** — FlipFlop, DisplayElement, AudioOutput, ToggleInput. Highest code reduction (~500 lines), largest refactor.
8. **Commands base class** — ElementsCommand with ID management + logging. Straightforward, ~150 lines saved.
9. **CodeGen shared base** — BaseCodeGen with shared utilities. Straightforward, ~200 lines saved.
10. **Extract MainWindow responsibilities** — TabController, ICEmbedController. Low urgency.
