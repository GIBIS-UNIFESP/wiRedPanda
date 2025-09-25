# wiRedPanda Verilog Test Suite

This directory contains Verilog HDL baseline files generated from the Arduino test circuits in `test/arduino/`. These serve as reference implementations for testing and validation.

## Generated Files

### ✅ Successfully Generated (12 files)

| File | Size | Description |
|------|------|-------------|
| `counter.v` | 3,097 bytes | Counter circuit with JK flip-flops |
| `decoder.v` | 5,790 bytes | BCD to 7-segment decoder |
| `dflipflop-masterslave.v` | 2,264 bytes | Master-slave D flip-flop implementation |
| `display-3bits-counter.v` | 5,253 bytes | 3-bit counter with 7-segment display |
| `display-3bits.v` | 7,161 bytes | 3-bit display driver |
| `display-4bits.v` | 9,592 bytes | 4-bit display driver |
| `ic.v` | 12,442 bytes | Complex IC with multiple clocks and 7-segment display |
| `input.v` | 1,967 bytes | Basic input/output test circuit |
| `notes.v` | 5,372 bytes | Musical note generator circuit |
| `register.v` | 2,854 bytes | Register implementation |
| `sequential.v` | 8,134 bytes | Sequential logic test circuit |
| `serialize.v` | 5,921 bytes | Serialization logic circuit |

### ❌ Failed to Generate (6 files - Circular Dependencies)

| File | Issue | Reason |
|------|-------|---------|
| `dflipflop.v` | 0 bytes | Circular dependency in circuit topology |
| `dflipflop2.v` | 0 bytes | Circular dependency in circuit topology |
| `display-4bits-counter.v` | 0 bytes | Circular dependency in circuit topology |
| `dlatch.v` | 0 bytes | Circular dependency in circuit topology |
| `jkflipflop.v` | 0 bytes | Circular dependency in circuit topology |
| `tflipflop.v` | 0 bytes | Circular dependency in circuit topology |

## Analysis

### Code Quality
The generated Verilog follows professional IEEE 1364 standards with:
- Comprehensive module headers with resource usage estimates
- FPGA target board selection (Generic-Small selected for all circuits)
- Proper signal declarations and assignments
- Sequential logic implemented with always blocks
- Combinational logic implemented with continuous assignments

### Validation Differences
The Verilog generator implements stricter circuit validation than the Arduino generator:
- **Topological sorting**: Detects and prevents circular dependencies
- **Resource estimation**: Calculates LUT, flip-flop, and I/O requirements
- **FPGA compatibility**: Validates against target device constraints

### Generated Code Features
Each Verilog module includes:
- Professional module headers with generation metadata
- Target FPGA information (Generic-Small: 1000 LUTs, 1000 FFs, 50 IOs)
- Resource usage estimates (e.g., "58/1000 LUTs, 215/1000 FFs, 37/50 IOs")
- Proper port declarations with descriptive names
- IEEE 1364-compliant Verilog syntax

## Usage

These files can be used for:
1. **Regression testing** - Verify Verilog generation doesn't regress
2. **FPGA synthesis** - Test with vendor tools (Vivado, Quartus, etc.)
3. **Simulation** - Verify functional correctness with testbenches
4. **Educational examples** - Demonstrate digital logic concepts in HDL

## Generation Command

Files were generated using:
```bash
./build/wiredpanda --verilog-file output.v input.panda
```

## Notes

- Empty files indicate circuits with circular dependencies that couldn't be resolved
- The Verilog generator is more restrictive than Arduino export for circuit topology
- All generated modules target generic FPGA architecture for maximum compatibility
- Resource estimates assume basic FPGA primitives (LUT4, FF, BRAM, etc.)

---

*Generated automatically by wiRedPanda Verilog export functionality*
*Date: September 25, 2025*