// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_HALF_ADDER (generated from level2_half_adder.panda)
module level2_half_adder_ic (
    input a,
    input b,
    output sum,
    output carry
);

wire aux_xor_1;
wire aux_and_2;

// Internal logic
assign aux_xor_1 = (a ^ b);
assign aux_and_2 = (a & b);

assign sum = aux_xor_1;
assign carry = aux_and_2;
endmodule

module level2_half_adder (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,

/* ========= Outputs ========== */
output led4_1,
output led5_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_HALF_ADDER (level2_half_adder_ic)
wire w_level2_half_adder_ic_inst_1_sum;
wire w_level2_half_adder_ic_inst_1_carry;


// Assigning aux variables. //
level2_half_adder_ic level2_half_adder_ic_inst_1 (
    .a(input_switch1),
    .b(input_switch2),
    .sum(w_level2_half_adder_ic_inst_1_sum),
    .carry(w_level2_half_adder_ic_inst_1_carry)
);

// Writing output data. //
assign led4_1 = w_level2_half_adder_ic_inst_1_sum;
assign led5_1 = w_level2_half_adder_ic_inst_1_carry;
endmodule
