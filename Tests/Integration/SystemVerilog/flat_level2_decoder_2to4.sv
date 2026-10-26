// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_decoder_2to4 (
/* ========= Inputs ========== */
input input_switch1_addr0,
input input_switch2_addr1,
input input_switch3_enable,

/* ========= Outputs ========== */
output led10_out0_1,
output led11_out1_1,
output led12_out2_1,
output led13_out3_1
);
/* ====== Aux. Variables ====== */
wire aux_not_1;
wire aux_not_2;
wire aux_and_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;


// Assigning aux variables. //
assign aux_not_1 = ~input_switch1_addr0;
assign aux_not_2 = ~input_switch2_addr1;
assign aux_and_3 = (aux_not_1 & aux_not_2 & input_switch3_enable);
assign aux_and_4 = (input_switch1_addr0 & aux_not_2 & input_switch3_enable);
assign aux_and_5 = (aux_not_1 & input_switch2_addr1 & input_switch3_enable);
assign aux_and_6 = (input_switch1_addr0 & input_switch2_addr1 & input_switch3_enable);

// Writing output data. //
assign led10_out0_1 = aux_and_3;
assign led11_out1_1 = aux_and_4;
assign led12_out2_1 = aux_and_5;
assign led13_out3_1 = aux_and_6;
endmodule
