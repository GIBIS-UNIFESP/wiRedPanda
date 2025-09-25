// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits_counter
// Generated: Thu Sep 25 21:23:05 2025
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
wire ic_display_3bits_display_3bits_ic_in_0_11; // IC input 0
assign ic_display_3bits_display_3bits_ic_in_0_11 = 1'b1;
wire ic_display_3bits_display_3bits_ic_in_1_12; // IC input 1
assign ic_display_3bits_display_3bits_ic_in_1_12 = 1'b1;
wire ic_display_3bits_display_3bits_ic_in_2_13; // IC input 2
assign ic_display_3bits_display_3bits_ic_in_2_13 = 1'b1;
wire ic_display_3bits_display_3bits_ic_out_0_14; // IC output 0
wire ic_display_3bits_display_3bits_ic_out_1_15; // IC output 1
wire ic_display_3bits_display_3bits_ic_out_2_16; // IC output 2
wire ic_display_3bits_display_3bits_ic_out_3_17; // IC output 3
wire ic_display_3bits_display_3bits_ic_out_4_18; // IC output 4
wire ic_display_3bits_display_3bits_ic_out_5_19; // IC output 5
wire ic_display_3bits_display_3bits_ic_out_6_20; // IC output 6
wire ic_display_3bits_display_3bits_ic_out_7_21; // IC output 7
    wire node_22;
    reg t_flip_flop_23_0_q = 1'b0;
    reg t_flip_flop_23_1_q = 1'b0;
    wire node_24;
    reg t_flip_flop_25_0_q = 1'b0;
    reg t_flip_flop_25_1_q = 1'b0;
    wire node_26;
    reg t_flip_flop_27_0_q = 1'b0;
    reg t_flip_flop_27_1_q = 1'b0;
    wire node_28;
    wire node_29;
    wire node_30;
    wire node_31;
    wire node_32;
    wire node_33;

    // ========= Logic Assignments =========
    assign node_33 = input_clock1_1; // Node
    assign node_32 = t_flip_flop_25_1_q; // Node
    assign node_31 = t_flip_flop_23_1_q; // Node
    assign node_30 = input_clock1_1; // Node
    assign node_29 = t_flip_flop_27_1_q; // Node
    assign node_28 = t_flip_flop_27_1_q; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge t_flip_flop_25_1_q) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_27_0_q <= t_flip_flop_27_1_q;
                t_flip_flop_27_1_q <= t_flip_flop_27_0_q;
            end
            // else hold
        end
    end

    assign node_26 = input_push_button2_btn_2; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge t_flip_flop_23_1_q) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_25_0_q <= t_flip_flop_25_1_q;
                t_flip_flop_25_1_q <= t_flip_flop_25_0_q;
            end
            // else hold
        end
    end

    assign node_24 = input_push_button2_btn_2; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_23_0_q <= t_flip_flop_23_1_q;
                t_flip_flop_23_1_q <= t_flip_flop_23_0_q;
            end
            // else hold
        end
    end

    assign node_22 = input_clock1_1; // Node

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_3 = ic_display_3bits_display_3bits_ic_out_0_14; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_4 = ic_display_3bits_display_3bits_ic_out_1_15; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_5 = ic_display_3bits_display_3bits_ic_out_2_16; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_6 = ic_display_3bits_display_3bits_ic_out_3_17; // 7-Segment Display
    assign output_7_segment_display1_a_top_7 = ic_display_3bits_display_3bits_ic_out_4_18; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_8 = ic_display_3bits_display_3bits_ic_out_5_19; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_9 = input_clock1_1; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_10 = ic_display_3bits_display_3bits_ic_out_7_21; // 7-Segment Display

endmodule // display_3bits_counter

// ====================================================================
// Module display_3bits_counter generation completed successfully
// Elements processed: 16
// Inputs: 2, Outputs: 8
// Warnings: 1
//   IC DISPLAY-3BITS output 6 is not connected
// ====================================================================
