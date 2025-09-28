// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: sequential
// Generated: Sun Sep 28 01:08:19 2025
// Target FPGA: Generic-Small
// Resource Usage: 45/1000 LUTs, 74/1000 FFs, 8/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module sequential (
    // ========= Input Ports =========
    input wire input_clock2_slow_clk_2,
    input wire input_clock3_fast_clk_3,

    // ========= Output Ports =========
    output wire output_led1_load_shift_0_4,
    output wire output_led2_l1_0_5,
    output wire output_led3_l3_0_6,
    output wire output_led4_l2_0_7,
    output wire output_led5_l0_0_8
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========
// ============== BEGIN IC: SERIALIZE ==============
// IC inputs: 6, IC outputs: 2
// Nesting depth: 0
// ============== END IC: SERIALIZE ==============
// ============== BEGIN IC: REGISTER ==============
// IC inputs: 2, IC outputs: 4
// Nesting depth: 0
// ============== END IC: REGISTER ==============
    reg seq_jk_flip_flop_12_1_q = 1'b0; // Sequential element register
    reg seq_jk_flip_flop_11_1_q = 1'b0; // Sequential element register
    reg seq_jk_flip_flop_9_0_q = 1'b0; // Sequential element register
    reg seq_jk_flip_flop_10_0_q = 1'b0; // Sequential element register
    reg seq_jk_flip_flop_10_1_q = 1'b0; // Sequential element register
    reg seq_jk_flip_flop_12_0_q = 1'b0; // Sequential element register
    reg seq_jk_flip_flop_9_1_q = 1'b0; // Sequential element register
    reg seq_jk_flip_flop_11_0_q = 1'b0; // Sequential element register

    // ========= Logic Assignments =========
    assign output_led1_load_shift_0_4 = input_clock2_slow_clk_2; // LED
    assign output_led2_l1_0_5 = (1'b0 & input_clock2_slow_clk_2); // LED
    assign output_led3_l3_0_6 = (1'b0 & input_clock2_slow_clk_2); // LED
    assign output_led4_l2_0_7 = (1'b0 & input_clock2_slow_clk_2); // LED
    assign output_led5_l0_0_8 = (1'b0 & input_clock2_slow_clk_2); // LED
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge seq_jk_flip_flop_11_0_q) begin
        if (1'b0) begin
            seq_jk_flip_flop_12_0_q <= 1'b1;
            seq_jk_flip_flop_12_1_q <= 1'b0;
        end else begin
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
        if (1'b0) begin
            seq_jk_flip_flop_11_0_q <= 1'b1;
            seq_jk_flip_flop_11_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_11_0_q <= 1'b0; seq_jk_flip_flop_11_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_11_0_q <= 1'b1; seq_jk_flip_flop_11_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_11_0_q <= seq_jk_flip_flop_11_1_q; seq_jk_flip_flop_11_1_q <= seq_jk_flip_flop_11_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge seq_jk_flip_flop_9_0_q) begin
        if (1'b0) begin
            seq_jk_flip_flop_10_0_q <= 1'b1;
            seq_jk_flip_flop_10_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_10_0_q <= 1'b0; seq_jk_flip_flop_10_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_10_0_q <= 1'b1; seq_jk_flip_flop_10_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_10_0_q <= seq_jk_flip_flop_10_1_q; seq_jk_flip_flop_10_1_q <= seq_jk_flip_flop_10_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock2_slow_clk_2) begin
        if (1'b0) begin
            seq_jk_flip_flop_9_0_q <= 1'b1;
            seq_jk_flip_flop_9_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin seq_jk_flip_flop_9_0_q <= 1'b0; seq_jk_flip_flop_9_1_q <= 1'b1; end
                2'b10: begin seq_jk_flip_flop_9_0_q <= 1'b1; seq_jk_flip_flop_9_1_q <= 1'b0; end
                2'b11: begin seq_jk_flip_flop_9_0_q <= seq_jk_flip_flop_9_1_q; seq_jk_flip_flop_9_1_q <= seq_jk_flip_flop_9_0_q; end // toggle
            endcase
        end
    end



    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 10
    // ULTRATHINK DEBUG: Scanned content size: 3152 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // sequential

// ====================================================================
// Module sequential generation completed successfully
// Elements processed: 38
// Inputs: 2, Outputs: 5
// ====================================================================
