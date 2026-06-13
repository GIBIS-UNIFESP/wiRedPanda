// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_PARITY_CHECKER (generated from level2_parity_checker.panda)
module level2_parity_checker_ic (
    input data0,
    input data1,
    input data2,
    input data3,
    input data4,
    input data5,
    input data6,
    input data7,
    input cascadein,
    output parity,
    output even
);

wire aux_xor_1;
wire aux_xor_2;
wire aux_xor_3;
wire aux_xor_4;
wire aux_xor_5;
wire aux_xor_6;
wire aux_xor_7;
wire aux_xor_8;
wire aux_not_9;

// Internal logic
assign aux_xor_1 = (data0 ^ data1);
assign aux_xor_2 = (data2 ^ data3);
assign aux_xor_3 = (data4 ^ data5);
assign aux_xor_4 = (data6 ^ data7);
assign aux_xor_5 = (aux_xor_1 ^ aux_xor_2);
assign aux_xor_6 = (aux_xor_3 ^ aux_xor_4);
assign aux_xor_7 = (aux_xor_5 ^ aux_xor_6);
assign aux_xor_8 = (aux_xor_7 ^ cascadein);
assign aux_not_9 = ~aux_xor_8;

assign parity = aux_xor_8;
assign even = aux_not_9;
endmodule

module level2_parity_checker (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,
input input_switch8,
input input_switch9,

/* ========= Outputs ========== */
output led11_1,
output led12_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_PARITY_CHECKER (level2_parity_checker_ic)
wire w_level2_parity_checker_ic_inst_1_parity;
wire w_level2_parity_checker_ic_inst_1_even;


// Assigning aux variables. //
level2_parity_checker_ic level2_parity_checker_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .data2(input_switch3),
    .data3(input_switch4),
    .data4(input_switch5),
    .data5(input_switch6),
    .data6(input_switch7),
    .data7(input_switch8),
    .cascadein(input_switch9),
    .parity(w_level2_parity_checker_ic_inst_1_parity),
    .even(w_level2_parity_checker_ic_inst_1_even)
);

// Writing output data. //
assign led11_1 = w_level2_parity_checker_ic_inst_1_parity;
assign led12_1 = w_level2_parity_checker_ic_inst_1_even;
endmodule
