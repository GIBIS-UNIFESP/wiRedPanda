// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: jkflipflop_fixed
// Generated: Fri Sep 26 20:59:38 2025
// Target FPGA: Generic-Small
// Resource Usage: 24/1000 LUTs, 35/1000 FFs, 7/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module jkflipflop_fixed (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_q_0_1,
    output wire output_led2_q_0_2
);

    // ========= Internal Signals =========
// ============== BEGIN IC: DFLIPFLOP ==============
// IC inputs: 4, IC outputs: 2
// Nesting depth: 0
    wire ic_dflipflop_ic_node_11_0;
    wire ic_dflipflop_ic_not_12_0;
    wire ic_dflipflop_ic_not_13_0;
    wire ic_dflipflop_ic_node_14_0;
    wire ic_dflipflop_ic_node_15_0;
    wire ic_dflipflop_ic_node_16_0;
    wire ic_dflipflop_ic_node_17_0;
    wire ic_dflipflop_ic_nand_18_0;
    wire ic_dflipflop_ic_not_19_0;
    wire ic_dflipflop_ic_node_20_0;
    wire ic_dflipflop_ic_node_21_0;
    wire ic_dflipflop_ic_node_22_0;
    wire ic_dflipflop_ic_node_23_0;
    wire ic_dflipflop_ic_nand_24_0;
    wire ic_dflipflop_ic_nand_25_0;
    wire ic_dflipflop_ic_node_26_0;
    wire ic_dflipflop_ic_nand_27_0;
    wire ic_dflipflop_ic_nand_28_0;
    wire ic_dflipflop_ic_node_29_0;
    wire ic_dflipflop_ic_nand_30_0;
    wire ic_dflipflop_ic_nand_31_0;
    wire ic_dflipflop_ic_node_32_0;
    wire ic_dflipflop_ic_nand_33_0;
    wire ic_dflipflop_ic_node_34_0;
    wire ic_dflipflop_ic_node_35_0;
// ============== END IC: DFLIPFLOP ==============
    wire node_36;
    wire node_37;
    wire node_38;
    wire node_39;
    wire node_40;
    wire node_41;
    wire node_42;
    wire not_43;
    wire node_44;
    wire and_45;
    wire and_46;
    wire or_47;

    // ========= Logic Assignments =========
    wire ic_dflipflop_ic_node_48_0;
    wire ic_dflipflop_ic_not_49_0;
    wire ic_dflipflop_ic_not_50_0;
    wire ic_dflipflop_ic_node_51_0;
    wire ic_dflipflop_ic_node_52_0;
    wire ic_dflipflop_ic_node_53_0;
    wire ic_dflipflop_ic_node_54_0;
    wire ic_dflipflop_ic_nand_55_0;
    wire ic_dflipflop_ic_not_56_0;
    wire ic_dflipflop_ic_node_57_0;
    wire ic_dflipflop_ic_node_58_0;
    wire ic_dflipflop_ic_node_59_0;
    wire ic_dflipflop_ic_node_60_0;
    wire ic_dflipflop_ic_nand_61_0;
    wire ic_dflipflop_ic_nand_62_0;
    wire ic_dflipflop_ic_node_63_0;
    wire ic_dflipflop_ic_nand_64_0;
    wire ic_dflipflop_ic_nand_65_0;
    wire ic_dflipflop_ic_node_66_0;
    wire ic_dflipflop_ic_nand_67_0;
    wire ic_dflipflop_ic_nand_68_0;
    wire ic_dflipflop_ic_node_69_0;
    wire ic_dflipflop_ic_nand_70_0;
    wire ic_dflipflop_ic_node_71_0;
    wire ic_dflipflop_ic_node_72_0;
    assign or_47 = (and_46 | and_45); // Or
    assign and_45 = (node_44 & node_42); // And
    assign node_42 = node_39; // Node
    assign node_44 = 1'b0; // Node
    assign and_46 = (node_41 & not_43); // And
    assign not_43 = ~1'b0; // Not
    assign node_41 = node_40; // Node
    assign node_38 = 1'b0; // Node
    assign node_37 = 1'b0; // Node
    assign node_36 = 1'b0; // Node

    // ========= Output Assignments =========
    assign output_led1_q_0_1 = ic_dflipflop_ic_node_71_0; // LED
    assign output_led2_q_0_2 = ic_dflipflop_ic_node_72_0; // LED

endmodule // jkflipflop_fixed

// ====================================================================
// Module jkflipflop_fixed generation completed successfully
// Elements processed: 20
// Inputs: 0, Outputs: 2
// ====================================================================
