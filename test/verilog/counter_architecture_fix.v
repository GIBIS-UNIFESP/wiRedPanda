// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: counter_architecture_fix
// Generated: Sat Sep 27 16:54:38 2025
// Target FPGA: Generic-Small
// Resource Usage: 8/1000 LUTs, 38/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module counter_architecture_fix (
    // ========= Input Ports =========
    input wire input_clock1_1,

    // ========= Output Ports =========
    output wire output_led1_0_2,
    output wire output_led2_0_3,
    output wire output_led3_0_4
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_0_2 = seq_jk_flip_flop_7_1_q; // LED
    assign output_led2_0_3 = seq_jk_flip_flop_6_1_q; // LED
    assign output_led3_0_4 = seq_jk_flip_flop_5_1_q; // LED
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge seq_jk_flip_flop_6_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_7_0_q <= 1'b0; seq_jk_flip_flop_7_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_7_0_q <= 1'b1; seq_jk_flip_flop_7_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_7_0_q <= seq_jk_flip_flop_7_1_q; seq_jk_flip_flop_7_1_q <= seq_jk_flip_flop_7_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge seq_jk_flip_flop_5_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_6_0_q <= 1'b0; seq_jk_flip_flop_6_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_6_0_q <= 1'b1; seq_jk_flip_flop_6_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_6_0_q <= seq_jk_flip_flop_6_1_q; seq_jk_flip_flop_6_1_q <= seq_jk_flip_flop_6_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_5_0_q <= 1'b0; seq_jk_flip_flop_5_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_5_0_q <= 1'b1; seq_jk_flip_flop_5_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_5_0_q <= seq_jk_flip_flop_5_1_q; seq_jk_flip_flop_5_1_q <= seq_jk_flip_flop_5_0_q; end // toggle
            endcase
        end
    end


endmodule // counter_architecture_fix

// ====================================================================
// Module counter_architecture_fix generation completed successfully
// Elements processed: 7
// Inputs: 1, Outputs: 3
// ====================================================================
