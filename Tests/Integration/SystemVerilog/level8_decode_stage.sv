// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Decoder4to16 (generated from level2_decoder_4to16.panda)
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

// Module for LEVEL8_DECODE_STAGE (generated from level8_decode_stage.panda)
module level8_decode_stage_ic (
    input opcode0,
    input opcode1,
    input opcode2,
    input opcode3,
    input opcode4,
    output aluop0,
    output regwrite,
    output memread,
    output memwrite,
    output aluop1,
    output aluop2,
    output instrdecodedlines0,
    output instrdecodedlines1,
    output instrdecodedlines2,
    output instrdecodedlines3,
    output instrdecodedlines4,
    output instrdecodedlines5,
    output instrdecodedlines6,
    output instrdecodedlines7,
    output instrdecodedlines8,
    output instrdecodedlines9,
    output instrdecodedlines10,
    output instrdecodedlines11,
    output instrdecodedlines12,
    output instrdecodedlines13,
    output instrdecodedlines14,
    output instrdecodedlines15,
    output instrdecodedlines16,
    output instrdecodedlines17,
    output instrdecodedlines18,
    output instrdecodedlines19,
    output instrdecodedlines20,
    output instrdecodedlines21,
    output instrdecodedlines22,
    output instrdecodedlines23,
    output instrdecodedlines24,
    output instrdecodedlines25,
    output instrdecodedlines26,
    output instrdecodedlines27,
    output instrdecodedlines28,
    output instrdecodedlines29,
    output instrdecodedlines30,
    output instrdecodedlines31
);

wire aux_not_1;
wire aux_not_2;
wire aux_and_3;
wire aux_and_4;
// IC instance: Decoder4to16 (level2_decoder_4to16)
wire w_level2_decoder_4to16_inst_5_out0;
wire w_level2_decoder_4to16_inst_5_out1;
wire w_level2_decoder_4to16_inst_5_out2;
wire w_level2_decoder_4to16_inst_5_out3;
wire w_level2_decoder_4to16_inst_5_out4;
wire w_level2_decoder_4to16_inst_5_out5;
wire w_level2_decoder_4to16_inst_5_out6;
wire w_level2_decoder_4to16_inst_5_out7;
wire w_level2_decoder_4to16_inst_5_out8;
wire w_level2_decoder_4to16_inst_5_out9;
wire w_level2_decoder_4to16_inst_5_out10;
wire w_level2_decoder_4to16_inst_5_out11;
wire w_level2_decoder_4to16_inst_5_out12;
wire w_level2_decoder_4to16_inst_5_out13;
wire w_level2_decoder_4to16_inst_5_out14;
wire w_level2_decoder_4to16_inst_5_out15;
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

