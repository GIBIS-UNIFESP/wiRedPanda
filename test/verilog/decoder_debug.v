// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: decoder_debug
// Generated: Sat Sep 27 15:27:50 2025
// Target FPGA: Generic-Small
// Resource Usage: 66/1000 LUTs, 0/1000 FFs, 12/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module decoder_debug (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2,
    output wire output_led3_0_3,
    output wire output_led4_0_4,
    output wire output_led5_0_5,
    output wire output_led6_0_6,
    output wire output_led7_0_7,
    output wire output_led8_0_8
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_0_1 = (1'b0 & 1'b0 & 1'b0); // LED
    assign output_led2_0_2 = (1'b0 & 1'b0 & ~1'b0); // LED
    assign output_led3_0_3 = (1'b0 & ~1'b0 & 1'b0); // LED
    assign output_led4_0_4 = (1'b0 & ~1'b0 & ~1'b0); // LED
    assign output_led5_0_5 = (~1'b0 & 1'b0 & 1'b0); // LED
    assign output_led6_0_6 = (~1'b0 & 1'b0 & ~1'b0); // LED
    assign output_led7_0_7 = (~1'b0 & ~1'b0 & 1'b0); // LED
    assign output_led8_0_8 = (~1'b0 & ~1'b0 & ~1'b0); // LED

endmodule // decoder_debug

// ====================================================================
// Module decoder_debug generation completed successfully
// Elements processed: 55
// Inputs: 0, Outputs: 8
// ====================================================================
