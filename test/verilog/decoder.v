// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: decoder
// Generated: Sun Sep 28 03:36:03 2025
// Target FPGA: Generic-Small
// Resource Usage: 66/1000 LUTs, 0/1000 FFs, 12/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module decoder (
    // ========= Input Ports =========
    input wire input_input_switch1_1,
    input wire input_input_switch2_2,
    input wire input_input_switch3_3,

    // ========= Output Ports =========
    output wire output_led1_0_4,
    output wire output_led2_0_5,
    output wire output_led3_0_6,
    output wire output_led4_0_7,
    output wire output_led5_0_8,
    output wire output_led6_0_9,
    output wire output_led7_0_10,
    output wire output_led8_0_11
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_0_4 = (~input_input_switch3_3 & ~input_input_switch1_1 & ~input_input_switch2_2); // LED
    assign output_led2_0_5 = (~input_input_switch3_3 & ~input_input_switch1_1 & input_input_switch2_2); // LED
    assign output_led3_0_6 = (~input_input_switch3_3 & input_input_switch1_1 & ~input_input_switch2_2); // LED
    assign output_led4_0_7 = (~input_input_switch3_3 & input_input_switch1_1 & input_input_switch2_2); // LED
    assign output_led5_0_8 = (input_input_switch3_3 & ~input_input_switch1_1 & ~input_input_switch2_2); // LED
    assign output_led6_0_9 = (input_input_switch3_3 & ~input_input_switch1_1 & input_input_switch2_2); // LED
    assign output_led7_0_10 = (input_input_switch3_3 & input_input_switch1_1 & ~input_input_switch2_2); // LED
    assign output_led8_0_11 = (input_input_switch2_2 & input_input_switch1_1 & input_input_switch3_3); // LED


    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 3
    // ULTRATHINK DEBUG: Scanned content size: 885 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // decoder

// ====================================================================
// Module decoder generation completed successfully
// Elements processed: 55
// Inputs: 3, Outputs: 8
// ====================================================================
