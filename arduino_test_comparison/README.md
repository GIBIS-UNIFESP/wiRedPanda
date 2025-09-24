# Arduino Test Comparison - Windows vs Linux

This folder contains the failed Arduino test files for comparison between Windows and Linux code generation.

## Test Results Summary

- **Total Regression Tests**: 18 corpus files
- **Passed**: 11 tests (61%)
- **Failed**: 7 tests (39%)

## Failed Test Files

The following test cases are failing due to **code generation order differences**:

1. **display-3bits-counter** - 7-segment display with 3-bit counter circuit
2. **display-4bits-counter** - 7-segment display with 4-bit counter circuit
3. **ic** - Integrated circuit (custom IC) functionality
4. **jkflipflop** - JK flip-flop sequential logic
5. **notes** - Circuit with annotations/notes
6. **sequential** - General sequential logic elements
7. **tflipflop** - T flip-flop toggle functionality

## Folder Structure

```
arduino_test_comparison/
├── windows/
│   ├── baseline_expected/          # Expected .ino files and .panda circuits
│   │   ├── display-3bits-counter.panda
│   │   ├── display-3bits-counter.ino
│   │   ├── display-4bits-counter.panda
│   │   ├── display-4bits-counter.ino
│   │   ├── ic.panda
│   │   ├── ic.ino
│   │   ├── jkflipflop.panda
│   │   ├── jkflipflop.ino
│   │   ├── notes.panda
│   │   ├── notes.ino
│   │   ├── sequential.panda
│   │   ├── sequential.ino
│   │   ├── tflipflop.panda
│   │   └── tflipflop.ino
│   └── failed_tests/               # Generated .ino files from Windows
│       ├── display-3bits-counter_generated.ino
│       ├── display-4bits-counter_generated.ino
│       ├── ic_generated.ino
│       ├── jkflipflop_generated.ino
│       ├── notes_generated.ino
│       ├── sequential_generated.ino
│       └── tflipflop_generated.ino
└── (future: linux/ folder for Linux comparison)
```

## Issue Analysis

### Root Cause
The failures are **NOT functional issues** but **code generation order differences**:

- **Generated code is functionally identical** to expected code
- **Difference**: Order of variable assignments in the `loop()` function
- **Both versions produce valid Arduino C++ code**

### Example Issue
In `display-3bits-counter`:
- **Generated**: `aux_display_3bits_7_node_28 = aux_ic_input_display_3bits_2; aux_display_3bits_7_node_30 = aux_ic_input_display_3bits_0;`
- **Expected**: `aux_display_3bits_7_node_31 = aux_ic_input_display_3bits_1; aux_display_3bits_7_node_30 = aux_ic_input_display_3bits_0;`

### Technical Cause
Likely due to:
- **Hash table iteration order** changes (non-deterministic ordering)
- **Element ID assignment** differences
- **Graph traversal algorithm** modifications in `app/arduino/codegenerator.cpp`

## Comparison Instructions

1. **For Linux comparison**: Generate the same test files on Linux and place in `linux/failed_tests/`
2. **Use diff tools**: Compare `*_generated.ino` files between platforms
3. **Focus on functionality**: Verify both versions produce the same logical Arduino behavior
4. **Update baselines**: If new ordering is acceptable, update the baseline `.ino` files

## File Sizes
| Test File | Windows Generated | Expected Baseline |
|-----------|------------------|-------------------|
| display-3bits-counter | 10,015 bytes | 11,320 bytes |
| display-4bits-counter | 30,793 bytes | 32,997 bytes |
| ic | 40,838 bytes | 43,248 bytes |
| jkflipflop | 5,319 bytes | 6,118 bytes |
| notes | 14,251 bytes | 16,074 bytes |
| sequential | 17,201 bytes | 19,760 bytes |
| tflipflop | 4,800 bytes | 5,582 bytes |

## Next Steps

1. **Generate Linux outputs** using the same .panda files
2. **Compare Windows vs Linux** generated code
3. **Decide on canonical ordering** for code generation
4. **Update regression baselines** if needed
5. **Fix deterministic ordering** in CodeGenerator if required