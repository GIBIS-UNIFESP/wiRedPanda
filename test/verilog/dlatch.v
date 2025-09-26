// DEBUG: Selected FPGA: Generic-Small (Small generic FPGA (educational))
// DEBUG: Estimated resources: 10 LUTs, 35 FFs, 4 IOs
// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dlatch
// Generated: Fri Sep 26 03:03:44 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dlatch (
    // ========= Input Ports =========
// DEBUG: Input port: Push Button -> input_push_button1_d_1 (Element: Push Button)
// DEBUG: Input port: Clock -> input_clock2_clk_2 (Element: Clock)
    input wire input_push_button1_d_1,
    input wire input_clock2_clk_2,

    // ========= Output Ports =========
// DEBUG: Output port: LED[0] -> output_led1_q_0_3 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led2_q_0_4 (Element: LED)
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Internal Signals =========
    wire not_5_0;
    wire nand_6_0;
    wire nand_7_0;
    wire nand_8_0;
    wire nand_9_0;

    // ========= Logic Assignments =========
// DEBUG: Processing 9 top-level elements with topological sorting
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Circular dependency detected, checking varMap (Element: Nand)
// DEBUG: otherPortName: varMap result for circular dependency: 'nand_7_0' (Element: Nand)
// DEBUG: otherPortName: Returning varMap result for circular dependency: nand_7_0 (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Not)
// DEBUG: otherPortName: Processing port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
    assign nand_7_0 = ~((input_push_button1_d_1 & input_clock2_clk_2) & nand_7_0 & ~(input_clock2_clk_2 & ~input_push_button1_d_1)); // Nand
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Circular dependency detected, checking varMap (Element: Nand)
// DEBUG: otherPortName: varMap result for circular dependency: 'nand_9_0' (Element: Nand)
// DEBUG: otherPortName: Returning varMap result for circular dependency: nand_9_0 (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Not)
// DEBUG: otherPortName: Processing port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
    assign nand_9_0 = (input_push_button1_d_1 & input_clock2_clk_2) & ~(nand_9_0 & ~(input_clock2_clk_2 & ~input_push_button1_d_1)); // Nand
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
    assign nand_8_0 = ~(input_push_button1_d_1 & input_clock2_clk_2); // Nand
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Not)
// DEBUG: otherPortName: Processing port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
    assign nand_6_0 = ~(input_clock2_clk_2 & ~input_push_button1_d_1); // Nand
// DEBUG: otherPortName: Processing port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
    assign not_5_0 = ~input_push_button1_d_1; // Not

    // ========= Output Assignments =========
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Circular dependency detected, checking varMap (Element: Nand)
// DEBUG: otherPortName: varMap result for circular dependency: 'nand_7_0' (Element: Nand)
// DEBUG: otherPortName: Returning varMap result for circular dependency: nand_7_0 (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Not)
// DEBUG: otherPortName: Processing port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
    assign output_led1_q_0_3 = ~((input_push_button1_d_1 & input_clock2_clk_2) & nand_7_0 & ~(input_clock2_clk_2 & ~input_push_button1_d_1)); // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Circular dependency detected, checking varMap (Element: Nand)
// DEBUG: otherPortName: varMap result for circular dependency: 'nand_9_0' (Element: Nand)
// DEBUG: otherPortName: Returning varMap result for circular dependency: nand_9_0 (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Nand)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_clk_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_clk_2 (Element: Clock)
// DEBUG: otherPortName: Processing port from element Nand (type 7) (Element: Nand)
// DEBUG: otherPortName: Found connected port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Not)
// DEBUG: otherPortName: Processing port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_d_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_d_1 (Element: Push Button)
    assign output_led2_q_0_4 = (input_push_button1_d_1 & input_clock2_clk_2) & ~(nand_9_0 & ~(input_clock2_clk_2 & ~input_push_button1_d_1)); // LED

endmodule // dlatch

// ====================================================================
// Module dlatch generation completed successfully
// Elements processed: 9
// Inputs: 2, Outputs: 2
// ====================================================================
