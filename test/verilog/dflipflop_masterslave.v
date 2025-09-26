// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_masterslave
// Generated: Fri Sep 26 21:32:26 2025
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

    // ========= Internal Signals =========
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
    wire ic_dlatch_ic_node_7_0;
    wire ic_dlatch_ic_not_8_0;
    wire ic_dlatch_ic_node_9_0;
    wire ic_dlatch_ic_nand_10_0;
    wire ic_dlatch_ic_nand_11_0;
    wire ic_dlatch_ic_nand_12_0;
    wire ic_dlatch_ic_nand_13_0;
    wire ic_dlatch_ic_node_14_0;
    wire ic_dlatch_ic_node_15_0;
// ============== END IC: DLATCH ==============
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
    wire ic_dlatch_ic_node_20_0;
    wire ic_dlatch_ic_not_21_0;
    wire ic_dlatch_ic_node_22_0;
    wire ic_dlatch_ic_nand_23_0;
    wire ic_dlatch_ic_nand_24_0;
    wire ic_dlatch_ic_nand_25_0;
    wire ic_dlatch_ic_nand_26_0;
    wire ic_dlatch_ic_node_27_0;
    wire ic_dlatch_ic_node_28_0;
// ============== END IC: DLATCH ==============
    wire not_29;
    wire node_30;
    wire node_31;

    // ========= Logic Assignments =========
    wire ic_dlatch_ic_node_32_0;
    wire ic_dlatch_ic_not_33_0;
    wire ic_dlatch_ic_node_34_0;
    wire ic_dlatch_ic_nand_35_0;
    wire ic_dlatch_ic_nand_36_0;
    wire ic_dlatch_ic_nand_37_0;
    wire ic_dlatch_ic_nand_38_0;
    wire ic_dlatch_ic_node_39_0;
    wire ic_dlatch_ic_node_40_0;
    assign node_31 = not_29; // Node
    wire ic_dlatch_ic_node_41_0;
    wire ic_dlatch_ic_not_42_0;
    wire ic_dlatch_ic_node_43_0;
    wire ic_dlatch_ic_nand_44_0;
    wire ic_dlatch_ic_nand_45_0;
    wire ic_dlatch_ic_nand_46_0;
    wire ic_dlatch_ic_nand_47_0;
    wire ic_dlatch_ic_node_48_0;
    wire ic_dlatch_ic_node_49_0;
    assign node_30 = 1'b0; // Node
    assign not_29 = ~1'b0; // Not

    // ========= Output Assignments =========
    assign output_led1_q_0_1 = ic_dlatch_ic_node_39_0; // LED
    assign output_led2_q_0_2 = ic_dlatch_ic_node_40_0; // LED

endmodule // dflipflop_masterslave

// ====================================================================
// Module dflipflop_masterslave generation completed successfully
// Elements processed: 9
// Inputs: 0, Outputs: 2
// Warnings: 1
//   IC DLATCH output 1 is not connected
// ====================================================================
