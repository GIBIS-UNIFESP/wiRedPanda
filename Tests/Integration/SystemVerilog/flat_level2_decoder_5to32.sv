// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_decoder_5to32 (
/* ========= Inputs ========== */
input input_switch1_addr0,
input input_switch2_addr1,
input input_switch3_addr2,
input input_switch4_addr3,
input input_switch5_addr4,
input input_switch6_enable,

/* ========= Outputs ========== */
output led44_out0_1,
output led45_out1_1,
output led46_out2_1,
output led47_out3_1,
output led48_out4_1,
output led49_out5_1,
output led50_out6_1,
output led51_out7_1,
output led52_out8_1,
output led53_out9_1,
output led54_out10_1,
output led55_out11_1,
output led56_out12_1,
output led57_out13_1,
output led58_out14_1,
output led59_out15_1,
output led60_out16_1,
output led61_out17_1,
output led62_out18_1,
output led63_out19_1,
output led64_out20_1,
output led65_out21_1,
output led66_out22_1,
output led67_out23_1,
output led68_out24_1,
output led69_out25_1,
output led70_out26_1,
output led71_out27_1,
output led72_out28_1,
output led73_out29_1,
output led74_out30_1,
output led75_out31_1
);
/* ====== Aux. Variables ====== */
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


// Assigning aux variables. //
assign aux_not_1 = ~input_switch1_addr0;
assign aux_not_2 = ~input_switch2_addr1;
assign aux_not_3 = ~input_switch3_addr2;
assign aux_not_4 = ~input_switch4_addr3;
assign aux_not_5 = ~input_switch5_addr4;
assign aux_and_6 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4 & aux_not_5 & input_switch6_enable);
assign aux_and_7 = (input_switch1_addr0 & aux_not_2 & aux_not_3 & aux_not_4 & aux_not_5 & input_switch6_enable);
assign aux_and_8 = (aux_not_1 & input_switch2_addr1 & aux_not_3 & aux_not_4 & aux_not_5 & input_switch6_enable);
assign aux_and_9 = (input_switch1_addr0 & input_switch2_addr1 & aux_not_3 & aux_not_4 & aux_not_5 & input_switch6_enable);
assign aux_and_10 = (aux_not_1 & aux_not_2 & input_switch3_addr2 & aux_not_4 & aux_not_5 & input_switch6_enable);
assign aux_and_11 = (input_switch1_addr0 & aux_not_2 & input_switch3_addr2 & aux_not_4 & aux_not_5 & input_switch6_enable);
assign aux_and_12 = (aux_not_1 & input_switch2_addr1 & input_switch3_addr2 & aux_not_4 & aux_not_5 & input_switch6_enable);
assign aux_and_13 = (input_switch1_addr0 & input_switch2_addr1 & input_switch3_addr2 & aux_not_4 & aux_not_5 & input_switch6_enable);
assign aux_and_14 = (aux_not_1 & aux_not_2 & aux_not_3 & input_switch4_addr3 & aux_not_5 & input_switch6_enable);
assign aux_and_15 = (input_switch1_addr0 & aux_not_2 & aux_not_3 & input_switch4_addr3 & aux_not_5 & input_switch6_enable);
assign aux_and_16 = (aux_not_1 & input_switch2_addr1 & aux_not_3 & input_switch4_addr3 & aux_not_5 & input_switch6_enable);
assign aux_and_17 = (input_switch1_addr0 & input_switch2_addr1 & aux_not_3 & input_switch4_addr3 & aux_not_5 & input_switch6_enable);
assign aux_and_18 = (aux_not_1 & aux_not_2 & input_switch3_addr2 & input_switch4_addr3 & aux_not_5 & input_switch6_enable);
assign aux_and_19 = (input_switch1_addr0 & aux_not_2 & input_switch3_addr2 & input_switch4_addr3 & aux_not_5 & input_switch6_enable);
assign aux_and_20 = (aux_not_1 & input_switch2_addr1 & input_switch3_addr2 & input_switch4_addr3 & aux_not_5 & input_switch6_enable);
assign aux_and_21 = (input_switch1_addr0 & input_switch2_addr1 & input_switch3_addr2 & input_switch4_addr3 & aux_not_5 & input_switch6_enable);
assign aux_and_22 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_23 = (input_switch1_addr0 & aux_not_2 & aux_not_3 & aux_not_4 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_24 = (aux_not_1 & input_switch2_addr1 & aux_not_3 & aux_not_4 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_25 = (input_switch1_addr0 & input_switch2_addr1 & aux_not_3 & aux_not_4 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_26 = (aux_not_1 & aux_not_2 & input_switch3_addr2 & aux_not_4 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_27 = (input_switch1_addr0 & aux_not_2 & input_switch3_addr2 & aux_not_4 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_28 = (aux_not_1 & input_switch2_addr1 & input_switch3_addr2 & aux_not_4 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_29 = (input_switch1_addr0 & input_switch2_addr1 & input_switch3_addr2 & aux_not_4 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_30 = (aux_not_1 & aux_not_2 & aux_not_3 & input_switch4_addr3 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_31 = (input_switch1_addr0 & aux_not_2 & aux_not_3 & input_switch4_addr3 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_32 = (aux_not_1 & input_switch2_addr1 & aux_not_3 & input_switch4_addr3 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_33 = (input_switch1_addr0 & input_switch2_addr1 & aux_not_3 & input_switch4_addr3 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_34 = (aux_not_1 & aux_not_2 & input_switch3_addr2 & input_switch4_addr3 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_35 = (input_switch1_addr0 & aux_not_2 & input_switch3_addr2 & input_switch4_addr3 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_36 = (aux_not_1 & input_switch2_addr1 & input_switch3_addr2 & input_switch4_addr3 & input_switch5_addr4 & input_switch6_enable);
assign aux_and_37 = (input_switch1_addr0 & input_switch2_addr1 & input_switch3_addr2 & input_switch4_addr3 & input_switch5_addr4 & input_switch6_enable);

// Writing output data. //
assign led44_out0_1 = aux_and_6;
assign led45_out1_1 = aux_and_7;
assign led46_out2_1 = aux_and_8;
assign led47_out3_1 = aux_and_9;
assign led48_out4_1 = aux_and_10;
assign led49_out5_1 = aux_and_11;
assign led50_out6_1 = aux_and_12;
assign led51_out7_1 = aux_and_13;
assign led52_out8_1 = aux_and_14;
assign led53_out9_1 = aux_and_15;
assign led54_out10_1 = aux_and_16;
assign led55_out11_1 = aux_and_17;
assign led56_out12_1 = aux_and_18;
assign led57_out13_1 = aux_and_19;
assign led58_out14_1 = aux_and_20;
assign led59_out15_1 = aux_and_21;
assign led60_out16_1 = aux_and_22;
assign led61_out17_1 = aux_and_23;
assign led62_out18_1 = aux_and_24;
assign led63_out19_1 = aux_and_25;
assign led64_out20_1 = aux_and_26;
assign led65_out21_1 = aux_and_27;
assign led66_out22_1 = aux_and_28;
assign led67_out23_1 = aux_and_29;
assign led68_out24_1 = aux_and_30;
assign led69_out25_1 = aux_and_31;
assign led70_out26_1 = aux_and_32;
assign led71_out27_1 = aux_and_33;
assign led72_out28_1 = aux_and_34;
assign led73_out29_1 = aux_and_35;
assign led74_out30_1 = aux_and_36;
assign led75_out31_1 = aux_and_37;
endmodule
