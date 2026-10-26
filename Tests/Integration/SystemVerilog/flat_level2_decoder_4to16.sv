// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_decoder_4to16 (
/* ========= Inputs ========== */
input input_switch1_addr0,
input input_switch2_addr1,
input input_switch3_addr2,
input input_switch4_addr3,
input input_switch5_enable,

/* ========= Outputs ========== */
output led26_out0_1,
output led27_out1_1,
output led28_out2_1,
output led29_out3_1,
output led30_out4_1,
output led31_out5_1,
output led32_out6_1,
output led33_out7_1,
output led34_out8_1,
output led35_out9_1,
output led36_out10_1,
output led37_out11_1,
output led38_out12_1,
output led39_out13_1,
output led40_out14_1,
output led41_out15_1
);
/* ====== Aux. Variables ====== */
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


// Assigning aux variables. //
assign aux_not_1 = ~input_switch1_addr0;
assign aux_not_2 = ~input_switch2_addr1;
assign aux_not_3 = ~input_switch3_addr2;
assign aux_not_4 = ~input_switch4_addr3;
assign aux_and_5 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4 & input_switch5_enable);
assign aux_and_6 = (input_switch1_addr0 & aux_not_2 & aux_not_3 & aux_not_4 & input_switch5_enable);
assign aux_and_7 = (aux_not_1 & input_switch2_addr1 & aux_not_3 & aux_not_4 & input_switch5_enable);
assign aux_and_8 = (input_switch1_addr0 & input_switch2_addr1 & aux_not_3 & aux_not_4 & input_switch5_enable);
assign aux_and_9 = (aux_not_1 & aux_not_2 & input_switch3_addr2 & aux_not_4 & input_switch5_enable);
assign aux_and_10 = (input_switch1_addr0 & aux_not_2 & input_switch3_addr2 & aux_not_4 & input_switch5_enable);
assign aux_and_11 = (aux_not_1 & input_switch2_addr1 & input_switch3_addr2 & aux_not_4 & input_switch5_enable);
assign aux_and_12 = (input_switch1_addr0 & input_switch2_addr1 & input_switch3_addr2 & aux_not_4 & input_switch5_enable);
assign aux_and_13 = (aux_not_1 & aux_not_2 & aux_not_3 & input_switch4_addr3 & input_switch5_enable);
assign aux_and_14 = (input_switch1_addr0 & aux_not_2 & aux_not_3 & input_switch4_addr3 & input_switch5_enable);
assign aux_and_15 = (aux_not_1 & input_switch2_addr1 & aux_not_3 & input_switch4_addr3 & input_switch5_enable);
assign aux_and_16 = (input_switch1_addr0 & input_switch2_addr1 & aux_not_3 & input_switch4_addr3 & input_switch5_enable);
assign aux_and_17 = (aux_not_1 & aux_not_2 & input_switch3_addr2 & input_switch4_addr3 & input_switch5_enable);
assign aux_and_18 = (input_switch1_addr0 & aux_not_2 & input_switch3_addr2 & input_switch4_addr3 & input_switch5_enable);
assign aux_and_19 = (aux_not_1 & input_switch2_addr1 & input_switch3_addr2 & input_switch4_addr3 & input_switch5_enable);
assign aux_and_20 = (input_switch1_addr0 & input_switch2_addr1 & input_switch3_addr2 & input_switch4_addr3 & input_switch5_enable);

// Writing output data. //
assign led26_out0_1 = aux_and_5;
assign led27_out1_1 = aux_and_6;
assign led28_out2_1 = aux_and_7;
assign led29_out3_1 = aux_and_8;
assign led30_out4_1 = aux_and_9;
assign led31_out5_1 = aux_and_10;
assign led32_out6_1 = aux_and_11;
assign led33_out7_1 = aux_and_12;
assign led34_out8_1 = aux_and_13;
assign led35_out9_1 = aux_and_14;
assign led36_out10_1 = aux_and_15;
assign led37_out11_1 = aux_and_16;
assign led38_out12_1 = aux_and_17;
assign led39_out13_1 = aux_and_18;
assign led40_out14_1 = aux_and_19;
assign led41_out15_1 = aux_and_20;
endmodule
