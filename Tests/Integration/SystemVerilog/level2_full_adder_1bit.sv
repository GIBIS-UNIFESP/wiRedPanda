// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_FULL_ADDER_1BIT (generated from level2_full_adder_1bit.panda)
module level2_full_adder_1bit_ic (
    input a,
    input b,
    input cin,
    output sum,
    output cout
);

wire aux_xor_1;
wire aux_and_2;
wire aux_xor_3;
wire aux_and_4;
wire aux_or_5;

// Internal logic
assign aux_xor_1 = (a ^ b);
assign aux_and_2 = (a & b);
assign aux_xor_3 = (aux_xor_1 ^ cin);
assign aux_and_4 = (aux_xor_1 & cin);
assign aux_or_5 = (aux_and_2 | aux_and_4);

assign sum = aux_xor_3;
assign cout = aux_or_5;
endmodule

module level2_full_adder_1bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,

/* ========= Outputs ========== */
output led5_1,
output led6_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_FULL_ADDER_1BIT (level2_full_adder_1bit_ic)
wire w_level2_full_adder_1bit_ic_inst_1_sum;
wire w_level2_full_adder_1bit_ic_inst_1_cout;


// Assigning aux variables. //
level2_full_adder_1bit_ic level2_full_adder_1bit_ic_inst_1 (
    .a(input_switch1),
    .b(input_switch2),
    .cin(input_switch3),
    .sum(w_level2_full_adder_1bit_ic_inst_1_sum),
    .cout(w_level2_full_adder_1bit_ic_inst_1_cout)
);

// Writing output data. //
assign led5_1 = w_level2_full_adder_1bit_ic_inst_1_sum;
assign led6_1 = w_level2_full_adder_1bit_ic_inst_1_cout;
endmodule
