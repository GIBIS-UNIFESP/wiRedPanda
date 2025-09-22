# Arduino Export Test Plan

## Current State Analysis

**Status**: No automated tests exist for Arduino export functionality.

**Arduino Export Implementation**:
- **Location**: `/workspace/app/arduino/codegenerator.h` and `/workspace/app/arduino/codegenerator.cpp`
- **Main Entry Point**: `MainWindow::exportToArduino()` in `/workspace/app/mainwindow.cpp:993`
- **Supported Boards**:
  - Arduino UNO R3/R4 (20 pins)
  - Arduino Nano (20 pins)
  - Arduino Mega 2560 (70 pins)
  - ESP32 (36 pins)

**Supported Elements**:
- Input: InputButton, InputSwitch
- Output: LED, Buzzer, Display7
- Logic Gates: AND, OR, NOT, XOR, NAND, NOR, XNOR
- Sequential: D/JK/SR/T FlipFlops, D/SR Latches
- Special: VCC/GND constants, Clock generators, TruthTable lookup
- Complex: IC (Integrated Circuits) with nested elements

## Test Implementation Plan

### 1. Test File Structure

Create new test files following existing project conventions:

```
test/testarduino.h       - Test class declaration
test/testarduino.cpp     - Test implementation
```

Update `CMakeLists.txt` to include the new test executable.

### 2. Test Categories

#### 2.1 Core Generation Tests

**Purpose**: Verify basic code generation functionality

- [ ] **Basic Code Generation**
  - Generate .ino file for simple circuit
  - Verify file creation and basic structure
  - Check header comments and metadata

- [ ] **Board Selection Logic**
  - Test pin requirement calculation
  - Verify correct board selection (UNO → Nano → Mega → ESP32)
  - Test error handling for circuits exceeding maximum pins (70+ pins)

- [ ] **File Output Validation**
  - Verify .ino file format compliance
  - Check Arduino IDE compatibility markers
  - Validate generated code syntax structure

#### 2.2 Element-Specific Tests

**Purpose**: Test code generation for individual element types

##### Input Elements
- [ ] **InputButton Code Generation**
  - Pin assignment and variable naming
  - pinMode() setup generation
  - digitalRead() in loop function

- [ ] **InputSwitch Code Generation**
  - Similar to InputButton validation
  - Label handling in variable names

##### Output Elements
- [ ] **LED Output Generation**
  - Pin assignment for simple LEDs
  - digitalWrite() calls in loop
  - Multiple output handling

- [ ] **Buzzer Element**
  - Frequency mapping for different notes
  - tone()/noTone() function calls
  - Special pin handling

- [ ] **Display7 Element**
  - Multi-pin output generation (7 pins per display)
  - Pin naming convention ("seven" replacement)

##### Logic Gates
- [ ] **Basic Logic Gates** (AND, OR, NOT)
  - Boolean expression generation
  - Input signal mapping
  - Output variable assignment

- [ ] **Complex Logic Gates** (NAND, NOR, XOR, XNOR)
  - Negation logic handling
  - Multi-input operations
  - Expression optimization

##### Sequential Elements
- [ ] **D Flip-Flop**
  - Clock edge detection logic
  - Preset/Clear handling
  - Dual output (Q, ~Q) generation

- [ ] **JK Flip-Flop**
  - Toggle mode implementation
  - Set/Reset/Hold mode logic
  - Preset/Clear priority

- [ ] **SR Flip-Flop & Latch**
  - Set-Reset logic
  - Invalid state handling
  - Latch vs Flip-flop timing differences

- [ ] **T Flip-Flop**
  - Toggle functionality
  - Clock edge detection

##### Special Elements
- [ ] **VCC/GND Constants**
  - Constant value assignment ("true"/"false")
  - No pin assignment required

- [ ] **Clock Generator**
  - Real-time timing implementation
  - millis() based frequency generation
  - Multiple clock support

- [ ] **TruthTable Element**
  - Lookup table generation
  - Index calculation logic
  - Multi-input truth table support

#### 2.3 IC (Integrated Circuit) Tests

**Purpose**: Test nested circuit code generation

- [ ] **Simple IC Generation**
  - Internal element variable mapping
  - IC input/output port handling
  - Variable name prefixing

- [ ] **Nested IC Handling**
  - IC within IC code generation
  - Recursive variable declaration
  - Complex signal routing

- [ ] **IC Port Mapping**
  - External to internal signal mapping
  - Port name handling and sanitization

#### 2.4 Code Quality & Validation Tests

**Purpose**: Ensure generated code quality and correctness

- [ ] **Variable Naming**
  - Forbidden character removal
  - Name uniqueness validation
  - Label integration in names

