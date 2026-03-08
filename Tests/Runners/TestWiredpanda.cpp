// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Runners/RunnerUtils.h"

// integration/ic/tests/cpu
#include "Tests/Integration/IC/Tests/Cpu/TestCPUAlu.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPUBranch.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPUControlUnit.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPUDecoders.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPUInstructionExecute.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPUInstructionFetch.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPUIntegration.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPUMemoryInterface.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPUProgramCounter.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPURegisterBank.h"
#include "Tests/Integration/IC/Tests/Cpu/TestCPURegisters.h"
// integration/ic/tests
#include "Tests/Integration/IC/Tests/TestLevel1DFlipFlop.h"
#include "Tests/Integration/IC/Tests/TestLevel1DLatch.h"
#include "Tests/Integration/IC/Tests/TestLevel1JkFlipFlop.h"
#include "Tests/Integration/IC/Tests/TestLevel1SrLatch.h"
#include "Tests/Integration/IC/Tests/TestLevel2Decoder2to4.h"
#include "Tests/Integration/IC/Tests/TestLevel2Decoder3to8.h"
#include "Tests/Integration/IC/Tests/TestLevel2Decoder4to16.h"
#include "Tests/Integration/IC/Tests/TestLevel2FullAdder1bit.h"
#include "Tests/Integration/IC/Tests/TestLevel2HalfAdder.h"
#include "Tests/Integration/IC/Tests/TestLevel2Mux2to1.h"
#include "Tests/Integration/IC/Tests/TestLevel2Mux4to1.h"
#include "Tests/Integration/IC/Tests/TestLevel2Mux8to1.h"
#include "Tests/Integration/IC/Tests/TestLevel2ParityChecker.h"
#include "Tests/Integration/IC/Tests/TestLevel2ParityGenerator.h"
#include "Tests/Integration/IC/Tests/TestLevel2PriorityEncoder8to3.h"
#include "Tests/Integration/IC/Tests/TestLevel2PriorityMux3to1.h"
#include "Tests/Integration/IC/Tests/TestLevel3AluSelector5way.h"
#include "Tests/Integration/IC/Tests/TestLevel3Bcd7segmentDecoder.h"
#include "Tests/Integration/IC/Tests/TestLevel3Comparator4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel3Comparator4bitEquality.h"
#include "Tests/Integration/IC/Tests/TestLevel3Register1bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4BinaryCounter4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4BusMux4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4BusMux8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4Comparator4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4JohnsonCounter4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4Ram4x1.h"
#include "Tests/Integration/IC/Tests/TestLevel4Ram8x1.h"
#include "Tests/Integration/IC/Tests/TestLevel4Register4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4RingCounter4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4RippleAdder4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4RippleAlu4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel4ShiftRegisterPiso.h"
#include "Tests/Integration/IC/Tests/TestLevel4ShiftRegisterSipo.h"
#include "Tests/Integration/IC/Tests/TestLevel5BarrelRotator.h"
#include "Tests/Integration/IC/Tests/TestLevel5BarrelShifter4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel5ClockGatedDecoder.h"
#include "Tests/Integration/IC/Tests/TestLevel5Controller4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel5InstructionDecoder4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel5LoadableCounter4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel5ModuloCounter4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel5ProgramCounter4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel5RegisterFile4x4.h"
#include "Tests/Integration/IC/Tests/TestLevel5RegisterFile8x8.h"
#include "Tests/Integration/IC/Tests/TestLevel6StackMemoryInterface.h"
#include "Tests/Integration/IC/Tests/TestLevel5UpDownCounter4bit.h"
#include "Tests/Integration/IC/Tests/TestLevel6Alu8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel6ProgramCounter8bitArithmetic.h"
#include "Tests/Integration/IC/Tests/TestLevel6Ram256x8.h"
#include "Tests/Integration/IC/Tests/TestLevel6Register8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel6RegisterFile8x8.h"
#include "Tests/Integration/IC/Tests/TestLevel6RippleAdder8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel6StackPointer8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel7Alu16bit.h"
#include "Tests/Integration/IC/Tests/TestLevel7CpuProgramCounter8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel7DataForwardingUnit.h"
#include "Tests/Integration/IC/Tests/TestLevel7ExecutionDatapath.h"
#include "Tests/Integration/IC/Tests/TestLevel7FlagRegister.h"
#include "Tests/Integration/IC/Tests/TestLevel7InstructionDecoder8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel7InstructionMemoryInterface.h"
#include "Tests/Integration/IC/Tests/TestLevel7InstructionRegister8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel8DecodeStage.h"
#include "Tests/Integration/IC/Tests/TestLevel8ExecuteStage.h"
#include "Tests/Integration/IC/Tests/TestLevel8FetchStage.h"
#include "Tests/Integration/IC/Tests/TestLevel8MemoryStage.h"
#include "Tests/Integration/IC/Tests/TestLevel9Cpu16bitRisc.h"
#include "Tests/Integration/IC/Tests/TestLevel9FetchStage16bit.h"
#include "Tests/Integration/IC/Tests/TestLevel9MultiCycleCpu8bit.h"
#include "Tests/Integration/IC/Tests/TestLevel9SingleCycleCpu8bit.h"
// integration/ic/tests/tests_without_panda
#include "Tests/Integration/IC/Tests/TestsWithoutPanda/TestDecoder8to256.h"
#include "Tests/Integration/IC/Tests/TestsWithoutPanda/TestMemorySettlingTime.h"
#include "Tests/Integration/IC/Tests/TestsWithoutPanda/TestRamCell1bit.h"
#include "Tests/Integration/IC/Tests/TestsWithoutPanda/TestSequential.h"
// integration/logic
#include "Tests/Integration/Logic/TestMuxDemuxComprehensive.h"
// integration
#include "Tests/Integration/TestArduino.h"
#include "Tests/Integration/TestFeedback.h"
#include "Tests/Integration/TestFiles.h"
#include "Tests/Integration/TestIc.h"
#include "Tests/Integration/TestSimulation.h"
#include "Tests/Integration/TestWorkspace.h"
#include "Tests/Integration/TestWorkspaceFileops.h"
// resources
#include "Tests/Resources/TestIcons.h"
// system
#include "Tests/System/TestWaveform.h"
// unit/commands
#include "Tests/Unit/Commands/TestCommands.h"
// unit/common
#include "Tests/Unit/Common/TestCommon.h"
#include "Tests/Unit/Common/TestEnums.h"
#include "Tests/Unit/Common/TestRecentFiles.h"
#include "Tests/Unit/Common/TestSettings.h"
#include "Tests/Unit/Common/TestThemeManager.h"
// unit/elements
#include "Tests/Unit/Elements/TestAudioBox.h"
#include "Tests/Unit/Elements/TestBuzzer.h"
#include "Tests/Unit/Elements/TestClock.h"
#include "Tests/Unit/Elements/TestClocksAdvanced.h"
#include "Tests/Unit/Elements/TestComponents.h"
#include "Tests/Unit/Elements/TestDisplays.h"
#include "Tests/Unit/Elements/TestElementLabel.h"
#include "Tests/Unit/Elements/TestElementProperties.h"
#include "Tests/Unit/Elements/TestFeatures.h"
#include "Tests/Unit/Elements/TestGeometry.h"
#include "Tests/Unit/Elements/TestGraphicelementAdvanced.h"
#include "Tests/Unit/Elements/TestInputElements.h"
#include "Tests/Unit/Elements/TestInputRotary.h"
#include "Tests/Unit/Elements/TestLed.h"
#include "Tests/Unit/Elements/TestLogicGates.h"
#include "Tests/Unit/Elements/TestMultiplexing.h"
#include "Tests/Unit/Elements/TestSequentialLogic.h"
#include "Tests/Unit/Elements/TestTruthTable.h"
// unit/factory
#include "Tests/Unit/Factory/TestElementFactory.h"
// unit/logic
#include "Tests/Unit/Logic/TestLogicElements.h"
#include "Tests/Unit/Logic/TestLogicElementsErrors.h"
#include "Tests/Unit/Logic/TestLogicNode.h"
// unit/nodes
#include "Tests/Unit/Nodes/TestConnectionSerialization.h"
#include "Tests/Unit/Nodes/TestConnections.h"
// unit/scene
#include "Tests/Unit/Scene/TestConnectionValidity.h"
#include "Tests/Unit/Scene/TestScene.h"
#include "Tests/Unit/Scene/TestSceneConnections.h"
#include "Tests/Unit/Scene/TestSceneState.h"
#include "Tests/Unit/Scene/TestSceneUndoredo.h"
// unit/serialization
#include "Tests/Unit/Serialization/TestSerialization.h"
// unit/simulation
#include "Tests/Unit/Simulation/TestElementMapping.h"
#include "Tests/Unit/Simulation/TestSimulationBlocker.h"
// unit/ui
#include "Tests/Unit/Ui/TestDialogs.h"

