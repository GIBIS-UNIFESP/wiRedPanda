# wiRedPanda Developer Guide

A comprehensive guide for new developers joining the wiRedPanda project. This document walks you through the codebase architecture, the simulation engine, how to add new elements, how to write tests, and where to find things.

If you are looking for build instructions, see [BUILD.md](BUILD.md). For contribution workflow (PRs, code style, translations), see [CONTRIBUTING.md](CONTRIBUTING.md).

## Table of Contents

- [What is wiRedPanda?](#what-is-wiredpanda)
- [Getting Started](#getting-started)
- [Project Structure](#project-structure)
- [Architecture Overview](#architecture-overview)
- [Core Layer (App/Core)](#core-layer-appcore)
- [The Element System (App/Element)](#the-element-system-appelement)
  - [Class Hierarchy](#class-hierarchy)
  - [Element: The Base Class](#element-the-base-class)
  - [ElementFactory](#elementfactory)
  - [How Elements Register Themselves](#how-elements-register-themselves)
  - [ElementTypeDescriptor and ElementConstraints](#elementtypedescriptor-and-elementconstraints)
  - [Element Catalog](#element-catalog)
  - [Anatomy of a Logic Gate](#anatomy-of-a-logic-gate)
  - [Anatomy of a Sequential Element](#anatomy-of-a-sequential-element)
  - [Anatomy of an Input Element](#anatomy-of-an-input-element)
  - [Integrated Circuits (ICs)](#integrated-circuits-ics)
- [Four-State Logic](#four-state-logic)
- [The Simulation Engine (App/Simulation)](#the-simulation-engine-appsimulation)
  - [Simulation Loop](#simulation-loop)
  - [Topological Sorting](#topological-sorting)
  - [Feedback Loop Handling](#feedback-loop-handling)
- [Simulation Semantics (the spec)](#simulation-semantics-the-spec)
  - [Values and Time](#values-and-time)
  - [Wiring](#wiring)
  - [What Each Node Computes](#what-each-node-computes)
  - [Power-On](#power-on)
  - [Rounds](#rounds)
  - [Memory](#memory)
  - [Zero-Delay Loops](#zero-delay-loops)
  - [Pulses and Divergent Unknowns](#pulses-and-divergent-unknowns)
  - [Trips and the Budget](#trips-and-the-budget)
  - [Delays and Ties](#delays-and-ties)
  - [Clocks](#clocks)
  - [Ticks and Settles](#ticks-and-settles)
  - [ICs and the Reset Source](#ics-and-the-reset-source)
  - [Rebuilds, Mode Switches and Snapshots](#rebuilds-mode-switches-and-snapshots)
  - [What the Model Promises](#what-the-model-promises)
- [Ports and Connections (App/Wiring)](#ports-and-connections-appwiring)
- [The Scene and Undo/Redo System (App/Scene)](#the-scene-and-undoredo-system-appscene)
- [User Interface (App/UI)](#user-interface-appui)
- [File Serialization (App/IO)](#file-serialization-appio)
- [Code Generation (App/CodeGen)](#code-generation-appcodegen)
- [BeWavedDolphin (Waveform Viewer)](#bewaveddolphin-waveform-viewer)
- [Resources and Internationalization](#resources-and-internationalization)
- [The Test System](#the-test-system)
  - [Test Organization](#test-organization)
  - [Test Utilities and CircuitBuilder](#test-utilities-and-circuitbuilder)
  - [Writing Your First Test](#writing-your-first-test)
  - [Testing Sequential Logic](#testing-sequential-logic)
  - [Running Tests](#running-tests)
  - [Coverage and Sanitizers](#coverage-and-sanitizers)
- [CI/CD and Workflows](#cicd-and-workflows)
- [How To: Add a New Element](#how-to-add-a-new-element)
- [How To: Add a New Test](#how-to-add-a-new-test)
- [Git Workflow](#git-workflow)
- [Suggested Learning Path](#suggested-learning-path)
- [Starter Tasks](#starter-tasks)
- [Tips and Common Pitfalls](#tips-and-common-pitfalls)
- [Key Files Reference](#key-files-reference)
- [Glossary](#glossary)
- [Additional Resources](#additional-resources)

---

## What is wiRedPanda?

wiRedPanda is an open-source digital logic circuit simulator designed for education. Students can drag-and-drop logic gates, flip-flops, switches, LEDs, and other components onto a canvas, wire them together, and watch the simulation run in real time.

The project is maintained by [GIBIS-UNIFESP](https://github.com/GIBIS-UNIFESP) and licensed under **GPL v3.0**.

### Key Features

- **Real-time simulation** of combinational and sequential logic circuits
- **Intuitive GUI** based on Qt6 with drag-and-drop
- **Cross-platform**: Linux, macOS, Windows, and WebAssembly
- **Integrated Circuits (ICs)**: create reusable sub-circuits from `.panda` files
- **Export**: images, PDF, Arduino code, and SystemVerilog
- **Waveform viewer**: BeWavedDolphin tool for temporal analysis
- **Guided learning**: built-in UI Tours and step-by-step circuit Exercises, with a tour auto-started on first launch
- **39 languages** supported via Weblate

### Tech Stack

| Component       | Technology                           |
|-----------------|--------------------------------------|
| Language        | C++20                                |
| UI Framework    | Qt 6.8+                              |
| Build System    | CMake 3.27+ with Ninja               |
| Tests           | Qt Test Framework + CTest            |
| CI/CD           | GitHub Actions                       |
| Crash Reporting | Sentry (Crashpad/Breakpad)           |
| Dependencies    | nlohmann/json, json-schema-validator |
| i18n            | Qt Linguist / Weblate                |

---

## Getting Started

### Recommended: Development Container

The fastest way to set up a working environment is the dev container:

1. Install [Docker](https://www.docker.com/) and [VS Code](https://code.visualstudio.com/) with the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers).
2. Clone the repository and open it in VS Code.
3. When prompted, select **"Reopen in Container"** (or use `Ctrl+Shift+P` > `Dev Containers: Reopen in Container`).
4. Build and run:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The container includes:

- Ubuntu 24.04 LTS
- Qt 6.12.0 with Multimedia and SVG modules
- CMake, Ninja, GCC 13+
- ccache (compilation cache)
- mold (fast linker)
- lcov (code coverage)

### Local Setup

See [BUILD.md](BUILD.md) for platform-specific instructions. The short version:

#### Linux and macOS

Install Qt 6.12.0 with aqtinstall, as [BUILD.md](BUILD.md) shows: it is the Qt CI tests and the releases bundle, the
same whatever the OS release. A distribution's own Qt 6.8 or later works too (Debian 13, Ubuntu 26.04, Fedora, Arch
Linux). Then the tools:

```bash
# Debian and Ubuntu
sudo apt install build-essential zlib1g-dev cmake ninja-build ccache mold
# Arch Linux
sudo pacman -S base-devel cmake ninja ccache mold
# macOS
brew install cmake ninja ccache
```

#### Windows

1. Install [Qt 6.12.0](https://www.qt.io/download) with Multimedia and SVG modules.
2. Install [CMake](https://cmake.org/download/) and [Ninja](https://github.com/nicollasricas/ninja) (`choco install cmake ninja` or `scoop install cmake ninja`).
3. Configure Visual Studio Build Tools 2022 or later.

> **Note**: On Linux/macOS, always use `python3` (never `python`). The `python` command may not exist or point to Python 2.

### Running the Application

```bash
./build/wiredpanda                                 # Linux
./build/wiredpanda.app/Contents/MacOS/wiredpanda    # macOS
./build/wiredpanda.exe                              # Windows
```

### Running Tests (Quick Reference)

```bash
ctest --preset debug                         # Run all tests
./build/test_wiredpanda TestLogicGates       # Run a specific test class
./build/test_wiredpanda -functions           # List all test classes
```

---

## Project Structure

```text
wiRedPanda/
├── App/                                    # All application source code
│   ├── Main.cpp                            #   Entry point
│   ├── Core/                               #   Central infrastructure
│   │   ├── Application.cpp/h               #     Custom QApplication
│   │   ├── Enums.cpp/h                     #     Central enums (ElementType, Status, etc.)
│   │   ├── Settings.cpp/h                  #     Persistent settings management
│   │   ├── ThemeManager.cpp/h              #     Theme system (light/dark/system)
│   │   ├── Priorities.h                    #     Topological sorting and update priorities
│   │   ├── ItemWithId.h                    #     Base with stable numeric identity
│   │   └── StatusOps.h                     #     Four-state logic operations
│   ├── Element/                            #   Logic elements and element factory
│   │   ├── Element.cpp/h                   #     The element class, the IC its one subclass
│   │   ├── ElementFactory.cpp/h            #     Factory singleton
│   │   ├── ElementInfo.h                   #     Self-registration template
│   │   ├── ElementTypeDescriptor.cpp/h     #     Each type: kernel, parameters, ports, appearance
│   │   ├── Parameters.cpp/h                #     A type's parameters and their rules
│   │   ├── IC.cpp/h                        #     Integrated circuit (sub-circuit) element
│   │   ├── ICLoader.cpp/h                  #     Loads an IC's sub-circuit from file or embedded blob
│   │   ├── ICRenderer.cpp/h                #     Builds an IC's hover-preview pixmap
│   │   ├── ICSimulation.cpp/h              #     Builds/runs an IC's internal simulation graph
│   │   ├── ElementSerializer.cpp/h         #     Serialization
│   │   └── GraphicElements/                #     Each element type's descriptor, one file per type
│   ├── Presentation/                       #   How elements are drawn: presenters, pictures, labels
│   ├── Simulation/                         #   Simulation engine
│   │   ├── KernelLogic.cpp/h               #     Each kernel's logic
│   │   ├── Simulation.cpp/h                #     Main simulation loop
│   │   └── SimulationBlocker.cpp/h         #     Pause mechanism
│   ├── Wiring/                             #   Ports and wire connections
│   │   ├── Port.cpp/h                      #     Input/output ports
│   │   ├── Connection.cpp/h                #     Connections between ports
│   │   └── ConnectionSerializer.cpp/h      #     Connection save/load
│   ├── Scene/                              #   Graphics scene, undo/redo, workspace
│   │   ├── Scene.cpp/h                     #     Main QGraphicsScene
│   │   ├── Commands.cpp/h                  #     Undo/redo commands
│   │   ├── Workspace.cpp/h                 #     Workspace management
│   │   ├── ClipboardManager.cpp/h          #     Copy/paste
│   │   ├── ConnectionManager.cpp/h         #     Connection creation
│   │   └── ICRegistry.cpp/h                #     Central IC registry (moved here from App/Element/)
│   ├── IO/                                 #   File serialization
│   │   ├── Serialization.cpp/h             #     .panda format
│   │   └── RecentFiles.cpp/h               #     Recent files tracking
│   ├── UI/                                 #   MainWindow, dialogs, toolbars
│   │   ├── MainWindow.cpp/h                #     Main window
│   │   ├── ElementPalette.cpp/h            #     Element palette
│   │   ├── ElementEditor.cpp/h             #     Property editor
│   │   ├── CircuitExporter.cpp/h           #     Image/PDF export
│   │   ├── MinimapWidget.cpp/h             #     Draggable/resizable canvas overview widget
│   │   └── LanguageManager.cpp/h           #     Language management
│   ├── CodeGen/                            #   Arduino & SystemVerilog export
│   │   ├── ArduinoCodeGen.cpp/h            #     Arduino/C code generation
│   │   └── SystemVerilogCodeGen.cpp/h      #     SystemVerilog HDL
│   ├── BeWavedDolphin/                     #   Waveform viewer/editor
│   ├── Exercise/                           #   Circuit-challenge Exercise system
│   ├── Tour/                               #   Guided UI Tour system
│   └── Resources/                          #   SVGs, icons, translations
│       ├── Components/                     #     Element SVG icons by category
│       ├── Assets/                         #     App logos and icons
│       ├── Exercises/                      #     Built-in Exercise .json files
│       ├── Tours/                          #     Built-in Tour .json files
│       ├── Interface/                      #     UI resources
│       └── Translations/                   #     Translation files (.ts)
├── Tests/
│   ├── Common/                             #   Shared test utilities (TestUtils, CircuitBuilder)
│   ├── Unit/                               #   Unit tests by component area
│   ├── Integration/                        #   IC tests, code generation tests
│   ├── System/                             #   System-level tests (waveform)
│   ├── BackwardCompatibility/              #   Old file format test fixtures
│   ├── Build/                              #   Tests of the build's own checks (CMake scripts)
│   ├── Resources/                          #   Resource validation tests
│   ├── Fixtures/                           #   Test .panda files
│   ├── Fuzz/                               #   libFuzzer harnesses (local/manual, not run in CI)
│   └── Runners/                            #   Unified test entry point
├── Examples/                               #   21 example .panda circuit files
├── MCP/                                    #   Model Context Protocol server (internal use)
├── Packaging/                              #   Distro packaging (AUR PKGBUILD, etc.)
├── Scripts/                                #   Build and utility scripts
├── cmake/                                  #   Build checks run by CMake (the model's include check)
├── .devcontainer/                          #   Dev container configuration
├── .github/workflows/                      #   CI/CD pipelines
├── CMakeLists.txt                          #   Top-level build configuration
├── CMakeSources.cmake                      #   Alphabetically sorted source file lists
├── CMakePresets.json                       #   Build presets (debug, release, asan, etc.)
├── pch.h                                   #   Precompiled header
├── BUILD.md                                #   Platform build instructions
└── CONTRIBUTING.md                         #   Contribution guidelines
```

### Key Organizational Principles

- **One element type = one `.cpp` file** in `App/Element/GraphicElements/`, its descriptor.
- **Source file lists** live in `CMakeSources.cmake` (not `CMakeLists.txt`) and must stay **alphabetically sorted**.
  The model (`MODEL_SOURCES`, `MODEL_HEADERS`) and presentation (`PRESENTATION_SOURCES`, `PRESENTATION_HEADERS`) have
  lists of their own: no model file includes a presentation header, and `cmake/CheckModelIncludes.cmake` fails the
  build, before the library compiles, naming each file that does.
- **All builds go into `build/`** — never build in the source tree.

### Build Presets

The project uses **CMake Presets** (`CMakePresets.json`) to standardize build configurations:

```bash
# Configure
cmake --preset debug        # or: release, relwithdebinfo

# Build
cmake --build --preset debug

# Test
ctest --preset debug
```

| Preset            | Purpose                                                                                            |
|-------------------|----------------------------------------------------------------------------------------------------|
| `debug`           | Development with debug symbols                                                                     |
| `release`         | Optimized build for distribution                                                                   |
| `relwithdebinfo`  | Release with symbols (deploy with Sentry)                                                          |
| `coverage`        | Code coverage instrumentation                                                                      |
| `asan`            | Address Sanitizer (invalid memory access)                                                          |
| `tsan`            | Thread Sanitizer (data races)                                                                      |
| `msan`            | Memory Sanitizer (Clang only, uninitialized mem)                                                   |
| `ubsan`           | Undefined Behavior Sanitizer                                                                       |
| `macos-universal` | Release build with universal x86_64+arm64 macOS binaries (Darwin only)                             |
| `fuzzer`          | Clang libFuzzer harnesses + ASan + UBSan (see [Coverage and Sanitizers](#coverage-and-sanitizers)) |
| `fuzzer-coverage` | Source-based coverage report over the fuzz corpus, no sanitizers                                   |
| `sentry-crashpad` | RelWithDebInfo build with the Sentry crashpad backend                                              |
| `sentry-breakpad` | RelWithDebInfo build with the Sentry breakpad backend (Darwin only)                                |

### Incremental Builds and Cache

The project is configured for fast builds:

- **ccache**: caches previous compilations. Partial recompilations are much faster.
- **mold**: modern parallel linker, significantly faster than `ld` or `gold`.
- **Unity Build**: source files are grouped to reduce compilation overhead (configurable in CMake).
- **Precompiled Headers (PCH)**: `pch.h` precompiles the most-used Qt headers.

> **Tip**: the first build may take several minutes. Subsequent builds with ccache take seconds.

---

## Architecture Overview

The application follows a layered architecture:

```text
┌─────────────────────────────────────┐
│           UI Layer (App/UI/)        │  MainWindow, dialogs, toolbars
├─────────────────────────────────────┤
│        Scene Layer (App/Scene/)     │  Canvas, undo/redo, clipboard
├─────────────────────────────────────┤
│  Presentation (App/Presentation/)   │  How each type is drawn
├─────────────────────────────────────┤
│      Element Layer (App/Element/)   │  Element types: descriptors, parameters
├──────────────┬──────────────────────┤
│  Simulation  │  Wiring (ports and   │  Engine loop, kernels, topological
│(Simulation/) │  wires, Wiring/)     │  sort; port/wire data model
├──────────────┴──────────────────────┤
│        Core Layer (App/Core/)       │  Enums, StatusOps, Settings
└─────────────────────────────────────┘
```

**Data flows bottom-up**: The simulation engine reads element connections and calls `updateLogic()` on each element in topological order. Elements compute their outputs from their inputs using the four-state logic system. The scene layer renders the results.

### Module Dependency Diagram

```text
                    ┌──────────────┐
                    │   Main.cpp   │
                    └──────┬───────┘
                           │
                    ┌──────▼───────┐
                    │  Application │
                    │   (Core/)    │
                    └──────┬───────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
       ┌──────▼───────┐ ┌──▼──────┐ ┌───▼────────┐
       │  MainWindow  │ │  Scene  │ │ Simulation │
       │   (UI/)      │ │(Scene/) │ │(Simulation/│
       └──────┬───────┘ └──┬──────┘ └───┬────────┘
              │            │            │
              └────────────┼────────────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
       ┌──────▼──────┐ ┌───▼─────┐ ┌────▼───────┐
       │  Elements   │ │ Wiring  │ │  Commands  │
       │ (Element/)  │ │(Wiring/)│ │  (Scene/)  │
       └─────────────┘ └─────────┘ └────────────┘
              │
       ┌──────▼──────┐
       │Serialization│
       │   (IO/)     │
       └─────────────┘
```

The **Scene** is the central point: it holds the elements, the connections, and coordinates with the **Simulation**. The **MainWindow** is the UI shell wrapping the Scene. **Elements** are independent of the UI and can be tested in isolation.

---

## Core Layer (App/Core)

### Application

`App/Core/Application.cpp/h` is a `QApplication` subclass that initializes:

- OS theme detection
- Persistent settings (via `QSettings`)
- Language/translation management
- Global exception handling

### Enums and Central Types

`App/Core/Enums.h` defines the fundamental enums:

#### `Status` (4-state logic)

```cpp
enum class Status {
    Unknown  = -1,  // Floating / unresolved (equivalent to VHDL 'U')
    Inactive =  0,  // Logic LOW (0)
    Active   =  1,  // Logic HIGH (1)
    Error    =  2,  // Bus conflict / indeterminate (X)
};
```

This 4-state system (inspired by VHDL's `std_logic`) is central to the entire simulation. Every port and connection carries a `Status` value.

#### `ElementType`

Each element type has a unique numeric identifier. Examples:

- `And = 5`, `Or = 6`, `Not = 4`
- `DFlipFlop = 17`, `JKFlipFlop = 18`
- `Led = 3`, `Display7 = 14`
- `IC = 22` (integrated circuit)
- `Node = 23` (wire junction / Tx/Rx wireless)

#### `ElementGroup`

Groups elements in the UI palette:

- `Gate` (logic gates), `Memory` (flip-flops and latches)
- `Input`, `Output`, `StaticInput` (VCC/GND)
- `Mux`, `IC`, `Other`

#### `WirelessMode`

Wireless node modes: `None` (default), `Tx` (transmitter), or `Rx` (receiver).

### Settings

`App/Core/Settings.cpp/h` wraps Qt's `QSettings` to persist user preferences:

- Last opened directory
- Selected language
- Theme
- Recent files

### Themes

`App/Core/ThemeManager.cpp/h` manages the UI themes:

- **Light**: light theme
- **Dark**: dark theme
- **System**: automatically follows the OS theme

`ThemeManager` detects OS theme changes in real time by connecting to Qt's cross-platform `QStyleHints::colorSchemeChanged` signal, with a workaround for Qt before 6.10, which leaves a pending theme-change event unflushed where later Qt flushes it.

---

## The Element System (App/Element)

This is the most important subsystem of the project. All circuit components (logic gates, flip-flops, displays, etc.) are elements.

### Class Hierarchy

```text
QGraphicsObject
  └── Element                     (every circuit component, an element of its type)
        └── IC                    (sub-circuit / integrated circuit)
```

The inputs the user operates have no class of their own either: the element answers for their levels (`isOn()`, `setOn()`), which
their descriptors read and set (`sourceLevel`, `setLevel`), and the lock (`isLocked()`, `setLocked()`).

The gates, the flip-flops and the latches, the multiplexers (Mux, Demux) and the truth table, the LED, the 7-, 14- and 16-segment displays, the buzzer and the audio box, the constants (VCC and GND), the switch, the push button, the rotary switch, the clock, the node, the text and Line have no class of their own: each is an `Element` of its type, which its descriptor describes and, for a type with logic, `KernelLogic` computes.

### Element: The One Element Class

`App/Element/Element.h` is the class of **all** graphic elements in the circuit: an element of any type but the IC is
an `Element` built for its type (`new Element(ElementType::And)`), which its type's descriptor
(`ElementTypeDescriptor`) describes. It inherits from:

- `QGraphicsObject`: visual representation in the Qt scene
- `ItemWithId`: stable numeric identity (for serialization and undo/redo)

**Responsibilities:**

- Port management (input and output)
- Rendering with pixmaps/skins
- Serialization to QDataStream (save/load)
- Grid-snapping (alignment to grid)
- Label display and keyboard triggers
- Selection highlighting

**What a type changes comes from its descriptor, not from an override:** its logic is its kernel's
(`descriptor.kernel`, which `updateLogic()` runs through `KernelLogic::of()`), its ports' places are its
`inputAttributes`/`outputAttributes`, its settings are its parameters (`descriptor.parameters`, read with
`param<T>()` and set with `setParam()`), what a press does is its `pressOperation`, and how an older file's data
reads is its `readBeforeMap`/`readOlderKeys` hooks. The IC alone keeps a subclass, whose overrides
(`updateLogic()`, `save()`, `loadState()`, `saveSimState()`, `restoreSimState()` and others) run its internal
circuit.

### ElementFactory

`App/Element/ElementFactory.h` is a **singleton** that creates elements from their `ElementType`:

```cpp
// Create a new AND element
Element *andGate = ElementFactory::buildElement(ElementType::And);

// Check if a type is registered
bool exists = ElementFactory::hasCreator(ElementType::And);

// Type ↔ string conversion
ElementType type = ElementFactory::textToType("And");
QString name = ElementFactory::typeToText(ElementType::And);
```

The factory does not manage IDs — that is the `Scene`'s responsibility.

### How Elements Register Themselves

wiRedPanda uses a **self-registering template pattern**. Each element type specializes `ElementInfo<Type>`, keyed by its `ElementType`, which:

1. Defines compile-time `ElementConstraints` (input/output count, group, features).
2. Provides a `describe()` function returning its type's descriptor, with display info (icon path, translated name).
3. Contains a `static inline const bool registered` lambda that runs at program startup to add the type's descriptor (`ElementTypeDescriptor::add()`) and register its creator with `ElementFactory`.

This means: **no manual factory registration calls**. Just defining the `ElementInfo<Type>` specialization in the `.cpp` file is enough to make the element available everywhere — in the palette, in serialization, and in tests.

The `ElementConstraints` are **validated at compile time** by the `constexpr validate()` function. A misconfigured element will not compile.

### ElementTypeDescriptor and ElementConstraints

`ElementConstraints` defines the restrictions for an element:

| Field                 | Description                                                                     |
|-----------------------|---------------------------------------------------------------------------------|
| `type`                | Element type (`ElementType`)                                                    |
| `group`               | Palette group (`ElementGroup`)                                                  |
| `minInputSize`        | Minimum number of input ports                                                   |
| `maxInputSize`        | Maximum number of input ports                                                   |
| `minOutputSize`       | Minimum number of output ports                                                  |
| `maxOutputSize`       | Maximum number of output ports                                                  |
| `canChangeAppearance` | Allows the user to assign a custom SVG appearance (e.g., ANSI/IEC)              |
| `hasTrigger`          | Can be triggered via keyboard                                                   |
| `hasLabel`            | Displays a text label                                                           |
| `rotatesGraphic`      | Rotating/flipping re-orients the graphic (false = icon fixed, ports still move) |

What only some types have, such as a colour, a frequency, a delay, a sound, a volume or a wireless mode, is a parameter of the type (`ElementTypeDescriptor::parameters`), not a field or an accessor of the element: it is asked with `offersParam(ParamId::Colour)`, read with `param<QString>(ParamId::Colour)` and set with `setParam(ParamId::Colour, value)`; a type without it has none, and asking for it is a broken invariant. `ElementColour` cycles the colour names and `WirelessNode` reads a node's mode as its enum.

`ElementTypeDescriptor` (`App/Element/ElementTypeDescriptor.h`) is everything an element of the type is, built from
the constraints by `descriptorFromConstraints()` and completed in `describe()`:

- **Display**: the picture (`pixmapPath`), the palette title and name, and their translation context.
- **Simulation**: the kernel (`kernel`, the parameters it reads, `kernelParameters`), the history it keeps from one
  update to the next at power-on (`kernelState`), and for an input the level it drives (`sourceLevel`, `setLevel`).
- **Parameters**: the type's schema (`parameters`), one `ParamSpec` per setting, with its key in a file, its default
  and its accept rule, which the editor, files, MCP and the setters all go through.
- **Operations**: what a press and its release do (`pressOperation`, `releaseOperation`).
- **Ports**: each port's place and visibility (`inputAttributes`, `outputAttributes`) and how the inputs follow the
  outputs (`inputsForOutputs`).
- **Files**: hooks reading an older file's data (`readBeforeMap`, `readOlderKeys`) and checking a loaded one
  (`checkLoad`, `reportFailedFiles`).
- **Appearance**: its `appearanceSlots`, each slot's key, which a file saves the slot's picture by, and its built-in
  picture, which an element's appearance list starts as; the slot shown (`shownSlot`) and the states the user can
  give a picture to (`appearanceStates`). `appearancePositions` keeps the slot each position of an older file's
  positional list named.

### Element Catalog

The 31 element types are each described in their own file in `App/Element/GraphicElements/`:

#### Logic Gates (combinational)

| Element | File       | Inputs | Description                  |
|---------|------------|--------|------------------------------|
| AND     | `And.cpp`  | 2-8    | Output 1 if all inputs are 1 |
| OR      | `Or.cpp`   | 2-8    | Output 1 if any input is 1   |
| NOT     | `Not.cpp`  | 1      | Inverts the signal           |
| NAND    | `Nand.cpp` | 2-8    | Inverted AND                 |
| NOR     | `Nor.cpp`  | 2-8    | Inverted OR                  |
| XOR     | `Xor.cpp`  | 2-8    | Exclusive OR                 |
| XNOR    | `Xnor.cpp` | 2-8    | Inverted exclusive OR        |

#### Sequential Elements (memory)

| Element      | File             | Description                        |
|--------------|------------------|------------------------------------|
| D Flip-Flop  | `DFlipFlop.cpp`  | Stores data on clock edge          |
| JK Flip-Flop | `JKFlipFlop.cpp` | Versatile flip-flop with J, K, clk |
| SR Flip-Flop | `SRFlipFlop.cpp` | Set-Reset with clock edge          |
| T Flip-Flop  | `TFlipFlop.cpp`  | Toggle on clock edge               |
| D Latch      | `DLatch.cpp`     | Level-sensitive latch              |
| SR Latch     | `SRLatch.cpp`    | Level-sensitive set-reset          |

#### Inputs

| Element     | File              | Description                             |
|-------------|-------------------|-----------------------------------------|
| InputButton | `InputButton.cpp` | Momentary button (active while pressed) |
| InputSwitch | `InputSwitch.cpp` | Toggle switch (on/off)                  |
| InputRotary | `InputRotary.cpp` | Rotary switch with N positions          |
| InputVCC    | `InputVCC.cpp`    | Constant HIGH source (1)                |
| InputGND    | `InputGND.cpp`    | Constant LOW source (0)                 |

#### Outputs

| Element   | File            | Description                         |
|-----------|-----------------|-------------------------------------|
| Led       | `Led.cpp`       | LED indicator (with color option)   |
| Display7  | `Display7.cpp`  | 7-segment display                   |
| Display14 | `Display14.cpp` | 14-segment display (alphanumeric)   |
| Display16 | `Display16.cpp` | 16-segment display                  |
| Buzzer    | `Buzzer.cpp`    | Audio buzzer with configurable freq |
| AudioBox  | `AudioBox.cpp`  | Audio player                        |

#### Multiplexing

| Element | File        | Description                           |
|---------|-------------|---------------------------------------|
| Mux     | `Mux.cpp`   | Multiplexer: selects 1 of N inputs    |
| Demux   | `Demux.cpp` | Demultiplexer: routes input to 1 of N |

#### Utilities

| Element    | File             | Description                           |
|------------|------------------|---------------------------------------|
| Clock      | `Clock.cpp`      | Clock generator with adjustable freq  |
| Node       | `Node.cpp`       | Wire junction / wireless node (Tx/Rx) |
| Line       | `Line.cpp`       | Decorative line (no logic function)   |
| Text       | `Text.cpp`       | Text annotation                       |
| TruthTable | `TruthTable.cpp` | Element defined by a truth table      |

### Anatomy of a Logic Gate

Here is the complete AND gate, the simplest element to understand. A gate has no class of its own: it is an
`Element` whose type's descriptor names its kernel, and the kernel's logic lives in `KernelLogic`.

**`App/Element/GraphicElements/And.cpp`**:

```cpp
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

// Registration: the constraints and the descriptor, keyed by the type
template<>
struct ElementInfo<ElementType::And> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::And,
        .group = ElementGroup::Gate,
        .minInputSize = 2, .maxInputSize = 8,
        .minOutputSize = 1, .maxOutputSize = 1,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe() {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::And;
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/and.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("And", "AND");
        descriptor.translatedName = QT_TRANSLATE_NOOP("And", "And");
        descriptor.trContext = "And";
        descriptor.appearanceSlots = {{"body", ":/Components/Logic/and.svg"}};
        return descriptor;
    }

    static inline const bool registered = [] {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
```

**`App/Simulation/KernelLogic.cpp`** (the AND kernel's logic, and its entry in `KernelLogic::of()`):

```cpp
void andGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(StatusOps::statusAndAll(element.simInputs()));
}

KernelLogic::Logic KernelLogic::of(const KernelKind kind)
{
    switch (kind) {
    case KernelKind::And:  return &KernelLogicDetail::andGate;
    // ...
    }
}
```

**What happens in `Element::updateLogic()`:** it runs `KernelLogic::of(descriptor.kernel)`, here `andGate()`:

1. `simUpdateInputsAllowUnknown()` reads the current value from each connected predecessor's output port (or uses the port's default if unconnected). Returns `false` if a required input has no value — the element does nothing this cycle.
2. `simInputs()` returns a `QVector<Status>` snapshot of all input values.
3. `StatusOps::statusAndAll()` ANDs all inputs together using the four-state logic rules.
4. `setOutputValue()` writes the result to output port 0. If the value changed, the element is flagged for downstream propagation.

All other combinational gates (`Or`, `Not`, `Xor`, `Nand`, `Nor`, `Xnor`) follow this exact same pattern: a descriptor naming the kernel, and the kernel's function in `KernelLogic`. The only difference is the `StatusOps` function called.

### Anatomy of a Sequential Element

Sequential elements like flip-flops store internal state and respond to clock edges. Like a gate, a flip-flop is an
`Element` of its type: its descriptor gives its ports (`inputAttributes`/`outputAttributes`) and the history its kernel
keeps (`kernelState`), and its logic is its kernel's, in `KernelLogic`.

**D Flip-Flop** (`DFlipFlop.cpp`'s descriptor, and `KernelLogic.cpp`'s kernel, simplified):

```cpp
// In the descriptor: the kernel's history at power-on, its last clock and its last D
descriptor.kernelState = {Status::Inactive, Status::Active};

// In KernelLogic.cpp
void dFlipFlop(Element &element)
{
    QVector<Status> &kept = element.kernelState();     // last clock, last D
    if (!element.simUpdateInputs()) {                   // Strict: needs ALL inputs valid
        kept[0] = Status::Unknown;                      // No edge from stale state on recovery
        return;
    }
    Status q0 = element.simOutputs().at(0);             // Current Q
    Status q1 = element.simOutputs().at(1);             // Current ~Q
    const Status d = element.simInputs().at(0);         // Data input
    const Status clk = element.simInputs().at(1);       // Clock input

    // Rising edge: latch the D sampled at the previous evaluation
    if ((clk == Status::Active) && (kept[0] == Status::Inactive)) {
        q0 = kept[1];
        q1 = (kept[1] == Status::Active) ? Status::Inactive : Status::Active;
    }
    // Asynchronous overrides, ~Preset and ~Clear (active-low, jointly evaluated)
    applyOverrides(element.simInputs().at(2), element.simInputs().at(3), q0, q1);
    kept[0] = clk;
    kept[1] = d;
    element.setOutputValue(0, q0);
    element.setOutputValue(1, q1);
}
```

Key differences from combinational gates:

- Uses `simUpdateInputs()` (strict) instead of `simUpdateInputsAllowUnknown()`.
- Keeps its last clock and data inputs across cycles for edge detection, in `Element::kernelState()`, which the base
  `resetSimState()`, `saveSimState()` and `restoreSimState()` reset, save and restore after the outputs.
- Has its own port layout, from its descriptor's attributes (Clock pin at bottom-left, Preset at top, etc.).

### Anatomy of an Input Element

Input elements like `InputSwitch` are driven by user interaction, not by upstream logic. What a press and its release
do is an operation in the type's descriptor, which sets the element's parameters:

```cpp
// InputSwitch's descriptor: a press toggles the level; a switch has no release operation
descriptor.pressOperation = [](Element &element) {
    element.setParam(ParamId::On, !element.param<bool>(ParamId::On));
};

// InputButton's: momentary, down on a press and up on its release
descriptor.pressOperation = [](Element &element) { element.setParam(ParamId::On, true); };
descriptor.releaseOperation = [](Element &element) { element.setParam(ParamId::On, false); };
```

No input handles the mouse itself. The scene dispatches the operations (`Scene::mousePressEvent()`,
`mouseReleaseEvent()`, `mouseDoubleClickEvent()`, `keyPressEvent()`, `keyReleaseEvent()`): a left press to the unlocked
element that took it, running the operation its presenter's `activationAt()` picks for the point; a left release to the
unlocked element holding the mouse; a trigger key to every unlocked element it names. `Element::operate()` runs an
operation and ignores the lock, which the scene checks. Each goes through `Scene::recordOperation()`, which records
what the operation changed in the element's saved state as one `UpdateCommand`, so a toggle is undoable and marks the
tab modified, while a held button, which saves no level, records nothing.

The `Clock` element is special: it is driven by wall-clock time via `ClockTiming::update()`, comparing the time elapsed since its phase (the element's simulation state) against the half-period of its frequency.

### Integrated Circuits (ICs)

The IC system allows reusing circuits as sub-components, split across several focused classes:

- **`IC.cpp/h`** (`App/Element/`): the element representing an IC in the scene (a "black box" with ports)
- **`ICLoader.cpp/h`** (`App/Element/`): loads an IC's sub-circuit from an external `.panda` file or an embedded blob
- **`ICRenderer.cpp/h`** (`App/Element/`): builds the IC's hover-preview pixmap; its presenter (`App/Presentation/FootprintPresenters.cpp`) draws the body
- **`ICSimulation.cpp/h`** (`App/Element/`): builds and runs the IC's internal simulation graph
- **`ICPreviewPopup.cpp/h`** (`App/Element/`): frameless popup showing a preview of an IC's internal circuit on hover
- **`ICRegistry.cpp/h`** (`App/Scene/`): central registry that monitors `.panda` files and auto-updates ICs when the file changes
- **`ICDefinition.cpp/h`** (`App/Element/`): a parsed `.panda` definition, shared by every IC that loads the same bytes; once no IC holds it, it is kept under a byte budget, the least recently released going first
- **`ICPreviewCache.cpp/h`** (`App/Element/`): the hover previews, one per definition and directory, LRU over 16 entries, dropped on a theme change
- **`PinOrder.cpp/h`** (`App/Element/`): the order of an IC's pins, as a file saves it

An IC's pins take their names when asked, so a pin shows the current interface language; the saved header keeps each pin's source name. A pin's order is the file's saved order, and a pin the order does not name takes its place by position. Every element snaps to the half grid on load, after the pin order has been taken, since a snap can move two close pins past one another.

ICs support **arbitrary hierarchy** — an IC can contain other ICs inside it. The tests include up to 9 levels of complexity, culminating in complete CPU implementations.

---

## Four-State Logic

All signals in wiRedPanda use four states, defined in `App/Core/Enums.h`:

| State      | Value | Meaning                      | Visual Color                 |
|------------|-------|------------------------------|------------------------------|
| `Unknown`  | -1    | Not yet resolved / floating  | Gray                         |
| `Inactive` | 0     | Logic LOW                    | Dark green (theme-dependent) |
| `Active`   | 1     | Logic HIGH                   | Green (theme-dependent)      |
| `Error`    | 2     | Bus conflict / indeterminate | Red                          |

`App/Core/StatusOps.h` provides logic operations that respect these states with **domination rules**:

- **AND**: `Inactive` dominates — `AND(0, Unknown) = 0` because any false input makes the AND false.
- **OR**: `Active` dominates — `OR(1, Unknown) = 1` because any true input makes the OR true.
- **NOT**: Inverts `Active`/`Inactive`; passes `Unknown` and `Error` through unchanged.
- **XOR**: Requires all inputs to be known; any `Unknown` yields `Unknown`.

This matters for simulation correctness: a combinational gate can produce a determinate output even when some inputs are unconnected or unknown, which matches real-world digital logic behavior.

### Strict vs. Permissive Input Updates

Elements choose which input-reading strategy to use:

- **`simUpdateInputsAllowUnknown()`** (permissive) — Used by combinational gates. Allows `Unknown`/`Error` through so domination rules can resolve them. Only fails if a port is truly unconnected with no default.
- **`simUpdateInputs()`** (strict) — Used by sequential elements. Any `Unknown` or `Error` input causes the element to propagate `Unknown` to all outputs and bail out. Sequential elements need complete, valid data.

---

## The Simulation Engine (App/Simulation)

The simulation engine is the heart of wiRedPanda. It determines how signals propagate through the circuit.

### Simulation Loop

The simulation (`App/Simulation/Simulation.cpp`) runs on a **1ms QTimer** tick. Each tick executes these phases in order:

```mermaid
flowchart TD
    timer(["QTimer (1 ms)"])
    step1["1. updateSourceOutputs()
    Update inputs: clocks, buttons, switches"]
    step2["2. updateLogic()
    Process elements in topological order:
    gates (zero-delay), flip-flops (edge)"]
    feedback{"Feedback loop?"}
    run_logic["Run updateLogic()
    on feedback elements"]
    settled{"Settled or
    iter ≥ 10?"}
    step3["3. updatePort()
    Propagate values via connections (out→in)"]
    step4["4. Update outputs
    LEDs, displays, buzzers redraw;
    the audio service plays or stops"]
    wait(["Wait for next tick (1 ms)"])

    timer --> step1 --> step2 --> feedback
    feedback -- Yes --> run_logic --> settled
    settled -- No --> run_logic
    settled -- Yes --> step3
    feedback -- No --> step3
    step3 --> step4 --> wait
```

### Topological Sorting

Before simulation starts, the engine builds a dependency graph from wire connections and computes a **topological sort** (`App/Core/Priorities.h`):

- Each element is assigned a **priority** = length of the longest path from it to the nearest sink (output).
- Elements with higher priority are evaluated first (sources before sinks).
- This guarantees that when an element's `updateLogic()` runs, all its predecessors have already been updated this cycle.

```text
Input → AND → OR → LED
  1st    2nd   3rd   4th
```

This is done during the `initialize()` phase, which builds the dependency graph from the scene's connections.

### Simulation Wiring

During simulation initialization, physical wire connections are converted into a lightweight predecessor map:

```cpp
struct SimInputConnection {
    Element *sourceElement = nullptr;
    int sourceOutputIndex = 0;
};
```

Each element stores a vector of these for its inputs. During `updateLogic()`, reading an input is just a direct pointer dereference — no graph traversal needed per tick.

### Feedback Loop Handling

Some circuits have feedback (e.g., an SR latch where outputs feed back to inputs). The engine detects these using **Tarjan's strongly connected components** algorithm:

1. Any element in a cycle is marked as a **feedback node** — including a single element whose own output feeds directly back into its own input (a size-1 strongly connected component via self-loop).
2. Instead of a single-pass evaluation, the engine uses **iterative settling**: it runs `updateLogic()` on all elements, checks if any output changed, and repeats — up to 10 iterations or until convergence.
3. If the circuit does not converge (e.g., a ring oscillator), a warning is emitted via the status bar.

---

## Simulation Semantics (the spec)

This section states the rules the simulation follows, in both modes. Functional mode is the same rules with every delay
zero and the clocks on wall time. It is the spec that the reference simulator in `Tests/Common` will implement and that
the oracles will check. The reference does not exist yet, and the engine in `App/Simulation`, described above, does not
follow these rules yet. The causes file, which lists each trace where the two differ, is written with the tests. A rule
changes here first, then in the oracles and the reference, and every trace a change moves is named by its cause.

### Values and Time

- Four values, one byte each: Inactive, Active, Unknown and Error. Unknown is below the two levels and Error. Error and
  the levels are unordered among themselves (the flat order), so an input lowered to Unknown may lower an output to
  Unknown, but never move it to another value.
- Temporal mode simulates nanoseconds. Functional mode runs the clocks and the user's changes on wall time. An instant
  is one time, settled in rounds.

### Wiring

- One node per element that computes: logic, memory, sources and clocks. A junction, an IC pin and a wireless Tx or Rx
  alias the slot that drives them, inside loops too, and cost no time. ICs are flattened into their nodes.
- An input nothing drives reads its port's default, and a combinational node with such an input whose default is Unknown
  outputs Unknown. A loop of aliases with no node in it reads Unknown.
- A net with more than one driver reads Error and warns once: two wires into one input, a label on more than one Tx in a
  scope, or an Rx that a wire also reaches.
- Wireless labels resolve once per scope (the top level and each IC instance), after the IC boundaries are mapped, so an
  Rx reads its scope's Tx's driver.
- A node with outputs has at most 11 inputs (a Mux's 8 data and 3 select lines), and a memory node at most 5.

### What Each Node Computes

- Every kernel and every memory sample reads an Unknown or Error input both as Inactive and as Active, and outputs what
  every reading agrees on. Where the readings disagree the output is Unknown if a reading is Unknown or an Unknown input
  decides it, and Error otherwise, since IEEE 1164 ranks Unknown first.
- So every kernel is monotone in the flat order, with one exception: a flip-flop's clock held at Unknown has no edge.
- A flip-flop stores on a rising edge, meaning its clock has gone from Inactive to Active since it last sampled. A clock
  that kept its value since then, Unknown both times included, does not rise, and one cause taking a clock from Active
  to Unknown makes no edge, since no reading of it rises. Preset and clear are active low and override the data, and
  latches are level-sensitive. So a falling clock with Unknown data holds, an asserted preset sets Q whatever the data,
  and a JK flip-flop with J Unknown, K low and Q high stays high.

### Power-On

- Sources sit at their levels, clocks start HIGH shifted by their phase, memory starts at its power-on state (Q low, ~Q
  high), and combinational outputs are Unknown until their inputs decide them. A reset source is Active through the
  power-on instant (see ICs and the Reset Source).
- A node whose outputs change with no changing input, as at power-on, is the cause of that change.
- The power-on settle cannot oscillate: a monotone gate changes at most once from Unknown.

### Rounds

- A change's cause is the node that made it, and changes with different causes happen in any order. An instant runs
  rounds: the due publications apply, each changed slot with its publisher as its cause; the combinational nodes that
  read a change run in topological order; the memory nodes whose inputs changed sample once their round's logic has
  settled; zero-delay loops settle; memory nodes and loops publish after the round, which starts the next round; and the
  instant ends when a round publishes nothing.
- A node whose changing inputs follow several causes is read over every order of them. Where every order takes its
  output once from the old value to the new, that is an event of its own; where every order keeps it, it does not
  change; otherwise (a third value, or the old value after the new) it may pulse.
- All memory nodes read before any publishes, so a shift register shifts one stage per edge, and a ripple counter
  ripples one stage per round within one instant.
- Nothing is canonicalised. The topological order only orders evaluation within a round, and nothing observable depends
  on it or on where an element sits on the canvas. A ring oscillator or a raced latch reads Unknown because its phase or
  its outcome is.
- Independent changes that meet in one round read Unknown where they reach memory or a loop, which is the round barrier:
  a cause that reaches a memory node or a loop through another one comes after the direct causes. Settling each cause
  fully before the next would read Unknown wherever a change through memory could arrive first, and would assume a skew
  between inputs. Outside a zero-delay loop, a hazard that only some delays produce does not pulse. Inside one, ternary
  iteration reads it Unknown where it can latch.

### Memory

- A memory node stores what every order of its input changes in the round agrees on, and Unknown where they disagree. An
  input that may pulse takes every value at every step, after the last event too. A clock that may pulse has not kept its
  value.
- So a flip-flop whose data changes in its edge's round stores Unknown where the old and the new data differ. The same
  holds for a latch closing as its data changes, a preset or clear released at an edge, and both released together.
- A fresh memory node, new at power-on or at a rebuild, samples once a round publishes nothing, from its last control
  levels taken from its inputs. Preset, clear and an enabled latch apply, and nothing fabricates an edge.

### Zero-Delay Loops

- A zero-delay loop, a set of combinational nodes that read each other, settles by ternary iteration. After each event it
  runs Eichelberger's algorithm A (the changing inputs read Unknown, iterated to a fixed point), then B (their new
  values). An input that may pulse takes every value at every step, and B runs with the pulses' actual values once every
  event has happened.
- Every order of the events is followed. Up to five events, that means enumerating the orders. Past five, a search over
  subsets of the events keeps the distinct states each reaches, within a work limit of 2^18 member evaluations, which is
  named apart from the round budget. Past that limit the orders are followed again with A alone between events, within
  a limit of its own. Past that, or past four inputs that move at will (3^4 combinations), a fallback applies in which
  every changed input reads Unknown in A, and what A makes Unknown pulses. Each settle past the work limit is counted.
- A ternary loop cannot oscillate: a ring that cannot settle settles at Unknown.

### Pulses and Divergent Unknowns

- A loop, and a memory node without delay, follows each order's path of every output. An output that some order moves
  twice, or that moves while the merge keeps its published Unknown, publishes as a pulse after the round. A delay swallows
  a pulse, as it does a gate's.
- A pulse moves at will at every step, after the last event too. Memory and loops take every value of a pulsing input.
- An Unknown merged within an instant from orders, branches or runs that disagree is divergent: it stands for one value
  per order until the instant ends. Loops set it, since ternary iteration does not tell a race from an Unknown input,
  and so do memory nodes, gates and the temporal tie merge (see Delays and Ties). Memory and loops take each of its
  values for a whole branch, and gates read a divergent input that did not change at each of its levels. An output that
  stays Unknown while its divergent mark turns on or off moved in some order, so it publishes as a pulse. A delayed
  publication lands in a later instant as a plain value.
- An event is a change of the four-state value, as in Verilog, where a net that stays `x` has no edge. A slot that stays
  Unknown while its inputs change wakes nothing, and from one instant to the next an Unknown is a value.

### Trips and the Budget

- When an instant's rounds repeat a state (memory clocking itself), the memory nodes that cycled store Unknown, and the
  user is warned once ("oscillation"). The state includes the pulses the round's publications made, and a memory node
  that published a pulse counts as cycled. A round that repeats an earlier state does not read its pulses again, and a
  pulse that changed its slot's value becomes that change, an event of its publisher.
- A budget of 1,000 rounds stops an instant, with a warning of its own. A trip never makes memory sample a cone that has
  not settled.
- No fixture, example or IC component reaches the budget or trips unless it is meant to, so a budget set too small shows
  in the test suite.

### Delays and Ties

- A node with a delay evaluates at its inputs' new values and schedules its result at `t + d`, superseding any
  publication it has pending (inertial delay). A loop with a delay in it runs in simulated time and may oscillate. That
  is behaviour, not a trip.
- A delayed node whose publication falls due in an instant's first round, at the time one of its inputs changes from
  another cause, publishes the merge of its old value and the due one: Unknown, or a pulse where the old value was
  Unknown. Its readers run again. A tie that the first round's pass finds undoes that pass, which runs again from the
  round's start with the merge.
- A covering rule applies: a memory node whose clock reaches Unknown through a divergent Unknown, one level in each
  order, may have seen an edge, so it stores Unknown rather than holding. The rule that a clock taken from a level to
  Unknown by one cause holds stays, for an Unknown that is no level. The exact oracle with delays shows the covering rule
  is needed.

### Clocks

- Functional mode, on wall time: an edge comes once more than a half period has passed since the clock's reference, which
  then advances by exactly a half period, so drift does not skew the frequency. A tick runs every edge due since the last
  one, each at its time.
- Temporal mode: a clock's edges are events at `phase + k · half period` of simulated time, for k of at least 1. The speed
  only maps simulated time to wall time, so it changes nothing a clock does.
- A locked clock stops. Unlocked, it shows the level its schedule gives, with missed edges dropped rather than replayed:
  at the rebuild that unlocks it, the grid edges since its stored reference are counted, the reference advances by that
  many half periods, and the output flips if the count is odd. From then on it matches a never-locked clock at every tick,
  with no state of its own while locked.
- A settle holds the temporal clocks for the simulated time it took, a locked one included. A lock changes what a clock
  shows, not where its edges fall.

### Ticks and Settles

- A functional tick runs everything due since the last tick in time order, with each distinct time an instant of its own:
  each clock edge at its due time, each user change at the time it was made. A temporal tick runs the user's changes
  first, as an instant of their own, then drains a window of simulated time. A temporal start with no change due runs no
  instant of its own.
- Both check one wall-time deadline between instants. A tick cut by it stops time at the next instant due, and the next
  tick goes on from there, so what remains runs later, in the same order, and nothing a tick shows is invented.
- A timed change waits for its time. An untimed one is seeded at the start of the tick that sees it, so the runner stamps
  each user change with the time it was made. A parameter or structural edit takes effect at the next tick's first
  instant. Only the changed sources are seeded (every input after a rebuild). A source seeded back to its published level
  drops the publication it had pending, and a tick with nothing due does nothing.
- A settle runs instants while one is due, each at the earliest time something is due, within a budget of instants. It
  seeds no clock and holds the temporal clocks.

### ICs and the Reset Source

- An unconnected IC input reads the level that the input element it stands for has inside the IC: a switch's, a button's
  or a rotary's. A clock pin, which is required, reads Unknown.
- A reset source is Active through the power-on instant in functional mode, and is released at time 1: the second tick,
  or a settle that runs the power-on instant. In temporal mode it is Active for its width (1,000 ns by default), and its
  release is an instant of its own. A reset that survives a rebuild keeps its state. A reset that a rebuild adds reads
  Inactive at once.

### Rebuilds, Mode Switches and Snapshots

- A rebuild keeps the outputs and memory state of the nodes that survive it, and fabricates no edge. A node whose element
  keeps its id and output count keeps its published outputs whatever its inputs did, so an edit that changes no output
  changes none: an And that loses an input keeps its value. A memory node keeps its stored state, last control levels and
  clock phase when its kind and port counts are unchanged, and otherwise starts fresh (Q low, control levels from the
  published inputs). A new combinational node starts at its result against the published values, not at its power-on
  Unknown. A new element, or one morphed to another type, is a new node, and loading a file is power-on.
- An IC's nodes survive a rebuild when the IC keeps the definitions it was built from. Each node is placed by its top-level
  element and its index path through the ICs nested in it.
- A switch between functional and temporal mode keeps every value, Unknown included, and seeds nothing.
- A snapshot holds the run's state with its publications in flight, so a sweep resumes a live run exactly.

### What the Model Promises

The rules trade precision for correctness. A value that every order agrees on can still read Unknown where two changes
reach memory or a loop through a merge, because the rounds do not canonicalise the order of causes. The model promises
that a value it reads as a level is the value every order of the changes gives, and it does not promise a level wherever
the orders agree. Unknown is the honest answer where they do not.

---

## Ports and Connections (App/Wiring)

### Port Architecture

- **`OutputPort`** — The source end of a wire. Can fan out to multiple inputs.
- **`InputPort`** — The destination end. Accepts at most one connection (unless optional).
- Both inherit from `Port`, which is a `QGraphicsItem` positioned at the element's edge.

Each port carries a `Status` (the current logic value) and maintains a list of connections.

### Connection

`App/Wiring/Connection.h` represents a **wire** between two ports. A connection links exactly one `OutputPort` to one `InputPort`; `App/Wiring/ConnectionSerializer.h` handles its save/load.

Properties:

- Rendered as a Bézier curve on the QGraphicsScene
- Color reflects the signal's logic state (dark green=0, green=1, gray=unknown, red=error) — exact shades are theme-dependent
- Magnetic snap to nearby ports during creation

### Connection Flow

1. User drags from an output port.
2. A `Connection` (wire) is created and follows the cursor.
3. On release over a valid input port, `ConnectionManager` validates the connection (no self-loops, no duplicates, no wireless conflicts).
4. If valid, an `AddItemsCommand` is pushed to the undo stack, committing the wire.

---

## The Scene and Undo/Redo System (App/Scene)

### Scene (`App/Scene/Scene.cpp`)

The `Scene` extends `QGraphicsScene` and owns:

- A **`SceneItemRegistry`** mapping integer IDs to elements (for stable undo/redo references via `ItemWithId`).
- The **`Simulation`** instance.
- A **`QUndoStack`** for undo/redo.
- Managers for connections, clipboard, visibility, and property shortcuts.
- A **`SceneDropHandler`** that decodes drag-and-drop payloads onto the canvas.
- Grid-snapping (16px grid, 8px snap). Selected elements can also be nudged with the arrow keys — one grid cell per press, or four cells with Shift held.
- **`SceneInteraction`**, which handles mouse event processing for drag, selection, and connection creation.

### Undo/Redo Commands (`App/Scene/Commands.h`)

All user operations are `QUndoCommand` subclasses:

| Command                         | What It Does                                        |
|---------------------------------|-----------------------------------------------------|
| `AddItemsCommand`               | Add elements and wires to the scene                 |
| `DeleteItemsCommand`            | Remove selected items                               |
| `MoveCommand`                   | Drag elements to new positions                      |
| `RotateCommand`                 | Rotate elements                                     |
| `FlipCommand`                   | Flip horizontally or vertically                     |
| `UpdateCommand`                 | Change element properties (label, color, frequency) |
| `MorphCommand`                  | Change element type while preserving connections    |
| `ChangePortSizeCommand`         | Add/remove input or output ports                    |
| `SplitCommand`                  | Insert a junction node into a wire                  |
| `ToggleTruthTableOutputCommand` | Toggle a truth table output bit                     |
| `RegisterBlobCommand`           | Register a new blob resource                        |
| `RemoveBlobCommand`             | Remove a blob resource                              |
| `RenameBlobCommand`             | Rename a blob resource                              |
| `UpdateBlobCommand`             | Update an existing blob resource                    |

Each command serializes enough state to fully undo and redo the operation, including saving and restoring wire connections that would otherwise be broken.

The `CommandUtils` namespace provides helpers for serializing/deserializing items during undo/redo, ensuring state is preserved correctly.

**Example flow:**

1. User drags an AND gate onto the scene.
2. `AddItemsCommand` is created with the element data.
3. `redo()` adds the element to the scene.
4. If the user presses Ctrl+Z, `undo()` removes the element.
5. If they press Ctrl+Y, `redo()` re-adds it.

### Workspace

`App/Scene/Workspace.h` manages the working context:

- Currently open file
- Modification state (asterisk in title)
- Save, save-as, and open operations
- Autosave to a temporary file for crash recovery — debounced 500ms after the last change; a tab restored from an autosave on launch is marked "(recovered)" in its tab title

---

## User Interface (App/UI)

### MainWindow

`App/UI/MainWindow.h` is the main application window. It contains:

- **Menu bar**: File, Edit, View, Simulation, Examples, Learn, Language, Help. The Learn menu holds Exercises and Tours submenus; "Report Translation Error" lives under Help.
- **Toolbar**: New, Open, Save | Undo, Redo, Rotate, Cut, Copy, Paste, Delete | Zoom In, Zoom Out, Reset Zoom, Zoom to Fit | Play, Restart, Waveform. Undo/Redo are inserted dynamically for the active tab by `App/UI/SceneUiBinder.cpp/h`.
- **Element palette** (side dock): drag elements onto the scene, plus a draggable/resizable **minimap** overlay (`App/UI/MinimapWidget.cpp/h`) over the canvas
- **Property editor** (bottom dock): edit properties of the selected element
- **Central area**: `GraphicsView` with the circuit `Scene`

### Element Palette

`App/UI/ElementPalette.h` organizes the available elements in tabs by category: I/O, Gates, Combinational, Memory, IC, and Misc, plus a Search tab that appears once the user types into the palette's filter box. The user drags an element from the palette onto the scene.

### Property Editor

`App/UI/ElementEditor.h` dynamically generates the property interface based on the descriptor of the selected element's type. For example:

- A **Clock** shows a frequency slider
- A **LED** shows a color selector
- An **AND** gate shows an input count control

---

## File Serialization (App/IO)

### The `.panda` File Format

Circuit files (`.panda`) use a binary format (`App/IO/Serialization.h`):

```text
[Preamble]
  - Magic number (0x57504346 = "WPCF") + format version
  - Metadata (QMap): dolphin data, rect, etc.; an IC's pin labels and source names (`inputLabels`, `outputLabels`);
    its pin order (`inputOrder`, `outputOrder`), each pin named by its port's serial id, as text; the copies of the IC
    files the document uses (`fileCopies`, below)

[Elements]
  - Element count
  - For each element:
    - Type (ElementType)
    - Position, rotation, flip state
    - Input and output ports with IDs
    - The type's parameters (frequency, colour, etc.), each under its spec's key (`ParamSpec::key`)
    - The appearance: the picture of each slot the user changed, under the slot's key

[Connections]
  - Connection count
  - For each connection:
    - Source port ID (output)
    - Destination port ID (input)
```

On load, a **port map** is built during element deserialization so that connections can resolve their endpoints. The format is backward-compatible and can read files from older wiRedPanda versions. The pin-order keys are added metadata, with no format revision: a file without them takes the order its positions give, read before the load's snap to the grid.

**File copies.** A document that holds file-backed ICs keeps a copy of each file's definition under the `fileCopies`
metadata entry, a map from the file's bare name (as the document references it) to its packed copy: the hash of the
file as the registry last read it, and the definition's bytes. The copies cover the files those ICs nest, too; when
several ICs nest one file, the first copy saved for its name is kept. On open, a copy stands for its file:

- a file unchanged since the save keeps the copy;
- a file changed since the save comes in as a reload of the IC instances that use it, which the undo returns to the copy;
- a file that is gone keeps the copy, and the IC keeps its link to the file;
- a file with no copy is read from disk, as before.

A change to a nested file reloads the instances of the files that nest it, with an undo, as a change to any file does.
The copies are per file, not per instance: when two instances nest different versions of one file, the first
instance's copy is the one saved. The copies sit outside the blob registry, so no embedded IC is listed for them.

### Version History

The file `App/IO/VersionInfo.h` tracks all format versions:

| Version | Release | Key Changes                                                                              |
|---------|---------|------------------------------------------------------------------------------------------|
| V_1_1   | 2015    | Clock element added                                                                      |
| V_2_4   | 2019    | Audio element support (Buzzer, AudioBox)                                                 |
| V_3_1   | 2021    | Lock state for input elements; display colors                                            |
| V_4_1   | 2024    | QMap-based format; port serial IDs                                                       |
| V_4_2   | 2024    | TruthTable output data stored in file                                                    |
| V_4_5   | 2025    | Preamble gains a metadata QMap                                                           |
| V_4_6   | 2025    | Preamble unified into a single metadata QMap (dolphin filename and scene rect folded in) |
| V_4_7   | 2026    | Connection uses QMap-based serialization                                                 |
| V_5_0   | 2026    | File format version bump for 5.0.0                                                       |
| V_5_1   | 2026    | Versioned blob registry                                                                  |

The deserialization code reads the file version and applies automatic migrations as needed. Reading older versions is **always** supported.

Past `V_5_1`, `App/Versions.h` switches to a monotonic single-segment revision scheme independent
of the app version: **`Rev100`** (the circuit payload is zlib-compressed after the header) and
**`Rev101`** ("slim portable payload" — elides default-valued fields, derives port serialIds from
element id + position, omits non-IC port names and resource-path appearance slots).

---

## Code Generation (App/CodeGen)

### Arduino (`App/CodeGen/ArduinoCodeGen.cpp`)

Converts a circuit into an Arduino sketch:

- Maps circuit inputs/outputs to GPIO pins.
- Emits `setup()` with `pinMode()` calls.
- Emits `computeLogic()` with boolean expressions for combinational logic and state machines for sequential elements.
- Supports multiple board configurations: Arduino UNO R3/R4, Arduino Nano, Arduino Mega 2560, and ESP32 (WiFi/Bluetooth).
- Generates testbench for validation.
- Supports IC hierarchy.

### SystemVerilog (`App/CodeGen/SystemVerilogCodeGen.cpp`)

Converts a circuit into synthesizable SystemVerilog:

- Emits `module` declarations with `input`/`output` ports.
- Generates `assign` statements for combinational logic.
- Handles hierarchical IC modules recursively.
- Supports sequential elements (flip-flops).
- Can be validated with iverilog, yosys, and verilator.

---

## BeWavedDolphin (Waveform Viewer)

The built-in waveform viewer (`App/BeWavedDolphin/`) provides signal visualization, similar to GTKWave. The tool grew from a handful of files into a set of focused classes:

- **`BeWavedDolphin.cpp`**: main tool window
- **`WaveformView.cpp`**: waveform rendering
- **`SignalModel.cpp`**: data model with input and output signals
- **`Serializer.cpp`**: saves/loads waveform captures
- **`DolphinCommands.cpp`**: undo/redo commands for waveform edits
- **`DolphinClipboard.cpp`** / **`DolphinEdits.cpp`**: copy/paste and cell-edit operations
- **`DolphinFile.cpp`** / **`DolphinExporter.cpp`**: file open/save workflow and PDF/PNG/text export
- **`WaveformSimulator.cpp`** / **`DolphinModelBuilder.cpp`**: drives the simulation and builds the signal table from it
- **`DolphinZoom.cpp`**: waveform view zoom handling

**Features:**

- **Table-based**: Rows = signals, Columns = time steps.
- **Simulation**: Runs the circuit for every input combination (or user-specified patterns).
- **Visualization**: Renders waveforms as line plots or binary values.
- **Export**: PDF, PNG, and text formats.
- **Interactive**: Users can edit input patterns, set clock waves, and zoom.
- **Undo/Redo**: Waveform cell edits are undoable, backed by `DolphinCommands`.

Access it from the **Simulation** menu → **Waveform** (Ctrl+W) to see how signals propagate through a circuit over time. Ideal for analyzing sequential circuits.

---

## Resources and Internationalization

### Element Icons

Icons live in `App/Resources/Components/`, organized by category:

- `Input/`, `Logic/`, `Memory/`, `Output/`, `Misc/`
- SVG format for scalability

### Translations

wiRedPanda supports 39 languages:

- `.ts` files (Qt Linguist) in `App/Resources/Translations/`
- Managed via [Weblate](https://hosted.weblate.org/projects/wiredpanda/wiredpanda)
- Compiled to `.qm` and merged directly into the binary during build via Qt's `qt_add_translations()` (`MERGE_QT_TRANSLATIONS`) — no separate `.qm` files are shipped
- Uses `tr()` and `QT_TR_NOOP()` in code

To add a new translatable string:

```cpp
QString msg = tr("New translatable message");
```

---

## The Test System

wiRedPanda has a comprehensive test suite of **over 230 test classes**.

### Test Organization

```text
Tests/
├── Unit/                      # Individual unit tests
│   ├── Commands/              #   Undo/redo tests
│   ├── Common/                #   Utility tests
│   ├── Core/                  #   Core type tests
│   ├── Elements/              #   Individual element tests
│   ├── Exercise/              #   ExerciseEngine tests
│   ├── Factory/               #   ElementFactory tests
│   ├── Logic/                 #   Logic gate tests
│   ├── MCP/                   #   MCP server tests (internal use)
│   ├── Scene/                 #   QGraphicsScene tests
│   ├── Serialization/         #   Serialization tests
│   ├── Simulation/            #   Simulation engine tests
│   ├── Tour/                  #   TourEngine tests
│   ├── Ui/                    #   UI component tests
│   └── Wiring/                #   Port and connection tests
├── Integration/               # Integration tests
│   ├── IC/                    #   Hierarchical IC tests (9 levels)
│   │   └── Tests/
│   │       ├── TestLevel1*    #     Basic sequential elements
│   │       ├── TestLevel2*    #     Combinational circuits
│   │       ├── ...
│   │       └── TestLevel9*    #     Complete CPUs
│   ├── Logic/                 #   Mux/Demux tests
│   ├── TestArduino.cpp        #   Arduino code generation
│   ├── TestFiles.cpp          #   .panda file loading
│   ├── TestSimulation.cpp     #   Simulation engine
│   └── TestWorkspace.cpp      #   Workspace operations
├── System/                    # System-level tests
│   └── TestWaveform.cpp       #   BeWavedDolphin
├── Fuzz/                      # libFuzzer harnesses (local/manual, not run in CI)
├── BackwardCompatibility/     # Old format compatibility
├── Resources/                 # Icon/resource validation
│   └── TestIcons.cpp
├── Common/                    # Shared utilities
│   ├── TestUtils.h/cpp        #   Test environment setup
│   └── ICTestHelpers.h        #   IC test helpers
├── Fixtures/                  # Test .panda files
└── Runners/
    └── TestWiredpanda.cpp     # Unified test entry point
```

All tests compile into a single binary: `build/test_wiredpanda`. Each test class is a `QObject` with private slots as test methods.

### Test Utilities and CircuitBuilder

`Tests/Common/TestUtils.h` provides everything you need to build and test circuits:

```cpp
// Environment setup (call once per test class)
TestUtils::setupTestEnvironment();  // Headless mode, suppress dialogs
TestUtils::configureApp();          // QApplication settings

// Create a fresh workspace with a scene
WorkSpace workspace;

// CircuitBuilder reduces wiring boilerplate
CircuitBuilder builder(workspace.scene());
Element sw1{ElementType::InputSwitch};
Element sw2{ElementType::InputSwitch};
Element andGate{ElementType::And};
Element led{ElementType::Led};
builder.add(&sw1, &sw2, &andGate, &led);
builder.connect(&sw1, 0, &andGate, 0);  // sw1 output 0 → AND input 0
builder.connect(&sw2, 0, &andGate, 1);  // sw2 output 0 → AND input 1
builder.connect(&andGate, 0, &led, 0);  // AND output 0 → LED input 0
auto *sim = builder.initSimulation();

// Set inputs and simulate
sw1.setOn(true);
sw2.setOn(false);
sim->update();

// Read outputs
bool result = TestUtils::inputStatus(&led);  // false (1 AND 0 = 0)
```

**Useful helpers:**

| Function                                       | Purpose                                                         |
|------------------------------------------------|-----------------------------------------------------------------|
| `TestUtils::createWorkspace()`                 | Create a `std::unique_ptr<WorkSpace>` with an initialized scene |
| `TestUtils::setMultiBitInput(switches, int)`   | Decompose an integer into bits                                  |
| `TestUtils::readMultiBitOutput<T>(elms, port)` | Combine multiple outputs into an integer                        |
| `TestUtils::clockCycle(sim, clk)`              | Full clock pulse: LOW → HIGH → LOW                              |
| `TestUtils::clockToggle(sim, clk)`             | Single clock edge toggle                                        |
| `TestUtils::inputStatus(elm, port = 0)`        | Read a port's input status as bool                              |
| `TestUtils::outputStatus(elm, port = 0)`       | Read a port's output status as bool                             |
| `TestUtils::countConnections(scene)`           | Count `Connection` items in a scene                             |
| `TestUtils::sceneConnections(scene)`           | List all `Connection` items in a scene                          |

> Note: `getInputStatus`/`getOutputStatus` were renamed to `inputStatus`/`outputStatus` (and
> `createSwitches`/`setInputValues` were removed) in a project-wide cleanup that dropped the
> `get`-prefix from public accessor APIs.

### Writing Your First Test

Here is a complete, minimal test for an AND gate:

**`Tests/Unit/Elements/TestMyGate.h`**:

```cpp
#pragma once
#include <QObject>

class TestMyGate : public QObject {
    Q_OBJECT
private slots:
    void testAndTruthTable();
};
```

**`Tests/Unit/Elements/TestMyGate.cpp`**:

```cpp
#include "TestMyGate.h"
#include "Tests/Common/TestUtils.h"

void TestMyGate::testAndTruthTable()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element sw0{ElementType::InputSwitch};
    Element sw1{ElementType::InputSwitch};
    Element gate{ElementType::And};
    Element led{ElementType::Led};

    builder.add(&sw0, &sw1, &gate, &led);
    builder.connect(&sw0, 0, &gate, 0);
    builder.connect(&sw1, 0, &gate, 1);
    builder.connect(&gate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // 0 AND 0 = 0
    sw0.setOn(false); sw1.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), false);

    // 0 AND 1 = 0
    sw0.setOn(false); sw1.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), false);

    // 1 AND 0 = 0
    sw0.setOn(true); sw1.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), false);

    // 1 AND 1 = 1
    sw0.setOn(true); sw1.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), true);
}
```

### Testing Sequential Logic

Sequential elements need clock edges:

```cpp
void TestMyFlipFlop::testDFlipFlop()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Element swD{ElementType::InputSwitch};
    Element swClk{ElementType::InputSwitch};
    Element dff{ElementType::DFlipFlop};
    Element ledQ{ElementType::Led}, ledNQ{ElementType::Led};

    builder.add(&swD, &swClk, &dff, &ledQ, &ledNQ);
    builder.connect(&swD, 0, &dff, 0);       // D input
    builder.connect(&swClk, 0, &dff, 1);     // Clock input
    builder.connect(&dff, 0, &ledQ, 0);      // Q output
    builder.connect(&dff, 1, &ledNQ, 0);     // ~Q output

    auto *sim = builder.initSimulation();

    // Set D = 1, then pulse clock
    swD.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);       // LOW → HIGH → LOW

    QCOMPARE(TestUtils::inputStatus(&ledQ), true);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), false);

    // Set D = 0, then pulse clock
    swD.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);

    QCOMPARE(TestUtils::inputStatus(&ledQ), false);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), true);
}
```

### Running Tests

```bash
# All tests (parallel execution)
ctest --preset debug

# A specific test class
./build/test_wiredpanda TestLogicGates

# List all available test classes
./build/test_wiredpanda -functions

# Verbose output
ctest --preset debug --verbose
```

> **Important**: Always run tests from the project root directory. Never `cd build/`.

**Conventions:**

- One test file per class being tested
- Method names start with `test`
- Use `QVERIFY(condition)` for boolean assertions
- Use `QCOMPARE(actual, expected)` for comparisons
- Use `QVERIFY_THROWS_EXCEPTION(ExceptionType, expr)` for exception tests
- **Always prefer fixing code over changing tests** to accept incorrect behavior

### Coverage and Sanitizers

```bash
# Code coverage
cmake --preset coverage
cmake --build --preset coverage
ctest --preset coverage
# Generates report with lcov/gcov

# Address Sanitizer (buffer overflows, use-after-free, etc.)
cmake --preset asan
cmake --build --preset asan
ctest --preset asan

# Thread Sanitizer (data races)
cmake --preset tsan
cmake --build --preset tsan
ctest --preset tsan

# Undefined Behavior Sanitizer
cmake --preset ubsan
cmake --build --preset ubsan
ctest --preset ubsan
```

---

## CI/CD and Workflows

The project uses **GitHub Actions** for continuous integration:

### `build.yml` — Build and Tests

- **Platform matrix**: Ubuntu (x86_64 and ARM), Windows (x86_64 and ARM), macOS
- **Qt versions**: 6.8.3, 6.12.0
- **Timeout**: 10 minutes for tests
- **MCP integration test**: runs `MCP/Client/run_tests.py` after every build
- **Artifacts**: uploads build logs on failure

### `deploy.yml` — Releases

- Generates distribution artifacts:
  - **Linux**: AppImage
  - **Windows**: ZIP
  - **macOS**: DMG
- Integrates Sentry for crash reporting
- Automatic debug symbol upload

### Other Workflows

| Workflow          | Purpose                                                                                                                                                      |
|-------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `codeql.yml`      | Code security analysis (CodeQL)                                                                                                                              |
| `coverage.yml`    | Code coverage tracking; uploads to Codecov                                                                                                                   |
| `sanitizers.yml`  | Runs the ASan/TSan/UBSan/MSan presets as their own dedicated CI job                                                                                          |
| `lint.yml`        | Lints the Python trees (`MCP/Client/`, `Scripts/`, test fixture generators) with ruff/pylint/pyrefly/vulture, plus `actionlint` for the workflow YAML itself |
| `fresh-deps.yml`  | Builds from a clean distro image using only the apt packages `BUILD.md` documents, to catch package drift and keep the CMake version floor honest            |
| `aur-publish.yml` | Publishes the Arch Linux AUR package (`Packaging/AUR/`)                                                                                                      |
| `translate.yml`   | Translation management                                                                                                                                       |
| `wasm.yml`        | WebAssembly build                                                                                                                                            |

---

## How To: Add a New Element

This walkthrough adds a hypothetical **Buffer** gate (output = input, 1 input, 1 output). A gate is a descriptor and
its kernel's logic; it has no class of its own.

### Step 1: Name the Kernel and Write Its Logic

1. Add `KernelKind::Buffer` to the `KernelKind` enum in `App/Element/ElementTypeDescriptor.h`.
2. Add its logic to `App/Simulation/KernelLogic.cpp`, and its entry to `KernelLogic::of()` (the switch names every kind,
   so the compiler finds a kind left out):

```cpp
void bufferGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(element.simInputs().at(0)); // Pass input through
}

// in KernelLogic::of():
    case KernelKind::Buffer: return &KernelLogicDetail::bufferGate;
```

### Step 2: Describe the Type

**`App/Element/GraphicElements/Buffer.cpp`**:

```cpp
// Copyright (C) 2026 - Your Name
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::Buffer> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Buffer,
        .group = ElementGroup::Gate,
        .minInputSize = 1, .maxInputSize = 1,
        .minOutputSize = 1, .maxOutputSize = 1,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe() {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::Buffer;
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/buffer.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Buffer", "BUFFER");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Buffer", "Buffer");
        descriptor.trContext = "Buffer";
        descriptor.appearanceSlots = {{"body", ":/Components/Logic/buffer.svg"}};
        return descriptor;
    }

    static inline const bool registered = [] {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
```

### Step 3: Add to the Build System

1. Add `ElementType::Buffer` to the enum in `App/Core/Enums.h`.
2. Add `App/Element/GraphicElements/Buffer.cpp` to the `MODEL_SOURCES` list in `CMakeSources.cmake`, in
   **alphabetical order**.
3. Create an SVG icon at `App/Resources/Components/Logic/buffer.svg`.
4. Add the SVG to the resource file if needed.

### Step 4: Write Tests

Create a test that verifies the buffer passes its input to its output.

### Step 5: Build and Verify

```bash
cmake --build --preset debug
./build/test_wiredpanda TestYourNewTest
```

---

## How To: Add a New Test

### Step 1: Create the Test Files

**`Tests/Unit/Elements/TestMyElement.h`**:

```cpp
#pragma once
#include <QObject>

class TestMyElement : public QObject {
    Q_OBJECT
private slots:
    void testProperties();
    void testBehavior();
};
```

**`Tests/Unit/Elements/TestMyElement.cpp`**:

```cpp
#include "TestMyElement.h"
#include "Tests/Common/TestUtils.h"
// Elements are built by their type: no element header to include

void TestMyElement::testProperties() {
    Element elm{ElementType::MyElement};
    QCOMPARE(elm.elementType(), ElementType::MyElement);
    // Verify port counts, group, etc.
}

void TestMyElement::testBehavior() {
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    // Build circuit, simulate, assert results...
}
```

### Step 2: Register in `CMakeSources.cmake`

Add both files to the `TEST_WIREDPANDA_SOURCES` list in alphabetical order:

```cmake
set(TEST_WIREDPANDA_SOURCES
    # ... existing entries ...
    Tests/Unit/Elements/TestMyElement.cpp
    Tests/Unit/Elements/TestMyElement.h
    # ... existing entries ...
)
```

### Step 3: Register in `Tests/Runners/TestWiredpanda.cpp`

Add an `#include` at the top with the other includes, then add an entry to the `tests` vector in `main()`:

```cpp
#include "Tests/Unit/Elements/TestMyElement.h"
```

```cpp
{"TestMyElement", []() -> QObject * { return new TestMyElement; }},
```

### Step 4: Register in `CMakeLists.txt`

Add an `add_test()` call and label it so CTest picks it up:

```cmake
add_test(NAME TestMyElement COMMAND test_wiredpanda TestMyElement WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(TestMyElement PROPERTIES LABELS "unit")
```

### Step 5: Run

```bash
cmake --build --preset debug
./build/test_wiredpanda TestMyElement
```

---

## Git Workflow

### Branches

- `master`: main branch, always stable
- Feature branches: created from `master`

### Workflow for Contributing

```bash
# 1. Clone and create a branch
git clone https://github.com/GIBIS-UNIFESP/wiRedPanda.git
cd wiRedPanda
git checkout -b feat/my-feature

# 2. Develop and test
cmake --preset debug
cmake --build --preset debug
ctest --preset debug

# 3. Commit with a descriptive message
git add App/Element/GraphicElements/MyElement.cpp
git commit -m "feat: add MyElement component

- Implement logic for new element
- Add unit tests
- Update element palette"

# 4. Push and create a Pull Request
git push origin feat/my-feature
```

### Commit Conventions

The project uses **Conventional Commits**:

| Prefix      | Use                                        |
|-------------|--------------------------------------------|
| `feat:`     | New feature                                |
| `fix:`      | Bug fix                                    |
| `refactor:` | Refactoring without behavior change        |
| `docs:`     | Documentation changes                      |
| `test:`     | Adding or fixing tests                     |
| `perf:`     | Performance improvement                    |
| `chore:`    | Maintenance tasks (CI, dependencies, etc.) |

---

## Suggested Learning Path

### Week 1: Orientation

1. **Build the project** and run the application.
2. **Open example circuits** from `Examples/` — try `counter.panda`, `decoder.panda`, `mux_demux.panda`.
3. **Play with the UI**: drag gates, wire them, flip switches, watch LEDs.
4. **Run the tests**: `ctest --preset debug` — see them all pass.

### Week 2: Read the Code

1. **Read one gate end-to-end**: Start with `And.cpp`'s `ElementInfo<ElementType::And>` specialization → its kernel's logic in `KernelLogic.cpp` → `StatusOps::statusAndAll()`.
2. **Compare with other gates**: `Not.cpp`, `Or.cpp`, `Xor.cpp` and their kernels — notice the pattern.
3. **Read a sequential element**: `DFlipFlop.cpp` and its kernel in `KernelLogic.cpp` — understand edge detection and the history it keeps.
4. **Read a test**: `TestLogicGates.cpp` — see how `CircuitBuilder` and `TestUtils` work.
5. **Trace the simulation loop**: Read `Simulation::update()` to see the five phases.

### Week 3: Make a Change

1. **Write a test** for an existing element — add a truth-table test for `Xnor` or test a flip-flop's async reset.
2. **Break something on purpose**: change an AND gate to OR behavior, run the tests, see what fails.
3. **Fix a small issue** or improve a tooltip string.

### Week 4: Build Something

1. **Implement a new element** following the walkthrough above.
2. **Add tests** for your new element.
3. **Open a PR** and go through the review process.

---

## Starter Tasks

Ordered by difficulty:

### Beginner

| Task                                                      | Skills Practiced            |
|-----------------------------------------------------------|-----------------------------|
| Add a missing truth-table test for an existing gate       | Test framework, element API |
| Add Doxygen comments to `Element.h` public methods | Code reading, documentation |
| Create a new example circuit (`.panda` file)              | Understanding the UI        |
| Fix a typo or improve a UI string                         | Build system, PR workflow   |

### Intermediate

| Task                                                    | Skills Practiced                   |
|---------------------------------------------------------|------------------------------------|
| Implement a simple new gate (e.g., 3-input majority)    | Full element lifecycle             |
| Add a test for async preset/clear on a flip-flop        | Sequential logic testing           |
| Trace and document how an IC loads its internal circuit | Serialization, hierarchical design |
| Improve an error message in the code generator          | CodeGen architecture               |

### Advanced

| Task                                                  | Skills Practiced                 |
|-------------------------------------------------------|----------------------------------|
| Add a new display variant (e.g., dot matrix)          | Qt painting, element rendering   |
| Add export to a new HDL format                        | Code generation, visitor pattern |
| Optimize the simulation loop for large circuits       | Profiling, performance           |
| Add a new waveform analysis feature to BeWavedDolphin | Signal visualization             |

---

## Tips and Common Pitfalls

### Do

- **Always build in `build/`** — never generate artifacts in the source tree
- **Use CMake presets** — ensures consistent configuration
- **Run tests before opening a PR** — `ctest --preset debug`
- **Keep `CMakeSources.cmake` alphabetically sorted** — source lists must stay sorted
- **Use `python3`** on Linux/macOS — `python` may not exist
- **End files with a trailing newline** — project requirement
- **Trim trailing whitespace** — no spaces/tabs at line ends
- **Use `///` for Doxygen docs** — never `//!`
- **Generate browsable API docs locally** (if Doxygen is installed) — `cmake --build --preset debug --target doxygen`, output at `build/docs/html/index.html`

### Don't

- **Don't `cd build/`** to run tests — always stay at the project root
- **Don't modify `CMakeLists.txt` to add sources** — use `CMakeSources.cmake`
- **Don't add backward-compatibility code** for features that haven't been released yet
- **Don't change tests to accept incorrect behavior** — fix the code instead
- **Don't use `--timeout 120000`** for compilation — use at least 5-10 minutes
- **Don't commit build files** — the `build/` directory is in `.gitignore`

### Slow Builds?

1. Check that **ccache** is enabled: `ccache -s`
2. Check that **mold** is being used: look for `mold` in CMake output
3. Use **Unity Build** for faster full compilations
4. The first build will always be slow — subsequent builds are incremental

### Common Errors

| Error                             | Likely Cause                               | Solution                                  |
|-----------------------------------|--------------------------------------------|-------------------------------------------|
| `Qt6 not found`                   | Qt not installed or not in PATH            | Install Qt 6.8+ and set CMAKE_PREFIX_PATH |
| `Ninja not found`                 | Ninja not installed                        | `apt install ninja-build`                 |
| `undefined reference to ...`      | .cpp file not listed in CMakeSources.cmake | Add the file to the source list           |
| Tests crash with segfault         | Accessing destroyed object                 | Use ASAN: `cmake --preset asan`           |
| Tests pass locally but fail in CI | Platform-dependent behavior                | Test with the same Qt versions as CI      |

---

## Key Files Reference

| File                                  | Purpose                                               |
|---------------------------------------|-------------------------------------------------------|
| `App/Core/Enums.h`                    | All enums: `ElementType`, `ElementGroup`, `Status`    |
| `App/Core/StatusOps.h`                | Four-state logic operations (AND, OR, NOT, XOR)       |
| `App/Core/Application.h`              | Custom QApplication with theme and i18n setup         |
| `App/Core/Settings.h`                 | Persistent settings management                        |
| `App/Core/ThemeManager.h`             | Theme system (light/dark/system)                      |
| `App/Core/Priorities.h`               | Topological sorting and feedback detection            |
| `App/Element/Element.h`               | The element class: every type, the IC its subclass    |
| `App/Element/InputLevel.h`            | The inputs' shared level and lock parameters          |
| `App/Element/ElementInfo.h`           | Self-registration template and constraints            |
| `App/Element/ElementFactory.h`        | Factory for creating elements by type                 |
| `App/Element/ElementTypeDescriptor.h` | Each type: kernel, parameters, ports, appearance      |
| `App/Element/Parameters.h`            | A type's parameters: specs, values and their rules    |
| `App/Element/ElementColour.h`         | The colour names and their cycle                      |
| `App/Element/WirelessNode.h`          | A node's wireless mode, read as its enum              |
| `App/Element/IC.h`                    | Integrated circuit element                            |
| `App/Element/ICLoader.h`              | Loads an IC's sub-circuit from file or embedded blob  |
| `App/Scene/ICRegistry.h`              | Central IC registry with file monitoring              |
| `App/Simulation/Simulation.h`         | Simulation engine and update loop                     |
| `App/Simulation/KernelLogic.h`        | Each kernel's logic, by the type's `KernelKind`       |
| `App/Presentation/ElementPresenter.h` | How each type's body, picture and label are drawn     |
| `App/Scene/Scene.h`                   | Main graphics scene                                   |
| `App/Scene/Commands.h`                | All undo/redo command classes                         |
| `App/Scene/Workspace.h`               | File and workspace management                         |
| `App/Wiring/Port.h`                   | Port base class, plus InputPort/OutputPort            |
| `App/Wiring/Connection.h`             | Wire connection class                                 |
| `App/IO/Serialization.h`              | File format save/load                                 |
| `App/Versions.h`                      | File-format version/revision constants                |
| `App/IO/VersionInfo.h`                | Named predicates for file-format compatibility checks |
| `Tests/Common/TestUtils.h`            | CircuitBuilder and test helpers                       |
| `CMakeSources.cmake`                  | Source file lists (add new files here)                |
| `CMakePresets.json`                   | Build presets (debug, release, asan, etc.)            |

---

## Glossary

| Term                      | Description                                                                                       |
|---------------------------|---------------------------------------------------------------------------------------------------|
| **Combinational Circuit** | Circuit whose output depends only on current inputs                                               |
| **Sequential Circuit**    | Circuit with memory elements (flip-flops, latches)                                                |
| **Clock**                 | Periodic signal that synchronizes sequential elements                                             |
| **Rising Edge**           | Transition from 0 to 1 on the clock signal                                                        |
| **Descriptor**            | Everything an element of a type is (`ElementTypeDescriptor`): kernel, parameters, ports, look     |
| **Kernel**                | What the simulation computes for a type (`KernelKind`), its logic in `KernelLogic`                |
| **Parameter**             | A setting a type has, such as a colour or a frequency (`ParamId`), kept by its spec's rule        |
| **Feedback Loop**         | Path where an output feeds back to an input                                                       |
| **Grid Snapping**         | Automatic alignment of elements to the 16px grid                                                  |
| **IC**                    | Integrated Circuit — sub-circuit encapsulated as a component                                      |
| **Latch**                 | Level-sensitive memory element                                                                    |
| **Flip-Flop**             | Edge-sensitive memory element                                                                     |
| **Mux/Demux**             | Multiplexer/Demultiplexer — signal routing                                                        |
| **PCH**                   | Precompiled Header — precompiled header for faster builds                                         |
| **QGraphicsScene**        | Qt class that manages 2D graphic items                                                            |
| **QDataStream**           | Qt class for binary serialization                                                                 |
| **Scene**                 | The main scene containing the circuit                                                             |
| **Appearance**            | The SVG used to render an element; user-replaceable via `canChangeAppearance` (e.g., ANSI vs IEC) |
| **Status**                | 4-value logic state (Unknown, Inactive, Active, Error)                                            |
| **Topological Sort**      | Ordering that respects dependencies between elements                                              |
| **Tx/Rx**                 | Transmitter/Receiver — wireless mode of the Node element                                          |
| **Unity Build**           | Compilation technique that groups .cpp files to reduce overhead                                   |
| **Workspace**             | Working context (open file, modification state)                                                   |

---

## Additional Resources

- **Repository**: [github.com/GIBIS-UNIFESP/wiRedPanda](https://github.com/GIBIS-UNIFESP/wiRedPanda)
- **Roadmap**: [GitHub Project](https://github.com/orgs/GIBIS-UNIFESP/projects/1)
- **Translations**: [Weblate](https://hosted.weblate.org/projects/wiredpanda/wiredpanda)
- **Wiki**: [GitHub Wiki](https://github.com/GIBIS-UNIFESP/wiRedPanda/wiki)
- **Issues**: [GitHub Issues](https://github.com/GIBIS-UNIFESP/wiRedPanda/issues) (look for `good first issue`)
- **Coverage**: [Codecov](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda)
