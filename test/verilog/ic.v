// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: ic
// Generated: Fri Sep 26 15:10:58 2025
// Target FPGA: Generic-Small
// Resource Usage: 58/1000 LUTs, 215/1000 FFs, 37/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module ic (
    // ========= Input Ports =========
    input wire input_clock1_1,
    input wire input_clock2_2,
    input wire input_clock3_3,
    input wire input_clock4_4,
    input wire input_clock5_5,
    input wire input_clock6_6,

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_7,
    output wire output_7_segment_display1_f_upper_left_8,
    output wire output_7_segment_display1_e_lower_left_9,
    output wire output_7_segment_display1_d_bottom_10,
    output wire output_7_segment_display1_a_top_11,
    output wire output_7_segment_display1_b_upper_right_12,
    output wire output_7_segment_display1_dp_dot_13,
    output wire output_7_segment_display1_c_lower_right_14,
    output wire output_7_segment_display2_g_middle_15,
    output wire output_7_segment_display2_f_upper_left_16,
    output wire output_7_segment_display2_e_lower_left_17,
    output wire output_7_segment_display2_d_bottom_18,
    output wire output_7_segment_display2_a_top_19,
    output wire output_7_segment_display2_b_upper_right_20,
    output wire output_7_segment_display2_dp_dot_21,
    output wire output_7_segment_display2_c_lower_right_22,
    output wire output_7_segment_display3_g_middle_23,
    output wire output_7_segment_display3_f_upper_left_24,
    output wire output_7_segment_display3_e_lower_left_25,
    output wire output_7_segment_display3_d_bottom_26,
    output wire output_7_segment_display3_a_top_27,
    output wire output_7_segment_display3_b_upper_right_28,
    output wire output_7_segment_display3_dp_dot_29,
    output wire output_7_segment_display3_c_lower_right_30,
    output wire output_led4_0_31,
    output wire output_led5_0_32,
    output wire output_led6_0_33,
    output wire output_led7_0_34
);

    // ========= Internal Signals =========
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_jkflipflop_node_35;
    wire ic_jkflipflop_ic_jkflipflop_node_36;
    wire ic_jkflipflop_ic_jkflipflop_node_38;
    wire ic_jkflipflop_ic_jkflipflop_node_39;
    wire ic_jkflipflop_ic_jkflipflop_node_40;
    wire ic_jkflipflop_ic_jkflipflop_node_41;
    wire ic_jkflipflop_ic_jkflipflop_not_43;
    wire ic_jkflipflop_ic_jkflipflop_and_44;
    wire ic_jkflipflop_ic_jkflipflop_node_45;
    wire ic_jkflipflop_ic_jkflipflop_or_46;
    wire ic_jkflipflop_ic_jkflipflop_node_47;
    wire ic_jkflipflop_ic_jkflipflop_node_48;
    wire ic_jkflipflop_ic_jkflipflop_node_49;
    wire ic_jkflipflop_ic_jkflipflop_node_50;
    wire ic_jkflipflop_ic_jkflipflop_and_51;
    wire ic_jkflipflop_ic_jkflipflop_node_52;
    wire ic_jkflipflop_ic_jkflipflop_node_53;
    assign ic_jkflipflop_ic_jkflipflop_node_35 = ic_jkflipflop_ic_jkflipflop_node_45; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_36 = ic_jkflipflop_ic_jkflipflop_node_47; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_38 = ic_jkflipflop_ic_jkflipflop_node_41; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_39 = ic_jkflipflop_ic_jkflipflop_node_40; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_43 = ~ic_jkflipflop_ic_jkflipflop_node_48; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_44 = (ic_jkflipflop_ic_jkflipflop_node_50 & ic_jkflipflop_ic_jkflipflop_node_39); // And
    assign ic_jkflipflop_ic_jkflipflop_or_46 = (ic_jkflipflop_ic_jkflipflop_and_51 | ic_jkflipflop_ic_jkflipflop_and_44); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_48 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_50 = ic_jkflipflop_ic_jkflipflop_node_49; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_51 = (ic_jkflipflop_ic_jkflipflop_node_38 & ic_jkflipflop_ic_jkflipflop_not_43); // And
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: INPUT ==============
// IC inputs: 2, IC outputs: 4
// Nesting depth: 0
    wire ic_input_ic_input_not_54;
    wire ic_input_ic_input_xor_55;
    wire ic_input_ic_input_or_56;
    wire ic_input_ic_input_and_57;
    wire ic_input_ic_input_node_60;
    wire ic_input_ic_input_node_61;
    wire ic_input_ic_input_node_62;
    wire ic_input_ic_input_node_63;
    assign ic_input_ic_input_not_54 = ~input_clock3_3; // Not
    assign ic_input_ic_input_xor_55 = (input_clock3_3 ^ input_clock2_2); // Xor
    assign ic_input_ic_input_or_56 = (input_clock3_3 | input_clock2_2); // Or
    assign ic_input_ic_input_and_57 = (input_clock3_3 & input_clock2_2); // And