int main(int argc, char **argv)
{
    return runTestSuite(argc, argv, {
        {"TestCPUAlu", []() -> QObject * { return new TestCPUAlu; }},
        {"TestCPUBranch", []() -> QObject * { return new TestCPUBranch; }},
        {"TestCPUControlUnit", []() -> QObject * { return new TestCPUControlUnit; }},
        {"TestCPUDecoders", []() -> QObject * { return new TestCPUDecoders; }},
        {"TestCPUInstructionExecute", []() -> QObject * { return new TestCPUInstructionExecute; }},
        {"TestCPUInstructionFetch", []() -> QObject * { return new TestCPUInstructionFetch; }},
        {"TestCPUIntegration", []() -> QObject * { return new TestCPUIntegration; }},
        {"TestCPUMemoryInterface", []() -> QObject * { return new TestCPUMemoryInterface; }},
        {"TestCPUProgramCounter", []() -> QObject * { return new TestCPUProgramCounter; }},
        {"TestCPURegisterBank", []() -> QObject * { return new TestCPURegisterBank; }},
        {"TestCPURegisters", []() -> QObject * { return new TestCPURegisters; }},
        {"TestLevel1DFlipFlop", []() -> QObject * { return new TestLevel1DFlipFlop; }},
        {"TestLevel1DLatch", []() -> QObject * { return new TestLevel1DLatch; }},
        {"TestLevel1JKFlipFlop", []() -> QObject * { return new TestLevel1JKFlipFlop; }},
        {"TestLevel1SRLatch", []() -> QObject * { return new TestLevel1SRLatch; }},
        {"TestLevel2Decoder2To4", []() -> QObject * { return new TestLevel2Decoder2To4; }},
        {"TestLevel2Decoder3To8", []() -> QObject * { return new TestLevel2Decoder3To8; }},
        {"TestLevel2Decoder4To16", []() -> QObject * { return new TestLevel2Decoder4To16; }},
        {"TestLevel2FullAdder1Bit", []() -> QObject * { return new TestLevel2FullAdder1Bit; }},
        {"TestLevel2HalfAdder", []() -> QObject * { return new TestLevel2HalfAdder; }},
        {"TestLevel2MUX2To1", []() -> QObject * { return new TestLevel2MUX2To1; }},
        {"TestLevel2MUX4To1", []() -> QObject * { return new TestLevel2MUX4To1; }},
        {"TestLevel2MUX8To1", []() -> QObject * { return new TestLevel2MUX8To1; }},
        {"TestLevel2ParityChecker", []() -> QObject * { return new TestLevel2ParityChecker; }},
        {"TestLevel2ParityGenerator", []() -> QObject * { return new TestLevel2ParityGenerator; }},
        {"TestLevel2PriorityEncoder8To3", []() -> QObject * { return new TestLevel2PriorityEncoder8To3; }},
        {"TestLevel2PriorityMUX3To1", []() -> QObject * { return new TestLevel2PriorityMUX3To1; }},
        {"TestLevel3ALUSelector5Way", []() -> QObject * { return new TestLevel3ALUSelector5Way; }},
        {"TestLevel3BCD7SegmentDecoder", []() -> QObject * { return new TestLevel3BCD7SegmentDecoder; }},
        {"TestLevel3Comparator4Bit", []() -> QObject * { return new TestLevel3Comparator4Bit; }},
        {"TestLevel3Comparator4BitEquality", []() -> QObject * { return new TestLevel3Comparator4BitEquality; }},
        {"TestLevel3Register1Bit", []() -> QObject * { return new TestLevel3Register1Bit; }},
        {"TestLevel4BinaryCounter4Bit", []() -> QObject * { return new TestLevel4BinaryCounter4Bit; }},
        {"TestLevel4BusMUX4Bit", []() -> QObject * { return new TestLevel4BusMUX4Bit; }},
        {"TestLevel4BusMUX8Bit", []() -> QObject * { return new TestLevel4BusMUX8Bit; }},
        {"TestLevel4Comparator4Bit", []() -> QObject * { return new TestLevel4Comparator4Bit; }},
        {"TestLevel4JohnsonCounter4Bit", []() -> QObject * { return new TestLevel4JohnsonCounter4Bit; }},
        {"TestLevel4RAM4X1", []() -> QObject * { return new TestLevel4RAM4X1; }},
        {"TestLevel4RAM8X1", []() -> QObject * { return new TestLevel4RAM8X1; }},
        {"TestLevel4Register4Bit", []() -> QObject * { return new TestLevel4Register4Bit; }},
        {"TestLevel4RingCounter4Bit", []() -> QObject * { return new TestLevel4RingCounter4Bit; }},
        {"TestLevel4RippleAdder4Bit", []() -> QObject * { return new TestLevel4RippleAdder4Bit; }},
        {"TestLevel4RippleALU4Bit", []() -> QObject * { return new TestLevel4RippleALU4Bit; }},
        {"TestLevel4ShiftRegisterPISO", []() -> QObject * { return new TestLevel4ShiftRegisterPISO; }},
        {"TestLevel4ShiftRegisterSIPO", []() -> QObject * { return new TestLevel4ShiftRegisterSIPO; }},
        {"TestLevel5BarrelRotator", []() -> QObject * { return new TestLevel5BarrelRotator; }},
        {"TestLevel5BarrelShifter4Bit", []() -> QObject * { return new TestLevel5BarrelShifter4Bit; }},
        {"TestLevel5ClockGatedDecoder", []() -> QObject * { return new TestLevel5ClockGatedDecoder; }},
        {"TestLevel5Controller4Bit", []() -> QObject * { return new TestLevel5Controller4Bit; }},
        {"TestLevel5InstructionDecoder4Bit", []() -> QObject * { return new TestLevel5InstructionDecoder4Bit; }},
        {"TestLevel5LoadableCounter4Bit", []() -> QObject * { return new TestLevel5LoadableCounter4Bit; }},
        {"TestLevel5ModuloCounter4Bit", []() -> QObject * { return new TestLevel5ModuloCounter4Bit; }},
        {"TestLevel5ProgramCounter4Bit", []() -> QObject * { return new TestLevel5ProgramCounter4Bit; }},
        {"TestLevel5RegisterFile4X4", []() -> QObject * { return new TestLevel5RegisterFile4X4; }},
        {"TestLevel5RegisterFile8X8", []() -> QObject * { return new TestLevel5RegisterFile8X8; }},
        {"TestLevel6StackMemoryInterface", []() -> QObject * { return new TestLevel6StackMemoryInterface; }},
        {"TestLevel5UpDownCounter4Bit", []() -> QObject * { return new TestLevel5UpDownCounter4Bit; }},
        {"TestLevel6ALU8Bit", []() -> QObject * { return new TestLevel6ALU8Bit; }},
        {"TestLevel6ProgramCounter8BitArithmetic", []() -> QObject * { return new TestLevel6ProgramCounter8BitArithmetic; }},
        {"TestLevel6RAM256X8", []() -> QObject * { return new TestLevel6RAM256X8; }},
        {"TestLevel6Register8Bit", []() -> QObject * { return new TestLevel6Register8Bit; }},
        {"TestLevel6RegisterFile8X8", []() -> QObject * { return new TestLevel6RegisterFile8X8; }},
        {"TestLevel6RippleAdder8Bit", []() -> QObject * { return new TestLevel6RippleAdder8Bit; }},
        {"TestLevel6StackPointer8Bit", []() -> QObject * { return new TestLevel6StackPointer8Bit; }},
        {"TestLevel7ALU16Bit", []() -> QObject * { return new TestLevel7ALU16Bit; }},
        {"TestLevel7CPUProgramCounter8Bit", []() -> QObject * { return new TestLevel7CPUProgramCounter8Bit; }},
        {"TestLevel7DataForwardingUnit", []() -> QObject * { return new TestLevel7DataForwardingUnit; }},
        {"TestLevel7ExecutionDatapath", []() -> QObject * { return new TestLevel7ExecutionDatapath; }},
        {"TestLevel7FlagRegister", []() -> QObject * { return new TestLevel7FlagRegister; }},
        {"TestLevel7InstructionDecoder8Bit", []() -> QObject * { return new TestLevel7InstructionDecoder8Bit; }},
        {"TestLevel7InstructionMemoryInterface", []() -> QObject * { return new TestLevel7InstructionMemoryInterface; }},
        {"TestLevel7InstructionRegister8Bit", []() -> QObject * { return new TestLevel7InstructionRegister8Bit; }},
        {"TestLevel8DecodeStage", []() -> QObject * { return new TestLevel8DecodeStage; }},
        {"TestLevel8ExecuteStage", []() -> QObject * { return new TestLevel8ExecuteStage; }},
        {"TestLevel8FetchStage", []() -> QObject * { return new TestLevel8FetchStage; }},
        {"TestLevel8MemoryStage", []() -> QObject * { return new TestLevel8MemoryStage; }},
        {"TestLevel9CPU16BitRISC", []() -> QObject * { return new TestLevel9CPU16BitRISC; }},
        {"TestLevel9FetchStage16Bit", []() -> QObject * { return new TestLevel9FetchStage16Bit; }},
        {"TestLevel9MultiCycleCPU8Bit", []() -> QObject * { return new TestLevel9MultiCycleCPU8Bit; }},
        {"TestLevel9SingleCycleCPU8Bit", []() -> QObject * { return new TestLevel9SingleCycleCPU8Bit; }},
        {"TestDecoder8To256", []() -> QObject * { return new TestDecoder8To256; }},
        {"TestMemorySettlingTime", []() -> QObject * { return new TestMemorySettlingTime; }},
        {"TestRAMCell1Bit", []() -> QObject * { return new TestRAMCell1Bit; }},
        {"TestSequential", []() -> QObject * { return new TestSequential; }},
        {"TestMUXDEMUXComprehensive", []() -> QObject * { return new TestMUXDEMUXComprehensive; }},
        {"TestArduino", []() -> QObject * { return new TestArduino; }},
        {"TestFeedback", []() -> QObject * { return new TestFeedback; }},
        {"TestFiles", []() -> QObject * { return new TestFiles; }},
        {"TestIC", []() -> QObject * { return new TestIC; }},
        {"TestSimulation", []() -> QObject * { return new TestSimulation; }},
        {"TestWorkspace", []() -> QObject * { return new TestWorkspace; }},
        {"TestWorkspaceFileops", []() -> QObject * { return new TestWorkspaceFileops; }},
        {"TestIcons", []() -> QObject * { return new TestIcons; }},
        {"TestWaveform", []() -> QObject * { return new TestWaveform; }},
        {"TestCommands", []() -> QObject * { return new TestCommands; }},
        {"TestCommon", []() -> QObject * { return new TestCommon; }},
        {"TestEnums", []() -> QObject * { return new TestEnums; }},
        {"TestRecentFiles", []() -> QObject * { return new TestRecentFiles; }},
        {"TestSettings", []() -> QObject * { return new TestSettings; }},
        {"TestThemeManager", []() -> QObject * { return new TestThemeManager; }},
        {"TestAudioBox", []() -> QObject * { return new TestAudioBox; }},
        {"TestBuzzer", []() -> QObject * { return new TestBuzzer; }},
        {"TestClock", []() -> QObject * { return new TestClock; }},
        {"TestClocksAdvanced", []() -> QObject * { return new TestClocksAdvanced; }},
        {"TestComponents", []() -> QObject * { return new TestComponents; }},
        {"TestDisplays", []() -> QObject * { return new TestDisplays; }},
        {"TestElementLabel", []() -> QObject * { return new TestElementLabel; }},
        {"TestElementProperties", []() -> QObject * { return new TestElementProperties; }},
        {"TestFeatures", []() -> QObject * { return new TestFeatures; }},
        {"TestGeometry", []() -> QObject * { return new TestGeometry; }},
        {"TestGraphicelementAdvanced", []() -> QObject * { return new TestGraphicelementAdvanced; }},
        {"TestInputElements", []() -> QObject * { return new TestInputElements; }},
        {"TestInputRotary", []() -> QObject * { return new TestInputRotary; }},
        {"TestLED", []() -> QObject * { return new TestLED; }},
        {"TestLogicGates", []() -> QObject * { return new TestLogicGates; }},
        {"TestMultiplexing", []() -> QObject * { return new TestMultiplexing; }},
        {"TestSequentialLogic", []() -> QObject * { return new TestSequentialLogic; }},
        {"TestTruthTable", []() -> QObject * { return new TestTruthTable; }},
        {"TestElementFactory", []() -> QObject * { return new TestElementFactory; }},
        {"TestLogicElements", []() -> QObject * { return new TestLogicElements; }},
        {"TestLogicElementsErrors", []() -> QObject * { return new TestLogicElementsErrors; }},
        {"TestLogicNode", []() -> QObject * { return new TestLogicNode; }},
        {"TestConnectionSerialization", []() -> QObject * { return new TestConnectionSerialization; }},
        {"TestConnections", []() -> QObject * { return new TestConnections; }},
        {"TestConnectionValidity", []() -> QObject * { return new TestConnectionValidity; }},
        {"TestScene", []() -> QObject * { return new TestScene; }},
        {"TestSceneConnections", []() -> QObject * { return new TestSceneConnections; }},
        {"TestSceneState", []() -> QObject * { return new TestSceneState; }},
        {"TestSceneUndoredo", []() -> QObject * { return new TestSceneUndoredo; }},
        {"TestSerialization", []() -> QObject * { return new TestSerialization; }},
        {"TestElementMapping", []() -> QObject * { return new TestElementMapping; }},
        {"TestSimulationBlocker", []() -> QObject * { return new TestSimulationBlocker; }},
        {"TestDialogs", []() -> QObject * { return new TestDialogs; }},
    });
}
