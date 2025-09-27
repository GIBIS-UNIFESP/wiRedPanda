// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_masterslave
// Generated: Sat Sep 27 11:33:02 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module dflipflop_masterslave (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_q_0_1,
    output wire output_led2_q_0_2
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

    // ========= Logic Assignments =========
    assign output_led1_q_0_1 = ic_dlatch_ic_node_14_0; // LED
    assign output_led2_q_0_2 = ic_dlatch_ic_node_15_0; // LED

endmodule // dflipflop_masterslave

// ====================================================================
// Module dflipflop_masterslave generation completed successfully
// Elements processed: 9
// Inputs: 0, Outputs: 2
// Warnings: 1
//   IC DLATCH output 1 is not connected
// ====================================================================
