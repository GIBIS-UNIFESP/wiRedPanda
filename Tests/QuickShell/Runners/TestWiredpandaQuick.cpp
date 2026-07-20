// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/Runners/QuickRunnerUtils.h"

#include "Tests/QuickShell/TestCanvasCommands.h"
#include "Tests/QuickShell/TestCanvasEmbeddedIC.h"
#include "Tests/QuickShell/TestCanvasInlineIC.h"
#include "Tests/QuickShell/TestCanvasItemInteraction.h"
#include "Tests/QuickShell/TestCanvasPortHover.h"
#include "Tests/QuickShell/TestCanvasItemSmoke.h"
#include "Tests/QuickShell/IC/TestLevel1DFlipFlop.h"
#include "Tests/QuickShell/IC/TestCPUProgramCounter.h"
#include "Tests/QuickShell/IC/TestCPUAlu.h"
#include "Tests/QuickShell/IC/TestCPUBranch.h"
#include "Tests/QuickShell/IC/TestCPUControlUnit.h"
#include "Tests/QuickShell/IC/TestCPUDecoders.h"
#include "Tests/QuickShell/IC/TestCPUInstructionExecute.h"
#include "Tests/QuickShell/IC/TestCPUInstructionFetch.h"
#include "Tests/QuickShell/IC/TestCPUIntegration.h"
#include "Tests/QuickShell/IC/TestCPUMemoryInterface.h"
#include "Tests/QuickShell/IC/TestCPURegisterBank.h"
#include "Tests/QuickShell/IC/TestCPURegisters.h"
#include "Tests/QuickShell/IC/TestLevel1DLatch.h"
#include "Tests/QuickShell/IC/TestLevel1SrLatch.h"
#include "Tests/QuickShell/IC/TestLevel1JkFlipFlop.h"
#include "Tests/QuickShell/IC/TestLevel2HalfAdder.h"
#include "Tests/QuickShell/IC/TestLevel2FullAdder1bit.h"
#include "Tests/QuickShell/IC/TestLevel2ParityChecker.h"
#include "Tests/QuickShell/IC/TestLevel2ParityGenerator.h"
#include "Tests/QuickShell/IC/TestLevel2Decoder2to4.h"
#include "Tests/QuickShell/IC/TestLevel2Decoder3to8.h"
#include "Tests/QuickShell/IC/TestLevel2Decoder4to16.h"
#include "Tests/QuickShell/IC/TestLevel2Decoder5to32.h"
#include "Tests/QuickShell/IC/TestLevel2Mux2to1.h"
#include "Tests/QuickShell/IC/TestLevel2Mux4to1.h"
#include "Tests/QuickShell/IC/TestLevel2Mux8to1.h"
#include "Tests/QuickShell/IC/TestLevel2PriorityEncoder8to3.h"
#include "Tests/QuickShell/IC/TestLevel2PriorityMux3to1.h"
#include "Tests/QuickShell/IC/TestLevel3AluSelector5way.h"
#include "Tests/QuickShell/IC/TestLevel3Bcd7segmentDecoder.h"
#include "Tests/QuickShell/IC/TestLevel3Comparator4bit.h"
#include "Tests/QuickShell/IC/TestLevel3Comparator4bitEquality.h"
#include "Tests/QuickShell/IC/TestLevel3Register1bit.h"
#include "Tests/QuickShell/IC/TestLevel4BinaryCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel4BusMux4bit.h"
#include "Tests/QuickShell/IC/TestLevel4BusMux8bit.h"
#include "Tests/QuickShell/IC/TestLevel4Comparator4bit.h"
#include "Tests/QuickShell/IC/TestLevel4JohnsonCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel4Ram4x1.h"
#include "Tests/QuickShell/IC/TestLevel4Ram8x1.h"
#include "Tests/QuickShell/IC/TestLevel4Register4bit.h"
#include "Tests/QuickShell/IC/TestLevel4RingCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel4RippleAdder4bit.h"
#include "Tests/QuickShell/IC/TestLevel4RippleAlu4bit.h"
#include "Tests/QuickShell/IC/TestLevel4ShiftRegisterPiso.h"
#include "Tests/QuickShell/IC/TestLevel4ShiftRegisterSipo.h"
#include "Tests/QuickShell/IC/TestLevel5RegisterFile4x4.h"
#include "Tests/QuickShell/IC/TestLevel5BarrelRotator.h"
#include "Tests/QuickShell/IC/TestLevel5BarrelShifter4bit.h"
#include "Tests/QuickShell/IC/TestLevel5ClockGatedDecoder.h"
#include "Tests/QuickShell/IC/TestLevel5Controller4bit.h"
#include "Tests/QuickShell/IC/TestLevel5InstructionDecoder4bit.h"
#include "Tests/QuickShell/IC/TestLevel5LoadableCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel5ModuloCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel5ProgramCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel5RegisterFile8x8.h"
#include "Tests/QuickShell/IC/TestLevel5UpDownCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel6Alu8bit.h"
#include "Tests/QuickShell/IC/TestLevel6ProgramCounter8bitArithmetic.h"
#include "Tests/QuickShell/IC/TestLevel6Ram8x8.h"
#include "Tests/QuickShell/IC/TestLevel6Register8bit.h"
#include "Tests/QuickShell/IC/TestLevel6RegisterFile8x8.h"
#include "Tests/QuickShell/IC/TestLevel6RippleAdder8bit.h"
#include "Tests/QuickShell/IC/TestLevel6StackMemoryInterface.h"
#include "Tests/QuickShell/IC/TestLevel6StackPointer8bit.h"
#include "Tests/QuickShell/IC/TestLevel7Alu16bit.h"
#include "Tests/QuickShell/IC/TestLevel7CpuProgramCounter8bit.h"
#include "Tests/QuickShell/IC/TestLevel7DataForwardingUnit.h"
#include "Tests/QuickShell/IC/TestLevel7ExecutionDatapath.h"
#include "Tests/QuickShell/IC/TestLevel7FlagRegister.h"
#include "Tests/QuickShell/IC/TestLevel7InstructionDecoder8bit.h"
#include "Tests/QuickShell/IC/TestLevel7InstructionMemoryInterface.h"
#include "Tests/QuickShell/IC/TestLevel7InstructionRegister8bit.h"
#include "Tests/QuickShell/IC/TestLevel8DecodeStage.h"
#include "Tests/QuickShell/IC/TestLevel8ExecuteStage.h"
#include "Tests/QuickShell/IC/TestLevel8FetchStage.h"
#include "Tests/QuickShell/IC/TestLevel8MemoryStage.h"
#include "Tests/QuickShell/IC/TestLevel9Cpu16bitRisc.h"
#include "Tests/QuickShell/IC/TestLevel9FetchStage16bit.h"
#include "Tests/QuickShell/IC/TestLevel9MemoryStage16bit.h"
#include "Tests/QuickShell/IC/TestLevel9MultiCycleCpu8bit.h"
#include "Tests/QuickShell/IC/TestLevel9RegisterFile32x16.h"
#include "Tests/QuickShell/IC/TestLevel9SingleCycleCpu8bit.h"
#include "Tests/QuickShell/TestFeedback.h"
#include "Tests/QuickShell/TestSimulation.h"
#include "Tests/QuickShell/TestQuickWorkSpace.h"
#include "Tests/QuickShell/TestConnection.h"
#include "Tests/QuickShell/TestConnectionValidity.h"
#include "Tests/QuickShell/TestPort.h"
#include "Tests/QuickShell/TestSceneConnections.h"
#include "Tests/QuickShell/TestSelectionCapabilities.h"
#include "Tests/QuickShell/TestQuickAppController.h"
#include "Tests/QuickShell/TestQuickCanvasZoom.h"
#include "Tests/QuickShell/TestQuickDolphinController.h"
#include "Tests/QuickShell/TestQuickElementEditor.h"
#include "Tests/QuickShell/TestQuickElementHandler.h"
#include "Tests/QuickShell/TestQuickElementPalette.h"
#include "Tests/QuickShell/TestQuickMinimap.h"
#include "Tests/QuickShell/TestQuickFileHandlerSecurity.h"
#include "Tests/QuickShell/TestQuickICHandlerSecurity.h"
#include "Tests/QuickShell/TestNotifyCatch.h"
#include "Tests/QuickShell/TestSimulationUnit.h"
#include "Tests/QuickShell/TestSimulationBlocker.h"
#include "Tests/QuickShell/TestDanglingPointer.h"
#include "Tests/QuickShell/TestICUnit.h"
#include "Tests/QuickShell/TestICRegistry.h"
#include "Tests/QuickShell/TestExerciseEngine.h"
#include "Tests/QuickShell/TestDemux.h"
#include "Tests/QuickShell/TestMux.h"
#include "Tests/QuickShell/TestPriorities.h"

