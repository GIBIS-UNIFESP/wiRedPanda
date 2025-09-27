// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: input_module
// Generated: Sat Sep 27 14:03:51 2025
// Target FPGA: Generic-Small
// Resource Usage: 12/1000 LUTs, 0/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module input_module (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_xor_0_1,
    output wire output_led2_not_x0_0_2,
    output wire output_led3_and_0_3,
    output wire output_led4_or_0_4
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_xor_0_1 = (1'b0 ^ 1'b0); // LED
    assign output_led2_not_x0_0_2 = ~1'b0; // LED
    assign output_led3_and_0_3 = (1'b0 & 1'b0); // LED
    assign output_led4_or_0_4 = (1'b0 | 1'b0); // LED

endmodule // input_module

// ====================================================================
// Module input_module generation completed successfully
// Elements processed: 10
// Inputs: 0, Outputs: 4
// ====================================================================
