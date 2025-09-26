// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits_counter
// Generated: Fri Sep 26 14:59:09 2025
// Target FPGA: Generic-Small
// Resource Usage: 42/1000 LUTs, 35/1000 FFs, 11/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module display_4bits_counter (
    // ========= Input Ports =========
    input wire input_push_button1_1,
    input wire input_clock2_2,

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
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_jkflipflop_node_11;
    wire ic_jkflipflop_ic_jkflipflop_node_12;
    wire ic_jkflipflop_ic_jkflipflop_node_14;
    wire ic_jkflipflop_ic_jkflipflop_node_15;
    wire ic_jkflipflop_ic_jkflipflop_node_16;
    wire ic_jkflipflop_ic_jkflipflop_node_17;
    wire ic_jkflipflop_ic_jkflipflop_not_19;
    wire ic_jkflipflop_ic_jkflipflop_and_20;
    wire ic_jkflipflop_ic_jkflipflop_node_21;
    wire ic_jkflipflop_ic_jkflipflop_or_22;
    wire ic_jkflipflop_ic_jkflipflop_node_23;
    wire ic_jkflipflop_ic_jkflipflop_node_24;
    wire ic_jkflipflop_ic_jkflipflop_node_25;
    wire ic_jkflipflop_ic_jkflipflop_node_26;
    wire ic_jkflipflop_ic_jkflipflop_and_27;
    wire ic_jkflipflop_ic_jkflipflop_node_28;
    wire ic_jkflipflop_ic_jkflipflop_node_29;
    assign ic_jkflipflop_ic_jkflipflop_node_11 = ic_jkflipflop_ic_jkflipflop_node_21; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_12 = ic_jkflipflop_ic_jkflipflop_node_23; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_14 = ic_jkflipflop_ic_jkflipflop_node_17; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_15 = ic_jkflipflop_ic_jkflipflop_node_16; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_16 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_17 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_19 = ~ic_jkflipflop_ic_jkflipflop_node_24; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_20 = (ic_jkflipflop_ic_jkflipflop_node_26 & ic_jkflipflop_ic_jkflipflop_node_15); // And
    assign ic_jkflipflop_ic_jkflipflop_node_21 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_22 = (ic_jkflipflop_ic_jkflipflop_and_27 | ic_jkflipflop_ic_jkflipflop_and_20); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_23 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_24 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_25 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_26 = ic_jkflipflop_ic_jkflipflop_node_25; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_27 = (ic_jkflipflop_ic_jkflipflop_node_14 & ic_jkflipflop_ic_jkflipflop_not_19); // And
    assign ic_jkflipflop_ic_jkflipflop_node_28 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_29 = 1'b0; // Node
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_jkflipflop_node_30;
    wire ic_jkflipflop_ic_jkflipflop_node_31;
    wire ic_jkflipflop_ic_jkflipflop_node_32;
    wire ic_jkflipflop_ic_jkflipflop_node_33;
    wire ic_jkflipflop_ic_jkflipflop_node_34;
    wire ic_jkflipflop_ic_jkflipflop_node_35;
    wire ic_jkflipflop_ic_jkflipflop_node_36;
    wire ic_jkflipflop_ic_jkflipflop_node_37;
    wire ic_jkflipflop_ic_jkflipflop_not_38;
    wire ic_jkflipflop_ic_jkflipflop_and_39;
    wire ic_jkflipflop_ic_jkflipflop_node_40;
    wire ic_jkflipflop_ic_jkflipflop_or_41;
    wire ic_jkflipflop_ic_jkflipflop_node_42;
    wire ic_jkflipflop_ic_jkflipflop_node_43;
    wire ic_jkflipflop_ic_jkflipflop_node_44;
    wire ic_jkflipflop_ic_jkflipflop_node_45;
    wire ic_jkflipflop_ic_jkflipflop_and_46;
    wire ic_jkflipflop_ic_jkflipflop_node_47;
    wire ic_jkflipflop_ic_jkflipflop_node_48;
    assign ic_jkflipflop_ic_jkflipflop_node_30 = ic_jkflipflop_ic_jkflipflop_node_40; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_31 = ic_jkflipflop_ic_jkflipflop_node_42; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_32 = ic_jkflipflop_ic_jkflipflop_node_37; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_33 = ic_jkflipflop_ic_jkflipflop_node_36; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_34 = ic_jkflipflop_ic_jkflipflop_node_35; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_35 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_36 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_37 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_38 = ~ic_jkflipflop_ic_jkflipflop_node_43; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_39 = (ic_jkflipflop_ic_jkflipflop_node_45 & ic_jkflipflop_ic_jkflipflop_node_34); // And
    assign ic_jkflipflop_ic_jkflipflop_node_40 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_41 = (ic_jkflipflop_ic_jkflipflop_and_46 | ic_jkflipflop_ic_jkflipflop_and_39); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_42 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_43 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_44 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_45 = ic_jkflipflop_ic_jkflipflop_node_44; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_46 = (ic_jkflipflop_ic_jkflipflop_node_33 & ic_jkflipflop_ic_jkflipflop_not_38); // And
    assign ic_jkflipflop_ic_jkflipflop_node_47 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_48 = 1'b0; // Node
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_jkflipflop_node_49;
    wire ic_jkflipflop_ic_jkflipflop_node_50;
    wire ic_jkflipflop_ic_jkflipflop_node_51;
    wire ic_jkflipflop_ic_jkflipflop_node_52;
    wire ic_jkflipflop_ic_jkflipflop_node_53;
    wire ic_jkflipflop_ic_jkflipflop_node_54;
    wire ic_jkflipflop_ic_jkflipflop_node_55;
    wire ic_jkflipflop_ic_jkflipflop_node_56;
    wire ic_jkflipflop_ic_jkflipflop_not_57;
    wire ic_jkflipflop_ic_jkflipflop_and_58;
    wire ic_jkflipflop_ic_jkflipflop_node_59;
    wire ic_jkflipflop_ic_jkflipflop_or_60;
    wire ic_jkflipflop_ic_jkflipflop_node_61;
    wire ic_jkflipflop_ic_jkflipflop_node_62;
    wire ic_jkflipflop_ic_jkflipflop_node_63;
    wire ic_jkflipflop_ic_jkflipflop_node_64;
    wire ic_jkflipflop_ic_jkflipflop_and_65;
    wire ic_jkflipflop_ic_jkflipflop_node_66;
    wire ic_jkflipflop_ic_jkflipflop_node_67;
    assign ic_jkflipflop_ic_jkflipflop_node_49 = ic_jkflipflop_ic_jkflipflop_node_59; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_50 = ic_jkflipflop_ic_jkflipflop_node_61; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_51 = ic_jkflipflop_ic_jkflipflop_node_56; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_52 = ic_jkflipflop_ic_jkflipflop_node_55; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_53 = ic_jkflipflop_ic_jkflipflop_node_54; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_54 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_55 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_56 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_57 = ~ic_jkflipflop_ic_jkflipflop_node_62; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_58 = (ic_jkflipflop_ic_jkflipflop_node_64 & ic_jkflipflop_ic_jkflipflop_node_53); // And
    assign ic_jkflipflop_ic_jkflipflop_node_59 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_60 = (ic_jkflipflop_ic_jkflipflop_and_65 | ic_jkflipflop_ic_jkflipflop_and_58); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_61 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_62 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_63 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_64 = ic_jkflipflop_ic_jkflipflop_node_63; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_65 = (ic_jkflipflop_ic_jkflipflop_node_52 & ic_jkflipflop_ic_jkflipflop_not_57); // And
    assign ic_jkflipflop_ic_jkflipflop_node_66 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_67 = 1'b0; // Node
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_jkflipflop_node_68;
    wire ic_jkflipflop_ic_jkflipflop_node_69;
    wire ic_jkflipflop_ic_jkflipflop_node_70;
    wire ic_jkflipflop_ic_jkflipflop_node_71;
    wire ic_jkflipflop_ic_jkflipflop_node_72;
    wire ic_jkflipflop_ic_jkflipflop_node_73;
    wire ic_jkflipflop_ic_jkflipflop_node_74;
    wire ic_jkflipflop_ic_jkflipflop_node_75;
    wire ic_jkflipflop_ic_jkflipflop_not_76;
    wire ic_jkflipflop_ic_jkflipflop_and_77;
    wire ic_jkflipflop_ic_jkflipflop_node_78;
    wire ic_jkflipflop_ic_jkflipflop_or_79;
    wire ic_jkflipflop_ic_jkflipflop_node_80;
    wire ic_jkflipflop_ic_jkflipflop_node_81;
    wire ic_jkflipflop_ic_jkflipflop_node_82;
    wire ic_jkflipflop_ic_jkflipflop_node_83;
    wire ic_jkflipflop_ic_jkflipflop_and_84;
    wire ic_jkflipflop_ic_jkflipflop_node_85;
    wire ic_jkflipflop_ic_jkflipflop_node_86;
    assign ic_jkflipflop_ic_jkflipflop_node_68 = ic_jkflipflop_ic_jkflipflop_node_78; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_69 = ic_jkflipflop_ic_jkflipflop_node_80; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_70 = ic_jkflipflop_ic_jkflipflop_node_75; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_71 = ic_jkflipflop_ic_jkflipflop_node_74; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_72 = ic_jkflipflop_ic_jkflipflop_node_73; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_73 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_74 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_75 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_76 = ~ic_jkflipflop_ic_jkflipflop_node_81; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_77 = (ic_jkflipflop_ic_jkflipflop_node_83 & ic_jkflipflop_ic_jkflipflop_node_72); // And
    assign ic_jkflipflop_ic_jkflipflop_node_78 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_79 = (ic_jkflipflop_ic_jkflipflop_and_84 | ic_jkflipflop_ic_jkflipflop_and_77); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_80 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_81 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_82 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_83 = ic_jkflipflop_ic_jkflipflop_node_82; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_84 = (ic_jkflipflop_ic_jkflipflop_node_71 & ic_jkflipflop_ic_jkflipflop_not_76); // And
    assign ic_jkflipflop_ic_jkflipflop_node_85 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_86 = 1'b0; // Node
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: DISPLAY-4BITS ==============
// IC inputs: 4, IC outputs: 8
// Nesting depth: 0
    wire ic_display_4bits_ic_display_4bits_node_87;
    wire ic_display_4bits_ic_display_4bits_node_88;
    wire ic_display_4bits_ic_display_4bits_node_89;
    wire ic_display_4bits_ic_display_4bits_node_90;
    wire ic_display_4bits_ic_display_4bits_and_91;
    wire ic_display_4bits_ic_display_4bits_node_92;
    wire ic_display_4bits_ic_display_4bits_node_93;
    wire ic_display_4bits_ic_display_4bits_node_94;
    wire ic_display_4bits_ic_display_4bits_node_95;
    wire ic_display_4bits_ic_display_4bits_node_96;
    wire ic_display_4bits_ic_display_4bits_node_97;
    wire ic_display_4bits_ic_display_4bits_node_98;
    wire ic_display_4bits_ic_display_4bits_node_99;
    wire ic_display_4bits_ic_display_4bits_node_100;
    wire ic_display_4bits_ic_display_4bits_node_101;
    wire ic_display_4bits_ic_display_4bits_node_102;
    wire ic_display_4bits_ic_display_4bits_or_103;
    wire ic_display_4bits_ic_display_4bits_or_104;
    wire ic_display_4bits_ic_display_4bits_node_105;
    wire ic_display_4bits_ic_display_4bits_node_106;
    wire ic_display_4bits_ic_display_4bits_node_107;
    wire ic_display_4bits_ic_display_4bits_node_108;
    wire ic_display_4bits_ic_display_4bits_node_109;
    wire ic_display_4bits_ic_display_4bits_node_110;
    wire ic_display_4bits_ic_display_4bits_node_111;
    wire ic_display_4bits_ic_display_4bits_or_112;
    wire ic_display_4bits_ic_display_4bits_or_113;
    wire ic_display_4bits_ic_display_4bits_or_114;
    wire ic_display_4bits_ic_display_4bits_node_115;
    wire ic_display_4bits_ic_display_4bits_node_116;
    wire ic_display_4bits_ic_display_4bits_node_117;
    wire ic_display_4bits_ic_display_4bits_node_118;
    wire ic_display_4bits_ic_display_4bits_node_119;
    wire ic_display_4bits_ic_display_4bits_node_120;
    wire ic_display_4bits_ic_display_4bits_node_121;
    wire ic_display_4bits_ic_display_4bits_node_122;
    wire ic_display_4bits_ic_display_4bits_node_123;
    wire ic_display_4bits_ic_display_4bits_node_124;
    wire ic_display_4bits_ic_display_4bits_node_125;
    wire ic_display_4bits_ic_display_4bits_or_126;
    wire ic_display_4bits_ic_display_4bits_node_127;
    wire ic_display_4bits_ic_display_4bits_node_128;
    wire ic_display_4bits_ic_display_4bits_node_129;
    wire ic_display_4bits_ic_display_4bits_node_130;
    wire ic_display_4bits_ic_display_4bits_or_131;
    wire ic_display_4bits_ic_display_4bits_and_132;
    wire ic_display_4bits_ic_display_4bits_and_133;
    wire ic_display_4bits_ic_display_4bits_and_134;
    wire ic_display_4bits_ic_display_4bits_and_135;
    wire ic_display_4bits_ic_display_4bits_and_136;
    wire ic_display_4bits_ic_display_4bits_and_137;
    wire ic_display_4bits_ic_display_4bits_node_138;
    wire ic_display_4bits_ic_display_4bits_node_139;
    wire ic_display_4bits_ic_display_4bits_and_140;
    wire ic_display_4bits_ic_display_4bits_node_141;
    wire ic_display_4bits_ic_display_4bits_node_142;
    wire ic_display_4bits_ic_display_4bits_and_143;
    wire ic_display_4bits_ic_display_4bits_node_144;
    wire ic_display_4bits_ic_display_4bits_node_145;
    wire ic_display_4bits_ic_display_4bits_node_146;
    wire ic_display_4bits_ic_display_4bits_node_147;
    wire ic_display_4bits_ic_display_4bits_node_148;
    wire ic_display_4bits_ic_display_4bits_node_149;
    wire ic_display_4bits_ic_display_4bits_node_150;
    wire ic_display_4bits_ic_display_4bits_node_151;
    wire ic_display_4bits_ic_display_4bits_node_152;
    wire ic_display_4bits_ic_display_4bits_node_153;
    wire ic_display_4bits_ic_display_4bits_node_154;
    wire ic_display_4bits_ic_display_4bits_node_155;
    wire ic_display_4bits_ic_display_4bits_node_156;
    wire ic_display_4bits_ic_display_4bits_node_157;
    wire ic_display_4bits_ic_display_4bits_node_158;
    wire ic_display_4bits_ic_display_4bits_node_159;
    wire ic_display_4bits_ic_display_4bits_not_160;
    wire ic_display_4bits_ic_display_4bits_not_161;
    wire ic_display_4bits_ic_display_4bits_not_162;
    wire ic_display_4bits_ic_display_4bits_not_163;
    assign ic_display_4bits_ic_display_4bits_node_87 = ic_display_4bits_ic_display_4bits_node_105; // Node
    assign ic_display_4bits_ic_display_4bits_node_88 = ic_display_4bits_ic_display_4bits_node_87; // Node
    assign ic_display_4bits_ic_display_4bits_node_89 = ic_display_4bits_ic_display_4bits_node_97; // Node
    assign ic_display_4bits_ic_display_4bits_node_90 = ic_display_4bits_ic_display_4bits_node_102; // Node
    assign ic_display_4bits_ic_display_4bits_and_91 = (ic_display_4bits_ic_display_4bits_node_89 & ic_display_4bits_ic_display_4bits_node_90); // And
    assign ic_display_4bits_ic_display_4bits_node_92 = ic_display_4bits_ic_display_4bits_node_93; // Node
    assign ic_display_4bits_ic_display_4bits_node_93 = ic_display_4bits_ic_display_4bits_and_137; // Node
    assign ic_display_4bits_ic_display_4bits_node_94 = ic_display_4bits_ic_display_4bits_node_95; // Node
    assign ic_display_4bits_ic_display_4bits_node_95 = ic_display_4bits_ic_display_4bits_and_134; // Node
    assign ic_display_4bits_ic_display_4bits_node_96 = ic_display_4bits_ic_display_4bits_node_110; // Node
    assign ic_display_4bits_ic_display_4bits_node_97 = ic_display_4bits_ic_display_4bits_node_109; // Node
    assign ic_display_4bits_ic_display_4bits_node_98 = ic_display_4bits_ic_display_4bits_node_111; // Node
    assign ic_display_4bits_ic_display_4bits_node_99 = ic_display_4bits_ic_display_4bits_node_125; // Node
    assign ic_display_4bits_ic_display_4bits_node_100 = ic_display_4bits_ic_display_4bits_node_101; // Node
    assign ic_display_4bits_ic_display_4bits_node_101 = ic_display_4bits_ic_display_4bits_node_128; // Node
    assign ic_display_4bits_ic_display_4bits_node_102 = ic_display_4bits_ic_display_4bits_node_124; // Node
    assign ic_display_4bits_ic_display_4bits_or_103 = (ic_display_4bits_ic_display_4bits_node_94 | ic_display_4bits_ic_display_4bits_and_133 | ic_display_4bits_ic_display_4bits_node_88 | ic_display_4bits_ic_display_4bits_and_135); // Or
    assign ic_display_4bits_ic_display_4bits_or_104 = (ic_display_4bits_ic_display_4bits_node_92 | ic_display_4bits_ic_display_4bits_and_91 | ic_display_4bits_ic_display_4bits_and_133 | ic_display_4bits_ic_display_4bits_node_87); // Or
    assign ic_display_4bits_ic_display_4bits_node_105 = ic_display_4bits_ic_display_4bits_node_130; // Node
    assign ic_display_4bits_ic_display_4bits_node_106 = ic_display_4bits_ic_display_4bits_node_107; // Node
    assign ic_display_4bits_ic_display_4bits_node_107 = ic_display_4bits_ic_display_4bits_node_108; // Node
    assign ic_display_4bits_ic_display_4bits_node_108 = ic_display_4bits_ic_display_4bits_and_140; // Node
    assign ic_display_4bits_ic_display_4bits_node_109 = ic_display_4bits_ic_display_4bits_node_142; // Node
    assign ic_display_4bits_ic_display_4bits_node_110 = ic_display_4bits_ic_display_4bits_node_123; // Node
    assign ic_display_4bits_ic_display_4bits_node_111 = ic_display_4bits_ic_display_4bits_node_127; // Node
    assign ic_display_4bits_ic_display_4bits_or_112 = (ic_display_4bits_ic_display_4bits_node_109 | ic_display_4bits_ic_display_4bits_node_110 | ic_display_4bits_ic_display_4bits_node_111); // Or
    assign ic_display_4bits_ic_display_4bits_or_113 = (ic_display_4bits_ic_display_4bits_node_106 | ic_display_4bits_ic_display_4bits_node_94); // Or
    assign ic_display_4bits_ic_display_4bits_or_114 = (ic_display_4bits_ic_display_4bits_node_105 | ic_display_4bits_ic_display_4bits_node_107 | ic_display_4bits_ic_display_4bits_node_95 | ic_display_4bits_ic_display_4bits_and_135 | ic_display_4bits_ic_display_4bits_and_132); // Or
    assign ic_display_4bits_ic_display_4bits_node_115 = ic_display_4bits_ic_display_4bits_or_103; // Node
    assign ic_display_4bits_ic_display_4bits_node_116 = ic_display_4bits_ic_display_4bits_or_104; // Node
    assign ic_display_4bits_ic_display_4bits_node_117 = ic_display_4bits_ic_display_4bits_or_113; // Node
    assign ic_display_4bits_ic_display_4bits_node_118 = ic_display_4bits_ic_display_4bits_or_114; // Node
    assign ic_display_4bits_ic_display_4bits_node_119 = ic_display_4bits_ic_display_4bits_or_131; // Node
    assign ic_display_4bits_ic_display_4bits_node_120 = ic_display_4bits_ic_display_4bits_or_126; // Node
    assign ic_display_4bits_ic_display_4bits_node_121 = 1'b0; // Node
    assign ic_display_4bits_ic_display_4bits_node_122 = ic_display_4bits_ic_display_4bits_or_112; // Node
    assign ic_display_4bits_ic_display_4bits_node_123 = ic_display_4bits_ic_display_4bits_not_162; // Node
    assign ic_display_4bits_ic_display_4bits_node_124 = ic_display_4bits_ic_display_4bits_node_138; // Node
    assign ic_display_4bits_ic_display_4bits_node_125 = ic_display_4bits_ic_display_4bits_node_139; // Node
    assign ic_display_4bits_ic_display_4bits_or_126 = (ic_display_4bits_ic_display_4bits_and_136 | ic_display_4bits_ic_display_4bits_node_125 | ic_display_4bits_ic_display_4bits_node_93); // Or
    assign ic_display_4bits_ic_display_4bits_node_127 = ic_display_4bits_ic_display_4bits_node_141; // Node
    assign ic_display_4bits_ic_display_4bits_node_128 = ic_display_4bits_ic_display_4bits_node_129; // Node
    assign ic_display_4bits_ic_display_4bits_node_129 = ic_display_4bits_ic_display_4bits_node_158; // Node
    assign ic_display_4bits_ic_display_4bits_node_130 = ic_display_4bits_ic_display_4bits_node_156; // Node
    assign ic_display_4bits_ic_display_4bits_or_131 = (ic_display_4bits_ic_display_4bits_and_143 | ic_display_4bits_ic_display_4bits_node_130 | ic_display_4bits_ic_display_4bits_node_129 | ic_display_4bits_ic_display_4bits_node_108); // Or
    assign ic_display_4bits_ic_display_4bits_and_132 = (ic_display_4bits_ic_display_4bits_node_98 & ic_display_4bits_ic_display_4bits_and_133); // And
    assign ic_display_4bits_ic_display_4bits_and_133 = (ic_display_4bits_ic_display_4bits_node_97 & ic_display_4bits_ic_display_4bits_node_96); // And
    assign ic_display_4bits_ic_display_4bits_and_134 = (ic_display_4bits_ic_display_4bits_node_101 & ic_display_4bits_ic_display_4bits_node_102); // And
    assign ic_display_4bits_ic_display_4bits_and_135 = (ic_display_4bits_ic_display_4bits_node_99 & ic_display_4bits_ic_display_4bits_node_100); // And
    assign ic_display_4bits_ic_display_4bits_and_136 = (ic_display_4bits_ic_display_4bits_node_128 & ic_display_4bits_ic_display_4bits_node_127); // And
    assign ic_display_4bits_ic_display_4bits_and_137 = (ic_display_4bits_ic_display_4bits_node_123 & ic_display_4bits_ic_display_4bits_node_124); // And
    assign ic_display_4bits_ic_display_4bits_node_138 = ic_display_4bits_ic_display_4bits_not_163; // Node
    assign ic_display_4bits_ic_display_4bits_node_139 = ic_display_4bits_ic_display_4bits_not_160; // Node
    assign ic_display_4bits_ic_display_4bits_and_140 = (ic_display_4bits_ic_display_4bits_node_139 & ic_display_4bits_ic_display_4bits_node_138); // And
    assign ic_display_4bits_ic_display_4bits_node_141 = ic_display_4bits_ic_display_4bits_node_159; // Node
    assign ic_display_4bits_ic_display_4bits_node_142 = ic_display_4bits_ic_display_4bits_node_157; // Node
    assign ic_display_4bits_ic_display_4bits_and_143 = (ic_display_4bits_ic_display_4bits_node_142 & ic_display_4bits_ic_display_4bits_node_141); // And
    assign ic_display_4bits_ic_display_4bits_node_144 = ic_display_4bits_ic_display_4bits_node_98; // Node
    assign ic_display_4bits_ic_display_4bits_node_145 = ic_display_4bits_ic_display_4bits_node_100; // Node
    assign ic_display_4bits_ic_display_4bits_node_146 = ic_display_4bits_ic_display_4bits_node_96; // Node
    assign ic_display_4bits_ic_display_4bits_node_147 = ic_display_4bits_ic_display_4bits_node_89; // Node
    assign ic_display_4bits_ic_display_4bits_node_148 = ic_display_4bits_ic_display_4bits_node_99; // Node
    assign ic_display_4bits_ic_display_4bits_node_149 = ic_display_4bits_ic_display_4bits_not_161; // Node
    assign ic_display_4bits_ic_display_4bits_node_150 = ic_display_4bits_ic_display_4bits_node_88; // Node
    assign ic_display_4bits_ic_display_4bits_node_151 = ic_display_4bits_ic_display_4bits_node_90; // Node
    assign ic_display_4bits_ic_display_4bits_node_152 = 1'b0; // Node
    assign ic_display_4bits_ic_display_4bits_node_153 = 1'b0; // Node
    assign ic_display_4bits_ic_display_4bits_node_154 = 1'b0; // Node
    assign ic_display_4bits_ic_display_4bits_node_155 = 1'b1; // Node
    assign ic_display_4bits_ic_display_4bits_node_156 = ic_display_4bits_ic_display_4bits_node_153; // Node
    assign ic_display_4bits_ic_display_4bits_node_157 = ic_display_4bits_ic_display_4bits_node_152; // Node
    assign ic_display_4bits_ic_display_4bits_node_158 = ic_display_4bits_ic_display_4bits_node_154; // Node
    assign ic_display_4bits_ic_display_4bits_node_159 = ic_display_4bits_ic_display_4bits_node_155; // Node
    assign ic_display_4bits_ic_display_4bits_not_160 = ~ic_display_4bits_ic_display_4bits_node_157; // Not
    assign ic_display_4bits_ic_display_4bits_not_161 = ~ic_display_4bits_ic_display_4bits_node_156; // Not
    assign ic_display_4bits_ic_display_4bits_not_162 = ~ic_display_4bits_ic_display_4bits_node_158; // Not
    assign ic_display_4bits_ic_display_4bits_not_163 = ~ic_display_4bits_ic_display_4bits_node_159; // Not
