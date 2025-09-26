// DEBUG: Selected FPGA: Generic-Small (Small generic FPGA (educational))
// DEBUG: Estimated resources: 12 LUTs, 0 FFs, 6 IOs
// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: wiredpanda_module
// Generated: Fri Sep 26 03:03:45 2025
// Target FPGA: Generic-Small
// Resource Usage: 12/1000 LUTs, 0/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module wiredpanda_module (
    // ========= Input Ports =========
// DEBUG: Input port: Push Button -> input_push_button1_x0_1 (Element: Push Button)
// DEBUG: Input port: Push Button -> input_push_button2_x1_2 (Element: Push Button)
    input wire input_push_button1_x0_1,
    input wire input_push_button2_x1_2,

    // ========= Output Ports =========
// DEBUG: Output port: LED[0] -> output_led1_xor_0_3 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led2_not_x0_0_4 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led3_and_0_5 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led4_or_0_6 (Element: LED)
    output wire output_led1_xor_0_3,
    output wire output_led2_not_x0_0_4,
    output wire output_led3_and_0_5,
    output wire output_led4_or_0_6
);

    // ========= Internal Signals =========
    wire not_7_0;
    wire xor_8_0;
    wire or_9_0;
    wire and_10_0;

    // ========= Logic Assignments =========
// DEBUG: Processing 10 top-level elements with topological sorting
// DEBUG: otherPortName: Processing port from element And (type 5) (Element: And)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_x0_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_x0_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element And (type 5) (Element: And)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button2_x1_2' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button2_x1_2 (Element: Push Button)
    assign and_10_0 = (input_push_button1_x0_1 & input_push_button2_x1_2); // And
// DEBUG: otherPortName: Processing port from element Or (type 6) (Element: Or)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_x0_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_x0_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Or (type 6) (Element: Or)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button2_x1_2' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button2_x1_2 (Element: Push Button)
    assign or_9_0 = (input_push_button1_x0_1 | input_push_button2_x1_2); // Or
// DEBUG: otherPortName: Processing port from element Xor (type 10) (Element: Xor)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_x0_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_x0_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Xor (type 10) (Element: Xor)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button2_x1_2' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button2_x1_2 (Element: Push Button)
    assign xor_8_0 = (input_push_button1_x0_1 ^ input_push_button2_x1_2); // Xor
// DEBUG: otherPortName: Processing port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_x0_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_x0_1 (Element: Push Button)
    assign not_7_0 = ~input_push_button1_x0_1; // Not

    // ========= Output Assignments =========
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element Xor (type 10) (Element: Xor)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Xor)
// DEBUG: otherPortName: Processing port from element Xor (type 10) (Element: Xor)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_x0_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_x0_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Xor (type 10) (Element: Xor)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button2_x1_2' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button2_x1_2 (Element: Push Button)
    assign output_led1_xor_0_3 = (input_push_button1_x0_1 ^ input_push_button2_x1_2); // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Not)
// DEBUG: otherPortName: Processing port from element Not (type 4) (Element: Not)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_x0_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_x0_1 (Element: Push Button)
    assign output_led2_not_x0_0_4 = ~input_push_button1_x0_1; // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element And (type 5) (Element: And)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: And)
// DEBUG: otherPortName: Processing port from element And (type 5) (Element: And)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_x0_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_x0_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element And (type 5) (Element: And)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button2_x1_2' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button2_x1_2 (Element: Push Button)
    assign output_led3_and_0_5 = (input_push_button1_x0_1 & input_push_button2_x1_2); // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element Or (type 6) (Element: Or)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Or)
// DEBUG: otherPortName: Processing port from element Or (type 6) (Element: Or)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button1_x0_1' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button1_x0_1 (Element: Push Button)
// DEBUG: otherPortName: Processing port from element Or (type 6) (Element: Or)
// DEBUG: otherPortName: Found connected port from element Push Button (type 1) (Element: Push Button)
// DEBUG: otherPortName: Checking varMap for final result (Element: Push Button)
// DEBUG: otherPortName: varMap lookup result: 'input_push_button2_x1_2' (Element: Push Button)
// DEBUG: otherPortName: Final result: input_push_button2_x1_2 (Element: Push Button)
    assign output_led4_or_0_6 = (input_push_button1_x0_1 | input_push_button2_x1_2); // LED

endmodule // wiredpanda_module

// ====================================================================
// Module wiredpanda_module generation completed successfully
// Elements processed: 10
// Inputs: 2, Outputs: 4
// ====================================================================
