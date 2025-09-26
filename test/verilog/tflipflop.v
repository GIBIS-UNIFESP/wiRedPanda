// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: tflipflop
// Generated: Fri Sep 26 18:49:32 2025
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
    wire ic_dflipflop_ic_node_47_0;
    wire ic_dflipflop_ic_not_48_0;
    wire ic_dflipflop_ic_not_49_0;
    wire ic_dflipflop_ic_node_50_0;
    wire ic_dflipflop_ic_node_51_0;
    wire ic_dflipflop_ic_node_52_0;
    wire ic_dflipflop_ic_node_53_0;
    wire ic_dflipflop_ic_nand_54_0;
    wire ic_dflipflop_ic_not_55_0;
    wire ic_dflipflop_ic_node_56_0;
    wire ic_dflipflop_ic_node_57_0;
    wire ic_dflipflop_ic_node_58_0;
    wire ic_dflipflop_ic_node_59_0;
    wire ic_dflipflop_ic_nand_60_0;
    wire ic_dflipflop_ic_nand_61_0;
    wire ic_dflipflop_ic_node_62_0;
    wire ic_dflipflop_ic_nand_63_0;
    wire ic_dflipflop_ic_nand_64_0;
    wire ic_dflipflop_ic_node_65_0;
    wire ic_dflipflop_ic_nand_66_0;
    wire ic_dflipflop_ic_nand_67_0;
    wire ic_dflipflop_ic_node_68_0;
    wire ic_dflipflop_ic_nand_69_0;
    wire ic_dflipflop_ic_node_70_0;
    wire ic_dflipflop_ic_node_71_0;
    assign or_44 = (and_43 | and_46); // Or
    assign and_46 = (node_45 & ic_dflipflop_ic_node_71_0); // And
    assign node_45 = input_push_button1_t_1; // Node
    assign and_43 = (node_41 & not_42); // And
    assign not_42 = ~input_push_button1_t_1; // Not
    assign node_41 = node_40; // Node

    // ========= Output Assignments =========
    assign output_led1_q_0_5 = ic_dflipflop_ic_node_70_0; // LED
    assign output_led2_q_0_6 = ic_dflipflop_ic_node_71_0; // LED

endmodule // tflipflop

// ====================================================================
// Module tflipflop generation completed successfully
// Elements processed: 14
// Inputs: 4, Outputs: 2
// ====================================================================
