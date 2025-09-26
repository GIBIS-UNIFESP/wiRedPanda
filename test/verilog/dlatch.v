// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dlatch
// Generated: Fri Sep 26 00:20:02 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dlatch (
    // ========= Input Ports =========
    input wire input_push_button1_d_1,
    input wire input_clock2_clk_2,

    // ========= Output Ports =========
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Internal Signals =========
    wire not_9_0;
    wire nand_8_0;
    wire nand_5_0;
    wire nand_7_0;
    wire nand_6_0;

    // ========= Logic Assignments =========
