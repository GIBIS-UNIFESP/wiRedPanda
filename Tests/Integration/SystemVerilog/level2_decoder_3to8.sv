// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_DECODER_3TO8 (generated from level2_decoder_3to8.panda)
module level2_decoder_3to8_ic (
    input addr0,
    input addr1,
    input addr2,
    output out0,
    output out1,
    output out2,
    output out3,
    output out4,
    output out5,
    output out6,
    output out7
);

wire aux_not_1;
wire aux_not_2;
wire aux_not_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;
wire aux_and_9;
wire aux_and_10;
wire aux_and_11;

// Internal logic
assign aux_not_1 = ~addr0;
assign aux_not_2 = ~addr1;
assign aux_not_3 = ~addr2;
assign aux_and_4 = (aux_not_1 & aux_not_2 & aux_not_3);
assign aux_and_5 = (addr0 & aux_not_2 & aux_not_3);
assign aux_and_6 = (aux_not_1 & addr1 & aux_not_3);
assign aux_and_7 = (addr0 & addr1 & aux_not_3);
assign aux_and_8 = (aux_not_1 & aux_not_2 & addr2);
assign aux_and_9 = (addr0 & aux_not_2 & addr2);
assign aux_and_10 = (aux_not_1 & addr1 & addr2);
assign aux_and_11 = (addr0 & addr1 & addr2);

assign out0 = aux_and_4;
assign out1 = aux_and_5;
assign out2 = aux_and_6;
assign out3 = aux_and_7;
assign out4 = aux_and_8;
assign out5 = aux_and_9;
assign out6 = aux_and_10;
assign out7 = aux_and_11;
endmodule

module level2_decoder_3to8 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,

/* ========= Outputs ========== */
output led5_1,
output led6_1,
output led7_1,
output led8_1,
output led9_1,
output led10_1,
output led11_1,
output led12_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_DECODER_3TO8 (level2_decoder_3to8_ic)
wire w_level2_decoder_3to8_ic_inst_1_out0;
wire w_level2_decoder_3to8_ic_inst_1_out1;
wire w_level2_decoder_3to8_ic_inst_1_out2;
wire w_level2_decoder_3to8_ic_inst_1_out3;
wire w_level2_decoder_3to8_ic_inst_1_out4;
wire w_level2_decoder_3to8_ic_inst_1_out5;
wire w_level2_decoder_3to8_ic_inst_1_out6;
wire w_level2_decoder_3to8_ic_inst_1_out7;


// Assigning aux variables. //
level2_decoder_3to8_ic level2_decoder_3to8_ic_inst_1 (
    .addr0(input_switch1),
    .addr1(input_switch2),
    .addr2(input_switch3),
    .out0(w_level2_decoder_3to8_ic_inst_1_out0),
    .out1(w_level2_decoder_3to8_ic_inst_1_out1),
    .out2(w_level2_decoder_3to8_ic_inst_1_out2),
    .out3(w_level2_decoder_3to8_ic_inst_1_out3),
    .out4(w_level2_decoder_3to8_ic_inst_1_out4),
    .out5(w_level2_decoder_3to8_ic_inst_1_out5),
    .out6(w_level2_decoder_3to8_ic_inst_1_out6),
    .out7(w_level2_decoder_3to8_ic_inst_1_out7)
);

// Writing output data. //
assign led5_1 = w_level2_decoder_3to8_ic_inst_1_out0;
assign led6_1 = w_level2_decoder_3to8_ic_inst_1_out1;
assign led7_1 = w_level2_decoder_3to8_ic_inst_1_out2;
assign led8_1 = w_level2_decoder_3to8_ic_inst_1_out3;
assign led9_1 = w_level2_decoder_3to8_ic_inst_1_out4;
assign led10_1 = w_level2_decoder_3to8_ic_inst_1_out5;
assign led11_1 = w_level2_decoder_3to8_ic_inst_1_out6;
assign led12_1 = w_level2_decoder_3to8_ic_inst_1_out7;
endmodule
