// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: wiredpanda_module
// Generated: Fri Sep 26 00:05:58 2025
// Target FPGA: Generic-Small
// Resource Usage: 12/1000 LUTs, 0/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module wiredpanda_module (
    // ========= Input Ports =========
    input wire input_push_button1_x0_1,
    input wire input_push_button2_x1_2,

    // ========= Output Ports =========
    output wire output_led1_xor_0_3,
    output wire output_led2_not_x0_0_4,
    output wire output_led3_and_0_5,
    output wire output_led4_or_0_6
);

    // ========= Internal Signals =========
    wire not_10_0;
    wire xor_9_0;
    wire or_8_0;
    wire and_7_0;

    // ========= Logic Assignments =========
    assign and_7_0 = (input_push_button1_x0_1 & input_push_button2_x1_2); // And
    assign or_8_0 = (input_push_button1_x0_1 | input_push_button2_x1_2); // Or
    assign xor_9_0 = (input_push_button1_x0_1 ^ input_push_button2_x1_2); // Xor
    assign not_10_0 = ~input_push_button1_x0_1; // Not

    // ========= Output Assignments =========
    assign output_led1_xor_0_3 = (input_push_button1_x0_1 ^ input_push_button2_x1_2); // LED
    assign output_led2_not_x0_0_4 = ~input_push_button1_x0_1; // LED
    assign output_led3_and_0_5 = (input_push_button1_x0_1 & input_push_button2_x1_2); // LED
    assign output_led4_or_0_6 = (input_push_button1_x0_1 | input_push_button2_x1_2); // LED

endmodule // wiredpanda_module

// ====================================================================
// Module wiredpanda_module generation completed successfully
// Elements processed: 10
// Inputs: 2, Outputs: 4
// ====================================================================
