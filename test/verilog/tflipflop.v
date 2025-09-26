// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: tflipflop
// Generated: Fri Sep 26 14:59:17 2025
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
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
    wire ic_dflipflop_ic_dflipflop_node_7;
    wire ic_dflipflop_ic_dflipflop_node_8;
    wire ic_dflipflop_ic_dflipflop_node_9;
    wire ic_dflipflop_ic_dflipflop_node_10;
    wire ic_dflipflop_ic_dflipflop_nand_11;
    wire ic_dflipflop_ic_dflipflop_node_12;
    wire ic_dflipflop_ic_dflipflop_nand_13;
    wire ic_dflipflop_ic_dflipflop_not_14;
    wire ic_dflipflop_ic_dflipflop_nand_15;
    wire ic_dflipflop_ic_dflipflop_nand_16;
    wire ic_dflipflop_ic_dflipflop_nand_17;
    wire ic_dflipflop_ic_dflipflop_nand_18;
    wire ic_dflipflop_ic_dflipflop_node_19;
    wire ic_dflipflop_ic_dflipflop_not_20;
    wire ic_dflipflop_ic_dflipflop_nand_21;
    wire ic_dflipflop_ic_dflipflop_nand_22;
    wire ic_dflipflop_ic_dflipflop_not_26;
    wire ic_dflipflop_ic_dflipflop_node_27;
    wire ic_dflipflop_ic_dflipflop_node_28;
    assign ic_dflipflop_ic_dflipflop_node_7 = ic_dflipflop_ic_dflipflop_node_28; // Node
    assign ic_dflipflop_ic_dflipflop_node_8 = ic_dflipflop_ic_dflipflop_not_14; // Node
    assign ic_dflipflop_ic_dflipflop_node_9 = ic_dflipflop_ic_dflipflop_node_27; // Node
    assign ic_dflipflop_ic_dflipflop_node_10 = 1'b0; // Node
    assign ic_dflipflop_ic_dflipflop_nand_11 = ~(ic_dflipflop_ic_dflipflop_node_10 & ic_dflipflop_ic_dflipflop_node_9); // Nand
    assign ic_dflipflop_ic_dflipflop_node_12 = ic_dflipflop_ic_dflipflop_nand_13; // Node
    assign ic_dflipflop_ic_dflipflop_nand_13 = ~(input_input_switch3__preset_3 & ic_dflipflop_ic_dflipflop_nand_17 & ic_dflipflop_ic_dflipflop_nand_18); // Nand
    assign ic_dflipflop_ic_dflipflop_not_14 = ~ic_dflipflop_ic_dflipflop_not_26; // Not
    assign ic_dflipflop_ic_dflipflop_nand_15 = ~(ic_dflipflop_ic_dflipflop_nand_16 & ic_dflipflop_ic_dflipflop_nand_22 & input_input_switch4__clear_4); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_16 = ~(input_input_switch3__preset_3 & ic_dflipflop_ic_dflipflop_nand_11 & ic_dflipflop_ic_dflipflop_nand_15); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_17 = ~(ic_dflipflop_ic_dflipflop_nand_16 & ic_dflipflop_ic_dflipflop_node_7); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_18 = ~(ic_dflipflop_ic_dflipflop_nand_13 & ic_dflipflop_ic_dflipflop_nand_21 & input_input_switch4__clear_4); // Nand
    assign ic_dflipflop_ic_dflipflop_node_19 = ic_dflipflop_ic_dflipflop_nand_18; // Node
    assign ic_dflipflop_ic_dflipflop_not_20 = ~ic_dflipflop_ic_dflipflop_node_10; // Not
    assign ic_dflipflop_ic_dflipflop_nand_21 = ~(ic_dflipflop_ic_dflipflop_node_28 & ic_dflipflop_ic_dflipflop_nand_15); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_22 = ~(ic_dflipflop_ic_dflipflop_node_27 & ic_dflipflop_ic_dflipflop_not_20); // Nand
    assign ic_dflipflop_ic_dflipflop_not_26 = ~input_clock2_c_2; // Not
    assign ic_dflipflop_ic_dflipflop_node_27 = ic_dflipflop_ic_dflipflop_not_26; // Node
    assign ic_dflipflop_ic_dflipflop_node_28 = ic_dflipflop_ic_dflipflop_node_8; // Node
