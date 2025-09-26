// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop
// Generated: Fri Sep 26 14:59:06 2025
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
    assign nand_22 = ~(node_21 & nand_20 & nand_25); // Nand
    assign nand_25 = ~(nand_22 & nand_23 & node_24); // Nand
    assign node_24 = input_input_switch4__clear_4; // Node
    assign nand_23 = ~(node_14 & nand_19); // Nand
    assign node_21 = input_input_switch3__preset_3; // Node
    assign nand_20 = ~(nand_16 & node_15); // Nand
    assign nand_16 = ~(input_input_switch3__preset_3 & nand_12 & nand_19); // Nand
    assign nand_19 = ~(nand_16 & nand_17 & node_18); // Nand
    assign node_18 = input_input_switch4__clear_4; // Node
    assign nand_17 = ~(node_9 & not_13); // Nand
    assign node_15 = node_14; // Node
    assign node_14 = node_10; // Node
    assign not_13 = ~input_push_button2_d_2; // Not
    assign nand_12 = ~(input_push_button2_d_2 & node_11); // Nand
    assign node_11 = node_9; // Node
    assign node_10 = not_8; // Node
    assign node_9 = not_7; // Node
    assign not_8 = ~not_7; // Not
    assign not_7 = ~input_clock1_clk_1; // Not

    // ========= Output Assignments =========
    assign output_led1_0_5 = nand_22; // LED
    assign output_led2_0_6 = nand_25; // LED

endmodule // dflipflop

// ====================================================================
// Module dflipflop generation completed successfully
// Elements processed: 25
// Inputs: 4, Outputs: 2
// ====================================================================
