// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9SingleCycleCpu8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using TestUtils::getInputStatus;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

/// ALU operation codes
enum ALUOp { ADD = 0, SUB = 1, AND = 2, OR = 3, XOR = 4, NOT = 5, SHL = 6, SHR = 7 };

/// Encode an 8-bit ALU instruction: ALUOp in bits [5:3], regAddr in bits [2:0].
/// Bit 7 = 0, bit 6 = 0 for ALU/register operations.
static int encodeInstruction(int aluOp, int regAddr)
{
    return ((aluOp & 0x7) << 3) | (regAddr & 0x7);
}

/// Encode a STORE instruction: write R0 to data memory at address regAddr.
/// Bit 7 = 1, bit 6 = 1 → MemWrite=1, RegWrite=0.
static int encodeStore(int regAddr)
{
    return 0xC0 | (regAddr & 0x7);
}

/// Encode a LOAD instruction: read data memory at address regAddr.
/// Bit 7 = 1, bit 6 = 0 → MemRead=1, RegWrite=0.
static int encodeLoad(int regAddr)
{
    return 0x80 | (regAddr & 0x7);
}

/// Fixture holding the full CPU test harness.
struct CPUFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *cpu = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    InputSwitch *progWrite = nullptr;
    QVector<InputSwitch *> progAddr;
    QVector<InputSwitch *> progData;
    InputSwitch *regProgWrite = nullptr;
    QVector<InputSwitch *> regProgAddr;
    QVector<InputSwitch *> regProgData;
    QVector<Led *> pcLeds;
    QVector<Led *> resultLeds;
    QVector<Led *> instrLeds;
    Led *zeroLed = nullptr;
    Led *signLed = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        cpu = loadBuildingBlockIC("level9_single_cycle_cpu_8bit.panda");
        builder.add(cpu);

        clk = new InputSwitch();
        reset = new InputSwitch();
        progWrite = new InputSwitch();
        regProgWrite = new InputSwitch();
        builder.add(clk, reset, progWrite, regProgWrite);

        for (int i = 0; i < 8; ++i) {
            auto *sw = new InputSwitch();
            builder.add(sw);
            progAddr.append(sw);
        }
        for (int i = 0; i < 8; ++i) {
            auto *sw = new InputSwitch();
            builder.add(sw);
            progData.append(sw);
        }
        for (int i = 0; i < 3; ++i) {
            auto *sw = new InputSwitch();
            builder.add(sw);
            regProgAddr.append(sw);
        }
        for (int i = 0; i < 8; ++i) {
            auto *sw = new InputSwitch();
            builder.add(sw);
            regProgData.append(sw);
        }

        // Connect inputs
        builder.connect(clk, 0, cpu, "Clock");
        builder.connect(reset, 0, cpu, "Reset");
        builder.connect(progWrite, 0, cpu, "ProgWrite");
        builder.connect(regProgWrite, 0, cpu, "RegProgWrite");

        for (int i = 0; i < 8; ++i) {
            builder.connect(progAddr[i], 0, cpu, QString("ProgAddr[%1]").arg(i));
            builder.connect(progData[i], 0, cpu, QString("ProgData[%1]").arg(i));
        }
        for (int i = 0; i < 3; ++i) {
            builder.connect(regProgAddr[i], 0, cpu, QString("RegProgAddr[%1]").arg(i));
        }
        for (int i = 0; i < 8; ++i) {
            builder.connect(regProgData[i], 0, cpu, QString("RegProgData[%1]").arg(i));
        }

        // Connect outputs
        for (int i = 0; i < 8; ++i) {
            auto *led = new Led();
            builder.add(led);
            pcLeds.append(led);
            builder.connect(cpu, QString("PC[%1]").arg(i), led, 0);
        }
        for (int i = 0; i < 8; ++i) {
            auto *led = new Led();
            builder.add(led);
            resultLeds.append(led);
            builder.connect(cpu, QString("Result[%1]").arg(i), led, 0);
        }
        for (int i = 0; i < 8; ++i) {
            auto *led = new Led();
            builder.add(led);
            instrLeds.append(led);
            builder.connect(cpu, QString("Instruction[%1]").arg(i), led, 0);
        }
        zeroLed = new Led();
        signLed = new Led();
        builder.add(zeroLed, signLed);
        builder.connect(cpu, "Zero", zeroLed, 0);
        builder.connect(cpu, "Sign", signLed, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    /// Program one instruction into instruction memory at the given address.
    void programInstruction(int addr, int instrByte)
    {
        setMultiBitInput(progAddr, addr);
        setMultiBitInput(progData, instrByte);
        progWrite->setOn(true);
        clockCycle(sim, clk);
        progWrite->setOn(false);
    }

    /// Write a value into a register via the programming port.
    void programRegister(int regIdx, int value)
    {
        setMultiBitInput(regProgAddr, regIdx);
        setMultiBitInput(regProgData, value);
        regProgWrite->setOn(true);
        clockCycle(sim, clk);
        regProgWrite->setOn(false);
    }

    /// Reset the CPU (pulse reset high then low).
    void resetCPU()
    {
        reset->setOn(true);
        clockCycle(sim, clk);
        reset->setOn(false);
        sim->update();
    }

    int readPC() { return readMultiBitOutput(QVector<GraphicElement *>(pcLeds.begin(), pcLeds.end()), 0); }
    int readResult() { return readMultiBitOutput(QVector<GraphicElement *>(resultLeds.begin(), resultLeds.end()), 0); }
    int readInstruction() { return readMultiBitOutput(QVector<GraphicElement *>(instrLeds.begin(), instrLeds.end()), 0); }
    bool readZero() { return getInputStatus(zeroLed); }
    bool readSign() { return getInputStatus(signLed); }
};

