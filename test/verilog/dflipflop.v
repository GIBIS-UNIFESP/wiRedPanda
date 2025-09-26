// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop
// Generated: Fri Sep 26 00:05:51 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dflipflop (
    // ========= Input Ports =========
    input wire input_clock1_clk_1,
    input wire input_push_button2_d_2,
    input wire input_input_switch3__preset_3,
    input wire input_input_switch4__clear_4,

    // ========= Output Ports =========
    output wire output_led1_0_5,
    output wire output_led2_0_6
);

    // ========= Internal Signals =========
    wire not_25_0;
    wire not_24_0;
    wire node_23_0;
    wire node_22_0;
    wire node_21_0;
    wire nand_20_0;
    wire not_19_0;
    wire node_18_0;
    wire node_17_0;
    wire nand_13_0;
    wire nand_16_0;
    wire node_15_0;
    wire nand_14_0;
    wire nand_12_0;
    wire node_11_0;
    wire nand_7_0;
    wire nand_10_0;
    wire node_9_0;
    wire nand_8_0;

    // ========= Logic Assignments =========
