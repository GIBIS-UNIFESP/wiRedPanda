// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_DECODER_4TO16 (generated from level2_decoder_4to16.panda)
module level2_decoder_4to16_ic (
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

module level2_decoder_4to16 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,

/* ========= Outputs ========== */
output led6_1,
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
output led21_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_DECODER_4TO16 (level2_decoder_4to16_ic)
wire w_level2_decoder_4to16_ic_inst_1_out0;
wire w_level2_decoder_4to16_ic_inst_1_out1;
wire w_level2_decoder_4to16_ic_inst_1_out2;
wire w_level2_decoder_4to16_ic_inst_1_out3;
wire w_level2_decoder_4to16_ic_inst_1_out4;
wire w_level2_decoder_4to16_ic_inst_1_out5;
wire w_level2_decoder_4to16_ic_inst_1_out6;
wire w_level2_decoder_4to16_ic_inst_1_out7;
wire w_level2_decoder_4to16_ic_inst_1_out8;
wire w_level2_decoder_4to16_ic_inst_1_out9;
wire w_level2_decoder_4to16_ic_inst_1_out10;
wire w_level2_decoder_4to16_ic_inst_1_out11;
wire w_level2_decoder_4to16_ic_inst_1_out12;
wire w_level2_decoder_4to16_ic_inst_1_out13;
wire w_level2_decoder_4to16_ic_inst_1_out14;
wire w_level2_decoder_4to16_ic_inst_1_out15;


// Assigning aux variables. //
level2_decoder_4to16_ic level2_decoder_4to16_ic_inst_1 (
    .addr0(input_switch1),
    .addr1(input_switch2),
    .addr2(input_switch3),
    .addr3(input_switch4),
    .out0(w_level2_decoder_4to16_ic_inst_1_out0),
    .out1(w_level2_decoder_4to16_ic_inst_1_out1),
    .out2(w_level2_decoder_4to16_ic_inst_1_out2),
    .out3(w_level2_decoder_4to16_ic_inst_1_out3),
    .out4(w_level2_decoder_4to16_ic_inst_1_out4),
    .out5(w_level2_decoder_4to16_ic_inst_1_out5),
    .out6(w_level2_decoder_4to16_ic_inst_1_out6),
    .out7(w_level2_decoder_4to16_ic_inst_1_out7),
    .out8(w_level2_decoder_4to16_ic_inst_1_out8),
    .out9(w_level2_decoder_4to16_ic_inst_1_out9),
    .out10(w_level2_decoder_4to16_ic_inst_1_out10),
    .out11(w_level2_decoder_4to16_ic_inst_1_out11),
    .out12(w_level2_decoder_4to16_ic_inst_1_out12),
    .out13(w_level2_decoder_4to16_ic_inst_1_out13),
    .out14(w_level2_decoder_4to16_ic_inst_1_out14),
    .out15(w_level2_decoder_4to16_ic_inst_1_out15)
);

// Writing output data. //
assign led6_1 = w_level2_decoder_4to16_ic_inst_1_out0;
assign led7_1 = w_level2_decoder_4to16_ic_inst_1_out1;
assign led8_1 = w_level2_decoder_4to16_ic_inst_1_out2;
assign led9_1 = w_level2_decoder_4to16_ic_inst_1_out3;
assign led10_1 = w_level2_decoder_4to16_ic_inst_1_out4;
assign led11_1 = w_level2_decoder_4to16_ic_inst_1_out5;
assign led12_1 = w_level2_decoder_4to16_ic_inst_1_out6;
assign led13_1 = w_level2_decoder_4to16_ic_inst_1_out7;
assign led14_1 = w_level2_decoder_4to16_ic_inst_1_out8;
assign led15_1 = w_level2_decoder_4to16_ic_inst_1_out9;
assign led16_1 = w_level2_decoder_4to16_ic_inst_1_out10;
assign led17_1 = w_level2_decoder_4to16_ic_inst_1_out11;
assign led18_1 = w_level2_decoder_4to16_ic_inst_1_out12;
assign led19_1 = w_level2_decoder_4to16_ic_inst_1_out13;
assign led20_1 = w_level2_decoder_4to16_ic_inst_1_out14;
assign led21_1 = w_level2_decoder_4to16_ic_inst_1_out15;
endmodule
