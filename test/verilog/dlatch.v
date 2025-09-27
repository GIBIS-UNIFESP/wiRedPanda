// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dlatch
// Generated: Sat Sep 27 18:03:51 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module dlatch (
    // ========= Input Ports =========
    input wire input_push_button1_d_1,
    input wire input_clock2_clk_2,

    // ========= Output Ports =========
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_led1_q_0_3_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock2_clk_2) begin
begin // Synchronous operation
            output_led1_q_0_3_behavioral_reg <= input_push_button1_d_1;
        end
    end


    // ========= Output Assignments =========
    // ========= Internal Signals =========

    // ========= Logic Assignments =========
    assign output_led1_q_0_3 = output_led1_q_0_3_behavioral_reg; // Connect behavioral register to output
    assign output_led2_q_0_4 = ~output_led1_q_0_3_behavioral_reg; // Complementary output
    assign output_led1_q_0_3 = ~((input_push_button1_d_1 & input_clock2_clk_2) & 1'b0 & ~(input_clock2_clk_2 & ~input_push_button1_d_1)); // LED
    assign output_led2_q_0_4 = (input_push_button1_d_1 & input_clock2_clk_2) & ~(1'b0 & ~(input_clock2_clk_2 & ~input_push_button1_d_1)); // LED

endmodule // dlatch

// ====================================================================
// Module dlatch generation completed successfully
// Elements processed: 9
// Inputs: 2, Outputs: 2
// ====================================================================
