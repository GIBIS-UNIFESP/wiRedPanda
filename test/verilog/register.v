// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: register
// Generated: Sun Sep 28 03:51:22 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 74/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module register (
    // ========= Input Ports =========
    input wire input_clock1_1,
    input wire input_clock2_2,

    // ========= Output Ports =========
    output wire output_led1_0_3,
    output wire output_led2_0_4,
    output wire output_led3_0_5,
    output wire output_led4_0_6
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========
    reg seq_d_flip_flop_7_0_q = 1'b0; // Sequential element register
    reg seq_d_flip_flop_9_0_q = 1'b0; // Sequential element register
    /* verilator lint_off UNUSED */
    reg seq_d_flip_flop_9_1_q = 1'b0; // Sequential element register (complement output)
    /* verilator lint_on UNUSED */
    /* verilator lint_off UNUSED */
    reg seq_d_flip_flop_7_1_q = 1'b0; // Sequential element register (complement output)
    /* verilator lint_on UNUSED */
    /* verilator lint_off UNUSED */
    reg seq_d_flip_flop_8_1_q = 1'b0; // Sequential element register (complement output)
    /* verilator lint_on UNUSED */
    reg seq_d_flip_flop_8_0_q = 1'b0; // Sequential element register
    reg seq_d_flip_flop_10_0_q = 1'b0; // Sequential element register
    /* verilator lint_off UNUSED */
    reg seq_d_flip_flop_10_1_q = 1'b0; // Sequential element register (complement output)
    /* verilator lint_on UNUSED */

    // ========= Logic Assignments =========
    assign output_led1_0_3 = seq_d_flip_flop_7_0_q; // LED
    assign output_led2_0_4 = seq_d_flip_flop_9_0_q; // LED
    assign output_led3_0_5 = seq_d_flip_flop_10_0_q; // LED
    assign output_led4_0_6 = seq_d_flip_flop_8_0_q; // LED
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            seq_d_flip_flop_10_0_q <= seq_d_flip_flop_9_0_q;
            seq_d_flip_flop_10_1_q <= ~seq_d_flip_flop_9_0_q;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            seq_d_flip_flop_9_0_q <= seq_d_flip_flop_8_0_q;
            seq_d_flip_flop_9_1_q <= ~seq_d_flip_flop_8_0_q;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            seq_d_flip_flop_8_0_q <= seq_d_flip_flop_7_0_q;
            seq_d_flip_flop_8_1_q <= ~seq_d_flip_flop_7_0_q;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            seq_d_flip_flop_7_0_q <= input_clock2_2;
            seq_d_flip_flop_7_1_q <= ~input_clock2_2;
        end
    end



    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 10
    // ULTRATHINK DEBUG: Scanned content size: 1144 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // register

// ====================================================================
// Module register generation completed successfully
// Elements processed: 14
// Inputs: 2, Outputs: 4
// ====================================================================
