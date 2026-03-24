// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL3_COMPARATOR_4BIT (generated from level3_comparator_4bit.panda)
module level3_comparator_4bit_ic (
    input a0,
    input a1,
    input a2,
    input a3,
    input b0,
    input b1,
    input b2,
    input b3,
    output greater,
    output equal,
    output less
);

wire aux_not_1;
wire aux_not_2;
wire aux_not_3;
wire aux_not_4;
wire aux_not_5;
wire aux_not_6;
wire aux_not_7;
wire aux_not_8;
wire aux_xnor_9;
wire aux_and_10;
wire aux_and_11;
wire aux_xnor_12;
wire aux_and_13;
wire aux_and_14;
wire aux_xnor_15;
wire aux_and_16;
wire aux_and_17;
wire aux_xnor_18;
wire aux_and_19;
wire aux_and_20;
wire aux_and_21;
wire aux_and_22;
wire aux_or_23;
wire aux_and_24;
wire aux_or_25;
wire aux_and_26;
wire aux_or_27;
wire aux_and_28;
wire aux_or_29;
wire aux_and_30;
wire aux_or_31;
wire aux_and_32;
wire aux_or_33;

// Internal logic
assign aux_not_1 = ~a0;
assign aux_not_2 = ~b0;
assign aux_not_3 = ~a1;
assign aux_not_4 = ~b1;
assign aux_not_5 = ~a2;
assign aux_not_6 = ~b2;
assign aux_not_7 = ~a3;
assign aux_not_8 = ~b3;
assign aux_xnor_9 = ~(a0 ^ b0);
assign aux_and_10 = (a0 & aux_not_2);
assign aux_and_11 = (aux_not_1 & b0);
assign aux_xnor_12 = ~(a1 ^ b1);
assign aux_and_13 = (a1 & aux_not_4);
assign aux_and_14 = (aux_not_3 & b1);
assign aux_xnor_15 = ~(a2 ^ b2);
assign aux_and_16 = (a2 & aux_not_6);
assign aux_and_17 = (aux_not_5 & b2);
assign aux_xnor_18 = ~(a3 ^ b3);
assign aux_and_19 = (a3 & aux_not_8);
assign aux_and_20 = (aux_not_7 & b3);
assign aux_and_21 = (aux_xnor_9 & aux_xnor_12 & aux_xnor_15 & aux_xnor_18);
assign aux_and_22 = (aux_xnor_12 & aux_and_10);
assign aux_or_23 = (aux_and_13 | aux_and_22);
assign aux_and_24 = (aux_xnor_12 & aux_and_11);
assign aux_or_25 = (aux_and_14 | aux_and_24);
assign aux_and_26 = (aux_xnor_15 & aux_or_23);
assign aux_or_27 = (aux_and_16 | aux_and_26);
assign aux_and_28 = (aux_xnor_15 & aux_or_25);
assign aux_or_29 = (aux_and_17 | aux_and_28);
assign aux_and_30 = (aux_xnor_18 & aux_or_27);
assign aux_or_31 = (aux_and_19 | aux_and_30);
assign aux_and_32 = (aux_xnor_18 & aux_or_29);
assign aux_or_33 = (aux_and_20 | aux_and_32);

assign greater = aux_or_31;
assign equal = aux_and_21;
assign less = aux_or_33;
endmodule

module level3_comparator_4bit (
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
// IC instance: LEVEL3_COMPARATOR_4BIT (level3_comparator_4bit_ic)
wire w_level3_comparator_4bit_ic_inst_1_greater;
wire w_level3_comparator_4bit_ic_inst_1_equal;
wire w_level3_comparator_4bit_ic_inst_1_less;


// Assigning aux variables. //
level3_comparator_4bit_ic level3_comparator_4bit_ic_inst_1 (
    .a0(input_switch1),
    .a1(input_switch2),
    .a2(input_switch3),
    .a3(input_switch4),
    .b0(input_switch5),
    .b1(input_switch6),
    .b2(input_switch7),
    .b3(input_switch8),
    .greater(w_level3_comparator_4bit_ic_inst_1_greater),
    .equal(w_level3_comparator_4bit_ic_inst_1_equal),
    .less(w_level3_comparator_4bit_ic_inst_1_less)
);

// Writing output data. //
assign led10_1 = w_level3_comparator_4bit_ic_inst_1_greater;
assign led11_1 = w_level3_comparator_4bit_ic_inst_1_equal;
assign led12_1 = w_level3_comparator_4bit_ic_inst_1_less;
endmodule
