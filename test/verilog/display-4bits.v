// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits
// Generated: Fri Sep 26 14:36:24 2025
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
    wire node_13;
    wire node_14;
    wire node_15;
    wire not_16;
    wire node_17;
    wire node_18;
    wire not_19;
    wire not_20;
    wire node_21;
    wire node_22;
    wire node_23;
    wire node_24;
    wire node_25;
    wire node_26;
    wire node_27;
    wire node_28;
    wire node_29;
    wire node_30;
    wire node_31;
    wire node_32;
    wire node_33;
    wire and_34;
    wire node_35;
    wire node_36;
    wire node_37;
    wire node_38;
    wire node_39;
    wire node_40;
    wire and_41;
    wire and_42;
    wire node_43;
    wire node_44;
    wire node_45;
    wire node_46;
    wire node_47;
    wire node_48;
    wire node_49;
    wire node_50;
    wire node_51;
    wire and_52;
    wire node_53;
    wire and_54;
    wire node_55;
    wire node_56;
    wire node_57;
    wire and_58;
    wire and_59;
    wire and_60;
    wire and_61;
    wire or_62;
    wire or_63;
    wire or_64;
    wire or_65;
    wire or_66;
    wire or_67;
    wire or_68;
    wire not_69;
    wire node_70;
    wire node_71;
    wire node_72;
    wire node_73;
    wire node_74;
    wire node_75;
    wire node_76;
    wire node_77;

    // ========= Logic Assignments =========
    assign node_77 = ~input_input_switch1_d_1; // Node
    assign node_76 = input_input_switch4_a_4; // Node
    assign node_75 = ~input_input_switch4_a_4; // Node
    assign node_74 = ~input_input_switch2_b_2; // Node
    assign node_73 = input_input_switch2_b_2; // Node
    assign node_72 = ~input_input_switch3_c_3; // Node
    assign node_71 = input_input_switch3_c_3; // Node
    assign node_70 = input_input_switch1_d_1; // Node
    assign not_69 = ~input_input_switch4_a_4; // Not
    assign or_68 = ((input_input_switch2_b_2 & input_input_switch1_d_1) | input_input_switch4_a_4 | input_input_switch3_c_3 | (~input_input_switch2_b_2 & ~input_input_switch1_d_1)); // Or
    assign or_67 = ((input_input_switch3_c_3 & input_input_switch1_d_1) | ~input_input_switch2_b_2 | (~input_input_switch3_c_3 & ~input_input_switch1_d_1)); // Or
    assign or_66 = (input_input_switch4_a_4 | (~input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch3_c_3 & ~input_input_switch1_d_1) | (~input_input_switch2_b_2 & input_input_switch3_c_3) | (input_input_switch1_d_1 & (input_input_switch2_b_2 & ~input_input_switch3_c_3))); // Or
    assign or_65 = ((~input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch3_c_3 & ~input_input_switch1_d_1)); // Or
    assign or_64 = (input_input_switch2_b_2 | ~input_input_switch3_c_3 | input_input_switch1_d_1); // Or
    assign or_63 = ((~input_input_switch3_c_3 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch3_c_3) | input_input_switch4_a_4); // Or
    assign or_62 = ((input_input_switch3_c_3 & ~input_input_switch1_d_1) | (input_input_switch2_b_2 & ~input_input_switch3_c_3) | input_input_switch4_a_4 | (~input_input_switch2_b_2 & input_input_switch3_c_3)); // Or
    assign and_61 = (input_input_switch2_b_2 & input_input_switch1_d_1); // And
    assign and_60 = (input_input_switch3_c_3 & input_input_switch1_d_1); // And
    assign and_59 = (~input_input_switch2_b_2 & input_input_switch3_c_3); // And
    assign and_58 = (input_input_switch1_d_1 & (input_input_switch2_b_2 & ~input_input_switch3_c_3)); // And
    assign node_57 = (~input_input_switch2_b_2 & ~input_input_switch1_d_1); // Node
    assign node_56 = (input_input_switch3_c_3 & ~input_input_switch1_d_1); // Node
    assign node_55 = (~input_input_switch3_c_3 & ~input_input_switch1_d_1); // Node
    assign and_54 = (input_input_switch2_b_2 & ~input_input_switch1_d_1); // And
    assign node_53 = input_input_switch4_a_4; // Node
    assign and_52 = (input_input_switch2_b_2 & ~input_input_switch3_c_3); // And
    assign node_51 = (~input_input_switch2_b_2 & ~input_input_switch1_d_1); // Node
    assign node_50 = input_input_switch3_c_3; // Node
    assign node_49 = ~input_input_switch2_b_2; // Node
    assign node_48 = input_input_switch1_d_1; // Node
    assign node_47 = (input_input_switch3_c_3 & ~input_input_switch1_d_1); // Node
    assign node_46 = (~input_input_switch3_c_3 & ~input_input_switch1_d_1); // Node
    assign node_45 = ~input_input_switch1_d_1; // Node
    assign node_44 = input_input_switch2_b_2; // Node
    assign node_43 = input_input_switch4_a_4; // Node
    assign and_42 = (~input_input_switch3_c_3 & ~input_input_switch1_d_1); // And
    assign and_41 = (input_input_switch3_c_3 & ~input_input_switch1_d_1); // And
    assign node_40 = ~input_input_switch2_b_2; // Node
    assign node_39 = input_input_switch1_d_1; // Node
    assign node_38 = (~input_input_switch2_b_2 & ~input_input_switch1_d_1); // Node
    assign node_37 = input_input_switch4_a_4; // Node
    assign node_36 = input_input_switch2_b_2; // Node
    assign node_35 = ~input_input_switch3_c_3; // Node
    assign and_34 = (~input_input_switch2_b_2 & ~input_input_switch1_d_1); // And
    assign node_33 = input_input_switch4_a_4; // Node
    assign node_32 = input_input_switch1_d_1; // Node
    assign node_31 = ~input_input_switch3_c_3; // Node
    assign node_30 = input_input_switch2_b_2; // Node
    assign node_29 = ~input_input_switch1_d_1; // Node
    assign node_28 = input_input_switch3_c_3; // Node
    assign node_27 = input_input_switch4_a_4; // Node
    assign node_26 = input_input_switch2_b_2; // Node
    assign node_25 = input_input_switch1_d_1; // Node
    assign node_24 = ~input_input_switch2_b_2; // Node
    assign node_23 = input_input_switch3_c_3; // Node
    assign node_22 = ~input_input_switch1_d_1; // Node
    assign node_21 = ~input_input_switch3_c_3; // Node
    assign not_20 = ~input_input_switch3_c_3; // Not
    assign not_19 = ~input_input_switch2_b_2; // Not
    assign node_18 = ~input_input_switch1_d_1; // Node
    assign node_17 = input_input_switch3_c_3; // Node
    assign not_16 = ~input_input_switch1_d_1; // Not
    assign node_15 = input_input_switch3_c_3; // Node
    assign node_14 = input_input_switch2_b_2; // Node
    assign node_13 = input_input_switch1_d_1; // Node

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
