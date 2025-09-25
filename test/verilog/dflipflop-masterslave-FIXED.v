// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_masterslave_fixed
// Generated: Thu Sep 25 22:55:24 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dflipflop_masterslave_fixed (
    // ========= Input Ports =========
    input wire input_clock1_clk_1,
    input wire input_push_button2_d_2,

    // ========= Output Ports =========
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Internal Signals =========
wire ic_dlatch_dlatch_ic_in_0_5; // IC input 0
assign ic_dlatch_dlatch_ic_in_0_5 = input_push_button2_d_2;
wire ic_dlatch_dlatch_ic_in_1_6; // IC input 1
assign ic_dlatch_dlatch_ic_in_1_6 = input_clock1_clk_1;
wire ic_dlatch_dlatch_ic_out_0_7; // IC output 0
wire ic_dlatch_dlatch_ic_out_1_8; // IC output 1
