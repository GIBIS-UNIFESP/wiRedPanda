// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop
// Generated: Sun Sep 28 03:36:06 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

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

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_led1_0_5_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock1_clk_1 or negedge input_input_switch3__preset_3) begin
        if (!input_input_switch3__preset_3) begin
            output_led1_0_5_behavioral_reg <= 1'b1; // Asynchronous preset
        end else begin // Synchronous operation
            output_led1_0_5_behavioral_reg <= input_push_button2_d_2;
        end
    end


    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_0_5 = output_led1_0_5_behavioral_reg; // Connect behavioral register to output
    assign output_led2_0_6 = ~output_led1_0_5_behavioral_reg; // Complementary output


    // ========= ULTRATHINK FINAL SCAN: Diagnostic Information =========
    // ULTRATHINK DEBUG: Found 0 potentially undeclared variables
    // ULTRATHINK DEBUG: Already declared variables count: 4
    // ULTRATHINK DEBUG: Scanned content size: 187 characters
    // ULTRATHINK DEBUG: No additional variables found to declare

endmodule // dflipflop

// ====================================================================
// Module dflipflop generation completed successfully
// Elements processed: 25
// Inputs: 4, Outputs: 2
// ====================================================================
