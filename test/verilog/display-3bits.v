// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits
// Generated: Fri Sep 26 00:05:54 2025
// Target FPGA: Generic-Small
// Resource Usage: 49/1000 LUTs, 0/1000 FFs, 12/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

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

    // ========= Internal Signals =========
    wire or_37_0;
    wire or_36_0;
    wire or_35_0;
    wire not_34_0;
    wire not_33_0;
    wire not_32_0;
    wire and_31_0;
    wire and_30_0;
    wire and_29_0;
    wire and_28_0;
    wire xnor_27_0;
    wire and_26_0;
    wire and_25_0;
    wire and_24_0;
    wire and_23_0;
    wire and_22_0;
    wire xnor_21_0;
    wire and_20_0;
    wire node_19_0;
    wire or_18_0;
    wire or_17_0;
    wire nand_16_0;
    wire or_15_0;
    wire or_14_0;
    wire or_13_0;
    wire or_12_0;

    // ========= Logic Assignments =========
    assign or_12_0 = (~(~(1'b0 | input_input_switch2_p1_2) ^ ~(1'b0 | input_input_switch1_p3_1)) | (1'b0 | input_input_switch3_p2_3)); // Or
    assign or_13_0 = ((~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1)) | (~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3)) | ((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & (1'b0 | input_input_switch1_p3_1))); // Or
    assign or_14_0 = (~(1'b0 | input_input_switch2_p1_2) | ~(~(1'b0 | input_input_switch3_p2_3) ^ ~(1'b0 | input_input_switch1_p3_1))); // Or
    assign or_15_0 = (((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3)) | (~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3)) | (input_input_switch3_p2_3 & ~(1'b0 | input_input_switch1_p3_1))); // Or
    assign nand_16_0 = ~(~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // Nand
    assign or_17_0 = ((~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1))); // Or
    assign or_18_0 = (((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | (~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1))); // Or
    assign node_19_0 = 1'b0; // Node
    assign and_20_0 = (~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3)); // And
    assign xnor_21_0 = ~(~(1'b0 | input_input_switch2_p1_2) ^ ~(1'b0 | input_input_switch1_p3_1)); // Xnor
    assign and_22_0 = (~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_23_0 = (~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_24_0 = (input_input_switch3_p2_3 & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_25_0 = ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & (1'b0 | input_input_switch1_p3_1)); // And
    assign and_26_0 = ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign xnor_27_0 = ~(~(1'b0 | input_input_switch3_p2_3) ^ ~(1'b0 | input_input_switch1_p3_1)); // Xnor
    assign and_28_0 = (~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_29_0 = ((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_30_0 = ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3)); // And
    assign and_31_0 = ((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign not_32_0 = ~(1'b0 | input_input_switch3_p2_3); // Not
    assign not_33_0 = ~(1'b0 | input_input_switch1_p3_1); // Not
    assign not_34_0 = ~(1'b0 | input_input_switch2_p1_2); // Not
    assign or_35_0 = (1'b0 | input_input_switch2_p1_2); // Or
    assign or_36_0 = (1'b0 | input_input_switch3_p2_3); // Or
    assign or_37_0 = (1'b0 | input_input_switch1_p3_1); // Or

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_4 = (((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3)) | (~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3)) | (input_input_switch3_p2_3 & ~(1'b0 | input_input_switch1_p3_1))); // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_5 = ((~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1))); // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_6 = (((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | (~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1))); // 7-Segment Display
    assign output_7_segment_display1_d_bottom_7 = ((~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1)) | (~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3)) | ((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & (1'b0 | input_input_switch1_p3_1))); // 7-Segment Display
    assign output_7_segment_display1_a_top_8 = (~(~(1'b0 | input_input_switch2_p1_2) ^ ~(1'b0 | input_input_switch1_p3_1)) | (1'b0 | input_input_switch3_p2_3)); // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_9 = (~(1'b0 | input_input_switch2_p1_2) | ~(~(1'b0 | input_input_switch3_p2_3) ^ ~(1'b0 | input_input_switch1_p3_1))); // 7-Segment Display
    assign output_7_segment_display1_dp_dot_10 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_11 = ~(~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // 7-Segment Display

endmodule // display_3bits

// ====================================================================
// Module display_3bits generation completed successfully
// Elements processed: 32
// Inputs: 3, Outputs: 8
// ====================================================================
