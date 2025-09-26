// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop2
// Generated: Fri Sep 26 19:56:46 2025
// Target FPGA: Generic-Small
// Resource Usage: 28/1000 LUTs, 36/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dflipflop2 (
    // ========= Input Ports =========
    input wire input_clock1_1,
    input wire input_input_switch2_2,

    // ========= Output Ports =========
    output wire output_led1_0_3,
    output wire output_led2_0_4
);

    // ========= Internal Signals =========
    wire not_5;
    wire node_6;
    wire node_7;
    wire not_8;
    wire not_9;
    wire node_10;
    wire nand_11;
    wire node_12;
    wire nand_13;
    wire nand_14;
    wire node_15;
    wire node_16;
    wire nand_17;
    wire node_18;
    wire node_19;
    wire nand_20;
    wire nand_21;
    reg d_flip_flop_22_0_q = 1'b0;
    reg d_flip_flop_22_1_q = 1'b0;
    wire nand_23;
    wire nand_24;

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_led1_0_3_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock1_1) begin
begin // Synchronous operation
            output_led1_0_3_behavioral_reg <= input_input_switch2_2;
        end
    end

    assign output_led1_0_3 = output_led1_0_3_behavioral_reg; // Connect behavioral register to output
    assign output_led2_0_4 = ~output_led1_0_3_behavioral_reg; // Complementary output

    // ========= Output Assignments =========
    assign output_led1_0_3 = d_flip_flop_22_0_q; // LED
    assign output_led2_0_4 = nand_21; // LED

endmodule // dflipflop2

// ====================================================================
// Module dflipflop2 generation completed successfully
// Elements processed: 24
// Inputs: 2, Outputs: 2
// ====================================================================
