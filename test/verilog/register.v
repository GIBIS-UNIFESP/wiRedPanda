// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: register
// Generated: Fri Sep 26 21:22:28 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 74/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module register (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2,
    output wire output_led3_0_3,
    output wire output_led4_0_4
);

    // ========= Internal Signals =========
    wire node_5;
    wire node_6;
    reg d_flip_flop_7_0_q = 1'b0;
    reg d_flip_flop_7_1_q = 1'b0;
    wire node_8;
    reg d_flip_flop_9_0_q = 1'b0;
    reg d_flip_flop_9_1_q = 1'b0;
    reg d_flip_flop_10_0_q = 1'b0;
    reg d_flip_flop_10_1_q = 1'b0;
    wire node_11;
    reg d_flip_flop_12_0_q = 1'b0;
    reg d_flip_flop_12_1_q = 1'b0;

    // ========= Logic Assignments =========
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_11) begin
        begin
            d_flip_flop_12_0_q <= d_flip_flop_10_0_q;
            d_flip_flop_12_1_q <= ~d_flip_flop_10_0_q;
        end
    end

    assign node_11 = node_8; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_8) begin
        begin
            d_flip_flop_10_0_q <= d_flip_flop_9_0_q;
            d_flip_flop_10_1_q <= ~d_flip_flop_9_0_q;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge node_6) begin
        begin
            d_flip_flop_9_0_q <= d_flip_flop_7_0_q;
            d_flip_flop_9_1_q <= ~d_flip_flop_7_0_q;
        end
    end

    assign node_8 = node_6; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_5) begin
        begin
            d_flip_flop_7_0_q <= 1'b0;
            d_flip_flop_7_1_q <= ~1'b0;
        end
    end

    assign node_6 = node_5; // Node
    assign node_5 = 1'b0; // Node

    // ========= Output Assignments =========
    assign output_led1_0_1 = d_flip_flop_7_0_q; // LED
    assign output_led2_0_2 = d_flip_flop_10_0_q; // LED
    assign output_led3_0_3 = d_flip_flop_12_0_q; // LED
    assign output_led4_0_4 = d_flip_flop_9_0_q; // LED

endmodule // register

// ====================================================================
// Module register generation completed successfully
// Elements processed: 14
// Inputs: 0, Outputs: 4
// ====================================================================
