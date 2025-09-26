// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: register
// Generated: Fri Sep 26 00:06:01 2025
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
    wire node_14_0;
    wire node_13_0;
    reg d_flip_flop_12_0_0 = 1'b0;
    reg d_flip_flop_12_1_1 = 1'b0;
    wire node_11_0;
    reg d_flip_flop_10_0_0 = 1'b0;
    reg d_flip_flop_10_1_1 = 1'b0;
    reg d_flip_flop_9_0_0 = 1'b0;
    reg d_flip_flop_9_1_1 = 1'b0;
    wire node_8_0;
    reg d_flip_flop_7_0_0 = 1'b0;
    reg d_flip_flop_7_1_1 = 1'b0;

    // ========= Logic Assignments =========
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_7_0_0 <= d_flip_flop_9_0_0;
            d_flip_flop_7_1_1 <= ~d_flip_flop_9_0_0;
        end
    end

    assign node_8_0 = input_clock1_1; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_9_0_0 <= d_flip_flop_10_0_0;
            d_flip_flop_9_1_1 <= ~d_flip_flop_10_0_0;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_10_0_0 <= d_flip_flop_12_0_0;
            d_flip_flop_10_1_1 <= ~d_flip_flop_12_0_0;
        end
    end

    assign node_11_0 = input_clock1_1; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_12_0_0 <= input_clock2_2;
            d_flip_flop_12_1_1 <= ~input_clock2_2;
        end
    end

    assign node_13_0 = input_clock1_1; // Node
    assign node_14_0 = input_clock1_1; // Node

    // ========= Output Assignments =========
    assign output_led1_0_3 = d_flip_flop_12_0_0; // LED
    assign output_led2_0_4 = d_flip_flop_9_0_0; // LED
    assign output_led3_0_5 = d_flip_flop_7_0_0; // LED
    assign output_led4_0_6 = d_flip_flop_10_0_0; // LED

endmodule // register

// ====================================================================
// Module register generation completed successfully
// Elements processed: 14
// Inputs: 2, Outputs: 4
// ====================================================================
