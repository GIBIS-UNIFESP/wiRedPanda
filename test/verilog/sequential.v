// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: sequential
// Generated: Sun Sep 28 03:51:23 2025
// Target FPGA: Generic-Small
// Resource Usage: 45/1000 LUTs, 74/1000 FFs, 8/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module sequential (
    // ========= Input Ports =========
    input wire input_push_button1_reset_1,
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

    // ========= Internal Sequential Register =========
    reg output_led1_load_shift_0_4_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock2_slow_clk_2) begin
begin // Synchronous operation
            output_led1_load_shift_0_4_behavioral_reg <= input_push_button1_reset_1;
        end
    end


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

    // ========= Logic Assignments =========
    assign output_led1_load_shift_0_4 = output_led1_load_shift_0_4_behavioral_reg; // Connect behavioral register to output
    assign output_led2_l1_0_5 = ~output_led1_load_shift_0_4_behavioral_reg; // Complementary output
    assign output_led3_l3_0_6 = ~output_led1_load_shift_0_4_behavioral_reg; // Complementary output
    assign output_led4_l2_0_7 = ~output_led1_load_shift_0_4_behavioral_reg; // Complementary output
    assign output_led5_l0_0_8 = ~output_led1_load_shift_0_4_behavioral_reg; // Complementary output


    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 3
    // ULTRATHINK DEBUG: Scanned content size: 523 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // sequential

// ====================================================================
// Module sequential generation completed successfully
// Elements processed: 38
// Inputs: 3, Outputs: 5
// ====================================================================
