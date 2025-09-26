// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: tflipflop
// Generated: Fri Sep 26 19:56:56 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 35/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module tflipflop (
    // ========= Input Ports =========
    input wire input_push_button1_t_1,
    input wire input_clock2_c_2,
    input wire input_input_switch3__preset_3,
    input wire input_input_switch4__clear_4,

    // ========= Output Ports =========
    output wire output_led1_q_0_5,
    output wire output_led2_q_0_6
);

    // ========= Internal Signals =========
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
    wire ic_dflipflop_ic_node_15_0;
    wire ic_dflipflop_ic_not_16_0;
    wire ic_dflipflop_ic_not_17_0;
    wire ic_dflipflop_ic_node_18_0;
    wire ic_dflipflop_ic_node_19_0;
    wire ic_dflipflop_ic_node_20_0;
    wire ic_dflipflop_ic_node_21_0;
    wire ic_dflipflop_ic_nand_22_0;
    wire ic_dflipflop_ic_not_23_0;
    wire ic_dflipflop_ic_node_24_0;
    wire ic_dflipflop_ic_node_25_0;
    wire ic_dflipflop_ic_node_26_0;
    wire ic_dflipflop_ic_node_27_0;
    wire ic_dflipflop_ic_nand_28_0;
    wire ic_dflipflop_ic_nand_29_0;
    wire ic_dflipflop_ic_node_30_0;
    wire ic_dflipflop_ic_nand_31_0;
    wire ic_dflipflop_ic_nand_32_0;
    wire ic_dflipflop_ic_node_33_0;
    wire ic_dflipflop_ic_nand_34_0;
    wire ic_dflipflop_ic_nand_35_0;
    wire ic_dflipflop_ic_node_36_0;
    wire ic_dflipflop_ic_nand_37_0;
    wire ic_dflipflop_ic_node_38_0;
    wire ic_dflipflop_ic_node_39_0;
// ============== END IC: DFLIPFLOP ==============
    wire node_40;
    wire node_41;
    wire not_42;
    wire and_43;
    wire or_44;
    wire node_45;
    wire and_46;

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_led1_q_0_5_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock2_c_2 or negedge input_input_switch3__preset_3) begin
        if (!input_input_switch3__preset_3) begin
            output_led1_q_0_5_behavioral_reg <= 1'b1; // Asynchronous preset
        end else begin // Synchronous operation
            output_led1_q_0_5_behavioral_reg <= input_push_button1_t_1;
        end
    end

    assign output_led1_q_0_5 = output_led1_q_0_5_behavioral_reg; // Connect behavioral register to output
    assign output_led2_q_0_6 = ~output_led1_q_0_5_behavioral_reg; // Complementary output

    // ========= Output Assignments =========
    assign output_led1_q_0_5 = ic_dflipflop_ic_node_38_0; // LED
    assign output_led2_q_0_6 = ic_dflipflop_ic_node_39_0; // LED

endmodule // tflipflop

// ====================================================================
// Module tflipflop generation completed successfully
// Elements processed: 14
// Inputs: 4, Outputs: 2
// ====================================================================
