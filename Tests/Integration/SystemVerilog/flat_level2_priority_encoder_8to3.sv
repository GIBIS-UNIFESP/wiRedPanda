// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_priority_encoder_8to3 (
/* ========= Inputs ========== */
input input_switch1_data2,
input input_switch2_data3,
input input_switch3_data4,
input input_switch4_data5,
input input_switch5_data6,
input input_switch6_data7,
input input_switch7_data1,
input input_switch8_data0,
input input_switch9_ei,

/* ========= Outputs ========== */
output led46_addr0_1,
output led47_addr1_1,
output led48_addr2_1,
output led49_valid_1,
output led50_eo_1
);
/* ====== Aux. Variables ====== */
wire aux_or_1;
wire aux_or_2;
wire aux_or_3;
wire aux_or_4;
wire aux_or_5;
wire aux_not_6;
wire aux_not_7;
wire aux_not_8;
wire aux_not_9;
wire aux_not_10;
wire aux_not_11;
wire aux_and_12;
wire aux_and_13;
wire aux_and_14;
wire aux_and_15;
wire aux_and_16;
wire aux_and_17;
wire aux_or_18;
wire aux_or_19;
wire aux_or_20;
wire aux_or_21;
wire aux_or_22;
wire aux_or_23;
wire aux_or_24;
wire aux_or_25;
wire aux_or_26;
wire aux_or_27;
wire aux_or_28;
wire aux_not_29;
wire aux_not_30;
wire aux_and_31;
wire aux_and_32;
wire aux_and_33;
wire aux_and_34;
wire aux_and_35;
wire aux_and_36;


// Assigning aux variables. //
assign aux_or_1 = (input_switch6_data7 | input_switch5_data6);
assign aux_or_2 = (input_switch6_data7 | input_switch5_data6 | input_switch4_data5);
assign aux_or_3 = (input_switch6_data7 | input_switch5_data6 | input_switch4_data5 | input_switch3_data4);
assign aux_or_4 = (input_switch6_data7 | input_switch5_data6 | input_switch4_data5 | input_switch3_data4 | input_switch2_data3);
assign aux_or_5 = (input_switch6_data7 | input_switch5_data6 | input_switch4_data5 | input_switch3_data4 | input_switch2_data3 | input_switch1_data2);
assign aux_not_6 = ~input_switch6_data7;
assign aux_not_7 = ~aux_or_1;
assign aux_not_8 = ~aux_or_2;
assign aux_not_9 = ~aux_or_3;
assign aux_not_10 = ~aux_or_4;
assign aux_not_11 = ~aux_or_5;
assign aux_and_12 = (input_switch5_data6 & aux_not_6);
assign aux_and_13 = (input_switch4_data5 & aux_not_7);
assign aux_and_14 = (input_switch3_data4 & aux_not_8);
assign aux_and_15 = (input_switch2_data3 & aux_not_9);
assign aux_and_16 = (input_switch1_data2 & aux_not_10);
assign aux_and_17 = (input_switch7_data1 & aux_not_11);
assign aux_or_18 = (aux_and_14 | aux_and_13);
assign aux_or_19 = (aux_and_12 | input_switch6_data7);
assign aux_or_20 = (aux_and_16 | aux_and_15);
assign aux_or_21 = (aux_and_12 | input_switch6_data7);
assign aux_or_22 = (aux_and_17 | aux_and_15);
assign aux_or_23 = (aux_and_13 | input_switch6_data7);
assign aux_or_24 = (input_switch8_data0 | input_switch7_data1 | input_switch1_data2 | input_switch2_data3 | input_switch3_data4 | input_switch4_data5 | input_switch5_data6 | input_switch6_data7);
assign aux_or_25 = (input_switch6_data7 | input_switch5_data6 | input_switch4_data5 | input_switch3_data4 | input_switch2_data3 | input_switch1_data2 | input_switch7_data1);
assign aux_or_26 = (aux_or_18 | aux_or_19);
assign aux_or_27 = (aux_or_20 | aux_or_21);
assign aux_or_28 = (aux_or_22 | aux_or_23);
assign aux_not_29 = ~aux_or_24;
assign aux_not_30 = ~aux_or_25;
assign aux_and_31 = (aux_or_28 & input_switch9_ei);
assign aux_and_32 = (aux_or_27 & input_switch9_ei);
assign aux_and_33 = (aux_or_26 & input_switch9_ei);
assign aux_and_34 = (aux_or_24 & input_switch9_ei);
assign aux_and_35 = (input_switch9_ei & aux_not_29);
assign aux_and_36 = (input_switch8_data0 & aux_not_30);

// Writing output data. //
assign led46_addr0_1 = aux_and_31;
assign led47_addr1_1 = aux_and_32;
assign led48_addr2_1 = aux_and_33;
assign led49_valid_1 = aux_and_34;
assign led50_eo_1 = aux_and_35;
endmodule
