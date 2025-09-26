// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: tflipflop
// Generated: Fri Sep 26 21:22:30 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module tflipflop (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_q_0_1,
    output wire output_led2_q_0_2
);

    // ========= Internal Signals =========
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
    wire ic_dflipflop_ic_node_11_0;
    wire ic_dflipflop_ic_not_12_0;
    wire ic_dflipflop_ic_not_13_0;
    wire ic_dflipflop_ic_node_14_0;
    wire ic_dflipflop_ic_node_15_0;
    wire ic_dflipflop_ic_node_16_0;
    wire ic_dflipflop_ic_node_17_0;
    wire ic_dflipflop_ic_nand_18_0;
    wire ic_dflipflop_ic_not_19_0;
    wire ic_dflipflop_ic_node_20_0;
    wire ic_dflipflop_ic_node_21_0;
    wire ic_dflipflop_ic_node_22_0;
    wire ic_dflipflop_ic_node_23_0;
    wire ic_dflipflop_ic_nand_24_0;
    wire ic_dflipflop_ic_nand_25_0;
    wire ic_dflipflop_ic_node_26_0;
    wire ic_dflipflop_ic_nand_27_0;
    wire ic_dflipflop_ic_nand_28_0;
    wire ic_dflipflop_ic_node_29_0;
    wire ic_dflipflop_ic_nand_30_0;
    wire ic_dflipflop_ic_nand_31_0;
    wire ic_dflipflop_ic_node_32_0;
    wire ic_dflipflop_ic_nand_33_0;
    wire ic_dflipflop_ic_node_34_0;
    wire ic_dflipflop_ic_node_35_0;
// ============== END IC: DFLIPFLOP ==============
    wire node_36;
    wire node_37;
    wire not_38;
    wire and_39;
    wire or_40;
    wire node_41;
    wire and_42;

    // ========= Logic Assignments =========
    wire ic_dflipflop_ic_node_43_0;
    wire ic_dflipflop_ic_not_44_0;
    wire ic_dflipflop_ic_not_45_0;
    wire ic_dflipflop_ic_node_46_0;
    wire ic_dflipflop_ic_node_47_0;
    wire ic_dflipflop_ic_node_48_0;
    wire ic_dflipflop_ic_node_49_0;
    wire ic_dflipflop_ic_nand_50_0;
    wire ic_dflipflop_ic_not_51_0;
    wire ic_dflipflop_ic_node_52_0;
    wire ic_dflipflop_ic_node_53_0;
    wire ic_dflipflop_ic_node_54_0;
    wire ic_dflipflop_ic_node_55_0;
    wire ic_dflipflop_ic_nand_56_0;
    wire ic_dflipflop_ic_nand_57_0;
    wire ic_dflipflop_ic_node_58_0;
    wire ic_dflipflop_ic_nand_59_0;
    wire ic_dflipflop_ic_nand_60_0;
    wire ic_dflipflop_ic_node_61_0;
    wire ic_dflipflop_ic_nand_62_0;
    wire ic_dflipflop_ic_nand_63_0;
    wire ic_dflipflop_ic_node_64_0;
    wire ic_dflipflop_ic_nand_65_0;
    wire ic_dflipflop_ic_node_66_0;
    wire ic_dflipflop_ic_node_67_0;
    assign or_40 = (and_39 | and_42); // Or
    assign and_42 = (node_41 & ic_dflipflop_ic_node_67_0); // And
    assign node_41 = 1'b0; // Node
    assign and_39 = (node_37 & not_38); // And
    assign not_38 = ~1'b0; // Not
    assign node_37 = node_36; // Node

    // ========= Output Assignments =========
    assign output_led1_q_0_1 = ic_dflipflop_ic_node_66_0; // LED
    assign output_led2_q_0_2 = ic_dflipflop_ic_node_67_0; // LED

endmodule // tflipflop

// ====================================================================
// Module tflipflop generation completed successfully
// Elements processed: 14
// Inputs: 0, Outputs: 2
// ====================================================================
