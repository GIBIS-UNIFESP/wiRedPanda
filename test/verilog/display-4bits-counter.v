// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits_counter
// Generated: Fri Sep 26 00:05:55 2025
// Target FPGA: Generic-Small
// Resource Usage: 42/1000 LUTs, 35/1000 FFs, 11/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module display_4bits_counter (
    // ========= Input Ports =========
    input wire input_push_button1_1,
    input wire input_clock2_2,

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_3,
    output wire output_7_segment_display1_f_upper_left_4,
    output wire output_7_segment_display1_e_lower_left_5,
    output wire output_7_segment_display1_d_bottom_6,
    output wire output_7_segment_display1_a_top_7,
    output wire output_7_segment_display1_b_upper_right_8,
    output wire output_7_segment_display1_dp_dot_9,
    output wire output_7_segment_display1_c_lower_right_10
);

    // ========= Internal Signals =========
wire ic_jkflipflop_jkflipflop_ic_in_0_29; // IC input 0
assign ic_jkflipflop_jkflipflop_ic_in_0_29 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_1_30; // IC input 1
assign ic_jkflipflop_jkflipflop_ic_in_1_30 = ~input_push_button1_1;
wire ic_jkflipflop_jkflipflop_ic_in_2_31; // IC input 2
assign ic_jkflipflop_jkflipflop_ic_in_2_31 = input_clock2_2;
wire ic_jkflipflop_jkflipflop_ic_in_3_32; // IC input 3
assign ic_jkflipflop_jkflipflop_ic_in_3_32 = ~input_push_button1_1;
wire ic_jkflipflop_jkflipflop_ic_in_4_33; // IC input 4
assign ic_jkflipflop_jkflipflop_ic_in_4_33 = ~(1'b0 & 1'b0 & 1'b0 & 1'b0);
wire ic_jkflipflop_jkflipflop_ic_out_0_34; // IC output 0
wire ic_jkflipflop_jkflipflop_ic_out_1_35; // IC output 1
wire ic_jkflipflop_jkflipflop_ic_in_0_36; // IC input 0
assign ic_jkflipflop_jkflipflop_ic_in_0_36 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_1_37; // IC input 1
assign ic_jkflipflop_jkflipflop_ic_in_1_37 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_2_38; // IC input 2
assign ic_jkflipflop_jkflipflop_ic_in_2_38 = ic_jkflipflop_jkflipflop_ic_out_1_35;
wire ic_jkflipflop_jkflipflop_ic_in_3_39; // IC input 3
assign ic_jkflipflop_jkflipflop_ic_in_3_39 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_4_40; // IC input 4
assign ic_jkflipflop_jkflipflop_ic_in_4_40 = ~(1'b0 & 1'b0 & 1'b0 & ic_jkflipflop_jkflipflop_ic_out_1_35);
wire ic_jkflipflop_jkflipflop_ic_out_0_41; // IC output 0
wire ic_jkflipflop_jkflipflop_ic_out_1_42; // IC output 1
wire ic_jkflipflop_jkflipflop_ic_in_0_43; // IC input 0
assign ic_jkflipflop_jkflipflop_ic_in_0_43 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_1_44; // IC input 1
assign ic_jkflipflop_jkflipflop_ic_in_1_44 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_2_45; // IC input 2
assign ic_jkflipflop_jkflipflop_ic_in_2_45 = ic_jkflipflop_jkflipflop_ic_out_1_42;
wire ic_jkflipflop_jkflipflop_ic_in_3_46; // IC input 3
assign ic_jkflipflop_jkflipflop_ic_in_3_46 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_4_47; // IC input 4
assign ic_jkflipflop_jkflipflop_ic_in_4_47 = ~(1'b0 & 1'b0 & ic_jkflipflop_jkflipflop_ic_out_0_41 & ic_jkflipflop_jkflipflop_ic_out_1_35);
wire ic_jkflipflop_jkflipflop_ic_out_0_48; // IC output 0
wire ic_jkflipflop_jkflipflop_ic_out_1_49; // IC output 1
wire ic_jkflipflop_jkflipflop_ic_in_0_50; // IC input 0
assign ic_jkflipflop_jkflipflop_ic_in_0_50 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_1_51; // IC input 1
assign ic_jkflipflop_jkflipflop_ic_in_1_51 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_2_52; // IC input 2
assign ic_jkflipflop_jkflipflop_ic_in_2_52 = ic_jkflipflop_jkflipflop_ic_out_1_49;
wire ic_jkflipflop_jkflipflop_ic_in_3_53; // IC input 3
assign ic_jkflipflop_jkflipflop_ic_in_3_53 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_4_54; // IC input 4
assign ic_jkflipflop_jkflipflop_ic_in_4_54 = ~(1'b0 & ic_jkflipflop_jkflipflop_ic_out_1_49 & ic_jkflipflop_jkflipflop_ic_out_0_41 & ic_jkflipflop_jkflipflop_ic_out_1_35);
wire ic_jkflipflop_jkflipflop_ic_out_0_55; // IC output 0
wire ic_jkflipflop_jkflipflop_ic_out_1_56; // IC output 1
wire ic_display_4bits_display_4bits_ic_in_0_57; // IC input 0
assign ic_display_4bits_display_4bits_ic_in_0_57 = ic_jkflipflop_jkflipflop_ic_out_0_55;
wire ic_display_4bits_display_4bits_ic_in_1_58; // IC input 1
assign ic_display_4bits_display_4bits_ic_in_1_58 = ic_jkflipflop_jkflipflop_ic_out_0_48;
wire ic_display_4bits_display_4bits_ic_in_2_59; // IC input 2
assign ic_display_4bits_display_4bits_ic_in_2_59 = ic_jkflipflop_jkflipflop_ic_out_0_41;
wire ic_display_4bits_display_4bits_ic_in_3_60; // IC input 3
assign ic_display_4bits_display_4bits_ic_in_3_60 = ic_jkflipflop_jkflipflop_ic_out_0_34;
wire ic_display_4bits_display_4bits_ic_out_0_61; // IC output 0
wire ic_display_4bits_display_4bits_ic_out_1_62; // IC output 1
wire ic_display_4bits_display_4bits_ic_out_2_63; // IC output 2
wire ic_display_4bits_display_4bits_ic_out_3_64; // IC output 3
wire ic_display_4bits_display_4bits_ic_out_4_65; // IC output 4
wire ic_display_4bits_display_4bits_ic_out_5_66; // IC output 5
wire ic_display_4bits_display_4bits_ic_out_6_67; // IC output 6
wire ic_display_4bits_display_4bits_ic_out_7_68; // IC output 7
    wire not_28_0;
    wire node_25_0;
    wire node_24_0;
    wire and_23_0;
    wire not_22_0;
    wire node_21_0;
    wire node_20_0;
    wire node_19_0;
    wire node_16_0;
    wire node_15_0;
    wire node_14_0;
    wire node_13_0;
    wire node_12_0;
    wire node_11_0;
    wire node_18_0;
    wire node_17_0;
    wire node_27_0;
    wire node_26_0;

    // ========= Logic Assignments =========
    assign node_11_0 = input_clock2_2; // Node
    assign node_12_0 = 1'b0; // Node
    assign node_13_0 = 1'b0; // Node
    assign node_14_0 = 1'b0; // Node
    assign node_15_0 = 1'b0; // Node
    assign node_16_0 = input_clock2_2; // Node
    assign node_17_0 = ~(1'b0 & 1'b0 & 1'b0 & 1'b0); // Node
    assign node_18_0 = ~(1'b0 & 1'b0 & 1'b0 & 1'b0); // Node
    assign node_19_0 = ~(1'b0 & 1'b0 & 1'b0 & 1'b0); // Node
    assign node_20_0 = ~(1'b0 & 1'b0 & 1'b0 & 1'b0); // Node
    assign node_21_0 = ~(1'b0 & 1'b0 & 1'b0 & 1'b0); // Node
    assign not_22_0 = ~(1'b0 & 1'b0 & 1'b0 & 1'b0); // Not
    assign and_23_0 = (1'b0 & 1'b0 & 1'b0 & 1'b0); // And
    assign node_24_0 = 1'b0; // Node
    assign node_25_0 = 1'b0; // Node
    assign node_26_0 = 1'b0; // Node
    assign node_27_0 = 1'b0; // Node
    assign not_28_0 = ~input_push_button1_1; // Not

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_3 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_4 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_5 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_6 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_8 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_9 = input_clock2_2; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_10 = 1'b0; // 7-Segment Display

endmodule // display_4bits_counter

// ====================================================================
// Module display_4bits_counter generation completed successfully
// Elements processed: 26
// Inputs: 2, Outputs: 8
// Warnings: 12
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP output 1 is not connected
//   IC DISPLAY-4BITS output 6 is not connected
// ====================================================================