// ============== END IC: INPUT ==============
// ============== BEGIN IC: DISPLAY-4BITS ==============
// IC inputs: 4, IC outputs: 8
// Nesting depth: 0
    wire ic_display_4bits_ic_display_4bits_node_64;
    wire ic_display_4bits_ic_display_4bits_node_65;
    wire ic_display_4bits_ic_display_4bits_node_66;
    wire ic_display_4bits_ic_display_4bits_node_67;
    wire ic_display_4bits_ic_display_4bits_and_68;
    wire ic_display_4bits_ic_display_4bits_node_69;
    wire ic_display_4bits_ic_display_4bits_node_70;
    wire ic_display_4bits_ic_display_4bits_node_71;
    wire ic_display_4bits_ic_display_4bits_node_72;
    wire ic_display_4bits_ic_display_4bits_node_73;
    wire ic_display_4bits_ic_display_4bits_node_74;
    wire ic_display_4bits_ic_display_4bits_node_75;
    wire ic_display_4bits_ic_display_4bits_node_76;
    wire ic_display_4bits_ic_display_4bits_node_77;
    wire ic_display_4bits_ic_display_4bits_node_78;
    wire ic_display_4bits_ic_display_4bits_node_79;
    wire ic_display_4bits_ic_display_4bits_or_80;
    wire ic_display_4bits_ic_display_4bits_or_81;
    wire ic_display_4bits_ic_display_4bits_node_82;
    wire ic_display_4bits_ic_display_4bits_node_83;
    wire ic_display_4bits_ic_display_4bits_node_84;
    wire ic_display_4bits_ic_display_4bits_node_85;
    wire ic_display_4bits_ic_display_4bits_node_86;
    wire ic_display_4bits_ic_display_4bits_node_87;
    wire ic_display_4bits_ic_display_4bits_node_88;
    wire ic_display_4bits_ic_display_4bits_or_89;
    wire ic_display_4bits_ic_display_4bits_or_90;
    wire ic_display_4bits_ic_display_4bits_or_91;
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
    wire ic_display_4bits_ic_display_4bits_node_104;
    wire ic_display_4bits_ic_display_4bits_node_105;
    wire ic_display_4bits_ic_display_4bits_node_106;
    wire ic_display_4bits_ic_display_4bits_node_107;
    wire ic_display_4bits_ic_display_4bits_or_108;
    wire ic_display_4bits_ic_display_4bits_and_109;
    wire ic_display_4bits_ic_display_4bits_and_110;
    wire ic_display_4bits_ic_display_4bits_and_111;
    wire ic_display_4bits_ic_display_4bits_and_112;
    wire ic_display_4bits_ic_display_4bits_and_113;
    wire ic_display_4bits_ic_display_4bits_and_114;
    wire ic_display_4bits_ic_display_4bits_node_115;
    wire ic_display_4bits_ic_display_4bits_node_116;
    wire ic_display_4bits_ic_display_4bits_and_117;
    wire ic_display_4bits_ic_display_4bits_node_118;
    wire ic_display_4bits_ic_display_4bits_node_119;
    wire ic_display_4bits_ic_display_4bits_and_120;
    wire ic_display_4bits_ic_display_4bits_node_121;
    wire ic_display_4bits_ic_display_4bits_node_122;
    wire ic_display_4bits_ic_display_4bits_node_123;
    wire ic_display_4bits_ic_display_4bits_node_124;
    wire ic_display_4bits_ic_display_4bits_node_125;
    wire ic_display_4bits_ic_display_4bits_node_126;
    wire ic_display_4bits_ic_display_4bits_node_127;
    wire ic_display_4bits_ic_display_4bits_node_128;
    wire ic_display_4bits_ic_display_4bits_node_129;
    wire ic_display_4bits_ic_display_4bits_node_130;
    wire ic_display_4bits_ic_display_4bits_node_131;
    wire ic_display_4bits_ic_display_4bits_node_132;
    wire ic_display_4bits_ic_display_4bits_node_133;
    wire ic_display_4bits_ic_display_4bits_node_134;
    wire ic_display_4bits_ic_display_4bits_node_135;
    wire ic_display_4bits_ic_display_4bits_node_136;
    wire ic_display_4bits_ic_display_4bits_not_137;
    wire ic_display_4bits_ic_display_4bits_not_138;
    wire ic_display_4bits_ic_display_4bits_not_139;
    wire ic_display_4bits_ic_display_4bits_not_140;
    assign ic_display_4bits_ic_display_4bits_node_64 = ic_display_4bits_ic_display_4bits_node_82; // Node
    assign ic_display_4bits_ic_display_4bits_node_65 = ic_display_4bits_ic_display_4bits_node_64; // Node
    assign ic_display_4bits_ic_display_4bits_node_66 = ic_display_4bits_ic_display_4bits_node_74; // Node
    assign ic_display_4bits_ic_display_4bits_node_67 = ic_display_4bits_ic_display_4bits_node_79; // Node
    assign ic_display_4bits_ic_display_4bits_and_68 = (ic_display_4bits_ic_display_4bits_node_66 & ic_display_4bits_ic_display_4bits_node_67); // And
    assign ic_display_4bits_ic_display_4bits_node_69 = ic_display_4bits_ic_display_4bits_node_70; // Node
    assign ic_display_4bits_ic_display_4bits_node_70 = ic_display_4bits_ic_display_4bits_and_114; // Node
    assign ic_display_4bits_ic_display_4bits_node_71 = ic_display_4bits_ic_display_4bits_node_72; // Node
    assign ic_display_4bits_ic_display_4bits_node_72 = ic_display_4bits_ic_display_4bits_and_111; // Node
    assign ic_display_4bits_ic_display_4bits_node_73 = ic_display_4bits_ic_display_4bits_node_87; // Node
    assign ic_display_4bits_ic_display_4bits_node_74 = ic_display_4bits_ic_display_4bits_node_86; // Node
    assign ic_display_4bits_ic_display_4bits_node_75 = ic_display_4bits_ic_display_4bits_node_88; // Node
    assign ic_display_4bits_ic_display_4bits_node_76 = ic_display_4bits_ic_display_4bits_node_102; // Node
    assign ic_display_4bits_ic_display_4bits_node_77 = ic_display_4bits_ic_display_4bits_node_78; // Node
    assign ic_display_4bits_ic_display_4bits_node_78 = ic_display_4bits_ic_display_4bits_node_105; // Node
    assign ic_display_4bits_ic_display_4bits_node_79 = ic_display_4bits_ic_display_4bits_node_101; // Node
    assign ic_display_4bits_ic_display_4bits_or_80 = (ic_display_4bits_ic_display_4bits_node_71 | ic_display_4bits_ic_display_4bits_and_110 | ic_display_4bits_ic_display_4bits_node_65 | ic_display_4bits_ic_display_4bits_and_112); // Or
    assign ic_display_4bits_ic_display_4bits_or_81 = (ic_display_4bits_ic_display_4bits_node_69 | ic_display_4bits_ic_display_4bits_and_68 | ic_display_4bits_ic_display_4bits_and_110 | ic_display_4bits_ic_display_4bits_node_64); // Or
    assign ic_display_4bits_ic_display_4bits_node_82 = ic_display_4bits_ic_display_4bits_node_107; // Node
    assign ic_display_4bits_ic_display_4bits_node_83 = ic_display_4bits_ic_display_4bits_node_84; // Node
    assign ic_display_4bits_ic_display_4bits_node_84 = ic_display_4bits_ic_display_4bits_node_85; // Node
    assign ic_display_4bits_ic_display_4bits_node_85 = ic_display_4bits_ic_display_4bits_and_117; // Node
    assign ic_display_4bits_ic_display_4bits_node_86 = ic_display_4bits_ic_display_4bits_node_119; // Node
    assign ic_display_4bits_ic_display_4bits_node_87 = ic_display_4bits_ic_display_4bits_node_100; // Node
    assign ic_display_4bits_ic_display_4bits_node_88 = ic_display_4bits_ic_display_4bits_node_104; // Node
    assign ic_display_4bits_ic_display_4bits_or_89 = (ic_display_4bits_ic_display_4bits_node_86 | ic_display_4bits_ic_display_4bits_node_87 | ic_display_4bits_ic_display_4bits_node_88); // Or
    assign ic_display_4bits_ic_display_4bits_or_90 = (ic_display_4bits_ic_display_4bits_node_83 | ic_display_4bits_ic_display_4bits_node_71); // Or
    assign ic_display_4bits_ic_display_4bits_or_91 = (ic_display_4bits_ic_display_4bits_node_82 | ic_display_4bits_ic_display_4bits_node_84 | ic_display_4bits_ic_display_4bits_node_72 | ic_display_4bits_ic_display_4bits_and_112 | ic_display_4bits_ic_display_4bits_and_109); // Or
    assign ic_display_4bits_ic_display_4bits_node_100 = ic_display_4bits_ic_display_4bits_not_139; // Node
    assign ic_display_4bits_ic_display_4bits_node_101 = ic_display_4bits_ic_display_4bits_node_115; // Node
    assign ic_display_4bits_ic_display_4bits_node_102 = ic_display_4bits_ic_display_4bits_node_116; // Node
    assign ic_display_4bits_ic_display_4bits_or_103 = (ic_display_4bits_ic_display_4bits_and_113 | ic_display_4bits_ic_display_4bits_node_102 | ic_display_4bits_ic_display_4bits_node_70); // Or
    assign ic_display_4bits_ic_display_4bits_node_104 = ic_display_4bits_ic_display_4bits_node_118; // Node
    assign ic_display_4bits_ic_display_4bits_node_105 = ic_display_4bits_ic_display_4bits_node_106; // Node
    assign ic_display_4bits_ic_display_4bits_node_106 = ic_display_4bits_ic_display_4bits_node_135; // Node
    assign ic_display_4bits_ic_display_4bits_node_107 = ic_display_4bits_ic_display_4bits_node_133; // Node
    assign ic_display_4bits_ic_display_4bits_or_108 = (ic_display_4bits_ic_display_4bits_and_120 | ic_display_4bits_ic_display_4bits_node_107 | ic_display_4bits_ic_display_4bits_node_106 | ic_display_4bits_ic_display_4bits_node_85); // Or
    assign ic_display_4bits_ic_display_4bits_and_109 = (ic_display_4bits_ic_display_4bits_node_75 & ic_display_4bits_ic_display_4bits_and_110); // And
    assign ic_display_4bits_ic_display_4bits_and_110 = (ic_display_4bits_ic_display_4bits_node_74 & ic_display_4bits_ic_display_4bits_node_73); // And
    assign ic_display_4bits_ic_display_4bits_and_111 = (ic_display_4bits_ic_display_4bits_node_78 & ic_display_4bits_ic_display_4bits_node_79); // And
    assign ic_display_4bits_ic_display_4bits_and_112 = (ic_display_4bits_ic_display_4bits_node_76 & ic_display_4bits_ic_display_4bits_node_77); // And
    assign ic_display_4bits_ic_display_4bits_and_113 = (ic_display_4bits_ic_display_4bits_node_105 & ic_display_4bits_ic_display_4bits_node_104); // And
    assign ic_display_4bits_ic_display_4bits_and_114 = (ic_display_4bits_ic_display_4bits_node_100 & ic_display_4bits_ic_display_4bits_node_101); // And
    assign ic_display_4bits_ic_display_4bits_node_115 = ic_display_4bits_ic_display_4bits_not_140; // Node
    assign ic_display_4bits_ic_display_4bits_node_116 = ic_display_4bits_ic_display_4bits_not_137; // Node
    assign ic_display_4bits_ic_display_4bits_and_117 = (ic_display_4bits_ic_display_4bits_node_116 & ic_display_4bits_ic_display_4bits_node_115); // And
    assign ic_display_4bits_ic_display_4bits_node_118 = ic_display_4bits_ic_display_4bits_node_136; // Node
    assign ic_display_4bits_ic_display_4bits_node_119 = ic_display_4bits_ic_display_4bits_node_134; // Node
    assign ic_display_4bits_ic_display_4bits_and_120 = (ic_display_4bits_ic_display_4bits_node_119 & ic_display_4bits_ic_display_4bits_node_118); // And
    assign ic_display_4bits_ic_display_4bits_node_133 = ic_display_4bits_ic_display_4bits_node_130; // Node
    assign ic_display_4bits_ic_display_4bits_node_134 = ic_display_4bits_ic_display_4bits_node_129; // Node
    assign ic_display_4bits_ic_display_4bits_node_135 = ic_display_4bits_ic_display_4bits_node_131; // Node
    assign ic_display_4bits_ic_display_4bits_node_136 = ic_display_4bits_ic_display_4bits_node_132; // Node
    assign ic_display_4bits_ic_display_4bits_not_137 = ~ic_display_4bits_ic_display_4bits_node_134; // Not
    assign ic_display_4bits_ic_display_4bits_not_138 = ~ic_display_4bits_ic_display_4bits_node_133; // Not
    assign ic_display_4bits_ic_display_4bits_not_139 = ~ic_display_4bits_ic_display_4bits_node_135; // Not
    assign ic_display_4bits_ic_display_4bits_not_140 = ~ic_display_4bits_ic_display_4bits_node_136; // Not
