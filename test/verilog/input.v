// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: wiredpanda_module
// Generated: Fri Sep 26 21:28:12 2025
// Target FPGA: Generic-Small
// Resource Usage: 12/1000 LUTs, 0/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module wiredpanda_module (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_xor_0_1,
    output wire output_led2_not_x0_0_2,
    output wire output_led3_and_0_3,
    output wire output_led4_or_0_4
);

    // ========= Internal Signals =========
    wire not_5;
    wire xor_6;
    wire or_7;
    wire and_8;

    // ========= Logic Assignments =========
    assign and_8 = (1'b0 & 1'b0); // And
    assign or_7 = (1'b0 | 1'b0); // Or
    assign xor_6 = (1'b0 ^ 1'b0); // Xor
    assign not_5 = ~1'b0; // Not

    // ========= Output Assignments =========
    assign output_led1_xor_0_1 = xor_6; // LED
    assign output_led2_not_x0_0_2 = not_5; // LED
    assign output_led3_and_0_3 = and_8; // LED
    assign output_led4_or_0_4 = or_7; // LED

endmodule // wiredpanda_module

// ====================================================================
// Module wiredpanda_module generation completed successfully
// Elements processed: 10
// Inputs: 0, Outputs: 4
// ====================================================================