// Internal logic
assign aux_not_1 = ~opcode4;
assign aux_not_2 = ~opcode3;
assign aux_and_3 = (opcode4 & aux_not_2);
assign aux_and_4 = (opcode4 & opcode3);
level2_decoder_4to16 level2_decoder_4to16_inst_5 (
    .addr0(opcode0),
    .addr1(opcode1),
    .addr2(opcode2),
    .addr3(opcode3),
    .out0(w_level2_decoder_4to16_inst_5_out0),
    .out1(w_level2_decoder_4to16_inst_5_out1),
    .out2(w_level2_decoder_4to16_inst_5_out2),
    .out3(w_level2_decoder_4to16_inst_5_out3),
    .out4(w_level2_decoder_4to16_inst_5_out4),
    .out5(w_level2_decoder_4to16_inst_5_out5),
    .out6(w_level2_decoder_4to16_inst_5_out6),
    .out7(w_level2_decoder_4to16_inst_5_out7),
    .out8(w_level2_decoder_4to16_inst_5_out8),
    .out9(w_level2_decoder_4to16_inst_5_out9),
    .out10(w_level2_decoder_4to16_inst_5_out10),
    .out11(w_level2_decoder_4to16_inst_5_out11),
    .out12(w_level2_decoder_4to16_inst_5_out12),
    .out13(w_level2_decoder_4to16_inst_5_out13),
    .out14(w_level2_decoder_4to16_inst_5_out14),
    .out15(w_level2_decoder_4to16_inst_5_out15)
);
assign aux_and_6 = (w_level2_decoder_4to16_inst_5_out0 & aux_not_1);
assign aux_and_7 = (w_level2_decoder_4to16_inst_5_out1 & aux_not_1);
assign aux_and_8 = (w_level2_decoder_4to16_inst_5_out2 & aux_not_1);
assign aux_and_9 = (w_level2_decoder_4to16_inst_5_out3 & aux_not_1);
assign aux_and_10 = (w_level2_decoder_4to16_inst_5_out4 & aux_not_1);
assign aux_and_11 = (w_level2_decoder_4to16_inst_5_out5 & aux_not_1);
assign aux_and_12 = (w_level2_decoder_4to16_inst_5_out6 & aux_not_1);
assign aux_and_13 = (w_level2_decoder_4to16_inst_5_out7 & aux_not_1);
assign aux_and_14 = (w_level2_decoder_4to16_inst_5_out8 & aux_not_1);
assign aux_and_15 = (w_level2_decoder_4to16_inst_5_out9 & aux_not_1);
assign aux_and_16 = (w_level2_decoder_4to16_inst_5_out10 & aux_not_1);
assign aux_and_17 = (w_level2_decoder_4to16_inst_5_out11 & aux_not_1);
assign aux_and_18 = (w_level2_decoder_4to16_inst_5_out12 & aux_not_1);
assign aux_and_19 = (w_level2_decoder_4to16_inst_5_out13 & aux_not_1);
assign aux_and_20 = (w_level2_decoder_4to16_inst_5_out14 & aux_not_1);
assign aux_and_21 = (w_level2_decoder_4to16_inst_5_out15 & aux_not_1);
assign aux_and_22 = (w_level2_decoder_4to16_inst_5_out0 & opcode4);
assign aux_and_23 = (w_level2_decoder_4to16_inst_5_out1 & opcode4);
assign aux_and_24 = (w_level2_decoder_4to16_inst_5_out2 & opcode4);
assign aux_and_25 = (w_level2_decoder_4to16_inst_5_out3 & opcode4);
assign aux_and_26 = (w_level2_decoder_4to16_inst_5_out4 & opcode4);
assign aux_and_27 = (w_level2_decoder_4to16_inst_5_out5 & opcode4);
assign aux_and_28 = (w_level2_decoder_4to16_inst_5_out6 & opcode4);
assign aux_and_29 = (w_level2_decoder_4to16_inst_5_out7 & opcode4);
assign aux_and_30 = (w_level2_decoder_4to16_inst_5_out8 & opcode4);
assign aux_and_31 = (w_level2_decoder_4to16_inst_5_out9 & opcode4);
assign aux_and_32 = (w_level2_decoder_4to16_inst_5_out10 & opcode4);
assign aux_and_33 = (w_level2_decoder_4to16_inst_5_out11 & opcode4);
assign aux_and_34 = (w_level2_decoder_4to16_inst_5_out12 & opcode4);
assign aux_and_35 = (w_level2_decoder_4to16_inst_5_out13 & opcode4);
assign aux_and_36 = (w_level2_decoder_4to16_inst_5_out14 & opcode4);
assign aux_and_37 = (w_level2_decoder_4to16_inst_5_out15 & opcode4);

