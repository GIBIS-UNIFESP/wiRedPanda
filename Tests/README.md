# wiRedPanda Test Suite

Comprehensive test suite for the wiRedPanda digital logic simulator with 132 tests organized hierarchically by test type and complexity level.

## Overview

The test suite is organized into multiple categories to ensure correctness at different levels of abstraction:

- **Integration Tests**: Full circuit testing with IC hierarchies, logic simulation, file operations
- **Unit Tests**: Individual component testing (commands, elements, logic, simulation, UI)
- **System Tests**: High-level system behavior (waveforms, performance)
- **Backward Compatibility**: File format validation across versions
- **Resources**: Asset and icon validation

## Test Statistics

- **Total Tests**: 132
- **IC Hierarchy Tests**: 69 (organized in 9 levels of increasing complexity)
- **Other Integration Tests**: 7
- **Unit Tests**: 45
- **System Tests**: 3
- **Resource Tests**: 1
- **Backward Compatibility**: 7 test files with multiple versions

## Directory Structure

```
Tests/
├── Integration/              # Full circuit and system integration tests
│   ├── IC/Tests/             # Hierarchical IC test suite (69 tests)
│   │   ├── TestLevel1*.cpp   # Basic sequential elements (4 tests)
│   │   ├── TestLevel2*.cpp   # Combinational circuits (12 tests)
│   │   ├── TestLevel3*.cpp   # Mid-complexity circuits (5 tests)
│   │   ├── TestLevel4*.cpp   # Complex circuits (13 tests)
│   │   ├── TestLevel5*.cpp   # Advanced building blocks (11 tests)
│   │   ├── TestLevel6*.cpp   # Multi-bit components (8 tests)
│   │   ├── TestLevel7*.cpp   # CPU subcomponents (8 tests)
│   │   ├── TestLevel8*.cpp   # CPU pipeline stages (4 tests)
│   │   ├── TestLevel9*.cpp   # Full CPU implementations (4 tests)
│   │   ├── Cpu/              # CPU-specific tests (12 integration tests)
│   │   ├── TestsWithoutPanda/# Tests for basic logic without circuit files
│   │   └── CpuTestUtils.h    # Utilities for loading and testing ICs
│   ├── Logic/                # Logic element tests
│   │   └── TestMuxDemuxComprehensive.cpp
│   ├── TestArduino.cpp       # Arduino code generation tests
│   ├── TestFeedback.cpp      # Feedback loop and cycle detection tests
│   ├── TestFiles.cpp         # File I/O and circuit file loading
│   ├── TestIc.cpp            # IC creation, hierarchy, and simulation
│   ├── TestSimulation.cpp    # Core simulation engine tests
│   ├── TestWorkspace.cpp     # Workspace and scene management
│   └── TestWorkspaceFileops.cpp # Workspace file operations
├── Unit/                     # Component-level unit tests
│   ├── Commands/             # Undo/Redo command tests
│   ├── Common/               # Utility and helper tests
│   ├── Elements/             # Individual element behavior
│   ├── Factory/              # Element factory tests
│   ├── Logic/                # Logic element unit tests
│   ├── Nodes/                # Node and port tests
│   ├── Scene/                # Scene graph and serialization
│   ├── Serialization/        # Data serialization tests
│   ├── Simulation/           # Simulation engine unit tests
│   └── Ui/                   # UI component tests
├── System/                   # System-level tests
│   └── TestWaveform.cpp      # Waveform capture and analysis
├── Resources/                # Resource validation
│   └── TestIcons.cpp         # Icon and resource tests
├── Runners/                  # Test harness and runners
│   └── TestWiredpanda.cpp    # Main test executable
├── BackwardCompatibility/    # File format compatibility tests
│   └── [version-folders]/    # Test files from wiRedPanda versions 1.8-4.1.9
└── Common/                   # Shared test utilities
    ├── TestUtils.h/cpp       # Test environment setup, helper functions
    └── CPUTestUtils.h/cpp    # IC loading and CPU-specific utilities
```

## Hierarchical IC Tests (69 tests across 9 levels)

The integration test suite includes 69 tests organized hierarchically by circuit complexity, enabling systematic validation of the simulator's ability to handle increasingly complex nested IC structures.

