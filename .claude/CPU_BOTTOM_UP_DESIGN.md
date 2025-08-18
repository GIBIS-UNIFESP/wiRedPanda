# PandaCPU: Complete 8-bit CPU Bottom-Up Design

## Vision: From Gates to Running Programs 🚀

**Ultimate Goal**: Build a complete, functional 8-bit CPU entirely from basic logic gates, demonstrating the full hierarchy of digital design from primitives to a working computer system.

## PandaCPU Architecture Specification

### **8-bit Simple RISC CPU "PandaCPU"**
- **Data Path**: 8-bit data width
- **Registers**: 8 general-purpose registers (R0-R7)
- **Memory**: 8-bit address space (256 bytes)
- **Architecture**: Harvard (separate instruction/data memory)
- **Execution**: Single-cycle per instruction
- **Instruction Word**: 16-bit (4-bit opcode + 12-bit operands)

### **Instruction Set Architecture (16 Instructions)**
```assembly
Opcode | Mnemonic     | Operation                | Format
-------|--------------|--------------------------|------------------
0000   | NOP          | No operation            | NOP
0001   | LDI Rd, #imm | Load immediate          | LDI R2, #42
0010   | LD Rd, addr  | Load from memory        | LD R1, 0x80
0011   | ST Rs, addr  | Store to memory         | ST R3, 0x90
0100   | MOV Rd, Rs   | Move register           | MOV R1, R2
0101   | ADD Rd, Rs   | Add registers           | ADD R1, R2
0110   | SUB Rd, Rs   | Subtract registers      | SUB R1, R2
0111   | AND Rd, Rs   | Bitwise AND             | AND R1, R2
1000   | OR Rd, Rs    | Bitwise OR              | OR R1, R2
1001   | XOR Rd, Rs   | Bitwise XOR             | XOR R1, R2
1010   | SHL Rd       | Shift left              | SHL R1
1011   | SHR Rd       | Shift right             | SHR R1
1100   | CMP Rd, Rs   | Compare (set flags)     | CMP R1, R2
1101   | JMP addr     | Unconditional jump      | JMP 0x40
1110   | JZ addr      | Jump if zero flag       | JZ 0x50
1111   | HLT          | Halt execution          | HLT
```

## Complete Bottom-Up Hierarchy

### **Level 0: Primitive Gates** ✅ (Already Available)
```
Basic Logic: AND, OR, NOT, XOR, NAND, NOR, Buffer
Status: Already implemented in wiRedPanda
```

### **Level 1: Basic Combinational Building Blocks**

#### **1.1 Arithmetic Building Blocks**
```cpp
class TestLevel1Combinational : public QObject {
private slots:
    void testHalfAdder();          // A⊕B (sum), A∧B (carry)
    void testFullAdder();          // 2 half-adders + OR gate
    void test8BitRippleAdder();    // Chain of 8 full adders
    void test8BitSubtractor();     // Adder + 2's complement logic
};
```

#### **1.2 Selection and Routing**
```cpp
    void test2to1Multiplexer();    // Data selection
    void test4to1Multiplexer();    // Built from 2:1 muxes
    void test8to1Multiplexer();    // Tree structure
    void test1to2Decoder();        // Address decoding
    void test3to8Decoder();        // Full address decoder
    void test8BitComparator();     // Equality and magnitude
```

#### **1.3 Basic Memory Elements**
```cpp
    void testSRLatchFromNORs();    // Cross-coupled NOR gates ✅ EXISTS
    void testSRLatchFromNANDs();   // Cross-coupled NAND gates
    void testDLatchFromGates();    // Gated D latch from SR + logic
```

### **Level 2: Sequential Building Blocks**

#### **2.1 Flip-Flops and Registers**
```cpp
class TestLevel2Sequential : public QObject {
private slots:
    void testDFlipFlopFromLatches();    // Master-slave from 2 D-latches
    void testJKFlipFlopFromGates();     // JK-FF from NAND gates
    void testTFlipFlopFromJK();         // T-FF from JK-FF
    void test8BitRegister();            // 8 D-FFs + common enable/clock
    void test8BitRegisterWithReset();   // Register + asynchronous reset
    void testTriStateRegister();       // Register + tri-state outputs
};
```

