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
- **Exercise/Tour content translations**: Exercise step text (`App/Resources/Exercises/*.json`) never passes through `tr()` — it's JSON, so `lupdate` can't see it. It goes through a *separate* Weblate "JSON file" component and a generated catalog, `App/Resources/Translations/ExerciseTour/en.json` (`Scripts/generate_exercise_tour_catalog.py`). Regenerating it is wired into the same `update_translations` target above — still one command. See "Exercise Content" below.

## Project Structure

- Main project file: `CMakeLists.txt`
- App code: `App/` directory
- Tests: `Tests/` directory with comprehensive test suite
- Test executable: single unified `test_wiredpanda` binary with 176 test classes (run via `ctest --preset debug`)

## Digital Logic Simulation

### Simulation Type: Hybrid Synchronous Cycle-Based with Event-Driven Clocks

- **Primary Model**: Synchronous cycle-based simulation with fixed 1ms update intervals
- **Secondary Model**: Event-driven clock elements with real-time timing
- **Design Goal**: Educational simplicity prioritizing correctness over timing accuracy

### Core Architecture (`App/Simulation/Simulation.cpp`)

- **Fixed Update Cycle**: 1ms intervals via QTimer for consistent simulation steps
- **Sequential Update Phases** per cycle:
  1. Update input elements (`updateOutputs()`)
  2. Update all logic elements (`updateLogic()`)
  3. Update connections (`updatePort()`)
  4. Update output elements
- **Topological Sorting**: Elements ordered by dependency depth for correct propagation

### Logic Element Behavior

- **Combinational Logic**: Zero-delay immediate updates (AND, OR, NOT, etc.)
- **Sequential Logic**: Synchronous state changes on clock edges (flip-flops, latches)
- **No Propagation Delays**: Logic gates update instantaneously

### Timing Characteristics

- **Clock Elements**: Only true event-driven components with configurable frequencies
- **Logic Gates**: Immediate response, no timing simulation
- **Sequential Elements**: Edge-triggered state changes without setup/hold timing
- **Update Order**: Priority-based topological sorting prevents race conditions

### Code Evidence

```cpp
// Fixed 1ms simulation cycle (App/Simulation/Simulation.cpp)
m_timer.setInterval(1ms);

// Immediate combinational logic with 4-state Status (And::updateLogic)
if (!simUpdateInputsAllowUnknown()) {
    return;
}
setOutputValue(StatusOps::statusAndAll(simInputs()));  // Zero delay

// Real-time clock timing (Clock::updateOutputs) — advance by exactly one
// half-period so accumulated drift doesn't skew the frequency
if (elapsed > m_interval) {
    m_startTime += m_interval;
    setOn(!m_isOn);
}
```

### Implementation Classification

- **Abstraction Level**: Functional simulation (no timing details)
- **Update Model**: Synchronous with topological ordering
- **Delay Model**: Zero-delay for logic, real-time for clocks
- **Target Audience**: Educational/demonstration use
- **IMPORTANT**: Always prefer fixing code logic over changing tests to conform to incorrect behavior

### Conceptual Correctness Assessment

#### ✅ **Educationally Sound Design**

- **Boolean Logic**: Correctly implements all fundamental logic operations
- **Combinational Circuits**: Proper dependency ordering via topological sorting
- **Sequential Logic**: Accurate edge-triggered state machine behavior
- **Circuit Topology**: Correct signal flow and causality relationships
- **Functional Verification**: Reliable testing of digital logic concepts

#### ✅ **Correct Abstractions for Learning**

- **Zero-delay model** eliminates timing complexity for beginners
- **Immediate feedback** enhances educational interaction
- **Race condition prevention** via priority-based update ordering

#### ⚠️ **Deliberate Real-World Omissions**

- **No propagation delays**: Real gates have nanosecond delays
- **No setup/hold constraints**: Real flip-flops need timing margins
- **No hazards/glitches**: Real circuits can have temporary incorrect outputs
- **No clock domain issues**: Real systems have multiple clocks and skew
- **No physical limitations**: Missing fan-out, drive strength, power concerns

#### 🎯 **Educational Target Alignment**

- **Perfect for**: Boolean algebra, combinational design, sequential concepts
- **Not intended for**: Timing analysis, synchronization, physical implementation
- **Design Philosophy**: Teach logic functionality before implementation complexity

#### **Verdict: Conceptually Correct for Educational Purpose**

The simulation accurately represents **ideal digital logic behavior** while deliberately abstracting **physical implementation details**. This approach is pedagogically sound - students learn fundamental concepts correctly without being overwhelmed by timing complexities that would obscure the core logic principles.

## Exercise Content

**Exercises** (`App/Exercise/`): circuit-building challenges validated against the live `Scene`, driven by JSON step content (`App/Resources/Exercises/*.json`).

### Content discovery: flat directory scan, not hardcoded lists

`ExerciseBrowserDialog` calls `ExerciseTourResources::discover("Exercises")` (`App/Core/ExerciseTourResources.h`), which merges four sources rather than one: the built-in bundled content (`scan(":/Exercises")`, mirroring `LanguageManager::availableLanguages()`'s `QDir(prefix).entryList({"*.json"}, QDir::Files)` pattern) plus three real, on-disk, end-user-writable locations — see below. Dropping a new built-in `.json` file into `App/Resources/Exercises/` (with a globally unique `id`) is picked up automatically on the next reconfigure — no C++ or `.qrc` edits.

### End-user-writable discovery: three locations, two audiences

Built-in content only solves *contributor* flexibility (source-tree + reconfigure). Getting a
custom exercise into an already-*compiled* app needs a real filesystem location, and this
splits by audience — deliberately kept separate, not merged into one "user folder":

- **`ExerciseTourResources::preferredContentDir(category)`** — what the **"Open Folder" button**
  in the browser dialog opens: the install-relative folder next to the app (mirrors
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

Exercise JSON is intentionally **not** listed in a `.qrc` (`rcc` can't glob, which would reintroduce the hardcoded-list problem). `CMakeLists.txt` globs the folder with `CONFIGURE_DEPENDS` and calls the target-based `qt6_add_resources(wiredpanda_resources "<name>" PREFIX ... BASE ... FILES ...)` API directly on the `wiredpanda_resources` OBJECT library, after it exists.

### i18n: a separate Weblate "JSON file" component

Step text never passes through `tr()`. `App/Resources/Translations/ExerciseTour/en.json` is the generated, committed English-source catalog (`Scripts/generate_exercise_tour_catalog.py`), keyed `<fileId>.title`, `<fileId>.description`, `<fileId>.<stepKey>.instruction`/`.hint`. Every translatable step has a stable, contributor-authored `"key"` used only to build these keys — never read by engine logic. `ExerciseTour/<lang>.json` files are Weblate-managed and embedded at `:/i18n/ExerciseTour/<lang>.json`. `ExerciseTourResources::translate(key, fallbackEnglish)` reloads the small catalog fresh on every call (no cache) — cheap given usage frequency (dialog open / exercise start), and always falls back to the raw English text.

See `App/Resources/Exercises/README.md` for the full schema and the closed `click` vocabulary.

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
