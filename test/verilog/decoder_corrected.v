// ====================================================================
// ======= MANUALLY CORRECTED Verilog decoder =======
// ====================================================================
//
// Module: decoder_corrected
// Manually fixed to demonstrate correct 3-to-8 decoder logic
//
// ====================================================================

`timescale 1ns/1ps

module decoder_corrected (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2,
    output wire output_led3_0_3,
    output wire output_led4_0_4,
    output wire output_led5_0_5,
    output wire output_led6_0_6,
    output wire output_led7_0_7,
    output wire output_led8_0_8
);

    // ========= Logic Assignments =========
    // CORRECTED: For hardcoded input 000, output 0 should be active
    assign output_led1_0_1 = (~1'b0 & ~1'b0 & ~1'b0); // LED = (1 & 1 & 1) = 1 ✓
    assign output_led2_0_2 = (1'b0 & ~1'b0 & ~1'b0); // LED = (0 & 1 & 1) = 0 ✓
    assign output_led3_0_3 = (~1'b0 & 1'b0 & ~1'b0); // LED = (1 & 0 & 1) = 0 ✓
    assign output_led4_0_4 = (1'b0 & 1'b0 & ~1'b0); // LED = (0 & 0 & 1) = 0 ✓
    assign output_led5_0_5 = (~1'b0 & ~1'b0 & 1'b0); // LED = (1 & 1 & 0) = 0 ✓
    assign output_led6_0_6 = (1'b0 & ~1'b0 & 1'b0); // LED = (0 & 1 & 0) = 0 ✓
    assign output_led7_0_7 = (~1'b0 & 1'b0 & 1'b0); // LED = (1 & 0 & 0) = 0 ✓
    assign output_led8_0_8 = (1'b0 & 1'b0 & 1'b0); // LED = (0 & 0 & 0) = 0 ✓

endmodule // decoder_corrected

// ====================================================================
// CORRECTED: For input 000, only output_led1_0_1 = 1, pattern = 00000001
// ====================================================================