### Level 1: Basic Sequential Elements (4 tests)
- D Flip-Flop
- JK Flip-Flop
- SR Latch
- D Latch

Basic state-holding elements that form the foundation of sequential logic.

### Level 2: Combinational Circuits (12 tests)
- Half Adder (1-bit)
- Full Adder (1-bit)
- Decoders (2-to-4, 3-to-8, 4-to-16)
- Multiplexers (2-to-1, 4-to-1, 8-to-1)
- Priority Encoder (8-to-3)
- Parity Generator/Checker

Pure logic circuits without state, used as building blocks for higher levels.

### Level 3: Mid-Complexity Circuits (5 tests)
- 1-bit Register
- 4-bit Comparator (equality)
- 4-bit Comparator (full comparison)
- 5-way ALU Selector
- BCD-to-7-Segment Decoder

Combinations of Level 2 circuits with additional control logic.

### Level 4: Complex Circuits (13 tests)
- 4-bit Binary Counter
- 4-bit Johnson Counter
- 4-bit Ring Counter
- 4-bit Register
- 4-bit Shift Registers (PISO, SIPO)
- 4-bit Ripple Adder
- 4-bit Bus Multiplexer (4-bit and 8-bit variants)
- 4-bit Comparator
- 1-bit and 8-bit RAM modules

Multi-bit circuits combining arithmetic, memory, and control elements.

### Level 5: Advanced Building Blocks (11 tests)
- 4-bit Loadable Counter
- 4-bit Up/Down Counter
- 4-bit Modulo Counter
- 4-bit Barrel Shifter/Rotator
- 4-bit Program Counter
- Clock-Gated Decoder
- 4x4 and 8x8 Register Files
- 4-bit Instruction Decoder/Controller

Complex building blocks used in CPU design with advanced control logic.

### Level 6: Multi-bit Components (8 tests)
- 8-bit ALU
- 8-bit Register
- 8-bit Ripple Adder
- 8-bit Program Counter (with arithmetic)
- 8x8 Register File
- 256x8 RAM
- Stack Pointer (8-bit)
- Stack Memory Interface

Scaled-up versions of Level 4-5 components for 8-bit data paths.

### Level 7: CPU Subcomponents (8 tests)
- 8-bit Flag Register
- 8-bit Instruction Register
- 8-bit Instruction Decoder
- 16-bit ALU
- 8-bit Program Counter (CPU variant)
- Instruction Memory Interface
- Data Forwarding Unit
- Execution Data Path

Individual CPU pipeline components designed for integration.

### Level 8: CPU Pipeline Stages (4 tests)
- Fetch Stage
- Decode Stage
- Execute Stage
- Memory Stage

Complete processor pipeline stages ready for multi-cycle CPU assembly.

### Level 9: Full CPU Implementations (4 tests)
- 8-bit Single-Cycle CPU
- 8-bit Multi-Cycle CPU
- 16-bit RISC CPU
- 16-bit Fetch Stage

Complete working CPU implementations demonstrating the entire test hierarchy.

## Running Tests

### Run All Tests
```bash
ctest --preset debug
```

### Run Tests with Verbose Output
```bash
ctest --preset debug --verbose
```

### Run Tests in Parallel
```bash
ctest --preset debug --parallel 4
```

All tests are properly isolated and safe to run in parallel. A 4-core parallel run completes in ~135 seconds.

### Run Specific Test Class
```bash
./build/test_wiredpanda TestClassName
```

### Run Tests Matching a Pattern
```bash
ctest --preset debug -R "Level2"  # Run all Level 2 tests
ctest --preset debug -R "TestIC"  # Run all IC tests
```

### Run with Coverage Analysis
```bash
cmake --preset coverage
cmake --build --preset coverage
ctest --preset debug
```

### Run with Sanitizers
```bash
# Address Sanitizer (detects memory issues)
cmake --preset asan
cmake --build --preset asan
ctest --preset debug

# Thread Sanitizer (detects race conditions)
cmake --preset tsan
```

## Test Organization

### Integration Tests (IC Hierarchy)

The IC hierarchy tests use `CpuTestUtils::loadBuildingBlockIC()` to load pre-built circuit files (.panda files) and test them with various input combinations. This enables:

