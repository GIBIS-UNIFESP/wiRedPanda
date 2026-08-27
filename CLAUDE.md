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
- Test executable: single unified `test_wiredpanda` binary with 192 test classes (run via `ctest --preset debug`)

## Digital Logic Simulation

### Simulation Type: Unified Event-Driven Engine (functional + temporal modes)

- **One engine, two modes**: a single event-driven drain (`Simulation::processEvents()`)
  reproduces the classic zero-delay behavior AND simulates real propagation delays. The ONLY
  difference between modes is the per-element delay applied when scheduling: functional mode
  forces every delay to 0; temporal mode uses `propagationDelay()`. There is no separate
  functional code path.
- **Functional mode** (`m_timePerTick == 0`, the default): every event lands at the current
  instant, so the drain degenerates to a full zero-delay settle — combinational and feedback
  circuits converge to a fixed point now. This is the educational default.
- **Temporal mode** (`m_timePerTick > 0`): events spread across future timestamps by
  per-element propagation delay, drained from a time-ordered `EventQueue`.
- **One flat netlist**: ICs do NOT simulate themselves. `Simulation::initialize()` flattens
  the IC hierarchy — every internal primitive (recursively, through nested ICs) joins the
  top-level netlist, with IC boundary ports spliced to their boundary Nodes. So per-element
  delays apply uniformly, including inside ICs, and internal signals are watchable.
- **Glitch-free by phase separation**: each timestamp runs three regions — **ACTIVE**
  (combinational settles to a fixed point via delta cycles), **SAMPLE** (Memory-group elements
  read their now-settled inputs and stage), **PUBLISH** (staged values apply and wake their
  readers). A flip-flop therefore never samples a cone that is still rippling, so
  internally-generated (e.g. gated) clocks reach their final value before anything downstream
  samples them. Events drain in `(time, kind, priority-desc)` order, but the topological
  priority is a tie-break, not the correctness mechanism: within the active region every
  evaluation reads only published values, so the result is order-independent.
- **Design Goal**: Educational simplicity (functional mode) without giving up the option to
  show propagation-delay/timing behavior (temporal mode).

### Core Architecture (`App/Simulation/Simulation.cpp`)

- **Fixed Update Cycle**: 1ms `QTimer` tick drives `update()`.
- **`update()` phases per tick**:
  1. Advance event-driven clocks (`updateClock()`, real wall-clock time)
  2. Propagate user inputs (`updateOutputs()`)
  3. Evaluate logic via `processEvents()` — the unified engine (both modes)
  4. Push values onto wires / refresh output visuals (throttled to display rate)
- **Unified drain, publish-side delay**: `processEvents()` drains `m_eventQueue` over
  `[currentTime, currentTime + timePerTick]` (`SimEvent`/`SimTime` in `App/Simulation/`).
  The delay is paid on **publication**, not on waking: an element evaluates *immediately*
  when an input it reads is published, stages its result, and schedules its own `Publish` at
  `t + its own delay` (0 in functional mode). This is what lets it sample its inputs at the
  instant they actually changed. A per-timestamp evaluation cap detects oscillation and
  canonicalizes feedback nodes to `Unknown`. Blocking and deterministic.
- **Inertial delay by supersession**: only one `Publish` per element is live at a time —
  scheduling another bumps a per-element generation stamp and the stale one is dropped when
  popped. An input that reverts within the delay re-stages the already-published value, so
  the superseding publish changes nothing and the pulse is absorbed. No value plumbing, and
  a pulse strictly **narrower** than the delay is absorbed; one exactly as wide as the delay
  (`w == delay`) **propagates**, which is the conventional inertial-delay rule. Pinned by
  `testPropertyPulseExactlyEqualToDelayPropagates`.
- **IC flattening**: `Simulation::initialize()` builds one flat primitive netlist via
  `collectFlatElements()` + `spliceICBoundaries()`; `IC::initializeSimulation()` only wires
  an IC's internal connections (it does not settle). `mirrorICOutputValues()` copies each IC's
  settled internal outputs onto its external ports for wire rendering.
- **Topological Sorting**: `topologicalSort()` orders elements by dependency depth and
  detects feedback loops; `sortSimElements()` builds the flat successor graph from output
  connections (with IC boundaries mapped to boundary Nodes) so adjacency order matches the
  priority system.
- **Non-blocking sequential commit**: synchronous elements (`ElementGroup::Memory`) are held
  out of the active region and sampled in the SAMPLE phase — they all read before any of them
  publishes, which is exactly SystemVerilog's non-blocking (`<=`) model. Commits are ordinary
  `Publish` events, so nothing has to re-propagate them afterwards: there is no tick-wide
  commit bracket and no post-edge re-settle pass. A ripple chain advances by element delays
  within a tick rather than one hop per tick.
