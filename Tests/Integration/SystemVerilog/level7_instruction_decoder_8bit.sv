// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Decoder4to16_low (generated from level2_decoder_4to16.panda)
module level2_decoder_4to16 (
    input addr0,
    input addr1,
    input addr2,
    input addr3,
    output out0,
    output out1,
    output out2,
    output out3,
    output out4,
    output out5,
    output out6,
    output out7,
    output out8,
    output out9,
    output out10,
    output out11,
    output out12,
    output out13,
    output out14,
    output out15
);

wire aux_not_1;
wire aux_not_2;
wire aux_not_3;
wire aux_not_4;
wire aux_and_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;
wire aux_and_9;
wire aux_and_10;
wire aux_and_11;
wire aux_and_12;
wire aux_and_13;
wire aux_and_14;
wire aux_and_15;
wire aux_and_16;
wire aux_and_17;
wire aux_and_18;
wire aux_and_19;
wire aux_and_20;

// Internal logic
assign aux_not_1 = ~addr0;
assign aux_not_2 = ~addr1;
assign aux_not_3 = ~addr2;
assign aux_not_4 = ~addr3;
assign aux_and_5 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4);
assign aux_and_6 = (addr0 & aux_not_2 & aux_not_3 & aux_not_4);
assign aux_and_7 = (aux_not_1 & addr1 & aux_not_3 & aux_not_4);
assign aux_and_8 = (addr0 & addr1 & aux_not_3 & aux_not_4);
assign aux_and_9 = (aux_not_1 & aux_not_2 & addr2 & aux_not_4);
assign aux_and_10 = (addr0 & aux_not_2 & addr2 & aux_not_4);
assign aux_and_11 = (aux_not_1 & addr1 & addr2 & aux_not_4);
assign aux_and_12 = (addr0 & addr1 & addr2 & aux_not_4);
assign aux_and_13 = (aux_not_1 & aux_not_2 & aux_not_3 & addr3);
assign aux_and_14 = (addr0 & aux_not_2 & aux_not_3 & addr3);
assign aux_and_15 = (aux_not_1 & addr1 & aux_not_3 & addr3);
assign aux_and_16 = (addr0 & addr1 & aux_not_3 & addr3);
assign aux_and_17 = (aux_not_1 & aux_not_2 & addr2 & addr3);
assign aux_and_18 = (addr0 & aux_not_2 & addr2 & addr3);
assign aux_and_19 = (aux_not_1 & addr1 & addr2 & addr3);
assign aux_and_20 = (addr0 & addr1 & addr2 & addr3);

assign out0 = aux_and_5;
assign out1 = aux_and_6;
assign out2 = aux_and_7;
assign out3 = aux_and_8;
assign out4 = aux_and_9;
assign out5 = aux_and_10;
assign out6 = aux_and_11;
assign out7 = aux_and_12;
assign out8 = aux_and_13;
assign out9 = aux_and_14;
assign out10 = aux_and_15;
assign out11 = aux_and_16;
assign out12 = aux_and_17;
assign out13 = aux_and_18;
assign out14 = aux_and_19;
assign out15 = aux_and_20;
endmodule

// Module for LEVEL7_INSTRUCTION_DECODER_8BIT (generated from level7_instruction_decoder_8bit.panda)
module level7_instruction_decoder_8bit_ic (
    input instr0,
    input instr1,
    input instr2,
    input instr3,
    input instr4,
    input instr5,
    input instr6,
    input instr7,
    output op0,
    output op1,
    output op2,
    output op3,
    output op4,
    output op5,
    output op6,
    output op7,
    output op8,
    output op9,
    output op10,
    output op11,
    output op12,
    output op13,
    output op14,
    output op15,
    output op16,
    output op17,
    output op18,
    output op19,
    output op20,
    output op21,
    output op22,
    output op23,
    output op24,
    output op25,
    output op26,
    output op27,
    output op28,
    output op29,
    output op30,
    output op31,
    output op32,
    output op33,
    output op34,
    output op35,
    output op36,
    output op37,
    output op38,
    output op39,
    output op40,
    output op41,
    output op42,
    output op43,
    output op44,
    output op45,
    output op46,
    output op47,
    output op48,
    output op49,
    output op50,
    output op51,
    output op52,
    output op53,
    output op54,
    output op55,
    output op56,
    output op57,
    output op58,
    output op59,
    output op60,
    output op61,
    output op62,
    output op63,
    output op64,
    output op65,
    output op66,
    output op67,
    output op68,
    output op69,
    output op70,
    output op71,
    output op72,
    output op73,
    output op74,
    output op75,
    output op76,
    output op77,
    output op78,
    output op79,
    output op80,
    output op81,
    output op82,
    output op83,
    output op84,
    output op85,
    output op86,
    output op87,
    output op88,
    output op89,
    output op90,
    output op91,
    output op92,
    output op93,
    output op94,
    output op95,
    output op96,
    output op97,
    output op98,
    output op99,
    output op100,
    output op101,
    output op102,
    output op103,
    output op104,
    output op105,
    output op106,
    output op107,
    output op108,
    output op109,
    output op110,
    output op111,
    output op112,
    output op113,
    output op114,
    output op115,
    output op116,
    output op117,
    output op118,
    output op119,
    output op120,
    output op121,
    output op122,
    output op123,
    output op124,
    output op125,
    output op126,
    output op127,
    output op128,
    output op129,
    output op130,
    output op131,
    output op132,
    output op133,
    output op134,
    output op135,
    output op136,
    output op137,
    output op138,
    output op139,
    output op140,
    output op141,
    output op142,
    output op143,
    output op144,
    output op145,
    output op146,
    output op147,
    output op148,
    output op149,
    output op150,
    output op151,
    output op152,
    output op153,
    output op154,
    output op155,
    output op156,
    output op157,
    output op158,
    output op159,
    output op160,
    output op161,
    output op162,
    output op163,
    output op164,
    output op165,
    output op166,
    output op167,
    output op168,
    output op169,
    output op170,
    output op171,
    output op172,
    output op173,
    output op174,
    output op175,
    output op176,
    output op177,
    output op178,
    output op179,
    output op180,
    output op181,
    output op182,
    output op183,
    output op184,
    output op185,
    output op186,
    output op187,
    output op188,
    output op189,
    output op190,
    output op191,
    output op192,
    output op193,
    output op194,
    output op195,
    output op196,
    output op197,
    output op198,
    output op199,
    output op200,
    output op201,
    output op202,
    output op203,
    output op204,
    output op205,
    output op206,
    output op207,
    output op208,
    output op209,
    output op210,
    output op211,
    output op212,
    output op213,
    output op214,
    output op215,
    output op216,
    output op217,
    output op218,
    output op219,
    output op220,
    output op221,
    output op222,
    output op223,
    output op224,
    output op225,
    output op226,
    output op227,
    output op228,
    output op229,
    output op230,
    output op231,
    output op232,
    output op233,
    output op234,
    output op235,
    output op236,
    output op237,
    output op238,
    output op239,
    output op240,
    output op241,
    output op242,
    output op243,
    output op244,
    output op245,
    output op246,
    output op247,
    output op248,
    output op249,
    output op250,
    output op251,
    output op252,
    output op253,
    output op254,
    output op255
);

