// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dlatch
// Generated: Fri Sep 26 21:28:11 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module dlatch (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_q_0_1,
    output wire output_led2_q_0_2
);

    // ========= Internal Signals =========
    wire not_3;
    wire nand_4;
    wire nand_5;
    wire nand_6;
    wire nand_7;

    // ========= Logic Assignments =========
    assign nand_5 = ~(nand_7 & nand_4); // Nand
    assign nand_7 = ~(nand_6 & nand_5); // Nand
    assign nand_6 = ~(1'b0 & 1'b0); // Nand
    assign nand_4 = ~(1'b0 & not_3); // Nand
    assign not_3 = ~1'b0; // Not

    // ========= Output Assignments =========
    assign output_led1_q_0_1 = nand_5; // LED
    assign output_led2_q_0_2 = nand_7; // LED

endmodule // dlatch

// ====================================================================
// Module dlatch generation completed successfully
// Elements processed: 9
// Inputs: 0, Outputs: 2
// ====================================================================
