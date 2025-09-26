// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dlatch
// Generated: Fri Sep 26 19:56:50 2025
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

    // ========= Internal Sequential Register =========
    reg output_led1_q_0_3_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock2_clk_2) begin
begin // Synchronous operation
            output_led1_q_0_3_behavioral_reg <= input_push_button1_d_1;
        end
    end

    assign output_led1_q_0_3 = output_led1_q_0_3_behavioral_reg; // Connect behavioral register to output
    assign output_led2_q_0_4 = ~output_led1_q_0_3_behavioral_reg; // Complementary output

    // ========= Output Assignments =========
    assign output_led1_q_0_3 = nand_7; // LED
    assign output_led2_q_0_4 = nand_9; // LED

endmodule // dlatch

// ====================================================================
// Module dlatch generation completed successfully
// Elements processed: 9
// Inputs: 2, Outputs: 2
// ====================================================================
