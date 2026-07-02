# Claude Memory - wiRedPanda Project

## Development Environment

### Build Tools

- **Build System**: CMake
- **CRITICAL**: Always build in `build/` directory to prevent accidental commits
- **Build Timeout**: Always use at least 5-10 minute timeout for compilation commands (2 minutes is insufficient)
- **ccache**: Compiler cache installed for faster builds - automatically used via PATH in devcontainer

### Build System Requirements

- **CMake Generator**: Ninja (required on all platforms for consistency)
- **Installation**:
  - Windows: `choco install ninja` or `scoop install ninja`
  - Linux: `apt install ninja-build` or `yum install ninja-build`
  - macOS: `brew install ninja`

### Build Commands (All Platforms)

```bash
# Configure with build type
cmake --preset debug
# Build
cmake --build --preset debug
# Tests
ctest --preset debug
```

- **mold linker**: Modern fast linker installed (`sudo apt install mold`) - automatically used by CMake when available
- **Visual Studio BuildTools**: `"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"`

### Python Usage

- **CRITICAL**: On Unix systems (Linux/macOS), always use `python3` command instead of `python`
- The `python` command may not exist or point to Python 2.x on many Unix systems
- All Python scripts in this project require Python 3.x

### Development Features

Advanced development features supported:

**Coverage Analysis**:

- `cmake --preset coverage` (sets `--coverage` flags for GCC/Clang)

**Sanitizers**:

- Address Sanitizer: `cmake --preset asan`
- Thread Sanitizer: `cmake --preset tsan`
- Memory Sanitizer: `cmake --preset msan` (Clang only)
- UB Sanitizer: `cmake --preset ubsan`

**Windows Metadata**: Comprehensive application properties for professional deployment

### Testing

- Project uses Qt Test framework
- **IMPORTANT**: Always stay in project root directory - don't cd to build/
- **Test execution**: Use CMake presets for automatic configuration:

  ```bash
  ctest --preset debug
  ```

- **Individual test execution**: Run specific test classes directly:

  ```bash
  ./build/test_wiredpanda TestSceneUndoredo    # Run a specific test class
  ./build/test_wiredpanda TestFiles            # Run file loading tests
  ./build/test_wiredpanda -functions           # List all available test classes
  ```

- **CTest preset options**:
  - `ctest --preset debug` - Debug build tests
  - `ctest --preset release` - Release build tests
  - `ctest --preset coverage` - Coverage analysis
  - `ctest --preset asan` - Address Sanitizer
  - `ctest --preset tsan` - Thread Sanitizer
  - `ctest --preset msan` - Memory Sanitizer
  - `ctest --preset ubsan` - Undefined Behavior Sanitizer
  - Parallel execution and output on failure are automatic via CMakeLists.txt configuration

### Translations

- **CRITICAL**: Always use `cmake --build --preset debug --target update_translations` to refresh `.ts` files — never call `lupdate` directly or use any other target.
- This target passes `-tr-function-alias tr+=PANDACEPTION` so strings wrapped in the `PANDACEPTION()` macro are correctly extracted. Using any other invocation silently discards those strings.
- **Exercise/Tour content translations**: Exercise/Tour step text (`App/Resources/Exercises/*.json`, `App/Resources/Tours/*.json`) never passes through `tr()` — it's JSON, so `lupdate` can't see it. It goes through a *separate* Weblate "JSON file" component and a generated catalog, `App/Resources/Translations/ExerciseTour/en.json` (`Scripts/generate_exercise_tour_catalog.py`). Regenerating it is wired into the same `update_translations` target above — still one command. See "Exercise/Tour Content" below.

## Project Structure

- Main project file: `CMakeLists.txt`
- App code: `App/` directory
- Tests: `Tests/` directory with comprehensive test suite
- Test executable: single unified `test_wiredpanda` binary with 178 test classes (run via `ctest --preset debug`)

