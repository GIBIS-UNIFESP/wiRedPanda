# Detailed Plan: Fix Arduino Export Feature with IC Support

## Current Problem Analysis

**Root Issue:** The Arduino export feature is **disabled** (`mainwindow.cpp:712`) because ICs are not supported in code generation. When encountered, the code generator throws a `PANDACEPTION` at `codegenerator.cpp:261`.

**Current Limitations:**
1. **IC Export Block**: `assignVariablesRec()` throws exception when encountering `ElementType::IC`
2. **Incomplete Implementation**: Commented TODO at `codegenerator.cpp:262` shows abandoned IC conversion
3. **Feature Disabled**: Export action deliberately disabled in UI (`mainwindow.cpp:712`)

**Current Arduino Code Generation:**
- Supports basic logic gates (AND, OR, NAND, NOR, XOR, XNOR, NOT)
- Supports flip-flops (D, T, SR, JK) and latches
- Generates Arduino C++ code with `elapsedMillis` library dependency
- Maps inputs to Arduino pins (A0-A5, 2-13, excluding 0-1 for serial)

## Implementation Strategy

### Phase 1: IC Flattening System
**Goal**: Convert IC hierarchies into flat Arduino-compatible logic

#### 1.1 IC Expansion Engine
```cpp
// New class: ICExpander
class ICExpander {
public:
    QVector<GraphicElement*> expandIC(IC* ic, QString& prefix);
    QMap<QNEPort*, QString> mapICConnections(IC* ic, const QString& prefix);
};
```

**Key Functions:**
- `flattenCircuit()`: Recursively expand all ICs into primitive elements
- `remapConnections()`: Update port connections after IC expansion
- `generateUniqueNames()`: Prevent variable name conflicts

#### 1.2 Connection Mapping
- Extract IC internal logic via `ic->generateMap()` (already exists)
- Map IC input/output ports to internal element connections
- Preserve signal flow through IC boundaries

### Phase 2: Enhanced Code Generation

#### 2.1 Pre-processing Step
```cpp
void CodeGenerator::preprocessElements() {
    // 1. Identify all ICs in circuit
    // 2. Recursively expand ICs to primitive elements
    // 3. Update m_elements with flattened list
    // 4. Update connection mappings
}
```

#### 2.2 Variable Name Management
- **Hierarchical naming**: `IC_name_element_name_counter`
- **Collision prevention**: Track all variable names globally
- **Port mapping**: Maintain original IC port → internal element mappings

#### 2.3 Update Core Generator
```cpp
void CodeGenerator::assignVariablesRec() {
    // Remove IC exception throw
    // Process flattened element list
    // Handle nested IC variable scoping
}
```

### Phase 3: Arduino/Emulator Compatibility

#### 3.1 Arduino Hardware Considerations
**Pin Limitations:**
- Arduino Uno: 14 digital pins (2 reserved for Serial)
- Arduino Mega: 54 digital pins 
- Pin allocation algorithm needed for complex circuits

**Memory Constraints:**
- 2KB SRAM on Arduino Uno
- Minimize boolean variable usage
- Optimize generated code size

#### 3.2 Emulator Support
**Target Platforms:**
- **Wokwi**: Web-based Arduino simulator
- **Proteus**: Professional circuit simulation
- **TinkerCAD**: Educational Arduino simulator

**Compatibility Features:**
- Standard Arduino C++ syntax
- No platform-specific extensions
- Compatible with Arduino IDE
- Support for common libraries (`elapsedMillis`)

### Phase 4: Advanced Features

#### 4.1 Smart Pin Assignment
```cpp
class PinManager {
    QStringList assignPins(const QVector<GraphicElement*>& elements);
    bool validatePinRequirements(int inputCount, int outputCount);
    QString generatePinoutDiagram();
};
```

#### 4.2 Code Optimization
- **Dead code elimination**: Remove unused variables
- **Boolean simplification**: Optimize logic expressions
- **Loop optimization**: Minimize computation in `loop()`

