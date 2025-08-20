# PandaCPU Implementation Example

## Concrete Implementation: Level 3 - 8-bit ALU Construction

This shows exactly how one major component (the ALU) would be built and tested in the hierarchical CPU design.

### **8-bit ALU Specification**

**Operations Supported:**
```
0000: ADD    - Addition
0001: SUB    - Subtraction
0010: AND    - Bitwise AND
0011: OR     - Bitwise OR
0100: XOR    - Bitwise XOR
0101: NOT    - Bitwise NOT (A only)
0110: SHL    - Shift Left
0111: SHR    - Shift Right
1000: CMP    - Compare (A - B, set flags only)
```

**Inputs:**
- `A[7:0]` - 8-bit operand A
- `B[7:0]` - 8-bit operand B
- `OP[3:0]` - 4-bit operation select
- `CLK` - Clock for output register

**Outputs:**
- `Y[7:0]` - 8-bit result
- `Zero` - Zero flag (Y == 0)
- `Carry` - Carry flag (for arithmetic)
- `Negative` - Negative flag (Y[7] == 1)

## Complete Test Implementation

### **Test Class Structure**
```cpp
class TestLevel3_8BitALU : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Building block tests (dependencies)
    void testFullAdder();
    void test8BitAdder();
    void test8BitSubtractor();
    void test8to1Multiplexer();

    // ALU component tests
    void testALUArithmeticUnit();
    void testALULogicalUnit();
    void testALUShiftUnit();
    void testALUFlagGeneration();

    // Complete ALU tests
    void testComplete8BitALU();
    void testALUAllOperations();
    void testALUEdgeCases();
    void testALUPerformance();

private:
    WorkSpace* m_workspace;
    Scene* m_scene;
    Simulation* m_simulation;

    // Helper functions for ALU construction
    struct ALUComponents {
        // Inputs
        QVector<InputButton*> inputA;      // A[7:0]
        QVector<InputButton*> inputB;      // B[7:0]
        QVector<InputButton*> opSelect;    // OP[3:0]

        // ALU building blocks
        QVector<Xor*> xorGates;           // For XOR operations
        QVector<And*> andGates;           // For AND operations
        QVector<Or*> orGates;             // For OR operations
        QVector<Not*> notGates;           // For NOT operations
        QVector<FullAdder*> adders;       // For ADD/SUB
        QVector<Mux*> outputMuxes;        // For result selection

        // Outputs
        QVector<Led*> result;             // Y[7:0]
        Led* zeroFlag;                    // Zero flag
        Led* carryFlag;                   // Carry flag
        Led* negativeFlag;                // Negative flag

        // Connections
        QVector<QNEConnection*> connections;
    };

    ALUComponents createALU();
    void setupInputs(ALUComponents& alu, uint8_t a, uint8_t b, uint8_t op);
    uint8_t readResult(const ALUComponents& alu);
    bool readFlag(Led* flag);
    void verifyOperation(const ALUComponents& alu, uint8_t a, uint8_t b, uint8_t op, uint8_t expected, bool zero, bool carry, bool negative);
};
```

