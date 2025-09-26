// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits_counter
// Generated: Fri Sep 26 00:41:59 2025
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
wire ic_display_3bits_display_3bits_ic_in_0_23; // IC input 0
assign ic_display_3bits_display_3bits_ic_in_0_23 = t_flip_flop_17_1_1;
wire ic_display_3bits_display_3bits_ic_in_1_24; // IC input 1
assign ic_display_3bits_display_3bits_ic_in_1_24 = t_flip_flop_19_1_1;
wire ic_display_3bits_display_3bits_ic_in_2_25; // IC input 2
assign ic_display_3bits_display_3bits_ic_in_2_25 = t_flip_flop_21_1_1;
wire ic_display_3bits_display_3bits_ic_out_0_26; // IC output 0
wire ic_display_3bits_display_3bits_ic_out_1_27; // IC output 1
wire ic_display_3bits_display_3bits_ic_out_2_28; // IC output 2
wire ic_display_3bits_display_3bits_ic_out_3_29; // IC output 3
wire ic_display_3bits_display_3bits_ic_out_4_30; // IC output 4
wire ic_display_3bits_display_3bits_ic_out_5_31; // IC output 5
wire ic_display_3bits_display_3bits_ic_out_6_32; // IC output 6
wire ic_display_3bits_display_3bits_ic_out_7_33; // IC output 7
    wire node_22_0;
    reg t_flip_flop_21_0_0 = 1'b0;
    reg t_flip_flop_21_1_1 = 1'b0;
    wire node_20_0;
    reg t_flip_flop_19_0_0 = 1'b0;
    reg t_flip_flop_19_1_1 = 1'b0;
    wire node_18_0;
    reg t_flip_flop_17_0_0 = 1'b0;
    reg t_flip_flop_17_1_1 = 1'b0;
    wire node_16_0;
    wire node_15_0;
    wire node_14_0;
    wire node_13_0;
    wire node_12_0;
    wire node_11_0;

    // ========= Logic Assignments =========
    assign node_11_0 = input_clock1_1; // Node
    assign node_12_0 = t_flip_flop_19_1_1; // Node
    assign node_13_0 = t_flip_flop_21_1_1; // Node
    assign node_14_0 = input_clock1_1; // Node
    assign node_15_0 = t_flip_flop_17_1_1; // Node
    assign node_16_0 = t_flip_flop_17_1_1; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge t_flip_flop_19_1_1) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_17_0_0 <= t_flip_flop_17_1_1;
                t_flip_flop_17_1_1 <= t_flip_flop_17_0_0;
            end
            // else hold
        end
    end

    assign node_18_0 = input_push_button2_btn_2; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge t_flip_flop_21_1_1) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_19_0_0 <= t_flip_flop_19_1_1;
                t_flip_flop_19_1_1 <= t_flip_flop_19_0_0;
            end
            // else hold
        end
    end

    assign node_20_0 = input_push_button2_btn_2; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_21_0_0 <= t_flip_flop_21_1_1;
                t_flip_flop_21_1_1 <= t_flip_flop_21_0_0;
            end
            // else hold
        end
    end

    assign node_22_0 = input_clock1_1; // Node

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_3 = ic_display_3bits_display_3bits_ic_out_0_26; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_4 = ic_display_3bits_display_3bits_ic_out_1_27; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_5 = ic_display_3bits_display_3bits_ic_out_2_28; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_6 = ic_display_3bits_display_3bits_ic_out_3_29; // 7-Segment Display
    assign output_7_segment_display1_a_top_7 = ic_display_3bits_display_3bits_ic_out_4_30; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_8 = ic_display_3bits_display_3bits_ic_out_5_31; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_9 = input_clock1_1; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_10 = ic_display_3bits_display_3bits_ic_out_7_33; // 7-Segment Display

endmodule // display_3bits_counter

// ====================================================================
// Module display_3bits_counter generation completed successfully
// Elements processed: 16
// Inputs: 2, Outputs: 8
// Warnings: 1
//   IC DISPLAY-3BITS output 6 is not connected
// ====================================================================
