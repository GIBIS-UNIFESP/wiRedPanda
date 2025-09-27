// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: serialize
// Generated: Sat Sep 27 18:03:56 2025
// Target FPGA: Generic-Small
// Resource Usage: 45/1000 LUTs, 4/1000 FFs, 8/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module serialize (
    // ========= Input Ports =========
    input wire input_input_switch1_load__shift_1,
    input wire input_input_switch2_clock_2,
    input wire input_input_switch3_d0_3,
    input wire input_input_switch4_d1_4,
    input wire input_input_switch5_d2_5,
    input wire input_input_switch6_d3_6,

    // ========= Output Ports =========
    output wire output_led1_0_7,
    output wire output_led2_0_8
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_0_7 = ~input_input_switch1_load__shift_1; // LED
    assign output_led2_0_8 = (input_input_switch1_load__shift_1 & seq_d_flip_flop_12_0_q); // LED
    wire seq_d_flip_flop_12_0_q_clk_wire;
    assign seq_d_flip_flop_12_0_q_clk_wire = ~input_input_switch2_clock_2; // Clock expression wire
    // D FlipFlop: D-Flip-Flop
    always @(posedge seq_d_flip_flop_12_0_q_clk_wire) begin
        begin
            seq_d_flip_flop_12_0_q <= ((~input_input_switch1_load__shift_1 & ~input_input_switch6_d3_6) | (input_input_switch1_load__shift_1 & seq_d_flip_flop_11_0_q));
            seq_d_flip_flop_12_1_q <= ~((~input_input_switch1_load__shift_1 & ~input_input_switch6_d3_6) | (input_input_switch1_load__shift_1 & seq_d_flip_flop_11_0_q));
        end
    end

    wire seq_d_flip_flop_11_0_q_clk_wire;
    assign seq_d_flip_flop_11_0_q_clk_wire = ~input_input_switch2_clock_2; // Clock expression wire
    // D FlipFlop: D-Flip-Flop
    always @(posedge seq_d_flip_flop_11_0_q_clk_wire) begin
        begin
            seq_d_flip_flop_11_0_q <= ((~input_input_switch1_load__shift_1 & ~input_input_switch5_d2_5) | (input_input_switch1_load__shift_1 & seq_d_flip_flop_10_0_q));
            seq_d_flip_flop_11_1_q <= ~((~input_input_switch1_load__shift_1 & ~input_input_switch5_d2_5) | (input_input_switch1_load__shift_1 & seq_d_flip_flop_10_0_q));
        end
    end

    wire seq_d_flip_flop_10_0_q_clk_wire;
    assign seq_d_flip_flop_10_0_q_clk_wire = ~input_input_switch2_clock_2; // Clock expression wire
    // D FlipFlop: D-Flip-Flop
    always @(posedge seq_d_flip_flop_10_0_q_clk_wire) begin
        begin
            seq_d_flip_flop_10_0_q <= ((~input_input_switch1_load__shift_1 & ~input_input_switch4_d1_4) | (input_input_switch1_load__shift_1 & seq_d_flip_flop_9_0_q));
            seq_d_flip_flop_10_1_q <= ~((~input_input_switch1_load__shift_1 & ~input_input_switch4_d1_4) | (input_input_switch1_load__shift_1 & seq_d_flip_flop_9_0_q));
        end
    end

    wire seq_d_flip_flop_9_0_q_clk_wire;
    assign seq_d_flip_flop_9_0_q_clk_wire = ~input_input_switch2_clock_2; // Clock expression wire
    // D FlipFlop: D-Flip-Flop
    always @(posedge seq_d_flip_flop_9_0_q_clk_wire) begin
        begin
            seq_d_flip_flop_9_0_q <= ~input_input_switch3_d0_3;
            seq_d_flip_flop_9_1_q <= ~~input_input_switch3_d0_3;
        end
    end


endmodule // serialize

// ====================================================================
// Module serialize generation completed successfully
// Elements processed: 38
// Inputs: 6, Outputs: 2
// ====================================================================