static std::unique_ptr<CPUFixture> s_cpu;

// ---------------------------------------------------------------------------
// Test implementations
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::initTestCase()
{
    s_cpu = std::make_unique<CPUFixture>();
    QVERIFY(s_cpu->build());
}

void TestLevel9SingleCycleCPU8Bit::cleanupTestCase()
{
    s_cpu.reset();
}

void TestLevel9SingleCycleCPU8Bit::cleanup()
{
}

void TestLevel9SingleCycleCPU8Bit::testCPUStructure()
{
    // Inputs: Clock + Reset + ProgAddr[0-7] + ProgData[0-7] + ProgWrite
    //       + RegProgAddr[0-2] + RegProgData[0-7] + RegProgWrite = 31
    QCOMPARE(s_cpu->cpu->inputSize(), 31);
    // Outputs: PC[0-7] + Result[0-7] + Instruction[0-7] + Zero + Sign = 26
    QCOMPARE(s_cpu->cpu->outputSize(), 26);
}

void TestLevel9SingleCycleCPU8Bit::testCPUReset()
{
    auto &f = *s_cpu;

    f.resetCPU();
    QCOMPARE(f.readPC(), 0);

    // Run 3 cycles
    for (int i = 0; i < 3; ++i) {
        clockCycle(f.sim, f.clk);
    }
    QVERIFY(f.readPC() > 0);

    // Reset mid-execution
    f.resetCPU();
    QCOMPARE(f.readPC(), 0);
}

void TestLevel9SingleCycleCPU8Bit::testPCIncrement()
{
    auto &f = *s_cpu;
    f.resetCPU();

    for (int i = 0; i < 5; ++i) {
        QCOMPARE(f.readPC(), i);
        clockCycle(f.sim, f.clk);
    }
    QCOMPARE(f.readPC(), 5);
}

// ---------------------------------------------------------------------------
// Helper: set up a single ALU instruction test
// ---------------------------------------------------------------------------

static void addALUTestColumns()
{
    QTest::addColumn<int>("operandA");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedSign");
}

/// Run one ALU instruction through the full CPU pipeline and verify.
static void runALUTest(int aluOp)
{
    QFETCH(int, operandA);
    QFETCH(int, operandB);
    QFETCH(int, expectedResult);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedSign);

    auto &f = *s_cpu;

    const int regAddr = 1;

    // 1. Program register R1 with operandB
    f.programRegister(regAddr, operandB);

    // 2. Program register R0 (accumulator) with operandA
    f.programRegister(0, operandA);

    // 3. Program instruction at address 0: ALU_OP R1
    f.programInstruction(0, encodeInstruction(aluOp, regAddr));

    // 4. Reset CPU — the ALU result is available combinationally before the clock edge
    f.resetCPU();

    // 5. Verify (read before clockCycle — single-cycle result is live on the ALU output)
    QCOMPARE(f.readResult(), expectedResult);
    QCOMPARE(f.readZero(), expectedZero);
    QCOMPARE(f.readSign(), expectedSign);
}

