// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_decoder_3to8 (
/* ========= Inputs ========== */
input input_switch1_addr0,
input input_switch2_addr1,
input input_switch3_addr2,
input input_switch4_enable,

/* ========= Outputs ========== */
output led16_out0_1,
output led17_out1_1,
output led18_out2_1,
output led19_out3_1,
output led20_out4_1,
output led21_out5_1,
output led22_out6_1,
output led23_out7_1
);
/* ====== Aux. Variables ====== */
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


// Assigning aux variables. //
assign aux_not_1 = ~input_switch1_addr0;
assign aux_not_2 = ~input_switch2_addr1;
assign aux_not_3 = ~input_switch3_addr2;
assign aux_and_4 = (aux_not_1 & aux_not_2 & aux_not_3 & input_switch4_enable);
assign aux_and_5 = (input_switch1_addr0 & aux_not_2 & aux_not_3 & input_switch4_enable);
assign aux_and_6 = (aux_not_1 & input_switch2_addr1 & aux_not_3 & input_switch4_enable);
assign aux_and_7 = (input_switch1_addr0 & input_switch2_addr1 & aux_not_3 & input_switch4_enable);
assign aux_and_8 = (aux_not_1 & aux_not_2 & input_switch3_addr2 & input_switch4_enable);
assign aux_and_9 = (input_switch1_addr0 & aux_not_2 & input_switch3_addr2 & input_switch4_enable);
assign aux_and_10 = (aux_not_1 & input_switch2_addr1 & input_switch3_addr2 & input_switch4_enable);
assign aux_and_11 = (input_switch1_addr0 & input_switch2_addr1 & input_switch3_addr2 & input_switch4_enable);

// Writing output data. //
assign led16_out0_1 = aux_and_4;
assign led17_out1_1 = aux_and_5;
assign led18_out2_1 = aux_and_6;
assign led19_out3_1 = aux_and_7;
assign led20_out4_1 = aux_and_8;
assign led21_out5_1 = aux_and_9;
assign led22_out6_1 = aux_and_10;
assign led23_out7_1 = aux_and_11;
endmodule
