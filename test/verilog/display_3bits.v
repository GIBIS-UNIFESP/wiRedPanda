// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits
// Generated: Sun Sep 28 03:36:11 2025
// Target FPGA: Generic-Small
// Resource Usage: 49/1000 LUTs, 0/1000 FFs, 12/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module display_3bits (
    // ========= Input Ports =========
    input wire input_input_switch1_p3_1,
    input wire input_input_switch2_p1_2,
    input wire input_input_switch3_p2_3,

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_4,
    output wire output_7_segment_display1_f_upper_left_5,
    output wire output_7_segment_display1_e_lower_left_6,
    output wire output_7_segment_display1_d_bottom_7,
    output wire output_7_segment_display1_a_top_8,
    output wire output_7_segment_display1_b_upper_right_9,
    output wire output_7_segment_display1_dp_dot_10,
    output wire output_7_segment_display1_c_lower_right_11
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_7_segment_display1_g_middle_4 = ((~input_input_switch2_p1_2 & input_input_switch3_p2_3) | (input_input_switch2_p1_2 & ~input_input_switch3_p2_3) | (~input_input_switch3_p2_3 & input_input_switch1_p3_1)); // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_5 = ((input_input_switch3_p2_3 & input_input_switch1_p3_1) | (~input_input_switch2_p1_2 & input_input_switch3_p2_3) | (~input_input_switch2_p1_2 & input_input_switch1_p3_1)); // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_6 = ((~input_input_switch3_p2_3 & input_input_switch1_p3_1) | (input_input_switch2_p1_2 & input_input_switch3_p2_3 & input_input_switch1_p3_1)); // 7-Segment Display
    assign output_7_segment_display1_d_bottom_7 = ((input_input_switch2_p1_2 & input_input_switch1_p3_1) | (input_input_switch2_p1_2 & ~input_input_switch3_p2_3) | (~input_input_switch3_p2_3 & input_input_switch1_p3_1) | (~input_input_switch2_p1_2 & input_input_switch3_p2_3 & ~input_input_switch1_p3_1)); // 7-Segment Display
    assign output_7_segment_display1_a_top_8 = (~(input_input_switch2_p1_2 ^ input_input_switch1_p3_1) | ~input_input_switch3_p2_3); // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_9 = (input_input_switch2_p1_2 | ~(input_input_switch3_p2_3 ^ input_input_switch1_p3_1)); // 7-Segment Display
    assign output_7_segment_display1_dp_dot_10 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_11 = ~(input_input_switch2_p1_2 & ~input_input_switch3_p2_3 & input_input_switch1_p3_1); // 7-Segment Display


    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 3
    // ULTRATHINK DEBUG: Scanned content size: 1583 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // display_3bits

// ====================================================================
// Module display_3bits generation completed successfully
// Elements processed: 32
// Inputs: 3, Outputs: 8
// ====================================================================
