// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_masterslave
// Generated: Sun Sep 28 03:51:11 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module dflipflop_masterslave (
    // ========= Input Ports =========
    input wire input_clock1_clk_1,

    // ========= Output Ports =========
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
// ============== END IC: DLATCH ==============
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
// ============== END IC: DLATCH ==============
    wire ic_dlatch_ic_node_16_0 = 1'b0; // Auto-declared and assigned default for referenced IC node
    wire ic_dlatch_ic_node_17_0 = 1'b0; // Auto-declared and assigned default for referenced IC node

    // ========= Logic Assignments =========
    assign output_led1_q_0_3 = ic_dlatch_ic_node_16_0; // LED
    assign output_led2_q_0_4 = ic_dlatch_ic_node_17_0; // LED


    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 1
    // ULTRATHINK DEBUG: Scanned content size: 123 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // dflipflop_masterslave

// ====================================================================
// Module dflipflop_masterslave generation completed successfully
// Elements processed: 9
// Inputs: 1, Outputs: 2
// Warnings: 1
//   IC DLATCH output 1 is not connected
// ====================================================================