// IC instance: Decoder4to16_low (level2_decoder_4to16)
wire w_level2_decoder_4to16_inst_1_out0;
wire w_level2_decoder_4to16_inst_1_out1;
wire w_level2_decoder_4to16_inst_1_out2;
wire w_level2_decoder_4to16_inst_1_out3;
wire w_level2_decoder_4to16_inst_1_out4;
wire w_level2_decoder_4to16_inst_1_out5;
wire w_level2_decoder_4to16_inst_1_out6;
wire w_level2_decoder_4to16_inst_1_out7;
wire w_level2_decoder_4to16_inst_1_out8;
wire w_level2_decoder_4to16_inst_1_out9;
wire w_level2_decoder_4to16_inst_1_out10;
wire w_level2_decoder_4to16_inst_1_out11;
wire w_level2_decoder_4to16_inst_1_out12;
wire w_level2_decoder_4to16_inst_1_out13;
wire w_level2_decoder_4to16_inst_1_out14;
wire w_level2_decoder_4to16_inst_1_out15;
// IC instance: Decoder4to16_high (level2_decoder_4to16)
wire w_level2_decoder_4to16_inst_2_out0;
wire w_level2_decoder_4to16_inst_2_out1;
wire w_level2_decoder_4to16_inst_2_out2;
wire w_level2_decoder_4to16_inst_2_out3;
wire w_level2_decoder_4to16_inst_2_out4;
wire w_level2_decoder_4to16_inst_2_out5;
wire w_level2_decoder_4to16_inst_2_out6;
wire w_level2_decoder_4to16_inst_2_out7;
wire w_level2_decoder_4to16_inst_2_out8;
wire w_level2_decoder_4to16_inst_2_out9;
wire w_level2_decoder_4to16_inst_2_out10;
wire w_level2_decoder_4to16_inst_2_out11;
wire w_level2_decoder_4to16_inst_2_out12;
wire w_level2_decoder_4to16_inst_2_out13;
wire w_level2_decoder_4to16_inst_2_out14;
wire w_level2_decoder_4to16_inst_2_out15;
wire aux_and_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;
wire aux_and_9;
wire aux_and_10;
wire aux_and_11;
wire aux_and_12;
wire aux_and_13;
wire aux_and_14;
wire aux_and_15;
wire aux_and_16;
wire aux_and_17;
wire aux_and_18;
wire aux_and_19;
wire aux_and_20;
wire aux_and_21;
wire aux_and_22;
wire aux_and_23;
wire aux_and_24;
wire aux_and_25;
wire aux_and_26;
wire aux_and_27;
wire aux_and_28;
wire aux_and_29;
wire aux_and_30;
wire aux_and_31;
wire aux_and_32;
wire aux_and_33;
wire aux_and_34;
wire aux_and_35;
wire aux_and_36;
wire aux_and_37;
wire aux_and_38;
wire aux_and_39;
wire aux_and_40;
wire aux_and_41;
wire aux_and_42;
wire aux_and_43;
wire aux_and_44;
wire aux_and_45;
wire aux_and_46;
wire aux_and_47;
wire aux_and_48;
wire aux_and_49;
wire aux_and_50;
wire aux_and_51;
wire aux_and_52;
wire aux_and_53;
wire aux_and_54;
wire aux_and_55;
wire aux_and_56;
wire aux_and_57;
wire aux_and_58;
wire aux_and_59;
wire aux_and_60;
wire aux_and_61;
wire aux_and_62;
wire aux_and_63;
wire aux_and_64;
wire aux_and_65;
wire aux_and_66;
wire aux_and_67;
wire aux_and_68;
wire aux_and_69;
wire aux_and_70;
wire aux_and_71;
wire aux_and_72;
wire aux_and_73;
wire aux_and_74;
wire aux_and_75;
wire aux_and_76;
wire aux_and_77;
wire aux_and_78;
wire aux_and_79;
wire aux_and_80;
wire aux_and_81;
wire aux_and_82;
wire aux_and_83;
wire aux_and_84;
wire aux_and_85;
wire aux_and_86;
wire aux_and_87;
wire aux_and_88;
wire aux_and_89;
wire aux_and_90;
wire aux_and_91;
wire aux_and_92;
wire aux_and_93;
wire aux_and_94;
wire aux_and_95;
wire aux_and_96;
wire aux_and_97;
wire aux_and_98;
wire aux_and_99;
wire aux_and_100;
wire aux_and_101;
wire aux_and_102;
wire aux_and_103;
wire aux_and_104;
wire aux_and_105;
wire aux_and_106;
wire aux_and_107;
wire aux_and_108;
wire aux_and_109;
wire aux_and_110;
wire aux_and_111;
wire aux_and_112;
wire aux_and_113;
wire aux_and_114;
wire aux_and_115;
wire aux_and_116;
wire aux_and_117;
wire aux_and_118;
wire aux_and_119;
wire aux_and_120;
wire aux_and_121;
wire aux_and_122;
wire aux_and_123;
wire aux_and_124;
wire aux_and_125;
wire aux_and_126;
wire aux_and_127;
wire aux_and_128;
wire aux_and_129;
wire aux_and_130;
wire aux_and_131;
wire aux_and_132;
wire aux_and_133;
wire aux_and_134;
wire aux_and_135;
wire aux_and_136;
wire aux_and_137;
wire aux_and_138;
wire aux_and_139;
wire aux_and_140;
wire aux_and_141;
wire aux_and_142;
wire aux_and_143;
wire aux_and_144;
wire aux_and_145;
wire aux_and_146;
wire aux_and_147;
wire aux_and_148;
wire aux_and_149;
wire aux_and_150;
wire aux_and_151;
wire aux_and_152;
wire aux_and_153;
wire aux_and_154;
wire aux_and_155;
wire aux_and_156;
wire aux_and_157;
wire aux_and_158;
wire aux_and_159;
wire aux_and_160;
wire aux_and_161;
wire aux_and_162;
wire aux_and_163;
wire aux_and_164;
wire aux_and_165;
wire aux_and_166;
wire aux_and_167;
wire aux_and_168;
wire aux_and_169;
wire aux_and_170;
wire aux_and_171;
wire aux_and_172;
wire aux_and_173;
wire aux_and_174;
wire aux_and_175;
wire aux_and_176;
wire aux_and_177;
wire aux_and_178;
wire aux_and_179;
wire aux_and_180;
wire aux_and_181;
wire aux_and_182;
wire aux_and_183;
wire aux_and_184;
wire aux_and_185;
wire aux_and_186;
wire aux_and_187;
wire aux_and_188;
wire aux_and_189;
wire aux_and_190;
wire aux_and_191;
wire aux_and_192;
wire aux_and_193;
wire aux_and_194;
wire aux_and_195;
wire aux_and_196;
wire aux_and_197;
wire aux_and_198;
wire aux_and_199;
wire aux_and_200;
wire aux_and_201;
wire aux_and_202;
wire aux_and_203;
wire aux_and_204;
wire aux_and_205;
wire aux_and_206;
wire aux_and_207;
wire aux_and_208;
wire aux_and_209;
wire aux_and_210;
wire aux_and_211;
wire aux_and_212;
wire aux_and_213;
wire aux_and_214;
wire aux_and_215;
wire aux_and_216;
wire aux_and_217;
wire aux_and_218;
wire aux_and_219;
wire aux_and_220;
wire aux_and_221;
wire aux_and_222;
wire aux_and_223;
wire aux_and_224;
wire aux_and_225;
wire aux_and_226;
wire aux_and_227;
wire aux_and_228;
wire aux_and_229;
wire aux_and_230;
wire aux_and_231;
wire aux_and_232;
wire aux_and_233;
wire aux_and_234;
wire aux_and_235;
wire aux_and_236;
wire aux_and_237;
wire aux_and_238;
wire aux_and_239;
wire aux_and_240;
wire aux_and_241;
wire aux_and_242;
wire aux_and_243;
wire aux_and_244;
wire aux_and_245;
wire aux_and_246;
wire aux_and_247;
wire aux_and_248;
wire aux_and_249;
wire aux_and_250;
wire aux_and_251;
wire aux_and_252;
wire aux_and_253;
wire aux_and_254;
wire aux_and_255;
wire aux_and_256;
wire aux_and_257;
wire aux_and_258;