// ---------------------------------------------------------------------------
// ADD tests
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_ADD_data()
{
    addALUTestColumns();
    QTest::newRow("3+5=8")           << 3   << 5   << 8    << false << false;
    QTest::newRow("0+0=0")           << 0   << 0   << 0    << true  << false;
    QTest::newRow("200+100=44 wrap") << 200 << 100 << 44   << false << false;
    QTest::newRow("128+128=0")       << 128 << 128 << 0    << true  << false;
    QTest::newRow("255+1=0")         << 255 << 1   << 0    << true  << false;
    QTest::newRow("0x42+0x33=0x75")  << 0x42 << 0x33 << 0x75 << false << false;
    QTest::newRow("0x0F+0x01=0x10 nibble carry") << 0x0F << 0x01 << 0x10 << false << false;
    QTest::newRow("0x7F+0x01=0x80 sign boundary") << 0x7F << 0x01 << 0x80 << false << true;
    QTest::newRow("N+0 identity")    << 0x42 << 0    << 0x42 << false << false;
    QTest::newRow("0xFF+0xFF=0xFE")  << 0xFF << 0xFF << 0xFE << false << true;
}

void TestLevel9SingleCycleCPU8Bit::testISA_ADD()
{
    runALUTest(ADD);
}

// ---------------------------------------------------------------------------
// SUB tests
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_SUB_data()
{
    addALUTestColumns();
    QTest::newRow("5-3=2")           << 5   << 3   << 2    << false << false;
    QTest::newRow("0-0=0")           << 0   << 0   << 0    << true  << false;
    QTest::newRow("0x50-0x30=0x20")  << 0x50 << 0x30 << 0x20 << false << false;
    QTest::newRow("0x42-0x42=0")     << 0x42 << 0x42 << 0x00 << true  << false;
    QTest::newRow("0x30-0x50=0xE0")  << 0x30 << 0x50 << 0xE0 << false << true;
    QTest::newRow("0-1=0xFF")        << 0   << 1   << 0xFF << false << true;
    QTest::newRow("0x10-0x01=0x0F nibble borrow")  << 0x10 << 0x01 << 0x0F << false << false;
    QTest::newRow("0xFF-0xFF=0")     << 0xFF << 0xFF << 0x00 << true  << false;
    QTest::newRow("0x80-0x01=0x7F sign boundary")  << 0x80 << 0x01 << 0x7F << false << false;
    QTest::newRow("N-0 identity")    << 0x42 << 0    << 0x42 << false << false;
    QTest::newRow("1-0xFF=0x02")     << 1    << 0xFF << 0x02 << false << false;
}

void TestLevel9SingleCycleCPU8Bit::testISA_SUB()
{
    runALUTest(SUB);
}

// ---------------------------------------------------------------------------
// AND tests
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_AND_data()
{
    addALUTestColumns();
    QTest::newRow("0xF0&0x0F=0x00")  << 0xF0 << 0x0F << 0x00 << true  << false;
    QTest::newRow("0xFF&0x42=0x42 identity")  << 0xFF << 0x42 << 0x42 << false << false;
    QTest::newRow("0xAA&0x55=0x00")  << 0xAA << 0x55 << 0x00 << true  << false;
    QTest::newRow("0xFF&0xFF=0xFF")  << 0xFF << 0xFF << 0xFF << false << true;
    QTest::newRow("N&0x00=0x00 annihilator") << 0x42 << 0x00 << 0x00 << true  << false;
    QTest::newRow("0x80&0x80=0x80")  << 0x80 << 0x80 << 0x80 << false << true;
}

void TestLevel9SingleCycleCPU8Bit::testISA_AND()
{
    runALUTest(AND);
}

// ---------------------------------------------------------------------------
// OR tests
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_OR_data()
{
    addALUTestColumns();
    QTest::newRow("0xF0|0x0F=0xFF")  << 0xF0 << 0x0F << 0xFF << false << true;
    QTest::newRow("N|0x00=N identity") << 0x42 << 0x00 << 0x42 << false << false;
    QTest::newRow("0x00|0x00=0x00")  << 0x00 << 0x00 << 0x00 << true  << false;
    QTest::newRow("0xAA|0x55=0xFF")  << 0xAA << 0x55 << 0xFF << false << true;
    QTest::newRow("N|0xFF=0xFF annihilator") << 0x42 << 0xFF << 0xFF << false << true;
    QTest::newRow("0x80|0x01=0x81")  << 0x80 << 0x01 << 0x81 << false << true;
}

