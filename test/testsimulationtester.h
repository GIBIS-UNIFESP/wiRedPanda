// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>
#include <functional>

class LogicInput;
class LogicOutput;
class Scene;
class Simulation;

class TestSimulationTester : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();
    
    // Phase 1: Basic Logic Gates
    void testBasicGates_AND();
    void testBasicGates_OR();
    void testBasicGates_NOT();
    void testBasicGates_XOR();
    void testBasicGates_NAND();
    void testBasicGates_NOR();
    void testBasicGates_XNOR();
    void testBasicGates_Buffer();
    
    // Phase 2: Sequential Logic
    void testSequentialLogic_DFlipFlop();
    void testSequentialLogic_JKFlipFlop();
    void testSequentialLogic_TFlipFlop();
    void testSequentialLogic_SRFlipFlop();
    void testSequentialLogic_SRLatch();
    void testSequentialLogic_DLatch();
    
    // Phase 3: Arithmetic Logic
    void testArithmetic_HalfAdder();
    void testArithmetic_FullAdder();
    void testArithmetic_RippleCarryAdder();
    void testArithmetic_Subtractor();
    void testArithmetic_Comparator();
    
    // Phase 4: Control Logic
    void testControl_Multiplexer();
    void testControl_Demultiplexer();
    void testControl_Decoder();
    void testControl_Encoder();
    void testControl_PriorityEncoder();
    
    // Phase 5: Memory Components
    void testMemory_Register();
    void testMemory_ShiftRegister();
    void testMemory_Counter();
    void testMemory_RAM();
    void testMemory_ROM();
    
    // Phase 6: Simple CPU Components
    void testCPU_ProgramCounter();
    void testCPU_InstructionRegister();
    void testCPU_ALU();
    void testCPU_ControlUnit();
    void testCPU_SimpleCPU();

private:
    QVector<LogicInput *> m_inputs{16};  // Support up to 16 inputs for complex tests
    QVector<LogicOutput *> m_outputs{16}; // Support up to 16 outputs
    Scene *m_scene{nullptr};
    Simulation *m_simulation{nullptr};
    
    // Helper methods
    void setupBasicCircuit(int numInputs, int numOutputs);
    void connectInputsToOutputs();
    void setInputValue(int index, bool value);
    bool getOutputValue(int index);
    void updateSimulation(int cycles = 1);
    void logTestProgress(const QString &phase, const QString &component, const QString &status);
};
