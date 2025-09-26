// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits
// Generated: Fri Sep 26 14:59:08 2025
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
    assign or_39 = (xnor_29 | or_14); // Or
    assign or_38 = (and_28 | and_31 | and_19 | and_25); // Or
    assign or_37 = (not_16 | xnor_23); // Or
    assign or_36 = (and_20 | and_31 | and_26); // Or
    assign nand_35 = ~(not_16 & or_14 & not_17); // Nand
    assign or_34 = (and_27 | and_20 | and_24); // Or
    assign or_33 = (and_21 | and_22); // Or
    assign node_32 = 1'b0; // Node
    assign and_31 = (not_16 & or_14); // And
    assign xnor_29 = ~(not_16 ^ not_17); // Xnor
    assign and_28 = (not_16 & not_17); // And
    assign and_27 = (not_18 & not_17); // And
    assign and_26 = (input_input_switch3_p2_3 & not_17); // And
    assign and_25 = (or_15 & not_18 & or_13); // And
    assign and_24 = (or_15 & not_17); // And
    assign xnor_23 = ~(not_18 ^ not_17); // Xnor
    assign and_22 = (not_16 & not_18 & not_17); // And
    assign and_21 = (or_14 & not_17); // And
    assign and_20 = (or_15 & not_18); // And
    assign and_19 = (or_14 & not_17); // And
    assign not_18 = ~or_14; // Not
    assign not_17 = ~or_13; // Not
    assign not_16 = ~or_15; // Not
    assign or_15 = (1'b0 | input_input_switch2_p1_2); // Or
    assign or_14 = (1'b0 | input_input_switch3_p2_3); // Or
    assign or_13 = (1'b0 | input_input_switch1_p3_1); // Or

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_4 = or_36; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_5 = or_34; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_6 = or_33; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_7 = or_38; // 7-Segment Display
    assign output_7_segment_display1_a_top_8 = or_39; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_9 = or_37; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_10 = node_32; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_11 = nand_35; // 7-Segment Display

endmodule // display_3bits

// ====================================================================
// Module display_3bits generation completed successfully
// Elements processed: 32
// Inputs: 3, Outputs: 8
// ====================================================================
