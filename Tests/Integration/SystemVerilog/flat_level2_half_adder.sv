// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_half_adder (
/* ========= Inputs ========== */
input input_switch1_a,
input input_switch2_b,

/* ========= Outputs ========== */
output led5_sum_1,
output led6_carry_1
);
/* ====== Aux. Variables ====== */
wire aux_xor_1;
wire aux_and_2;


// Assigning aux variables. //
assign aux_xor_1 = (input_switch1_a ^ input_switch2_b);
assign aux_and_2 = (input_switch1_a & input_switch2_b);

// Writing output data. //
assign led5_sum_1 = aux_xor_1;
assign led6_carry_1 = aux_and_2;
endmodule