int main(int argc, char **argv)
{
    return runQuickTestSuite(argc, argv, {
        {"TestCanvasItemSmoke", []() -> QObject * { return new TestCanvasItemSmoke; }},
        {"TestCanvasCommands", []() -> QObject * { return new TestCanvasCommands; }},
        {"TestQuickElementHandler", []() -> QObject * { return new TestQuickElementHandler; }},
        {"TestQuickFileHandlerSecurity", []() -> QObject * { return new TestQuickFileHandlerSecurity; }},
        {"TestQuickICHandlerSecurity", []() -> QObject * { return new TestQuickICHandlerSecurity; }},
        {"TestQuickDolphinController", []() -> QObject * { return new TestQuickDolphinController; }},
        {"TestQuickAppController", []() -> QObject * { return new TestQuickAppController; }},
        {"TestCanvasEmbeddedIC", []() -> QObject * { return new TestCanvasEmbeddedIC; }},
        {"TestCanvasInlineIC", []() -> QObject * { return new TestCanvasInlineIC; }},
        {"TestCanvasItemInteraction", []() -> QObject * { return new TestCanvasItemInteraction; }},
        {"TestCanvasPortHover", []() -> QObject * { return new TestCanvasPortHover; }},
        {"TestQuickElementEditor", []() -> QObject * { return new TestQuickElementEditor; }},
        {"TestQuickElementPalette", []() -> QObject * { return new TestQuickElementPalette; }},
        {"TestQuickCanvasZoom", []() -> QObject * { return new TestQuickCanvasZoom; }},
        {"TestQuickMinimap", []() -> QObject * { return new TestQuickMinimap; }},
        {"TestConnection", []() -> QObject * { return new TestConnection; }},
        {"TestConnectionValidity", []() -> QObject * { return new TestConnectionValidity; }},
        {"TestPort", []() -> QObject * { return new TestPort; }},
        {"TestSelectionCapabilities", []() -> QObject * { return new TestSelectionCapabilities; }},
        {"TestSceneConnections", []() -> QObject * { return new TestSceneConnections; }},
        {"TestFeedback", []() -> QObject * { return new TestFeedback; }},
        {"TestSimulation", []() -> QObject * { return new TestSimulation; }},
        {"TestQuickWorkSpace", []() -> QObject * { return new TestQuickWorkSpace; }},
        {"TestLevel1DFlipFlop", []() -> QObject * { return new TestLevel1DFlipFlop; }},
        {"TestCPUProgramCounter", []() -> QObject * { return new TestCPUProgramCounter; }},
        {"TestCPUAlu", []() -> QObject * { return new TestCPUAlu; }},
        {"TestCPUBranch", []() -> QObject * { return new TestCPUBranch; }},
        {"TestCPUControlUnit", []() -> QObject * { return new TestCPUControlUnit; }},
        {"TestCPUDecoders", []() -> QObject * { return new TestCPUDecoders; }},
        {"TestCPUInstructionExecute", []() -> QObject * { return new TestCPUInstructionExecute; }},
        {"TestCPUInstructionFetch", []() -> QObject * { return new TestCPUInstructionFetch; }},
        {"TestCPUIntegration", []() -> QObject * { return new TestCPUIntegration; }},
        {"TestCPUMemoryInterface", []() -> QObject * { return new TestCPUMemoryInterface; }},
        {"TestCPURegisterBank", []() -> QObject * { return new TestCPURegisterBank; }},
        {"TestCPURegisters", []() -> QObject * { return new TestCPURegisters; }},
        {"TestLevel1DLatch", []() -> QObject * { return new TestLevel1DLatch; }},
        {"TestLevel1SRLatch", []() -> QObject * { return new TestLevel1SRLatch; }},
        {"TestLevel1JKFlipFlop", []() -> QObject * { return new TestLevel1JKFlipFlop; }},
        {"TestLevel2HalfAdder", []() -> QObject * { return new TestLevel2HalfAdder; }},
        {"TestLevel2FullAdder1Bit", []() -> QObject * { return new TestLevel2FullAdder1Bit; }},
        {"TestLevel2ParityChecker", []() -> QObject * { return new TestLevel2ParityChecker; }},
        {"TestLevel2ParityGenerator", []() -> QObject * { return new TestLevel2ParityGenerator; }},
        {"TestLevel2Decoder2To4", []() -> QObject * { return new TestLevel2Decoder2To4; }},
        {"TestLevel2Decoder3To8", []() -> QObject * { return new TestLevel2Decoder3To8; }},
        {"TestLevel2Decoder4To16", []() -> QObject * { return new TestLevel2Decoder4To16; }},
        {"TestLevel2Decoder5To32", []() -> QObject * { return new TestLevel2Decoder5To32; }},
        {"TestLevel2MUX2To1", []() -> QObject * { return new TestLevel2MUX2To1; }},
        {"TestLevel2MUX4To1", []() -> QObject * { return new TestLevel2MUX4To1; }},
        {"TestLevel2MUX8To1", []() -> QObject * { return new TestLevel2MUX8To1; }},
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
        {"TestLevel5RegisterFile4X4", []() -> QObject * { return new TestLevel5RegisterFile4X4; }},
        {"TestLevel5BarrelRotator", []() -> QObject * { return new TestLevel5BarrelRotator; }},
        {"TestLevel5BarrelShifter4Bit", []() -> QObject * { return new TestLevel5BarrelShifter4Bit; }},
        {"TestLevel5ClockGatedDecoder", []() -> QObject * { return new TestLevel5ClockGatedDecoder; }},
        {"TestLevel5Controller4Bit", []() -> QObject * { return new TestLevel5Controller4Bit; }},
        {"TestLevel5InstructionDecoder4Bit", []() -> QObject * { return new TestLevel5InstructionDecoder4Bit; }},
        {"TestLevel5LoadableCounter4Bit", []() -> QObject * { return new TestLevel5LoadableCounter4Bit; }},
        {"TestLevel5ModuloCounter4Bit", []() -> QObject * { return new TestLevel5ModuloCounter4Bit; }},
        {"TestLevel5ProgramCounter4Bit", []() -> QObject * { return new TestLevel5ProgramCounter4Bit; }},
        {"TestLevel5RegisterFile8X8", []() -> QObject * { return new TestLevel5RegisterFile8X8; }},
        {"TestLevel5UpDownCounter4Bit", []() -> QObject * { return new TestLevel5UpDownCounter4Bit; }},
        {"TestLevel6ALU8Bit", []() -> QObject * { return new TestLevel6ALU8Bit; }},
        {"TestLevel6ProgramCounter8BitArithmetic", []() -> QObject * { return new TestLevel6ProgramCounter8BitArithmetic; }},
        {"TestLevel6RAM8X8", []() -> QObject * { return new TestLevel6RAM8X8; }},
        {"TestLevel6Register8Bit", []() -> QObject * { return new TestLevel6Register8Bit; }},
        {"TestLevel6RegisterFile8X8", []() -> QObject * { return new TestLevel6RegisterFile8X8; }},
        {"TestLevel6RippleAdder8Bit", []() -> QObject * { return new TestLevel6RippleAdder8Bit; }},
        {"TestLevel6StackMemoryInterface", []() -> QObject * { return new TestLevel6StackMemoryInterface; }},
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
        {"TestLevel9MemoryStage16Bit", []() -> QObject * { return new TestLevel9MemoryStage16Bit; }},
        {"TestLevel9MultiCycleCPU8Bit", []() -> QObject * { return new TestLevel9MultiCycleCPU8Bit; }},
        {"TestLevel9RegisterFile32X16", []() -> QObject * { return new TestLevel9RegisterFile32X16; }},
        {"TestLevel9SingleCycleCPU8Bit", []() -> QObject * { return new TestLevel9SingleCycleCPU8Bit; }},
        {"TestNotifyCatch", []() -> QObject * { return new TestNotifyCatch; }},
        {"TestSimulationUnit", []() -> QObject * { return new TestSimulationUnit; }},
        {"TestSimulationBlocker", []() -> QObject * { return new TestSimulationBlocker; }},
        {"TestDanglingPointer", []() -> QObject * { return new TestDanglingPointer; }},
        {"TestICUnit", []() -> QObject * { return new TestICUnit; }},
        {"TestICRegistry", []() -> QObject * { return new TestICRegistry; }},
        {"TestExerciseEngine", []() -> QObject * { return new TestExerciseEngine; }},
        {"TestDemux", []() -> QObject * { return new TestDemux; }},
        {"TestMux", []() -> QObject * { return new TestMux; }},
        {"TestPriorities", []() -> QObject * { return new TestPriorities; }},
    });
}
