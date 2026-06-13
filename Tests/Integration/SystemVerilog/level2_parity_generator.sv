// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_PARITY_GENERATOR (generated from level2_parity_generator.panda)
module level2_parity_generator_ic (
    input data0,
    input data1,
    input data2,
    input data3,
    output parity,
    output even
);

wire aux_xor_1;
wire aux_xor_2;
wire aux_xor_3;
wire aux_not_4;

// Internal logic
assign aux_xor_1 = (data0 ^ data1);
assign aux_xor_2 = (data2 ^ data3);
assign aux_xor_3 = (aux_xor_1 ^ aux_xor_2);
assign aux_not_4 = ~aux_xor_3;

assign parity = aux_xor_3;
assign even = aux_not_4;
endmodule

module level2_parity_generator (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,

/* ========= Outputs ========== */
output led6_1,
output led7_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_PARITY_GENERATOR (level2_parity_generator_ic)
wire w_level2_parity_generator_ic_inst_1_parity;
wire w_level2_parity_generator_ic_inst_1_even;


// Assigning aux variables. //
level2_parity_generator_ic level2_parity_generator_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .data2(input_switch3),
    .data3(input_switch4),
    .parity(w_level2_parity_generator_ic_inst_1_parity),
    .even(w_level2_parity_generator_ic_inst_1_even)
);

// Writing output data. //
assign led6_1 = w_level2_parity_generator_ic_inst_1_parity;
assign led7_1 = w_level2_parity_generator_ic_inst_1_even;
endmodule
