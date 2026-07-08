// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_DECODER_5TO32 (generated from level2_decoder_5to32.panda)
module level2_decoder_5to32_ic (
    input addr0,
    input addr1,
    input addr2,
    input addr3,
    input addr4,
    input enable,
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
    output out15,
    output out16,
    output out17,
    output out18,
    output out19,
    output out20,
    output out21,
    output out22,
    output out23,
    output out24,
    output out25,
    output out26,
    output out27,
    output out28,
    output out29,
    output out30,
    output out31
);

wire aux_not_1;
wire aux_not_2;
wire aux_not_3;
wire aux_not_4;
wire aux_not_5;
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
assign aux_not_1 = ~addr0;
assign aux_not_2 = ~addr1;
assign aux_not_3 = ~addr2;
assign aux_not_4 = ~addr3;
assign aux_not_5 = ~addr4;
assign aux_and_6 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4 & aux_not_5 & enable);
assign aux_and_7 = (addr0 & aux_not_2 & aux_not_3 & aux_not_4 & aux_not_5 & enable);
assign aux_and_8 = (aux_not_1 & addr1 & aux_not_3 & aux_not_4 & aux_not_5 & enable);
assign aux_and_9 = (addr0 & addr1 & aux_not_3 & aux_not_4 & aux_not_5 & enable);
assign aux_and_10 = (aux_not_1 & aux_not_2 & addr2 & aux_not_4 & aux_not_5 & enable);
assign aux_and_11 = (addr0 & aux_not_2 & addr2 & aux_not_4 & aux_not_5 & enable);
assign aux_and_12 = (aux_not_1 & addr1 & addr2 & aux_not_4 & aux_not_5 & enable);
assign aux_and_13 = (addr0 & addr1 & addr2 & aux_not_4 & aux_not_5 & enable);
assign aux_and_14 = (aux_not_1 & aux_not_2 & aux_not_3 & addr3 & aux_not_5 & enable);
assign aux_and_15 = (addr0 & aux_not_2 & aux_not_3 & addr3 & aux_not_5 & enable);
assign aux_and_16 = (aux_not_1 & addr1 & aux_not_3 & addr3 & aux_not_5 & enable);
assign aux_and_17 = (addr0 & addr1 & aux_not_3 & addr3 & aux_not_5 & enable);
assign aux_and_18 = (aux_not_1 & aux_not_2 & addr2 & addr3 & aux_not_5 & enable);
assign aux_and_19 = (addr0 & aux_not_2 & addr2 & addr3 & aux_not_5 & enable);
assign aux_and_20 = (aux_not_1 & addr1 & addr2 & addr3 & aux_not_5 & enable);
assign aux_and_21 = (addr0 & addr1 & addr2 & addr3 & aux_not_5 & enable);
assign aux_and_22 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4 & addr4 & enable);
assign aux_and_23 = (addr0 & aux_not_2 & aux_not_3 & aux_not_4 & addr4 & enable);
assign aux_and_24 = (aux_not_1 & addr1 & aux_not_3 & aux_not_4 & addr4 & enable);
assign aux_and_25 = (addr0 & addr1 & aux_not_3 & aux_not_4 & addr4 & enable);
assign aux_and_26 = (aux_not_1 & aux_not_2 & addr2 & aux_not_4 & addr4 & enable);
assign aux_and_27 = (addr0 & aux_not_2 & addr2 & aux_not_4 & addr4 & enable);
assign aux_and_28 = (aux_not_1 & addr1 & addr2 & aux_not_4 & addr4 & enable);
assign aux_and_29 = (addr0 & addr1 & addr2 & aux_not_4 & addr4 & enable);
assign aux_and_30 = (aux_not_1 & aux_not_2 & aux_not_3 & addr3 & addr4 & enable);
assign aux_and_31 = (addr0 & aux_not_2 & aux_not_3 & addr3 & addr4 & enable);
assign aux_and_32 = (aux_not_1 & addr1 & aux_not_3 & addr3 & addr4 & enable);
assign aux_and_33 = (addr0 & addr1 & aux_not_3 & addr3 & addr4 & enable);
assign aux_and_34 = (aux_not_1 & aux_not_2 & addr2 & addr3 & addr4 & enable);
assign aux_and_35 = (addr0 & aux_not_2 & addr2 & addr3 & addr4 & enable);
assign aux_and_36 = (aux_not_1 & addr1 & addr2 & addr3 & addr4 & enable);
assign aux_and_37 = (addr0 & addr1 & addr2 & addr3 & addr4 & enable);

