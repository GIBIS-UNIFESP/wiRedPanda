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

// Module for BusMux_LoadHold (generated from level4_bus_mux_4bit.panda)
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

// Module for LEVEL4_REGISTER_4BIT (generated from level4_register_4bit.panda)
module level4_register_4bit_ic (
    input clk,
    input en,
    input d0,
    input d1,
    input d2,
    input d3,
    output q0,
    output q1,
    output q2,
    output q3
);

// IC instance: BusMux_LoadHold (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_1_out0;
wire w_level4_bus_mux_4bit_inst_1_out1;
wire w_level4_bus_mux_4bit_inst_1_out2;
wire w_level4_bus_mux_4bit_inst_1_out3;
reg aux_d_flip_flop_2_0_q = 1'b0;
reg aux_d_flip_flop_2_1_q = 1'b1;
reg aux_d_flip_flop_3_0_q = 1'b0;
reg aux_d_flip_flop_3_1_q = 1'b1;
reg aux_d_flip_flop_4_0_q = 1'b0;
reg aux_d_flip_flop_4_1_q = 1'b1;
reg aux_d_flip_flop_5_0_q = 1'b0;
reg aux_d_flip_flop_5_1_q = 1'b1;
wire aux_not_6;

// Internal logic
level4_bus_mux_4bit level4_bus_mux_4bit_inst_1 (
    .in00(d0),
    .in01(d1),
    .in02(d2),
    .in03(d3),
    .in10(aux_d_flip_flop_2_0_q),
    .in11(aux_d_flip_flop_3_0_q),
    .in12(aux_d_flip_flop_4_0_q),
    .in13(aux_d_flip_flop_5_0_q),
    .sel(aux_not_6),
    .out0(w_level4_bus_mux_4bit_inst_1_out0),
    .out1(w_level4_bus_mux_4bit_inst_1_out1),
    .out2(w_level4_bus_mux_4bit_inst_1_out2),
    .out3(w_level4_bus_mux_4bit_inst_1_out3)
);
    //D FlipFlop
    always @(posedge clk)
    begin
            aux_d_flip_flop_2_0_q <= w_level4_bus_mux_4bit_inst_1_out0;
            aux_d_flip_flop_2_1_q <= ~w_level4_bus_mux_4bit_inst_1_out0;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clk)
    begin
            aux_d_flip_flop_3_0_q <= w_level4_bus_mux_4bit_inst_1_out1;
            aux_d_flip_flop_3_1_q <= ~w_level4_bus_mux_4bit_inst_1_out1;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clk)
    begin
            aux_d_flip_flop_4_0_q <= w_level4_bus_mux_4bit_inst_1_out2;
            aux_d_flip_flop_4_1_q <= ~w_level4_bus_mux_4bit_inst_1_out2;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clk)
    begin
            aux_d_flip_flop_5_0_q <= w_level4_bus_mux_4bit_inst_1_out3;
            aux_d_flip_flop_5_1_q <= ~w_level4_bus_mux_4bit_inst_1_out3;
    end
    //End of D FlipFlop
assign aux_not_6 = ~en;

assign q0 = aux_d_flip_flop_2_0_q;
assign q1 = aux_d_flip_flop_3_0_q;
assign q2 = aux_d_flip_flop_4_0_q;
assign q3 = aux_d_flip_flop_5_0_q;
endmodule

module level4_register_4bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,

/* ========= Outputs ========== */
output led8_1,
output led9_1,
output led10_1,
output led11_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_REGISTER_4BIT (level4_register_4bit_ic)
wire w_level4_register_4bit_ic_inst_1_q0;
wire w_level4_register_4bit_ic_inst_1_q1;
wire w_level4_register_4bit_ic_inst_1_q2;
wire w_level4_register_4bit_ic_inst_1_q3;


// Assigning aux variables. //
level4_register_4bit_ic level4_register_4bit_ic_inst_1 (
    .clk(input_switch1),
    .en(input_switch2),
    .d0(input_switch3),
    .d1(input_switch4),
    .d2(input_switch5),
    .d3(input_switch6),
    .q0(w_level4_register_4bit_ic_inst_1_q0),
    .q1(w_level4_register_4bit_ic_inst_1_q1),
    .q2(w_level4_register_4bit_ic_inst_1_q2),
    .q3(w_level4_register_4bit_ic_inst_1_q3)
);

// Writing output data. //
assign led8_1 = w_level4_register_4bit_ic_inst_1_q0;
assign led9_1 = w_level4_register_4bit_ic_inst_1_q1;
assign led10_1 = w_level4_register_4bit_ic_inst_1_q2;
assign led11_1 = w_level4_register_4bit_ic_inst_1_q3;
endmodule