void TestLevel9SingleCycleCPU8Bit::testISA_OR()
{
    runALUTest(OR);
}

// ---------------------------------------------------------------------------
// XOR tests
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_XOR_data()
{
    addALUTestColumns();
    QTest::newRow("0xFF^0xFF=0x00")  << 0xFF << 0xFF << 0x00 << true  << false;
    QTest::newRow("N^0x00=N identity") << 0xFF << 0x00 << 0xFF << false << true;
    QTest::newRow("0xAA^0x55=0xFF")  << 0xAA << 0x55 << 0xFF << false << true;
    QTest::newRow("N^N=0 self-cancel") << 0x42 << 0x42 << 0x00 << true  << false;
    QTest::newRow("0x80^0x7F=0xFF")  << 0x80 << 0x7F << 0xFF << false << true;
    QTest::newRow("0x0F^0xF0=0xFF")  << 0x0F << 0xF0 << 0xFF << false << true;
}

void TestLevel9SingleCycleCPU8Bit::testISA_XOR()
{
    runALUTest(XOR);
}

// ---------------------------------------------------------------------------
// NOT tests (unary — operandB is ignored by ALU, operandA is complemented)
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_NOT_data()
{
    addALUTestColumns();
    QTest::newRow("~0x00=0xFF")  << 0x00 << 0x00 << 0xFF << false << true;
    QTest::newRow("~0xFF=0x00")  << 0xFF << 0x00 << 0x00 << true  << false;
    QTest::newRow("~0x42=0xBD")  << 0x42 << 0x00 << 0xBD << false << true;
    QTest::newRow("~0x80=0x7F")  << 0x80 << 0x00 << 0x7F << false << false;
    QTest::newRow("~0x55=0xAA")  << 0x55 << 0x00 << 0xAA << false << true;
    QTest::newRow("~0xAA=0x55")  << 0xAA << 0x00 << 0x55 << false << false;
    QTest::newRow("~0x01=0xFE")  << 0x01 << 0x00 << 0xFE << false << true;
    QTest::newRow("~0x7F=0x80")  << 0x7F << 0x00 << 0x80 << false << true;
}

void TestLevel9SingleCycleCPU8Bit::testISA_NOT()
{
    runALUTest(NOT);
}

// ---------------------------------------------------------------------------
// SHL tests (shift left by 1 — operandA << 1)
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_SHL_data()
{
    addALUTestColumns();
    QTest::newRow("0x42<<1=0x84")  << 0x42 << 0x00 << 0x84 << false << true;
    QTest::newRow("0x01<<1=0x02")  << 0x01 << 0x00 << 0x02 << false << false;
    QTest::newRow("0x80<<1=0x00 MSB lost")  << 0x80 << 0x00 << 0x00 << true  << false;
    QTest::newRow("0x00<<1=0x00")  << 0x00 << 0x00 << 0x00 << true  << false;
    QTest::newRow("0xFF<<1=0xFE")  << 0xFF << 0x00 << 0xFE << false << true;
    QTest::newRow("0x7F<<1=0xFE")  << 0x7F << 0x00 << 0xFE << false << true;
    QTest::newRow("0xAA<<1=0x54")  << 0xAA << 0x00 << 0x54 << false << false;
    QTest::newRow("0x55<<1=0xAA")  << 0x55 << 0x00 << 0xAA << false << true;
}

void TestLevel9SingleCycleCPU8Bit::testISA_SHL()
{
    runALUTest(SHL);
}

// ---------------------------------------------------------------------------
// SHR tests (shift right by 1 — operandA >> 1)
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_SHR_data()
{
    addALUTestColumns();
    QTest::newRow("0x84>>1=0x42")  << 0x84 << 0x00 << 0x42 << false << false;
    QTest::newRow("0xFF>>1=0x7F")  << 0xFF << 0x00 << 0x7F << false << false;
    QTest::newRow("0x01>>1=0x00 LSB lost") << 0x01 << 0x00 << 0x00 << true  << false;
    QTest::newRow("0x00>>1=0x00")  << 0x00 << 0x00 << 0x00 << true  << false;
    QTest::newRow("0x80>>1=0x40")  << 0x80 << 0x00 << 0x40 << false << false;
    QTest::newRow("0x7F>>1=0x3F")  << 0x7F << 0x00 << 0x3F << false << false;
    QTest::newRow("0xAA>>1=0x55")  << 0xAA << 0x00 << 0x55 << false << false;
    QTest::newRow("0x55>>1=0x2A")  << 0x55 << 0x00 << 0x2A << false << false;
}

