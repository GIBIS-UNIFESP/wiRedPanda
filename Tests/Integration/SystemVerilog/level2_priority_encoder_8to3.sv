// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_PRIORITY_ENCODER_8TO3 (generated from level2_priority_encoder_8to3.panda)
module level2_priority_encoder_8to3_ic (
    input data0,
    input data1,
    input data2,
    input data3,
    input data4,
    input data5,
    input data6,
    input data7,
    output addr0,
    output addr1,
    output addr2
);

wire aux_not_1;
wire aux_or_2;
wire aux_or_3;
wire aux_or_4;
wire aux_or_5;
wire aux_or_6;
wire aux_or_7;
wire aux_or_8;
wire aux_not_9;
wire aux_not_10;
wire aux_not_11;
wire aux_not_12;
wire aux_not_13;
wire aux_not_14;
wire aux_and_15;
wire aux_and_16;
wire aux_and_17;
wire aux_and_18;
wire aux_and_19;
wire aux_and_20;
wire aux_and_21;
wire aux_or_22;
wire aux_or_23;
wire aux_or_24;
wire aux_or_25;
wire aux_or_26;
wire aux_or_27;
wire aux_or_28;
wire aux_or_29;
wire aux_or_30;

// Internal logic
assign aux_not_1 = ~data7;
assign aux_or_2 = (data7 | data6);
assign aux_or_3 = (data7 | data6 | data5);
assign aux_or_4 = (data7 | data6 | data5 | data4);
assign aux_or_5 = (data7 | data6 | data5 | data4 | data3);
assign aux_or_6 = (data7 | data6 | data5 | data4 | data3 | data2);
assign aux_or_7 = (data7 | data6 | data5 | data4 | data3 | data2 | data1);
assign aux_or_8 = (data7 | data6 | data5 | data4 | data3 | data2 | data1 | data0);
assign aux_not_9 = ~aux_or_2;
assign aux_not_10 = ~aux_or_3;
assign aux_not_11 = ~aux_or_4;
assign aux_not_12 = ~aux_or_5;
assign aux_not_13 = ~aux_or_6;
assign aux_not_14 = ~aux_or_7;
assign aux_and_15 = (data6 & aux_not_1);
assign aux_and_16 = (data5 & aux_not_9);
assign aux_and_17 = (data4 & aux_not_10);
assign aux_and_18 = (data3 & aux_not_11);
assign aux_and_19 = (data2 & aux_not_12);
assign aux_and_20 = (data1 & aux_not_13);
assign aux_and_21 = (data0 & aux_not_14);
assign aux_or_22 = (aux_and_17 | aux_and_16);
assign aux_or_23 = (aux_and_15 | data7);
assign aux_or_24 = (aux_or_22 | aux_or_23);
assign aux_or_25 = (aux_and_19 | aux_and_18);
assign aux_or_26 = (aux_and_15 | data7);
assign aux_or_27 = (aux_or_25 | aux_or_26);
assign aux_or_28 = (aux_and_20 | aux_and_18);
assign aux_or_29 = (aux_and_16 | data7);
assign aux_or_30 = (aux_or_28 | aux_or_29);

assign addr0 = aux_or_30;
assign addr1 = aux_or_27;
assign addr2 = aux_or_24;
endmodule

module level2_priority_encoder_8to3 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,
input input_switch8,

/* ========= Outputs ========== */
output led10_1,
output led11_1,
output led12_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_PRIORITY_ENCODER_8TO3 (level2_priority_encoder_8to3_ic)
wire w_level2_priority_encoder_8to3_ic_inst_1_addr0;
wire w_level2_priority_encoder_8to3_ic_inst_1_addr1;
wire w_level2_priority_encoder_8to3_ic_inst_1_addr2;


// Assigning aux variables. //
level2_priority_encoder_8to3_ic level2_priority_encoder_8to3_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .data2(input_switch3),
    .data3(input_switch4),
    .data4(input_switch5),
    .data5(input_switch6),
    .data6(input_switch7),
    .data7(input_switch8),
    .addr0(w_level2_priority_encoder_8to3_ic_inst_1_addr0),
    .addr1(w_level2_priority_encoder_8to3_ic_inst_1_addr1),
    .addr2(w_level2_priority_encoder_8to3_ic_inst_1_addr2)
);

// Writing output data. //
assign led10_1 = w_level2_priority_encoder_8to3_ic_inst_1_addr0;
assign led11_1 = w_level2_priority_encoder_8to3_ic_inst_1_addr1;
assign led12_1 = w_level2_priority_encoder_8to3_ic_inst_1_addr2;
endmodule
