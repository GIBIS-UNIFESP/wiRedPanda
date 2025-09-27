// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: jkflipflop
// Generated: Sat Sep 27 11:33:27 2025
// Target FPGA: Generic-Small
// Resource Usage: 24/1000 LUTs, 35/1000 FFs, 7/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module jkflipflop (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_q_0_1,
    output wire output_led2_q_0_2
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
// ============== END IC: DFLIPFLOP ==============

    // ========= Logic Assignments =========
    assign output_led1_q_0_1 = ic_dflipflop_ic_node_34_0; // LED
    assign output_led2_q_0_2 = ic_dflipflop_ic_node_35_0; // LED

endmodule // jkflipflop

// ====================================================================
// Module jkflipflop generation completed successfully
// Elements processed: 20
// Inputs: 0, Outputs: 2
// ====================================================================
