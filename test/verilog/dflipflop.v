// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop
// Generated: Fri Sep 26 14:36:19 2025
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
    wire not_7;
    wire not_8;
    wire node_9;
    wire node_10;
    wire node_11;
    wire nand_12;
    wire not_13;
    wire node_14;
    wire node_15;
    wire nand_16;
    wire nand_17;
    wire node_18;
    wire nand_19;
    wire nand_20;
    wire node_21;
    wire nand_22;
    wire nand_23;
    wire node_24;
    wire nand_25;

    // ========= Logic Assignments =========
    /* verilator lint_off UNOPTFLAT */
    assign nand_22 = ~(input_input_switch3__preset_3 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & ~(nand_16 & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4)) & input_clock1_clk_1 & ~(nand_22 & ~(input_clock1_clk_1 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & nand_19) & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4) & input_input_switch4__clear_4)); // Nand
    /* verilator lint_on UNOPTFLAT */
    /* verilator lint_off UNOPTFLAT */
    assign nand_25 = (input_input_switch3__preset_3 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & ~(nand_16 & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4)) & input_clock1_clk_1 & nand_25) & ~(input_clock1_clk_1 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & nand_19) & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4) & input_input_switch4__clear_4; // Nand
    /* verilator lint_on UNOPTFLAT */
    assign node_24 = input_input_switch4__clear_4; // Node
    assign nand_23 = ~(input_clock1_clk_1 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & nand_19) & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4); // Nand
    assign node_21 = input_input_switch3__preset_3; // Node
    assign nand_20 = (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & ~(nand_16 & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4)) & input_clock1_clk_1; // Nand
    /* verilator lint_off UNOPTFLAT */
    assign nand_16 = ~(input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & ~(nand_16 & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4)); // Nand
    /* verilator lint_on UNOPTFLAT */
    /* verilator lint_off UNOPTFLAT */
    assign nand_19 = (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & nand_19) & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4; // Nand
    /* verilator lint_on UNOPTFLAT */
    assign node_18 = input_input_switch4__clear_4; // Node
    assign nand_17 = input_clock1_clk_1 & ~input_push_button2_d_2; // Nand
    assign node_15 = input_clock1_clk_1; // Node
    assign node_14 = input_clock1_clk_1; // Node
    assign not_13 = ~input_push_button2_d_2; // Not
    assign nand_12 = ~(input_push_button2_d_2 & ~input_clock1_clk_1); // Nand
    assign node_11 = ~input_clock1_clk_1; // Node
    assign node_10 = input_clock1_clk_1; // Node
    assign node_9 = ~input_clock1_clk_1; // Node
    assign not_8 = input_clock1_clk_1; // Not
    assign not_7 = ~input_clock1_clk_1; // Not

    // ========= Output Assignments =========
    assign output_led1_0_5 = ~(input_input_switch3__preset_3 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & ~(nand_16 & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4)) & input_clock1_clk_1 & ~(nand_22 & ~(input_clock1_clk_1 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & nand_19) & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4) & input_input_switch4__clear_4)); // LED
    assign output_led2_0_6 = (input_input_switch3__preset_3 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & ~(nand_16 & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4)) & input_clock1_clk_1 & nand_25) & ~(input_clock1_clk_1 & (input_input_switch3__preset_3 & ~(input_push_button2_d_2 & ~input_clock1_clk_1) & nand_19) & input_clock1_clk_1 & ~input_push_button2_d_2 & input_input_switch4__clear_4) & input_input_switch4__clear_4; // LED

endmodule // dflipflop

// ====================================================================
// Module dflipflop generation completed successfully
// Elements processed: 25
// Inputs: 4, Outputs: 2
// ====================================================================