// Internal logic
level2_decoder_4to16 level2_decoder_4to16_inst_1 (
    .addr0(instr0),
    .addr1(instr1),
    .addr2(instr2),
    .addr3(instr3),
    .out0(w_level2_decoder_4to16_inst_1_out0),
    .out1(w_level2_decoder_4to16_inst_1_out1),
    .out2(w_level2_decoder_4to16_inst_1_out2),
    .out3(w_level2_decoder_4to16_inst_1_out3),
    .out4(w_level2_decoder_4to16_inst_1_out4),
    .out5(w_level2_decoder_4to16_inst_1_out5),
    .out6(w_level2_decoder_4to16_inst_1_out6),
    .out7(w_level2_decoder_4to16_inst_1_out7),
    .out8(w_level2_decoder_4to16_inst_1_out8),
    .out9(w_level2_decoder_4to16_inst_1_out9),
    .out10(w_level2_decoder_4to16_inst_1_out10),
    .out11(w_level2_decoder_4to16_inst_1_out11),
    .out12(w_level2_decoder_4to16_inst_1_out12),
    .out13(w_level2_decoder_4to16_inst_1_out13),
    .out14(w_level2_decoder_4to16_inst_1_out14),
    .out15(w_level2_decoder_4to16_inst_1_out15)
);
level2_decoder_4to16 level2_decoder_4to16_inst_2 (
    .addr0(instr4),
    .addr1(instr5),
    .addr2(instr6),
    .addr3(instr7),
    .out0(w_level2_decoder_4to16_inst_2_out0),
    .out1(w_level2_decoder_4to16_inst_2_out1),
    .out2(w_level2_decoder_4to16_inst_2_out2),
    .out3(w_level2_decoder_4to16_inst_2_out3),
    .out4(w_level2_decoder_4to16_inst_2_out4),
    .out5(w_level2_decoder_4to16_inst_2_out5),
    .out6(w_level2_decoder_4to16_inst_2_out6),
    .out7(w_level2_decoder_4to16_inst_2_out7),
    .out8(w_level2_decoder_4to16_inst_2_out8),
    .out9(w_level2_decoder_4to16_inst_2_out9),
    .out10(w_level2_decoder_4to16_inst_2_out10),
    .out11(w_level2_decoder_4to16_inst_2_out11),
    .out12(w_level2_decoder_4to16_inst_2_out12),
    .out13(w_level2_decoder_4to16_inst_2_out13),
    .out14(w_level2_decoder_4to16_inst_2_out14),
    .out15(w_level2_decoder_4to16_inst_2_out15)
);
assign aux_and_3 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_4 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_5 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_6 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_7 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_8 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_9 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_10 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_11 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_12 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_13 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_14 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_15 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_16 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_17 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_18 = (w_level2_decoder_4to16_inst_2_out0 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_19 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_20 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_21 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_22 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_23 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_24 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_25 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_26 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_27 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_28 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_29 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_30 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_31 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_32 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_33 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_34 = (w_level2_decoder_4to16_inst_2_out1 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_35 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_36 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_37 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_38 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_39 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_40 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_41 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_42 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_43 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_44 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_45 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_46 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_47 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_48 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_49 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_50 = (w_level2_decoder_4to16_inst_2_out2 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_51 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_52 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_53 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_54 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_55 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_56 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_57 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_58 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_59 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_60 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_61 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_62 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_63 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_64 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_65 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_66 = (w_level2_decoder_4to16_inst_2_out3 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_67 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_68 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_69 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_70 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_71 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_72 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_73 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_74 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_75 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_76 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_77 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_78 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_79 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_80 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_81 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_82 = (w_level2_decoder_4to16_inst_2_out4 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_83 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_84 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_85 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_86 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_87 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_88 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_89 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_90 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_91 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_92 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_93 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_94 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_95 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_96 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_97 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_98 = (w_level2_decoder_4to16_inst_2_out5 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_99 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_100 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_101 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_102 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_103 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_104 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_105 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_106 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_107 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_108 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_109 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_110 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_111 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_112 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_113 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_114 = (w_level2_decoder_4to16_inst_2_out6 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_115 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_116 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_117 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_118 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_119 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_120 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_121 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_122 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_123 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_124 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_125 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_126 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_127 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_128 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_129 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_130 = (w_level2_decoder_4to16_inst_2_out7 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_131 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_132 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_133 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_134 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_135 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_136 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_137 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_138 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_139 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_140 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_141 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_142 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_143 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_144 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_145 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_146 = (w_level2_decoder_4to16_inst_2_out8 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_147 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_148 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_149 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_150 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_151 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_152 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_153 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_154 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_155 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_156 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_157 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_158 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_159 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_160 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_161 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_162 = (w_level2_decoder_4to16_inst_2_out9 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_163 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_164 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_165 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_166 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_167 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_168 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_169 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_170 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_171 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_172 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_173 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_174 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_175 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_176 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_177 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_178 = (w_level2_decoder_4to16_inst_2_out10 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_179 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_180 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_181 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_182 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_183 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_184 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_185 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_186 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_187 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_188 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_189 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_190 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_191 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_192 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_193 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_194 = (w_level2_decoder_4to16_inst_2_out11 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_195 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_196 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_197 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_198 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_199 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_200 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_201 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_202 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_203 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_204 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_205 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_206 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_207 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_208 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_209 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_210 = (w_level2_decoder_4to16_inst_2_out12 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_211 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_212 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_213 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_214 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_215 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_216 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_217 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_218 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_219 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_220 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_221 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_222 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_223 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_224 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_225 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_226 = (w_level2_decoder_4to16_inst_2_out13 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_227 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_228 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_229 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_230 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_231 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_232 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_233 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_234 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_235 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_236 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_237 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_238 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_239 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_240 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_241 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_242 = (w_level2_decoder_4to16_inst_2_out14 & w_level2_decoder_4to16_inst_1_out15);
assign aux_and_243 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out0);
assign aux_and_244 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out1);
assign aux_and_245 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out2);
assign aux_and_246 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out3);
assign aux_and_247 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out4);
assign aux_and_248 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out5);
assign aux_and_249 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out6);
assign aux_and_250 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out7);
assign aux_and_251 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out8);
assign aux_and_252 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out9);
assign aux_and_253 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out10);
assign aux_and_254 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out11);
assign aux_and_255 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out12);
assign aux_and_256 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out13);
assign aux_and_257 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out14);
assign aux_and_258 = (w_level2_decoder_4to16_inst_2_out15 & w_level2_decoder_4to16_inst_1_out15);

