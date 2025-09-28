// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: tflipflop
// Generated: Sun Sep 28 03:03:45 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module tflipflop (
    // ========= Input Ports =========
    input wire input_push_button1_t_1,

    // ========= Output Ports =========
    output wire output_led1_q_0_5,
    output wire output_led2_q_0_6
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
// ============== END IC: DFLIPFLOP ==============
    wire ic_dflipflop_ic_node_39_0 = 1'b0; // Auto-declared and assigned default for referenced IC node
    wire ic_dflipflop_ic_node_38_0 = 1'b0; // Auto-declared and assigned default for referenced IC node

    // ========= Logic Assignments =========
    assign output_led1_q_0_5 = ic_dflipflop_ic_node_38_0; // LED
    assign output_led2_q_0_6 = ic_dflipflop_ic_node_39_0; // LED


    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 1
    // ULTRATHINK DEBUG: Scanned content size: 129 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // tflipflop

// ====================================================================
// Module tflipflop generation completed successfully
// Elements processed: 14
// Inputs: 1, Outputs: 2
// ====================================================================