void TestLevel9SingleCycleCPU8Bit::testISA_SHR()
{
    runALUTest(SHR);
}

// ---------------------------------------------------------------------------
// Zero flag tests (across operations)
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testZeroFlag_data()
{
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("operandA");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<bool>("expectedZero");

    QTest::newRow("ADD 0+0 -> zero")       << static_cast<int>(ADD) << 0    << 0    << true;
    QTest::newRow("ADD 1+0 -> not zero")   << static_cast<int>(ADD) << 1    << 0    << false;
    QTest::newRow("SUB 5-5 -> zero")       << static_cast<int>(SUB) << 5    << 5    << true;
    QTest::newRow("AND 0xAA&0x55 -> zero") << static_cast<int>(AND) << 0xAA << 0x55 << true;
    QTest::newRow("XOR 0x42^0x42 -> zero") << static_cast<int>(XOR) << 0x42 << 0x42 << true;
    QTest::newRow("NOT ~0xFF -> zero")     << static_cast<int>(NOT) << 0xFF << 0    << true;
    QTest::newRow("SHL 0x80<<1 -> zero")   << static_cast<int>(SHL) << 0x80 << 0    << true;
    QTest::newRow("SHR 0x01>>1 -> zero")   << static_cast<int>(SHR) << 0x01 << 0    << true;
}

void TestLevel9SingleCycleCPU8Bit::testZeroFlag()
{
    QFETCH(int, aluOp);
    QFETCH(int, operandA);
    QFETCH(int, operandB);
    QFETCH(bool, expectedZero);

    auto &f = *s_cpu;

    f.programRegister(1, operandB);
    f.programRegister(0, operandA);
    f.programInstruction(0, encodeInstruction(aluOp, 1));
    f.resetCPU();

    QCOMPARE(f.readZero(), expectedZero);
}

// ---------------------------------------------------------------------------
// Sign flag tests (bit 7 of result set)
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testSignFlag_data()
{
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("operandA");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<bool>("expectedSign");

    QTest::newRow("ADD 0x42+0x33 -> no sign")    << static_cast<int>(ADD) << 0x42 << 0x33 << false;
    QTest::newRow("SUB 0x30-0x50 -> sign")        << static_cast<int>(SUB) << 0x30 << 0x50 << true;
    QTest::newRow("OR 0xF0|0x0F -> sign")         << static_cast<int>(OR)  << 0xF0 << 0x0F << true;
    QTest::newRow("NOT ~0x00 -> sign")            << static_cast<int>(NOT) << 0x00 << 0x00 << true;
    QTest::newRow("SHL 0x42<<1=0x84 -> sign")     << static_cast<int>(SHL) << 0x42 << 0x00 << true;
    QTest::newRow("SHR 0x84>>1=0x42 -> no sign")  << static_cast<int>(SHR) << 0x84 << 0x00 << false;
}

void TestLevel9SingleCycleCPU8Bit::testSignFlag()
{
    QFETCH(int, aluOp);
    QFETCH(int, operandA);
    QFETCH(int, operandB);
    QFETCH(bool, expectedSign);

    auto &f = *s_cpu;

    f.programRegister(1, operandB);
    f.programRegister(0, operandA);
    f.programInstruction(0, encodeInstruction(aluOp, 1));
    f.resetCPU();

    QCOMPARE(f.readSign(), expectedSign);
}

// ---------------------------------------------------------------------------
// Multi-instruction sequence: programs 3 instructions and verifies final result
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testMultipleInstructions()
{
    auto &f = *s_cpu;
    f.sim->restart();
    f.sim->update();

    // Pre-load registers
    f.programRegister(0, 0x10);  // R0 (accumulator) = 0x10
    f.programRegister(1, 0x20);  // R1 = 0x20
    f.programRegister(2, 0x05);  // R2 = 0x05

    // Program instructions:
    // Addr 0: ADD R1  -> R0 = 0x10 + 0x20 = 0x30
    // Addr 1: SUB R2  -> R0 = 0x30 - 0x05 = 0x2B
    // Addr 2: AND R1  -> R0 = 0x2B & 0x20 = 0x20
    f.programInstruction(0, encodeInstruction(ADD, 1));
    f.programInstruction(1, encodeInstruction(SUB, 2));
    f.programInstruction(2, encodeInstruction(AND, 1));

    f.resetCPU();

    // Instruction at address 0: ADD R1 -> R0 = 0x10 + 0x20 = 0x30
    QCOMPARE(f.readResult(), 0x30);
    clockCycle(f.sim, f.clk);  // latch R0=0x30, advance PC to 1
    f.sim->update();

    // Instruction at address 1: SUB R2 -> R0 = 0x30 - 0x05 = 0x2B
    QCOMPARE(f.readResult(), 0x2B);
    clockCycle(f.sim, f.clk);  // latch R0=0x2B, advance PC to 2
    f.sim->update();

    // Instruction at address 2: AND R1 -> R0 = 0x2B & 0x20 = 0x20
    QCOMPARE(f.readResult(), 0x20);
}

