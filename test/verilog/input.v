// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: wiredpanda_module
// Generated: Fri Sep 26 15:10:59 2025
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
    wire not_7;
    wire xor_8;
    wire or_9;
    wire and_10;

    // ========= Logic Assignments =========
    assign and_10 = (input_push_button1_x0_1 & input_push_button2_x1_2); // And
    assign or_9 = (input_push_button1_x0_1 | input_push_button2_x1_2); // Or
    assign xor_8 = (input_push_button1_x0_1 ^ input_push_button2_x1_2); // Xor
    assign not_7 = ~input_push_button1_x0_1; // Not

    // ========= Output Assignments =========
    assign output_led1_xor_0_3 = xor_8; // LED
    assign output_led2_not_x0_0_4 = not_7; // LED
    assign output_led3_and_0_5 = and_10; // LED
    assign output_led4_or_0_6 = or_9; // LED

endmodule // wiredpanda_module

// ====================================================================
// Module wiredpanda_module generation completed successfully
// Elements processed: 10
// Inputs: 2, Outputs: 4
// ====================================================================