- **Phase separation is load-bearing mainly at zero delay**: with any non-zero delay,
  publication at `edge + delay` already orders after every evaluation at the edge instant.
  The explicit phases are what preserve functional mode's simultaneity.

### Logic Element Behavior

- **Combinational Logic**: zero-delay in functional mode; inertial propagation delay in
  temporal mode (a pulse shorter than the gate delay is absorbed).
- **Sequential Logic**: edge-triggered state changes (flip-flops, latches), committed
  non-blocking (see above) so combinational logic between them reads pre-tick state.
- **Propagation delay** (temporal mode): `GraphicElement::propagationDelay()` returns a
  per-element override or a per-type default (NOT 5 ns, AND/OR 10 ns, NAND/NOR 8 ns,
  sequential ~15-20 ns; sources/sinks/nodes/ICs 0). `Simulation::initialize()` seeds
  `m_delays` from it; `setElementDelay()` overrides at runtime.

### BeWavedDolphin temporal mode

The waveform editor's column sweep renders delays directly, independently of the main
window's mode (a sweep never inherits the live window). Its own status bar carries the same
**Functional / Temporal** selector, plus a ns/column resolution shown only while temporal;
together they make `run()` bracket its sweep in
`Simulation::beginTimedRun()`/`endTimedRun()`, so each column advances that much sim-time
and a gate's propagation delay renders as **column-lag** — an output transition appears
`≈ delay / ns-per-column` columns after its cause. The bracket is unconditional: a
non-temporal sweep runs at 0 ns/tick rather than inheriting whatever window the shared
`Simulation` was left in. Mode and resolution are session-only view settings — the
`.dolphin` format stores inputs only, and outputs are always recomputed on load.

Because the model is *inertial*, a pulse narrower than a gate's delay is absorbed
entirely: at a resolution fine enough to show lag, an input row toggling every column
drives its outputs flat. That is correct behavior, and the toggle says so in the status bar.

### Timing Characteristics

- **Clock Elements**: real-time, wall-clock driven (independent of sim mode).
- **Logic Gates**: immediate (functional) or delayed by `propagationDelay()` (temporal).
- **Sequential Elements**: edge-triggered; no setup/hold modeling.
- **Update Order**: events drain one at a time in topological-priority order within each
  timestamp, so upstream logic settles before downstream samples it — preventing race
  conditions and zero-delay clock glitches.

### Code Evidence

```cpp
// 1 ms tick drives update()
m_timer.setInterval(1ms);

// Publish-side delay: evaluate NOW (stage), publish one own-delay later.
// Functional mode is the same path with every delay forced to 0.
element->beginDeferredCommit();
element->updateLogic();
schedulePublish(t + delayTo(element), element);

// Sequential elements are held back and sampled only once the active region settles,
// then all publish together — non-blocking semantics without a commit bracket.
for (auto *element : std::as_const(pendingSamples)) {
    element->beginDeferredCommit();
    element->updateLogic();
    schedulePublish(t + delayTo(element), element);
}

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

There is no separate browser dialog class — `MainWindow::populateContentMenu()` rebuilds the
**Learn → Exercises**/**Tours** submenu every time it's about to show (`QMenu::aboutToShow`,
wired up in `MainWindow::setupExercisesMenu()`/`setupToursMenu()`), calling
`ExerciseTourResources::discover("Exercises")`/`discover("Tours")` (`App/Core/ExerciseTourResources.h`)
each time. `discover()` merges four sources rather than one: the built-in bundled content
(`scan(":/Exercises")`, mirroring `LanguageManager::availableLanguages()`'s
`QDir(prefix).entryList({"*.json"}, QDir::Files)` pattern) plus three real, on-disk,
end-user-writable locations — see below. Dropping a new built-in `.json` file into
`App/Resources/Exercises/` or `App/Resources/Tours/` (with a globally unique `id`) is picked up
automatically on the next reconfigure — no C++ or `.qrc` edits.

### End-user-writable discovery: three locations, two audiences

Built-in content only solves *contributor* flexibility (source-tree + reconfigure). Getting a
custom exercise or tour into an already-*compiled* app needs a real filesystem location, and
this splits by audience — deliberately kept separate, not merged into one "user folder":

- **`ExerciseTourResources::preferredContentDir(category)`** — what the **"Open My
  Exercises/Tours Folder"** menu action (added by `populateContentMenu()` at the top of each
  submenu) opens: the install-relative folder next to the app (mirrors
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
  - GCC 13+ with C++20 support (Ubuntu 24.04's default `build-essential`), Qt 6.9.3 pinned
  - CMake and build tools pre-configured
  - VS Code extensions for C++/Qt development
- **Usage**: Open project in VS Code and select "Reopen in Container"
- **Testing**: Supports headless test execution

## Release Pipeline (`.github/workflows/deploy.yml`)

Not part of the dev container — this runs on CI when cutting a release:

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