#### **2.2 Counters and Shift Registers**
```cpp
    void test8BitBinaryCounter();      // 8 T-FFs + ripple carry
    void test8BitCounterWithLoad();    // Counter + parallel load
    void testModulo16Counter();        // Custom modulo counter
    void test8BitShiftRegister();      // Serial in, parallel out ✅ EXISTS
    void testBidirectionalShiftReg();  // Left/right shift capability
```

### **Level 3: Functional Units**

#### **3.1 Arithmetic Logic Unit (ALU)**
```cpp
class TestLevel3Functional : public QObject {
private slots:
    void test8BitALUArithmetic();      // ADD, SUB operations
    void test8BitALULogical();         // AND, OR, XOR, NOT
    void test8BitALUShift();           // SHL, SHR operations
    void testALUFlagGeneration();      // Zero, Carry, Negative flags
    void testComplete8BitALU();        // All 16 ALU operations
};
```

#### **3.2 Register File**
```cpp
    void test8RegisterFile();          // 8×8-bit registers
    void testRegFileWithDecoder();     // 3-bit address decoder
    void testRegFileDualPort();        // Simultaneous read/write
    void testRegFileWithEnable();      // Write enable control
```

#### **3.3 Memory Units**
```cpp
    void test256ByteRAM();             // 256×8-bit memory array
    void testRAMWithAddressDecoder();  // 8-bit address decoding
    void test256WordROM();             // 256×16-bit instruction storage
    void testMemoryInterface();       // MAR, MDR, control signals
```

### **Level 4: CPU Data Path Components**

#### **4.1 Data Path Architecture**
```cpp
class TestLevel4DataPath : public QObject {
private slots:
    void testDataPathMultiplexers();   // ALU input selection
    void testAddressGeneration();      // PC, effective address calc
    void testInstructionRegister();    // 16-bit instruction storage
    void testProgramCounter();         // PC with increment/load
    void testFlagRegister();           // Status flags storage
};
```

#### **4.2 Integrated Data Path**
```cpp
    void testALUToRegisterPath();      // ALU output → register file
    void testMemoryToRegisterPath();   // Memory → register file
    void testRegisterToMemoryPath();   // Register file → memory
    void testCompleteDataPath();       // All paths integrated
```

### **Level 5: CPU Control Unit**

#### **5.1 Instruction Decoding**
```cpp
class TestLevel5Control : public QObject {
private slots:
    void testOpcodeDecoder();          // 4-bit opcode → control signals
    void testRegisterAddressDecoder(); // Register field decoding
    void testImmediateExtension();     // Sign/zero extend immediates
    void testInstructionDecoder();     // Complete instruction decode
};
```

#### **5.2 Control Logic**
```cpp
    void testControlSignalGeneration(); // All CPU control signals
    void testFetchDecodeExecuteFSM();   // CPU state machine
    void testJumpLogic();               // Branch condition evaluation
    void testHaltLogic();               // CPU halt/resume control
```

### **Level 6: Complete PandaCPU**

#### **6.1 CPU Integration**
```cpp
class TestLevel6CompleteCPU : public QObject {
private slots:
    void testCPUDataPathIntegration();  // Data path + control unit
    void testInstructionExecution();   // Single instruction execution
    void testCPUStateManagement();     // Register/flag state
    void testMemoryInterface();        // CPU ↔ memory communication
};
```

#### **6.2 Individual Instruction Testing**
```cpp
    // Arithmetic Instructions
    void testLDI_Instruction();        // Load immediate
    void testADD_Instruction();        // Register addition
    void testSUB_Instruction();        // Register subtraction

    // Logical Instructions
    void testAND_Instruction();        // Bitwise AND
    void testOR_Instruction();         // Bitwise OR
    void testXOR_Instruction();        // Bitwise XOR

    // Memory Instructions
    void testLD_Instruction();         // Load from memory
    void testST_Instruction();         // Store to memory
    void testMOV_Instruction();        // Register move

    // Shift Instructions
    void testSHL_Instruction();        // Shift left
    void testSHR_Instruction();        // Shift right

    // Control Instructions
    void testCMP_Instruction();        // Compare with flags
    void testJMP_Instruction();        // Unconditional jump
    void testJZ_Instruction();         // Conditional jump
    void testNOP_Instruction();        // No operation
    void testHLT_Instruction();        // Halt execution
```

### **Level 7: Computer System**

#### **7.1 Complete Computer System**
```cpp
class TestLevel7ComputerSystem : public QObject {
private slots:
    void testCPUWithRAM();             // CPU + 256-byte RAM
    void testCPUWithROM();             // CPU + instruction ROM
    void testSystemInitialization();   // Power-on reset sequence
    void testIOInterface();            // LED outputs for results
};
```