## Digital Logic Simulation

### Simulation Type: Unified Event-Driven Engine (functional + temporal modes)

- **One engine, two modes**: a single event-driven drain (`Simulation::processEvents()`)
  reproduces the classic zero-delay behavior AND simulates real propagation delays. The ONLY
  difference between modes is the per-element delay applied when scheduling: functional mode
  forces every delay to 0; temporal mode uses `propagationDelay()`. There is no separate
  functional code path (`iterativeSettle()` and `eventSettle()` were both removed).
- **Functional mode** (`m_timePerTick == 0`, the default): every event lands at the current
  instant, so the drain degenerates to a full zero-delay settle — combinational and feedback
  circuits converge to a fixed point now. This is the educational default.
- **Temporal mode** (`m_timePerTick > 0`): events spread across future timestamps by
  per-element propagation delay, drained from a time-ordered `EventQueue`. Drives the
  Live Analyzer timing diagrams.
- **One flat netlist**: ICs do NOT simulate themselves. `Simulation::initialize()` flattens
  the IC hierarchy — every internal primitive (recursively, through nested ICs) joins the
  top-level netlist, with IC boundary ports spliced to their boundary Nodes. So per-element
  delays apply uniformly, including inside ICs, and internal signals are watchable.
- **Glitch-free ordering**: events drain in `(time, then topological-priority-desc)` order,
  one at a time — a gate is never evaluated until its upstream inputs have settled, so
  internally-generated (e.g. gated) clocks reach their final value before any downstream
  flip-flop samples them (no zero-delay glitches across the former IC boundary).
- **Design Goal**: Educational simplicity (functional mode) without giving up the option to
  show propagation-delay/timing behavior (temporal mode).

### Core Architecture (`App/Simulation/Simulation.cpp`)

- **Fixed Update Cycle**: 1ms `QTimer` tick drives `update()`.
- **`update()` phases per tick**:
  1. Advance event-driven clocks (`updateClock()`, real wall-clock time)
  2. Propagate user inputs (`updateOutputs()`)
  3. Evaluate logic via `processEvents()` — the unified engine (both modes)
  4. Push values onto wires / refresh output visuals (throttled to display rate)
- **Unified drain**: `processEvents()` drains `m_eventQueue` over
  `[currentTime, currentTime + timePerTick]`, popping one event at a time in
  `(time, priority-desc)` order (`SimEvent`/`SimTime` in `App/Simulation/`). Each changed
  element schedules its successors at `t + delay` (delay 0 in functional mode); a
  per-timestamp evaluation cap detects oscillation and canonicalizes feedback nodes to
  `Unknown`. Blocking and deterministic.
- **IC flattening**: `Simulation::initialize()` builds one flat primitive netlist via
  `collectFlatElements()` + `spliceICBoundaries()`; `IC::initializeSimulation()` only wires
  an IC's internal connections (it no longer settles). `mirrorICOutputs()` copies each IC's
  settled internal outputs onto its external ports for wire rendering.
- **Topological Sorting**: `topologicalSort()` orders elements by dependency depth and
  detects feedback loops; `sortSimElements()` builds the flat successor graph from output
  connections (with IC boundaries mapped to boundary Nodes) so adjacency order matches the
  priority system.
- **Non-blocking sequential commit**: synchronous elements (`ElementGroup::Memory`, collected
  recursively across the IC hierarchy by `collectSequentialElements()`) are bracketed each
  tick with `beginDeferredCommit()`/`commitDeferredOutputs()` so their new outputs publish
  together after the drain settles, matching SystemVerilog's non-blocking (`<=`) model. A
  post-edge `resettleCombinational()` pass re-propagates the committed values once.

### Logic Element Behavior

- **Combinational Logic**: zero-delay in functional mode; inertial propagation delay in
  temporal mode (a pulse shorter than the gate delay is absorbed).
