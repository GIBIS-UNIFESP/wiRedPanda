// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits
// Generated: Fri Sep 26 00:42:02 2025
// Target FPGA: Generic-Small
// Resource Usage: 94/1000 LUTs, 0/1000 FFs, 13/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

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

    // ========= Internal Signals =========
    wire node_77_0;
    wire node_76_0;
    wire node_75_0;
    wire not_74_0;
    wire node_73_0;
    wire node_72_0;
    wire not_71_0;
    wire not_70_0;
    wire node_69_0;
    wire node_68_0;
    wire node_67_0;
    wire node_66_0;
    wire node_65_0;
    wire node_64_0;
    wire node_63_0;
    wire node_62_0;
    wire node_61_0;
    wire node_60_0;
    wire node_59_0;
    wire node_58_0;
    wire node_57_0;
    wire and_56_0;
    wire node_55_0;
    wire node_54_0;
    wire node_53_0;
    wire node_52_0;
    wire node_51_0;
    wire node_50_0;
    wire and_49_0;
    wire and_48_0;
    wire node_47_0;
    wire node_46_0;
    wire node_45_0;
    wire node_44_0;
    wire node_43_0;
    wire node_42_0;
    wire node_41_0;
    wire node_40_0;
    wire node_39_0;
    wire and_38_0;
    wire node_37_0;
    wire and_36_0;
    wire node_35_0;
    wire node_34_0;
    wire node_33_0;
    wire and_32_0;
    wire and_31_0;
    wire and_30_0;
    wire and_29_0;
    wire or_28_0;
    wire or_27_0;
    wire or_26_0;
    wire or_25_0;
    wire or_24_0;
    wire or_23_0;
    wire or_22_0;
    wire not_21_0;
    wire node_20_0;
    wire node_19_0;
    wire node_18_0;
    wire node_17_0;
    wire node_16_0;
    wire node_15_0;
    wire node_14_0;
    wire node_13_0;

    // ========= Logic Assignments =========
    assign node_13_0 = ~input_input_switch1_d_1; // Node
    assign node_14_0 = input_input_switch4_a_4; // Node
    assign node_15_0 = ~input_input_switch4_a_4; // Node
    assign node_16_0 = ~input_input_switch2_b_2; // Node
    assign node_17_0 = input_input_switch2_b_2; // Node
    assign node_18_0 = ~input_input_switch3_c_3; // Node
    assign node_19_0 = input_input_switch3_c_3; // Node
    assign node_20_0 = input_input_switch1_d_1; // Node
    assign not_21_0 = ~input_input_switch4_a_4; // Not
    assign or_22_0 = ((input_input_switch2_b_2 & input_input_switch1_d_1) | input_input_switch4_a_4 | input_input_switch3_c_3 | (~input_input_switch2_b_2 & ~input_input_switch1_d_1)); // Or
    assign or_23_0 = ((input_input_switch3_c_3 & input_input_switch1_d_1) | ~input_input_switch2_b_2 | (~input_input_switch3_c_3 & ~input_input_switch1_d_1)); // Or
    assign or_24_0 = (input_input_switch4_a_4 | (~input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch3_c_3 & ~input_input_switch1_d_1) | (~input_input_switch2_b_2 & input_input_switch3_c_3) | (input_input_switch1_d_1 & (input_input_switch2_b_2 & ~input_input_switch3_c_3))); // Or
    assign or_25_0 = ((~input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch3_c_3 & ~input_input_switch1_d_1)); // Or
    assign or_26_0 = (input_input_switch2_b_2 | ~input_input_switch3_c_3 | input_input_switch1_d_1); // Or
    assign or_27_0 = ((~input_input_switch3_c_3 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch3_c_3) | input_input_switch4_a_4); // Or
    assign or_28_0 = ((input_input_switch3_c_3 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch3_c_3) | input_input_switch4_a_4 | (~input_input_switch2_b_2 & input_input_switch3_c_3)); // Or
    assign and_29_0 = (input_input_switch2_b_2 & input_input_switch1_d_1); // And
    assign and_30_0 = (input_input_switch3_c_3 & input_input_switch1_d_1); // And
    assign and_31_0 = (~input_input_switch2_b_2 & input_input_switch3_c_3); // And
    assign and_32_0 = (input_input_switch1_d_1 & (input_input_switch2_b_2 & ~input_input_switch3_c_3)); // And
    assign node_33_0 = (~input_input_switch2_b_2 & ~input_input_switch1_d_1); // Node
    assign node_34_0 = (input_input_switch3_c_3 & ~input_input_switch1_d_1); // Node
    assign node_35_0 = (~input_input_switch3_c_3 & ~input_input_switch1_d_1); // Node
    assign and_36_0 = (input_input_switch2_b_2 & ~input_input_switch1_d_1); // And
    assign node_37_0 = input_input_switch4_a_4; // Node
    assign and_38_0 = (input_input_switch2_b_2 & ~input_input_switch3_c_3); // And
    assign node_39_0 = (~input_input_switch2_b_2 & ~input_input_switch1_d_1); // Node
    assign node_40_0 = input_input_switch3_c_3; // Node
    assign node_41_0 = ~input_input_switch2_b_2; // Node
    assign node_42_0 = input_input_switch1_d_1; // Node
    assign node_43_0 = (input_input_switch3_c_3 & ~input_input_switch1_d_1); // Node
    assign node_44_0 = (~input_input_switch3_c_3 & ~input_input_switch1_d_1); // Node
    assign node_45_0 = ~input_input_switch1_d_1; // Node
    assign node_46_0 = input_input_switch2_b_2; // Node
    assign node_47_0 = input_input_switch4_a_4; // Node
    assign and_48_0 = (~input_input_switch3_c_3 & ~input_input_switch1_d_1); // And
    assign and_49_0 = (input_input_switch3_c_3 & ~input_input_switch1_d_1); // And
    assign node_50_0 = ~input_input_switch2_b_2; // Node
    assign node_51_0 = input_input_switch1_d_1; // Node
    assign node_52_0 = (~input_input_switch2_b_2 & ~input_input_switch1_d_1); // Node
    assign node_53_0 = input_input_switch4_a_4; // Node
    assign node_54_0 = input_input_switch2_b_2; // Node
    assign node_55_0 = ~input_input_switch3_c_3; // Node
    assign and_56_0 = (~input_input_switch2_b_2 & ~input_input_switch1_d_1); // And
    assign node_57_0 = input_input_switch4_a_4; // Node
    assign node_58_0 = input_input_switch1_d_1; // Node
    assign node_59_0 = ~input_input_switch3_c_3; // Node
    assign node_60_0 = input_input_switch2_b_2; // Node
    assign node_61_0 = ~input_input_switch1_d_1; // Node
    assign node_62_0 = input_input_switch3_c_3; // Node
    assign node_63_0 = input_input_switch4_a_4; // Node
    assign node_64_0 = input_input_switch2_b_2; // Node
    assign node_65_0 = input_input_switch1_d_1; // Node
    assign node_66_0 = ~input_input_switch2_b_2; // Node
    assign node_67_0 = input_input_switch3_c_3; // Node
    assign node_68_0 = ~input_input_switch1_d_1; // Node
    assign node_69_0 = ~input_input_switch3_c_3; // Node
    assign not_70_0 = ~input_input_switch3_c_3; // Not
    assign not_71_0 = ~input_input_switch2_b_2; // Not
    assign node_72_0 = ~input_input_switch1_d_1; // Node
    assign node_73_0 = input_input_switch3_c_3; // Node
    assign not_74_0 = ~input_input_switch1_d_1; // Not
    assign node_75_0 = input_input_switch3_c_3; // Node
    assign node_76_0 = input_input_switch2_b_2; // Node
    assign node_77_0 = input_input_switch1_d_1; // Node

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_5 = ((input_input_switch3_c_3 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch3_c_3) | input_input_switch4_a_4 | (~input_input_switch2_b_2 & input_input_switch3_c_3)); // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_6 = ((~input_input_switch3_c_3 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch3_c_3) | input_input_switch4_a_4); // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_7 = ((~input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch3_c_3 & ~input_input_switch1_d_1)); // 7-Segment Display
    assign output_7_segment_display1_d_bottom_8 = (input_input_switch4_a_4 | (~input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch3_c_3 & ~input_input_switch1_d_1) | (~input_input_switch2_b_2 & input_input_switch3_c_3) | (input_input_switch1_d_1 & (input_input_switch2_b_2 & ~input_input_switch3_c_3))); // 7-Segment Display
    assign output_7_segment_display1_a_top_9 = ((input_input_switch2_b_2 & input_input_switch1_d_1) | input_input_switch4_a_4 | input_input_switch3_c_3 | (~input_input_switch2_b_2 & ~input_input_switch1_d_1)); // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_10 = ((input_input_switch3_c_3 & input_input_switch1_d_1) | ~input_input_switch2_b_2 | (~input_input_switch3_c_3 & ~input_input_switch1_d_1)); // 7-Segment Display
    assign output_7_segment_display1_dp_dot_11 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_12 = (input_input_switch2_b_2 | ~input_input_switch3_c_3 | input_input_switch1_d_1); // 7-Segment Display

endmodule // display_4bits

// ====================================================================
// Module display_4bits generation completed successfully
// Elements processed: 70
// Inputs: 4, Outputs: 8
// Warnings: 1
//   Output element 7-Segment Display input is not connected
// ====================================================================
