// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop
// Generated: Fri Sep 26 21:28:06 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module dflipflop (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2
);

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
    assign nand_18 = ~(node_17 & nand_16 & nand_21); // Nand
    assign nand_21 = ~(nand_18 & nand_19 & node_20); // Nand
    assign node_20 = 1'b0; // Node
    assign nand_19 = ~(node_10 & nand_15); // Nand
    assign node_17 = 1'b0; // Node
    assign nand_16 = ~(nand_12 & node_11); // Nand
    assign nand_12 = ~(1'b0 & nand_8 & nand_15); // Nand
    assign nand_15 = ~(nand_12 & nand_13 & node_14); // Nand
    assign node_14 = 1'b0; // Node
    assign nand_13 = ~(node_5 & not_9); // Nand
    assign node_11 = node_10; // Node
    assign node_10 = node_6; // Node
    assign not_9 = ~1'b0; // Not
    assign nand_8 = ~(1'b0 & node_7); // Nand
    assign node_7 = node_5; // Node
    assign node_5 = not_3; // Node
    assign not_4 = ~not_3; // Not
    assign not_3 = ~1'b0; // Not

    // ========= Output Assignments =========
    assign output_led1_0_1 = nand_18; // LED
    assign output_led2_0_2 = nand_21; // LED

endmodule // dflipflop

// ====================================================================
// Module dflipflop generation completed successfully
// Elements processed: 25
// Inputs: 0, Outputs: 2
// ====================================================================