- **Sequential Logic**: edge-triggered state changes (flip-flops, latches), committed
  non-blocking (see above) so combinational logic between them reads pre-tick state.
- **Propagation delay** (temporal mode): `GraphicElement::propagationDelay()` returns a
  per-element override or a per-type default (NOT 5 ns, AND/OR 10 ns, NAND/NOR 8 ns,
  sequential ~15-20 ns; sources/sinks/nodes/ICs 0). `Simulation::initialize()` seeds
  `m_delays` from it; `setElementDelay()` overrides at runtime; the Element Editor exposes a
  per-element "Prop. delay" field.

### Timing Characteristics

- **Clock Elements**: real-time, wall-clock driven (independent of sim mode).
- **Logic Gates**: immediate (functional) or delayed by `propagationDelay()` (temporal).
- **Sequential Elements**: edge-triggered; no setup/hold modeling.
- **Update Order**: events drain one at a time in topological-priority order within each
  timestamp, so upstream logic settles before downstream samples it — preventing race
  conditions and zero-delay clock glitches.

### Waveform Recording (temporal mode)

- `WaveformRecorder` (owned by `Simulation`) captures timestamped transitions of watched
  output ports during the temporal drain; the Live Analyzer (`LiveAnalyzerPanel` in
  `App/BeWavedDolphin/LiveAnalyzer.{h,cpp}`) renders the timing diagram. It is the second
  page of BeWavedDolphin (Stimulus Editor | Live Analyzer), which `Simulation ▸ Waveform`
  opens hosted in a bottom `QDockWidget` (floatable; one BewavedDolphin instance per
  circuit tab, swapped into the dock on tab switch). Toggle Functional/Temporal via the
  toolbar selector.

### Code Evidence

```cpp
// 1 ms tick drives update()
m_timer.setInterval(1ms);

// One drain for both modes; functional forces delay 0, temporal uses per-element delay
const SimTime d = (m_timePerTick == 0) ? 0 : m_delays.value(successor, 0);
// Schedule in (time, priority-desc) order so upstream settles before downstream samples
m_eventQueue.schedule({t + d, m_simPriorities.value(successor, -1), successor});

// Real-time clock timing (both modes)
if (elapsed > m_interval) {
    setOn(!m_isOn);  // Toggle based on frequency
}
```

### Implementation Classification

- **Abstraction Level**: functional simulation by default; optional propagation-delay
  (temporal) simulation.
- **Update Model**: unified event-driven engine over one flat primitive netlist — a single
  priority-ordered drain for combinational, feedback, and timed circuits, with ICs flattened
  in (no separate IC-internal settle).
- **Delay Model**: zero-delay (functional) or per-element inertial delay (temporal); clocks
  always real-time.
- **Target Audience**: Educational/demonstration use.
- **IMPORTANT**: Always prefer fixing code logic over changing tests to conform to incorrect behavior

### Conceptual Correctness Assessment

#### ✅ **Educationally Sound Design**

- **Boolean Logic**: Correctly implements all fundamental logic operations
- **Combinational Circuits**: Proper dependency ordering via topological sorting
- **Sequential Logic**: Accurate edge-triggered state machine behavior
- **Circuit Topology**: Correct signal flow and causality relationships
- **Functional Verification**: Reliable testing of digital logic concepts

#### ✅ **Correct Abstractions for Learning**

- **Zero-delay model** (functional mode) eliminates timing complexity for beginners
- **Immediate feedback** enhances educational interaction
- **Race condition prevention** via priority-based update ordering
- **Optional temporal mode** introduces propagation delays when timing behavior is wanted

#### ⚠️ **Deliberate Real-World Omissions**

- **No setup/hold constraints**: Real flip-flops need timing margins
- **Inertial (not transport) delay**: sub-delay pulses are absorbed, not propagated
- **No clock domain issues**: Real systems have multiple clocks and skew
- **No physical limitations**: Missing fan-out, drive strength, power concerns

