// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: jkflipflop
// Generated: Fri Sep 26 00:42:04 2025
// Target FPGA: Generic-Small
// Resource Usage: 24/1000 LUTs, 35/1000 FFs, 7/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module jkflipflop (
    // ========= Input Ports =========
    input wire input_clock1_c_1,
    input wire input_input_switch2__preset_2,
    input wire input_input_switch3__clear_3,
    input wire input_input_switch4_j_4,
    input wire input_input_switch5_k_5,

    // ========= Output Ports =========
    output wire output_led1_q_0_6,
    output wire output_led2_q_0_7
);

    // ========= Internal Signals =========
wire ic_dflipflop_dflipflop_ic_in_0_20; // IC input 0
assign ic_dflipflop_dflipflop_ic_in_0_20 = input_input_switch2__preset_2;
wire ic_dflipflop_dflipflop_ic_in_1_21; // IC input 1
assign ic_dflipflop_dflipflop_ic_in_1_21 = ((1'b0 & ~input_input_switch4_j_4) | (input_input_switch5_k_5 & 1'b0));
wire ic_dflipflop_dflipflop_ic_in_2_22; // IC input 2
assign ic_dflipflop_dflipflop_ic_in_2_22 = input_clock1_c_1;
wire ic_dflipflop_dflipflop_ic_in_3_23; // IC input 3
assign ic_dflipflop_dflipflop_ic_in_3_23 = input_input_switch3__clear_3;
wire ic_dflipflop_dflipflop_ic_out_0_24; // IC output 0
wire ic_dflipflop_dflipflop_ic_out_1_25; // IC output 1
    wire node_19_0;
    wire node_18_0;
    wire node_17_0;
    wire node_11_0;
    wire node_16_0;
    wire node_15_0;
    wire node_10_0;
    wire not_14_0;
    wire node_12_0;
    wire and_9_0;
    wire and_13_0;
    wire or_8_0;

    // ========= Logic Assignments =========
    assign or_8_0 = ((ic_dflipflop_dflipflop_ic_out_0_24 & ~input_input_switch4_j_4) | (input_input_switch5_k_5 & ic_dflipflop_dflipflop_ic_out_1_25)); // Or
    assign and_9_0 = (input_input_switch5_k_5 & ic_dflipflop_dflipflop_ic_out_1_25); // And
    assign node_10_0 = ic_dflipflop_dflipflop_ic_out_1_25; // Node
    assign node_11_0 = ic_dflipflop_dflipflop_ic_out_1_25; // Node
    assign node_12_0 = input_input_switch5_k_5; // Node
    assign and_13_0 = (ic_dflipflop_dflipflop_ic_out_0_24 & ~input_input_switch4_j_4); // And
    assign not_14_0 = ~input_input_switch4_j_4; // Not
    assign node_15_0 = ic_dflipflop_dflipflop_ic_out_0_24; // Node
    assign node_16_0 = ic_dflipflop_dflipflop_ic_out_0_24; // Node
    assign node_17_0 = input_clock1_c_1; // Node
    assign node_18_0 = input_input_switch3__clear_3; // Node
    assign node_19_0 = input_input_switch2__preset_2; // Node

    // ========= Output Assignments =========
    assign output_led1_q_0_6 = ic_dflipflop_dflipflop_ic_out_0_24; // LED
    assign output_led2_q_0_7 = ic_dflipflop_dflipflop_ic_out_1_25; // LED

endmodule // jkflipflop

// ====================================================================
// Module jkflipflop generation completed successfully
// Elements processed: 20
// Inputs: 5, Outputs: 2
// ====================================================================