assign op0 = aux_and_3;
assign op1 = aux_and_4;
assign op2 = aux_and_5;
assign op3 = aux_and_6;
assign op4 = aux_and_7;
assign op5 = aux_and_8;
assign op6 = aux_and_9;
assign op7 = aux_and_10;
assign op8 = aux_and_11;
assign op9 = aux_and_12;
assign op10 = aux_and_13;
assign op11 = aux_and_14;
assign op12 = aux_and_15;
assign op13 = aux_and_16;
assign op14 = aux_and_17;
assign op15 = aux_and_18;
assign op16 = aux_and_19;
assign op17 = aux_and_20;
assign op18 = aux_and_21;
assign op19 = aux_and_22;
assign op20 = aux_and_23;
assign op21 = aux_and_24;
assign op22 = aux_and_25;
assign op23 = aux_and_26;
assign op24 = aux_and_27;
assign op25 = aux_and_28;
assign op26 = aux_and_29;
assign op27 = aux_and_30;
assign op28 = aux_and_31;
assign op29 = aux_and_32;
assign op30 = aux_and_33;
assign op31 = aux_and_34;
assign op32 = aux_and_35;
assign op33 = aux_and_36;
assign op34 = aux_and_37;
assign op35 = aux_and_38;
assign op36 = aux_and_39;
assign op37 = aux_and_40;
assign op38 = aux_and_41;
assign op39 = aux_and_42;
assign op40 = aux_and_43;
assign op41 = aux_and_44;
assign op42 = aux_and_45;
assign op43 = aux_and_46;
assign op44 = aux_and_47;
assign op45 = aux_and_48;
assign op46 = aux_and_49;
assign op47 = aux_and_50;
assign op48 = aux_and_51;
assign op49 = aux_and_52;
assign op50 = aux_and_53;
assign op51 = aux_and_54;
assign op52 = aux_and_55;
assign op53 = aux_and_56;
assign op54 = aux_and_57;
assign op55 = aux_and_58;
assign op56 = aux_and_59;
assign op57 = aux_and_60;
assign op58 = aux_and_61;
assign op59 = aux_and_62;
assign op60 = aux_and_63;
assign op61 = aux_and_64;
assign op62 = aux_and_65;
assign op63 = aux_and_66;
assign op64 = aux_and_67;
assign op65 = aux_and_68;
assign op66 = aux_and_69;
assign op67 = aux_and_70;
assign op68 = aux_and_71;
assign op69 = aux_and_72;
assign op70 = aux_and_73;
assign op71 = aux_and_74;
assign op72 = aux_and_75;
assign op73 = aux_and_76;
assign op74 = aux_and_77;
assign op75 = aux_and_78;
assign op76 = aux_and_79;
assign op77 = aux_and_80;
assign op78 = aux_and_81;
assign op79 = aux_and_82;
assign op80 = aux_and_83;
assign op81 = aux_and_84;
assign op82 = aux_and_85;
assign op83 = aux_and_86;
assign op84 = aux_and_87;
assign op85 = aux_and_88;
assign op86 = aux_and_89;
assign op87 = aux_and_90;
assign op88 = aux_and_91;
assign op89 = aux_and_92;
assign op90 = aux_and_93;
assign op91 = aux_and_94;
assign op92 = aux_and_95;
assign op93 = aux_and_96;
assign op94 = aux_and_97;
assign op95 = aux_and_98;
assign op96 = aux_and_99;
assign op97 = aux_and_100;
assign op98 = aux_and_101;
assign op99 = aux_and_102;
assign op100 = aux_and_103;
assign op101 = aux_and_104;
assign op102 = aux_and_105;
assign op103 = aux_and_106;
assign op104 = aux_and_107;
assign op105 = aux_and_108;
assign op106 = aux_and_109;
assign op107 = aux_and_110;
assign op108 = aux_and_111;
assign op109 = aux_and_112;
assign op110 = aux_and_113;
assign op111 = aux_and_114;
assign op112 = aux_and_115;
assign op113 = aux_and_116;
assign op114 = aux_and_117;
assign op115 = aux_and_118;
assign op116 = aux_and_119;
assign op117 = aux_and_120;
assign op118 = aux_and_121;
assign op119 = aux_and_122;
assign op120 = aux_and_123;
assign op121 = aux_and_124;
assign op122 = aux_and_125;
assign op123 = aux_and_126;
assign op124 = aux_and_127;
assign op125 = aux_and_128;
assign op126 = aux_and_129;
assign op127 = aux_and_130;
assign op128 = aux_and_131;
assign op129 = aux_and_132;
assign op130 = aux_and_133;
assign op131 = aux_and_134;
assign op132 = aux_and_135;
assign op133 = aux_and_136;
assign op134 = aux_and_137;
assign op135 = aux_and_138;
assign op136 = aux_and_139;
assign op137 = aux_and_140;
assign op138 = aux_and_141;
assign op139 = aux_and_142;
assign op140 = aux_and_143;
assign op141 = aux_and_144;
assign op142 = aux_and_145;
assign op143 = aux_and_146;
assign op144 = aux_and_147;
assign op145 = aux_and_148;
assign op146 = aux_and_149;
assign op147 = aux_and_150;
assign op148 = aux_and_151;
assign op149 = aux_and_152;
assign op150 = aux_and_153;
assign op151 = aux_and_154;
assign op152 = aux_and_155;
assign op153 = aux_and_156;
assign op154 = aux_and_157;
assign op155 = aux_and_158;
assign op156 = aux_and_159;
assign op157 = aux_and_160;
assign op158 = aux_and_161;
assign op159 = aux_and_162;
assign op160 = aux_and_163;
assign op161 = aux_and_164;
assign op162 = aux_and_165;
assign op163 = aux_and_166;
assign op164 = aux_and_167;
assign op165 = aux_and_168;
assign op166 = aux_and_169;
assign op167 = aux_and_170;
assign op168 = aux_and_171;
assign op169 = aux_and_172;
assign op170 = aux_and_173;
assign op171 = aux_and_174;
assign op172 = aux_and_175;
assign op173 = aux_and_176;
assign op174 = aux_and_177;
assign op175 = aux_and_178;
assign op176 = aux_and_179;
assign op177 = aux_and_180;
assign op178 = aux_and_181;
assign op179 = aux_and_182;
assign op180 = aux_and_183;
assign op181 = aux_and_184;
assign op182 = aux_and_185;
assign op183 = aux_and_186;
assign op184 = aux_and_187;
assign op185 = aux_and_188;
assign op186 = aux_and_189;
assign op187 = aux_and_190;
assign op188 = aux_and_191;
assign op189 = aux_and_192;
assign op190 = aux_and_193;
assign op191 = aux_and_194;
assign op192 = aux_and_195;
assign op193 = aux_and_196;
assign op194 = aux_and_197;
assign op195 = aux_and_198;
assign op196 = aux_and_199;
assign op197 = aux_and_200;
assign op198 = aux_and_201;
assign op199 = aux_and_202;
assign op200 = aux_and_203;
assign op201 = aux_and_204;
assign op202 = aux_and_205;
assign op203 = aux_and_206;
assign op204 = aux_and_207;
assign op205 = aux_and_208;
assign op206 = aux_and_209;
assign op207 = aux_and_210;
assign op208 = aux_and_211;
assign op209 = aux_and_212;
assign op210 = aux_and_213;
assign op211 = aux_and_214;
assign op212 = aux_and_215;
assign op213 = aux_and_216;
assign op214 = aux_and_217;
assign op215 = aux_and_218;
assign op216 = aux_and_219;
assign op217 = aux_and_220;
assign op218 = aux_and_221;
assign op219 = aux_and_222;
assign op220 = aux_and_223;
assign op221 = aux_and_224;
assign op222 = aux_and_225;
assign op223 = aux_and_226;
assign op224 = aux_and_227;
assign op225 = aux_and_228;
assign op226 = aux_and_229;
assign op227 = aux_and_230;
assign op228 = aux_and_231;
assign op229 = aux_and_232;
assign op230 = aux_and_233;
assign op231 = aux_and_234;
assign op232 = aux_and_235;
assign op233 = aux_and_236;
assign op234 = aux_and_237;
assign op235 = aux_and_238;
assign op236 = aux_and_239;
assign op237 = aux_and_240;
assign op238 = aux_and_241;
assign op239 = aux_and_242;
assign op240 = aux_and_243;
assign op241 = aux_and_244;
assign op242 = aux_and_245;
assign op243 = aux_and_246;
assign op244 = aux_and_247;
assign op245 = aux_and_248;
assign op246 = aux_and_249;
assign op247 = aux_and_250;
assign op248 = aux_and_251;
assign op249 = aux_and_252;
assign op250 = aux_and_253;
assign op251 = aux_and_254;
assign op252 = aux_and_255;
assign op253 = aux_and_256;
assign op254 = aux_and_257;
assign op255 = aux_and_258;
endmodule

module level7_instruction_decoder_8bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,
input input_switch8,