// ============== END IC: DISPLAY-4BITS ==============
// ============== BEGIN IC: DISPLAY-4BITS ==============
// IC inputs: 4, IC outputs: 8
// Nesting depth: 0
    wire ic_display_4bits_ic_display_4bits_node_144;
    wire ic_display_4bits_ic_display_4bits_and_145;
    wire ic_display_4bits_ic_display_4bits_node_146;
    wire ic_display_4bits_ic_display_4bits_node_147;
    wire ic_display_4bits_ic_display_4bits_node_148;
    wire ic_display_4bits_ic_display_4bits_node_149;
    wire ic_display_4bits_ic_display_4bits_node_150;
    wire ic_display_4bits_ic_display_4bits_node_153;
    wire ic_display_4bits_ic_display_4bits_node_156;
    wire ic_display_4bits_ic_display_4bits_or_157;
    wire ic_display_4bits_ic_display_4bits_or_158;
    wire ic_display_4bits_ic_display_4bits_node_160;
    wire ic_display_4bits_ic_display_4bits_node_161;
    wire ic_display_4bits_ic_display_4bits_node_162;
    wire ic_display_4bits_ic_display_4bits_node_164;
    wire ic_display_4bits_ic_display_4bits_or_166;
    wire ic_display_4bits_ic_display_4bits_or_167;
    wire ic_display_4bits_ic_display_4bits_or_168;
    wire ic_display_4bits_ic_display_4bits_node_169;
    wire ic_display_4bits_ic_display_4bits_node_170;
    wire ic_display_4bits_ic_display_4bits_node_171;
    wire ic_display_4bits_ic_display_4bits_node_172;
    wire ic_display_4bits_ic_display_4bits_node_173;
    wire ic_display_4bits_ic_display_4bits_node_174;
    wire ic_display_4bits_ic_display_4bits_node_175;
    wire ic_display_4bits_ic_display_4bits_node_176;
    wire ic_display_4bits_ic_display_4bits_node_177;
    wire ic_display_4bits_ic_display_4bits_node_178;
    wire ic_display_4bits_ic_display_4bits_node_179;
    wire ic_display_4bits_ic_display_4bits_or_180;
    wire ic_display_4bits_ic_display_4bits_or_185;
    wire ic_display_4bits_ic_display_4bits_and_186;
    wire ic_display_4bits_ic_display_4bits_and_187;
    wire ic_display_4bits_ic_display_4bits_and_188;
    wire ic_display_4bits_ic_display_4bits_and_189;
    wire ic_display_4bits_ic_display_4bits_and_190;
    wire ic_display_4bits_ic_display_4bits_and_191;
    wire ic_display_4bits_ic_display_4bits_node_192;
    wire ic_display_4bits_ic_display_4bits_node_193;
    wire ic_display_4bits_ic_display_4bits_and_194;
    wire ic_display_4bits_ic_display_4bits_and_197;
    wire ic_display_4bits_ic_display_4bits_node_200;
    wire ic_display_4bits_ic_display_4bits_node_202;
    wire ic_display_4bits_ic_display_4bits_node_203;
    wire ic_display_4bits_ic_display_4bits_node_205;
    wire ic_display_4bits_ic_display_4bits_not_214;
    wire ic_display_4bits_ic_display_4bits_not_215;
    wire ic_display_4bits_ic_display_4bits_not_216;
    wire ic_display_4bits_ic_display_4bits_not_217;
    assign ic_display_4bits_ic_display_4bits_node_144 = ic_display_4bits_ic_display_4bits_node_156; // Node
    assign ic_display_4bits_ic_display_4bits_and_145 = (input_clock2_2 & ic_display_4bits_ic_display_4bits_node_144); // And
    assign ic_display_4bits_ic_display_4bits_node_146 = ic_display_4bits_ic_display_4bits_node_147; // Node
    assign ic_display_4bits_ic_display_4bits_node_147 = ic_display_4bits_ic_display_4bits_and_191; // Node
    assign ic_display_4bits_ic_display_4bits_node_148 = ic_display_4bits_ic_display_4bits_node_149; // Node
    assign ic_display_4bits_ic_display_4bits_node_149 = ic_display_4bits_ic_display_4bits_and_188; // Node
    assign ic_display_4bits_ic_display_4bits_node_150 = ic_display_4bits_ic_display_4bits_node_164; // Node
    assign ic_display_4bits_ic_display_4bits_node_153 = ic_display_4bits_ic_display_4bits_node_179; // Node
    assign ic_display_4bits_ic_display_4bits_node_156 = ic_display_4bits_ic_display_4bits_node_178; // Node
    assign ic_display_4bits_ic_display_4bits_or_157 = (ic_display_4bits_ic_display_4bits_node_148 | ic_display_4bits_ic_display_4bits_and_187 | input_clock3_3 | ic_display_4bits_ic_display_4bits_and_189); // Or
    assign ic_display_4bits_ic_display_4bits_or_158 = (ic_display_4bits_ic_display_4bits_node_146 | ic_display_4bits_ic_display_4bits_and_145 | ic_display_4bits_ic_display_4bits_and_187 | input_clock3_3); // Or
    assign ic_display_4bits_ic_display_4bits_node_160 = ic_display_4bits_ic_display_4bits_node_161; // Node
    assign ic_display_4bits_ic_display_4bits_node_161 = ic_display_4bits_ic_display_4bits_node_162; // Node
    assign ic_display_4bits_ic_display_4bits_node_162 = ic_display_4bits_ic_display_4bits_and_194; // Node
    assign ic_display_4bits_ic_display_4bits_node_164 = ic_display_4bits_ic_display_4bits_node_177; // Node
    assign ic_display_4bits_ic_display_4bits_or_166 = (input_clock2_2 | ic_display_4bits_ic_display_4bits_node_164 | input_clock4_4); // Or
    assign ic_display_4bits_ic_display_4bits_or_167 = (ic_display_4bits_ic_display_4bits_node_160 | ic_display_4bits_ic_display_4bits_node_148); // Or
    assign ic_display_4bits_ic_display_4bits_or_168 = (input_clock3_3 | ic_display_4bits_ic_display_4bits_node_161 | ic_display_4bits_ic_display_4bits_node_149 | ic_display_4bits_ic_display_4bits_and_189 | ic_display_4bits_ic_display_4bits_and_186); // Or
    assign ic_display_4bits_ic_display_4bits_node_177 = ic_display_4bits_ic_display_4bits_not_216; // Node
    assign ic_display_4bits_ic_display_4bits_node_178 = ic_display_4bits_ic_display_4bits_node_192; // Node
    assign ic_display_4bits_ic_display_4bits_node_179 = ic_display_4bits_ic_display_4bits_node_193; // Node
    assign ic_display_4bits_ic_display_4bits_or_180 = (ic_display_4bits_ic_display_4bits_and_190 | ic_display_4bits_ic_display_4bits_node_179 | ic_display_4bits_ic_display_4bits_node_147); // Or
    assign ic_display_4bits_ic_display_4bits_or_185 = (ic_display_4bits_ic_display_4bits_and_197 | input_clock3_3 | input_clock5_5 | ic_display_4bits_ic_display_4bits_node_162); // Or
    assign ic_display_4bits_ic_display_4bits_and_186 = (input_clock4_4 & ic_display_4bits_ic_display_4bits_and_187); // And
    assign ic_display_4bits_ic_display_4bits_and_187 = (input_clock2_2 & ic_display_4bits_ic_display_4bits_node_150); // And
    assign ic_display_4bits_ic_display_4bits_and_188 = (input_clock5_5 & ic_display_4bits_ic_display_4bits_node_156); // And
    assign ic_display_4bits_ic_display_4bits_and_189 = (ic_display_4bits_ic_display_4bits_node_153 & input_clock5_5); // And
    assign ic_display_4bits_ic_display_4bits_and_190 = (input_clock5_5 & input_clock4_4); // And
    assign ic_display_4bits_ic_display_4bits_and_191 = (ic_display_4bits_ic_display_4bits_node_177 & ic_display_4bits_ic_display_4bits_node_178); // And
    assign ic_display_4bits_ic_display_4bits_node_192 = ic_display_4bits_ic_display_4bits_not_217; // Node
    assign ic_display_4bits_ic_display_4bits_node_193 = ic_display_4bits_ic_display_4bits_not_214; // Node
    assign ic_display_4bits_ic_display_4bits_and_194 = (ic_display_4bits_ic_display_4bits_node_193 & ic_display_4bits_ic_display_4bits_node_192); // And
    assign ic_display_4bits_ic_display_4bits_and_197 = (input_clock2_2 & input_clock4_4); // And
    assign ic_display_4bits_ic_display_4bits_not_214 = ~input_clock2_2; // Not
    assign ic_display_4bits_ic_display_4bits_not_215 = ~input_clock3_3; // Not
    assign ic_display_4bits_ic_display_4bits_not_216 = ~input_clock5_5; // Not
    assign ic_display_4bits_ic_display_4bits_not_217 = ~input_clock4_4; // Not
