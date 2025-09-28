// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: jkflipflop
// Generated: Sun Sep 28 03:36:26 2025
// Target FPGA: Generic-Small
// Resource Usage: 24/1000 LUTs, 35/1000 FFs, 7/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module jkflipflop (
    // ========= Input Ports =========
    input wire input_clock1_c_1,
    input wire input_input_switch2__preset_2,
    input wire input_input_switch3__clear_3,
    input wire input_input_switch4_j_4,
    input wire input_input_switch5_k_5,

    // ========= Output Ports =========
    output wire output_led1_q_0_6,
    output wire output_led2_q_0_7
);

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_led1_q_0_6_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock1_c_1 or negedge input_input_switch2__preset_2) begin
        if (!input_input_switch2__preset_2) begin
            output_led1_q_0_6_behavioral_reg <= 1'b1; // Asynchronous preset
        end else begin // Synchronous operation
            output_led1_q_0_6_behavioral_reg <= input_input_switch2__preset_2;
        end
    end


    // ========= Output Assignments =========
    // ========= Internal Signals =========
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
// ============== END IC: DFLIPFLOP ==============

    // ========= Logic Assignments =========
    assign output_led1_q_0_6 = output_led1_q_0_6_behavioral_reg; // Connect behavioral register to output
    assign output_led2_q_0_7 = ~output_led1_q_0_6_behavioral_reg; // Complementary output


    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 5
    // ULTRATHINK DEBUG: Scanned content size: 195 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // jkflipflop

// ====================================================================
// Module jkflipflop generation completed successfully
// Elements processed: 20
// Inputs: 5, Outputs: 2
// ====================================================================
