// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL3_COMPARATOR_4BIT_EQUALITY (generated from level3_comparator_4bit_equality.panda)
module level3_comparator_4bit_equality_ic (
    input a0,
    input a1,
    input a2,
    input a3,
    input b0,
    input b1,
    input b2,
    input b3,
    output equal
);

wire aux_xnor_1;
wire aux_xnor_2;
wire aux_xnor_3;
wire aux_xnor_4;
wire aux_and_5;

// Internal logic
assign aux_xnor_1 = ~(a0 ^ b0);
assign aux_xnor_2 = ~(a1 ^ b1);
assign aux_xnor_3 = ~(a2 ^ b2);
assign aux_xnor_4 = ~(a3 ^ b3);
assign aux_and_5 = (aux_xnor_1 & aux_xnor_2 & aux_xnor_3 & aux_xnor_4);

assign equal = aux_and_5;
endmodule

module level3_comparator_4bit_equality (
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
output led10_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL3_COMPARATOR_4BIT_EQUALITY (level3_comparator_4bit_equality_ic)
wire w_level3_comparator_4bit_equality_ic_inst_1_equal;


// Assigning aux variables. //
level3_comparator_4bit_equality_ic level3_comparator_4bit_equality_ic_inst_1 (
    .a0(input_switch1),
    .a1(input_switch2),
    .a2(input_switch3),
    .a3(input_switch4),
    .b0(input_switch5),
    .b1(input_switch6),
    .b2(input_switch7),
    .b3(input_switch8),
    .equal(w_level3_comparator_4bit_equality_ic_inst_1_equal)
);

// Writing output data. //
assign led10_1 = w_level3_comparator_4bit_equality_ic_inst_1_equal;
endmodule
