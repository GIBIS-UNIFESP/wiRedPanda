// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/Runners/QuickRunnerUtils.h"

#include "Tests/QuickShell/IC/TestCPUAlu.h"
#include "Tests/QuickShell/IC/TestCPUBranch.h"
#include "Tests/QuickShell/IC/TestCPUControlUnit.h"
#include "Tests/QuickShell/IC/TestCPUDecoders.h"
#include "Tests/QuickShell/IC/TestCPUInstructionExecute.h"
#include "Tests/QuickShell/IC/TestCPUInstructionFetch.h"
#include "Tests/QuickShell/IC/TestCPUIntegration.h"
#include "Tests/QuickShell/IC/TestCPUMemoryInterface.h"
#include "Tests/QuickShell/IC/TestCPUProgramCounter.h"
#include "Tests/QuickShell/IC/TestCPURegisterBank.h"
#include "Tests/QuickShell/IC/TestCPURegisters.h"
#include "Tests/QuickShell/IC/TestDecoder8to256.h"
#include "Tests/QuickShell/IC/TestICFixtureLayout.h"
#include "Tests/QuickShell/IC/TestLevel1DFlipFlop.h"
#include "Tests/QuickShell/IC/TestLevel1DLatch.h"
#include "Tests/QuickShell/IC/TestLevel1JkFlipFlop.h"
#include "Tests/QuickShell/IC/TestLevel1SrLatch.h"
#include "Tests/QuickShell/IC/TestLevel2Decoder2to4.h"
#include "Tests/QuickShell/IC/TestLevel2Decoder3to8.h"
#include "Tests/QuickShell/IC/TestLevel2Decoder4to16.h"
#include "Tests/QuickShell/IC/TestLevel2Decoder5to32.h"
#include "Tests/QuickShell/IC/TestLevel2FullAdder1bit.h"
#include "Tests/QuickShell/IC/TestLevel2HalfAdder.h"
#include "Tests/QuickShell/IC/TestLevel2Mux2to1.h"
#include "Tests/QuickShell/IC/TestLevel2Mux4to1.h"
#include "Tests/QuickShell/IC/TestLevel2Mux8to1.h"
#include "Tests/QuickShell/IC/TestLevel2ParityChecker.h"
#include "Tests/QuickShell/IC/TestLevel2ParityGenerator.h"
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
#include "Tests/QuickShell/IC/TestLevel5BarrelRotator.h"
#include "Tests/QuickShell/IC/TestLevel5BarrelShifter4bit.h"
#include "Tests/QuickShell/IC/TestLevel5ClockGatedDecoder.h"
#include "Tests/QuickShell/IC/TestLevel5Controller4bit.h"
#include "Tests/QuickShell/IC/TestLevel5InstructionDecoder4bit.h"
#include "Tests/QuickShell/IC/TestLevel5LoadableCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel5ModuloCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel5ProgramCounter4bit.h"
#include "Tests/QuickShell/IC/TestLevel5RegisterFile4x4.h"
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
#include "Tests/QuickShell/IC/TestMemorySettlingTime.h"
#include "Tests/QuickShell/IC/TestRamCell1bit.h"
#include "Tests/QuickShell/IC/TestSequential.h"
#include "Tests/QuickShell/TestArduino.h"
#include "Tests/QuickShell/TestArduinoCodeGenUnit.h"
#include "Tests/QuickShell/TestAudioBox.h"
#include "Tests/QuickShell/TestBuzzer.h"
#include "Tests/QuickShell/TestCanvasCommands.h"
#include "Tests/QuickShell/TestCanvasEmbeddedIC.h"
#include "Tests/QuickShell/TestCanvasInlineIC.h"
#include "Tests/QuickShell/TestCanvasItemInteraction.h"
#include "Tests/QuickShell/TestCanvasItemSmoke.h"
#include "Tests/QuickShell/TestClock.h"
#include "Tests/QuickShell/TestClocksAdvanced.h"
#include "Tests/QuickShell/TestCodeGenUtils.h"
#include "Tests/QuickShell/TestCommon.h"
#include "Tests/QuickShell/TestComponents.h"
#include "Tests/QuickShell/TestConnection.h"
#include "Tests/QuickShell/TestConnectionSerialization.h"
#include "Tests/QuickShell/TestConnectionValidity.h"
#include "Tests/QuickShell/TestConnections.h"
#include "Tests/QuickShell/TestDanglingPointer.h"
#include "Tests/QuickShell/TestDemux.h"
#include "Tests/QuickShell/TestDisplay.h"
#include "Tests/QuickShell/TestDisplay7.h"
#include "Tests/QuickShell/TestDisplays.h"
#include "Tests/QuickShell/TestDragDropPayload.h"
#include "Tests/QuickShell/TestElementAppearance.h"
#include "Tests/QuickShell/TestElementFactory.h"
#include "Tests/QuickShell/TestElementLogic.h"
#include "Tests/QuickShell/TestElementLogicErrors.h"
#include "Tests/QuickShell/TestElementProperties.h"
#include "Tests/QuickShell/TestEnums.h"
#include "Tests/QuickShell/TestExerciseTourResources.h"
#include "Tests/QuickShell/TestExternalFilePath.h"
#include "Tests/QuickShell/TestFeatures.h"
#include "Tests/QuickShell/TestFeedback.h"
#include "Tests/QuickShell/TestFiles.h"
#include "Tests/QuickShell/TestGeometry.h"
#include "Tests/QuickShell/TestGraphicElement.h"
#include "Tests/QuickShell/TestGraphicElementSerializer.h"
#include "Tests/QuickShell/TestGraphicelementAdvanced.h"
#include "Tests/QuickShell/TestICInline.h"
#include "Tests/QuickShell/TestICUnit.h"
#include "Tests/QuickShell/TestIcons.h"
#include "Tests/QuickShell/TestInstallRelativePaths.h"
#include "Tests/QuickShell/TestInputElements.h"
#include "Tests/QuickShell/TestInputRotary.h"
#include "Tests/QuickShell/TestLanguageManager.h"
#include "Tests/QuickShell/TestLogicGates.h"
#include "Tests/QuickShell/TestMultiplexing.h"
#include "Tests/QuickShell/TestMux.h"
#include "Tests/QuickShell/TestMuxDemuxComprehensive.h"
#include "Tests/QuickShell/TestNodeLogic.h"
#include "Tests/QuickShell/TestNotifyCatch.h"
#include "Tests/QuickShell/TestPort.h"
#include "Tests/QuickShell/TestPriorities.h"
#include "Tests/QuickShell/TestQuickAppController.h"
#include "Tests/QuickShell/TestQuickBaseHandler.h"
#include "Tests/QuickShell/TestQuickCanvasZoom.h"
#include "Tests/QuickShell/TestQuickConnectionHandler.h"
#include "Tests/QuickShell/TestQuickDialogProvider.h"
#include "Tests/QuickShell/TestQuickDolphinController.h"
#include "Tests/QuickShell/TestQuickElementEditor.h"
#include "Tests/QuickShell/TestQuickElementHandler.h"
#include "Tests/QuickShell/TestQuickElementPalette.h"
#include "Tests/QuickShell/TestQuickExerciseController.h"
#include "Tests/QuickShell/TestQuickFileHandlerSecurity.h"
#include "Tests/QuickShell/TestQuickHistoryHandler.h"
#include "Tests/QuickShell/TestQuickICHandler.h"
#include "Tests/QuickShell/TestQuickICHandlerSecurity.h"
#include "Tests/QuickShell/TestQuickICPreview.h"
#include "Tests/QuickShell/TestQuickMinimap.h"
#include "Tests/QuickShell/TestQuickSimulationHandler.h"
#include "Tests/QuickShell/TestQuickTourController.h"
#include "Tests/QuickShell/TestQuickUpdateController.h"
#include "Tests/QuickShell/TestQuickWorkSpace.h"
#include "Tests/QuickShell/TestSceneConnections.h"
#include "Tests/QuickShell/TestSelectionCapabilities.h"
#include "Tests/QuickShell/TestSerialization.h"
#include "Tests/QuickShell/TestSimulation.h"
#include "Tests/QuickShell/TestSimulationBlocker.h"
#include "Tests/QuickShell/TestSimulationUnit.h"
#include "Tests/QuickShell/TestStatusOps.h"
#include "Tests/QuickShell/TestSystemVerilogCodeGenUnit.h"
#include "Tests/QuickShell/TestSystemVerilogExport.h"
#include "Tests/QuickShell/TestTextureAtlas.h"
#include "Tests/QuickShell/TestThemeHandler.h"
#include "Tests/QuickShell/TestTourEngine.h"
#include "Tests/QuickShell/TestTruthTable.h"
#include "Tests/QuickShell/TestWirelessNode.h"

