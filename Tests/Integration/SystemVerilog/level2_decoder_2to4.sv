// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_DECODER_2TO4 (generated from level2_decoder_2to4.panda)
module level2_decoder_2to4_ic (
    input addr0,
    input addr1,
    output out0,
    output out1,
    output out2,
    output out3
);

wire aux_not_1;
wire aux_not_2;
wire aux_and_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;

// Internal logic
assign aux_not_1 = ~addr0;
assign aux_not_2 = ~addr1;
assign aux_and_3 = (aux_not_1 & aux_not_2);
assign aux_and_4 = (addr0 & aux_not_2);
assign aux_and_5 = (aux_not_1 & addr1);
assign aux_and_6 = (addr0 & addr1);

assign out0 = aux_and_3;
assign out1 = aux_and_4;
assign out2 = aux_and_5;
assign out3 = aux_and_6;
endmodule

module level2_decoder_2to4 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,

/* ========= Outputs ========== */
output led4_1,
output led5_1,
output led6_1,
output led7_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_DECODER_2TO4 (level2_decoder_2to4_ic)
wire w_level2_decoder_2to4_ic_inst_1_out0;
wire w_level2_decoder_2to4_ic_inst_1_out1;
wire w_level2_decoder_2to4_ic_inst_1_out2;
wire w_level2_decoder_2to4_ic_inst_1_out3;


// Assigning aux variables. //
level2_decoder_2to4_ic level2_decoder_2to4_ic_inst_1 (
    .addr0(input_switch1),
    .addr1(input_switch2),
    .out0(w_level2_decoder_2to4_ic_inst_1_out0),
    .out1(w_level2_decoder_2to4_ic_inst_1_out1),
    .out2(w_level2_decoder_2to4_ic_inst_1_out2),
    .out3(w_level2_decoder_2to4_ic_inst_1_out3)
);

// Writing output data. //
assign led4_1 = w_level2_decoder_2to4_ic_inst_1_out0;
assign led5_1 = w_level2_decoder_2to4_ic_inst_1_out1;
assign led6_1 = w_level2_decoder_2to4_ic_inst_1_out2;
assign led7_1 = w_level2_decoder_2to4_ic_inst_1_out3;
endmodule
