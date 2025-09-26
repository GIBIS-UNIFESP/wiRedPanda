// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits_counter
// Generated: Fri Sep 26 21:22:20 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 38/1000 FFs, 11/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module display_3bits_counter (
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
// ============== BEGIN IC: DISPLAY-3BITS ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
    wire ic_display_3bits_ic_node_15_0;
    wire ic_display_3bits_ic_node_16_0;
    wire ic_display_3bits_ic_node_17_0;
    wire ic_display_3bits_ic_gnd_18_0;
    wire ic_display_3bits_ic_or_19_0;
    wire ic_display_3bits_ic_or_20_0;
    wire ic_display_3bits_ic_or_21_0;
    wire ic_display_3bits_ic_not_22_0;
    wire ic_display_3bits_ic_not_23_0;
    wire ic_display_3bits_ic_not_24_0;
    wire ic_display_3bits_ic_and_25_0;
    wire ic_display_3bits_ic_and_26_0;
    wire ic_display_3bits_ic_and_27_0;
    wire ic_display_3bits_ic_and_28_0;
    wire ic_display_3bits_ic_xnor_29_0;
    wire ic_display_3bits_ic_and_30_0;
    wire ic_display_3bits_ic_and_31_0;
    wire ic_display_3bits_ic_and_32_0;
    wire ic_display_3bits_ic_and_33_0;
    wire ic_display_3bits_ic_and_34_0;
    wire ic_display_3bits_ic_xnor_35_0;
    wire ic_display_3bits_ic_gnd_36_0;
    wire ic_display_3bits_ic_and_37_0;
    wire ic_display_3bits_ic_node_38_0;
    wire ic_display_3bits_ic_or_39_0;
    wire ic_display_3bits_ic_or_40_0;
    wire ic_display_3bits_ic_nand_41_0;
    wire ic_display_3bits_ic_or_42_0;
    wire ic_display_3bits_ic_or_43_0;
    wire ic_display_3bits_ic_or_44_0;
    wire ic_display_3bits_ic_or_45_0;
    wire ic_display_3bits_ic_node_46_0;
    wire ic_display_3bits_ic_node_47_0;
    wire ic_display_3bits_ic_node_48_0;
    wire ic_display_3bits_ic_node_49_0;
    wire ic_display_3bits_ic_node_50_0;
    wire ic_display_3bits_ic_node_51_0;
    wire ic_display_3bits_ic_node_52_0;
    wire ic_display_3bits_ic_node_53_0;
// ============== END IC: DISPLAY-3BITS ==============
    wire node_54;
    reg t_flip_flop_55_0_q = 1'b0;
    reg t_flip_flop_55_1_q = 1'b0;
    wire node_56;
    reg t_flip_flop_57_0_q = 1'b0;
    reg t_flip_flop_57_1_q = 1'b0;
    wire node_58;
    reg t_flip_flop_59_0_q = 1'b0;
    reg t_flip_flop_59_1_q = 1'b0;
    wire node_60;
    wire node_61;
    wire node_62;
    wire node_63;
    wire node_64;
    wire node_65;

    // ========= Logic Assignments =========
    wire ic_display_3bits_ic_node_66_0;
    wire ic_display_3bits_ic_node_67_0;
    wire ic_display_3bits_ic_node_68_0;
    wire ic_display_3bits_ic_gnd_69_0;
    wire ic_display_3bits_ic_or_70_0;
    wire ic_display_3bits_ic_or_71_0;
    wire ic_display_3bits_ic_or_72_0;
    wire ic_display_3bits_ic_not_73_0;
    wire ic_display_3bits_ic_not_74_0;
    wire ic_display_3bits_ic_not_75_0;
    wire ic_display_3bits_ic_and_76_0;
    wire ic_display_3bits_ic_and_77_0;
    wire ic_display_3bits_ic_and_78_0;
    wire ic_display_3bits_ic_and_79_0;
    wire ic_display_3bits_ic_xnor_80_0;
    wire ic_display_3bits_ic_and_81_0;
    wire ic_display_3bits_ic_and_82_0;
    wire ic_display_3bits_ic_and_83_0;
    wire ic_display_3bits_ic_and_84_0;
    wire ic_display_3bits_ic_and_85_0;
    wire ic_display_3bits_ic_xnor_86_0;
    wire ic_display_3bits_ic_gnd_87_0;
    wire ic_display_3bits_ic_and_88_0;
    wire ic_display_3bits_ic_node_89_0;
    wire ic_display_3bits_ic_or_90_0;
    wire ic_display_3bits_ic_or_91_0;
    wire ic_display_3bits_ic_nand_92_0;
    wire ic_display_3bits_ic_or_93_0;
    wire ic_display_3bits_ic_or_94_0;
    wire ic_display_3bits_ic_or_95_0;
    wire ic_display_3bits_ic_or_96_0;
    wire ic_display_3bits_ic_node_97_0;
    wire ic_display_3bits_ic_node_98_0;
    wire ic_display_3bits_ic_node_99_0;
    wire ic_display_3bits_ic_node_100_0;
    wire ic_display_3bits_ic_node_101_0;
    wire ic_display_3bits_ic_node_102_0;
    wire ic_display_3bits_ic_node_103_0;
    wire ic_display_3bits_ic_node_104_0;
    assign node_65 = node_62; // Node
    assign node_64 = t_flip_flop_57_1_q; // Node
    assign node_63 = t_flip_flop_55_1_q; // Node
    assign node_61 = node_60; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge t_flip_flop_57_1_q) begin
        begin
            if (node_58) begin // toggle
                t_flip_flop_59_0_q <= t_flip_flop_59_1_q;
                t_flip_flop_59_1_q <= t_flip_flop_59_0_q;
            end
            // else hold
        end
    end

    assign node_58 = node_56; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge t_flip_flop_55_1_q) begin
        begin
            if (node_56) begin // toggle
                t_flip_flop_57_0_q <= t_flip_flop_57_1_q;
                t_flip_flop_57_1_q <= t_flip_flop_57_0_q;
            end
            // else hold
        end
    end

    assign node_56 = 1'b0; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge node_54) begin
        begin
            if (1'b0) begin // toggle
                t_flip_flop_55_0_q <= t_flip_flop_55_1_q;
                t_flip_flop_55_1_q <= t_flip_flop_55_0_q;
            end
            // else hold
        end
    end

    assign node_54 = 1'b0; // Node

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_1 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_2 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_3 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_4 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_5 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_6 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_7 = node_65; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_8 = 1'b0; // 7-Segment Display

endmodule // display_3bits_counter

// ====================================================================
// Module display_3bits_counter generation completed successfully
// Elements processed: 16
// Inputs: 0, Outputs: 8
// Warnings: 1
//   IC DISPLAY-3BITS output 6 is not connected
// ====================================================================