#### 4.3 Multi-Platform Generation
```cpp
enum class TargetPlatform {
    ArduinoUno,
    ArduinoMega,
    ArduinoNano,
    ESP32,
    Emulator
};
```

### Phase 5: Testing & Validation

#### 5.1 Unit Tests
```cpp
// Test IC expansion
void testSimpleICExpansion();
void testNestedICExpansion(); 
void testICConnectionMapping();

// Test code generation
void testPinAllocation();
void testVariableNaming();
void testCodeCompilation();
```

#### 5.2 Integration Tests
**Real Arduino Testing:**
- Upload generated code to Arduino Uno/Mega
- Verify input/output behavior matches simulation
- Test with various IC combinations

**Emulator Testing:**
- Wokwi simulation validation
- TinkerCAD compatibility testing
- Proteus verification

#### 5.3 Example Circuits
- Counter with 7-segment display IC
- ALU using multiple logic ICs
- Sequential circuits with memory ICs
- Mixed analog/digital circuits

## Implementation Timeline

### Sprint 1 (Week 1-2): Foundation
- [ ] IC flattening algorithm
- [ ] Basic connection remapping
- [ ] Update CodeGenerator preprocessing

### Sprint 2 (Week 3-4): Code Generation
- [ ] Variable naming system
- [ ] Remove IC exception handling
- [ ] Generate compilable Arduino code

### Sprint 3 (Week 5-6): Optimization
- [ ] Pin assignment algorithm
- [ ] Code size optimization
- [ ] Memory usage optimization

### Sprint 4 (Week 7-8): Testing
- [ ] Unit test suite
- [ ] Real hardware validation
- [ ] Emulator compatibility

### Sprint 5 (Week 9-10): Polish
- [ ] Error handling & user feedback
- [ ] Documentation & examples
- [ ] UI re-enablement

## Risk Mitigation

**High Complexity Circuits:**
- Implement circuit complexity analysis
- Warn users about pin/memory limitations
- Suggest circuit simplification

**Performance Issues:**
- Implement timeout for code generation
- Optimize algorithm for large IC hierarchies
- Consider incremental compilation

**Hardware Limitations:**
- Detect target Arduino model
- Validate pin/memory requirements
- Generate platform-specific warnings

## Success Criteria

✅ **Functional**: IC-containing circuits export to working Arduino code  
✅ **Compatible**: Code compiles and runs on real Arduino hardware  
✅ **Emulator-Ready**: Generated code works in popular Arduino emulators  
✅ **Scalable**: Handles nested ICs and complex hierarchies  
✅ **Optimized**: Efficient pin usage and memory consumption  
✅ **User-Friendly**: Clear error messages and export feedback

This plan transforms the currently disabled Arduino export into a fully functional feature supporting the complete wiRedPanda element ecosystem, including complex IC-based circuits.

## Key Files to Modify

### Primary Implementation Files
- `app/arduino/codegenerator.cpp` - Core code generation logic
- `app/arduino/codegenerator.h` - Class definitions and interfaces
- `app/mainwindow.cpp` - Re-enable Arduino export action
- `app/ic.cpp` - IC expansion and mapping functionality

### New Files to Create
- `app/arduino/icexpander.cpp` - IC flattening algorithms
- `app/arduino/icexpander.h` - IC expansion interface
- `app/arduino/pinmanager.cpp` - Smart pin allocation
- `app/arduino/pinmanager.h` - Pin management interface

### Testing Files
- `test/testarduinoexport.cpp` - Unit tests for Arduino export
- `test/testarduinoexport.h` - Test definitions
- `examples/arduino/` - Example circuits for testing

## Dependencies

### Required Libraries
- `elapsedMillis` - Already used for clock timing
- Standard Arduino libraries (included by default)

### Optional Enhancements
- JSON library for configuration management
- Custom optimization libraries for code generation

## Backward Compatibility

This implementation maintains full backward compatibility:
- Existing non-IC circuits continue to work unchanged
- Current Arduino code generation remains functional
- No breaking changes to existing APIs
- Graceful degradation for unsupported features