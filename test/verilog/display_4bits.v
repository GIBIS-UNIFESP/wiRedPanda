// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits
// Generated: Fri Sep 26 21:22:23 2025
// Target FPGA: Generic-Small
// Resource Usage: 94/1000 LUTs, 0/1000 FFs, 13/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module display_4bits (
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
    wire node_9;
    wire node_10;
    wire node_11;
    wire not_12;
    wire node_13;
    wire node_14;
    wire not_15;
    wire not_16;
    wire node_17;
    wire node_18;
    wire node_19;
    wire node_20;
    wire node_21;
    wire node_22;
    wire node_23;
    wire node_24;
    wire node_25;
    wire node_26;
    wire node_27;
    wire node_28;
    wire node_29;
    wire and_30;
    wire node_31;
    wire node_32;
    wire node_33;
    wire node_34;
    wire node_35;
    wire node_36;
    wire and_37;
    wire and_38;
    wire node_39;
    wire node_40;
    wire node_41;
    wire node_42;
    wire node_43;
    wire node_44;
    wire node_45;
    wire node_46;
    wire node_47;
    wire and_48;
    wire node_49;
    wire and_50;
    wire node_51;
    wire node_52;
    wire node_53;
    wire and_54;
    wire and_55;
    wire and_56;
    wire and_57;
    wire or_58;
    wire or_59;
    wire or_60;
    wire or_61;
    wire or_62;
    wire or_63;
    wire or_64;
    wire not_65;

    // ========= Logic Assignments =========
    assign not_65 = ~node_23; // Not
    assign or_64 = (and_57 | node_29 | node_13 | node_34); // Or
    assign or_63 = (and_56 | node_36 | node_42); // Or
    assign or_62 = (node_33 | node_47 | node_43 | and_55 | and_54); // Or
    assign or_61 = (node_53 | node_52); // Or
    assign or_60 = (node_26 | node_27 | node_35); // Or
    assign or_59 = (node_51 | and_50 | and_48 | node_39); // Or
    assign or_58 = (node_52 | and_48 | node_49 | and_55); // Or
    assign and_57 = (node_22 & node_21); // And
    assign and_56 = (node_19 & node_28); // And
    assign and_55 = (node_45 & node_46); // And
    assign and_54 = (node_44 & and_48); // And
    assign node_53 = node_47; // Node
    assign node_52 = node_43; // Node
    assign node_51 = node_42; // Node
    assign and_50 = (node_40 & node_41); // And
    assign node_49 = node_39; // Node
    assign and_48 = (node_32 & node_31); // And
    assign node_47 = node_34; // Node
    assign node_46 = node_24; // Node
    assign node_45 = node_36; // Node
    assign node_44 = node_35; // Node
    assign node_43 = and_37; // Node
    assign node_42 = and_38; // Node
    assign node_41 = node_25; // Node
    assign node_40 = node_32; // Node
    assign node_39 = node_33; // Node
    assign and_38 = (node_17 & node_18); // And
    assign and_37 = (node_24 & node_25); // And
    assign node_36 = node_20; // Node
    assign node_35 = node_28; // Node
    assign node_34 = and_30; // Node
    assign node_33 = node_29; // Node
    assign node_32 = node_26; // Node
    assign node_31 = node_27; // Node
    assign and_30 = (node_20 & node_14); // And
    assign node_29 = node_23; // Node
    assign node_28 = node_21; // Node
    assign node_27 = node_17; // Node
    assign node_26 = node_22; // Node
    assign node_25 = node_18; // Node
    assign node_24 = node_19; // Node
    assign node_23 = 1'b0; // Node
    assign node_22 = node_10; // Node
    assign node_21 = node_9; // Node
    assign node_20 = not_15; // Node
    assign node_19 = node_13; // Node
    assign node_18 = node_14; // Node
    assign node_17 = not_16; // Node
    assign not_16 = ~node_11; // Not
    assign not_15 = ~node_10; // Not
    assign node_14 = not_12; // Node
    assign node_13 = node_11; // Node
    assign not_12 = ~node_9; // Not
    assign node_11 = 1'b0; // Node
    assign node_10 = 1'b0; // Node
    assign node_9 = 1'b0; // Node

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_1 = or_58; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_2 = or_59; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_3 = or_61; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_4 = or_62; // 7-Segment Display
    assign output_7_segment_display1_a_top_5 = or_64; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_6 = or_63; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_8 = or_60; // 7-Segment Display

endmodule // display_4bits

// ====================================================================
// Module display_4bits generation completed successfully
// Elements processed: 70
// Inputs: 0, Outputs: 8
// Warnings: 1
//   Output element 7-Segment Display input is not connected
// ====================================================================
