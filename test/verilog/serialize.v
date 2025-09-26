// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: serialize
// Generated: Fri Sep 26 21:32:37 2025
// Target FPGA: Generic-Small
// Resource Usage: 45/1000 LUTs, 4/1000 FFs, 8/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module serialize (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2
);

    // ========= Internal Signals =========
    wire node_3;
    wire node_4;
    wire node_5;
    wire not_6;
    wire node_7;
    wire node_8;
    reg d_flip_flop_9_0_q = 1'b0;
    reg d_flip_flop_9_1_q = 1'b0;
    wire and_10;
    wire and_11;
    wire node_12;
    wire or_13;
    wire node_14;
    wire node_15;
    reg d_flip_flop_16_0_q = 1'b0;
    reg d_flip_flop_16_1_q = 1'b0;
    wire and_17;
    wire and_18;
    wire node_19;
    wire or_20;
    wire node_21;
    wire node_22;
    reg d_flip_flop_23_0_q = 1'b0;
    reg d_flip_flop_23_1_q = 1'b0;
    wire and_24;
    wire and_25;
    wire node_26;
    wire or_27;
    wire node_28;
    wire node_29;
    reg d_flip_flop_30_0_q = 1'b0;
    reg d_flip_flop_30_1_q = 1'b0;
    wire node_31;
    wire and_32;

    // ========= Logic Assignments =========
    assign and_32 = (node_28 & d_flip_flop_30_0_q); // And
    assign node_31 = node_29; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_26) begin
        begin
            d_flip_flop_30_0_q <= or_27;
            d_flip_flop_30_1_q <= ~or_27;
        end
    end

    assign node_28 = not_6; // Node
    assign or_27 = (and_25 | and_24); // Or
    assign node_26 = 1'b0; // Node
    assign and_25 = (node_21 & 1'b0); // And
    assign and_24 = (node_22 & d_flip_flop_23_0_q); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_19) begin
        begin
            d_flip_flop_23_0_q <= or_20;
            d_flip_flop_23_1_q <= ~or_20;
        end
    end

    assign node_22 = not_6; // Node
    assign node_21 = node_5; // Node
    assign or_20 = (and_17 | and_18); // Or
    assign node_19 = 1'b0; // Node
    assign and_18 = (node_15 & d_flip_flop_16_0_q); // And
    assign and_17 = (node_14 & 1'b0); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_12) begin
        begin
            d_flip_flop_16_0_q <= or_13;
            d_flip_flop_16_1_q <= ~or_13;
        end
    end

    assign node_15 = not_6; // Node
    assign node_14 = node_5; // Node
    assign or_13 = (and_11 | and_10); // Or
    assign node_12 = 1'b0; // Node
    assign and_11 = (node_7 & 1'b0); // And
    assign and_10 = (node_8 & d_flip_flop_9_0_q); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_4) begin
        begin
            d_flip_flop_9_0_q <= 1'b0;
            d_flip_flop_9_1_q <= ~1'b0;
        end
    end

    assign node_8 = not_6; // Node
    assign node_7 = node_5; // Node
    assign not_6 = ~node_3; // Not
    assign node_4 = 1'b0; // Node
    assign node_3 = 1'b0; // Node

    // ========= Output Assignments =========
    assign output_led1_0_1 = node_31; // LED
    assign output_led2_0_2 = and_32; // LED

endmodule // serialize

// ====================================================================
// Module serialize generation completed successfully
// Elements processed: 38
// Inputs: 0, Outputs: 2
// ====================================================================