// ---------------------------------------------------------------------------
// STORE test: write R0 to data memory, verify by LOADing it back
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_STORE()
{
    auto &f = *s_cpu;
    f.sim->restart();
    f.sim->update();

    // Helper: store R0 to address, then load back and verify
    auto storeAndVerify = [&](int value, int addr) {
        f.programRegister(0, value);
        f.programInstruction(0, encodeStore(addr));
        f.programInstruction(1, encodeLoad(addr));
        f.resetCPU();
        clockCycle(f.sim, f.clk);  // execute STORE on clock edge
        f.sim->update();
        QCOMPARE(f.readResult(), value);  // LOAD reads back the stored value
    };

    storeAndVerify(0x42, 3);   // normal value, middle address
    storeAndVerify(0x00, 0);   // zero value, address 0
    storeAndVerify(0xFF, 7);   // max value, max address
    storeAndVerify(0x80, 1);   // sign bit set
    storeAndVerify(0x01, 6);   // min nonzero
}

// ---------------------------------------------------------------------------
// LOAD test: verify that LOAD reads from data memory
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_LOAD()
{
    auto &f = *s_cpu;
    f.sim->restart();
    f.sim->update();

    // Data memory starts at all zeros. LOAD from any address should give 0.
    f.programInstruction(0, encodeLoad(0));
    f.resetCPU();
    QCOMPARE(f.readResult(), 0x00);
    QCOMPARE(f.readZero(), true);

    // LOAD from address 7 (also empty)
    f.programInstruction(0, encodeLoad(7));
    f.resetCPU();
    QCOMPARE(f.readResult(), 0x00);
    QCOMPARE(f.readZero(), true);
}

// ---------------------------------------------------------------------------
// Store-Load round-trip: store multiple values, load them back
// ---------------------------------------------------------------------------

void TestLevel9SingleCycleCPU8Bit::testISA_StoreLoad()
{
    auto &f = *s_cpu;
    f.sim->restart();
    f.sim->update();

    // Store 0xAB to mem[2], change R0 to 0xCD, store to mem[3], then load both back.
    // Pre-load registers
    f.programRegister(0, 0xAB);
    f.programRegister(1, 0xCD);

    // Program full instruction sequence before running:
    // 0: STORE 2        → mem[2] = R0 (0xAB)
    // 1: XOR R0         → R0 = R0 ^ R0 = 0 (clear accumulator)
    // 2: ADD R1         → R0 = 0 + R1 = 0xCD
    // 3: STORE 3        → mem[3] = R0 (0xCD)
    // 4: LOAD 2         → Result = mem[2] (expect 0xAB)
    // 5: LOAD 3         → Result = mem[3] (expect 0xCD)
    f.programInstruction(0, encodeStore(2));
    f.programInstruction(1, encodeInstruction(XOR, 0));
    f.programInstruction(2, encodeInstruction(ADD, 1));
    f.programInstruction(3, encodeStore(3));
    f.programInstruction(4, encodeLoad(2));
    f.programInstruction(5, encodeLoad(3));

    f.resetCPU();

    // Execute instructions 0-3 (STORE, XOR, ADD, STORE)
    for (int i = 0; i < 4; ++i) {
        clockCycle(f.sim, f.clk);
        f.sim->update();
    }

    // Now PC=4, instruction@4=LOAD 2 is live. Result = mem[2].
    QCOMPARE(f.readResult(), 0xAB);

    clockCycle(f.sim, f.clk);  // advance PC to 5
    f.sim->update();

    // instruction@5=LOAD 3 is live. Result = mem[3].
    QCOMPARE(f.readResult(), 0xCD);
}

