// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_parity_generator (
/* ========= Inputs ========== */
input input_switch1_data0,
input input_switch2_data1,
input input_switch3_data2,
input input_switch4_data3,
input input_switch5_cascadein,

/* ========= Outputs ========== */
output led10_parity_1
);
/* ====== Aux. Variables ====== */
wire aux_xor_1;
wire aux_xor_2;
wire aux_xor_3;
wire aux_xor_4;


// Assigning aux variables. //
assign aux_xor_1 = (input_switch1_data0 ^ input_switch2_data1);
assign aux_xor_2 = (input_switch3_data2 ^ input_switch4_data3);
assign aux_xor_3 = (aux_xor_1 ^ aux_xor_2);
assign aux_xor_4 = (aux_xor_3 ^ input_switch5_cascadein);

// Writing output data. //
assign led10_parity_1 = aux_xor_4;
endmodule