/* ========= Outputs ========== */
output led10_1,
output led11_1,
output led12_1,
output led13_1,
output led14_1,
output led15_1,
output led16_1,
output led17_1,
output led18_1,
output led19_1,
output led20_1,
output led21_1,
output led22_1,
output led23_1,
output led24_1,
output led25_1,
output led26_1,
output led27_1,
output led28_1,
output led29_1,
output led30_1,
output led31_1,
output led32_1,
output led33_1,
output led34_1,
output led35_1,
output led36_1,
output led37_1,
output led38_1,
output led39_1,
output led40_1,
output led41_1,
output led42_1,
output led43_1,
output led44_1,
output led45_1,
output led46_1,
output led47_1,
output led48_1,
output led49_1,
output led50_1,
output led51_1,
output led52_1,
output led53_1,
output led54_1,
output led55_1,
output led56_1,
output led57_1,
output led58_1,
output led59_1,
output led60_1,
output led61_1,
output led62_1,
output led63_1,
output led64_1,
output led65_1,
output led66_1,
output led67_1,
output led68_1,
output led69_1,
output led70_1,
output led71_1,
output led72_1,
output led73_1,
output led74_1,
output led75_1,
output led76_1,
output led77_1,
output led78_1,
output led79_1,
output led80_1,
output led81_1,
output led82_1,
output led83_1,
output led84_1,
output led85_1,
output led86_1,
output led87_1,
output led88_1,
output led89_1,
output led90_1,
output led91_1,
output led92_1,
output led93_1,
output led94_1,
output led95_1,
output led96_1,
output led97_1,
output led98_1,
output led99_1,
output led100_1,
output led101_1,
output led102_1,
output led103_1,
output led104_1,
output led105_1,
output led106_1,
output led107_1,
output led108_1,
output led109_1,
output led110_1,
output led111_1,
output led112_1,
output led113_1,
output led114_1,
output led115_1,
output led116_1,
output led117_1,
output led118_1,
output led119_1,
output led120_1,
output led121_1,
output led122_1,
output led123_1,
output led124_1,
output led125_1,
output led126_1,
output led127_1,
output led128_1,
output led129_1,
output led130_1,
output led131_1,
output led132_1,
output led133_1,
output led134_1,
output led135_1,
output led136_1,
output led137_1,
output led138_1,
output led139_1,
output led140_1,
output led141_1,
output led142_1,
output led143_1,
output led144_1,
output led145_1,
output led146_1,
output led147_1,
output led148_1,
output led149_1,
output led150_1,
output led151_1,
output led152_1,
output led153_1,
output led154_1,
output led155_1,
output led156_1,
output led157_1,
output led158_1,
output led159_1,
output led160_1,
output led161_1,
output led162_1,
output led163_1,
output led164_1,
output led165_1,
output led166_1,
output led167_1,
output led168_1,
output led169_1,
output led170_1,
output led171_1,
output led172_1,
output led173_1,
output led174_1,
output led175_1,
output led176_1,
output led177_1,
output led178_1,
output led179_1,
output led180_1,
output led181_1,
output led182_1,
output led183_1,
output led184_1,
output led185_1,
output led186_1,
output led187_1,
output led188_1,
output led189_1,
output led190_1,
output led191_1,
output led192_1,
output led193_1,
output led194_1,
output led195_1,
output led196_1,
output led197_1,
output led198_1,
output led199_1,
output led200_1,
output led201_1,
output led202_1,
output led203_1,
output led204_1,
output led205_1,
output led206_1,
output led207_1,
output led208_1,
output led209_1,
output led210_1,
output led211_1,
output led212_1,
output led213_1,
output led214_1,
output led215_1,
output led216_1,
output led217_1,
output led218_1,
output led219_1,
output led220_1,
output led221_1,
output led222_1,
output led223_1,
output led224_1,
output led225_1,
output led226_1,
output led227_1,
output led228_1,
output led229_1,
output led230_1,
output led231_1,
output led232_1,
output led233_1,
output led234_1,
output led235_1,
output led236_1,
output led237_1,
output led238_1,
output led239_1,
output led240_1,
output led241_1,
output led242_1,
output led243_1,
output led244_1,
output led245_1,
output led246_1,
output led247_1,
output led248_1,
output led249_1,
output led250_1,
output led251_1,
output led252_1,
output led253_1,
output led254_1,
output led255_1,
output led256_1,
output led257_1,
output led258_1,
output led259_1,
output led260_1,
output led261_1,
output led262_1,
output led263_1,
output led264_1,
output led265_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL7_INSTRUCTION_DECODER_8BIT (level7_instruction_decoder_8bit_ic)
wire w_level7_instruction_decoder_8bit_ic_inst_1_op0;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op1;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op2;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op3;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op4;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op5;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op6;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op7;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op8;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op9;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op10;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op11;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op12;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op13;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op14;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op15;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op16;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op17;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op18;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op19;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op20;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op21;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op22;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op23;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op24;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op25;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op26;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op27;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op28;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op29;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op30;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op31;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op32;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op33;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op34;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op35;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op36;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op37;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op38;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op39;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op40;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op41;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op42;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op43;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op44;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op45;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op46;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op47;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op48;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op49;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op50;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op51;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op52;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op53;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op54;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op55;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op56;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op57;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op58;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op59;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op60;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op61;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op62;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op63;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op64;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op65;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op66;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op67;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op68;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op69;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op70;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op71;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op72;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op73;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op74;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op75;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op76;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op77;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op78;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op79;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op80;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op81;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op82;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op83;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op84;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op85;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op86;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op87;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op88;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op89;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op90;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op91;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op92;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op93;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op94;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op95;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op96;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op97;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op98;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op99;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op100;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op101;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op102;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op103;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op104;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op105;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op106;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op107;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op108;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op109;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op110;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op111;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op112;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op113;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op114;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op115;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op116;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op117;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op118;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op119;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op120;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op121;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op122;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op123;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op124;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op125;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op126;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op127;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op128;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op129;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op130;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op131;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op132;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op133;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op134;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op135;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op136;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op137;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op138;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op139;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op140;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op141;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op142;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op143;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op144;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op145;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op146;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op147;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op148;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op149;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op150;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op151;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op152;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op153;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op154;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op155;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op156;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op157;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op158;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op159;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op160;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op161;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op162;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op163;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op164;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op165;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op166;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op167;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op168;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op169;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op170;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op171;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op172;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op173;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op174;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op175;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op176;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op177;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op178;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op179;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op180;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op181;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op182;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op183;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op184;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op185;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op186;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op187;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op188;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op189;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op190;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op191;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op192;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op193;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op194;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op195;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op196;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op197;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op198;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op199;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op200;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op201;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op202;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op203;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op204;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op205;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op206;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op207;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op208;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op209;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op210;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op211;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op212;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op213;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op214;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op215;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op216;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op217;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op218;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op219;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op220;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op221;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op222;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op223;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op224;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op225;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op226;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op227;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op228;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op229;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op230;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op231;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op232;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op233;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op234;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op235;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op236;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op237;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op238;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op239;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op240;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op241;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op242;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op243;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op244;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op245;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op246;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op247;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op248;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op249;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op250;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op251;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op252;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op253;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op254;
wire w_level7_instruction_decoder_8bit_ic_inst_1_op255;