- **Modular testing**: Each IC is tested independently
- **Hierarchical validation**: Level N tests may use Level N-1 ICs
- **Real circuit files**: Tests use actual wiRedPanda design files
- **Functional verification**: Complete input-output truth table validation

Example test pattern:
```cpp
void TestLevel2HalfAdder::testHalfAdderTruthTable()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create inputs/outputs
    InputSwitch a, b, sum, carry;

    // Load the pre-built Half Adder IC
    IC *adder = loadBuildingBlockIC("level2_half_adder.panda");

    // Connect and test all truth table rows
    // ...
}
```

### Unit Tests

Unit tests validate individual components:
- **Commands**: Undo/Redo functionality (FlipCommand, MorphCommand, ChangeInputSizeCommand, etc.)
- **Elements**: Individual element behavior (AND, OR, NOT, flip-flops, etc.)
- **Factory**: Element creation and type management
- **Serialization**: Data persistence and format compatibility
- **Simulation**: Core simulation engine mechanics
- **Scene**: Graphics and topology management

### Other Integration Tests

- **TestArduino**: Arduino code generation for circuits (88 tests with expected file comparison)
  - Generates and validates Arduino sketches for 24 level 1-4 IC test cases
  - Uses expected file comparison for 5x faster execution (~8s vs ~47s)
  - See "Arduino Test Optimization" section below
- **TestFeedback**: Feedback loops and cycle detection
- **TestFiles**: Circuit file loading and saving
- **TestIc**: IC hierarchy creation and simulation
- **TestSimulation**: Core simulator behavior with various circuits
- **TestWorkspace**: Workspace state management
- **TestWorkspaceFileops**: File operations and persistence

### Backward Compatibility Tests

Located in `Tests/BackwardCompatibility/`, these directories contain .panda files from various wiRedPanda versions:
- 1.8-Beta, 1.9.1
- 2.0-Beta, 2.1-RC
- V2.3, V2.3.3
- V2.4.0, V2.5.1
- V4.0.0-RC, RC2, RC3, RC4
- V4.1.9

These ensure that the simulator correctly loads and handles files from all previous versions.

## Test Utilities

### TestUtils (Common/TestUtils.h/cpp)

Provides the test environment setup and common helpers:
- `setupTestEnvironment()`: Initialize Qt, graphics system, and global properties
- `configureApp()`: Configure application metadata
- `createWorkspace()`: Create isolated test workspace
- `createSwitches()`: Create input switches for testing
- `getInputStatus()`: Retrieve input switch states

### CpuTestUtils (Integration/IC/Tests/CpuTestUtils.h/cpp)

Specialized utilities for IC and CPU testing:
- `loadBuildingBlockIC()`: Load pre-built circuit files
- Truth table generation and validation
- Pin connection helpers
- Simulation stepping and verification

### CircuitBuilder

A fluent helper class for constructing test circuits:
```cpp
CircuitBuilder builder(workspace.scene());
builder.add(&switch1, &switch2, &element, &led);
builder.connect(&switch1, 0, &element, "inputPort");
```

## Test Environment Setup

The test environment is automatically configured by `TestUtils::setupTestEnvironment()`:

1. **Platform Setup**: Headless rendering on Linux
2. **Type Registration**: Qt meta-object registration
3. **Logging**: Disabled verbose output for cleaner test results
4. **Global Configuration**: Test mode enabled, auto-migration disabled for backward compatibility

## Test Patterns and Best Practices

### Circuit Testing Pattern
1. Create isolated `WorkSpace` instance
2. Use `CircuitBuilder` to construct test circuit
3. Load pre-built ICs with `CpuTestUtils::loadBuildingBlockIC()`
4. Connect elements with semantic port labels when available
5. Step simulation with `workspace.scene()->getSimulation()->update()`
6. Verify outputs with assertions

### Truth Table Validation
Tests systematically verify all possible input combinations:
```cpp
for (int i = 0; i < 4; ++i) {  // 2^2 inputs
    // Set inputs based on binary representation of i
    // Step simulation
    // Verify expected output
    QCOMPARE(output, expectedResult);
}
```

### Hierarchical Testing
Higher-level tests may load lower-level ICs as components, validating both the IC and its proper integration into larger circuits.

