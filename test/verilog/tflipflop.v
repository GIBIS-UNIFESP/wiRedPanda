// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: tflipflop
// Generated: Fri Sep 26 00:06:03 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module tflipflop (
    // ========= Input Ports =========
    input wire input_push_button1_t_1,
    input wire input_clock2_c_2,
    input wire input_input_switch3__preset_3,
    input wire input_input_switch4__clear_4,

    // ========= Output Ports =========
    output wire output_led1_q_0_5,
    output wire output_led2_q_0_6
);

    // ========= Internal Signals =========
wire ic_dflipflop_dflipflop_ic_in_0_14; // IC input 0
assign ic_dflipflop_dflipflop_ic_in_0_14 = input_input_switch3__preset_3;
wire ic_dflipflop_dflipflop_ic_in_1_15; // IC input 1
assign ic_dflipflop_dflipflop_ic_in_1_15 = ((1'b0 & ~input_push_button1_t_1) | (input_push_button1_t_1 & 1'b0));
wire ic_dflipflop_dflipflop_ic_in_2_16; // IC input 2
assign ic_dflipflop_dflipflop_ic_in_2_16 = input_clock2_c_2;
wire ic_dflipflop_dflipflop_ic_in_3_17; // IC input 3
assign ic_dflipflop_dflipflop_ic_in_3_17 = input_input_switch4__clear_4;
wire ic_dflipflop_dflipflop_ic_out_0_18; // IC output 0
wire ic_dflipflop_dflipflop_ic_out_1_19; // IC output 1
