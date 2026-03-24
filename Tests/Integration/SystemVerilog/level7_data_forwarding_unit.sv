// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Mux4to1_0 (generated from level2_mux_4to1.panda)
module level2_mux_4to1 (
    input data0,
    input data1,
    input data2,
    input data3,
    input sel0,
    input sel1,
    output p_output
);

reg aux_mux_1 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel1, sel0})
            2'd0: aux_mux_1 = data0;
            2'd1: aux_mux_1 = data1;
            2'd2: aux_mux_1 = data2;
            2'd3: aux_mux_1 = data3;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign p_output = aux_mux_1;
endmodule

// Module for LEVEL7_DATA_FORWARDING_UNIT (generated from level7_data_forwarding_unit.panda)
module level7_data_forwarding_unit_ic (
    input dataa0,
    input dataa1,
    input dataa2,
    input dataa3,
    input dataa4,
    input dataa5,
    input dataa6,
    input dataa7,
    input datab0,
    input datab1,
    input datab2,
    input datab3,
    input datab4,
    input datab5,
    input datab6,
    input datab7,
    input select0,
    input select1,
    input datac0,
    input datac1,
    input datac2,
    input datac3,
    input datac4,
    input datac5,
    input datac6,
    input datac7,
    input datad0,
    input datad1,
    input datad2,
    input datad3,
    input datad4,
    input datad5,
    input datad6,
    input datad7,
    output forwardeddata0,
    output forwardeddata1,
    output forwardeddata2,
    output forwardeddata3,
    output forwardeddata4,
    output forwardeddata5,
    output forwardeddata6,
    output forwardeddata7
);

// IC instance: Mux4to1_0 (level2_mux_4to1)
wire w_level2_mux_4to1_inst_1_p_output;
// IC instance: Mux4to1_1 (level2_mux_4to1)
wire w_level2_mux_4to1_inst_2_p_output;
// IC instance: Mux4to1_2 (level2_mux_4to1)
wire w_level2_mux_4to1_inst_3_p_output;
// IC instance: Mux4to1_3 (level2_mux_4to1)
wire w_level2_mux_4to1_inst_4_p_output;
// IC instance: Mux4to1_4 (level2_mux_4to1)
wire w_level2_mux_4to1_inst_5_p_output;
// IC instance: Mux4to1_5 (level2_mux_4to1)
wire w_level2_mux_4to1_inst_6_p_output;
// IC instance: Mux4to1_6 (level2_mux_4to1)
wire w_level2_mux_4to1_inst_7_p_output;
// IC instance: Mux4to1_7 (level2_mux_4to1)
wire w_level2_mux_4to1_inst_8_p_output;

// Internal logic
level2_mux_4to1 level2_mux_4to1_inst_1 (
    .data0(dataa0),
    .data1(datab0),
    .data2(datac0),
    .data3(datad0),
    .sel0(select0),
    .sel1(select1),
    .p_output(w_level2_mux_4to1_inst_1_p_output)
);
level2_mux_4to1 level2_mux_4to1_inst_2 (
    .data0(dataa1),
    .data1(datab1),
    .data2(datac1),
    .data3(datad1),
    .sel0(select0),
    .sel1(select1),
    .p_output(w_level2_mux_4to1_inst_2_p_output)
);
level2_mux_4to1 level2_mux_4to1_inst_3 (
    .data0(dataa2),
    .data1(datab2),
    .data2(datac2),
    .data3(datad2),
    .sel0(select0),
    .sel1(select1),
    .p_output(w_level2_mux_4to1_inst_3_p_output)
);
level2_mux_4to1 level2_mux_4to1_inst_4 (
    .data0(dataa3),
    .data1(datab3),
    .data2(datac3),
    .data3(datad3),
    .sel0(select0),
    .sel1(select1),
    .p_output(w_level2_mux_4to1_inst_4_p_output)
);
level2_mux_4to1 level2_mux_4to1_inst_5 (
    .data0(dataa4),
    .data1(datab4),
    .data2(datac4),
    .data3(datad4),
    .sel0(select0),
    .sel1(select1),
    .p_output(w_level2_mux_4to1_inst_5_p_output)
);
level2_mux_4to1 level2_mux_4to1_inst_6 (
    .data0(dataa5),
    .data1(datab5),
    .data2(datac5),
    .data3(datad5),
    .sel0(select0),
    .sel1(select1),
    .p_output(w_level2_mux_4to1_inst_6_p_output)
);
level2_mux_4to1 level2_mux_4to1_inst_7 (
    .data0(dataa6),
    .data1(datab6),
    .data2(datac6),
    .data3(datad6),
    .sel0(select0),
    .sel1(select1),
    .p_output(w_level2_mux_4to1_inst_7_p_output)
);
level2_mux_4to1 level2_mux_4to1_inst_8 (
    .data0(dataa7),
    .data1(datab7),
    .data2(datac7),
    .data3(datad7),
    .sel0(select0),
    .sel1(select1),
    .p_output(w_level2_mux_4to1_inst_8_p_output)
);