// Assigning aux variables. //
level7_instruction_decoder_8bit_ic level7_instruction_decoder_8bit_ic_inst_1 (
    .instr0(input_switch1),
    .instr1(input_switch2),
    .instr2(input_switch3),
    .instr3(input_switch4),
    .instr4(input_switch5),
    .instr5(input_switch6),
    .instr6(input_switch7),
    .instr7(input_switch8),
    .op0(w_level7_instruction_decoder_8bit_ic_inst_1_op0),
    .op1(w_level7_instruction_decoder_8bit_ic_inst_1_op1),
    .op2(w_level7_instruction_decoder_8bit_ic_inst_1_op2),
    .op3(w_level7_instruction_decoder_8bit_ic_inst_1_op3),
    .op4(w_level7_instruction_decoder_8bit_ic_inst_1_op4),
    .op5(w_level7_instruction_decoder_8bit_ic_inst_1_op5),
    .op6(w_level7_instruction_decoder_8bit_ic_inst_1_op6),
    .op7(w_level7_instruction_decoder_8bit_ic_inst_1_op7),
    .op8(w_level7_instruction_decoder_8bit_ic_inst_1_op8),
    .op9(w_level7_instruction_decoder_8bit_ic_inst_1_op9),
    .op10(w_level7_instruction_decoder_8bit_ic_inst_1_op10),
    .op11(w_level7_instruction_decoder_8bit_ic_inst_1_op11),
    .op12(w_level7_instruction_decoder_8bit_ic_inst_1_op12),
    .op13(w_level7_instruction_decoder_8bit_ic_inst_1_op13),
    .op14(w_level7_instruction_decoder_8bit_ic_inst_1_op14),
    .op15(w_level7_instruction_decoder_8bit_ic_inst_1_op15),
    .op16(w_level7_instruction_decoder_8bit_ic_inst_1_op16),
    .op17(w_level7_instruction_decoder_8bit_ic_inst_1_op17),
    .op18(w_level7_instruction_decoder_8bit_ic_inst_1_op18),
    .op19(w_level7_instruction_decoder_8bit_ic_inst_1_op19),
    .op20(w_level7_instruction_decoder_8bit_ic_inst_1_op20),
    .op21(w_level7_instruction_decoder_8bit_ic_inst_1_op21),
    .op22(w_level7_instruction_decoder_8bit_ic_inst_1_op22),
    .op23(w_level7_instruction_decoder_8bit_ic_inst_1_op23),
    .op24(w_level7_instruction_decoder_8bit_ic_inst_1_op24),
    .op25(w_level7_instruction_decoder_8bit_ic_inst_1_op25),
    .op26(w_level7_instruction_decoder_8bit_ic_inst_1_op26),
    .op27(w_level7_instruction_decoder_8bit_ic_inst_1_op27),
    .op28(w_level7_instruction_decoder_8bit_ic_inst_1_op28),
    .op29(w_level7_instruction_decoder_8bit_ic_inst_1_op29),
    .op30(w_level7_instruction_decoder_8bit_ic_inst_1_op30),
    .op31(w_level7_instruction_decoder_8bit_ic_inst_1_op31),
    .op32(w_level7_instruction_decoder_8bit_ic_inst_1_op32),
    .op33(w_level7_instruction_decoder_8bit_ic_inst_1_op33),
    .op34(w_level7_instruction_decoder_8bit_ic_inst_1_op34),
    .op35(w_level7_instruction_decoder_8bit_ic_inst_1_op35),
    .op36(w_level7_instruction_decoder_8bit_ic_inst_1_op36),
    .op37(w_level7_instruction_decoder_8bit_ic_inst_1_op37),
    .op38(w_level7_instruction_decoder_8bit_ic_inst_1_op38),
    .op39(w_level7_instruction_decoder_8bit_ic_inst_1_op39),
    .op40(w_level7_instruction_decoder_8bit_ic_inst_1_op40),
    .op41(w_level7_instruction_decoder_8bit_ic_inst_1_op41),
    .op42(w_level7_instruction_decoder_8bit_ic_inst_1_op42),
    .op43(w_level7_instruction_decoder_8bit_ic_inst_1_op43),
    .op44(w_level7_instruction_decoder_8bit_ic_inst_1_op44),
    .op45(w_level7_instruction_decoder_8bit_ic_inst_1_op45),
    .op46(w_level7_instruction_decoder_8bit_ic_inst_1_op46),
    .op47(w_level7_instruction_decoder_8bit_ic_inst_1_op47),
    .op48(w_level7_instruction_decoder_8bit_ic_inst_1_op48),
    .op49(w_level7_instruction_decoder_8bit_ic_inst_1_op49),
    .op50(w_level7_instruction_decoder_8bit_ic_inst_1_op50),
    .op51(w_level7_instruction_decoder_8bit_ic_inst_1_op51),
    .op52(w_level7_instruction_decoder_8bit_ic_inst_1_op52),
    .op53(w_level7_instruction_decoder_8bit_ic_inst_1_op53),
    .op54(w_level7_instruction_decoder_8bit_ic_inst_1_op54),
    .op55(w_level7_instruction_decoder_8bit_ic_inst_1_op55),
    .op56(w_level7_instruction_decoder_8bit_ic_inst_1_op56),
    .op57(w_level7_instruction_decoder_8bit_ic_inst_1_op57),
    .op58(w_level7_instruction_decoder_8bit_ic_inst_1_op58),
    .op59(w_level7_instruction_decoder_8bit_ic_inst_1_op59),
    .op60(w_level7_instruction_decoder_8bit_ic_inst_1_op60),
    .op61(w_level7_instruction_decoder_8bit_ic_inst_1_op61),
    .op62(w_level7_instruction_decoder_8bit_ic_inst_1_op62),
    .op63(w_level7_instruction_decoder_8bit_ic_inst_1_op63),
    .op64(w_level7_instruction_decoder_8bit_ic_inst_1_op64),
    .op65(w_level7_instruction_decoder_8bit_ic_inst_1_op65),
    .op66(w_level7_instruction_decoder_8bit_ic_inst_1_op66),
    .op67(w_level7_instruction_decoder_8bit_ic_inst_1_op67),
    .op68(w_level7_instruction_decoder_8bit_ic_inst_1_op68),
    .op69(w_level7_instruction_decoder_8bit_ic_inst_1_op69),
    .op70(w_level7_instruction_decoder_8bit_ic_inst_1_op70),
    .op71(w_level7_instruction_decoder_8bit_ic_inst_1_op71),
    .op72(w_level7_instruction_decoder_8bit_ic_inst_1_op72),
    .op73(w_level7_instruction_decoder_8bit_ic_inst_1_op73),
    .op74(w_level7_instruction_decoder_8bit_ic_inst_1_op74),
    .op75(w_level7_instruction_decoder_8bit_ic_inst_1_op75),
    .op76(w_level7_instruction_decoder_8bit_ic_inst_1_op76),
    .op77(w_level7_instruction_decoder_8bit_ic_inst_1_op77),
    .op78(w_level7_instruction_decoder_8bit_ic_inst_1_op78),
    .op79(w_level7_instruction_decoder_8bit_ic_inst_1_op79),
    .op80(w_level7_instruction_decoder_8bit_ic_inst_1_op80),
    .op81(w_level7_instruction_decoder_8bit_ic_inst_1_op81),
    .op82(w_level7_instruction_decoder_8bit_ic_inst_1_op82),
    .op83(w_level7_instruction_decoder_8bit_ic_inst_1_op83),
    .op84(w_level7_instruction_decoder_8bit_ic_inst_1_op84),
    .op85(w_level7_instruction_decoder_8bit_ic_inst_1_op85),
    .op86(w_level7_instruction_decoder_8bit_ic_inst_1_op86),
    .op87(w_level7_instruction_decoder_8bit_ic_inst_1_op87),
    .op88(w_level7_instruction_decoder_8bit_ic_inst_1_op88),
    .op89(w_level7_instruction_decoder_8bit_ic_inst_1_op89),
    .op90(w_level7_instruction_decoder_8bit_ic_inst_1_op90),
    .op91(w_level7_instruction_decoder_8bit_ic_inst_1_op91),
    .op92(w_level7_instruction_decoder_8bit_ic_inst_1_op92),
    .op93(w_level7_instruction_decoder_8bit_ic_inst_1_op93),
    .op94(w_level7_instruction_decoder_8bit_ic_inst_1_op94),
    .op95(w_level7_instruction_decoder_8bit_ic_inst_1_op95),
    .op96(w_level7_instruction_decoder_8bit_ic_inst_1_op96),
    .op97(w_level7_instruction_decoder_8bit_ic_inst_1_op97),
    .op98(w_level7_instruction_decoder_8bit_ic_inst_1_op98),
    .op99(w_level7_instruction_decoder_8bit_ic_inst_1_op99),
    .op100(w_level7_instruction_decoder_8bit_ic_inst_1_op100),
    .op101(w_level7_instruction_decoder_8bit_ic_inst_1_op101),
    .op102(w_level7_instruction_decoder_8bit_ic_inst_1_op102),
    .op103(w_level7_instruction_decoder_8bit_ic_inst_1_op103),
    .op104(w_level7_instruction_decoder_8bit_ic_inst_1_op104),
    .op105(w_level7_instruction_decoder_8bit_ic_inst_1_op105),
    .op106(w_level7_instruction_decoder_8bit_ic_inst_1_op106),
    .op107(w_level7_instruction_decoder_8bit_ic_inst_1_op107),
    .op108(w_level7_instruction_decoder_8bit_ic_inst_1_op108),
    .op109(w_level7_instruction_decoder_8bit_ic_inst_1_op109),
    .op110(w_level7_instruction_decoder_8bit_ic_inst_1_op110),
    .op111(w_level7_instruction_decoder_8bit_ic_inst_1_op111),
    .op112(w_level7_instruction_decoder_8bit_ic_inst_1_op112),
    .op113(w_level7_instruction_decoder_8bit_ic_inst_1_op113),
    .op114(w_level7_instruction_decoder_8bit_ic_inst_1_op114),
    .op115(w_level7_instruction_decoder_8bit_ic_inst_1_op115),
    .op116(w_level7_instruction_decoder_8bit_ic_inst_1_op116),
    .op117(w_level7_instruction_decoder_8bit_ic_inst_1_op117),
    .op118(w_level7_instruction_decoder_8bit_ic_inst_1_op118),
    .op119(w_level7_instruction_decoder_8bit_ic_inst_1_op119),
    .op120(w_level7_instruction_decoder_8bit_ic_inst_1_op120),
    .op121(w_level7_instruction_decoder_8bit_ic_inst_1_op121),
    .op122(w_level7_instruction_decoder_8bit_ic_inst_1_op122),
    .op123(w_level7_instruction_decoder_8bit_ic_inst_1_op123),
    .op124(w_level7_instruction_decoder_8bit_ic_inst_1_op124),
    .op125(w_level7_instruction_decoder_8bit_ic_inst_1_op125),
    .op126(w_level7_instruction_decoder_8bit_ic_inst_1_op126),
    .op127(w_level7_instruction_decoder_8bit_ic_inst_1_op127),
    .op128(w_level7_instruction_decoder_8bit_ic_inst_1_op128),
    .op129(w_level7_instruction_decoder_8bit_ic_inst_1_op129),
    .op130(w_level7_instruction_decoder_8bit_ic_inst_1_op130),
    .op131(w_level7_instruction_decoder_8bit_ic_inst_1_op131),
    .op132(w_level7_instruction_decoder_8bit_ic_inst_1_op132),
    .op133(w_level7_instruction_decoder_8bit_ic_inst_1_op133),
    .op134(w_level7_instruction_decoder_8bit_ic_inst_1_op134),
    .op135(w_level7_instruction_decoder_8bit_ic_inst_1_op135),
    .op136(w_level7_instruction_decoder_8bit_ic_inst_1_op136),
    .op137(w_level7_instruction_decoder_8bit_ic_inst_1_op137),
    .op138(w_level7_instruction_decoder_8bit_ic_inst_1_op138),
    .op139(w_level7_instruction_decoder_8bit_ic_inst_1_op139),
    .op140(w_level7_instruction_decoder_8bit_ic_inst_1_op140),
    .op141(w_level7_instruction_decoder_8bit_ic_inst_1_op141),
    .op142(w_level7_instruction_decoder_8bit_ic_inst_1_op142),
    .op143(w_level7_instruction_decoder_8bit_ic_inst_1_op143),
    .op144(w_level7_instruction_decoder_8bit_ic_inst_1_op144),
    .op145(w_level7_instruction_decoder_8bit_ic_inst_1_op145),
    .op146(w_level7_instruction_decoder_8bit_ic_inst_1_op146),
    .op147(w_level7_instruction_decoder_8bit_ic_inst_1_op147),
    .op148(w_level7_instruction_decoder_8bit_ic_inst_1_op148),
    .op149(w_level7_instruction_decoder_8bit_ic_inst_1_op149),
    .op150(w_level7_instruction_decoder_8bit_ic_inst_1_op150),
    .op151(w_level7_instruction_decoder_8bit_ic_inst_1_op151),
    .op152(w_level7_instruction_decoder_8bit_ic_inst_1_op152),
    .op153(w_level7_instruction_decoder_8bit_ic_inst_1_op153),
    .op154(w_level7_instruction_decoder_8bit_ic_inst_1_op154),
    .op155(w_level7_instruction_decoder_8bit_ic_inst_1_op155),
    .op156(w_level7_instruction_decoder_8bit_ic_inst_1_op156),
    .op157(w_level7_instruction_decoder_8bit_ic_inst_1_op157),
    .op158(w_level7_instruction_decoder_8bit_ic_inst_1_op158),
    .op159(w_level7_instruction_decoder_8bit_ic_inst_1_op159),
    .op160(w_level7_instruction_decoder_8bit_ic_inst_1_op160),
    .op161(w_level7_instruction_decoder_8bit_ic_inst_1_op161),
    .op162(w_level7_instruction_decoder_8bit_ic_inst_1_op162),
    .op163(w_level7_instruction_decoder_8bit_ic_inst_1_op163),
    .op164(w_level7_instruction_decoder_8bit_ic_inst_1_op164),
    .op165(w_level7_instruction_decoder_8bit_ic_inst_1_op165),
    .op166(w_level7_instruction_decoder_8bit_ic_inst_1_op166),
    .op167(w_level7_instruction_decoder_8bit_ic_inst_1_op167),
    .op168(w_level7_instruction_decoder_8bit_ic_inst_1_op168),
    .op169(w_level7_instruction_decoder_8bit_ic_inst_1_op169),
    .op170(w_level7_instruction_decoder_8bit_ic_inst_1_op170),
    .op171(w_level7_instruction_decoder_8bit_ic_inst_1_op171),
    .op172(w_level7_instruction_decoder_8bit_ic_inst_1_op172),
    .op173(w_level7_instruction_decoder_8bit_ic_inst_1_op173),
    .op174(w_level7_instruction_decoder_8bit_ic_inst_1_op174),
    .op175(w_level7_instruction_decoder_8bit_ic_inst_1_op175),
    .op176(w_level7_instruction_decoder_8bit_ic_inst_1_op176),
    .op177(w_level7_instruction_decoder_8bit_ic_inst_1_op177),
    .op178(w_level7_instruction_decoder_8bit_ic_inst_1_op178),
    .op179(w_level7_instruction_decoder_8bit_ic_inst_1_op179),
    .op180(w_level7_instruction_decoder_8bit_ic_inst_1_op180),
    .op181(w_level7_instruction_decoder_8bit_ic_inst_1_op181),
    .op182(w_level7_instruction_decoder_8bit_ic_inst_1_op182),
    .op183(w_level7_instruction_decoder_8bit_ic_inst_1_op183),
    .op184(w_level7_instruction_decoder_8bit_ic_inst_1_op184),
    .op185(w_level7_instruction_decoder_8bit_ic_inst_1_op185),
    .op186(w_level7_instruction_decoder_8bit_ic_inst_1_op186),
    .op187(w_level7_instruction_decoder_8bit_ic_inst_1_op187),
    .op188(w_level7_instruction_decoder_8bit_ic_inst_1_op188),
    .op189(w_level7_instruction_decoder_8bit_ic_inst_1_op189),
    .op190(w_level7_instruction_decoder_8bit_ic_inst_1_op190),
    .op191(w_level7_instruction_decoder_8bit_ic_inst_1_op191),
    .op192(w_level7_instruction_decoder_8bit_ic_inst_1_op192),
    .op193(w_level7_instruction_decoder_8bit_ic_inst_1_op193),
    .op194(w_level7_instruction_decoder_8bit_ic_inst_1_op194),
    .op195(w_level7_instruction_decoder_8bit_ic_inst_1_op195),
    .op196(w_level7_instruction_decoder_8bit_ic_inst_1_op196),
    .op197(w_level7_instruction_decoder_8bit_ic_inst_1_op197),
    .op198(w_level7_instruction_decoder_8bit_ic_inst_1_op198),
    .op199(w_level7_instruction_decoder_8bit_ic_inst_1_op199),
    .op200(w_level7_instruction_decoder_8bit_ic_inst_1_op200),
    .op201(w_level7_instruction_decoder_8bit_ic_inst_1_op201),
    .op202(w_level7_instruction_decoder_8bit_ic_inst_1_op202),
    .op203(w_level7_instruction_decoder_8bit_ic_inst_1_op203),
    .op204(w_level7_instruction_decoder_8bit_ic_inst_1_op204),
    .op205(w_level7_instruction_decoder_8bit_ic_inst_1_op205),
    .op206(w_level7_instruction_decoder_8bit_ic_inst_1_op206),
    .op207(w_level7_instruction_decoder_8bit_ic_inst_1_op207),
    .op208(w_level7_instruction_decoder_8bit_ic_inst_1_op208),
    .op209(w_level7_instruction_decoder_8bit_ic_inst_1_op209),
    .op210(w_level7_instruction_decoder_8bit_ic_inst_1_op210),
    .op211(w_level7_instruction_decoder_8bit_ic_inst_1_op211),
    .op212(w_level7_instruction_decoder_8bit_ic_inst_1_op212),
    .op213(w_level7_instruction_decoder_8bit_ic_inst_1_op213),
    .op214(w_level7_instruction_decoder_8bit_ic_inst_1_op214),
    .op215(w_level7_instruction_decoder_8bit_ic_inst_1_op215),
    .op216(w_level7_instruction_decoder_8bit_ic_inst_1_op216),
    .op217(w_level7_instruction_decoder_8bit_ic_inst_1_op217),
    .op218(w_level7_instruction_decoder_8bit_ic_inst_1_op218),
    .op219(w_level7_instruction_decoder_8bit_ic_inst_1_op219),
    .op220(w_level7_instruction_decoder_8bit_ic_inst_1_op220),
    .op221(w_level7_instruction_decoder_8bit_ic_inst_1_op221),
    .op222(w_level7_instruction_decoder_8bit_ic_inst_1_op222),
    .op223(w_level7_instruction_decoder_8bit_ic_inst_1_op223),
    .op224(w_level7_instruction_decoder_8bit_ic_inst_1_op224),
    .op225(w_level7_instruction_decoder_8bit_ic_inst_1_op225),
    .op226(w_level7_instruction_decoder_8bit_ic_inst_1_op226),
    .op227(w_level7_instruction_decoder_8bit_ic_inst_1_op227),
    .op228(w_level7_instruction_decoder_8bit_ic_inst_1_op228),
    .op229(w_level7_instruction_decoder_8bit_ic_inst_1_op229),
    .op230(w_level7_instruction_decoder_8bit_ic_inst_1_op230),
    .op231(w_level7_instruction_decoder_8bit_ic_inst_1_op231),
    .op232(w_level7_instruction_decoder_8bit_ic_inst_1_op232),
    .op233(w_level7_instruction_decoder_8bit_ic_inst_1_op233),
    .op234(w_level7_instruction_decoder_8bit_ic_inst_1_op234),
    .op235(w_level7_instruction_decoder_8bit_ic_inst_1_op235),
    .op236(w_level7_instruction_decoder_8bit_ic_inst_1_op236),
    .op237(w_level7_instruction_decoder_8bit_ic_inst_1_op237),
    .op238(w_level7_instruction_decoder_8bit_ic_inst_1_op238),
    .op239(w_level7_instruction_decoder_8bit_ic_inst_1_op239),
    .op240(w_level7_instruction_decoder_8bit_ic_inst_1_op240),
    .op241(w_level7_instruction_decoder_8bit_ic_inst_1_op241),
    .op242(w_level7_instruction_decoder_8bit_ic_inst_1_op242),
    .op243(w_level7_instruction_decoder_8bit_ic_inst_1_op243),
    .op244(w_level7_instruction_decoder_8bit_ic_inst_1_op244),
    .op245(w_level7_instruction_decoder_8bit_ic_inst_1_op245),
    .op246(w_level7_instruction_decoder_8bit_ic_inst_1_op246),
    .op247(w_level7_instruction_decoder_8bit_ic_inst_1_op247),
    .op248(w_level7_instruction_decoder_8bit_ic_inst_1_op248),
    .op249(w_level7_instruction_decoder_8bit_ic_inst_1_op249),
    .op250(w_level7_instruction_decoder_8bit_ic_inst_1_op250),
    .op251(w_level7_instruction_decoder_8bit_ic_inst_1_op251),
    .op252(w_level7_instruction_decoder_8bit_ic_inst_1_op252),
    .op253(w_level7_instruction_decoder_8bit_ic_inst_1_op253),
    .op254(w_level7_instruction_decoder_8bit_ic_inst_1_op254),
    .op255(w_level7_instruction_decoder_8bit_ic_inst_1_op255)
);

