// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_full_adder_1bit (
/* ========= Inputs ========== */
input input_switch1_a,
input input_switch2_b,
input input_switch3_cin,

/* ========= Outputs ========== */
output led9_sum_1,
output led10_cout_1
);
/* ====== Aux. Variables ====== */
wire aux_xor_1;
wire aux_and_2;
wire aux_and_3;
wire aux_xor_4;
wire aux_or_5;


// Assigning aux variables. //
assign aux_xor_1 = (input_switch1_a ^ input_switch2_b);
assign aux_and_2 = (input_switch1_a & input_switch2_b);
assign aux_and_3 = (aux_xor_1 & input_switch3_cin);
assign aux_xor_4 = (aux_xor_1 ^ input_switch3_cin);
assign aux_or_5 = (aux_and_2 | aux_and_3);

// Writing output data. //
assign led9_sum_1 = aux_xor_4;
assign led10_cout_1 = aux_or_5;
endmodule
