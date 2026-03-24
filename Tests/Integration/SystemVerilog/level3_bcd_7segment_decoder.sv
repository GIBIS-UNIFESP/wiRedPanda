// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL3_BCD_7SEGMENT_DECODER (generated from level3_bcd_7segment_decoder.panda)
module level3_bcd_7segment_decoder_ic (
    input bcd0,
    input bcd1,
    input bcd2,
    input bcd3,
    output segment_a,
    output segment_b,
    output segment_c,
    output segment_d,
    output segment_e,
    output segment_f,
    output segment_g
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
wire aux_or_15;
wire aux_or_16;
wire aux_or_17;
wire aux_or_18;
wire aux_or_19;
wire aux_or_20;
wire aux_or_21;
wire aux_or_22;

// Internal logic
assign aux_not_1 = ~bcd0;
assign aux_not_2 = ~bcd1;
assign aux_not_3 = ~bcd2;
assign aux_not_4 = ~bcd3;
assign aux_and_5 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4);
assign aux_and_6 = (bcd0 & aux_not_2 & aux_not_3 & aux_not_4);
assign aux_and_7 = (aux_not_1 & bcd1 & aux_not_3 & aux_not_4);
assign aux_and_8 = (bcd0 & bcd1 & aux_not_3 & aux_not_4);
assign aux_and_9 = (aux_not_1 & aux_not_2 & bcd2 & aux_not_4);
assign aux_and_10 = (bcd0 & aux_not_2 & bcd2 & aux_not_4);
assign aux_and_11 = (aux_not_1 & bcd1 & bcd2 & aux_not_4);
assign aux_and_12 = (bcd0 & bcd1 & bcd2 & aux_not_4);
assign aux_and_13 = (aux_not_1 & aux_not_2 & aux_not_3 & bcd3);
assign aux_and_14 = (bcd0 & aux_not_2 & aux_not_3 & bcd3);
assign aux_or_15 = (aux_and_5 | aux_and_7 | aux_and_8 | aux_and_10 | aux_and_11 | aux_and_12 | aux_and_13 | aux_and_14);
assign aux_or_16 = (aux_and_5 | aux_and_6 | aux_and_7 | aux_and_8 | aux_and_9 | aux_and_12 | aux_and_13 | aux_and_14);
assign aux_or_17 = (aux_and_5 | aux_and_6 | aux_and_8 | aux_and_9 | aux_and_10 | aux_and_11 | aux_and_12 | aux_and_13);
assign aux_or_18 = (aux_and_14 | aux_or_17);
assign aux_or_19 = (aux_and_5 | aux_and_7 | aux_and_8 | aux_and_10 | aux_and_11 | aux_and_13 | aux_and_14);
assign aux_or_20 = (aux_and_5 | aux_and_7 | aux_and_11 | aux_and_13);
assign aux_or_21 = (aux_and_5 | aux_and_9 | aux_and_10 | aux_and_11 | aux_and_13 | aux_and_14);
assign aux_or_22 = (aux_and_7 | aux_and_8 | aux_and_9 | aux_and_10 | aux_and_11 | aux_and_13 | aux_and_14);

assign segment_a = aux_or_15;
assign segment_b = aux_or_16;
assign segment_c = aux_or_18;
assign segment_d = aux_or_19;
assign segment_e = aux_or_20;
assign segment_f = aux_or_21;
assign segment_g = aux_or_22;
endmodule

module level3_bcd_7segment_decoder (
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
output led12_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL3_BCD_7SEGMENT_DECODER (level3_bcd_7segment_decoder_ic)
wire w_level3_bcd_7segment_decoder_ic_inst_1_segment_a;
wire w_level3_bcd_7segment_decoder_ic_inst_1_segment_b;
wire w_level3_bcd_7segment_decoder_ic_inst_1_segment_c;
wire w_level3_bcd_7segment_decoder_ic_inst_1_segment_d;
wire w_level3_bcd_7segment_decoder_ic_inst_1_segment_e;
wire w_level3_bcd_7segment_decoder_ic_inst_1_segment_f;
wire w_level3_bcd_7segment_decoder_ic_inst_1_segment_g;


// Assigning aux variables. //
level3_bcd_7segment_decoder_ic level3_bcd_7segment_decoder_ic_inst_1 (
    .bcd0(input_switch1),
    .bcd1(input_switch2),
    .bcd2(input_switch3),
    .bcd3(input_switch4),
    .segment_a(w_level3_bcd_7segment_decoder_ic_inst_1_segment_a),
    .segment_b(w_level3_bcd_7segment_decoder_ic_inst_1_segment_b),
    .segment_c(w_level3_bcd_7segment_decoder_ic_inst_1_segment_c),
    .segment_d(w_level3_bcd_7segment_decoder_ic_inst_1_segment_d),
    .segment_e(w_level3_bcd_7segment_decoder_ic_inst_1_segment_e),
    .segment_f(w_level3_bcd_7segment_decoder_ic_inst_1_segment_f),
    .segment_g(w_level3_bcd_7segment_decoder_ic_inst_1_segment_g)
);

// Writing output data. //
assign led6_1 = w_level3_bcd_7segment_decoder_ic_inst_1_segment_a;
assign led7_1 = w_level3_bcd_7segment_decoder_ic_inst_1_segment_b;
assign led8_1 = w_level3_bcd_7segment_decoder_ic_inst_1_segment_c;
assign led9_1 = w_level3_bcd_7segment_decoder_ic_inst_1_segment_d;
assign led10_1 = w_level3_bcd_7segment_decoder_ic_inst_1_segment_e;
assign led11_1 = w_level3_bcd_7segment_decoder_ic_inst_1_segment_f;
assign led12_1 = w_level3_bcd_7segment_decoder_ic_inst_1_segment_g;
endmodule