#### 🎯 **Educational Target Alignment**

- **Perfect for**: Boolean algebra, combinational design, sequential concepts; basic propagation-delay/timing intuition via temporal mode
- **Not intended for**: precise timing analysis, synchronization, physical implementation
- **Design Philosophy**: Teach logic functionality first (functional mode); reveal timing behavior on demand (temporal mode)

#### **Verdict: Conceptually Correct for Educational Purpose**

The simulation accurately represents **ideal digital logic behavior** in functional mode and **inertial-delay timing behavior** in temporal mode, while deliberately abstracting deeper **physical implementation details**. This approach is pedagogically sound - students learn fundamental concepts correctly, then can explore propagation-delay timing without switching tools.

## Exercise/Tour Content

Two JSON-driven, in-app learning features share one architecture:

- **Exercises** (`App/Exercise/`): circuit-building challenges validated against the live `Scene` (`App/Resources/Exercises/*.json`).
- **Tours** (`App/Tour/`): guided UI walkthroughs with a spotlight overlay (`App/Resources/Tours/*.json`).

### Content discovery: flat directory scan, not hardcoded lists

`ExerciseBrowserDialog`/`TourBrowserDialog` call `ExerciseTourResources::discover("Exercises")`/`discover("Tours")` (`App/Core/ExerciseTourResources.h`), which merges four sources rather than one: the built-in bundled content (`scan(":/Exercises")`, mirroring `LanguageManager::availableLanguages()`'s `QDir(prefix).entryList({"*.json"}, QDir::Files)` pattern) plus three real, on-disk, end-user-writable locations — see below. Dropping a new built-in `.json` file into `App/Resources/Exercises/` or `App/Resources/Tours/` (with a globally unique `id`) is picked up automatically on the next reconfigure — no C++ or `.qrc` edits.

### End-user-writable discovery: three locations, two audiences

Built-in content only solves *contributor* flexibility (source-tree + reconfigure). Getting a
custom exercise or tour into an already-*compiled* app needs a real filesystem location, and
this splits by audience — deliberately kept separate, not merged into one "user folder":

- **`ExerciseTourResources::preferredContentDir(category)`** — what the **"Open Folder" button**
  in the browser dialogs opens: the install-relative folder next to the app (mirrors
  `MainWindow::setupExamplesMenu()`'s per-platform search paths, parameterized instead of
  hardcoded to `"Examples"`), or, if that isn't writable (e.g. installed under Program Files
  without admin rights), a `Documents/wiRedPanda/<category>` fallback created only at that
  point. Both are simple, visible, end-user-facing locations.
