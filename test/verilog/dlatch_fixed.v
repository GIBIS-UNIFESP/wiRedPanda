// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dlatch_fixed
// Generated: Fri Sep 26 00:40:07 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dlatch_fixed (
    // ========= Input Ports =========
    input wire input_push_button1_d_1,
    input wire input_clock2_clk_2,

    // ========= Output Ports =========
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Internal Signals =========
    wire not_9_0;
    wire nand_8_0;
    wire nand_5_0;
    wire nand_7_0;
    wire nand_6_0;

    // ========= Logic Assignments =========
    assign nand_5_0 = ~(~(~(input_push_button1_d_1 & input_clock2_clk_2) & nand_5_0) & ~(input_clock2_clk_2 & ~input_push_button1_d_1)); // Nand
    assign nand_6_0 = ~(~(input_push_button1_d_1 & input_clock2_clk_2) & ~(nand_6_0 & ~(input_clock2_clk_2 & ~input_push_button1_d_1))); // Nand
    assign nand_7_0 = ~(input_push_button1_d_1 & input_clock2_clk_2); // Nand
    assign nand_8_0 = ~(input_clock2_clk_2 & ~input_push_button1_d_1); // Nand
    assign not_9_0 = ~input_push_button1_d_1; // Not

    // ========= Output Assignments =========
    assign output_led1_q_0_3 = ~(~(~(input_push_button1_d_1 & input_clock2_clk_2) & nand_5_0) & ~(input_clock2_clk_2 & ~input_push_button1_d_1)); // LED
    assign output_led2_q_0_4 = ~(~(input_push_button1_d_1 & input_clock2_clk_2) & ~(nand_6_0 & ~(input_clock2_clk_2 & ~input_push_button1_d_1))); // LED

endmodule // dlatch_fixed

// ====================================================================
// Module dlatch_fixed generation completed successfully
// Elements processed: 9
// Inputs: 2, Outputs: 2
// ====================================================================
