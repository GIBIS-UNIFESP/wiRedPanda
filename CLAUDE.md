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

## Building and testing

What no single change owns: the pitfalls of building and testing this code base, and how to avoid each.

### Builds

- A debug build hides GCC's optimization-only diagnostics, so checks build the relwithdebinfo preset; CI builds release
  at `-O3 -Werror` with a strict GNU warning set (`-Wconversion`, `-Wold-style-cast`, `-Wuseless-cast`, `-Wshadow`,
  `-Wnull-dereference`, …). What GCC 15 at `-O2` raises, with the fix each needs:
  - `-Wnull-dereference` after `qgraphicsitem_cast` following a type test: cast in the condition,
    `if (auto *x = qgraphicsitem_cast<T *>(item))`; and on a conversion to a second base, which is null-checked: a
    comparator on the class's own type.
  - `-Wuseless-cast` on a cast of a `qsizetype` to itself; `-Wshadow` between a loop variable and an `if` initializer.
  - `-Werror=stringop-overflow` on `QVector<int>(count, -1)` when `count` may be negative: use `std::max(0, count)`.
    GCC reports it at the Qt header's line, so a pragma cannot target it.
  - `-Werror=sign-conversion` when casting `reserve()` sizes: cast to `std::size_t`.
- Unity builds put 8 sources in one translation unit, so names in anonymous namespaces and file-scope statics collide
  across neighbours: scan each batch, or put helpers in named namespaces. Adding a source shifts later batches, which
  can hide or reveal a missing include, and each target's precompiled header also supplies includes a source lacks.
  So a changed source is compiled alone: its flags from `compile_commands.json`, minus the unity batch and the
  `-include …/cmake_pch.hxx`, with `-fsyntax-only`. The check's negative controls, a source missing a project include
  and one using a Qt class only the precompiled header includes, must both fail. CI also builds without unity (the
  sanitizer presets) and with one large unity batch.
