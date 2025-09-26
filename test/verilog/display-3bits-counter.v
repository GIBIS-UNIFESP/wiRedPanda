// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits_counter
// Generated: Fri Sep 26 19:56:47 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 38/1000 FFs, 11/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module display_3bits_counter (
    // ========= Input Ports =========
    input wire input_clock1_1,
    input wire input_push_button2_btn_2,

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
// ============== BEGIN IC: DISPLAY-3BITS ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
    wire ic_display_3bits_ic_node_17_0;
    wire ic_display_3bits_ic_node_18_0;
    wire ic_display_3bits_ic_node_19_0;
    wire ic_display_3bits_ic_gnd_20_0;
    wire ic_display_3bits_ic_or_21_0;
    wire ic_display_3bits_ic_or_22_0;
    wire ic_display_3bits_ic_or_23_0;
    wire ic_display_3bits_ic_not_24_0;
    wire ic_display_3bits_ic_not_25_0;
    wire ic_display_3bits_ic_not_26_0;
    wire ic_display_3bits_ic_and_27_0;
    wire ic_display_3bits_ic_and_28_0;
    wire ic_display_3bits_ic_and_29_0;
    wire ic_display_3bits_ic_and_30_0;
    wire ic_display_3bits_ic_xnor_31_0;
    wire ic_display_3bits_ic_and_32_0;
    wire ic_display_3bits_ic_and_33_0;
    wire ic_display_3bits_ic_and_34_0;
    wire ic_display_3bits_ic_and_35_0;
    wire ic_display_3bits_ic_and_36_0;
    wire ic_display_3bits_ic_xnor_37_0;
    wire ic_display_3bits_ic_gnd_38_0;
    wire ic_display_3bits_ic_and_39_0;
    wire ic_display_3bits_ic_node_40_0;
    wire ic_display_3bits_ic_or_41_0;
    wire ic_display_3bits_ic_or_42_0;
    wire ic_display_3bits_ic_nand_43_0;
    wire ic_display_3bits_ic_or_44_0;
    wire ic_display_3bits_ic_or_45_0;
    wire ic_display_3bits_ic_or_46_0;
    wire ic_display_3bits_ic_or_47_0;
    wire ic_display_3bits_ic_node_48_0;
    wire ic_display_3bits_ic_node_49_0;
    wire ic_display_3bits_ic_node_50_0;
    wire ic_display_3bits_ic_node_51_0;
    wire ic_display_3bits_ic_node_52_0;
    wire ic_display_3bits_ic_node_53_0;
    wire ic_display_3bits_ic_node_54_0;
    wire ic_display_3bits_ic_node_55_0;
// ============== END IC: DISPLAY-3BITS ==============
    wire node_56;
    reg t_flip_flop_57_0_q = 1'b0;
    reg t_flip_flop_57_1_q = 1'b0;
    wire node_58;
    reg t_flip_flop_59_0_q = 1'b0;
    reg t_flip_flop_59_1_q = 1'b0;
    wire node_60;
    reg t_flip_flop_61_0_q = 1'b0;
    reg t_flip_flop_61_1_q = 1'b0;
    wire node_62;
    wire node_63;
    wire node_64;
    wire node_65;
    wire node_66;
    wire node_67;

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_7_segment_display1_g_middle_3_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock1_1) begin
begin // Synchronous operation
            output_7_segment_display1_g_middle_3_behavioral_reg <= input_push_button2_btn_2;
        end
    end

    assign output_7_segment_display1_g_middle_3 = output_7_segment_display1_g_middle_3_behavioral_reg; // Connect behavioral register to output
    assign output_7_segment_display1_f_upper_left_4 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_e_lower_left_5 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_d_bottom_6 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_a_top_7 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_b_upper_right_8 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_dp_dot_9 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_c_lower_right_10 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_3 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_4 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_5 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_6 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_8 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_9 = node_67; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_10 = 1'b0; // 7-Segment Display

endmodule // display_3bits_counter

// ====================================================================
// Module display_3bits_counter generation completed successfully
// Elements processed: 16
// Inputs: 2, Outputs: 8
// Warnings: 1
//   IC DISPLAY-3BITS output 6 is not connected
// ====================================================================