## Test Infrastructure

### Test Compilation

Tests are compiled as a single executable `test_wiredpanda` with multiple test classes:
```bash
cmake --build --preset debug
```

### Test Execution Framework

Uses **Qt Test Framework** for test execution:
- Per-class `initTestCase()` and `cleanupTestCase()`
- Per-test `cleanup()` for isolation
- QCOMPARE, QVERIFY for assertions
- QFAIL for explicit failures

### Continuous Integration

Tests are executed in CI/CD pipeline (see `.github/workflows/`):
- Run on each commit
- Validate all platforms (Linux, Windows, macOS)
- Generate coverage reports with `--coverage` preset
- Detect memory issues with Address/Thread Sanitizers

## Debugging Tests

### Run Single Test
```bash
./build/test_wiredpanda TestClassName::testMethodName
```

### Verbose Output
```bash
ctest --preset debug --verbose -R TestName
```

### Debug in GDB
```bash
gdb ./build/test_wiredpanda
(gdb) run TestClassName::testMethodName
```

### Memory Leak Detection
```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset debug -R "TestName"
```

## Adding New Tests

### Create Test Class
1. Create `.h` and `.cpp` files in appropriate `Tests/` subdirectory
2. Inherit from `QObject`
3. Define test methods as private slots

```cpp
// Tests/Integration/IC/Tests/TestLevel10NewCircuit.h
#pragma once

#include <QtTest/QtTest>

class TestLevel10NewCircuit : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void testNewCircuitBehavior();
};
```

### Register Test in CMakeLists.txt

Add to `Tests/CMakeLists.txt`:
```cmake
qt6_add_executable(test_wiredpanda
    # ... existing tests ...
    Tests/Integration/IC/Tests/TestLevel10NewCircuit.h
    Tests/Integration/IC/Tests/TestLevel10NewCircuit.cpp
)

qt_add_test(test_wiredpanda NAME TestLevel10NewCircuit)
```

### Use Test Utilities
```cpp
void TestLevel10NewCircuit::testNewCircuitBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Build test circuit
    // Run assertions
}
```

## Arduino Test Optimization

The TestArduino suite uses **expected file comparison** instead of arduino-cli compilation for 5x faster execution:

### How It Works

1. **Expected files**: 24 pre-generated Arduino sketches stored in `Tests/Integration/Arduino/`
2. **Normal runs**: Compare generated code against expected files (instant, no compilation)
3. **First-time or regeneration**: Fall back to arduino-cli validation if expected files don't exist

### Regenerating Expected Files

If Arduino codegen changes, regenerate expected files:
```bash
GENERATE_EXPECTED_ARDUINO=1 ./build/test_wiredpanda TestArduino
```

This:
- Generates all 24 expected .ino files in `Tests/Integration/Arduino/`
- Tests pass and files are automatically saved
- Commit the updated files to git

### Performance Impact

- **Before**: ~47 seconds (arduino-cli compilation for each of 24 test cases)
- **After**: ~8 seconds (file comparison, no compilation)
- **Improvement**: 5.8x faster

## Performance Considerations

- **Fast tests**: Unit tests and logic tests execute in milliseconds
- **Medium tests**: IC hierarchy tests execute in seconds (complex circuits)
- **Slow tests**: CPU integration and memory interface tests may take 10-20 seconds
- **Arduino tests**: TestArduino runs in ~8 seconds using expected file comparison (5x faster than arduino-cli compilation)

## Known Limitations

- **Headless rendering**: Tests run without graphics display (QT_QPA_PLATFORM=offscreen on Linux)
- **No timing simulation**: Tests verify logic correctness, not propagation delays
- **Simplified I/O**: No audio/video features tested
- **No UI tests**: Only programmatic circuit manipulation tested (no mouse/keyboard events)

## Contributing

When adding new tests:
1. Follow the hierarchical level organization for IC tests
2. Use `CircuitBuilder` for consistency
3. Test complete truth tables for combinational circuits
4. Verify edge-triggered behavior for sequential elements
5. Add comments documenting the circuit purpose and test strategy
6. Ensure tests are deterministic and isolation-friendly
7. Run all tests before submitting PR
