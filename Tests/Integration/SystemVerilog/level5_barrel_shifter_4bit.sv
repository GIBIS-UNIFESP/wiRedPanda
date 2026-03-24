// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Mux[0] (generated from level2_mux_2to1.panda)
module level2_mux_2to1 (
    input data0,
    input data1,
    input sel0,
    output p_output
);

reg aux_mux_1 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel0})
            1'd0: aux_mux_1 = data0;
            1'd1: aux_mux_1 = data1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign p_output = aux_mux_1;
endmodule

// Module for BusMux_Left_S1 (generated from level4_bus_mux_4bit.panda)
module level4_bus_mux_4bit (
    input in00,
    input in01,
    input in02,
    input in03,
    input in10,
    input in11,
    input in12,
    input in13,
    input sel,
    output out0,
    output out1,
    output out2,
    output out3
);

// IC instance: Mux[0] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_1_p_output;
// IC instance: Mux[1] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_2_p_output;
// IC instance: Mux[2] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_3_p_output;
// IC instance: Mux[3] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_4_p_output;

// Internal logic
level2_mux_2to1 level2_mux_2to1_inst_1 (
    .data0(in00),
    .data1(in10),
    .sel0(sel),
    .p_output(w_level2_mux_2to1_inst_1_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_2 (
    .data0(in01),
    .data1(in11),
    .sel0(sel),
    .p_output(w_level2_mux_2to1_inst_2_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_3 (
    .data0(in02),
    .data1(in12),
    .sel0(sel),
    .p_output(w_level2_mux_2to1_inst_3_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_4 (
    .data0(in03),
    .data1(in13),
    .sel0(sel),
    .p_output(w_level2_mux_2to1_inst_4_p_output)
);

assign out0 = w_level2_mux_2to1_inst_1_p_output;
assign out1 = w_level2_mux_2to1_inst_2_p_output;
assign out2 = w_level2_mux_2to1_inst_3_p_output;
assign out3 = w_level2_mux_2to1_inst_4_p_output;
endmodule

// Module for LEVEL5_BARREL_SHIFTER_4BIT (generated from level5_barrel_shifter_4bit.panda)
module level5_barrel_shifter_4bit_ic (
    input data0,
    input data1,
    input data2,
    input data3,
    input shiftamount0,
    input shiftamount1,
    input shiftdirection,
    output shifteddata0,
    output shifteddata1,
    output shifteddata2,
    output shifteddata3
);

// IC instance: BusMux_Left_S1 (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_2_out0;
wire w_level4_bus_mux_4bit_inst_2_out1;
wire w_level4_bus_mux_4bit_inst_2_out2;
wire w_level4_bus_mux_4bit_inst_2_out3;
// IC instance: BusMux_Left_S2 (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_3_out0;
wire w_level4_bus_mux_4bit_inst_3_out1;
wire w_level4_bus_mux_4bit_inst_3_out2;
wire w_level4_bus_mux_4bit_inst_3_out3;
// IC instance: BusMux_Right_S1 (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_4_out0;
wire w_level4_bus_mux_4bit_inst_4_out1;
wire w_level4_bus_mux_4bit_inst_4_out2;
wire w_level4_bus_mux_4bit_inst_4_out3;
// IC instance: BusMux_Right_S2 (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_5_out0;
wire w_level4_bus_mux_4bit_inst_5_out1;
wire w_level4_bus_mux_4bit_inst_5_out2;
wire w_level4_bus_mux_4bit_inst_5_out3;
// IC instance: BusMux_Direction (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_6_out0;
wire w_level4_bus_mux_4bit_inst_6_out1;
wire w_level4_bus_mux_4bit_inst_6_out2;
wire w_level4_bus_mux_4bit_inst_6_out3;

// Internal logic
level4_bus_mux_4bit level4_bus_mux_4bit_inst_2 (
    .in00(data0),
    .in01(data1),
    .in02(data2),
    .in03(data3),
    .in10(1'b0),
    .in11(data0),
    .in12(data1),
    .in13(data2),
    .sel(shiftamount0),
    .out0(w_level4_bus_mux_4bit_inst_2_out0),
    .out1(w_level4_bus_mux_4bit_inst_2_out1),
    .out2(w_level4_bus_mux_4bit_inst_2_out2),
    .out3(w_level4_bus_mux_4bit_inst_2_out3)
);
level4_bus_mux_4bit level4_bus_mux_4bit_inst_3 (
    .in00(w_level4_bus_mux_4bit_inst_2_out0),
    .in01(w_level4_bus_mux_4bit_inst_2_out1),
    .in02(w_level4_bus_mux_4bit_inst_2_out2),
    .in03(w_level4_bus_mux_4bit_inst_2_out3),
    .in10(1'b0),
    .in11(1'b0),
    .in12(w_level4_bus_mux_4bit_inst_2_out0),
    .in13(w_level4_bus_mux_4bit_inst_2_out1),
    .sel(shiftamount1),
    .out0(w_level4_bus_mux_4bit_inst_3_out0),
    .out1(w_level4_bus_mux_4bit_inst_3_out1),
    .out2(w_level4_bus_mux_4bit_inst_3_out2),
    .out3(w_level4_bus_mux_4bit_inst_3_out3)
);
level4_bus_mux_4bit level4_bus_mux_4bit_inst_4 (
    .in00(data0),
    .in01(data1),
    .in02(data2),
    .in03(data3),
    .in10(data1),
    .in11(data2),
    .in12(data3),
    .in13(1'b0),
    .sel(shiftamount0),
    .out0(w_level4_bus_mux_4bit_inst_4_out0),
    .out1(w_level4_bus_mux_4bit_inst_4_out1),
    .out2(w_level4_bus_mux_4bit_inst_4_out2),
    .out3(w_level4_bus_mux_4bit_inst_4_out3)
);
level4_bus_mux_4bit level4_bus_mux_4bit_inst_5 (
    .in00(w_level4_bus_mux_4bit_inst_4_out0),
    .in01(w_level4_bus_mux_4bit_inst_4_out1),
    .in02(w_level4_bus_mux_4bit_inst_4_out2),
    .in03(w_level4_bus_mux_4bit_inst_4_out3),
    .in10(w_level4_bus_mux_4bit_inst_4_out2),
    .in11(w_level4_bus_mux_4bit_inst_4_out3),
    .in12(1'b0),
    .in13(1'b0),
    .sel(shiftamount1),
    .out0(w_level4_bus_mux_4bit_inst_5_out0),
    .out1(w_level4_bus_mux_4bit_inst_5_out1),
    .out2(w_level4_bus_mux_4bit_inst_5_out2),
    .out3(w_level4_bus_mux_4bit_inst_5_out3)
);
level4_bus_mux_4bit level4_bus_mux_4bit_inst_6 (
    .in00(w_level4_bus_mux_4bit_inst_3_out0),
    .in01(w_level4_bus_mux_4bit_inst_3_out1),
    .in02(w_level4_bus_mux_4bit_inst_3_out2),
    .in03(w_level4_bus_mux_4bit_inst_3_out3),
    .in10(w_level4_bus_mux_4bit_inst_5_out0),
    .in11(w_level4_bus_mux_4bit_inst_5_out1),
    .in12(w_level4_bus_mux_4bit_inst_5_out2),
    .in13(w_level4_bus_mux_4bit_inst_5_out3),
    .sel(shiftdirection),
    .out0(w_level4_bus_mux_4bit_inst_6_out0),
    .out1(w_level4_bus_mux_4bit_inst_6_out1),
    .out2(w_level4_bus_mux_4bit_inst_6_out2),
    .out3(w_level4_bus_mux_4bit_inst_6_out3)
);

assign shifteddata0 = w_level4_bus_mux_4bit_inst_6_out0;
assign shifteddata1 = w_level4_bus_mux_4bit_inst_6_out1;
assign shifteddata2 = w_level4_bus_mux_4bit_inst_6_out2;
assign shifteddata3 = w_level4_bus_mux_4bit_inst_6_out3;
endmodule

module level5_barrel_shifter_4bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,

/* ========= Outputs ========== */
output led9_1,
output led10_1,
output led11_1,
output led12_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL5_BARREL_SHIFTER_4BIT (level5_barrel_shifter_4bit_ic)
wire w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata0;
wire w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata1;
wire w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata2;
wire w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata3;


// Assigning aux variables. //
level5_barrel_shifter_4bit_ic level5_barrel_shifter_4bit_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .data2(input_switch3),
    .data3(input_switch4),
    .shiftamount0(input_switch5),
    .shiftamount1(input_switch6),
    .shiftdirection(input_switch7),
    .shifteddata0(w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata0),
    .shifteddata1(w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata1),
    .shifteddata2(w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata2),
    .shifteddata3(w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata3)
);

// Writing output data. //
assign led9_1 = w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata0;
assign led10_1 = w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata1;
assign led11_1 = w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata2;
assign led12_1 = w_level5_barrel_shifter_4bit_ic_inst_1_shifteddata3;
endmodule
