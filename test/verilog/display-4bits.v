// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits
// Generated: Fri Sep 26 15:10:57 2025
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
    assign not_69 = ~node_27; // Not
    assign or_68 = (and_61 | node_33 | node_17 | node_38); // Or
    assign or_67 = (and_60 | node_40 | node_46); // Or
    assign or_66 = (node_37 | node_51 | node_47 | and_59 | and_58); // Or
    assign or_65 = (node_57 | node_56); // Or
    assign or_64 = (node_30 | node_31 | node_39); // Or
    assign or_63 = (node_55 | and_54 | and_52 | node_43); // Or
    assign or_62 = (node_56 | and_52 | node_53 | and_59); // Or
    assign and_61 = (node_26 & node_25); // And
    assign and_60 = (node_23 & node_32); // And
    assign and_59 = (node_49 & node_50); // And
    assign and_58 = (node_48 & and_52); // And
    assign node_57 = node_51; // Node
    assign node_56 = node_47; // Node
    assign node_55 = node_46; // Node
    assign and_54 = (node_44 & node_45); // And
    assign node_53 = node_43; // Node
    assign and_52 = (node_36 & node_35); // And
    assign node_51 = node_38; // Node
    assign node_50 = node_28; // Node
    assign node_49 = node_40; // Node
    assign node_48 = node_39; // Node
    assign node_47 = and_41; // Node
    assign node_46 = and_42; // Node
    assign node_45 = node_29; // Node
    assign node_44 = node_36; // Node
    assign node_43 = node_37; // Node
    assign and_42 = (node_21 & node_22); // And
    assign and_41 = (node_28 & node_29); // And
    assign node_40 = node_24; // Node
    assign node_39 = node_32; // Node
    assign node_38 = and_34; // Node
    assign node_37 = node_33; // Node
    assign node_36 = node_30; // Node
    assign node_35 = node_31; // Node
    assign and_34 = (node_24 & node_18); // And
    assign node_33 = node_27; // Node
    assign node_32 = node_25; // Node
    assign node_31 = node_21; // Node
    assign node_30 = node_26; // Node
    assign node_29 = node_22; // Node
    assign node_28 = node_23; // Node
    assign node_27 = input_input_switch4_a_4; // Node
    assign node_26 = node_14; // Node
    assign node_25 = node_13; // Node
    assign node_24 = not_19; // Node
    assign node_23 = node_17; // Node
    assign node_22 = node_18; // Node
    assign node_21 = not_20; // Node
    assign not_20 = ~node_15; // Not
    assign not_19 = ~node_14; // Not
    assign node_18 = not_16; // Node
    assign node_17 = node_15; // Node
    assign not_16 = ~node_13; // Not
    assign node_15 = input_input_switch3_c_3; // Node
    assign node_14 = input_input_switch2_b_2; // Node
    assign node_13 = input_input_switch1_d_1; // Node

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_5 = or_62; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_6 = or_63; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_7 = or_65; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_8 = or_66; // 7-Segment Display
    assign output_7_segment_display1_a_top_9 = or_68; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_10 = or_67; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_11 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_12 = or_64; // 7-Segment Display

endmodule // display_4bits

// ====================================================================
// Module display_4bits generation completed successfully
// Elements processed: 70
// Inputs: 4, Outputs: 8
// Warnings: 1
//   Output element 7-Segment Display input is not connected
// ====================================================================