// ============== END IC: DISPLAY-4BITS ==============
    wire not_164;
    wire node_165;
    wire node_166;
    wire and_167;
    wire not_168;
    wire node_169;
    wire node_170;
    wire node_171;
    wire node_172;
    wire node_173;
    wire node_174;
    wire node_175;
    wire node_176;
    wire node_177;
    wire node_178;
    wire node_179;
    wire node_180;
    wire node_181;

    // ========= Logic Assignments =========
    assign node_177 = node_172; // Node
    assign ic_display_4bits_ic_display_4bits_node_87 = ic_display_4bits_ic_display_4bits_node_105; // Node
    assign ic_display_4bits_ic_display_4bits_node_88 = ic_display_4bits_ic_display_4bits_node_87; // Node
    assign ic_display_4bits_ic_display_4bits_node_89 = ic_display_4bits_ic_display_4bits_node_97; // Node
    assign ic_display_4bits_ic_display_4bits_node_90 = ic_display_4bits_ic_display_4bits_node_102; // Node
    assign ic_display_4bits_ic_display_4bits_and_91 = (ic_display_4bits_ic_display_4bits_node_89 & ic_display_4bits_ic_display_4bits_node_90); // And
    assign ic_display_4bits_ic_display_4bits_node_92 = ic_display_4bits_ic_display_4bits_node_93; // Node
    assign ic_display_4bits_ic_display_4bits_node_93 = ic_display_4bits_ic_display_4bits_and_137; // Node
    assign ic_display_4bits_ic_display_4bits_node_94 = ic_display_4bits_ic_display_4bits_node_95; // Node
    assign ic_display_4bits_ic_display_4bits_node_95 = ic_display_4bits_ic_display_4bits_and_134; // Node
    assign ic_display_4bits_ic_display_4bits_node_96 = ic_display_4bits_ic_display_4bits_node_110; // Node
    assign ic_display_4bits_ic_display_4bits_node_97 = ic_display_4bits_ic_display_4bits_node_109; // Node
    assign ic_display_4bits_ic_display_4bits_node_98 = ic_display_4bits_ic_display_4bits_node_111; // Node
    assign ic_display_4bits_ic_display_4bits_node_99 = ic_display_4bits_ic_display_4bits_node_125; // Node
    assign ic_display_4bits_ic_display_4bits_node_100 = ic_display_4bits_ic_display_4bits_node_101; // Node
    assign ic_display_4bits_ic_display_4bits_node_101 = ic_display_4bits_ic_display_4bits_node_128; // Node
    assign ic_display_4bits_ic_display_4bits_node_102 = ic_display_4bits_ic_display_4bits_node_124; // Node
    assign ic_display_4bits_ic_display_4bits_or_103 = (ic_display_4bits_ic_display_4bits_node_94 | ic_display_4bits_ic_display_4bits_and_133 | ic_display_4bits_ic_display_4bits_node_88 | ic_display_4bits_ic_display_4bits_and_135); // Or
    assign ic_display_4bits_ic_display_4bits_or_104 = (ic_display_4bits_ic_display_4bits_node_92 | ic_display_4bits_ic_display_4bits_and_91 | ic_display_4bits_ic_display_4bits_and_133 | ic_display_4bits_ic_display_4bits_node_87); // Or
    assign ic_display_4bits_ic_display_4bits_node_105 = ic_display_4bits_ic_display_4bits_node_130; // Node
    assign ic_display_4bits_ic_display_4bits_node_106 = ic_display_4bits_ic_display_4bits_node_107; // Node
    assign ic_display_4bits_ic_display_4bits_node_107 = ic_display_4bits_ic_display_4bits_node_108; // Node
    assign ic_display_4bits_ic_display_4bits_node_108 = ic_display_4bits_ic_display_4bits_and_140; // Node
    assign ic_display_4bits_ic_display_4bits_node_109 = ic_display_4bits_ic_display_4bits_node_142; // Node
    assign ic_display_4bits_ic_display_4bits_node_110 = ic_display_4bits_ic_display_4bits_node_123; // Node
    assign ic_display_4bits_ic_display_4bits_node_111 = ic_display_4bits_ic_display_4bits_node_127; // Node
    assign ic_display_4bits_ic_display_4bits_or_112 = (ic_display_4bits_ic_display_4bits_node_109 | ic_display_4bits_ic_display_4bits_node_110 | ic_display_4bits_ic_display_4bits_node_111); // Or
    assign ic_display_4bits_ic_display_4bits_or_113 = (ic_display_4bits_ic_display_4bits_node_106 | ic_display_4bits_ic_display_4bits_node_94); // Or
    assign ic_display_4bits_ic_display_4bits_or_114 = (ic_display_4bits_ic_display_4bits_node_105 | ic_display_4bits_ic_display_4bits_node_107 | ic_display_4bits_ic_display_4bits_node_95 | ic_display_4bits_ic_display_4bits_and_135 | ic_display_4bits_ic_display_4bits_and_132); // Or
    assign ic_display_4bits_ic_display_4bits_node_115 = ic_display_4bits_ic_display_4bits_or_103; // Node
    assign ic_display_4bits_ic_display_4bits_node_116 = ic_display_4bits_ic_display_4bits_or_104; // Node
    assign ic_display_4bits_ic_display_4bits_node_117 = ic_display_4bits_ic_display_4bits_or_113; // Node
    assign ic_display_4bits_ic_display_4bits_node_118 = ic_display_4bits_ic_display_4bits_or_114; // Node
    assign ic_display_4bits_ic_display_4bits_node_119 = ic_display_4bits_ic_display_4bits_or_131; // Node
    assign ic_display_4bits_ic_display_4bits_node_120 = ic_display_4bits_ic_display_4bits_or_126; // Node
    assign ic_display_4bits_ic_display_4bits_node_121 = 1'b0; // Node
    assign ic_display_4bits_ic_display_4bits_node_122 = ic_display_4bits_ic_display_4bits_or_112; // Node
    assign ic_display_4bits_ic_display_4bits_node_123 = ic_display_4bits_ic_display_4bits_not_162; // Node
    assign ic_display_4bits_ic_display_4bits_node_124 = ic_display_4bits_ic_display_4bits_node_138; // Node
    assign ic_display_4bits_ic_display_4bits_node_125 = ic_display_4bits_ic_display_4bits_node_139; // Node
    assign ic_display_4bits_ic_display_4bits_or_126 = (ic_display_4bits_ic_display_4bits_and_136 | ic_display_4bits_ic_display_4bits_node_125 | ic_display_4bits_ic_display_4bits_node_93); // Or
    assign ic_display_4bits_ic_display_4bits_node_127 = ic_display_4bits_ic_display_4bits_node_141; // Node
    assign ic_display_4bits_ic_display_4bits_node_128 = ic_display_4bits_ic_display_4bits_node_129; // Node
    assign ic_display_4bits_ic_display_4bits_node_129 = ic_display_4bits_ic_display_4bits_node_158; // Node
    assign ic_display_4bits_ic_display_4bits_node_130 = ic_display_4bits_ic_display_4bits_node_156; // Node
    assign ic_display_4bits_ic_display_4bits_or_131 = (ic_display_4bits_ic_display_4bits_and_143 | ic_display_4bits_ic_display_4bits_node_130 | ic_display_4bits_ic_display_4bits_node_129 | ic_display_4bits_ic_display_4bits_node_108); // Or
    assign ic_display_4bits_ic_display_4bits_and_132 = (ic_display_4bits_ic_display_4bits_node_98 & ic_display_4bits_ic_display_4bits_and_133); // And
    assign ic_display_4bits_ic_display_4bits_and_133 = (ic_display_4bits_ic_display_4bits_node_97 & ic_display_4bits_ic_display_4bits_node_96); // And
    assign ic_display_4bits_ic_display_4bits_and_134 = (ic_display_4bits_ic_display_4bits_node_101 & ic_display_4bits_ic_display_4bits_node_102); // And
    assign ic_display_4bits_ic_display_4bits_and_135 = (ic_display_4bits_ic_display_4bits_node_99 & ic_display_4bits_ic_display_4bits_node_100); // And
    assign ic_display_4bits_ic_display_4bits_and_136 = (ic_display_4bits_ic_display_4bits_node_128 & ic_display_4bits_ic_display_4bits_node_127); // And
    assign ic_display_4bits_ic_display_4bits_and_137 = (ic_display_4bits_ic_display_4bits_node_123 & ic_display_4bits_ic_display_4bits_node_124); // And
    assign ic_display_4bits_ic_display_4bits_node_138 = ic_display_4bits_ic_display_4bits_not_163; // Node
    assign ic_display_4bits_ic_display_4bits_node_139 = ic_display_4bits_ic_display_4bits_not_160; // Node
    assign ic_display_4bits_ic_display_4bits_and_140 = (ic_display_4bits_ic_display_4bits_node_139 & ic_display_4bits_ic_display_4bits_node_138); // And
    assign ic_display_4bits_ic_display_4bits_node_141 = ic_display_4bits_ic_display_4bits_node_159; // Node
    assign ic_display_4bits_ic_display_4bits_node_142 = ic_display_4bits_ic_display_4bits_node_157; // Node
    assign ic_display_4bits_ic_display_4bits_and_143 = (ic_display_4bits_ic_display_4bits_node_142 & ic_display_4bits_ic_display_4bits_node_141); // And
    assign ic_display_4bits_ic_display_4bits_node_144 = ic_display_4bits_ic_display_4bits_node_98; // Node
    assign ic_display_4bits_ic_display_4bits_node_145 = ic_display_4bits_ic_display_4bits_node_100; // Node
    assign ic_display_4bits_ic_display_4bits_node_146 = ic_display_4bits_ic_display_4bits_node_96; // Node
    assign ic_display_4bits_ic_display_4bits_node_147 = ic_display_4bits_ic_display_4bits_node_89; // Node
    assign ic_display_4bits_ic_display_4bits_node_148 = ic_display_4bits_ic_display_4bits_node_99; // Node
    assign ic_display_4bits_ic_display_4bits_node_149 = ic_display_4bits_ic_display_4bits_not_161; // Node
    assign ic_display_4bits_ic_display_4bits_node_150 = ic_display_4bits_ic_display_4bits_node_88; // Node
    assign ic_display_4bits_ic_display_4bits_node_151 = ic_display_4bits_ic_display_4bits_node_90; // Node
    assign ic_display_4bits_ic_display_4bits_node_152 = 1'b0; // Node
    assign ic_display_4bits_ic_display_4bits_node_153 = 1'b0; // Node
    assign ic_display_4bits_ic_display_4bits_node_154 = 1'b0; // Node
    assign ic_display_4bits_ic_display_4bits_node_155 = 1'b1; // Node
    assign ic_display_4bits_ic_display_4bits_node_156 = ic_display_4bits_ic_display_4bits_node_153; // Node
    assign ic_display_4bits_ic_display_4bits_node_157 = ic_display_4bits_ic_display_4bits_node_152; // Node
    assign ic_display_4bits_ic_display_4bits_node_158 = ic_display_4bits_ic_display_4bits_node_154; // Node
    assign ic_display_4bits_ic_display_4bits_node_159 = ic_display_4bits_ic_display_4bits_node_155; // Node
    assign ic_display_4bits_ic_display_4bits_not_160 = ~ic_display_4bits_ic_display_4bits_node_157; // Not
    assign ic_display_4bits_ic_display_4bits_not_161 = ~ic_display_4bits_ic_display_4bits_node_156; // Not
    assign ic_display_4bits_ic_display_4bits_not_162 = ~ic_display_4bits_ic_display_4bits_node_158; // Not
    assign ic_display_4bits_ic_display_4bits_not_163 = ~ic_display_4bits_ic_display_4bits_node_159; // Not
    assign node_176 = 1'b0; // Node
    assign node_175 = 1'b0; // Node
    assign node_174 = 1'b0; // Node
    assign node_173 = 1'b0; // Node
    assign node_172 = input_clock2_2; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_11 = ic_jkflipflop_ic_jkflipflop_node_21; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_12 = ic_jkflipflop_ic_jkflipflop_node_23; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_14 = ic_jkflipflop_ic_jkflipflop_node_17; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_15 = ic_jkflipflop_ic_jkflipflop_node_16; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_16 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_17 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_19 = ~ic_jkflipflop_ic_jkflipflop_node_24; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_20 = (ic_jkflipflop_ic_jkflipflop_node_26 & ic_jkflipflop_ic_jkflipflop_node_15); // And
    assign ic_jkflipflop_ic_jkflipflop_node_21 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_22 = (ic_jkflipflop_ic_jkflipflop_and_27 | ic_jkflipflop_ic_jkflipflop_and_20); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_23 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_24 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_25 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_26 = ic_jkflipflop_ic_jkflipflop_node_25; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_27 = (ic_jkflipflop_ic_jkflipflop_node_14 & ic_jkflipflop_ic_jkflipflop_not_19); // And
    assign ic_jkflipflop_ic_jkflipflop_node_28 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_29 = 1'b0; // Node
    assign node_179 = node_178; // Node
    assign node_178 = node_171; // Node
    assign node_171 = node_170; // Node
    assign node_170 = node_169; // Node
    assign node_169 = not_168; // Node
    assign not_168 = ~and_167; // Not
    assign and_167 = (node_180 & node_181 & node_165 & node_166); // And
    assign node_166 = 1'b0; // Node
    assign node_165 = 1'b0; // Node
    assign node_181 = 1'b0; // Node
    assign node_180 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_68 = ic_jkflipflop_ic_jkflipflop_node_78; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_69 = ic_jkflipflop_ic_jkflipflop_node_80; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_70 = ic_jkflipflop_ic_jkflipflop_node_75; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_71 = ic_jkflipflop_ic_jkflipflop_node_74; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_72 = ic_jkflipflop_ic_jkflipflop_node_73; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_73 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_74 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_75 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_76 = ~ic_jkflipflop_ic_jkflipflop_node_81; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_77 = (ic_jkflipflop_ic_jkflipflop_node_83 & ic_jkflipflop_ic_jkflipflop_node_72); // And
    assign ic_jkflipflop_ic_jkflipflop_node_78 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_79 = (ic_jkflipflop_ic_jkflipflop_and_84 | ic_jkflipflop_ic_jkflipflop_and_77); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_80 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_81 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_82 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_83 = ic_jkflipflop_ic_jkflipflop_node_82; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_84 = (ic_jkflipflop_ic_jkflipflop_node_71 & ic_jkflipflop_ic_jkflipflop_not_76); // And
    assign ic_jkflipflop_ic_jkflipflop_node_85 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_86 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_49 = ic_jkflipflop_ic_jkflipflop_node_59; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_50 = ic_jkflipflop_ic_jkflipflop_node_61; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_51 = ic_jkflipflop_ic_jkflipflop_node_56; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_52 = ic_jkflipflop_ic_jkflipflop_node_55; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_53 = ic_jkflipflop_ic_jkflipflop_node_54; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_54 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_55 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_56 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_57 = ~ic_jkflipflop_ic_jkflipflop_node_62; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_58 = (ic_jkflipflop_ic_jkflipflop_node_64 & ic_jkflipflop_ic_jkflipflop_node_53); // And
    assign ic_jkflipflop_ic_jkflipflop_node_59 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_60 = (ic_jkflipflop_ic_jkflipflop_and_65 | ic_jkflipflop_ic_jkflipflop_and_58); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_61 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_62 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_63 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_64 = ic_jkflipflop_ic_jkflipflop_node_63; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_65 = (ic_jkflipflop_ic_jkflipflop_node_52 & ic_jkflipflop_ic_jkflipflop_not_57); // And
    assign ic_jkflipflop_ic_jkflipflop_node_66 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_67 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_30 = ic_jkflipflop_ic_jkflipflop_node_40; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_31 = ic_jkflipflop_ic_jkflipflop_node_42; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_32 = ic_jkflipflop_ic_jkflipflop_node_37; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_33 = ic_jkflipflop_ic_jkflipflop_node_36; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_34 = ic_jkflipflop_ic_jkflipflop_node_35; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_35 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_36 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_37 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_38 = ~ic_jkflipflop_ic_jkflipflop_node_43; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_39 = (ic_jkflipflop_ic_jkflipflop_node_45 & ic_jkflipflop_ic_jkflipflop_node_34); // And
    assign ic_jkflipflop_ic_jkflipflop_node_40 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_41 = (ic_jkflipflop_ic_jkflipflop_and_46 | ic_jkflipflop_ic_jkflipflop_and_39); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_42 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_43 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_44 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_45 = ic_jkflipflop_ic_jkflipflop_node_44; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_46 = (ic_jkflipflop_ic_jkflipflop_node_33 & ic_jkflipflop_ic_jkflipflop_not_38); // And
    assign ic_jkflipflop_ic_jkflipflop_node_47 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_48 = 1'b0; // Node
    assign not_164 = ~input_push_button1_1; // Not

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_3 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_4 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_5 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_6 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_8 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_9 = node_177; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_10 = 1'b0; // 7-Segment Display

endmodule // display_4bits_counter

// ====================================================================
// Module display_4bits_counter generation completed successfully
// Elements processed: 26
// Inputs: 2, Outputs: 8
// Warnings: 12
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP output 1 is not connected
//   IC DISPLAY-4BITS output 6 is not connected
// ====================================================================
