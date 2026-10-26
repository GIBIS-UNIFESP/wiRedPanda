// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_parity_checker (
/* ========= Inputs ========== */
input input_switch1_data0,
input input_switch2_data1,
input input_switch3_data2,
input input_switch4_data3,
input input_switch5_data4,
input input_switch6_data5,
input input_switch7_data6,
input input_switch8_data7,
input input_switch9_cascadein,

/* ========= Outputs ========== */
output led18_parity_1
);
/* ====== Aux. Variables ====== */
wire aux_xor_1;
wire aux_xor_2;
wire aux_xor_3;
wire aux_xor_4;
wire aux_xor_5;
wire aux_xor_6;
wire aux_xor_7;
wire aux_xor_8;


// Assigning aux variables. //
assign aux_xor_1 = (input_switch1_data0 ^ input_switch2_data1);
assign aux_xor_2 = (input_switch3_data2 ^ input_switch4_data3);
assign aux_xor_3 = (input_switch5_data4 ^ input_switch6_data5);
assign aux_xor_4 = (input_switch7_data6 ^ input_switch8_data7);
assign aux_xor_5 = (aux_xor_1 ^ aux_xor_2);
assign aux_xor_6 = (aux_xor_3 ^ aux_xor_4);
assign aux_xor_7 = (aux_xor_5 ^ aux_xor_6);
assign aux_xor_8 = (aux_xor_7 ^ input_switch9_cascadein);

// Writing output data. //
assign led18_parity_1 = aux_xor_8;
endmodule
