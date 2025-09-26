// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_masterslave
// Generated: Fri Sep 26 18:49:20 2025
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
    wire ic_dlatch_ic_node_9_0;
    wire ic_dlatch_ic_not_10_0;
    wire ic_dlatch_ic_node_11_0;
    wire ic_dlatch_ic_nand_12_0;
    wire ic_dlatch_ic_nand_13_0;
    wire ic_dlatch_ic_nand_14_0;
    wire ic_dlatch_ic_nand_15_0;
    wire ic_dlatch_ic_node_16_0;
    wire ic_dlatch_ic_node_17_0;
// ============== END IC: DLATCH ==============
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
    wire ic_dlatch_ic_node_22_0;
    wire ic_dlatch_ic_not_23_0;
    wire ic_dlatch_ic_node_24_0;
    wire ic_dlatch_ic_nand_25_0;
    wire ic_dlatch_ic_nand_26_0;
    wire ic_dlatch_ic_nand_27_0;
    wire ic_dlatch_ic_nand_28_0;
    wire ic_dlatch_ic_node_29_0;
    wire ic_dlatch_ic_node_30_0;
// ============== END IC: DLATCH ==============
    wire not_31;
    wire node_32;
    wire node_33;

    // ========= Logic Assignments =========
    wire ic_dlatch_ic_node_34_0;
    wire ic_dlatch_ic_not_35_0;
    wire ic_dlatch_ic_node_36_0;
    wire ic_dlatch_ic_nand_37_0;
    wire ic_dlatch_ic_nand_38_0;
    wire ic_dlatch_ic_nand_39_0;
    wire ic_dlatch_ic_nand_40_0;
    wire ic_dlatch_ic_node_41_0;
    wire ic_dlatch_ic_node_42_0;
    assign node_33 = not_31; // Node
    wire ic_dlatch_ic_node_43_0;
    wire ic_dlatch_ic_not_44_0;
    wire ic_dlatch_ic_node_45_0;
    wire ic_dlatch_ic_nand_46_0;
    wire ic_dlatch_ic_nand_47_0;
    wire ic_dlatch_ic_nand_48_0;
    wire ic_dlatch_ic_nand_49_0;
    wire ic_dlatch_ic_node_50_0;
    wire ic_dlatch_ic_node_51_0;
    assign node_32 = input_clock1_clk_1; // Node
    assign not_31 = ~input_clock1_clk_1; // Not

    // ========= Output Assignments =========
    assign output_led1_q_0_3 = ic_dlatch_ic_node_41_0; // LED
    assign output_led2_q_0_4 = ic_dlatch_ic_node_42_0; // LED

endmodule // dflipflop_masterslave

// ====================================================================
// Module dflipflop_masterslave generation completed successfully
// Elements processed: 9
// Inputs: 2, Outputs: 2
// Warnings: 1
//   IC DLATCH output 1 is not connected
// ====================================================================
