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
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
# Build
cmake --build build --target wiredpanda
# Tests
cmake --build build --target wiredpanda-test
```

**Windows Qt Path**: Add `-DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64"` to configure command if needed.
- **mold linker**: Modern fast linker installed (`sudo apt install mold`) - automatically used by CMake when available
- **Visual Studio BuildTools**: `"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"`

### Development Features

Advanced development features supported:

**Coverage Analysis**:

- `cmake -B build -DENABLE_COVERAGE=ON` (sets `--coverage` flags for GCC/Clang)

**Sanitizers**:

- Address Sanitizer: `cmake -B build -DENABLE_ADDRESS_SANITIZER=ON`
- Thread Sanitizer: `cmake -B build -DENABLE_THREAD_SANITIZER=ON`
- Memory Sanitizer: `cmake -B build -DENABLE_MEMORY_SANITIZER=ON` (Clang only)
- UB Sanitizer: `cmake -B build -DENABLE_UB_SANITIZER=ON`

**Windows Metadata**: Comprehensive application properties for professional deployment

### Testing

- Project uses Qt Test framework
- **IMPORTANT**: Always stay in project root directory - don't cd to build/
- **Test execution (Linux/DevContainer)**: Always run in headless mode from project root:

  ```bash
  QT_QPA_PLATFORM=offscreen ./build/wiredpanda-test
  ```

- **Test execution (Windows after windeployqt)**: From `build` directory:

  ```powershell
  powershell -Command "Start-Process -FilePath 'wiredpanda-test.exe' -Wait -NoNewWindow"
  ```

- **Note**: Direct bash execution fails, cmd causes segfaults. PowerShell is the reliable method on Windows.

## Project Structure

- Main project file: `CMakeLists.txt`
- App code: `app/` directory
- Tests: `test/` directory with comprehensive test suite
- Test executable: `wiredpanda-test`

## Digital Logic Simulation

### Simulation Type: Hybrid Synchronous Cycle-Based with Event-Driven Clocks

- **Primary Model**: Synchronous cycle-based simulation with fixed 1ms update intervals
- **Secondary Model**: Event-driven clock elements with real-time timing
- **Design Goal**: Educational simplicity prioritizing correctness over timing accuracy

### Core Architecture (`app/simulation.cpp`)

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
// Fixed 1ms simulation cycle
m_timer.setInterval(1ms);

// Immediate combinational logic (LogicAnd)
const auto result = std::accumulate(inputs, true, std::bit_and<>());
setOutputValue(result);  // Zero delay

// Real-time clock timing
if (elapsed > m_interval) {
    setOn(!m_isOn);  // Toggle based on frequency
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

## Development Container

- **Ubuntu 22.04 LTS** based development environment
- **Location**: `.devcontainer/` directory with full configuration
- **Features**:
  - Qt 5.15 development environment
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

- **Trailing Newlines**: All source files must end with a trailing newline character
- **Line Trimming**: All lines must have trailing whitespace trimmed (no spaces/tabs at line ends)
- **File Types**: Applies to all code files (.cpp, .h, .yml, .yaml, .cmake, CMakeLists.txt, .sh, .py, .js, .ts, .md, etc.)