- GCC passes test code that clang rejects under `-Werror`: `QWARN` (`QTest::qWarn`, deprecated since Qt 6.3; GCC does
  not warn inside a system header's macro expansion), a `float` passed as a `qreal` argument (clang's
  `-Wdouble-promotion`), and a lambda init-capture named after the structured binding it copies (`[inputs = inputs]`,
  clang's `-Wshadow`). CI's macOS jobs build the tests with Apple clang; nothing checks clang locally, and ASan and
  UBSan are left to CI's Sanitizers workflow.
- CI's Debian i386 job has a 32-bit `qsizetype`, so a `qint64` such as `QIODevice::pos()` passed where a `qsizetype` is
  taken narrows (`-Wconversion`), while a `static_cast<qsizetype>` is a useless cast on 64-bit Qt 6.9, where
  `qsizetype` is `qint64`: a value known to be small is cast to `int`.
- The libFuzzer build sees includes the GCC build hides, such as a header using `QPointF` and `QTransform` without
  including them.
- Build all targets before running the suite: the test target alone leaves out the one that copies the MCP schema, and
  twelve tests fail.
- Long runs (sweeps, oracles, timings) use an optimized build, one case per core, never a debug build.
- CI runs pylint over `Scripts/`, `Tests/Fixtures/` and the IC generators; the MCP client is checked with ruff, pyright
  and mypy, which are run before pushing a change to it.
- `Scripts/fix_style.py` runs over the C++ sources before every push: it orders the includes (the related header, then
  the standard library, then Qt and third parties, then the project's, each group sorted without regard to case),
  collapses runs of blank lines, and ends each file with one newline. It rewrites files in place and has no `--help`
  (an unknown option still runs it), so its changes are read before they are kept.

### Translatable strings

The translatable-strings golden compares `build/translatable_strings.ts`, which only its own target refreshes, so a
change that adds or removes `tr()` strings builds that target and updates the golden in the same commit; a build that
skips it compares a stale catalog and passes. A class that moves or is renamed keeps its strings' translation context,
or every language silently falls back to English. The `.ts` catalogs themselves are refreshed at release, not
mid-branch.

### Tests

- A new QTest class is registered in four places, each of which fails silently when missed: the source list in
  `CMakeSources.cmake` (sorted); the runner's include and registry entry in `Tests/Runners/TestWiredpanda.cpp` (without
  it the class does not exist for `test_wiredpanda`); an `add_test` line and the label list in `CMakeLists.txt` (without
  the label, `ctest -L` skips it); and the `ALL_TESTS` list, whose fixture makes the GUI tests wait for every other
  test.
- Every new check is broken on purpose once (an order reversed, a render skipped, a fixed defect put back) and must
  fail before it is trusted.
- Suite defaults: `Application::renderingEnabled` is off, so wires have no paths; tests start on the System theme; the
  first simulation update after a structural edit rebuilds the simulation and starts every new clock HIGH (a rebuild
  keeps an existing clock's level; `Simulation::restart()` starts them all afresh), so a test of a fast path builds
  first; an InputSwitch does not rotate its graphic; `qHash` is seeded per process, so seeds use
  FNV-1a; two random draws in one expression need an explicit order; fonts differ per platform (Noto, DejaVu and
  Liberation Sans all pass, or the font-dependent value is masked); `QDrag::exec()` cannot be driven, so drops are sent
  as events; clipboard tests hold the `clipboard` resource lock.
- `InputSwitch::setOff()` and the no-argument `setOn()` both toggle, by design (a switch toggles its latched state);
  only `setOn(bool)` sets a level. A test that "clears" a switch with `setOff()` turns an off switch on.
- Goldens regenerate when `GENERATE_EXPECTED_SNAPSHOTS` is set, even to an empty value, so nothing that runs the suite
  may export it.
- QtTest's watchdog aborts a slot after 300 s and its stack dump can hang the process; long slots run with
  `QTEST_FUNCTION_TIMEOUT` raised and `QTEST_DISABLE_STACK_DUMP=1`. QTest drops every message after 2,000 warnings, so
  a log that is counted needs `-maxwarnings 0`, and a check for "Maximum amount of warnings exceeded".
- Tests of loops and oscillators have timeouts, so a mutant that makes a ring run forever fails instead of hanging.
- A test class's name can differ from its file's (`TestLevel6Alu8bit.cpp` holds `TestLevel6ALU8Bit`), and
  `test_wiredpanda` takes the class name. Fixture `.panda` files are read at run time, so changing one needs no rebuild;
  a test source does.
- No absolute wall-time bounds in tests (ratios or counts instead), and CTest labels that match what the test does.

### Fuzzing

The `fuzzer` preset is libFuzzer with ASan and UBSan. Only each harness's own source gets `-fsanitize=fuzzer-no-link`,
so libFuzzer sees the harness's edges alone (154 edges in three minutes, against 27,474 with the libraries
instrumented); coverage feedback needs the flag on the whole project. A new harness is registered in both fuzzer
presets, the unattended-run script and the coverage report, with its own corpus directory; each seeded harness checks
its seed at start-up and runs once with `-runs=1` as a smoke test; `Tests/lsan_suppressions.txt` silences leaks in
`libfontconfig` and `libexpat`. No CI job runs the harnesses, and CI's Sanitizers workflow ignores changes under
`Tests/Fuzz/`: they run overnight on a developer's machine, through `Tests/Fuzz/run-unattended.sh`, and a change that
adds or changes a harness lands after that run.

### Profiling

Instruction counts under callgrind on a relwithdebinfo build, through `Scripts/profile_load.sh`; rendering
compared by CPU profile, not frames per second; GPU frames timed by blocking on the presented frame; high-frequency
events counted with `perf` or counters, not debugger breakpoints. Wall-clock and CPU numbers on a virtual machine vary
12 to 54% from run to run, so a single run proves nothing: runs are interleaved in pairs and only a direction that holds
across all of them is reported, or instructions are counted instead. A running application's CPU share is measured on
a settled process, attached after the load has finished (`perf stat -e task-clock -p`), since `ps`'s CPU figure is an
average since launch that keeps falling for a minute or more as the start-up cost dilutes, and two programs are compared
over matched settle windows.

## Project Structure

- Main project file: `CMakeLists.txt`
- App code: `App/` directory
- Tests: `Tests/` directory with comprehensive test suite
- Test executable: single unified `test_wiredpanda` binary with 192 test classes (run via `ctest --preset debug`)

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
