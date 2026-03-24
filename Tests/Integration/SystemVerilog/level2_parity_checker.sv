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
    output parity
);

wire aux_xor_1;
wire aux_xor_2;
wire aux_xor_3;
wire aux_xor_4;
wire aux_xor_5;
wire aux_xor_6;
wire aux_xor_7;

// Internal logic
assign aux_xor_1 = (data0 ^ data1);
assign aux_xor_2 = (data2 ^ data3);
assign aux_xor_3 = (data4 ^ data5);
assign aux_xor_4 = (data6 ^ data7);
assign aux_xor_5 = (aux_xor_1 ^ aux_xor_2);
assign aux_xor_6 = (aux_xor_3 ^ aux_xor_4);
assign aux_xor_7 = (aux_xor_5 ^ aux_xor_6);

assign parity = aux_xor_7;
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

/* ========= Outputs ========== */
output led10_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_PARITY_CHECKER (level2_parity_checker_ic)
wire w_level2_parity_checker_ic_inst_1_parity;


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
    .parity(w_level2_parity_checker_ic_inst_1_parity)
);

// Writing output data. //
assign led10_1 = w_level2_parity_checker_ic_inst_1_parity;
endmodule
