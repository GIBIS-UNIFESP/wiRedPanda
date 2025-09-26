// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: ic
// Generated: Fri Sep 26 00:42:03 2025
// Target FPGA: Generic-Small
// Resource Usage: 58/1000 LUTs, 215/1000 FFs, 37/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module ic (
    // ========= Input Ports =========
    input wire input_clock1_1,
    input wire input_clock2_2,
    input wire input_clock3_3,
    input wire input_clock4_4,
    input wire input_clock5_5,
    input wire input_clock6_6,

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_7,
    output wire output_7_segment_display1_f_upper_left_8,
    output wire output_7_segment_display1_e_lower_left_9,
    output wire output_7_segment_display1_d_bottom_10,
    output wire output_7_segment_display1_a_top_11,
    output wire output_7_segment_display1_b_upper_right_12,
    output wire output_7_segment_display1_dp_dot_13,
    output wire output_7_segment_display1_c_lower_right_14,
    output wire output_7_segment_display2_g_middle_15,
    output wire output_7_segment_display2_f_upper_left_16,
    output wire output_7_segment_display2_e_lower_left_17,
    output wire output_7_segment_display2_d_bottom_18,
    output wire output_7_segment_display2_a_top_19,
    output wire output_7_segment_display2_b_upper_right_20,
    output wire output_7_segment_display2_dp_dot_21,
    output wire output_7_segment_display2_c_lower_right_22,
    output wire output_7_segment_display3_g_middle_23,
    output wire output_7_segment_display3_f_upper_left_24,
    output wire output_7_segment_display3_e_lower_left_25,
    output wire output_7_segment_display3_d_bottom_26,
    output wire output_7_segment_display3_a_top_27,
    output wire output_7_segment_display3_b_upper_right_28,
    output wire output_7_segment_display3_dp_dot_29,
    output wire output_7_segment_display3_c_lower_right_30,
    output wire output_led4_0_31,
    output wire output_led5_0_32,
    output wire output_led6_0_33,
    output wire output_led7_0_34
);

    // ========= Internal Signals =========