- [ ] **Pin Assignment**
  - No duplicate pin assignments
  - Pin availability checking
  - Board-specific pin validation

- [ ] **Generated Code Syntax**
  - Valid C++ syntax verification
  - Arduino function usage validation
  - Comment and structure correctness

#### 2.5 Integration & Complex Circuit Tests

**Purpose**: Test real-world circuit scenarios

- [ ] **Multi-Element Circuits**
  - Circuits combining multiple element types
  - Signal flow validation through complex paths
  - Variable dependency resolution

- [ ] **Real Circuit Examples**
  - Binary counter implementation
  - 7-segment display decoder
  - Traffic light controller
  - Simple state machines

- [ ] **Edge Cases & Error Handling**
  - Empty circuits
  - Disconnected elements
  - Unsupported element types
  - Invalid circuit topologies

### 3. Test Infrastructure Design

#### 3.1 Test Utilities

```cpp
class ArduinoTestHelper {
public:
    // Circuit building helpers
    static GraphicElement* createInputSwitch(const QString& label = "");
    static GraphicElement* createLED(const QString& label = "");
    static GraphicElement* createLogicGate(ElementType type, int inputs = 2);

    // Connection helpers
    static void connectElements(GraphicElement* from, GraphicElement* to, int fromPort = 0, int toPort = 0);
    static QVector<GraphicElement*> createSimpleCircuit();

    // Validation helpers
    static bool validateArduinoCode(const QString& code);
    static QStringList extractVariableDeclarations(const QString& code);
    static QStringList extractPinAssignments(const QString& code);
    static bool checkPinUniqueness(const QStringList& pins);
};
```

#### 3.2 Test Data Management

- **Temporary Files**: Use QTemporaryFile for .ino output testing
- **Mock Objects**: Create lightweight GraphicElement mocks for testing
- **Test Circuits**: Pre-defined circuit configurations for consistent testing

#### 3.3 Validation Methods

1. **String Analysis**: Parse generated Arduino code for expected patterns
2. **Pin Validation**: Verify pin assignments are unique and valid for selected board
3. **Logic Verification**: Check that boolean expressions match circuit logic
4. **Compilation Testing** (Optional): Use Arduino CLI to verify code compiles

### 4. Implementation Phases

#### Phase 1: Foundation (Priority 1)
- [ ] Test file creation and CMake integration
- [ ] Basic test infrastructure and helpers
- [ ] Simple code generation framework test
- [ ] File output validation

#### Phase 2: Core Elements (Priority 2)
- [ ] Input element tests (InputButton, InputSwitch)
- [ ] Output element tests (LED basic functionality)
- [ ] Basic logic gate tests (AND, OR, NOT)
- [ ] Board selection algorithm tests

#### Phase 3: Advanced Elements (Priority 3)
- [ ] Complex logic gates (NAND, NOR, XOR, XNOR)
- [ ] Sequential element tests (all flip-flops and latches)
- [ ] Special element tests (VCC, GND, Clock, TruthTable)
- [ ] Buzzer and Display7 tests

#### Phase 4: Complex Features (Priority 4)
- [ ] IC and nested circuit tests
- [ ] Integration tests with multiple element types
- [ ] Real-world circuit examples
- [ ] Performance and edge case testing

#### Phase 5: Quality Assurance (Priority 5)
- [ ] Code quality validation tests
- [ ] Error handling verification
- [ ] Documentation and test coverage analysis
- [ ] Optional Arduino CLI compilation testing

### 5. Success Criteria

- [ ] **Coverage**: All Arduino export code paths tested
- [ ] **Reliability**: Tests consistently pass on different platforms
- [ ] **Maintainability**: Easy to add tests for new Arduino features
- [ ] **Documentation**: Clear test descriptions and failure diagnostics
- [ ] **Integration**: Seamlessly integrated with existing Qt Test framework

### 6. Future Enhancements

- **Arduino CLI Integration**: Automatic compilation verification of generated code
- **Hardware-in-Loop Testing**: Optional testing with actual Arduino hardware
- **Performance Benchmarking**: Test generation speed for large circuits
- **Code Optimization Testing**: Verify generated code efficiency

### 7. Dependencies

- **Qt Test Framework**: For test execution and assertions
- **Existing Test Infrastructure**: Follow patterns from `testelements.cpp`, etc.
- **CodeGenerator Class**: Main testing target
- **GraphicElement Mocks**: For isolated testing
- **File I/O Utilities**: For temporary file handling

---

This comprehensive test plan ensures the Arduino export functionality is thoroughly validated while maintaining consistency with the existing wiRedPanda test architecture.