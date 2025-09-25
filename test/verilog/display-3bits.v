// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits
// Generated: Thu Sep 25 21:23:06 2025
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
    wire or_13;
    wire or_14;
    wire or_15;
    wire not_16;
    wire not_17;
    wire not_18;
    wire and_19;
    wire and_20;
    wire and_21;
    wire and_22;
    wire xnor_23;
    wire and_24;
    wire and_25;
    wire and_26;
    wire and_27;
    wire and_28;
    wire xnor_29;
    wire and_31;
    wire node_32;
    wire or_33;
    wire or_34;
    wire nand_35;
    wire or_36;
    wire or_37;
    wire or_38;
    wire or_39;

    // ========= Logic Assignments =========
    assign or_39 = (~(~(1'b0 | input_input_switch2_p1_2) ^ ~(1'b0 | input_input_switch1_p3_1)) | (1'b0 | input_input_switch3_p2_3)); // Or
    assign or_38 = ((~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1)) | (~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3)) | ((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & (1'b0 | input_input_switch1_p3_1))); // Or
    assign or_37 = (~(1'b0 | input_input_switch2_p1_2) | ~(~(1'b0 | input_input_switch3_p2_3) ^ ~(1'b0 | input_input_switch1_p3_1))); // Or
    assign or_36 = (((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3)) | (~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3)) | (input_input_switch3_p2_3 & ~(1'b0 | input_input_switch1_p3_1))); // Or
    assign nand_35 = ~(~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // Nand
    assign or_34 = ((~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3)) | ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1))); // Or
    assign or_33 = (((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)) | (~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1))); // Or
    assign node_32 = 1'b0; // Node
    assign and_31 = (~(1'b0 | input_input_switch2_p1_2) & (1'b0 | input_input_switch3_p2_3)); // And
    assign xnor_29 = ~(~(1'b0 | input_input_switch2_p1_2) ^ ~(1'b0 | input_input_switch1_p3_1)); // Xnor
    assign and_28 = (~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_27 = (~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_26 = (input_input_switch3_p2_3 & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_25 = ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & (1'b0 | input_input_switch1_p3_1)); // And
    assign and_24 = ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign xnor_23 = ~(~(1'b0 | input_input_switch3_p2_3) ^ ~(1'b0 | input_input_switch1_p3_1)); // Xnor
    assign and_22 = (~(1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_21 = ((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign and_20 = ((1'b0 | input_input_switch2_p1_2) & ~(1'b0 | input_input_switch3_p2_3)); // And
    assign and_19 = ((1'b0 | input_input_switch3_p2_3) & ~(1'b0 | input_input_switch1_p3_1)); // And
    assign not_18 = ~(1'b0 | input_input_switch3_p2_3); // Not
    assign not_17 = ~(1'b0 | input_input_switch1_p3_1); // Not
    assign not_16 = ~(1'b0 | input_input_switch2_p1_2); // Not
    assign or_15 = (1'b0 | input_input_switch2_p1_2); // Or
    assign or_14 = (1'b0 | input_input_switch3_p2_3); // Or
    assign or_13 = (1'b0 | input_input_switch1_p3_1); // Or

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
