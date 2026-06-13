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
    output addr2,
    output valid
);

wire aux_not_1;
wire aux_or_2;
wire aux_or_3;
wire aux_or_4;
wire aux_or_5;
wire aux_or_6;
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

// Internal logic
assign aux_not_1 = ~data7;
assign aux_or_2 = (data7 | data6);
assign aux_or_3 = (data7 | data6 | data5);
assign aux_or_4 = (data7 | data6 | data5 | data4);
assign aux_or_5 = (data7 | data6 | data5 | data4 | data3);
assign aux_or_6 = (data7 | data6 | data5 | data4 | data3 | data2);
assign aux_not_7 = ~aux_or_2;
assign aux_not_8 = ~aux_or_3;
assign aux_not_9 = ~aux_or_4;
assign aux_not_10 = ~aux_or_5;
assign aux_not_11 = ~aux_or_6;
assign aux_and_12 = (data6 & aux_not_1);
assign aux_and_13 = (data5 & aux_not_7);
assign aux_and_14 = (data4 & aux_not_8);
assign aux_and_15 = (data3 & aux_not_9);
assign aux_and_16 = (data2 & aux_not_10);
assign aux_and_17 = (data1 & aux_not_11);
assign aux_or_18 = (aux_and_14 | aux_and_13);
assign aux_or_19 = (aux_and_12 | data7);
assign aux_or_20 = (aux_or_18 | aux_or_19);
assign aux_or_21 = (aux_and_16 | aux_and_15);
assign aux_or_22 = (aux_and_12 | data7);
assign aux_or_23 = (aux_or_21 | aux_or_22);
assign aux_or_24 = (aux_and_17 | aux_and_15);
assign aux_or_25 = (aux_and_13 | data7);
assign aux_or_26 = (aux_or_24 | aux_or_25);
assign aux_or_27 = (data0 | data1 | data2 | data3 | data4 | data5 | data6 | data7);

assign addr0 = aux_or_26;
assign addr1 = aux_or_23;
assign addr2 = aux_or_20;
assign valid = aux_or_27;
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
output led12_1,
output led13_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_PRIORITY_ENCODER_8TO3 (level2_priority_encoder_8to3_ic)
wire w_level2_priority_encoder_8to3_ic_inst_1_addr0;
wire w_level2_priority_encoder_8to3_ic_inst_1_addr1;
wire w_level2_priority_encoder_8to3_ic_inst_1_addr2;
wire w_level2_priority_encoder_8to3_ic_inst_1_valid;


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
    .addr2(w_level2_priority_encoder_8to3_ic_inst_1_addr2),
    .valid(w_level2_priority_encoder_8to3_ic_inst_1_valid)
);

// Writing output data. //
assign led10_1 = w_level2_priority_encoder_8to3_ic_inst_1_addr0;
assign led11_1 = w_level2_priority_encoder_8to3_ic_inst_1_addr1;
assign led12_1 = w_level2_priority_encoder_8to3_ic_inst_1_addr2;
assign led13_1 = w_level2_priority_encoder_8to3_ic_inst_1_valid;
endmodule
