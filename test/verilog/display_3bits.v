// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits
// Generated: Fri Sep 26 21:28:08 2025
// Target FPGA: Generic-Small
// Resource Usage: 49/1000 LUTs, 0/1000 FFs, 12/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module display_3bits (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_1,
    output wire output_7_segment_display1_f_upper_left_2,
    output wire output_7_segment_display1_e_lower_left_3,
    output wire output_7_segment_display1_d_bottom_4,
    output wire output_7_segment_display1_a_top_5,
    output wire output_7_segment_display1_b_upper_right_6,
    output wire output_7_segment_display1_dp_dot_7,
    output wire output_7_segment_display1_c_lower_right_8
);

    // ========= Internal Signals =========
    wire or_10;
    wire or_11;
    wire or_12;
    wire not_13;
    wire not_14;
    wire not_15;
    wire and_16;
    wire and_17;
    wire and_18;
    wire and_19;
    wire xnor_20;
    wire and_21;
    wire and_22;
    wire and_23;
    wire and_24;
    wire and_25;
    wire xnor_26;
    wire and_28;
    wire node_29;
    wire or_30;
    wire or_31;
    wire nand_32;
    wire or_33;
    wire or_34;
    wire or_35;
    wire or_36;

    // ========= Logic Assignments =========
    assign or_36 = (xnor_26 | or_11); // Or
    assign or_35 = (and_25 | and_28 | and_16 | and_22); // Or
    assign or_34 = (not_13 | xnor_20); // Or
    assign or_33 = (and_17 | and_28 | and_23); // Or
    assign nand_32 = ~(not_13 & or_11 & not_14); // Nand
    assign or_31 = (and_24 | and_17 | and_21); // Or
    assign or_30 = (and_18 | and_19); // Or
    assign node_29 = 1'b0; // Node
    assign and_28 = (not_13 & or_11); // And
    assign xnor_26 = ~(not_13 ^ not_14); // Xnor
    assign and_25 = (not_13 & not_14); // And
    assign and_24 = (not_15 & not_14); // And
    assign and_23 = (1'b0 & not_14); // And
    assign and_22 = (or_12 & not_15 & or_10); // And
    assign and_21 = (or_12 & not_14); // And
    assign xnor_20 = ~(not_15 ^ not_14); // Xnor
    assign and_19 = (not_13 & not_15 & not_14); // And
    assign and_18 = (or_11 & not_14); // And
    assign and_17 = (or_12 & not_15); // And
    assign and_16 = (or_11 & not_14); // And
    assign not_15 = ~or_11; // Not
    assign not_14 = ~or_10; // Not
    assign not_13 = ~or_12; // Not
    assign or_12 = (1'b0 | 1'b0); // Or
    assign or_11 = (1'b0 | 1'b0); // Or
    assign or_10 = (1'b0 | 1'b0); // Or

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_1 = or_33; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_2 = or_31; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_3 = or_30; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_4 = or_35; // 7-Segment Display
    assign output_7_segment_display1_a_top_5 = or_36; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_6 = or_34; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_7 = node_29; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_8 = nand_32; // 7-Segment Display

endmodule // display_3bits

// ====================================================================
// Module display_3bits generation completed successfully
// Elements processed: 32
// Inputs: 0, Outputs: 8
// ====================================================================
