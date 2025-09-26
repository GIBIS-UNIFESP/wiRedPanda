// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dlatch
// Generated: Fri Sep 26 14:59:11 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dlatch (
    // ========= Input Ports =========
    input wire input_push_button1_d_1,
    input wire input_clock2_clk_2,

    // ========= Output Ports =========
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Internal Signals =========
    wire not_5;
    wire nand_6;
    wire nand_7;
    wire nand_8;
    wire nand_9;

    // ========= Logic Assignments =========
    assign nand_7 = ~(nand_9 & nand_6); // Nand
    assign nand_9 = ~(nand_8 & nand_7); // Nand
    assign nand_8 = ~(input_push_button1_d_1 & input_clock2_clk_2); // Nand
    assign nand_6 = ~(input_clock2_clk_2 & not_5); // Nand
    assign not_5 = ~input_push_button1_d_1; // Not

    // ========= Output Assignments =========
    assign output_led1_q_0_3 = nand_7; // LED
    assign output_led2_q_0_4 = nand_9; // LED

endmodule // dlatch

// ====================================================================
// Module dlatch generation completed successfully
// Elements processed: 9
// Inputs: 2, Outputs: 2
// ====================================================================