// ============== END IC: DFLIPFLOP ==============
    wire node_32;
    wire node_33;
    wire not_34;
    wire and_35;
    wire or_36;
    wire node_37;
    wire and_38;

    // ========= Logic Assignments =========
    assign node_32 = 1'b0; // Node
    assign ic_dflipflop_ic_dflipflop_node_7 = ic_dflipflop_ic_dflipflop_node_28; // Node
    assign ic_dflipflop_ic_dflipflop_node_8 = ic_dflipflop_ic_dflipflop_not_14; // Node
    assign ic_dflipflop_ic_dflipflop_node_9 = ic_dflipflop_ic_dflipflop_node_27; // Node
    assign ic_dflipflop_ic_dflipflop_node_10 = 1'b0; // Node
    assign ic_dflipflop_ic_dflipflop_nand_11 = ~(ic_dflipflop_ic_dflipflop_node_10 & ic_dflipflop_ic_dflipflop_node_9); // Nand
    assign ic_dflipflop_ic_dflipflop_node_12 = ic_dflipflop_ic_dflipflop_nand_13; // Node
    assign ic_dflipflop_ic_dflipflop_nand_13 = ~(input_input_switch3__preset_3 & ic_dflipflop_ic_dflipflop_nand_17 & ic_dflipflop_ic_dflipflop_nand_18); // Nand
    assign ic_dflipflop_ic_dflipflop_not_14 = ~ic_dflipflop_ic_dflipflop_not_26; // Not
    assign ic_dflipflop_ic_dflipflop_nand_15 = ~(ic_dflipflop_ic_dflipflop_nand_16 & ic_dflipflop_ic_dflipflop_nand_22 & input_input_switch4__clear_4); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_16 = ~(input_input_switch3__preset_3 & ic_dflipflop_ic_dflipflop_nand_11 & ic_dflipflop_ic_dflipflop_nand_15); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_17 = ~(ic_dflipflop_ic_dflipflop_nand_16 & ic_dflipflop_ic_dflipflop_node_7); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_18 = ~(ic_dflipflop_ic_dflipflop_nand_13 & ic_dflipflop_ic_dflipflop_nand_21 & input_input_switch4__clear_4); // Nand
    assign ic_dflipflop_ic_dflipflop_node_19 = ic_dflipflop_ic_dflipflop_nand_18; // Node
    assign ic_dflipflop_ic_dflipflop_not_20 = ~ic_dflipflop_ic_dflipflop_node_10; // Not
    assign ic_dflipflop_ic_dflipflop_nand_21 = ~(ic_dflipflop_ic_dflipflop_node_28 & ic_dflipflop_ic_dflipflop_nand_15); // Nand
    assign ic_dflipflop_ic_dflipflop_nand_22 = ~(ic_dflipflop_ic_dflipflop_node_27 & ic_dflipflop_ic_dflipflop_not_20); // Nand
    assign ic_dflipflop_ic_dflipflop_not_26 = ~input_clock2_c_2; // Not
    assign ic_dflipflop_ic_dflipflop_node_27 = ic_dflipflop_ic_dflipflop_not_26; // Node
    assign ic_dflipflop_ic_dflipflop_node_28 = ic_dflipflop_ic_dflipflop_node_8; // Node
    assign or_36 = (and_35 | and_38); // Or
    assign and_38 = (node_37 & 1'b0); // And
    assign node_37 = input_push_button1_t_1; // Node
    assign and_35 = (node_33 & not_34); // And
    assign not_34 = ~input_push_button1_t_1; // Not
    assign node_33 = node_32; // Node

    // ========= Output Assignments =========
    assign output_led1_q_0_5 = 1'b0; // LED
    assign output_led2_q_0_6 = 1'b0; // LED

endmodule // tflipflop

// ====================================================================
// Module tflipflop generation completed successfully
// Elements processed: 14
// Inputs: 4, Outputs: 2
// ====================================================================
