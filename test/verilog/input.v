// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: input_module
// Generated: Sat Sep 27 18:03:53 2025
// Target FPGA: Generic-Small
// Resource Usage: 12/1000 LUTs, 0/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module input_module (
    // ========= Input Ports =========
    input wire input_push_button1_x0_1,
    input wire input_push_button2_x1_2,

    // ========= Output Ports =========
    output wire output_led1_xor_0_3,
    output wire output_led2_not_x0_0_4,
    output wire output_led3_and_0_5,
    output wire output_led4_or_0_6
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_xor_0_3 = (input_push_button1_x0_1 ^ input_push_button2_x1_2); // LED
    assign output_led2_not_x0_0_4 = ~input_push_button1_x0_1; // LED
    assign output_led3_and_0_5 = (input_push_button1_x0_1 & input_push_button2_x1_2); // LED
    assign output_led4_or_0_6 = (input_push_button1_x0_1 | input_push_button2_x1_2); // LED

endmodule // input_module

// ====================================================================
// Module input_module generation completed successfully
// Elements processed: 10
// Inputs: 2, Outputs: 4
// ====================================================================