wire ic_jkflipflop_jkflipflop_ic_in_0_39; // IC input 0
assign ic_jkflipflop_jkflipflop_ic_in_0_39 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_1_40; // IC input 1
assign ic_jkflipflop_jkflipflop_ic_in_1_40 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_2_41; // IC input 2
assign ic_jkflipflop_jkflipflop_ic_in_2_41 = input_clock1_1;
wire ic_jkflipflop_jkflipflop_ic_in_3_42; // IC input 3
assign ic_jkflipflop_jkflipflop_ic_in_3_42 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_4_43; // IC input 4
assign ic_jkflipflop_jkflipflop_ic_in_4_43 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_out_0_44; // IC output 0
wire ic_jkflipflop_jkflipflop_ic_out_1_45; // IC output 1
wire ic_input_input_ic_in_0_46; // IC input 0
assign ic_input_input_ic_in_0_46 = input_clock3_3;
wire ic_input_input_ic_in_1_47; // IC input 1
assign ic_input_input_ic_in_1_47 = input_clock2_2;
wire ic_input_input_ic_out_0_48; // IC output 0
wire ic_input_input_ic_out_1_49; // IC output 1
wire ic_input_input_ic_out_2_50; // IC output 2
wire ic_input_input_ic_out_3_51; // IC output 3
wire ic_display_4bits_display_4bits_ic_in_0_52; // IC input 0
assign ic_display_4bits_display_4bits_ic_in_0_52 = jk_flip_flop_36_1_1;
wire ic_display_4bits_display_4bits_ic_in_1_53; // IC input 1
assign ic_display_4bits_display_4bits_ic_in_1_53 = jk_flip_flop_37_1_1;
wire ic_display_4bits_display_4bits_ic_in_2_54; // IC input 2
assign ic_display_4bits_display_4bits_ic_in_2_54 = jk_flip_flop_38_1_1;
wire ic_display_4bits_display_4bits_ic_in_3_55; // IC input 3
assign ic_display_4bits_display_4bits_ic_in_3_55 = ic_jkflipflop_jkflipflop_ic_out_1_45;
wire ic_display_4bits_display_4bits_ic_out_0_56; // IC output 0
wire ic_display_4bits_display_4bits_ic_out_1_57; // IC output 1
wire ic_display_4bits_display_4bits_ic_out_2_58; // IC output 2
wire ic_display_4bits_display_4bits_ic_out_3_59; // IC output 3
wire ic_display_4bits_display_4bits_ic_out_4_60; // IC output 4
wire ic_display_4bits_display_4bits_ic_out_5_61; // IC output 5
wire ic_display_4bits_display_4bits_ic_out_6_62; // IC output 6
wire ic_display_4bits_display_4bits_ic_out_7_63; // IC output 7
wire ic_display_4bits_display_4bits_ic_in_0_64; // IC input 0
assign ic_display_4bits_display_4bits_ic_in_0_64 = input_clock3_3;
wire ic_display_4bits_display_4bits_ic_in_1_65; // IC input 1
assign ic_display_4bits_display_4bits_ic_in_1_65 = input_clock2_2;
wire ic_display_4bits_display_4bits_ic_in_2_66; // IC input 2
assign ic_display_4bits_display_4bits_ic_in_2_66 = input_clock5_5;
wire ic_display_4bits_display_4bits_ic_in_3_67; // IC input 3
assign ic_display_4bits_display_4bits_ic_in_3_67 = input_clock4_4;
wire ic_display_4bits_display_4bits_ic_out_0_68; // IC output 0
wire ic_display_4bits_display_4bits_ic_out_1_69; // IC output 1
wire ic_display_4bits_display_4bits_ic_out_2_70; // IC output 2
wire ic_display_4bits_display_4bits_ic_out_3_71; // IC output 3
wire ic_display_4bits_display_4bits_ic_out_4_72; // IC output 4
wire ic_display_4bits_display_4bits_ic_out_5_73; // IC output 5
wire ic_display_4bits_display_4bits_ic_out_6_74; // IC output 6
wire ic_display_4bits_display_4bits_ic_out_7_75; // IC output 7
wire ic_display_3bits_display_3bits_ic_in_0_76; // IC input 0
assign ic_display_3bits_display_3bits_ic_in_0_76 = ic_input_input_ic_out_0_48;
wire ic_display_3bits_display_3bits_ic_in_1_77; // IC input 1
assign ic_display_3bits_display_3bits_ic_in_1_77 = ic_input_input_ic_out_1_49;
wire ic_display_3bits_display_3bits_ic_in_2_78; // IC input 2
assign ic_display_3bits_display_3bits_ic_in_2_78 = ic_input_input_ic_out_2_50;
wire ic_display_3bits_display_3bits_ic_out_0_79; // IC output 0
wire ic_display_3bits_display_3bits_ic_out_1_80; // IC output 1
wire ic_display_3bits_display_3bits_ic_out_2_81; // IC output 2
wire ic_display_3bits_display_3bits_ic_out_3_82; // IC output 3
wire ic_display_3bits_display_3bits_ic_out_4_83; // IC output 4
wire ic_display_3bits_display_3bits_ic_out_5_84; // IC output 5
wire ic_display_3bits_display_3bits_ic_out_6_85; // IC output 6
wire ic_display_3bits_display_3bits_ic_out_7_86; // IC output 7
    reg jk_flip_flop_38_0_0 = 1'b0;
    reg jk_flip_flop_38_1_1 = 1'b0;
    reg jk_flip_flop_37_0_0 = 1'b0;
    reg jk_flip_flop_37_1_1 = 1'b0;
    reg jk_flip_flop_36_0_0 = 1'b0;
    reg jk_flip_flop_36_1_1 = 1'b0;
    reg jk_flip_flop_35_0_0 = 1'b0;
    reg jk_flip_flop_35_1_1 = 1'b0;

    // ========= Logic Assignments =========
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock6_6) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_35_0_0 <= 1'b0; jk_flip_flop_35_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_35_0_0 <= 1'b1; jk_flip_flop_35_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_35_0_0 <= jk_flip_flop_35_1_1; jk_flip_flop_35_1_1 <= jk_flip_flop_35_0_0; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_37_0_0) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_36_0_0 <= 1'b0; jk_flip_flop_36_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_36_0_0 <= 1'b1; jk_flip_flop_36_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_36_0_0 <= jk_flip_flop_36_1_1; jk_flip_flop_36_1_1 <= jk_flip_flop_36_0_0; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_38_0_0) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_37_0_0 <= 1'b0; jk_flip_flop_37_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_37_0_0 <= 1'b1; jk_flip_flop_37_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_37_0_0 <= jk_flip_flop_37_1_1; jk_flip_flop_37_1_1 <= jk_flip_flop_37_0_0; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge ic_jkflipflop_jkflipflop_ic_out_0_44) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_38_0_0 <= 1'b0; jk_flip_flop_38_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_38_0_0 <= 1'b1; jk_flip_flop_38_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_38_0_0 <= jk_flip_flop_38_1_1; jk_flip_flop_38_1_1 <= jk_flip_flop_38_0_0; end // toggle
            endcase
        end
    end


    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_7 = ic_display_3bits_display_3bits_ic_out_0_79; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_8 = ic_display_3bits_display_3bits_ic_out_1_80; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_9 = ic_display_3bits_display_3bits_ic_out_2_81; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_10 = ic_display_3bits_display_3bits_ic_out_3_82; // 7-Segment Display
    assign output_7_segment_display1_a_top_11 = ic_display_3bits_display_3bits_ic_out_4_83; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_12 = ic_display_3bits_display_3bits_ic_out_5_84; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_13 = ic_display_3bits_display_3bits_ic_out_6_85; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_14 = ic_display_3bits_display_3bits_ic_out_7_86; // 7-Segment Display
    assign output_7_segment_display2_g_middle_15 = ic_display_4bits_display_4bits_ic_out_0_56; // 7-Segment Display
    assign output_7_segment_display2_f_upper_left_16 = ic_display_4bits_display_4bits_ic_out_1_57; // 7-Segment Display
    assign output_7_segment_display2_e_lower_left_17 = ic_display_4bits_display_4bits_ic_out_2_58; // 7-Segment Display
    assign output_7_segment_display2_d_bottom_18 = ic_display_4bits_display_4bits_ic_out_3_59; // 7-Segment Display
    assign output_7_segment_display2_a_top_19 = ic_display_4bits_display_4bits_ic_out_4_60; // 7-Segment Display
    assign output_7_segment_display2_b_upper_right_20 = ic_display_4bits_display_4bits_ic_out_5_61; // 7-Segment Display
    assign output_7_segment_display2_dp_dot_21 = ic_display_4bits_display_4bits_ic_out_6_62; // 7-Segment Display
    assign output_7_segment_display2_c_lower_right_22 = ic_display_4bits_display_4bits_ic_out_7_63; // 7-Segment Display
    assign output_7_segment_display3_g_middle_23 = ic_display_4bits_display_4bits_ic_out_0_68; // 7-Segment Display
    assign output_7_segment_display3_f_upper_left_24 = ic_display_4bits_display_4bits_ic_out_1_69; // 7-Segment Display
    assign output_7_segment_display3_e_lower_left_25 = ic_display_4bits_display_4bits_ic_out_2_70; // 7-Segment Display
    assign output_7_segment_display3_d_bottom_26 = ic_display_4bits_display_4bits_ic_out_3_71; // 7-Segment Display
    assign output_7_segment_display3_a_top_27 = ic_display_4bits_display_4bits_ic_out_4_72; // 7-Segment Display
    assign output_7_segment_display3_b_upper_right_28 = ic_display_4bits_display_4bits_ic_out_5_73; // 7-Segment Display
    assign output_7_segment_display3_dp_dot_29 = ic_display_4bits_display_4bits_ic_out_6_74; // 7-Segment Display
    assign output_7_segment_display3_c_lower_right_30 = ic_display_4bits_display_4bits_ic_out_7_75; // 7-Segment Display
    assign output_led4_0_31 = ic_input_input_ic_out_1_49; // LED
    assign output_led5_0_32 = ic_input_input_ic_out_0_48; // LED
    assign output_led6_0_33 = ic_input_input_ic_out_2_50; // LED
    assign output_led7_0_34 = jk_flip_flop_35_0_0; // LED

endmodule // ic

// ====================================================================
// Module ic generation completed successfully
// Elements processed: 22
// Inputs: 6, Outputs: 28
// Warnings: 5
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 4 is not connected
//   IC INPUT output 3 is not connected
// ====================================================================
