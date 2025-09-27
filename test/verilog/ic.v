// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: ic
// Generated: Sat Sep 27 18:03:52 2025
// Target FPGA: Generic-Small
// Resource Usage: 58/1000 LUTs, 215/1000 FFs, 37/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

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

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: INPUT ==============
// IC inputs: 2, IC outputs: 4
// Nesting depth: 0
// ============== END IC: INPUT ==============
// ============== BEGIN IC: DISPLAY-4BITS ==============
// IC inputs: 4, IC outputs: 8
// Nesting depth: 0
// ============== END IC: DISPLAY-4BITS ==============
// ============== BEGIN IC: DISPLAY-4BITS ==============
// IC inputs: 4, IC outputs: 8
// Nesting depth: 0
// ============== END IC: DISPLAY-4BITS ==============
// ============== BEGIN IC: DISPLAY-3BITS ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
// ============== END IC: DISPLAY-3BITS ==============
    wire ic_input_ic_node_267_0 = 1'b0; // Auto-declared and assigned default for referenced IC node
    wire ic_input_ic_node_265_0 = 1'b0; // Auto-declared and assigned default for referenced IC node
    wire ic_input_ic_node_266_0 = 1'b0; // Auto-declared and assigned default for referenced IC node

    // ========= Logic Assignments =========
    assign output_7_segment_display1_g_middle_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_8 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_9 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_10 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_11 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_12 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_13 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_14 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_g_middle_15 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_f_upper_left_16 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_e_lower_left_17 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_d_bottom_18 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_a_top_19 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_b_upper_right_20 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_dp_dot_21 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_c_lower_right_22 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_g_middle_23 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_f_upper_left_24 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_e_lower_left_25 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_d_bottom_26 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_a_top_27 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_b_upper_right_28 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_dp_dot_29 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_c_lower_right_30 = 1'b0; // 7-Segment Display
    assign output_led4_0_31 = ic_input_ic_node_266_0; // LED
    assign output_led5_0_32 = ic_input_ic_node_265_0; // LED
    assign output_led6_0_33 = ic_input_ic_node_267_0; // LED
    assign output_led7_0_34 = seq_jk_flip_flop_38_0_q; // LED
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock6_6) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_38_0_q <= 1'b0; seq_jk_flip_flop_38_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_38_0_q <= 1'b1; seq_jk_flip_flop_38_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_38_0_q <= seq_jk_flip_flop_38_1_q; seq_jk_flip_flop_38_1_q <= seq_jk_flip_flop_38_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge seq_jk_flip_flop_36_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_37_0_q <= 1'b0; seq_jk_flip_flop_37_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_37_0_q <= 1'b1; seq_jk_flip_flop_37_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_37_0_q <= seq_jk_flip_flop_37_1_q; seq_jk_flip_flop_37_1_q <= seq_jk_flip_flop_37_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge seq_jk_flip_flop_35_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_36_0_q <= 1'b0; seq_jk_flip_flop_36_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_36_0_q <= 1'b1; seq_jk_flip_flop_36_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_36_0_q <= seq_jk_flip_flop_36_1_q; seq_jk_flip_flop_36_1_q <= seq_jk_flip_flop_36_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        seq_jk_flip_flop_35_0_q = 1'b0;
        seq_jk_flip_flop_35_1_q = 1'b1;
    end


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
