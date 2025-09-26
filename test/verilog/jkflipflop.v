// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: jkflipflop
// Generated: Fri Sep 26 19:56:52 2025
// Target FPGA: Generic-Small
// Resource Usage: 24/1000 LUTs, 35/1000 FFs, 7/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module jkflipflop (
    // ========= Input Ports =========
    input wire input_clock1_c_1,
    input wire input_input_switch2__preset_2,
    input wire input_input_switch3__clear_3,
    input wire input_input_switch4_j_4,
    input wire input_input_switch5_k_5,

    // ========= Output Ports =========
    output wire output_led1_q_0_6,
    output wire output_led2_q_0_7
);

    // ========= Internal Signals =========
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
    wire ic_dflipflop_ic_node_16_0;
    wire ic_dflipflop_ic_not_17_0;
    wire ic_dflipflop_ic_not_18_0;
    wire ic_dflipflop_ic_node_19_0;
    wire ic_dflipflop_ic_node_20_0;
    wire ic_dflipflop_ic_node_21_0;
    wire ic_dflipflop_ic_node_22_0;
    wire ic_dflipflop_ic_nand_23_0;
    wire ic_dflipflop_ic_not_24_0;
    wire ic_dflipflop_ic_node_25_0;
    wire ic_dflipflop_ic_node_26_0;
    wire ic_dflipflop_ic_node_27_0;
    wire ic_dflipflop_ic_node_28_0;
    wire ic_dflipflop_ic_nand_29_0;
    wire ic_dflipflop_ic_nand_30_0;
    wire ic_dflipflop_ic_node_31_0;
    wire ic_dflipflop_ic_nand_32_0;
    wire ic_dflipflop_ic_nand_33_0;
    wire ic_dflipflop_ic_node_34_0;
    wire ic_dflipflop_ic_nand_35_0;
    wire ic_dflipflop_ic_nand_36_0;
    wire ic_dflipflop_ic_node_37_0;
    wire ic_dflipflop_ic_nand_38_0;
    wire ic_dflipflop_ic_node_39_0;
    wire ic_dflipflop_ic_node_40_0;
// ============== END IC: DFLIPFLOP ==============
    wire node_41;
    wire node_42;
    wire node_43;
    wire node_44;
    wire node_45;
    wire node_46;
    wire node_47;
    wire not_48;
    wire node_49;
    wire and_50;
    wire and_51;
    wire or_52;

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_led1_q_0_6_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock1_c_1 or negedge input_input_switch2__preset_2) begin
        if (!input_input_switch2__preset_2) begin
            output_led1_q_0_6_behavioral_reg <= 1'b1; // Asynchronous preset
        end else begin // Synchronous operation
            output_led1_q_0_6_behavioral_reg <= input_input_switch2__preset_2;
        end
    end

    assign output_led1_q_0_6 = output_led1_q_0_6_behavioral_reg; // Connect behavioral register to output
    assign output_led2_q_0_7 = ~output_led1_q_0_6_behavioral_reg; // Complementary output

    // ========= Output Assignments =========
    assign output_led1_q_0_6 = ic_dflipflop_ic_node_39_0; // LED
    assign output_led2_q_0_7 = ic_dflipflop_ic_node_40_0; // LED

endmodule // jkflipflop

// ====================================================================
// Module jkflipflop generation completed successfully
// Elements processed: 20
// Inputs: 5, Outputs: 2
// ====================================================================
