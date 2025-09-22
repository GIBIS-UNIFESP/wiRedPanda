# Arduino Code Generation Regression Testing

## Overview

This document describes the Arduino code generation regression testing system that validates the consistency and quality of generated Arduino code from wiRedPanda circuits.

## Test Corpus

The regression test uses a comprehensive test corpus located in `test/arduino/` containing:

- **18 .panda files** - Original circuit files covering diverse element types
- **19 .ino files** - Expected Arduino code outputs (baseline)
- **19 JSON files** - Tinkercad component specifications
- **19 .md files** - Setup guides for recreating circuits
- **77 total files** - Complete regression test dataset

### Corpus Coverage

The test corpus includes circuits with:
- **Counters** (3-bit, 4-bit binary counters)
- **Flip-flops** (D, JK, T, SR flip-flops and latches)
- **Logic gates** (AND, OR, NOT, NAND, NOR, XOR, XNOR)
- **Displays** (7-segment displays with various bit inputs)
- **Sequential circuits** (registers, shift registers)
- **Complex ICs** (integrated circuits with multiple pins)
- **Input/output elements** (switches, buttons, LEDs, buzzers)

## Running Regression Tests

### Individual Test Methods

```bash
# Test critical individual files
./build/testarduino testIndividualCorpusFiles

# Run full regression corpus (compares all files)
./build/testarduino testArduinoRegressionCorpus

# Test code quality metrics
./build/testarduino testCorpusCodeQuality
```

### Complete Test Suite

```bash
# Run all Arduino tests including regression tests
./build/testarduino

# Run via CTest
ctest --test-dir build -R testarduino
```

## Test Results Interpretation

### Expected Behavior

The regression tests are designed to:

1. **✅ PASS** when generated code matches expected baselines exactly
2. **⚠️ DETECT** when code generation changes (may indicate regressions)
3. **❌ FAIL** when code generation breaks or produces invalid Arduino code

### When Tests Detect Differences

**Differences are not necessarily failures!** They indicate changes in code generation that should be reviewed:

**Legitimate Changes (Update baselines):**
- Intentional code generation improvements
- New optimization features
- Board selection algorithm updates
- Variable naming improvements

**Potential Regressions (Investigate):**
- Broken Arduino syntax
- Missing pin assignments
- Incorrect logic implementation
- Performance degradations

### Difference Types

Common differences detected:
- **Variable naming**: `led2_0` vs `led5_0`
- **Element IDs**: `aux_jk_flip_flop_4_0_q` vs `aux_jk_flip_flop_1_0_q`
- **Pin assignments**: `A0` vs `A1`
- **Board selection**: UNO vs Mega vs ESP32

## Updating Test Baselines

When legitimate changes are made to code generation:

1. **Regenerate the test corpus:**
   ```bash
   # Export updated .ino files
   cd examples
   for panda in *.panda; do
       ../build/wiredpanda --arduino-file "${panda%.panda}.ino" "$panda"
   done

   # Copy updated files to test corpus
   cp *.ino *.panda ../test/arduino/
   ```

2. **Verify the updated code:**
   ```bash
   # Test individual files work correctly
   ./build/testarduino testIndividualCorpusFiles

   # Verify code quality
   ./build/testarduino testCorpusCodeQuality
   ```

3. **Commit the updated baselines:**
   ```bash
   git add test/arduino/
   git commit -m "update: Arduino regression test baselines"
   ```

## Test Architecture

### Class Structure

```cpp
class TestArduino {
    // Phase 6: Regression Testing
    void testArduinoRegressionCorpus();      // Main regression test
    void testIndividualCorpusFiles();        // Critical file validation
    void testCorpusCodeQuality();            // Code quality metrics

private:
    // Helper methods
    QVector<GraphicElement*> loadPandaFile(const QString& filePath);
    QString loadExpectedCode(const QString& inoFilePath);
    bool compareArduinoCode(const QString& generated, const QString& expected, QString& diffReport);
    QStringList getCorpusPandaFiles() const;
    QString normalizeArduinoCode(const QString& code) const;
};
```

### Test Flow

1. **Load** .panda circuit file using Serialization::deserialize()
2. **Generate** Arduino code using CodeGenerator
3. **Compare** against expected .ino baseline
4. **Report** differences with detailed diff information
5. **Validate** code quality using existing quality metrics

### Code Normalization

The test normalizes code for comparison by:
- Removing comments and extra whitespace
- Standardizing line breaks
- Trimming unnecessary characters

This focuses comparison on functional differences rather than cosmetic formatting.

## Integration with CI/CD

The regression tests integrate with the existing CTest framework:

```bash
# CMakeLists.txt automatically includes testarduino
ctest --test-dir build
```

Consider running regression tests:
- **On every commit** to catch regressions early
- **Before releases** to ensure stability
- **After Arduino code generation changes** to validate improvements

## Performance Considerations

- **Full regression test**: ~1000ms for 18 files
- **Individual file test**: ~200ms for 3 critical files
- **Code quality test**: ~400ms for 5 sample files

Tests are optimized for CI environments and can run in headless mode.

## Troubleshooting

### Common Issues

**File not found errors:**
- Ensure `test/arduino/` directory exists
- Verify .panda and .ino files are properly copied
- Check file permissions and paths

**Serialization errors:**
- Confirm `registerTypes()` is called before loading
- Verify ElementFactory initialization
- Check .panda file format compatibility

**Code generation failures:**
- Validate input circuit elements load correctly
- Check CodeGenerator initialization
- Verify temporary file creation permissions

### Debug Mode

Enable verbose output for debugging:
```cpp
Comment::setVerbosity(5);  // Enable debug logging
```

This provides detailed information about:
- File loading progress
- Element deserialization
- Code generation steps
- Comparison details

## Future Enhancements

Potential improvements:
- **Semantic comparison** instead of text comparison
- **Performance benchmarking** across corpus
- **Arduino compilation testing** with arduino-cli
- **Hardware-in-the-loop testing** with real Arduino boards
- **Automated baseline updates** for legitimate changes