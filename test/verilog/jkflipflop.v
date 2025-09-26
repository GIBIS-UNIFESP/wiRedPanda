// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: jkflipflop
// Generated: Fri Sep 26 18:49:28 2025
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
    wire ic_dflipflop_ic_node_53_0;
    wire ic_dflipflop_ic_not_54_0;
    wire ic_dflipflop_ic_not_55_0;
    wire ic_dflipflop_ic_node_56_0;
    wire ic_dflipflop_ic_node_57_0;
    wire ic_dflipflop_ic_node_58_0;
    wire ic_dflipflop_ic_node_59_0;
    wire ic_dflipflop_ic_nand_60_0;
    wire ic_dflipflop_ic_not_61_0;
    wire ic_dflipflop_ic_node_62_0;
    wire ic_dflipflop_ic_node_63_0;
    wire ic_dflipflop_ic_node_64_0;
    wire ic_dflipflop_ic_node_65_0;
    wire ic_dflipflop_ic_nand_66_0;
    wire ic_dflipflop_ic_nand_67_0;
    wire ic_dflipflop_ic_node_68_0;
    wire ic_dflipflop_ic_nand_69_0;
    wire ic_dflipflop_ic_nand_70_0;
    wire ic_dflipflop_ic_node_71_0;
    wire ic_dflipflop_ic_nand_72_0;
    wire ic_dflipflop_ic_nand_73_0;
    wire ic_dflipflop_ic_node_74_0;
    wire ic_dflipflop_ic_nand_75_0;
    wire ic_dflipflop_ic_node_76_0;
    wire ic_dflipflop_ic_node_77_0;
    assign or_52 = (and_51 | and_50); // Or
    assign and_50 = (node_49 & node_47); // And
    assign node_47 = node_44; // Node
    assign node_49 = input_input_switch5_k_5; // Node
    assign and_51 = (node_46 & not_48); // And
    assign not_48 = ~input_input_switch4_j_4; // Not
    assign node_46 = node_45; // Node
    assign node_43 = input_clock1_c_1; // Node
    assign node_42 = input_input_switch3__clear_3; // Node
    assign node_41 = input_input_switch2__preset_2; // Node

    // ========= Output Assignments =========
    assign output_led1_q_0_6 = ic_dflipflop_ic_node_76_0; // LED
    assign output_led2_q_0_7 = ic_dflipflop_ic_node_77_0; // LED

endmodule // jkflipflop

// ====================================================================
// Module jkflipflop generation completed successfully
// Elements processed: 20
// Inputs: 5, Outputs: 2
// ====================================================================
