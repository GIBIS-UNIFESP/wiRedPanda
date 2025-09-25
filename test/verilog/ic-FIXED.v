// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: ic_fixed
// Generated: Thu Sep 25 23:03:53 2025
// Target FPGA: Generic-Small
// Resource Usage: 58/1000 LUTs, 215/1000 FFs, 37/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module ic_fixed (
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
wire ic_jkflipflop_jkflipflop_ic_in_0_35; // IC input 0
assign ic_jkflipflop_jkflipflop_ic_in_0_35 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_1_36; // IC input 1
assign ic_jkflipflop_jkflipflop_ic_in_1_36 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_2_37; // IC input 2
assign ic_jkflipflop_jkflipflop_ic_in_2_37 = input_clock1_1;
wire ic_jkflipflop_jkflipflop_ic_in_3_38; // IC input 3
assign ic_jkflipflop_jkflipflop_ic_in_3_38 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_4_39; // IC input 4
assign ic_jkflipflop_jkflipflop_ic_in_4_39 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_out_0_40; // IC output 0
wire ic_jkflipflop_jkflipflop_ic_out_1_41; // IC output 1
wire ic_input_input_ic_in_0_42; // IC input 0
assign ic_input_input_ic_in_0_42 = input_clock3_3;
wire ic_input_input_ic_in_1_43; // IC input 1
assign ic_input_input_ic_in_1_43 = input_clock2_2;
wire ic_input_input_ic_out_0_44; // IC output 0
wire ic_input_input_ic_out_1_45; // IC output 1
wire ic_input_input_ic_out_2_46; // IC output 2
wire ic_input_input_ic_out_3_47; // IC output 3
wire ic_display_4bits_display_4bits_ic_in_0_48; // IC input 0
assign ic_display_4bits_display_4bits_ic_in_0_48 = 1'b1;
wire ic_display_4bits_display_4bits_ic_in_1_49; // IC input 1
assign ic_display_4bits_display_4bits_ic_in_1_49 = 1'b1;
wire ic_display_4bits_display_4bits_ic_in_2_50; // IC input 2
assign ic_display_4bits_display_4bits_ic_in_2_50 = 1'b1;
wire ic_display_4bits_display_4bits_ic_in_3_51; // IC input 3
assign ic_display_4bits_display_4bits_ic_in_3_51 = ic_jkflipflop_jkflipflop_ic_out_1_41;
wire ic_display_4bits_display_4bits_ic_out_0_52; // IC output 0
wire ic_display_4bits_display_4bits_ic_out_1_53; // IC output 1
wire ic_display_4bits_display_4bits_ic_out_2_54; // IC output 2
wire ic_display_4bits_display_4bits_ic_out_3_55; // IC output 3
wire ic_display_4bits_display_4bits_ic_out_4_56; // IC output 4
wire ic_display_4bits_display_4bits_ic_out_5_57; // IC output 5
wire ic_display_4bits_display_4bits_ic_out_6_58; // IC output 6
wire ic_display_4bits_display_4bits_ic_out_7_59; // IC output 7
wire ic_display_4bits_display_4bits_ic_in_0_60; // IC input 0
assign ic_display_4bits_display_4bits_ic_in_0_60 = input_clock3_3;
wire ic_display_4bits_display_4bits_ic_in_1_61; // IC input 1
assign ic_display_4bits_display_4bits_ic_in_1_61 = input_clock2_2;
wire ic_display_4bits_display_4bits_ic_in_2_62; // IC input 2
assign ic_display_4bits_display_4bits_ic_in_2_62 = input_clock5_5;
wire ic_display_4bits_display_4bits_ic_in_3_63; // IC input 3
assign ic_display_4bits_display_4bits_ic_in_3_63 = input_clock4_4;
wire ic_display_4bits_display_4bits_ic_out_0_64; // IC output 0
wire ic_display_4bits_display_4bits_ic_out_1_65; // IC output 1
wire ic_display_4bits_display_4bits_ic_out_2_66; // IC output 2
wire ic_display_4bits_display_4bits_ic_out_3_67; // IC output 3
wire ic_display_4bits_display_4bits_ic_out_4_68; // IC output 4
wire ic_display_4bits_display_4bits_ic_out_5_69; // IC output 5
wire ic_display_4bits_display_4bits_ic_out_6_70; // IC output 6
wire ic_display_4bits_display_4bits_ic_out_7_71; // IC output 7
wire ic_display_3bits_display_3bits_ic_in_0_72; // IC input 0
assign ic_display_3bits_display_3bits_ic_in_0_72 = ic_input_input_ic_out_0_44;
wire ic_display_3bits_display_3bits_ic_in_1_73; // IC input 1
assign ic_display_3bits_display_3bits_ic_in_1_73 = ic_input_input_ic_out_1_45;
wire ic_display_3bits_display_3bits_ic_in_2_74; // IC input 2
assign ic_display_3bits_display_3bits_ic_in_2_74 = ic_input_input_ic_out_2_46;
wire ic_display_3bits_display_3bits_ic_out_0_75; // IC output 0
wire ic_display_3bits_display_3bits_ic_out_1_76; // IC output 1
wire ic_display_3bits_display_3bits_ic_out_2_77; // IC output 2
wire ic_display_3bits_display_3bits_ic_out_3_78; // IC output 3
wire ic_display_3bits_display_3bits_ic_out_4_79; // IC output 4
wire ic_display_3bits_display_3bits_ic_out_5_80; // IC output 5
wire ic_display_3bits_display_3bits_ic_out_6_81; // IC output 6
wire ic_display_3bits_display_3bits_ic_out_7_82; // IC output 7
    reg jk_flip_flop_86_0_0 = 1'b0;
    reg jk_flip_flop_86_1_1 = 1'b0;
    reg jk_flip_flop_85_0_0 = 1'b0;
    reg jk_flip_flop_85_1_1 = 1'b0;
    reg jk_flip_flop_84_0_0 = 1'b0;
    reg jk_flip_flop_84_1_1 = 1'b0;
    reg jk_flip_flop_83_0_0 = 1'b0;
    reg jk_flip_flop_83_1_1 = 1'b0;

    // ========= Logic Assignments =========
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock6_6) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_83_0_0 <= 1'b0; jk_flip_flop_83_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_83_0_0 <= 1'b1; jk_flip_flop_83_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_83_0_0 <= jk_flip_flop_83_1_1; jk_flip_flop_83_1_1 <= jk_flip_flop_83_0_0; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_85_0_0) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_84_0_0 <= 1'b0; jk_flip_flop_84_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_84_0_0 <= 1'b1; jk_flip_flop_84_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_84_0_0 <= jk_flip_flop_84_1_1; jk_flip_flop_84_1_1 <= jk_flip_flop_84_0_0; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_86_0_0) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_85_0_0 <= 1'b0; jk_flip_flop_85_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_85_0_0 <= 1'b1; jk_flip_flop_85_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_85_0_0 <= jk_flip_flop_85_1_1; jk_flip_flop_85_1_1 <= jk_flip_flop_85_0_0; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge ic_jkflipflop_jkflipflop_ic_out_0_40) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_86_0_0 <= 1'b0; jk_flip_flop_86_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_86_0_0 <= 1'b1; jk_flip_flop_86_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_86_0_0 <= jk_flip_flop_86_1_1; jk_flip_flop_86_1_1 <= jk_flip_flop_86_0_0; end // toggle
            endcase
        end
    end


    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_7 = ic_display_3bits_display_3bits_ic_out_0_75; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_8 = ic_display_3bits_display_3bits_ic_out_1_76; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_9 = ic_display_3bits_display_3bits_ic_out_2_77; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_10 = ic_display_3bits_display_3bits_ic_out_3_78; // 7-Segment Display
    assign output_7_segment_display1_a_top_11 = ic_display_3bits_display_3bits_ic_out_4_79; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_12 = ic_display_3bits_display_3bits_ic_out_5_80; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_13 = ic_display_3bits_display_3bits_ic_out_6_81; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_14 = ic_display_3bits_display_3bits_ic_out_7_82; // 7-Segment Display
    assign output_7_segment_display2_g_middle_15 = ic_display_4bits_display_4bits_ic_out_0_52; // 7-Segment Display
    assign output_7_segment_display2_f_upper_left_16 = ic_display_4bits_display_4bits_ic_out_1_53; // 7-Segment Display
    assign output_7_segment_display2_e_lower_left_17 = ic_display_4bits_display_4bits_ic_out_2_54; // 7-Segment Display
    assign output_7_segment_display2_d_bottom_18 = ic_display_4bits_display_4bits_ic_out_3_55; // 7-Segment Display
    assign output_7_segment_display2_a_top_19 = ic_display_4bits_display_4bits_ic_out_4_56; // 7-Segment Display
    assign output_7_segment_display2_b_upper_right_20 = ic_display_4bits_display_4bits_ic_out_5_57; // 7-Segment Display
    assign output_7_segment_display2_dp_dot_21 = ic_display_4bits_display_4bits_ic_out_6_58; // 7-Segment Display
    assign output_7_segment_display2_c_lower_right_22 = ic_display_4bits_display_4bits_ic_out_7_59; // 7-Segment Display
    assign output_7_segment_display3_g_middle_23 = ic_display_4bits_display_4bits_ic_out_0_64; // 7-Segment Display
    assign output_7_segment_display3_f_upper_left_24 = ic_display_4bits_display_4bits_ic_out_1_65; // 7-Segment Display
    assign output_7_segment_display3_e_lower_left_25 = ic_display_4bits_display_4bits_ic_out_2_66; // 7-Segment Display
    assign output_7_segment_display3_d_bottom_26 = ic_display_4bits_display_4bits_ic_out_3_67; // 7-Segment Display
    assign output_7_segment_display3_a_top_27 = ic_display_4bits_display_4bits_ic_out_4_68; // 7-Segment Display
    assign output_7_segment_display3_b_upper_right_28 = ic_display_4bits_display_4bits_ic_out_5_69; // 7-Segment Display
    assign output_7_segment_display3_dp_dot_29 = ic_display_4bits_display_4bits_ic_out_6_70; // 7-Segment Display
    assign output_7_segment_display3_c_lower_right_30 = ic_display_4bits_display_4bits_ic_out_7_71; // 7-Segment Display
    assign output_led4_0_31 = ic_input_input_ic_out_1_45; // LED
    assign output_led5_0_32 = ic_input_input_ic_out_0_44; // LED
    assign output_led6_0_33 = ic_input_input_ic_out_2_46; // LED
    assign output_led7_0_34 = jk_flip_flop_83_0_0; // LED

endmodule // ic_fixed

// ====================================================================
// Module ic_fixed generation completed successfully
// Elements processed: 22
// Inputs: 6, Outputs: 28
// Warnings: 5
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 4 is not connected
//   IC INPUT output 3 is not connected
// ====================================================================
