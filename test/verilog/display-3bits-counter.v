// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_3bits_counter
// Generated: Fri Sep 26 14:36:21 2025
// Target FPGA: Generic-Small
// Resource Usage: 30/1000 LUTs, 38/1000 FFs, 11/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module display_3bits_counter (
    // ========= Input Ports =========
    input wire input_clock1_1,
    input wire input_push_button2_btn_2,

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_3,
    output wire output_7_segment_display1_f_upper_left_4,
    output wire output_7_segment_display1_e_lower_left_5,
    output wire output_7_segment_display1_d_bottom_6,
    output wire output_7_segment_display1_a_top_7,
    output wire output_7_segment_display1_b_upper_right_8,
    output wire output_7_segment_display1_dp_dot_9,
    output wire output_7_segment_display1_c_lower_right_10
);

    // ========= Internal Signals =========
// ============== BEGIN IC: DISPLAY-3BITS ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
    wire ic_display_3bits_ic_display_3bits_node_11;
    wire ic_display_3bits_ic_display_3bits_or_12;
    wire ic_display_3bits_ic_display_3bits_or_13;
    wire ic_display_3bits_ic_display_3bits_and_14;
    wire ic_display_3bits_ic_display_3bits_or_15;
    wire ic_display_3bits_ic_display_3bits_not_16;
    wire ic_display_3bits_ic_display_3bits_and_17;
    wire ic_display_3bits_ic_display_3bits_and_18;
    wire ic_display_3bits_ic_display_3bits_and_19;
    wire ic_display_3bits_ic_display_3bits_xnor_20;
    wire ic_display_3bits_ic_display_3bits_and_21;
    wire ic_display_3bits_ic_display_3bits_or_22;
    wire ic_display_3bits_ic_display_3bits_nand_23;
    wire ic_display_3bits_ic_display_3bits_or_24;
    wire ic_display_3bits_ic_display_3bits_or_25;
    wire ic_display_3bits_ic_display_3bits_node_26;
    wire ic_display_3bits_ic_display_3bits_node_27;
    wire ic_display_3bits_ic_display_3bits_node_28;
    wire ic_display_3bits_ic_display_3bits_node_29;
    wire ic_display_3bits_ic_display_3bits_node_30;
    wire ic_display_3bits_ic_display_3bits_node_31;
    wire ic_display_3bits_ic_display_3bits_node_32;
    wire ic_display_3bits_ic_display_3bits_node_33;
    wire ic_display_3bits_ic_display_3bits_not_34;
    wire ic_display_3bits_ic_display_3bits_and_35;
    wire ic_display_3bits_ic_display_3bits_and_36;
    wire ic_display_3bits_ic_display_3bits_and_37;
    wire ic_display_3bits_ic_display_3bits_and_38;
    wire ic_display_3bits_ic_display_3bits_node_39;
    wire ic_display_3bits_ic_display_3bits_or_40;
    wire ic_display_3bits_ic_display_3bits_node_41;
    wire ic_display_3bits_ic_display_3bits_node_42;
    wire ic_display_3bits_ic_display_3bits_xnor_43;
    wire ic_display_3bits_ic_display_3bits_gnd_44;
    wire ic_display_3bits_ic_display_3bits_or_45;
    wire ic_display_3bits_ic_display_3bits_and_46;
    wire ic_display_3bits_ic_display_3bits_not_47;
    wire ic_display_3bits_ic_display_3bits_or_48;
    wire ic_display_3bits_ic_display_3bits_gnd_49;
    assign ic_display_3bits_ic_display_3bits_node_11 = ic_display_3bits_ic_display_3bits_gnd_44; // Node
    assign ic_display_3bits_ic_display_3bits_or_12 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Or
    assign ic_display_3bits_ic_display_3bits_or_13 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42); // Or
    assign ic_display_3bits_ic_display_3bits_and_14 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_or_15 = (((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Or
    assign ic_display_3bits_ic_display_3bits_not_16 = ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41); // Not
    assign ic_display_3bits_ic_display_3bits_and_17 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)); // And
    assign ic_display_3bits_ic_display_3bits_and_18 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_and_19 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_xnor_20 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Xnor
    assign ic_display_3bits_ic_display_3bits_and_21 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_or_22 = ((~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Or
    assign ic_display_3bits_ic_display_3bits_nand_23 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Nand
    assign ic_display_3bits_ic_display_3bits_or_24 = (((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | (ic_display_3bits_ic_display_3bits_node_42 & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Or
    assign ic_display_3bits_ic_display_3bits_or_25 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) | (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // Or
    assign ic_display_3bits_ic_display_3bits_node_26 = (((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | (ic_display_3bits_ic_display_3bits_node_42 & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Node
    assign ic_display_3bits_ic_display_3bits_node_27 = ((~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Node
    assign ic_display_3bits_ic_display_3bits_node_28 = (((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Node
    assign ic_display_3bits_ic_display_3bits_node_29 = ((~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Node
    assign ic_display_3bits_ic_display_3bits_node_30 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39) | (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)); // Node
    assign ic_display_3bits_ic_display_3bits_node_31 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) | (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // Node
    assign ic_display_3bits_ic_display_3bits_node_32 = ic_display_3bits_ic_display_3bits_node_11; // Node
    assign ic_display_3bits_ic_display_3bits_node_33 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Node
    assign ic_display_3bits_ic_display_3bits_not_34 = ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Not
    assign ic_display_3bits_ic_display_3bits_and_35 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_and_36 = (ic_display_3bits_ic_display_3bits_node_42 & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_and_37 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_and_38 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_node_39 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_or_40 = ((~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Or
    assign ic_display_3bits_ic_display_3bits_node_41 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_node_42 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_xnor_43 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Xnor
    assign ic_display_3bits_ic_display_3bits_or_45 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39) | (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)); // Or
    assign ic_display_3bits_ic_display_3bits_and_46 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)); // And
    assign ic_display_3bits_ic_display_3bits_not_47 = ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42); // Not
    assign ic_display_3bits_ic_display_3bits_or_48 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41); // Or
// ============== END IC: DISPLAY-3BITS ==============
    wire node_50;
    reg t_flip_flop_51_0_q = 1'b0;
    reg t_flip_flop_51_1_q = 1'b0;
    wire node_52;
    reg t_flip_flop_53_0_q = 1'b0;
    reg t_flip_flop_53_1_q = 1'b0;
    wire node_54;
    reg t_flip_flop_55_0_q = 1'b0;
    reg t_flip_flop_55_1_q = 1'b0;
    wire node_56;
    wire node_57;
    wire node_58;
    wire node_59;
    wire node_60;
    wire node_61;

    // ========= Logic Assignments =========
    assign ic_display_3bits_ic_display_3bits_node_11 = ic_display_3bits_ic_display_3bits_gnd_44; // Node
    assign ic_display_3bits_ic_display_3bits_or_12 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Or
    assign ic_display_3bits_ic_display_3bits_or_13 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42); // Or
    assign ic_display_3bits_ic_display_3bits_and_14 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_or_15 = (((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Or
    assign ic_display_3bits_ic_display_3bits_not_16 = ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41); // Not
    assign ic_display_3bits_ic_display_3bits_and_17 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)); // And
    assign ic_display_3bits_ic_display_3bits_and_18 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_and_19 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_xnor_20 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Xnor
    assign ic_display_3bits_ic_display_3bits_and_21 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_or_22 = ((~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Or
    assign ic_display_3bits_ic_display_3bits_nand_23 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Nand
    assign ic_display_3bits_ic_display_3bits_or_24 = (((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | (ic_display_3bits_ic_display_3bits_node_42 & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Or
    assign ic_display_3bits_ic_display_3bits_or_25 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) | (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // Or
    assign ic_display_3bits_ic_display_3bits_node_26 = (((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | (ic_display_3bits_ic_display_3bits_node_42 & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Node
    assign ic_display_3bits_ic_display_3bits_node_27 = ((~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Node
    assign ic_display_3bits_ic_display_3bits_node_28 = (((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Node
    assign ic_display_3bits_ic_display_3bits_node_29 = ((~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Node
    assign ic_display_3bits_ic_display_3bits_node_30 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39) | (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)); // Node
    assign ic_display_3bits_ic_display_3bits_node_31 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) | (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // Node
    assign ic_display_3bits_ic_display_3bits_node_32 = ic_display_3bits_ic_display_3bits_node_11; // Node
    assign ic_display_3bits_ic_display_3bits_node_33 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Node
    assign ic_display_3bits_ic_display_3bits_not_34 = ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Not
    assign ic_display_3bits_ic_display_3bits_and_35 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_and_36 = (ic_display_3bits_ic_display_3bits_node_42 & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_and_37 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_and_38 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)); // And
    assign ic_display_3bits_ic_display_3bits_node_39 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_or_40 = ((~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39)) | ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39))); // Or
    assign ic_display_3bits_ic_display_3bits_node_41 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_node_42 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_xnor_43 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39); // Xnor
    assign ic_display_3bits_ic_display_3bits_or_45 = ((ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) ^ ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_39) | (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)); // Or
    assign ic_display_3bits_ic_display_3bits_and_46 = (~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41) & (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42)); // And
    assign ic_display_3bits_ic_display_3bits_not_47 = ~(ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_42); // Not
    assign ic_display_3bits_ic_display_3bits_or_48 = (ic_display_3bits_ic_display_3bits_gnd_49 | ic_display_3bits_ic_display_3bits_node_41); // Or
    assign node_61 = input_clock1_1; // Node
    assign node_60 = t_flip_flop_53_1_q; // Node
    assign node_59 = t_flip_flop_51_1_q; // Node
    assign node_58 = input_clock1_1; // Node
    assign node_57 = t_flip_flop_55_1_q; // Node
    assign node_56 = t_flip_flop_55_1_q; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge t_flip_flop_53_1_q) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_55_0_q <= t_flip_flop_55_1_q;
                t_flip_flop_55_1_q <= t_flip_flop_55_0_q;
            end
            // else hold
        end
    end

    assign node_54 = input_push_button2_btn_2; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge t_flip_flop_51_1_q) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_53_0_q <= t_flip_flop_53_1_q;
                t_flip_flop_53_1_q <= t_flip_flop_53_0_q;
            end
            // else hold
        end
    end

    assign node_52 = input_push_button2_btn_2; // Node
    // T FlipFlop: T-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            if (input_push_button2_btn_2) begin // toggle
                t_flip_flop_51_0_q <= t_flip_flop_51_1_q;
                t_flip_flop_51_1_q <= t_flip_flop_51_0_q;
            end
            // else hold
        end
    end

    assign node_50 = input_clock1_1; // Node

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_3 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_4 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_5 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_6 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_8 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_9 = input_clock1_1; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_10 = 1'b0; // 7-Segment Display

endmodule // display_3bits_counter

// ====================================================================
// Module display_3bits_counter generation completed successfully
// Elements processed: 16
// Inputs: 2, Outputs: 8
// Warnings: 1
//   IC DISPLAY-3BITS output 6 is not connected
// ====================================================================
