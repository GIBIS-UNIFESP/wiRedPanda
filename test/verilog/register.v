// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: register
// Generated: Fri Sep 26 15:11:01 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 74/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module register (
    // ========= Input Ports =========
    input wire input_clock1_1,
    input wire input_clock2_2,

    // ========= Output Ports =========
    output wire output_led1_0_3,
    output wire output_led2_0_4,
    output wire output_led3_0_5,
    output wire output_led4_0_6
);

    // ========= Internal Signals =========
    wire node_7;
    wire node_8;
    reg d_flip_flop_9_0_q = 1'b0;
    reg d_flip_flop_9_1_q = 1'b0;
    wire node_10;
    reg d_flip_flop_11_0_q = 1'b0;
    reg d_flip_flop_11_1_q = 1'b0;
    reg d_flip_flop_12_0_q = 1'b0;
    reg d_flip_flop_12_1_q = 1'b0;
    wire node_13;
    reg d_flip_flop_14_0_q = 1'b0;
    reg d_flip_flop_14_1_q = 1'b0;

    // ========= Logic Assignments =========
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_13) begin
        begin
            d_flip_flop_14_0_q <= d_flip_flop_12_0_q;
            d_flip_flop_14_1_q <= ~d_flip_flop_12_0_q;
        end
    end

    assign node_13 = node_10; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_10) begin
        begin
            d_flip_flop_12_0_q <= d_flip_flop_11_0_q;
            d_flip_flop_12_1_q <= ~d_flip_flop_11_0_q;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge node_8) begin
        begin
            d_flip_flop_11_0_q <= d_flip_flop_9_0_q;
            d_flip_flop_11_1_q <= ~d_flip_flop_9_0_q;
        end
    end

    assign node_10 = node_8; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_7) begin
        begin
            d_flip_flop_9_0_q <= input_clock2_2;
            d_flip_flop_9_1_q <= ~input_clock2_2;
        end
    end

    assign node_8 = node_7; // Node
    assign node_7 = input_clock1_1; // Node

    // ========= Output Assignments =========
    assign output_led1_0_3 = d_flip_flop_9_0_q; // LED
    assign output_led2_0_4 = d_flip_flop_12_0_q; // LED
    assign output_led3_0_5 = d_flip_flop_14_0_q; // LED
    assign output_led4_0_6 = d_flip_flop_11_0_q; // LED

endmodule // register

// ====================================================================
// Module register generation completed successfully
// Elements processed: 14
// Inputs: 2, Outputs: 4
// ====================================================================
