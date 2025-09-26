// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_masterslave
// Generated: Fri Sep 26 00:19:55 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dflipflop_masterslave (
    // ========= Input Ports =========
    input wire input_clock1_clk_1,
    input wire input_push_button2_d_2,

    // ========= Output Ports =========
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Internal Signals =========
wire ic_dlatch_dlatch_ic_in_0_8; // IC input 0
assign ic_dlatch_dlatch_ic_in_0_8 = input_push_button2_d_2;
wire ic_dlatch_dlatch_ic_in_1_9; // IC input 1
assign ic_dlatch_dlatch_ic_in_1_9 = input_clock1_clk_1;
wire ic_dlatch_dlatch_ic_out_0_10; // IC output 0
wire ic_dlatch_dlatch_ic_out_1_11; // IC output 1
