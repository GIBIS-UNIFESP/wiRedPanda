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

// Module for BusMux[0-3] (generated from level4_bus_mux_4bit.panda)
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

// Module for LEVEL4_BUS_MUX_8BIT (generated from level4_bus_mux_8bit.panda)
module level4_bus_mux_8bit_ic (
    input in00,
    input in01,
    input in02,
    input in03,
    input in04,
    input in05,
    input in06,
    input in07,
    input in10,
    input in11,
    input in12,
    input in13,
    input in14,
    input in15,
    input in16,
    input in17,
    input sel,
    output out0,
    output out1,
    output out2,
    output out3,
    output out4,
    output out5,
    output out6,
    output out7
);

// IC instance: BusMux[0-3] (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_1_out0;
wire w_level4_bus_mux_4bit_inst_1_out1;
wire w_level4_bus_mux_4bit_inst_1_out2;
wire w_level4_bus_mux_4bit_inst_1_out3;
// IC instance: BusMux[4-7] (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_2_out0;
wire w_level4_bus_mux_4bit_inst_2_out1;
wire w_level4_bus_mux_4bit_inst_2_out2;
wire w_level4_bus_mux_4bit_inst_2_out3;

// Internal logic
level4_bus_mux_4bit level4_bus_mux_4bit_inst_1 (
    .in00(in00),
    .in01(in01),
    .in02(in02),
    .in03(in03),
    .in10(in10),
    .in11(in11),
    .in12(in12),
    .in13(in13),
    .sel(sel),
    .out0(w_level4_bus_mux_4bit_inst_1_out0),
    .out1(w_level4_bus_mux_4bit_inst_1_out1),
    .out2(w_level4_bus_mux_4bit_inst_1_out2),
    .out3(w_level4_bus_mux_4bit_inst_1_out3)
);
level4_bus_mux_4bit level4_bus_mux_4bit_inst_2 (
    .in00(in04),
    .in01(in05),
    .in02(in06),
    .in03(in07),
    .in10(in14),
    .in11(in15),
    .in12(in16),
    .in13(in17),
    .sel(sel),
    .out0(w_level4_bus_mux_4bit_inst_2_out0),
    .out1(w_level4_bus_mux_4bit_inst_2_out1),
    .out2(w_level4_bus_mux_4bit_inst_2_out2),
    .out3(w_level4_bus_mux_4bit_inst_2_out3)
);

assign out0 = w_level4_bus_mux_4bit_inst_1_out0;
assign out1 = w_level4_bus_mux_4bit_inst_1_out1;
assign out2 = w_level4_bus_mux_4bit_inst_1_out2;
assign out3 = w_level4_bus_mux_4bit_inst_1_out3;
assign out4 = w_level4_bus_mux_4bit_inst_2_out0;
assign out5 = w_level4_bus_mux_4bit_inst_2_out1;
assign out6 = w_level4_bus_mux_4bit_inst_2_out2;
assign out7 = w_level4_bus_mux_4bit_inst_2_out3;
endmodule

module level4_bus_mux_8bit (
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

/* ========= Outputs ========== */
output led19_1,
output led20_1,
output led21_1,
output led22_1,
output led23_1,
output led24_1,
output led25_1,
output led26_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_BUS_MUX_8BIT (level4_bus_mux_8bit_ic)
wire w_level4_bus_mux_8bit_ic_inst_1_out0;
wire w_level4_bus_mux_8bit_ic_inst_1_out1;
wire w_level4_bus_mux_8bit_ic_inst_1_out2;
wire w_level4_bus_mux_8bit_ic_inst_1_out3;
wire w_level4_bus_mux_8bit_ic_inst_1_out4;
wire w_level4_bus_mux_8bit_ic_inst_1_out5;
wire w_level4_bus_mux_8bit_ic_inst_1_out6;
wire w_level4_bus_mux_8bit_ic_inst_1_out7;


// Assigning aux variables. //
level4_bus_mux_8bit_ic level4_bus_mux_8bit_ic_inst_1 (
    .in00(input_switch1),
    .in01(input_switch2),
    .in02(input_switch3),
    .in03(input_switch4),
    .in04(input_switch5),
    .in05(input_switch6),
    .in06(input_switch7),
    .in07(input_switch8),
    .in10(input_switch9),
    .in11(input_switch10),
    .in12(input_switch11),
    .in13(input_switch12),
    .in14(input_switch13),
    .in15(input_switch14),
    .in16(input_switch15),
    .in17(input_switch16),
    .sel(input_switch17),
    .out0(w_level4_bus_mux_8bit_ic_inst_1_out0),
    .out1(w_level4_bus_mux_8bit_ic_inst_1_out1),
    .out2(w_level4_bus_mux_8bit_ic_inst_1_out2),
    .out3(w_level4_bus_mux_8bit_ic_inst_1_out3),
    .out4(w_level4_bus_mux_8bit_ic_inst_1_out4),
    .out5(w_level4_bus_mux_8bit_ic_inst_1_out5),
    .out6(w_level4_bus_mux_8bit_ic_inst_1_out6),
    .out7(w_level4_bus_mux_8bit_ic_inst_1_out7)
);

// Writing output data. //
assign led19_1 = w_level4_bus_mux_8bit_ic_inst_1_out0;
assign led20_1 = w_level4_bus_mux_8bit_ic_inst_1_out1;
assign led21_1 = w_level4_bus_mux_8bit_ic_inst_1_out2;
assign led22_1 = w_level4_bus_mux_8bit_ic_inst_1_out3;
assign led23_1 = w_level4_bus_mux_8bit_ic_inst_1_out4;
assign led24_1 = w_level4_bus_mux_8bit_ic_inst_1_out5;
assign led25_1 = w_level4_bus_mux_8bit_ic_inst_1_out6;
assign led26_1 = w_level4_bus_mux_8bit_ic_inst_1_out7;
endmodule