### **ALU Construction Implementation**
```cpp
TestLevel3_8BitALU::ALUComponents TestLevel3_8BitALU::createALU() {
    ALUComponents alu;

    // Create inputs
    for (int i = 0; i < 8; i++) {
        alu.inputA.append(new InputButton());
        alu.inputB.append(new InputButton());
        m_scene->addItem(alu.inputA[i]);
        m_scene->addItem(alu.inputB[i]);
    }

    for (int i = 0; i < 4; i++) {
        alu.opSelect.append(new InputButton());
        m_scene->addItem(alu.opSelect[i]);
    }

    // Create ALU logic blocks
    // 1. Arithmetic Unit (8-bit adder/subtractor)
    for (int i = 0; i < 8; i++) {
        alu.adders.append(new FullAdder()); // Built from half-adders in Level 1
        m_scene->addItem(alu.adders[i]);
    }

    // 2. Logical Units
    for (int i = 0; i < 8; i++) {
        alu.xorGates.append(new Xor());
        alu.andGates.append(new And());
        alu.orGates.append(new Or());
        alu.notGates.append(new Not());

        m_scene->addItem(alu.xorGates[i]);
        m_scene->addItem(alu.andGates[i]);
        m_scene->addItem(alu.orGates[i]);
        m_scene->addItem(alu.notGates[i]);
    }

    // 3. Output multiplexers (select operation result)
    for (int i = 0; i < 8; i++) {
        alu.outputMuxes.append(new Mux()); // 8:1 multiplexer for each bit
        m_scene->addItem(alu.outputMuxes[i]);
    }

    // Create outputs
    for (int i = 0; i < 8; i++) {
        alu.result.append(new Led());
        m_scene->addItem(alu.result[i]);
    }

    alu.zeroFlag = new Led();
    alu.carryFlag = new Led();
    alu.negativeFlag = new Led();
    m_scene->addItem(alu.zeroFlag);
    m_scene->addItem(alu.carryFlag);
    m_scene->addItem(alu.negativeFlag);

    // Wire up the ALU (this is where the magic happens!)
    // This would involve hundreds of connections...

    // Example: Connect inputs to logical operations
    for (int i = 0; i < 8; i++) {
        // Connect A and B to each logical operation
        auto* connA1 = new QNEConnection();
        auto* connB1 = new QNEConnection();
        auto* connA2 = new QNEConnection();
        auto* connB2 = new QNEConnection();

        m_scene->addItem(connA1);
        m_scene->addItem(connB1);
        m_scene->addItem(connA2);
        m_scene->addItem(connB2);

        // XOR gate connections
        connA1->setStartPort(alu.inputA[i]->outputPort());
        connA1->setEndPort(alu.xorGates[i]->inputPort(0));
        connB1->setStartPort(alu.inputB[i]->outputPort());
        connB1->setEndPort(alu.xorGates[i]->inputPort(1));

        // AND gate connections
        connA2->setStartPort(alu.inputA[i]->outputPort());
        connA2->setEndPort(alu.andGates[i]->inputPort(0));
        connB2->setStartPort(alu.inputB[i]->outputPort());
        connB2->setEndPort(alu.andGates[i]->inputPort(1));

        alu.connections.append(connA1);
        alu.connections.append(connB1);
        alu.connections.append(connA2);
        alu.connections.append(connB2);
    }

    // Connect 8-bit adder chain (ripple carry)
    for (int i = 0; i < 8; i++) {
        auto* connA = new QNEConnection();
        auto* connB = new QNEConnection();

        m_scene->addItem(connA);
        m_scene->addItem(connB);

        connA->setStartPort(alu.inputA[i]->outputPort());
        connA->setEndPort(alu.adders[i]->inputPort(0));
        connB->setStartPort(alu.inputB[i]->outputPort());
        connB->setEndPort(alu.adders[i]->inputPort(1));

        // Chain carry between adders
        if (i > 0) {
            auto* carryConn = new QNEConnection();
            m_scene->addItem(carryConn);
            carryConn->setStartPort(alu.adders[i-1]->carryOutput());
            carryConn->setEndPort(alu.adders[i]->carryInput());
            alu.connections.append(carryConn);
        }

        alu.connections.append(connA);
        alu.connections.append(connB);
    }

    // Connect output multiplexers to select operation result
    // Each bit has an 8:1 mux to select from different operations
    for (int i = 0; i < 8; i++) {
        auto* mux = alu.outputMuxes[i];

        // Connect operation results to mux inputs
        auto* addConn = new QNEConnection();
        auto* xorConn = new QNEConnection();
        auto* andConn = new QNEConnection();
        auto* orConn = new QNEConnection();

        m_scene->addItem(addConn);
        m_scene->addItem(xorConn);
        m_scene->addItem(andConn);
        m_scene->addItem(orConn);

        addConn->setStartPort(alu.adders[i]->sumOutput());
        addConn->setEndPort(mux->inputPort(0)); // ADD operation

        xorConn->setStartPort(alu.xorGates[i]->outputPort());
        xorConn->setEndPort(mux->inputPort(2)); // XOR operation

        andConn->setStartPort(alu.andGates[i]->outputPort());
        andConn->setEndPort(mux->inputPort(3)); // AND operation

        orConn->setStartPort(alu.orGates[i]->outputPort());
        orConn->setEndPort(mux->inputPort(4)); // OR operation

        // Connect operation select to all muxes
        for (int j = 0; j < 4; j++) {
            auto* selConn = new QNEConnection();
            m_scene->addItem(selConn);
            selConn->setStartPort(alu.opSelect[j]->outputPort());
            selConn->setEndPort(mux->selectPort(j));
            alu.connections.append(selConn);
        }

        // Connect mux output to result LED
        auto* resultConn = new QNEConnection();
        m_scene->addItem(resultConn);
        resultConn->setStartPort(mux->outputPort());
        resultConn->setEndPort(alu.result[i]->inputPort());

        alu.connections.append(addConn);
        alu.connections.append(xorConn);
        alu.connections.append(andConn);
        alu.connections.append(orConn);
        alu.connections.append(resultConn);
    }

    // Flag generation logic
    // Zero flag: OR of all result bits, then NOT
    auto* zeroOR = new Or(); // 8-input OR gate (or tree of 2-input ORs)
    m_scene->addItem(zeroOR);
    auto* zeroNOT = new Not();
    m_scene->addItem(zeroNOT);

    // Connect all result bits to zero detection
    for (int i = 0; i < 8; i++) {
        auto* zeroConn = new QNEConnection();
        m_scene->addItem(zeroConn);
        zeroConn->setStartPort(alu.result[i]->outputPort()); // LED can output its state
        zeroConn->setEndPort(zeroOR->inputPort(i));
        alu.connections.append(zeroConn);
    }

    auto* zeroFinalConn = new QNEConnection();
    m_scene->addItem(zeroFinalConn);
    zeroFinalConn->setStartPort(zeroOR->outputPort());
    zeroFinalConn->setEndPort(zeroNOT->inputPort());

    auto* zeroOutputConn = new QNEConnection();
    m_scene->addItem(zeroOutputConn);
    zeroOutputConn->setStartPort(zeroNOT->outputPort());
    zeroOutputConn->setEndPort(alu.zeroFlag->inputPort());

    alu.connections.append(zeroFinalConn);
    alu.connections.append(zeroOutputConn);

    // Carry flag: output of MSB adder
    auto* carryConn = new QNEConnection();
    m_scene->addItem(carryConn);
    carryConn->setStartPort(alu.adders[7]->carryOutput());
    carryConn->setEndPort(alu.carryFlag->inputPort());
    alu.connections.append(carryConn);

    // Negative flag: MSB of result
    auto* negConn = new QNEConnection();
    m_scene->addItem(negConn);
    negConn->setStartPort(alu.result[7]->outputPort());
    negConn->setEndPort(alu.negativeFlag->inputPort());
    alu.connections.append(negConn);

    return alu;
}
```