assign out0 = aux_and_6;
assign out1 = aux_and_7;
assign out2 = aux_and_8;
assign out3 = aux_and_9;
assign out4 = aux_and_10;
assign out5 = aux_and_11;
assign out6 = aux_and_12;
assign out7 = aux_and_13;
assign out8 = aux_and_14;
assign out9 = aux_and_15;
assign out10 = aux_and_16;
assign out11 = aux_and_17;
assign out12 = aux_and_18;
assign out13 = aux_and_19;
assign out14 = aux_and_20;
assign out15 = aux_and_21;
assign out16 = aux_and_22;
assign out17 = aux_and_23;
assign out18 = aux_and_24;
assign out19 = aux_and_25;
assign out20 = aux_and_26;
assign out21 = aux_and_27;
assign out22 = aux_and_28;
assign out23 = aux_and_29;
assign out24 = aux_and_30;
assign out25 = aux_and_31;
assign out26 = aux_and_32;
assign out27 = aux_and_33;
assign out28 = aux_and_34;
assign out29 = aux_and_35;
assign out30 = aux_and_36;
assign out31 = aux_and_37;
endmodule

module level2_decoder_5to32 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,

/* ========= Outputs ========== */
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
output led39_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_DECODER_5TO32 (level2_decoder_5to32_ic)
wire w_level2_decoder_5to32_ic_inst_1_out0;
wire w_level2_decoder_5to32_ic_inst_1_out1;
wire w_level2_decoder_5to32_ic_inst_1_out2;
wire w_level2_decoder_5to32_ic_inst_1_out3;
wire w_level2_decoder_5to32_ic_inst_1_out4;
wire w_level2_decoder_5to32_ic_inst_1_out5;
wire w_level2_decoder_5to32_ic_inst_1_out6;
wire w_level2_decoder_5to32_ic_inst_1_out7;
wire w_level2_decoder_5to32_ic_inst_1_out8;
wire w_level2_decoder_5to32_ic_inst_1_out9;
wire w_level2_decoder_5to32_ic_inst_1_out10;
wire w_level2_decoder_5to32_ic_inst_1_out11;
wire w_level2_decoder_5to32_ic_inst_1_out12;
wire w_level2_decoder_5to32_ic_inst_1_out13;
wire w_level2_decoder_5to32_ic_inst_1_out14;
wire w_level2_decoder_5to32_ic_inst_1_out15;
wire w_level2_decoder_5to32_ic_inst_1_out16;
wire w_level2_decoder_5to32_ic_inst_1_out17;
wire w_level2_decoder_5to32_ic_inst_1_out18;
wire w_level2_decoder_5to32_ic_inst_1_out19;
wire w_level2_decoder_5to32_ic_inst_1_out20;
wire w_level2_decoder_5to32_ic_inst_1_out21;
wire w_level2_decoder_5to32_ic_inst_1_out22;
wire w_level2_decoder_5to32_ic_inst_1_out23;
wire w_level2_decoder_5to32_ic_inst_1_out24;
wire w_level2_decoder_5to32_ic_inst_1_out25;
wire w_level2_decoder_5to32_ic_inst_1_out26;
wire w_level2_decoder_5to32_ic_inst_1_out27;
wire w_level2_decoder_5to32_ic_inst_1_out28;
wire w_level2_decoder_5to32_ic_inst_1_out29;
wire w_level2_decoder_5to32_ic_inst_1_out30;
wire w_level2_decoder_5to32_ic_inst_1_out31;


