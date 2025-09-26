// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop
// Generated: Fri Sep 26 19:56:45 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

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

    // ========= Internal Signals =========
    wire not_7;
    wire not_8;
    wire node_9;
    wire node_10;
    wire node_11;
    wire nand_12;
    wire not_13;
    wire node_14;
    wire node_15;
    wire nand_16;
    wire nand_17;
    wire node_18;
    wire nand_19;
    wire nand_20;
    wire node_21;
    wire nand_22;
    wire nand_23;
    wire node_24;
    wire nand_25;

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

    assign output_led1_0_5 = output_led1_0_5_behavioral_reg; // Connect behavioral register to output
    assign output_led2_0_6 = ~output_led1_0_5_behavioral_reg; // Complementary output

    // ========= Output Assignments =========
    assign output_led1_0_5 = nand_22; // LED
    assign output_led2_0_6 = nand_25; // LED

endmodule // dflipflop

// ====================================================================
// Module dflipflop generation completed successfully
// Elements processed: 25
// Inputs: 4, Outputs: 2
// ====================================================================