assign aluop0 = opcode0;
assign regwrite = aux_not_1;
assign memread = aux_and_3;
assign memwrite = aux_and_4;
assign aluop1 = opcode1;
assign aluop2 = opcode2;
assign instrdecodedlines0 = aux_and_6;
assign instrdecodedlines1 = aux_and_7;
assign instrdecodedlines2 = aux_and_8;
assign instrdecodedlines3 = aux_and_9;
assign instrdecodedlines4 = aux_and_10;
assign instrdecodedlines5 = aux_and_11;
assign instrdecodedlines6 = aux_and_12;
assign instrdecodedlines7 = aux_and_13;
assign instrdecodedlines8 = aux_and_14;
assign instrdecodedlines9 = aux_and_15;
assign instrdecodedlines10 = aux_and_16;
assign instrdecodedlines11 = aux_and_17;
assign instrdecodedlines12 = aux_and_18;
assign instrdecodedlines13 = aux_and_19;
assign instrdecodedlines14 = aux_and_20;
assign instrdecodedlines15 = aux_and_21;
assign instrdecodedlines16 = aux_and_22;
assign instrdecodedlines17 = aux_and_23;
assign instrdecodedlines18 = aux_and_24;
assign instrdecodedlines19 = aux_and_25;
assign instrdecodedlines20 = aux_and_26;
assign instrdecodedlines21 = aux_and_27;
assign instrdecodedlines22 = aux_and_28;
assign instrdecodedlines23 = aux_and_29;
assign instrdecodedlines24 = aux_and_30;
assign instrdecodedlines25 = aux_and_31;
assign instrdecodedlines26 = aux_and_32;
assign instrdecodedlines27 = aux_and_33;
assign instrdecodedlines28 = aux_and_34;
assign instrdecodedlines29 = aux_and_35;
assign instrdecodedlines30 = aux_and_36;
assign instrdecodedlines31 = aux_and_37;
endmodule