// Assigning aux variables. //
level2_decoder_5to32_ic level2_decoder_5to32_ic_inst_1 (
    .addr0(input_switch1),
    .addr1(input_switch2),
    .addr2(input_switch3),
    .addr3(input_switch4),
    .addr4(input_switch5),
    .enable(input_switch6),
    .out0(w_level2_decoder_5to32_ic_inst_1_out0),
    .out1(w_level2_decoder_5to32_ic_inst_1_out1),
    .out2(w_level2_decoder_5to32_ic_inst_1_out2),
    .out3(w_level2_decoder_5to32_ic_inst_1_out3),
    .out4(w_level2_decoder_5to32_ic_inst_1_out4),
    .out5(w_level2_decoder_5to32_ic_inst_1_out5),
    .out6(w_level2_decoder_5to32_ic_inst_1_out6),
    .out7(w_level2_decoder_5to32_ic_inst_1_out7),
    .out8(w_level2_decoder_5to32_ic_inst_1_out8),
    .out9(w_level2_decoder_5to32_ic_inst_1_out9),
    .out10(w_level2_decoder_5to32_ic_inst_1_out10),
    .out11(w_level2_decoder_5to32_ic_inst_1_out11),
    .out12(w_level2_decoder_5to32_ic_inst_1_out12),
    .out13(w_level2_decoder_5to32_ic_inst_1_out13),
    .out14(w_level2_decoder_5to32_ic_inst_1_out14),
    .out15(w_level2_decoder_5to32_ic_inst_1_out15),
    .out16(w_level2_decoder_5to32_ic_inst_1_out16),
    .out17(w_level2_decoder_5to32_ic_inst_1_out17),
    .out18(w_level2_decoder_5to32_ic_inst_1_out18),
    .out19(w_level2_decoder_5to32_ic_inst_1_out19),
    .out20(w_level2_decoder_5to32_ic_inst_1_out20),
    .out21(w_level2_decoder_5to32_ic_inst_1_out21),
    .out22(w_level2_decoder_5to32_ic_inst_1_out22),
    .out23(w_level2_decoder_5to32_ic_inst_1_out23),
    .out24(w_level2_decoder_5to32_ic_inst_1_out24),
    .out25(w_level2_decoder_5to32_ic_inst_1_out25),
    .out26(w_level2_decoder_5to32_ic_inst_1_out26),
    .out27(w_level2_decoder_5to32_ic_inst_1_out27),
    .out28(w_level2_decoder_5to32_ic_inst_1_out28),
    .out29(w_level2_decoder_5to32_ic_inst_1_out29),
    .out30(w_level2_decoder_5to32_ic_inst_1_out30),
    .out31(w_level2_decoder_5to32_ic_inst_1_out31)
);

// Writing output data. //
assign led8_1 = w_level2_decoder_5to32_ic_inst_1_out0;
assign led9_1 = w_level2_decoder_5to32_ic_inst_1_out1;
assign led10_1 = w_level2_decoder_5to32_ic_inst_1_out2;
assign led11_1 = w_level2_decoder_5to32_ic_inst_1_out3;
assign led12_1 = w_level2_decoder_5to32_ic_inst_1_out4;
assign led13_1 = w_level2_decoder_5to32_ic_inst_1_out5;
assign led14_1 = w_level2_decoder_5to32_ic_inst_1_out6;
assign led15_1 = w_level2_decoder_5to32_ic_inst_1_out7;
assign led16_1 = w_level2_decoder_5to32_ic_inst_1_out8;
assign led17_1 = w_level2_decoder_5to32_ic_inst_1_out9;
assign led18_1 = w_level2_decoder_5to32_ic_inst_1_out10;
assign led19_1 = w_level2_decoder_5to32_ic_inst_1_out11;
assign led20_1 = w_level2_decoder_5to32_ic_inst_1_out12;
assign led21_1 = w_level2_decoder_5to32_ic_inst_1_out13;
assign led22_1 = w_level2_decoder_5to32_ic_inst_1_out14;
assign led23_1 = w_level2_decoder_5to32_ic_inst_1_out15;
assign led24_1 = w_level2_decoder_5to32_ic_inst_1_out16;
assign led25_1 = w_level2_decoder_5to32_ic_inst_1_out17;
assign led26_1 = w_level2_decoder_5to32_ic_inst_1_out18;
assign led27_1 = w_level2_decoder_5to32_ic_inst_1_out19;
assign led28_1 = w_level2_decoder_5to32_ic_inst_1_out20;
assign led29_1 = w_level2_decoder_5to32_ic_inst_1_out21;
assign led30_1 = w_level2_decoder_5to32_ic_inst_1_out22;
assign led31_1 = w_level2_decoder_5to32_ic_inst_1_out23;
assign led32_1 = w_level2_decoder_5to32_ic_inst_1_out24;
assign led33_1 = w_level2_decoder_5to32_ic_inst_1_out25;
assign led34_1 = w_level2_decoder_5to32_ic_inst_1_out26;
assign led35_1 = w_level2_decoder_5to32_ic_inst_1_out27;
assign led36_1 = w_level2_decoder_5to32_ic_inst_1_out28;
assign led37_1 = w_level2_decoder_5to32_ic_inst_1_out29;
assign led38_1 = w_level2_decoder_5to32_ic_inst_1_out30;
assign led39_1 = w_level2_decoder_5to32_ic_inst_1_out31;
endmodule
