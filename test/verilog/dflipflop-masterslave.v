// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_masterslave
// Generated: Fri Sep 26 15:10:52 2025
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
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
    wire ic_dlatch_ic_dlatch_node_5;
    wire ic_dlatch_ic_dlatch_node_6;
    wire ic_dlatch_ic_dlatch_nand_7;
    wire ic_dlatch_ic_dlatch_nand_8;
    wire ic_dlatch_ic_dlatch_not_9;
    wire ic_dlatch_ic_dlatch_nand_10;
    wire ic_dlatch_ic_dlatch_nand_11;
    assign ic_dlatch_ic_dlatch_nand_7 = ~(ic_dlatch_ic_dlatch_nand_8 & ic_dlatch_ic_dlatch_nand_10); // Nand
    assign ic_dlatch_ic_dlatch_nand_8 = ~(ic_dlatch_ic_dlatch_nand_11 & ic_dlatch_ic_dlatch_nand_7); // Nand
    assign ic_dlatch_ic_dlatch_not_9 = ~input_push_button2_d_2; // Not
    assign ic_dlatch_ic_dlatch_nand_10 = ~(input_clock1_clk_1 & ic_dlatch_ic_dlatch_not_9); // Nand
    assign ic_dlatch_ic_dlatch_nand_11 = ~(input_push_button2_d_2 & input_clock1_clk_1); // Nand
// ============== END IC: DLATCH ==============
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
    wire ic_dlatch_ic_dlatch_node_14;
    wire ic_dlatch_ic_dlatch_node_15;
    wire ic_dlatch_ic_dlatch_nand_16;
    wire ic_dlatch_ic_dlatch_nand_17;
    wire ic_dlatch_ic_dlatch_not_18;
    wire ic_dlatch_ic_dlatch_nand_19;
    wire ic_dlatch_ic_dlatch_nand_20;
    wire ic_dlatch_ic_dlatch_node_22;
    assign ic_dlatch_ic_dlatch_nand_16 = ~(ic_dlatch_ic_dlatch_nand_17 & ic_dlatch_ic_dlatch_nand_19); // Nand
    assign ic_dlatch_ic_dlatch_nand_17 = ~(ic_dlatch_ic_dlatch_nand_20 & ic_dlatch_ic_dlatch_nand_16); // Nand
    assign ic_dlatch_ic_dlatch_not_18 = ~ic_dlatch_ic_dlatch_node_22; // Not
    assign ic_dlatch_ic_dlatch_nand_19 = input_clock1_clk_1 & ic_dlatch_ic_dlatch_not_18; // Nand
    assign ic_dlatch_ic_dlatch_nand_20 = ~(ic_dlatch_ic_dlatch_node_22 & ~input_clock1_clk_1); // Nand
    assign ic_dlatch_ic_dlatch_node_22 = 1'b0; // Node
// ============== END IC: DLATCH ==============
    wire not_23;
    wire node_24;
    wire node_25;

    // ========= Logic Assignments =========
    assign ic_dlatch_ic_dlatch_nand_16 = ~(ic_dlatch_ic_dlatch_nand_17 & ic_dlatch_ic_dlatch_nand_19); // Nand
    assign ic_dlatch_ic_dlatch_nand_17 = ~(ic_dlatch_ic_dlatch_nand_20 & ic_dlatch_ic_dlatch_nand_16); // Nand
    assign ic_dlatch_ic_dlatch_not_18 = ~ic_dlatch_ic_dlatch_node_22; // Not
    assign ic_dlatch_ic_dlatch_nand_19 = input_clock1_clk_1 & ic_dlatch_ic_dlatch_not_18; // Nand
    assign ic_dlatch_ic_dlatch_nand_20 = ~(ic_dlatch_ic_dlatch_node_22 & ~input_clock1_clk_1); // Nand
    assign ic_dlatch_ic_dlatch_node_22 = 1'b0; // Node
    assign node_25 = not_23; // Node
    assign ic_dlatch_ic_dlatch_nand_7 = ~(ic_dlatch_ic_dlatch_nand_8 & ic_dlatch_ic_dlatch_nand_10); // Nand
    assign ic_dlatch_ic_dlatch_nand_8 = ~(ic_dlatch_ic_dlatch_nand_11 & ic_dlatch_ic_dlatch_nand_7); // Nand
    assign ic_dlatch_ic_dlatch_not_9 = ~input_push_button2_d_2; // Not
    assign ic_dlatch_ic_dlatch_nand_10 = ~(input_clock1_clk_1 & ic_dlatch_ic_dlatch_not_9); // Nand
    assign ic_dlatch_ic_dlatch_nand_11 = ~(input_push_button2_d_2 & input_clock1_clk_1); // Nand
    assign node_24 = input_clock1_clk_1; // Node
    assign not_23 = ~input_clock1_clk_1; // Not

    // ========= Output Assignments =========
    assign output_led1_q_0_3 = 1'b0; // LED
    assign output_led2_q_0_4 = 1'b0; // LED

endmodule // dflipflop_masterslave

// ====================================================================
// Module dflipflop_masterslave generation completed successfully
// Elements processed: 9
// Inputs: 2, Outputs: 2
// Warnings: 1
//   IC DLATCH output 1 is not connected
// ====================================================================
