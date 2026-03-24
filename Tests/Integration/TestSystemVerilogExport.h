// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTest>

class TestSystemVerilogExport : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Wireless node codegen
    void testWirelessNodeGeneration();
    void testWirelessOrphanedRxCodegen();

    // Level 2 - Combinational circuits (all pure gates, no flip-flops)
    void testSystemVerilogExportHalfAdder();
    void testSystemVerilogExportFullAdder1Bit();
    void testSystemVerilogExportDecoder2to4();
    void testSystemVerilogExportMux2to1();
    void testSystemVerilogExportDecoder3to8();
    void testSystemVerilogExportParityChecker();
    void testSystemVerilogExportDecoder4to16();
    void testSystemVerilogExportMux4to1();
    void testSystemVerilogExportMux8to1();
    void testSystemVerilogExportParityGenerator();
    void testSystemVerilogExportPriorityEncoder8to3();
    void testSystemVerilogExportPriorityMux3to1();

    // Level 1 - Sequential circuits (flip-flops / latches)
    void testSystemVerilogExportDFlipFlop();
    void testSystemVerilogExportDLatch();
    void testSystemVerilogExportJKFlipFlop();
    void testSystemVerilogExportSRLatch();

    // Level 3 - Medium combinational
    void testSystemVerilogExportAluSelector5Way();
    void testSystemVerilogExportBcd7SegmentDecoder();
    void testSystemVerilogExportComparator4Bit();
    void testSystemVerilogExportComparator4BitEquality();
    void testSystemVerilogExportRegister1Bit();

    // Level 4 - 4-bit datapaths (ICs nested inside ICs)
    void testSystemVerilogExportRippleAdder4Bit();
    void testSystemVerilogExportRegister4Bit();
    void testSystemVerilogExportBinaryCounter4Bit();
    void testSystemVerilogExportBusMux4Bit();
    void testSystemVerilogExportBusMux8Bit();
    void testSystemVerilogExportComparator4BitLevel4();
    void testSystemVerilogExportJohnsonCounter4Bit();
    void testSystemVerilogExportRam4x1();
    void testSystemVerilogExportRam8x1();
    void testSystemVerilogExportRingCounter4Bit();
    void testSystemVerilogExportRippleAlu4Bit();
    void testSystemVerilogExportShiftRegisterPiso();
    void testSystemVerilogExportShiftRegisterSipo();

    // Level 5 - Advanced 4-bit
    void testSystemVerilogExportBarrelRotator();
    void testSystemVerilogExportBarrelShifter4Bit();
    void testSystemVerilogExportClockGatedDecoder();
    void testSystemVerilogExportController4Bit();
    void testSystemVerilogExportInstructionDecoder4Bit();
    void testSystemVerilogExportLoadableCounter4Bit();
    void testSystemVerilogExportModuloCounter4Bit();
    void testSystemVerilogExportProgramCounter4Bit();
    void testSystemVerilogExportRegisterFile4x4();
    void testSystemVerilogExportRegisterFile8x8Level5();
    void testSystemVerilogExportStackMemoryInterface();
    void testSystemVerilogExportUpDownCounter4Bit();

    // Level 6 - 8-bit subsystems (deeper nesting)
    void testSystemVerilogExportRippleAdder8Bit();
    void testSystemVerilogExportALU8Bit();
    void testSystemVerilogExportProgramCounter8BitArithmetic();
    void testSystemVerilogExportRam256x8();
    void testSystemVerilogExportRegister8Bit();
    void testSystemVerilogExportRegisterFile8x8Level6();
    void testSystemVerilogExportStackPointer8Bit();

    // Level 7 - 16-bit and CPU subsystems
    void testSystemVerilogExportAlu16Bit();
    void testSystemVerilogExportCpuProgramCounter8Bit();
    void testSystemVerilogExportDataForwardingUnit();
    void testSystemVerilogExportExecutionDatapath();
    void testSystemVerilogExportFlagRegister();
    void testSystemVerilogExportInstructionDecoder8Bit();
    void testSystemVerilogExportInstructionMemoryInterface();
    void testSystemVerilogExportInstructionRegister8Bit();

    // Level 8 - CPU pipeline stages
    void testSystemVerilogExportDecodeStage();
    void testSystemVerilogExportExecuteStage();
    void testSystemVerilogExportFetchStage();
    void testSystemVerilogExportMemoryStage();

    // Level 9 - Full CPUs
    void testSystemVerilogExportCpu16BitRisc();
    void testSystemVerilogExportFetchStage16Bit();
    void testSystemVerilogExportMultiCycleCpu8Bit();
    void testSystemVerilogExportSingleCycleCpu8Bit();

private:
    /**
     * @brief Test helper for verifying SystemVerilog export with validation tools
     *
     * Creates a circuit with IC + input switches + output LEDs, exports to SystemVerilog,
     * and validates the output with available tools (iverilog, yosys, verilator).
     *
     * @param icFile IC filename from Test/Integration/IC/Components/
     */
    void testSystemVerilogExportHelper(const QString &icFile);

    /**
     * @brief Validate SystemVerilog code with iverilog
     *
     * Attempts to compile the SystemVerilog file with iverilog.
     *
     * @param verilogFile Path to the SystemVerilog file
     * @return true if validation passes, false if tool unavailable or validation fails
     */
    static bool validateWithIverilog(const QString &verilogFile);

    /**
     * @brief Validate SystemVerilog code with yosys
     *
     * Attempts to synthesize the SystemVerilog file with yosys.
     *
     * @param verilogFile Path to the SystemVerilog file
     * @param moduleName Module name (defaults to filename without extension)
     * @return true if validation passes, false if tool unavailable or validation fails
     */
    static bool validateWithYosys(const QString &verilogFile, const QString &moduleName = QString());

    /**
     * @brief Validate SystemVerilog code with verilator
     *
     * Attempts to lint the SystemVerilog file with verilator.
     *
     * @param verilogFile Path to the SystemVerilog file
     * @return true if validation passes, false if tool unavailable or validation fails
     */
    static bool validateWithVerilator(const QString &verilogFile);

    /**
     * @brief Validate SystemVerilog code functionally using a testbench
     *
     * Compiles and runs a testbench with iverilog/vvp. The testbench drives
     * input vectors and compares outputs against expected values from wiRedPanda.
     *
     * @param verilogFile Path to the DUT SystemVerilog file
     * @param tbFile Path to the testbench SystemVerilog file
     * @return true if validation passes, false if tool unavailable or validation fails
     */
    static bool validateWithTestbench(const QString &verilogFile,
                                      const QString &tbFile);
};

