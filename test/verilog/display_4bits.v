// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits
// Generated: Sun Sep 28 03:51:17 2025
// Target FPGA: Generic-Small
// Resource Usage: 94/1000 LUTs, 0/1000 FFs, 13/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module display_4bits (
    // ========= Input Ports =========
    input wire input_input_switch1_d_1,
    input wire input_input_switch2_b_2,
    input wire input_input_switch3_c_3,
    input wire input_input_switch4_a_4,

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_5,
    output wire output_7_segment_display1_f_upper_left_6,
    output wire output_7_segment_display1_e_lower_left_7,
    output wire output_7_segment_display1_d_bottom_8,
    output wire output_7_segment_display1_a_top_9,
    output wire output_7_segment_display1_b_upper_right_10,
    output wire output_7_segment_display1_dp_dot_11,
    output wire output_7_segment_display1_c_lower_right_12
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_7_segment_display1_g_middle_5 = ((~input_input_switch3_c_3 & input_input_switch1_d_1) | (~input_input_switch2_b_2 & input_input_switch3_c_3) | ~input_input_switch4_a_4 | (input_input_switch2_b_2 & ~input_input_switch3_c_3)); // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_6 = ((input_input_switch3_c_3 & input_input_switch1_d_1) | (~input_input_switch2_b_2 & input_input_switch1_d_1) | (~input_input_switch2_b_2 & input_input_switch3_c_3) | ~input_input_switch4_a_4); // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_7 = ((input_input_switch2_b_2 & input_input_switch1_d_1) | (~input_input_switch3_c_3 & input_input_switch1_d_1)); // 7-Segment Display
    assign output_7_segment_display1_d_bottom_8 = (~input_input_switch4_a_4 | (input_input_switch2_b_2 & input_input_switch1_d_1) | (~input_input_switch3_c_3 & input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch3_c_3) | (~input_input_switch1_d_1 & (~input_input_switch2_b_2 & input_input_switch3_c_3))); // 7-Segment Display
    assign output_7_segment_display1_a_top_9 = ((~input_input_switch2_b_2 & ~input_input_switch1_d_1) | ~input_input_switch4_a_4 | ~input_input_switch3_c_3 | (input_input_switch2_b_2 & input_input_switch1_d_1)); // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_10 = ((~input_input_switch3_c_3 & ~input_input_switch1_d_1) | input_input_switch2_b_2 | (input_input_switch3_c_3 & input_input_switch1_d_1)); // 7-Segment Display
    assign output_7_segment_display1_dp_dot_11 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_12 = (~input_input_switch2_b_2 | input_input_switch3_c_3 | ~input_input_switch1_d_1); // 7-Segment Display


    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 4
    // ULTRATHINK DEBUG: Scanned content size: 1743 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // display_4bits

// ====================================================================
// Module display_4bits generation completed successfully
// Elements processed: 70
// Inputs: 4, Outputs: 8
// Warnings: 1
//   Output element 7-Segment Display input is not connected
// ====================================================================