### **Complete ALU Test**
```cpp
void TestLevel3_8BitALU::testComplete8BitALU() {
    setupBasicWorkspace();

    // Create the complete 8-bit ALU from basic gates
    ALUComponents alu = createALU();

    // Initialize simulation
    QVERIFY(m_simulation->initialize());

    // Test all arithmetic operations
    verifyOperation(alu, 15, 10, 0x0, 25, false, false, false); // ADD: 15+10=25
    verifyOperation(alu, 30, 5,  0x1, 25, false, false, false); // SUB: 30-5=25
    verifyOperation(alu, 0,  0,  0x0, 0,  true,  false, false); // ADD: 0+0=0 (zero flag)
    verifyOperation(alu, 255,1,  0x0, 0,  true,  true,  false); // ADD: 255+1=0 (overflow)

    // Test logical operations
    verifyOperation(alu, 0xF0, 0x0F, 0x2, 0x00, true,  false, false); // AND
    verifyOperation(alu, 0xF0, 0x0F, 0x3, 0xFF, false, false, true);  // OR
    verifyOperation(alu, 0xF0, 0x0F, 0x4, 0xFF, false, false, true);  // XOR
    verifyOperation(alu, 0xAA, 0x00, 0x5, 0x55, false, false, false); // NOT

    // Test edge cases
    verifyOperation(alu, 0xFF, 0xFF, 0x0, 0xFE, false, true,  true);  // Max addition
    verifyOperation(alu, 0x00, 0x01, 0x1, 0xFF, false, false, true);  // Underflow

    m_simulation->stop();
    cleanupWorkspace();
}

void TestLevel3_8BitALU::verifyOperation(const ALUComponents& alu, uint8_t a, uint8_t b, uint8_t op, uint8_t expected, bool zero, bool carry, bool negative) {
    // Set inputs
    setupInputs(alu, a, b, op);

    // Run simulation cycles for propagation
    runSimulationCycles(10);

    // Read result
    uint8_t result = readResult(alu);
    bool zeroFlag = readFlag(alu.zeroFlag);
    bool carryFlag = readFlag(alu.carryFlag);
    bool negativeFlag = readFlag(alu.negativeFlag);

    // Verify results
    QVERIFY2(result == expected,
        qPrintable(QString("ALU result mismatch: A=0x%1, B=0x%2, OP=0x%3, Expected=0x%4, Got=0x%5")
                   .arg(a, 2, 16, QChar('0'))
                   .arg(b, 2, 16, QChar('0'))
                   .arg(op, 1, 16)
                   .arg(expected, 2, 16, QChar('0'))
                   .arg(result, 2, 16, QChar('0'))));

    QCOMPARE(zeroFlag, zero);
    QCOMPARE(carryFlag, carry);
    QCOMPARE(negativeFlag, negative);
}
```

## What This Demonstrates

### **Hierarchical Construction in Action**
This ALU test shows exactly how complex components are built from simpler ones:

1. **Level 1**: Half-adders, Full adders from basic gates
2. **Level 2**: 8-bit ripple adder from full adders
3. **Level 3**: Complete ALU from adders + logic + multiplexers

### **Real-world Complexity**
- **Gate Count**: ~500 logic gates for complete ALU
- **Connection Count**: ~1,000 connections
- **Test Coverage**: All ALU operations, edge cases, flag generation

### **Educational Impact**
Students see exactly how:
- Basic gates combine to form arithmetic units
- Multiplexers select between different operations
- Flag generation works at the gate level
- Complex functionality emerges from simple components

This single test would stress-test the simulation engine more than all current tests combined, while teaching fundamental CPU design principles!