- **`ExerciseTourResources::managedContentDir(category)`** — `QStandardPaths::AppDataLocation + "/" + category"` (same idiom as `Workspace.cpp`'s autosaves folder). Reserved *exclusively* for a
  teacher/IT admin to pre-provision content in a managed classroom install. `discover()` scans
  it, but **no code path ever opens or is allowed to fall back into it from the button** — that
  invariant is deliberate and is exercised by
  `TestExerciseTourResources::testPreferredContentDirReturnsWritablePathOutsideManagedDir`.

`discover()`'s merge order is built-in → AppData → install-relative → Documents fallback; a
colliding `id` is dropped from whichever source loses, via the testable `mergeUnique()` seam,
never silently overwriting an earlier entry.

### Resource embedding: CMake glob, no `.qrc`

Exercise/Tour JSON is intentionally **not** listed in a `.qrc` (`rcc` can't glob, which would reintroduce the hardcoded-list problem). `CMakeLists.txt` globs the folders with `CONFIGURE_DEPENDS` and calls the target-based `qt6_add_resources(wiredpanda_resources "<name>" PREFIX ... BASE ... FILES ...)` API directly on the `wiredpanda_resources` OBJECT library, after it exists.

### i18n: a separate Weblate "JSON file" component

Step text never passes through `tr()`. `App/Resources/Translations/ExerciseTour/en.json` is the generated, committed English-source catalog (`Scripts/generate_exercise_tour_catalog.py`), keyed `<fileId>.title`, `<fileId>.description`, `<fileId>.<stepKey>.instruction`/`.hint` (Exercises) or `.title`/`.body` (Tours). Every translatable step has a stable, contributor-authored `"key"` used only to build these keys — never read by engine logic. `ExerciseTour/<lang>.json` files are Weblate-managed and embedded at `:/i18n/ExerciseTour/<lang>.json`. `ExerciseTourResources::translate(key, fallbackEnglish)` reloads the small catalog fresh on every call (no cache) — cheap given usage frequency (dialog open / exercise or tour start), and always falls back to the raw English text.

See `App/Resources/Exercises/README.md` and `App/Resources/Tours/README.md` for the full schema and the closed `target`/`click` vocabulary.

## Development Container

- **Ubuntu 24.04 LTS** based development environment
- **Location**: `.devcontainer/` directory with full configuration
- **Features**:
  - Qt 6.2+ development environment
  - CMake and build tools pre-configured
  - VS Code extensions for C++/Qt development
- **Usage**: Open project in VS Code and select "Reopen in Container"
- **Testing**: Supports headless test execution
- **Sentry Integration**: Comprehensive crash reporting with platform-specific backends
  - **Ubuntu/Windows**: Crashpad backend with handler process
  - **macOS**: Breakpad backend (in-process, sandbox-compatible)
  - **SDK Caching**: Reduces build time by ~80% on cache hits
  - **Artifact Validation**: Automatic verification of release packages and required libraries
- **Release Artifacts**: AppImage (Ubuntu), ZIP (Windows), DMG (macOS) with embedded crash reporting

## Analysis & Fix Documentation Protocol

- **IMPORTANT**: Whenever analyzing or fixing issues, create/update markdown documentation for progress tracking
- **Documentation Location**: Store all analysis markdowns in `.claude/` directory to avoid root bloat
- **Sentry Integration**: Connected to wiredpanda project (see `.github/workflows/deploy.yml`)
  - Debug symbols uploaded automatically on release
  - Platform-specific backends for optimal crash reporting
  - Comprehensive artifact validation before release
- **GitHub Project Integration**: GIBIS-UNIFESP organization project #1 "wiRedPanda" (public)
  - **Project URL**: <https://github.com/orgs/GIBIS-UNIFESP/projects/1>
  - **Access**: Available via `gh project` commands with authentication
  - **Fields**: Status, Priority, Size, Assignees, Labels, Milestones, etc.
- **Analysis Reports**: Store comprehensive crash analysis in dedicated markdown files (e.g., `.claude/SENTRY_CRASH_ANALYSIS.md`)
- **Fix Tracking**: Document root causes, code locations, and implemented solutions for future reference
- **Issue Reference**: Always include Sentry issue IDs (e.g., WIREDPANDA-J) in commit messages to auto-close issues

## Code Style Standards

- **Trailing Newline**: All source files must end with a single trailing newline byte (`\n`) — POSIX convention, satisfies git's "no newline at end of file" warning, and matches what `black`, `pycodestyle`/`flake8` (W391), and `pylint` (C0305) expect. Enforced by `Scripts/fix_style.py`.
- **Line Trimming**: All lines must have trailing whitespace trimmed (no spaces/tabs at line ends)
- **Documentation Comments** (Doxygen C++ style):
  - Single-line documentation: Use `///` (e.g., `/// Brief description`)
  - Multi-line documentation: Use `/** ... */` (e.g., `/** \brief ... \details ... */`)
  - Do NOT use `//!` style — all instances have been converted to `///`
- **File Types**: Applies to all code files (.cpp, .h, .yml, .yaml, .cmake, CMakeLists.txt, .sh, .py, .js, .ts, .md, etc.)
