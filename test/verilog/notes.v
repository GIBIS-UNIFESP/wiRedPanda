// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: notes
// Generated: Sat Sep 27 18:03:54 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 38/1000 FFs, 9/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module notes (
    // ========= Input Ports =========
    input wire input_clock1_1,

    // ========= Output Ports =========
    output wire output_buzzer1_g6_2,
    output wire output_buzzer2_f6_3,
    output wire output_buzzer3_d6_4,
    output wire output_buzzer4_b7_5,
    output wire output_buzzer5_c6_6,
    output wire output_buzzer6_a7_7,
    output wire output_buzzer7_e6_8,
    output wire output_buzzer8_c7_9
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: DECODER ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
// ============== END IC: DECODER ==============

    // ========= Logic Assignments =========
    assign output_buzzer1_g6_2 = 1'b0; // Buzzer
    assign output_buzzer2_f6_3 = 1'b0; // Buzzer
    assign output_buzzer3_d6_4 = 1'b0; // Buzzer
    assign output_buzzer4_b7_5 = 1'b0; // Buzzer
    assign output_buzzer5_c6_6 = 1'b0; // Buzzer
    assign output_buzzer6_a7_7 = 1'b0; // Buzzer
    assign output_buzzer7_e6_8 = 1'b0; // Buzzer
    assign output_buzzer8_c7_9 = 1'b0; // Buzzer
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge seq_jk_flip_flop_11_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_12_0_q <= 1'b0; seq_jk_flip_flop_12_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_12_0_q <= 1'b1; seq_jk_flip_flop_12_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_12_0_q <= seq_jk_flip_flop_12_1_q; seq_jk_flip_flop_12_1_q <= seq_jk_flip_flop_12_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge seq_jk_flip_flop_10_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_11_0_q <= 1'b0; seq_jk_flip_flop_11_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_11_0_q <= 1'b1; seq_jk_flip_flop_11_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_11_0_q <= seq_jk_flip_flop_11_1_q; seq_jk_flip_flop_11_1_q <= seq_jk_flip_flop_11_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        seq_jk_flip_flop_10_0_q = 1'b0;
        seq_jk_flip_flop_10_1_q = 1'b1;
    end


endmodule // notes

// ====================================================================
// Module notes generation completed successfully
// Elements processed: 14
// Inputs: 1, Outputs: 8
// Warnings: 5
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 4 is not connected
//   IC JKFLIPFLOP output 1 is not connected
// ====================================================================