#### **7.2 Program Execution**
```cpp
    void testSingleInstructionProgram(); // Execute one instruction
    void testMultiInstructionSequence(); // Execute instruction sequence
    void testProgramWithLoops();         // Loops and branches
    void testCompletePrograms();         // Full program execution
```

### **Level 8: Real Programs**

#### **8.1 Sample Programs for Testing**
```cpp
class TestLevel8Programs : public QObject {
private slots:
    void testCountToTen();             // Simple counting loop
    void testFibonacciSequence();      // Recursive calculation
    void testArithmeticCalculator();   // Basic arithmetic
    void testMemoryPatternFill();      // Memory manipulation
    void testBubbleSortAlgorithm();    // Complex algorithm
};
```

## Sample Test Programs

### **Program 1: Count from 0 to 10**
```assembly
start:  LDI R0, #0      ; Initialize counter
        LDI R1, #10     ; Set limit
loop:   CMP R0, R1      ; Compare counter with limit
        JZ done         ; Jump if equal
        ADD R0, #1      ; Increment counter
        JMP loop        ; Repeat
done:   ST R0, 0xFF     ; Store result to output
        HLT             ; Stop
```

### **Program 2: Fibonacci Sequence**
```assembly
        LDI R0, #0      ; F(0) = 0
        LDI R1, #1      ; F(1) = 1
        LDI R2, #8      ; Calculate 8 terms
        ST R0, 0x80     ; Store F(0)
        ST R1, 0x81     ; Store F(1)
        LDI R3, #2      ; Index = 2
loop:   CMP R3, R2      ; Check if done
        JZ done         ; Exit if done
        ADD R4, R0, R1  ; F(n) = F(n-1) + F(n-2)
        MOV R0, R1      ; Shift values
        MOV R1, R4
        ST R1, R3       ; Store F(n)
        ADD R3, #1      ; Increment index
        JMP loop        ; Continue
done:   HLT
```

## Testing Strategy & Coverage Impact

### **Progressive Complexity Testing**
1. **Unit Tests**: Each building block individually
2. **Integration Tests**: Combinations of building blocks
3. **System Tests**: Complete subsystems
4. **End-to-End Tests**: Full programs running on complete CPU

### **Coverage Explosion**
- **Current Coverage**: 35.2% overall
- **Expected Coverage**: 95%+ overall
- **Gate-level Coverage**: Near 100% (every gate type used extensively)
- **Path Coverage**: 100% (every simulation path exercised)
- **Stress Testing**: Thousands of gates, complex dependencies

### **Educational Impact**
- **Complete Design Methodology**: From gates to programs
- **Real-world Relevance**: Industry-standard hierarchical design
- **Proof of Concept**: wiRedPanda can handle real complexity
- **Teaching Tool**: Shows how computers actually work

## Implementation Timeline

### **Phase 1: Building Blocks (Weeks 1-2)**
- Level 1-2 tests: Basic combinational and sequential blocks
- Foundation for all higher levels

### **Phase 2: Functional Units (Weeks 3-4)**
- Level 3-4 tests: ALU, register file, memory units
- CPU component integration

### **Phase 3: Complete CPU (Weeks 5-6)**
- Level 5-6 tests: Control unit, complete CPU
- Individual instruction testing

### **Phase 4: Computer System (Weeks 7-8)**
- Level 7-8 tests: Complete system, real programs
- Full program execution validation

## Success Metrics

### **Technical Achievements**
- **Circuit Complexity**: 5,000+ logic gates successfully simulated
- **Instruction Execution**: All 16 instructions working correctly
- **Program Execution**: Complex programs running successfully
- **Performance**: Large circuits simulating in reasonable time

### **Coverage Achievements**
- **Line Coverage**: 95%+ across all modules
- **Function Coverage**: 95%+ across all modules
- **Path Coverage**: 100% of simulation engine paths
- **Edge Case Coverage**: All error conditions tested

### **Educational Achievements**
- **Complete Learning Path**: Gates → CPU → Programs
- **Industry Relevance**: Real digital design methodology
- **Inspiration**: Students see what's possible with basic gates

This would be the most ambitious educational digital design project ever attempted, demonstrating that complex systems truly emerge from simple components through hierarchical design!