// ============== END IC: DISPLAY-4BITS ==============
// ============== BEGIN IC: DISPLAY-3BITS ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
    wire ic_display_3bits_ic_display_3bits_node_218;
    wire ic_display_3bits_ic_display_3bits_or_219;
    wire ic_display_3bits_ic_display_3bits_or_220;
    wire ic_display_3bits_ic_display_3bits_and_221;
    wire ic_display_3bits_ic_display_3bits_or_222;
    wire ic_display_3bits_ic_display_3bits_not_223;
    wire ic_display_3bits_ic_display_3bits_and_224;
    wire ic_display_3bits_ic_display_3bits_and_225;
    wire ic_display_3bits_ic_display_3bits_and_226;
    wire ic_display_3bits_ic_display_3bits_xnor_227;
    wire ic_display_3bits_ic_display_3bits_and_228;
    wire ic_display_3bits_ic_display_3bits_or_229;
    wire ic_display_3bits_ic_display_3bits_nand_230;
    wire ic_display_3bits_ic_display_3bits_or_231;
    wire ic_display_3bits_ic_display_3bits_or_232;
    wire ic_display_3bits_ic_display_3bits_node_233;
    wire ic_display_3bits_ic_display_3bits_node_234;
    wire ic_display_3bits_ic_display_3bits_node_235;
    wire ic_display_3bits_ic_display_3bits_node_236;
    wire ic_display_3bits_ic_display_3bits_node_237;
    wire ic_display_3bits_ic_display_3bits_node_238;
    wire ic_display_3bits_ic_display_3bits_node_239;
    wire ic_display_3bits_ic_display_3bits_node_240;
    wire ic_display_3bits_ic_display_3bits_not_241;
    wire ic_display_3bits_ic_display_3bits_and_242;
    wire ic_display_3bits_ic_display_3bits_and_243;
    wire ic_display_3bits_ic_display_3bits_and_244;
    wire ic_display_3bits_ic_display_3bits_and_245;
    wire ic_display_3bits_ic_display_3bits_node_246;
    wire ic_display_3bits_ic_display_3bits_or_247;
    wire ic_display_3bits_ic_display_3bits_node_248;
    wire ic_display_3bits_ic_display_3bits_node_249;
    wire ic_display_3bits_ic_display_3bits_xnor_250;
    wire ic_display_3bits_ic_display_3bits_gnd_251;
    wire ic_display_3bits_ic_display_3bits_or_252;
    wire ic_display_3bits_ic_display_3bits_and_253;
    wire ic_display_3bits_ic_display_3bits_not_254;
    wire ic_display_3bits_ic_display_3bits_or_255;
    wire ic_display_3bits_ic_display_3bits_gnd_256;
    assign ic_display_3bits_ic_display_3bits_or_219 = (ic_display_3bits_ic_display_3bits_gnd_256 | ic_display_3bits_ic_display_3bits_node_246); // Or
    assign ic_display_3bits_ic_display_3bits_or_220 = (ic_display_3bits_ic_display_3bits_gnd_256 | ic_display_3bits_ic_display_3bits_node_249); // Or
    assign ic_display_3bits_ic_display_3bits_and_221 = (ic_display_3bits_ic_display_3bits_or_220 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_or_222 = (ic_display_3bits_ic_display_3bits_and_225 | ic_display_3bits_ic_display_3bits_and_226); // Or
    assign ic_display_3bits_ic_display_3bits_not_223 = ~ic_display_3bits_ic_display_3bits_or_255; // Not
    assign ic_display_3bits_ic_display_3bits_and_224 = (ic_display_3bits_ic_display_3bits_or_255 & ic_display_3bits_ic_display_3bits_not_254); // And
    assign ic_display_3bits_ic_display_3bits_and_225 = (ic_display_3bits_ic_display_3bits_or_220 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_and_226 = (ic_display_3bits_ic_display_3bits_not_223 & ic_display_3bits_ic_display_3bits_not_254 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_xnor_227 = ~(ic_display_3bits_ic_display_3bits_not_254 ^ ic_display_3bits_ic_display_3bits_not_241); // Xnor
    assign ic_display_3bits_ic_display_3bits_and_228 = (ic_display_3bits_ic_display_3bits_or_255 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_or_229 = (ic_display_3bits_ic_display_3bits_and_244 | ic_display_3bits_ic_display_3bits_and_224 | ic_display_3bits_ic_display_3bits_and_228); // Or
    assign ic_display_3bits_ic_display_3bits_nand_230 = ~(ic_display_3bits_ic_display_3bits_not_223 & ic_display_3bits_ic_display_3bits_or_220 & ic_display_3bits_ic_display_3bits_not_241); // Nand
    assign ic_display_3bits_ic_display_3bits_or_231 = (ic_display_3bits_ic_display_3bits_and_224 | ic_display_3bits_ic_display_3bits_and_253 | ic_display_3bits_ic_display_3bits_and_243); // Or
    assign ic_display_3bits_ic_display_3bits_or_232 = (ic_display_3bits_ic_display_3bits_not_223 | ic_display_3bits_ic_display_3bits_xnor_227); // Or
    assign ic_display_3bits_ic_display_3bits_not_241 = ~ic_display_3bits_ic_display_3bits_or_219; // Not
    assign ic_display_3bits_ic_display_3bits_and_242 = (ic_display_3bits_ic_display_3bits_or_255 & ic_display_3bits_ic_display_3bits_not_254 & ic_display_3bits_ic_display_3bits_or_219); // And
    assign ic_display_3bits_ic_display_3bits_and_243 = (ic_display_3bits_ic_display_3bits_node_249 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_and_244 = (ic_display_3bits_ic_display_3bits_not_254 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_and_245 = (ic_display_3bits_ic_display_3bits_not_223 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_node_246 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_or_247 = (ic_display_3bits_ic_display_3bits_and_245 | ic_display_3bits_ic_display_3bits_and_253 | ic_display_3bits_ic_display_3bits_and_221 | ic_display_3bits_ic_display_3bits_and_242); // Or
    assign ic_display_3bits_ic_display_3bits_node_248 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_node_249 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_xnor_250 = ~(ic_display_3bits_ic_display_3bits_not_223 ^ ic_display_3bits_ic_display_3bits_not_241); // Xnor
    assign ic_display_3bits_ic_display_3bits_or_252 = (ic_display_3bits_ic_display_3bits_xnor_250 | ic_display_3bits_ic_display_3bits_or_220); // Or
    assign ic_display_3bits_ic_display_3bits_and_253 = (ic_display_3bits_ic_display_3bits_not_223 & ic_display_3bits_ic_display_3bits_or_220); // And
    assign ic_display_3bits_ic_display_3bits_not_254 = ~ic_display_3bits_ic_display_3bits_or_220; // Not
    assign ic_display_3bits_ic_display_3bits_or_255 = (ic_display_3bits_ic_display_3bits_gnd_256 | ic_display_3bits_ic_display_3bits_node_248); // Or
// ============== END IC: DISPLAY-3BITS ==============
    reg jk_flip_flop_257_0_q = 1'b0;
    reg jk_flip_flop_257_1_q = 1'b0;
    reg jk_flip_flop_258_0_q = 1'b0;
    reg jk_flip_flop_258_1_q = 1'b0;
    reg jk_flip_flop_259_0_q = 1'b0;
    reg jk_flip_flop_259_1_q = 1'b0;
    reg jk_flip_flop_260_0_q = 1'b0;
    reg jk_flip_flop_260_1_q = 1'b0;

    // ========= Logic Assignments =========
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock6_6) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_260_0_q <= 1'b0; jk_flip_flop_260_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_260_0_q <= 1'b1; jk_flip_flop_260_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_260_0_q <= jk_flip_flop_260_1_q; jk_flip_flop_260_1_q <= jk_flip_flop_260_0_q; end // toggle
            endcase
        end
    end

    assign ic_display_3bits_ic_display_3bits_or_219 = (ic_display_3bits_ic_display_3bits_gnd_256 | ic_display_3bits_ic_display_3bits_node_246); // Or
    assign ic_display_3bits_ic_display_3bits_or_220 = (ic_display_3bits_ic_display_3bits_gnd_256 | ic_display_3bits_ic_display_3bits_node_249); // Or
    assign ic_display_3bits_ic_display_3bits_and_221 = (ic_display_3bits_ic_display_3bits_or_220 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_or_222 = (ic_display_3bits_ic_display_3bits_and_225 | ic_display_3bits_ic_display_3bits_and_226); // Or
    assign ic_display_3bits_ic_display_3bits_not_223 = ~ic_display_3bits_ic_display_3bits_or_255; // Not
    assign ic_display_3bits_ic_display_3bits_and_224 = (ic_display_3bits_ic_display_3bits_or_255 & ic_display_3bits_ic_display_3bits_not_254); // And
    assign ic_display_3bits_ic_display_3bits_and_225 = (ic_display_3bits_ic_display_3bits_or_220 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_and_226 = (ic_display_3bits_ic_display_3bits_not_223 & ic_display_3bits_ic_display_3bits_not_254 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_xnor_227 = ~(ic_display_3bits_ic_display_3bits_not_254 ^ ic_display_3bits_ic_display_3bits_not_241); // Xnor
    assign ic_display_3bits_ic_display_3bits_and_228 = (ic_display_3bits_ic_display_3bits_or_255 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_or_229 = (ic_display_3bits_ic_display_3bits_and_244 | ic_display_3bits_ic_display_3bits_and_224 | ic_display_3bits_ic_display_3bits_and_228); // Or
    assign ic_display_3bits_ic_display_3bits_nand_230 = ~(ic_display_3bits_ic_display_3bits_not_223 & ic_display_3bits_ic_display_3bits_or_220 & ic_display_3bits_ic_display_3bits_not_241); // Nand
    assign ic_display_3bits_ic_display_3bits_or_231 = (ic_display_3bits_ic_display_3bits_and_224 | ic_display_3bits_ic_display_3bits_and_253 | ic_display_3bits_ic_display_3bits_and_243); // Or
    assign ic_display_3bits_ic_display_3bits_or_232 = (ic_display_3bits_ic_display_3bits_not_223 | ic_display_3bits_ic_display_3bits_xnor_227); // Or
    assign ic_display_3bits_ic_display_3bits_not_241 = ~ic_display_3bits_ic_display_3bits_or_219; // Not
    assign ic_display_3bits_ic_display_3bits_and_242 = (ic_display_3bits_ic_display_3bits_or_255 & ic_display_3bits_ic_display_3bits_not_254 & ic_display_3bits_ic_display_3bits_or_219); // And
    assign ic_display_3bits_ic_display_3bits_and_243 = (ic_display_3bits_ic_display_3bits_node_249 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_and_244 = (ic_display_3bits_ic_display_3bits_not_254 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_and_245 = (ic_display_3bits_ic_display_3bits_not_223 & ic_display_3bits_ic_display_3bits_not_241); // And
    assign ic_display_3bits_ic_display_3bits_node_246 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_or_247 = (ic_display_3bits_ic_display_3bits_and_245 | ic_display_3bits_ic_display_3bits_and_253 | ic_display_3bits_ic_display_3bits_and_221 | ic_display_3bits_ic_display_3bits_and_242); // Or
    assign ic_display_3bits_ic_display_3bits_node_248 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_node_249 = 1'b0; // Node
    assign ic_display_3bits_ic_display_3bits_xnor_250 = ~(ic_display_3bits_ic_display_3bits_not_223 ^ ic_display_3bits_ic_display_3bits_not_241); // Xnor
    assign ic_display_3bits_ic_display_3bits_or_252 = (ic_display_3bits_ic_display_3bits_xnor_250 | ic_display_3bits_ic_display_3bits_or_220); // Or
    assign ic_display_3bits_ic_display_3bits_and_253 = (ic_display_3bits_ic_display_3bits_not_223 & ic_display_3bits_ic_display_3bits_or_220); // And
    assign ic_display_3bits_ic_display_3bits_not_254 = ~ic_display_3bits_ic_display_3bits_or_220; // Not
    assign ic_display_3bits_ic_display_3bits_or_255 = (ic_display_3bits_ic_display_3bits_gnd_256 | ic_display_3bits_ic_display_3bits_node_248); // Or
    assign ic_display_4bits_ic_display_4bits_node_144 = ic_display_4bits_ic_display_4bits_node_156; // Node
    assign ic_display_4bits_ic_display_4bits_and_145 = (input_clock2_2 & ic_display_4bits_ic_display_4bits_node_144); // And
    assign ic_display_4bits_ic_display_4bits_node_146 = ic_display_4bits_ic_display_4bits_node_147; // Node
    assign ic_display_4bits_ic_display_4bits_node_147 = ic_display_4bits_ic_display_4bits_and_191; // Node
    assign ic_display_4bits_ic_display_4bits_node_148 = ic_display_4bits_ic_display_4bits_node_149; // Node
    assign ic_display_4bits_ic_display_4bits_node_149 = ic_display_4bits_ic_display_4bits_and_188; // Node
    assign ic_display_4bits_ic_display_4bits_node_150 = ic_display_4bits_ic_display_4bits_node_164; // Node
    assign ic_display_4bits_ic_display_4bits_node_153 = ic_display_4bits_ic_display_4bits_node_179; // Node
    assign ic_display_4bits_ic_display_4bits_node_156 = ic_display_4bits_ic_display_4bits_node_178; // Node
    assign ic_display_4bits_ic_display_4bits_or_157 = (ic_display_4bits_ic_display_4bits_node_148 | ic_display_4bits_ic_display_4bits_and_187 | input_clock3_3 | ic_display_4bits_ic_display_4bits_and_189); // Or
    assign ic_display_4bits_ic_display_4bits_or_158 = (ic_display_4bits_ic_display_4bits_node_146 | ic_display_4bits_ic_display_4bits_and_145 | ic_display_4bits_ic_display_4bits_and_187 | input_clock3_3); // Or
    assign ic_display_4bits_ic_display_4bits_node_160 = ic_display_4bits_ic_display_4bits_node_161; // Node
    assign ic_display_4bits_ic_display_4bits_node_161 = ic_display_4bits_ic_display_4bits_node_162; // Node
    assign ic_display_4bits_ic_display_4bits_node_162 = ic_display_4bits_ic_display_4bits_and_194; // Node
    assign ic_display_4bits_ic_display_4bits_node_164 = ic_display_4bits_ic_display_4bits_node_177; // Node
    assign ic_display_4bits_ic_display_4bits_or_166 = (input_clock2_2 | ic_display_4bits_ic_display_4bits_node_164 | input_clock4_4); // Or
    assign ic_display_4bits_ic_display_4bits_or_167 = (ic_display_4bits_ic_display_4bits_node_160 | ic_display_4bits_ic_display_4bits_node_148); // Or
    assign ic_display_4bits_ic_display_4bits_or_168 = (input_clock3_3 | ic_display_4bits_ic_display_4bits_node_161 | ic_display_4bits_ic_display_4bits_node_149 | ic_display_4bits_ic_display_4bits_and_189 | ic_display_4bits_ic_display_4bits_and_186); // Or
    assign ic_display_4bits_ic_display_4bits_node_177 = ic_display_4bits_ic_display_4bits_not_216; // Node
    assign ic_display_4bits_ic_display_4bits_node_178 = ic_display_4bits_ic_display_4bits_node_192; // Node
    assign ic_display_4bits_ic_display_4bits_node_179 = ic_display_4bits_ic_display_4bits_node_193; // Node
    assign ic_display_4bits_ic_display_4bits_or_180 = (ic_display_4bits_ic_display_4bits_and_190 | ic_display_4bits_ic_display_4bits_node_179 | ic_display_4bits_ic_display_4bits_node_147); // Or
    assign ic_display_4bits_ic_display_4bits_or_185 = (ic_display_4bits_ic_display_4bits_and_197 | input_clock3_3 | input_clock5_5 | ic_display_4bits_ic_display_4bits_node_162); // Or
    assign ic_display_4bits_ic_display_4bits_and_186 = (input_clock4_4 & ic_display_4bits_ic_display_4bits_and_187); // And
    assign ic_display_4bits_ic_display_4bits_and_187 = (input_clock2_2 & ic_display_4bits_ic_display_4bits_node_150); // And
    assign ic_display_4bits_ic_display_4bits_and_188 = (input_clock5_5 & ic_display_4bits_ic_display_4bits_node_156); // And
    assign ic_display_4bits_ic_display_4bits_and_189 = (ic_display_4bits_ic_display_4bits_node_153 & input_clock5_5); // And
    assign ic_display_4bits_ic_display_4bits_and_190 = (input_clock5_5 & input_clock4_4); // And
    assign ic_display_4bits_ic_display_4bits_and_191 = (ic_display_4bits_ic_display_4bits_node_177 & ic_display_4bits_ic_display_4bits_node_178); // And
    assign ic_display_4bits_ic_display_4bits_node_192 = ic_display_4bits_ic_display_4bits_not_217; // Node
    assign ic_display_4bits_ic_display_4bits_node_193 = ic_display_4bits_ic_display_4bits_not_214; // Node
    assign ic_display_4bits_ic_display_4bits_and_194 = (ic_display_4bits_ic_display_4bits_node_193 & ic_display_4bits_ic_display_4bits_node_192); // And
    assign ic_display_4bits_ic_display_4bits_and_197 = (input_clock2_2 & input_clock4_4); // And
    assign ic_display_4bits_ic_display_4bits_not_214 = ~input_clock2_2; // Not
    assign ic_display_4bits_ic_display_4bits_not_215 = ~input_clock3_3; // Not
    assign ic_display_4bits_ic_display_4bits_not_216 = ~input_clock5_5; // Not
    assign ic_display_4bits_ic_display_4bits_not_217 = ~input_clock4_4; // Not
    assign ic_display_4bits_ic_display_4bits_node_64 = ic_display_4bits_ic_display_4bits_node_82; // Node
    assign ic_display_4bits_ic_display_4bits_node_65 = ic_display_4bits_ic_display_4bits_node_64; // Node
    assign ic_display_4bits_ic_display_4bits_node_66 = ic_display_4bits_ic_display_4bits_node_74; // Node
    assign ic_display_4bits_ic_display_4bits_node_67 = ic_display_4bits_ic_display_4bits_node_79; // Node
    assign ic_display_4bits_ic_display_4bits_and_68 = (ic_display_4bits_ic_display_4bits_node_66 & ic_display_4bits_ic_display_4bits_node_67); // And
    assign ic_display_4bits_ic_display_4bits_node_69 = ic_display_4bits_ic_display_4bits_node_70; // Node
    assign ic_display_4bits_ic_display_4bits_node_70 = ic_display_4bits_ic_display_4bits_and_114; // Node
    assign ic_display_4bits_ic_display_4bits_node_71 = ic_display_4bits_ic_display_4bits_node_72; // Node
    assign ic_display_4bits_ic_display_4bits_node_72 = ic_display_4bits_ic_display_4bits_and_111; // Node
    assign ic_display_4bits_ic_display_4bits_node_73 = ic_display_4bits_ic_display_4bits_node_87; // Node
    assign ic_display_4bits_ic_display_4bits_node_74 = ic_display_4bits_ic_display_4bits_node_86; // Node
    assign ic_display_4bits_ic_display_4bits_node_75 = ic_display_4bits_ic_display_4bits_node_88; // Node
    assign ic_display_4bits_ic_display_4bits_node_76 = ic_display_4bits_ic_display_4bits_node_102; // Node
    assign ic_display_4bits_ic_display_4bits_node_77 = ic_display_4bits_ic_display_4bits_node_78; // Node
    assign ic_display_4bits_ic_display_4bits_node_78 = ic_display_4bits_ic_display_4bits_node_105; // Node
    assign ic_display_4bits_ic_display_4bits_node_79 = ic_display_4bits_ic_display_4bits_node_101; // Node
    assign ic_display_4bits_ic_display_4bits_or_80 = (ic_display_4bits_ic_display_4bits_node_71 | ic_display_4bits_ic_display_4bits_and_110 | ic_display_4bits_ic_display_4bits_node_65 | ic_display_4bits_ic_display_4bits_and_112); // Or
    assign ic_display_4bits_ic_display_4bits_or_81 = (ic_display_4bits_ic_display_4bits_node_69 | ic_display_4bits_ic_display_4bits_and_68 | ic_display_4bits_ic_display_4bits_and_110 | ic_display_4bits_ic_display_4bits_node_64); // Or
    assign ic_display_4bits_ic_display_4bits_node_82 = ic_display_4bits_ic_display_4bits_node_107; // Node
    assign ic_display_4bits_ic_display_4bits_node_83 = ic_display_4bits_ic_display_4bits_node_84; // Node
    assign ic_display_4bits_ic_display_4bits_node_84 = ic_display_4bits_ic_display_4bits_node_85; // Node
    assign ic_display_4bits_ic_display_4bits_node_85 = ic_display_4bits_ic_display_4bits_and_117; // Node
    assign ic_display_4bits_ic_display_4bits_node_86 = ic_display_4bits_ic_display_4bits_node_119; // Node
    assign ic_display_4bits_ic_display_4bits_node_87 = ic_display_4bits_ic_display_4bits_node_100; // Node
    assign ic_display_4bits_ic_display_4bits_node_88 = ic_display_4bits_ic_display_4bits_node_104; // Node
    assign ic_display_4bits_ic_display_4bits_or_89 = (ic_display_4bits_ic_display_4bits_node_86 | ic_display_4bits_ic_display_4bits_node_87 | ic_display_4bits_ic_display_4bits_node_88); // Or
    assign ic_display_4bits_ic_display_4bits_or_90 = (ic_display_4bits_ic_display_4bits_node_83 | ic_display_4bits_ic_display_4bits_node_71); // Or
    assign ic_display_4bits_ic_display_4bits_or_91 = (ic_display_4bits_ic_display_4bits_node_82 | ic_display_4bits_ic_display_4bits_node_84 | ic_display_4bits_ic_display_4bits_node_72 | ic_display_4bits_ic_display_4bits_and_112 | ic_display_4bits_ic_display_4bits_and_109); // Or
    assign ic_display_4bits_ic_display_4bits_node_100 = ic_display_4bits_ic_display_4bits_not_139; // Node
    assign ic_display_4bits_ic_display_4bits_node_101 = ic_display_4bits_ic_display_4bits_node_115; // Node
    assign ic_display_4bits_ic_display_4bits_node_102 = ic_display_4bits_ic_display_4bits_node_116; // Node
    assign ic_display_4bits_ic_display_4bits_or_103 = (ic_display_4bits_ic_display_4bits_and_113 | ic_display_4bits_ic_display_4bits_node_102 | ic_display_4bits_ic_display_4bits_node_70); // Or
    assign ic_display_4bits_ic_display_4bits_node_104 = ic_display_4bits_ic_display_4bits_node_118; // Node
    assign ic_display_4bits_ic_display_4bits_node_105 = ic_display_4bits_ic_display_4bits_node_106; // Node
    assign ic_display_4bits_ic_display_4bits_node_106 = ic_display_4bits_ic_display_4bits_node_135; // Node
    assign ic_display_4bits_ic_display_4bits_node_107 = ic_display_4bits_ic_display_4bits_node_133; // Node
    assign ic_display_4bits_ic_display_4bits_or_108 = (ic_display_4bits_ic_display_4bits_and_120 | ic_display_4bits_ic_display_4bits_node_107 | ic_display_4bits_ic_display_4bits_node_106 | ic_display_4bits_ic_display_4bits_node_85); // Or
    assign ic_display_4bits_ic_display_4bits_and_109 = (ic_display_4bits_ic_display_4bits_node_75 & ic_display_4bits_ic_display_4bits_and_110); // And
    assign ic_display_4bits_ic_display_4bits_and_110 = (ic_display_4bits_ic_display_4bits_node_74 & ic_display_4bits_ic_display_4bits_node_73); // And
    assign ic_display_4bits_ic_display_4bits_and_111 = (ic_display_4bits_ic_display_4bits_node_78 & ic_display_4bits_ic_display_4bits_node_79); // And
    assign ic_display_4bits_ic_display_4bits_and_112 = (ic_display_4bits_ic_display_4bits_node_76 & ic_display_4bits_ic_display_4bits_node_77); // And
    assign ic_display_4bits_ic_display_4bits_and_113 = (ic_display_4bits_ic_display_4bits_node_105 & ic_display_4bits_ic_display_4bits_node_104); // And
    assign ic_display_4bits_ic_display_4bits_and_114 = (ic_display_4bits_ic_display_4bits_node_100 & ic_display_4bits_ic_display_4bits_node_101); // And
    assign ic_display_4bits_ic_display_4bits_node_115 = ic_display_4bits_ic_display_4bits_not_140; // Node
    assign ic_display_4bits_ic_display_4bits_node_116 = ic_display_4bits_ic_display_4bits_not_137; // Node
    assign ic_display_4bits_ic_display_4bits_and_117 = (ic_display_4bits_ic_display_4bits_node_116 & ic_display_4bits_ic_display_4bits_node_115); // And
    assign ic_display_4bits_ic_display_4bits_node_118 = ic_display_4bits_ic_display_4bits_node_136; // Node
    assign ic_display_4bits_ic_display_4bits_node_119 = ic_display_4bits_ic_display_4bits_node_134; // Node
    assign ic_display_4bits_ic_display_4bits_and_120 = (ic_display_4bits_ic_display_4bits_node_119 & ic_display_4bits_ic_display_4bits_node_118); // And
    assign ic_display_4bits_ic_display_4bits_node_133 = ic_display_4bits_ic_display_4bits_node_130; // Node
    assign ic_display_4bits_ic_display_4bits_node_134 = ic_display_4bits_ic_display_4bits_node_129; // Node
    assign ic_display_4bits_ic_display_4bits_node_135 = ic_display_4bits_ic_display_4bits_node_131; // Node
    assign ic_display_4bits_ic_display_4bits_node_136 = ic_display_4bits_ic_display_4bits_node_132; // Node
    assign ic_display_4bits_ic_display_4bits_not_137 = ~ic_display_4bits_ic_display_4bits_node_134; // Not
    assign ic_display_4bits_ic_display_4bits_not_138 = ~ic_display_4bits_ic_display_4bits_node_133; // Not
    assign ic_display_4bits_ic_display_4bits_not_139 = ~ic_display_4bits_ic_display_4bits_node_135; // Not
    assign ic_display_4bits_ic_display_4bits_not_140 = ~ic_display_4bits_ic_display_4bits_node_136; // Not
    assign ic_input_ic_input_not_54 = ~input_clock3_3; // Not
    assign ic_input_ic_input_xor_55 = (input_clock3_3 ^ input_clock2_2); // Xor
    assign ic_input_ic_input_or_56 = (input_clock3_3 | input_clock2_2); // Or
    assign ic_input_ic_input_and_57 = (input_clock3_3 & input_clock2_2); // And
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_258_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_259_0_q <= 1'b0; jk_flip_flop_259_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_259_0_q <= 1'b1; jk_flip_flop_259_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_259_0_q <= jk_flip_flop_259_1_q; jk_flip_flop_259_1_q <= jk_flip_flop_259_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_257_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_258_0_q <= 1'b0; jk_flip_flop_258_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_258_0_q <= 1'b1; jk_flip_flop_258_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_258_0_q <= jk_flip_flop_258_1_q; jk_flip_flop_258_1_q <= jk_flip_flop_258_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        jk_flip_flop_257_0_q = 1'b0;
        jk_flip_flop_257_1_q = 1'b1;
    end

    assign ic_jkflipflop_ic_jkflipflop_node_35 = ic_jkflipflop_ic_jkflipflop_node_45; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_36 = ic_jkflipflop_ic_jkflipflop_node_47; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_38 = ic_jkflipflop_ic_jkflipflop_node_41; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_39 = ic_jkflipflop_ic_jkflipflop_node_40; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_43 = ~ic_jkflipflop_ic_jkflipflop_node_48; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_44 = (ic_jkflipflop_ic_jkflipflop_node_50 & ic_jkflipflop_ic_jkflipflop_node_39); // And
    assign ic_jkflipflop_ic_jkflipflop_or_46 = (ic_jkflipflop_ic_jkflipflop_and_51 | ic_jkflipflop_ic_jkflipflop_and_44); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_48 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_50 = ic_jkflipflop_ic_jkflipflop_node_49; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_51 = (ic_jkflipflop_ic_jkflipflop_node_38 & ic_jkflipflop_ic_jkflipflop_not_43); // And

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_8 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_9 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_10 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_11 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_12 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_13 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_14 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_g_middle_15 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_f_upper_left_16 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_e_lower_left_17 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_d_bottom_18 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_a_top_19 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_b_upper_right_20 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_dp_dot_21 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_c_lower_right_22 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_g_middle_23 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_f_upper_left_24 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_e_lower_left_25 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_d_bottom_26 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_a_top_27 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_b_upper_right_28 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_dp_dot_29 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_c_lower_right_30 = 1'b0; // 7-Segment Display
    assign output_led4_0_31 = 1'b0; // LED
    assign output_led5_0_32 = 1'b0; // LED
    assign output_led6_0_33 = 1'b0; // LED
    assign output_led7_0_34 = jk_flip_flop_260_0_q; // LED

endmodule // ic

// ====================================================================
// Module ic generation completed successfully
// Elements processed: 22
// Inputs: 6, Outputs: 28
// Warnings: 5
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 4 is not connected
//   IC INPUT output 3 is not connected
// ====================================================================
