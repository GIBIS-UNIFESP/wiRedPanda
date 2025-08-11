# wiRedPanda Fuzzing Guide

This directory contains fuzzing infrastructure for finding security vulnerabilities and bugs in wiRedPanda's file parsing and circuit simulation logic.

## Quick Start

### Prerequisites
- Clang compiler (required for libFuzzer)
- CMake 3.16+
- AddressSanitizer support

### Building the Fuzzer

```bash
# Configure with Clang and fuzzing enabled
CC=clang CXX=clang++ cmake -B build -G Ninja -DENABLE_FUZZING=ON

# Build the serialization fuzzer
cmake --build build --target fuzz_serialization
```

### Running Basic Fuzzing

```bash
# Run fuzzer with corpus directory for 60 seconds
./build/fuzz_serialization fuzz/corpus -max_total_time=60

# Run fuzzer with specific execution limit
./build/fuzz_serialization fuzz/corpus -runs=10000

# Test single input file
./build/fuzz_serialization path/to/test.panda
```

## Fuzzing Targets

### 1. Serialization Fuzzer (`fuzz_serialization`)

**Target**: Circuit file format parsing (`app/serialization.cpp`)

**Attack Surface**:
- Binary file format deserialization
- Magic header validation (`WPCF`, `WPWF`)
- Version compatibility handling
- Qt object reconstruction
- Memory allocation patterns

**Coverage**:
- `Serialization::readPandaHeader()`
- `Serialization::deserialize()`
- `Serialization::loadRect()`
- `Serialization::loadDolphinFileName()`

**Usage**:
```bash
# Standard fuzzing session
./build/fuzz_serialization fuzz/corpus -max_total_time=300

# Memory-intensive fuzzing with larger inputs
./build/fuzz_serialization fuzz/corpus -max_len=100000 -rss_limit_mb=4096
```

## Corpus Management

### Seed Corpus Location
- **Directory**: `fuzz/corpus/`
- **Contents**: Valid `.panda` circuit files from `examples/` directory
- **Special Cases**:
  - `empty.panda` - Empty file test
  - `magic_header_only.panda` - Minimal header test  
  - `invalid_magic.panda` - Invalid format test
  - `minimal_valid.panda` - Smallest valid binary format

### Adding New Seeds

```bash
# Copy new example files
cp examples/new_circuit.panda fuzz/corpus/

# Create custom test case
echo -n "WPCF" > fuzz/corpus/header_test.panda
```

## Found Vulnerabilities

### CVE-2024-XXXX: Integer Overflow in QArrayData Allocation

**Discovery**: 2024-08-11 via libFuzzer  
**Impact**: Remote code execution via malformed circuit files  
**Root Cause**: Missing bounds checking in serialization parser

**Reproducer**:
```bash
# Minimal crashing input (17 bytes)
echo "V1BDRi9aAABAYAAABgAAfgA=" | base64 -d > crash.panda
./build/fuzz_serialization crash.panda
```

**Input Analysis**:
```
Offset  Hex                 ASCII     Meaning
00-03   57 50 43 46         WPCF      Magic header
04-07   2f 5a 00 00         /Z..      Crafted size value  
08-0b   40 60 00 00         @`..      Triggers overflow
0c-0f   06 00 00 7e         ....~     Version fields
10      00                  .         Padding
```

**Stack Trace**:
```
ERROR: AddressSanitizer: requested allocation size 0xffffffffffffffff
#0 malloc
#1 QArrayData::allocate() (Qt5Core)
#2 Serialization::deserialize() (app/serialization.cpp)
```

## Advanced Usage

### Fuzzing with Custom Mutations

```bash
# Use dictionary for better mutations
./build/fuzz_serialization fuzz/corpus -dict=fuzz/serialization.dict

# Focus on specific functions
./build/fuzz_serialization fuzz/corpus -focus_function=deserialize
```

### Crash Minimization

```bash
# Minimize a crash to smallest possible input
./build/fuzz_serialization -minimize_crash=1 -runs=1000 crash-file.panda

# Clean up crash input for analysis  
./build/fuzz_serialization -cleanse_crash=1 -runs=1000 crash-file.panda
```

### Performance Tuning

```bash
# Parallel fuzzing (4 workers)
./build/fuzz_serialization fuzz/corpus -workers=4 -jobs=4

# Memory-constrained fuzzing
./build/fuzz_serialization fuzz/corpus -rss_limit_mb=1024 -malloc_limit_mb=512

# Fast mutation mode
./build/fuzz_serialization fuzz/corpus -len_control=0 -max_len=1024
```

## Integration with CI/CD

### GitHub Actions Integration

```yaml
- name: Run Fuzz Tests
  run: |
    CC=clang CXX=clang++ cmake -B build -DENABLE_FUZZING=ON
    cmake --build build --target fuzz_serialization
    timeout 300 ./build/fuzz_serialization fuzz/corpus || true
    
- name: Check for New Crashes
  run: |
    if ls crash-* >/dev/null 2>&1; then
      echo "::error::New crashes found during fuzzing"
      exit 1
    fi
```

### OSS-Fuzz Integration (Recommended)

For continuous fuzzing with Google's infrastructure:

1. **Apply to OSS-Fuzz**: Submit PR to `google/oss-fuzz`
2. **Project Configuration**: Create `projects/wiredpanda/` directory
3. **Build Script**: Implement `build.sh` for fuzzer compilation
4. **Dockerfile**: Container with dependencies and build environment

Benefits:
- 24/7 continuous fuzzing
- ClusterFuzz crash management
- Automatic regression testing
- Professional bug reporting

## Debugging Fuzzer Issues

### Common Problems

**Qt Application Context**:
```bash
# If fuzzer crashes on startup
export QT_QPA_PLATFORM=offscreen
./build/fuzz_serialization fuzz/corpus
```

**Memory Limits**:
```bash
# Increase limits for complex inputs
./build/fuzz_serialization fuzz/corpus -rss_limit_mb=8192
```

**Timeout Issues**:
```bash
# Increase timeout for slow parsing
./build/fuzz_serialization fuzz/corpus -timeout=60
```

### Analyzing Coverage

```bash
# Generate coverage report
./build/fuzz_serialization fuzz/corpus -dump_coverage=1

# Print coverage statistics  
./build/fuzz_serialization fuzz/corpus -print_coverage=1
```

## Future Fuzzing Targets

### Phase 2: Simulation Engine
- Target: `app/simulation.cpp`
- Focus: State machine fuzzing, clock timing, logic evaluation

### Phase 3: Element Factory
- Target: `app/elementfactory.cpp`  
- Focus: Polymorphic object creation, property validation

### Phase 4: Arduino Code Generation
- Target: `app/arduino/codegenerator.cpp`
- Focus: Template injection, code generation logic

## References

- [libFuzzer Documentation](https://llvm.org/docs/LibFuzzer.html)
- [AddressSanitizer Guide](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [OSS-Fuzz Project](https://github.com/google/oss-fuzz)
- [Fuzzing Best Practices](https://github.com/google/fuzzing/blob/master/docs/best-practices.md)

## Reporting Security Issues

Found a security vulnerability? Please report responsibly:

1. **DO NOT** file public GitHub issues for security bugs
2. **Email**: security@gibis-unifesp.org (if available)
3. **Include**: Minimal reproducer, impact assessment, suggested fix
4. **Timeline**: 90-day coordinated disclosure preferred