// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: jkflipflop
// Generated: Fri Sep 26 14:36:27 2025
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
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
    wire ic_dflipflop_ic_dflipflop_node_8;
    wire ic_dflipflop_ic_dflipflop_node_9;
    wire ic_dflipflop_ic_dflipflop_node_10;
    wire ic_dflipflop_ic_dflipflop_node_11;
    wire ic_dflipflop_ic_dflipflop_nand_12;
    wire ic_dflipflop_ic_dflipflop_node_13;
    wire ic_dflipflop_ic_dflipflop_nand_14;
    wire ic_dflipflop_ic_dflipflop_not_15;
    wire ic_dflipflop_ic_dflipflop_nand_16;
    wire ic_dflipflop_ic_dflipflop_nand_17;
    wire ic_dflipflop_ic_dflipflop_nand_18;
    wire ic_dflipflop_ic_dflipflop_nand_19;
    wire ic_dflipflop_ic_dflipflop_node_20;
    wire ic_dflipflop_ic_dflipflop_not_21;
    wire ic_dflipflop_ic_dflipflop_nand_22;
    wire ic_dflipflop_ic_dflipflop_nand_23;
    wire ic_dflipflop_ic_dflipflop_not_27;
    wire ic_dflipflop_ic_dflipflop_node_28;
    wire ic_dflipflop_ic_dflipflop_node_29;
    assign ic_dflipflop_ic_dflipflop_node_8 = ic_dflipflop_ic_dflipflop_node_29; // Node
    assign ic_dflipflop_ic_dflipflop_node_9 = input_clock1_c_1; // Node
    assign ic_dflipflop_ic_dflipflop_node_10 = ic_dflipflop_ic_dflipflop_node_28; // Node
    assign ic_dflipflop_ic_dflipflop_node_11 = 1'b0; // Node
    assign ic_dflipflop_ic_dflipflop_nand_12 = ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10); // Nand
    assign ic_dflipflop_ic_dflipflop_node_13 = ~(input_input_switch2__preset_2 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8 & ~(ic_dflipflop_ic_dflipflop_nand_14 & ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3) & input_input_switch3__clear_3)); // Node
    /* verilator lint_off UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_14 = ~(input_input_switch2__preset_2 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8 & ~(ic_dflipflop_ic_dflipflop_nand_14 & ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3) & input_input_switch3__clear_3)); // Nand
    /* verilator lint_on UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_not_15 = input_clock1_c_1; // Not
    /* verilator lint_off UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_16 = (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3; // Nand
    /* verilator lint_on UNOPTFLAT */
    /* verilator lint_off UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_17 = ~(input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)); // Nand
    /* verilator lint_on UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_18 = (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8; // Nand
    /* verilator lint_off UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_19 = (input_input_switch2__preset_2 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8 & ic_dflipflop_ic_dflipflop_nand_19) & ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3) & input_input_switch3__clear_3; // Nand
    /* verilator lint_on UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_node_20 = (input_input_switch2__preset_2 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8 & ic_dflipflop_ic_dflipflop_nand_19) & ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3) & input_input_switch3__clear_3; // Node
    assign ic_dflipflop_ic_dflipflop_not_21 = ~ic_dflipflop_ic_dflipflop_node_11; // Not
    assign ic_dflipflop_ic_dflipflop_nand_22 = ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_23 = ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11); // Nand
    assign ic_dflipflop_ic_dflipflop_not_27 = ~input_clock1_c_1; // Not
    assign ic_dflipflop_ic_dflipflop_node_28 = ~input_clock1_c_1; // Node
    assign ic_dflipflop_ic_dflipflop_node_29 = ic_dflipflop_ic_dflipflop_node_9; // Node
// ============== END IC: DFLIPFLOP ==============
    wire node_33;
    wire node_34;
    wire node_35;
    wire node_36;
    wire node_37;
    wire node_38;
    wire node_39;
    wire not_40;
    wire node_41;
    wire and_42;
    wire and_43;
    wire or_44;

    // ========= Logic Assignments =========
    assign ic_dflipflop_ic_dflipflop_node_8 = ic_dflipflop_ic_dflipflop_node_29; // Node
    assign ic_dflipflop_ic_dflipflop_node_9 = input_clock1_c_1; // Node
    assign ic_dflipflop_ic_dflipflop_node_10 = ic_dflipflop_ic_dflipflop_node_28; // Node
    assign ic_dflipflop_ic_dflipflop_node_11 = 1'b0; // Node
    assign ic_dflipflop_ic_dflipflop_nand_12 = ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10); // Nand
    assign ic_dflipflop_ic_dflipflop_node_13 = ~(input_input_switch2__preset_2 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8 & ~(ic_dflipflop_ic_dflipflop_nand_14 & ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3) & input_input_switch3__clear_3)); // Node
    /* verilator lint_off UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_14 = ~(input_input_switch2__preset_2 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8 & ~(ic_dflipflop_ic_dflipflop_nand_14 & ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3) & input_input_switch3__clear_3)); // Nand
    /* verilator lint_on UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_not_15 = input_clock1_c_1; // Not
    /* verilator lint_off UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_16 = (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3; // Nand
    /* verilator lint_on UNOPTFLAT */
    /* verilator lint_off UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_17 = ~(input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)); // Nand
    /* verilator lint_on UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_18 = (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8; // Nand
    /* verilator lint_off UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_nand_19 = (input_input_switch2__preset_2 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8 & ic_dflipflop_ic_dflipflop_nand_19) & ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3) & input_input_switch3__clear_3; // Nand
    /* verilator lint_on UNOPTFLAT */
    assign ic_dflipflop_ic_dflipflop_node_20 = (input_input_switch2__preset_2 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ~(ic_dflipflop_ic_dflipflop_nand_17 & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3)) & ic_dflipflop_ic_dflipflop_node_8 & ic_dflipflop_ic_dflipflop_nand_19) & ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3) & input_input_switch3__clear_3; // Node
    assign ic_dflipflop_ic_dflipflop_not_21 = ~ic_dflipflop_ic_dflipflop_node_11; // Not
    assign ic_dflipflop_ic_dflipflop_nand_22 = ~(ic_dflipflop_ic_dflipflop_node_29 & (input_input_switch2__preset_2 & ~(ic_dflipflop_ic_dflipflop_node_11 & ic_dflipflop_ic_dflipflop_node_10) & ic_dflipflop_ic_dflipflop_nand_16) & ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11) & input_input_switch3__clear_3); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_23 = ~(ic_dflipflop_ic_dflipflop_node_28 & ~ic_dflipflop_ic_dflipflop_node_11); // Nand
    assign ic_dflipflop_ic_dflipflop_not_27 = ~input_clock1_c_1; // Not
    assign ic_dflipflop_ic_dflipflop_node_28 = ~input_clock1_c_1; // Node
    assign ic_dflipflop_ic_dflipflop_node_29 = ic_dflipflop_ic_dflipflop_node_9; // Node
    assign or_44 = ((1'b0 & ~input_input_switch4_j_4) | (input_input_switch5_k_5 & 1'b0)); // Or
    assign and_42 = (input_input_switch5_k_5 & 1'b0); // And
    assign node_39 = 1'b0; // Node
    assign node_36 = 1'b0; // Node
    assign node_41 = input_input_switch5_k_5; // Node
    assign and_43 = (1'b0 & ~input_input_switch4_j_4); // And
    assign not_40 = ~input_input_switch4_j_4; // Not
    assign node_38 = 1'b0; // Node
    assign node_37 = 1'b0; // Node
    assign node_35 = input_clock1_c_1; // Node
    assign node_34 = input_input_switch3__clear_3; // Node
    assign node_33 = input_input_switch2__preset_2; // Node

    // ========= Output Assignments =========
    assign output_led1_q_0_6 = 1'b0; // LED
    assign output_led2_q_0_7 = 1'b0; // LED

endmodule // jkflipflop

// ====================================================================
// Module jkflipflop generation completed successfully
// Elements processed: 20
// Inputs: 5, Outputs: 2
// ====================================================================
