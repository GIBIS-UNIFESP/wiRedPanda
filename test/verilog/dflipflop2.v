// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop2
// Generated: Fri Sep 26 21:22:20 2025
// Target FPGA: Generic-Small
// Resource Usage: 28/1000 LUTs, 36/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module dflipflop2 (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2
);

    // ========= Internal Signals =========
    wire not_3;
    wire node_4;
    wire node_5;
    wire not_6;
    wire not_7;
    wire node_8;
    wire nand_9;
    wire node_10;
    wire nand_11;
    wire nand_12;
    wire node_13;
    wire node_14;
    wire nand_15;
    wire node_16;
    wire node_17;
    wire nand_18;
    wire nand_19;
    reg d_flip_flop_20_0_q = 1'b0;
    reg d_flip_flop_20_1_q = 1'b0;
    wire nand_21;
    wire nand_22;

    // ========= Logic Assignments =========
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_17) begin
        begin
            d_flip_flop_20_0_q <= node_10;
            d_flip_flop_20_1_q <= ~node_10;
        end
    end

    assign nand_19 = ~(nand_18 & nand_22); // Nand
    assign nand_22 = ~(nand_19 & nand_21); // Nand
    assign nand_21 = ~(node_13 & nand_11); // Nand
    assign nand_18 = ~(nand_15 & node_14); // Nand
    assign node_17 = node_16; // Node
    assign node_14 = node_13; // Node
    assign node_13 = node_8; // Node
    assign nand_11 = ~(nand_15 & nand_9); // Nand
    assign nand_15 = ~(nand_12 & nand_11); // Nand
    assign nand_12 = ~(node_10 & not_3); // Nand
    assign node_10 = 1'b0; // Node
    assign nand_9 = ~(node_4 & not_6); // Nand
    assign not_7 = ~node_5; // Not
    assign not_6 = ~1'b0; // Not
    assign node_5 = node_4; // Node
    assign node_4 = not_3; // Node
    assign not_3 = ~1'b0; // Not

    // ========= Output Assignments =========
    assign output_led1_0_1 = d_flip_flop_20_0_q; // LED
    assign output_led2_0_2 = nand_19; // LED

endmodule // dflipflop2

// ====================================================================
// Module dflipflop2 generation completed successfully
// Elements processed: 24
// Inputs: 0, Outputs: 2
// ====================================================================
