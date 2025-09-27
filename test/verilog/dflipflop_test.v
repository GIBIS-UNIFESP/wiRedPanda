// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_test
// Generated: Sat Sep 27 11:32:16 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module dflipflop_test (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2
);

    // ========= Logic Assignments =========

    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_0_1 = ~(1'b0 & (1'b0 & ~(1'b0 & ~1'b0) & ~(1'b0 & 1'b0 & ~1'b0 & 1'b0)) & 1'b0 & ~(1'b0 & ~(1'b0 & (1'b0 & ~(1'b0 & ~1'b0) & 1'b0) & 1'b0 & ~1'b0 & 1'b0) & 1'b0)); // LED
    assign output_led2_0_2 = (1'b0 & (1'b0 & ~(1'b0 & ~1'b0) & ~(1'b0 & 1'b0 & ~1'b0 & 1'b0)) & 1'b0 & 1'b0) & ~(1'b0 & (1'b0 & ~(1'b0 & ~1'b0) & 1'b0) & 1'b0 & ~1'b0 & 1'b0) & 1'b0; // LED

endmodule // dflipflop_test

// ====================================================================
// Module dflipflop_test generation completed successfully
// Elements processed: 25
// Inputs: 0, Outputs: 2
// ====================================================================