// Writing output data. //
assign led10_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op0;
assign led11_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op1;
assign led12_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op2;
assign led13_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op3;
assign led14_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op4;
assign led15_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op5;
assign led16_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op6;
assign led17_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op7;
assign led18_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op8;
assign led19_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op9;
assign led20_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op10;
assign led21_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op11;
assign led22_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op12;
assign led23_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op13;
assign led24_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op14;
assign led25_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op15;
assign led26_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op16;
assign led27_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op17;
assign led28_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op18;
assign led29_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op19;
assign led30_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op20;
assign led31_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op21;
assign led32_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op22;
assign led33_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op23;
assign led34_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op24;
assign led35_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op25;
assign led36_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op26;
assign led37_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op27;
assign led38_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op28;
assign led39_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op29;
assign led40_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op30;
assign led41_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op31;
assign led42_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op32;
assign led43_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op33;
assign led44_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op34;
assign led45_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op35;
assign led46_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op36;
assign led47_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op37;
assign led48_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op38;
assign led49_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op39;
assign led50_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op40;
assign led51_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op41;
assign led52_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op42;
assign led53_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op43;
assign led54_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op44;
assign led55_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op45;
assign led56_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op46;
assign led57_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op47;
assign led58_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op48;
assign led59_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op49;
assign led60_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op50;
assign led61_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op51;
assign led62_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op52;
assign led63_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op53;
assign led64_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op54;
assign led65_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op55;
assign led66_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op56;
assign led67_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op57;
assign led68_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op58;
assign led69_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op59;
assign led70_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op60;
assign led71_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op61;
assign led72_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op62;
assign led73_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op63;
assign led74_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op64;
assign led75_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op65;
assign led76_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op66;
assign led77_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op67;
assign led78_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op68;
assign led79_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op69;
assign led80_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op70;
assign led81_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op71;
assign led82_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op72;
assign led83_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op73;
assign led84_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op74;
assign led85_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op75;
assign led86_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op76;
assign led87_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op77;
assign led88_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op78;
assign led89_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op79;
assign led90_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op80;
assign led91_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op81;
assign led92_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op82;
assign led93_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op83;
assign led94_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op84;
assign led95_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op85;
assign led96_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op86;
assign led97_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op87;
assign led98_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op88;
assign led99_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op89;
assign led100_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op90;
assign led101_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op91;
assign led102_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op92;
assign led103_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op93;
assign led104_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op94;
assign led105_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op95;
assign led106_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op96;
assign led107_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op97;
assign led108_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op98;
assign led109_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op99;
assign led110_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op100;
assign led111_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op101;
assign led112_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op102;
assign led113_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op103;
assign led114_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op104;
assign led115_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op105;
assign led116_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op106;
assign led117_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op107;
assign led118_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op108;
assign led119_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op109;
assign led120_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op110;
assign led121_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op111;
assign led122_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op112;
assign led123_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op113;
assign led124_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op114;
assign led125_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op115;
assign led126_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op116;
assign led127_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op117;
assign led128_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op118;
assign led129_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op119;
assign led130_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op120;
assign led131_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op121;
assign led132_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op122;
assign led133_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op123;
assign led134_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op124;
assign led135_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op125;
assign led136_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op126;
assign led137_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op127;
assign led138_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op128;
assign led139_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op129;
assign led140_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op130;
assign led141_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op131;
assign led142_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op132;
assign led143_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op133;
assign led144_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op134;
assign led145_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op135;
assign led146_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op136;
assign led147_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op137;
assign led148_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op138;
assign led149_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op139;
assign led150_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op140;
assign led151_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op141;
assign led152_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op142;
assign led153_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op143;
assign led154_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op144;
assign led155_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op145;
assign led156_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op146;
assign led157_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op147;
assign led158_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op148;
assign led159_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op149;
assign led160_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op150;
assign led161_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op151;
assign led162_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op152;
assign led163_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op153;
assign led164_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op154;
assign led165_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op155;
assign led166_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op156;
assign led167_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op157;
assign led168_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op158;
assign led169_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op159;
assign led170_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op160;
assign led171_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op161;
assign led172_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op162;
assign led173_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op163;
assign led174_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op164;
assign led175_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op165;
assign led176_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op166;
assign led177_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op167;
assign led178_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op168;
assign led179_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op169;
assign led180_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op170;
assign led181_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op171;
assign led182_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op172;
assign led183_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op173;
assign led184_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op174;
assign led185_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op175;
assign led186_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op176;
assign led187_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op177;
assign led188_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op178;
assign led189_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op179;
assign led190_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op180;
assign led191_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op181;
assign led192_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op182;
assign led193_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op183;
assign led194_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op184;
assign led195_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op185;
assign led196_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op186;
assign led197_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op187;
assign led198_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op188;
assign led199_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op189;
assign led200_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op190;
assign led201_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op191;
assign led202_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op192;
assign led203_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op193;
assign led204_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op194;
assign led205_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op195;
assign led206_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op196;
assign led207_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op197;
assign led208_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op198;
assign led209_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op199;
assign led210_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op200;
assign led211_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op201;
assign led212_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op202;
assign led213_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op203;
assign led214_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op204;
assign led215_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op205;
assign led216_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op206;
assign led217_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op207;
assign led218_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op208;
assign led219_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op209;
assign led220_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op210;
assign led221_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op211;
assign led222_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op212;
assign led223_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op213;
assign led224_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op214;
assign led225_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op215;
assign led226_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op216;
assign led227_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op217;
assign led228_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op218;
assign led229_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op219;
assign led230_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op220;
assign led231_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op221;
assign led232_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op222;
assign led233_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op223;
assign led234_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op224;
assign led235_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op225;
assign led236_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op226;
assign led237_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op227;
assign led238_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op228;
assign led239_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op229;
assign led240_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op230;
assign led241_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op231;
assign led242_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op232;
assign led243_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op233;
assign led244_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op234;
assign led245_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op235;
assign led246_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op236;
assign led247_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op237;
assign led248_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op238;
assign led249_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op239;
assign led250_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op240;
assign led251_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op241;
assign led252_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op242;
assign led253_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op243;
assign led254_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op244;
assign led255_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op245;
assign led256_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op246;
assign led257_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op247;
assign led258_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op248;
assign led259_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op249;
assign led260_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op250;
assign led261_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op251;
assign led262_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op252;
assign led263_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op253;
assign led264_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op254;
assign led265_1 = w_level7_instruction_decoder_8bit_ic_inst_1_op255;
endmodule
