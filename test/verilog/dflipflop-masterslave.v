// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop_masterslave
// Generated: Fri Sep 26 19:56:45 2025
// Target FPGA: Generic-Small
// Resource Usage: 10/1000 LUTs, 35/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dflipflop_masterslave (
    // ========= Input Ports =========
    input wire input_clock1_clk_1,
    input wire input_push_button2_d_2,

    // ========= Output Ports =========
    output wire output_led1_q_0_3,
    output wire output_led2_q_0_4
);

    // ========= Internal Signals =========
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
    wire ic_dlatch_ic_node_9_0;
    wire ic_dlatch_ic_not_10_0;
    wire ic_dlatch_ic_node_11_0;
    wire ic_dlatch_ic_nand_12_0;
    wire ic_dlatch_ic_nand_13_0;
    wire ic_dlatch_ic_nand_14_0;
    wire ic_dlatch_ic_nand_15_0;
    wire ic_dlatch_ic_node_16_0;
    wire ic_dlatch_ic_node_17_0;
// ============== END IC: DLATCH ==============
// ============== BEGIN IC: DLATCH ==============
// IC inputs: 2, IC outputs: 2
// Nesting depth: 0
    wire ic_dlatch_ic_node_22_0;
    wire ic_dlatch_ic_not_23_0;
    wire ic_dlatch_ic_node_24_0;
    wire ic_dlatch_ic_nand_25_0;
    wire ic_dlatch_ic_nand_26_0;
    wire ic_dlatch_ic_nand_27_0;
    wire ic_dlatch_ic_nand_28_0;
    wire ic_dlatch_ic_node_29_0;
    wire ic_dlatch_ic_node_30_0;
// ============== END IC: DLATCH ==============
    wire not_31;
    wire node_32;
    wire node_33;

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_led1_q_0_3_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock1_clk_1) begin
begin // Synchronous operation
            output_led1_q_0_3_behavioral_reg <= input_push_button2_d_2;
        end
    end

    assign output_led1_q_0_3 = output_led1_q_0_3_behavioral_reg; // Connect behavioral register to output
    assign output_led2_q_0_4 = ~output_led1_q_0_3_behavioral_reg; // Complementary output

    // ========= Output Assignments =========
    assign output_led1_q_0_3 = ic_dlatch_ic_node_29_0; // LED
    assign output_led2_q_0_4 = ic_dlatch_ic_node_30_0; // LED

endmodule // dflipflop_masterslave

// ====================================================================
// Module dflipflop_masterslave generation completed successfully
// Elements processed: 9
// Inputs: 2, Outputs: 2
// Warnings: 1
//   IC DLATCH output 1 is not connected
// ====================================================================
