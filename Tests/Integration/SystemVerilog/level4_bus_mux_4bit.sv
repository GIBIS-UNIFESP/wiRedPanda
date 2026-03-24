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

// Module for LEVEL4_BUS_MUX_4BIT (generated from level4_bus_mux_4bit.panda)
module level4_bus_mux_4bit_ic (
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

module level4_bus_mux_4bit (
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

/* ========= Outputs ========== */
output led11_1,
output led12_1,
output led13_1,
output led14_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_BUS_MUX_4BIT (level4_bus_mux_4bit_ic)
wire w_level4_bus_mux_4bit_ic_inst_1_out0;
wire w_level4_bus_mux_4bit_ic_inst_1_out1;
wire w_level4_bus_mux_4bit_ic_inst_1_out2;
wire w_level4_bus_mux_4bit_ic_inst_1_out3;


// Assigning aux variables. //
level4_bus_mux_4bit_ic level4_bus_mux_4bit_ic_inst_1 (
    .in00(input_switch1),
    .in01(input_switch2),
    .in02(input_switch3),
    .in03(input_switch4),
    .in10(input_switch5),
    .in11(input_switch6),
    .in12(input_switch7),
    .in13(input_switch8),
    .sel(input_switch9),
    .out0(w_level4_bus_mux_4bit_ic_inst_1_out0),
    .out1(w_level4_bus_mux_4bit_ic_inst_1_out1),
    .out2(w_level4_bus_mux_4bit_ic_inst_1_out2),
    .out3(w_level4_bus_mux_4bit_ic_inst_1_out3)
);

// Writing output data. //
assign led11_1 = w_level4_bus_mux_4bit_ic_inst_1_out0;
assign led12_1 = w_level4_bus_mux_4bit_ic_inst_1_out1;
assign led13_1 = w_level4_bus_mux_4bit_ic_inst_1_out2;
assign led14_1 = w_level4_bus_mux_4bit_ic_inst_1_out3;
endmodule
