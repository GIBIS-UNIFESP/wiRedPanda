// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_new
// Generated: Sat Sep 27 11:17:38 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module dflipflop_new (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2
);

    // ========= Logic Assignments =========

    // ========= Internal Signals =========
    wire not_3;
    wire not_4;
    wire node_5;
    wire node_6;
    wire node_7;
    wire nand_8;
    wire not_9;
    wire node_10;
    wire node_11;
    wire nand_12;
    wire nand_13;
    wire node_14;
    wire nand_15;
    wire nand_16;
    wire node_17;
    wire nand_18;
    wire nand_19;
    wire node_20;
    wire nand_21;

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    assign output_led1_0_1 = nand_18; // LED
    assign output_led2_0_2 = nand_21; // LED

endmodule // dflipflop_new

// ====================================================================
// Module dflipflop_new generation completed successfully
// Elements processed: 25
// Inputs: 0, Outputs: 2
// ====================================================================