module level8_decode_stage (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,

/* ========= Outputs ========== */
output led7_1,
output led8_1,
output led9_1,
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
output led44_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL8_DECODE_STAGE (level8_decode_stage_ic)
wire w_level8_decode_stage_ic_inst_1_aluop0;
wire w_level8_decode_stage_ic_inst_1_regwrite;
wire w_level8_decode_stage_ic_inst_1_memread;
wire w_level8_decode_stage_ic_inst_1_memwrite;
wire w_level8_decode_stage_ic_inst_1_aluop1;
wire w_level8_decode_stage_ic_inst_1_aluop2;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines0;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines1;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines2;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines3;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines4;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines5;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines6;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines7;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines8;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines9;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines10;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines11;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines12;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines13;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines14;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines15;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines16;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines17;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines18;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines19;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines20;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines21;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines22;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines23;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines24;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines25;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines26;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines27;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines28;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines29;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines30;
wire w_level8_decode_stage_ic_inst_1_instrdecodedlines31;


// Assigning aux variables. //
level8_decode_stage_ic level8_decode_stage_ic_inst_1 (
    .opcode0(input_switch1),
    .opcode1(input_switch2),
    .opcode2(input_switch3),
    .opcode3(input_switch4),
    .opcode4(input_switch5),
    .aluop0(w_level8_decode_stage_ic_inst_1_aluop0),
    .regwrite(w_level8_decode_stage_ic_inst_1_regwrite),
    .memread(w_level8_decode_stage_ic_inst_1_memread),
    .memwrite(w_level8_decode_stage_ic_inst_1_memwrite),
    .aluop1(w_level8_decode_stage_ic_inst_1_aluop1),
    .aluop2(w_level8_decode_stage_ic_inst_1_aluop2),
    .instrdecodedlines0(w_level8_decode_stage_ic_inst_1_instrdecodedlines0),
    .instrdecodedlines1(w_level8_decode_stage_ic_inst_1_instrdecodedlines1),
    .instrdecodedlines2(w_level8_decode_stage_ic_inst_1_instrdecodedlines2),
    .instrdecodedlines3(w_level8_decode_stage_ic_inst_1_instrdecodedlines3),
    .instrdecodedlines4(w_level8_decode_stage_ic_inst_1_instrdecodedlines4),
    .instrdecodedlines5(w_level8_decode_stage_ic_inst_1_instrdecodedlines5),
    .instrdecodedlines6(w_level8_decode_stage_ic_inst_1_instrdecodedlines6),
    .instrdecodedlines7(w_level8_decode_stage_ic_inst_1_instrdecodedlines7),
    .instrdecodedlines8(w_level8_decode_stage_ic_inst_1_instrdecodedlines8),
    .instrdecodedlines9(w_level8_decode_stage_ic_inst_1_instrdecodedlines9),
    .instrdecodedlines10(w_level8_decode_stage_ic_inst_1_instrdecodedlines10),
    .instrdecodedlines11(w_level8_decode_stage_ic_inst_1_instrdecodedlines11),
    .instrdecodedlines12(w_level8_decode_stage_ic_inst_1_instrdecodedlines12),
    .instrdecodedlines13(w_level8_decode_stage_ic_inst_1_instrdecodedlines13),
    .instrdecodedlines14(w_level8_decode_stage_ic_inst_1_instrdecodedlines14),
    .instrdecodedlines15(w_level8_decode_stage_ic_inst_1_instrdecodedlines15),
    .instrdecodedlines16(w_level8_decode_stage_ic_inst_1_instrdecodedlines16),
    .instrdecodedlines17(w_level8_decode_stage_ic_inst_1_instrdecodedlines17),
    .instrdecodedlines18(w_level8_decode_stage_ic_inst_1_instrdecodedlines18),
    .instrdecodedlines19(w_level8_decode_stage_ic_inst_1_instrdecodedlines19),
    .instrdecodedlines20(w_level8_decode_stage_ic_inst_1_instrdecodedlines20),
    .instrdecodedlines21(w_level8_decode_stage_ic_inst_1_instrdecodedlines21),
    .instrdecodedlines22(w_level8_decode_stage_ic_inst_1_instrdecodedlines22),
    .instrdecodedlines23(w_level8_decode_stage_ic_inst_1_instrdecodedlines23),
    .instrdecodedlines24(w_level8_decode_stage_ic_inst_1_instrdecodedlines24),
    .instrdecodedlines25(w_level8_decode_stage_ic_inst_1_instrdecodedlines25),
    .instrdecodedlines26(w_level8_decode_stage_ic_inst_1_instrdecodedlines26),
    .instrdecodedlines27(w_level8_decode_stage_ic_inst_1_instrdecodedlines27),
    .instrdecodedlines28(w_level8_decode_stage_ic_inst_1_instrdecodedlines28),
    .instrdecodedlines29(w_level8_decode_stage_ic_inst_1_instrdecodedlines29),
    .instrdecodedlines30(w_level8_decode_stage_ic_inst_1_instrdecodedlines30),
    .instrdecodedlines31(w_level8_decode_stage_ic_inst_1_instrdecodedlines31)
);

// Writing output data. //
assign led7_1 = w_level8_decode_stage_ic_inst_1_aluop0;
assign led8_1 = w_level8_decode_stage_ic_inst_1_regwrite;
assign led9_1 = w_level8_decode_stage_ic_inst_1_memread;
assign led10_1 = w_level8_decode_stage_ic_inst_1_memwrite;
assign led11_1 = w_level8_decode_stage_ic_inst_1_aluop1;
assign led12_1 = w_level8_decode_stage_ic_inst_1_aluop2;
assign led13_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines0;
assign led14_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines1;
assign led15_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines2;
assign led16_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines3;
assign led17_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines4;
assign led18_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines5;
assign led19_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines6;
assign led20_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines7;
assign led21_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines8;
assign led22_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines9;
assign led23_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines10;
assign led24_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines11;
assign led25_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines12;
assign led26_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines13;
assign led27_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines14;
assign led28_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines15;
assign led29_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines16;
assign led30_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines17;
assign led31_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines18;
assign led32_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines19;
assign led33_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines20;
assign led34_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines21;
assign led35_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines22;
assign led36_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines23;
assign led37_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines24;
assign led38_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines25;
assign led39_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines26;
assign led40_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines27;
assign led41_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines28;
assign led42_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines29;
assign led43_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines30;
assign led44_1 = w_level8_decode_stage_ic_inst_1_instrdecodedlines31;
endmodule
