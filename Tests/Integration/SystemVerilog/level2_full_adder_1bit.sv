// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for HA1 (generated from level2_half_adder.panda)
module level2_half_adder (
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

// Module for LEVEL2_FULL_ADDER_1BIT (generated from level2_full_adder_1bit.panda)
module level2_full_adder_1bit_ic (
    input a,
    input b,
    input cin,
    output sum,
    output cout
);

// IC instance: HA1 (level2_half_adder)
wire w_level2_half_adder_inst_1_sum;
wire w_level2_half_adder_inst_1_carry;
// IC instance: HA2 (level2_half_adder)
wire w_level2_half_adder_inst_2_sum;
wire w_level2_half_adder_inst_2_carry;
wire aux_or_3;

// Internal logic
level2_half_adder level2_half_adder_inst_1 (
    .a(a),
    .b(b),
    .sum(w_level2_half_adder_inst_1_sum),
    .carry(w_level2_half_adder_inst_1_carry)
);
level2_half_adder level2_half_adder_inst_2 (
    .a(w_level2_half_adder_inst_1_sum),
    .b(cin),
    .sum(w_level2_half_adder_inst_2_sum),
    .carry(w_level2_half_adder_inst_2_carry)
);
assign aux_or_3 = (w_level2_half_adder_inst_1_carry | w_level2_half_adder_inst_2_carry);

assign sum = w_level2_half_adder_inst_2_sum;
assign cout = aux_or_3;
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