int main(int argc, char **argv)
{
    return runQuickTestSuite(argc, argv, {
        {"TestArduino", []() -> QObject * { return new TestArduino; }},
        {"TestArduinoCodeGenUnit", []() -> QObject * { return new TestArduinoCodeGenUnit; }},
        {"TestAudioBox", []() -> QObject * { return new TestAudioBox; }},
        {"TestBuzzer", []() -> QObject * { return new TestBuzzer; }},
        {"TestCanvasCommands", []() -> QObject * { return new TestCanvasCommands; }},
        {"TestCanvasEmbeddedIC", []() -> QObject * { return new TestCanvasEmbeddedIC; }},
        {"TestCanvasInlineIC", []() -> QObject * { return new TestCanvasInlineIC; }},
        {"TestCanvasItemInteraction", []() -> QObject * { return new TestCanvasItemInteraction; }},
        {"TestCanvasItemSmoke", []() -> QObject * { return new TestCanvasItemSmoke; }},
        {"TestClock", []() -> QObject * { return new TestClock; }},
        {"TestClocksAdvanced", []() -> QObject * { return new TestClocksAdvanced; }},
        {"TestCodeGenUtils", []() -> QObject * { return new TestCodeGenUtils; }},
        {"TestCommon", []() -> QObject * { return new TestCommon; }},
        {"TestComponents", []() -> QObject * { return new TestComponents; }},
        {"TestConnection", []() -> QObject * { return new TestConnection; }},
        {"TestConnectionSerialization", []() -> QObject * { return new TestConnectionSerialization; }},
        {"TestConnectionValidity", []() -> QObject * { return new TestConnectionValidity; }},
        {"TestConnections", []() -> QObject * { return new TestConnections; }},
        {"TestDanglingPointer", []() -> QObject * { return new TestDanglingPointer; }},
        {"TestDemux", []() -> QObject * { return new TestDemux; }},
        {"TestDisplay", []() -> QObject * { return new TestDisplay; }},
        {"TestDisplay7", []() -> QObject * { return new TestDisplay7; }},
        {"TestDisplays", []() -> QObject * { return new TestDisplays; }},
        {"TestDragDropPayload", []() -> QObject * { return new TestDragDropPayload; }},
        {"TestElementAppearance", []() -> QObject * { return new TestElementAppearance; }},
        {"TestElementFactory", []() -> QObject * { return new TestElementFactory; }},
        {"TestElementLogic", []() -> QObject * { return new TestElementLogic; }},
        {"TestElementLogicErrors", []() -> QObject * { return new TestElementLogicErrors; }},
        {"TestElementProperties", []() -> QObject * { return new TestElementProperties; }},
        {"TestEnums", []() -> QObject * { return new TestEnums; }},
        {"TestExerciseTourResources", []() -> QObject * { return new TestExerciseTourResources; }},
        {"TestExternalFilePath", []() -> QObject * { return new TestExternalFilePath; }},
        {"TestFeatures", []() -> QObject * { return new TestFeatures; }},
        {"TestFeedback", []() -> QObject * { return new TestFeedback; }},
        {"TestFiles", []() -> QObject * { return new TestFiles; }},
        {"TestGeometry", []() -> QObject * { return new TestGeometry; }},
        {"TestGraphicElement", []() -> QObject * { return new TestGraphicElement; }},
        {"TestGraphicElementSerializer", []() -> QObject * { return new TestGraphicElementSerializer; }},
        {"TestGraphicelementAdvanced", []() -> QObject * { return new TestGraphicelementAdvanced; }},
        {"TestICInline", []() -> QObject * { return new TestICInline; }},
        {"TestICUnit", []() -> QObject * { return new TestICUnit; }},
        {"TestIcons", []() -> QObject * { return new TestIcons; }},
        {"TestInstallRelativePaths", []() -> QObject * { return new TestInstallRelativePaths; }},
        {"TestInputElements", []() -> QObject * { return new TestInputElements; }},
        {"TestInputRotary", []() -> QObject * { return new TestInputRotary; }},
        {"TestLanguageManager", []() -> QObject * { return new TestLanguageManager; }},
        {"TestLogicGates", []() -> QObject * { return new TestLogicGates; }},
        {"TestMultiplexing", []() -> QObject * { return new TestMultiplexing; }},
        {"TestMUXDEMUXComprehensive", []() -> QObject * { return new TestMUXDEMUXComprehensive; }},
        {"TestMux", []() -> QObject * { return new TestMux; }},
        {"TestNodeLogic", []() -> QObject * { return new TestNodeLogic; }},
        {"TestNotifyCatch", []() -> QObject * { return new TestNotifyCatch; }},
        {"TestSerialization", []() -> QObject * { return new TestSerialization; }},
        {"TestPort", []() -> QObject * { return new TestPort; }},
        {"TestPriorities", []() -> QObject * { return new TestPriorities; }},
        {"TestQuickAppController", []() -> QObject * { return new TestQuickAppController; }},
        {"TestQuickBaseHandler", []() -> QObject * { return new TestQuickBaseHandler; }},
        {"TestQuickCanvasZoom", []() -> QObject * { return new TestQuickCanvasZoom; }},
        {"TestQuickConnectionHandler", []() -> QObject * { return new TestQuickConnectionHandler; }},
        {"TestQuickDialogProvider", []() -> QObject * { return new TestQuickDialogProvider; }},
        {"TestQuickDolphinController", []() -> QObject * { return new TestQuickDolphinController; }},
        {"TestQuickElementEditor", []() -> QObject * { return new TestQuickElementEditor; }},
        {"TestQuickElementHandler", []() -> QObject * { return new TestQuickElementHandler; }},
        {"TestQuickElementPalette", []() -> QObject * { return new TestQuickElementPalette; }},
        {"TestQuickExerciseController", []() -> QObject * { return new TestQuickExerciseController; }},
        {"TestQuickFileHandlerSecurity", []() -> QObject * { return new TestQuickFileHandlerSecurity; }},
        {"TestQuickHistoryHandler", []() -> QObject * { return new TestQuickHistoryHandler; }},
        {"TestQuickICHandler", []() -> QObject * { return new TestQuickICHandler; }},
        {"TestQuickICHandlerSecurity", []() -> QObject * { return new TestQuickICHandlerSecurity; }},
        {"TestQuickICPreview", []() -> QObject * { return new TestQuickICPreview; }},
        {"TestQuickMinimap", []() -> QObject * { return new TestQuickMinimap; }},
        {"TestQuickSimulationHandler", []() -> QObject * { return new TestQuickSimulationHandler; }},
        {"TestQuickTourController", []() -> QObject * { return new TestQuickTourController; }},
        {"TestQuickUpdateController", []() -> QObject * { return new TestQuickUpdateController; }},
        {"TestQuickWorkSpace", []() -> QObject * { return new TestQuickWorkSpace; }},
        {"TestSceneConnections", []() -> QObject * { return new TestSceneConnections; }},
        {"TestSelectionCapabilities", []() -> QObject * { return new TestSelectionCapabilities; }},
        {"TestSimulation", []() -> QObject * { return new TestSimulation; }},
        {"TestSimulationBlocker", []() -> QObject * { return new TestSimulationBlocker; }},
        {"TestSimulationUnit", []() -> QObject * { return new TestSimulationUnit; }},
        {"TestStatusOps", []() -> QObject * { return new TestStatusOps; }},
        {"TestSystemVerilogCodeGenUnit", []() -> QObject * { return new TestSystemVerilogCodeGenUnit; }},
        {"TestSystemVerilogExport", []() -> QObject * { return new TestSystemVerilogExport; }},
        {"TestTextureAtlas", []() -> QObject * { return new TestTextureAtlas; }},
        {"TestThemeHandler", []() -> QObject * { return new TestThemeHandler; }},
        {"TestTourEngine", []() -> QObject * { return new TestTourEngine; }},
        {"TestTruthTable", []() -> QObject * { return new TestTruthTable; }},
        {"TestWirelessNode", []() -> QObject * { return new TestWirelessNode; }},
        {"TestCPUProgramCounter", []() -> QObject * { return new TestCPUProgramCounter; }},
        {"TestCPUAlu", []() -> QObject * { return new TestCPUAlu; }},
        {"TestCPUBranch", []() -> QObject * { return new TestCPUBranch; }},
        {"TestCPUControlUnit", []() -> QObject * { return new TestCPUControlUnit; }},
        {"TestCPUDecoders", []() -> QObject * { return new TestCPUDecoders; }},
        {"TestCPUInstructionExecute", []() -> QObject * { return new TestCPUInstructionExecute; }},
        {"TestCPUInstructionFetch", []() -> QObject * { return new TestCPUInstructionFetch; }},
        {"TestCPUMemoryInterface", []() -> QObject * { return new TestCPUMemoryInterface; }},
        {"TestCPURegisterBank", []() -> QObject * { return new TestCPURegisterBank; }},
        {"TestDecoder8To256", []() -> QObject * { return new TestDecoder8To256; }},
        {"TestCPURegisters", []() -> QObject * { return new TestCPURegisters; }},
        {"TestICFixtureLayout", []() -> QObject * { return new TestICFixtureLayout; }},
        {"TestCPUIntegration", []() -> QObject * { return new TestCPUIntegration; }},
        {"TestLevel1DFlipFlop", []() -> QObject * { return new TestLevel1DFlipFlop; }},
        {"TestLevel1DLatch", []() -> QObject * { return new TestLevel1DLatch; }},
        {"TestLevel1JKFlipFlop", []() -> QObject * { return new TestLevel1JKFlipFlop; }},
        {"TestLevel1SRLatch", []() -> QObject * { return new TestLevel1SRLatch; }},
        {"TestLevel2Decoder2To4", []() -> QObject * { return new TestLevel2Decoder2To4; }},
        {"TestLevel2Decoder3To8", []() -> QObject * { return new TestLevel2Decoder3To8; }},
        {"TestLevel2Decoder4To16", []() -> QObject * { return new TestLevel2Decoder4To16; }},
        {"TestLevel2Decoder5To32", []() -> QObject * { return new TestLevel2Decoder5To32; }},
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
        {"TestMemorySettlingTime", []() -> QObject * { return new TestMemorySettlingTime; }},
        {"TestRAMCell1Bit", []() -> QObject * { return new TestRAMCell1Bit; }},
        {"TestSequential", []() -> QObject * { return new TestSequential; }},
    });
}