assign forwardeddata0 = w_level2_mux_4to1_inst_1_p_output;
assign forwardeddata1 = w_level2_mux_4to1_inst_2_p_output;
assign forwardeddata2 = w_level2_mux_4to1_inst_3_p_output;
assign forwardeddata3 = w_level2_mux_4to1_inst_4_p_output;
assign forwardeddata4 = w_level2_mux_4to1_inst_5_p_output;
assign forwardeddata5 = w_level2_mux_4to1_inst_6_p_output;
assign forwardeddata6 = w_level2_mux_4to1_inst_7_p_output;
assign forwardeddata7 = w_level2_mux_4to1_inst_8_p_output;
endmodule

module level7_data_forwarding_unit (
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
input input_switch10,
input input_switch11,
input input_switch12,
input input_switch13,
input input_switch14,
input input_switch15,
input input_switch16,
input input_switch17,
input input_switch18,
input input_switch19,
input input_switch20,
input input_switch21,
input input_switch22,
input input_switch23,
input input_switch24,
input input_switch25,
input input_switch26,
input input_switch27,
input input_switch28,
input input_switch29,
input input_switch30,
input input_switch31,
input input_switch32,
input input_switch33,
input input_switch34,

/* ========= Outputs ========== */
output led36_1,
output led37_1,
output led38_1,
output led39_1,
output led40_1,
output led41_1,
output led42_1,
output led43_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL7_DATA_FORWARDING_UNIT (level7_data_forwarding_unit_ic)
wire w_level7_data_forwarding_unit_ic_inst_1_forwardeddata0;
wire w_level7_data_forwarding_unit_ic_inst_1_forwardeddata1;
wire w_level7_data_forwarding_unit_ic_inst_1_forwardeddata2;
wire w_level7_data_forwarding_unit_ic_inst_1_forwardeddata3;
wire w_level7_data_forwarding_unit_ic_inst_1_forwardeddata4;
wire w_level7_data_forwarding_unit_ic_inst_1_forwardeddata5;
wire w_level7_data_forwarding_unit_ic_inst_1_forwardeddata6;
wire w_level7_data_forwarding_unit_ic_inst_1_forwardeddata7;


// Assigning aux variables. //
level7_data_forwarding_unit_ic level7_data_forwarding_unit_ic_inst_1 (
    .dataa0(input_switch1),
    .dataa1(input_switch2),
    .dataa2(input_switch3),
    .dataa3(input_switch4),
    .dataa4(input_switch5),
    .dataa5(input_switch6),
    .dataa6(input_switch7),
    .dataa7(input_switch8),
    .datab0(input_switch9),
    .datab1(input_switch10),
    .datab2(input_switch11),
    .datab3(input_switch12),
    .datab4(input_switch13),
    .datab5(input_switch14),
    .datab6(input_switch15),
    .datab7(input_switch16),
    .select0(input_switch17),
    .select1(input_switch18),
    .datac0(input_switch19),
    .datac1(input_switch20),
    .datac2(input_switch21),
    .datac3(input_switch22),
    .datac4(input_switch23),
    .datac5(input_switch24),
    .datac6(input_switch25),
    .datac7(input_switch26),
    .datad0(input_switch27),
    .datad1(input_switch28),
    .datad2(input_switch29),
    .datad3(input_switch30),
    .datad4(input_switch31),
    .datad5(input_switch32),
    .datad6(input_switch33),
    .datad7(input_switch34),
    .forwardeddata0(w_level7_data_forwarding_unit_ic_inst_1_forwardeddata0),
    .forwardeddata1(w_level7_data_forwarding_unit_ic_inst_1_forwardeddata1),
    .forwardeddata2(w_level7_data_forwarding_unit_ic_inst_1_forwardeddata2),
    .forwardeddata3(w_level7_data_forwarding_unit_ic_inst_1_forwardeddata3),
    .forwardeddata4(w_level7_data_forwarding_unit_ic_inst_1_forwardeddata4),
    .forwardeddata5(w_level7_data_forwarding_unit_ic_inst_1_forwardeddata5),
    .forwardeddata6(w_level7_data_forwarding_unit_ic_inst_1_forwardeddata6),
    .forwardeddata7(w_level7_data_forwarding_unit_ic_inst_1_forwardeddata7)
);

// Writing output data. //
assign led36_1 = w_level7_data_forwarding_unit_ic_inst_1_forwardeddata0;
assign led37_1 = w_level7_data_forwarding_unit_ic_inst_1_forwardeddata1;
assign led38_1 = w_level7_data_forwarding_unit_ic_inst_1_forwardeddata2;
assign led39_1 = w_level7_data_forwarding_unit_ic_inst_1_forwardeddata3;
assign led40_1 = w_level7_data_forwarding_unit_ic_inst_1_forwardeddata4;
assign led41_1 = w_level7_data_forwarding_unit_ic_inst_1_forwardeddata5;
assign led42_1 = w_level7_data_forwarding_unit_ic_inst_1_forwardeddata6;
assign led43_1 = w_level7_data_forwarding_unit_ic_inst_1_forwardeddata7;
endmodule
