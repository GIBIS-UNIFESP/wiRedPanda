// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Behavioral module for level1_d_flip_flop (generated from level1_d_flip_flop.panda)
module level1_d_flip_flop (
    input d,
    input clock,
    input preset,
    input clear,
    output reg q,
    output reg q_bar
);
    initial begin
        q = 1'b1;
        q_bar = 1'b0;
    end
    always @(posedge clock or negedge preset or negedge clear)
    begin
        if (~preset)
        begin
            q <= 1'b1;
            q_bar <= 1'b0;
        end
        else if (~clear)
        begin
            q <= 1'b0;
            q_bar <= 1'b1;
        end
        else
        begin
            q <= d;
            q_bar <= ~d;
        end
    end
endmodule

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

// Module for BusMux_Load (generated from level4_bus_mux_4bit.panda)
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

// Module for LEVEL4_SHIFT_REGISTER_PISO (generated from level4_shift_register_piso.panda)
module level4_shift_register_piso_ic (
    input clk,
    input load,
    input d0,
    input d1,
    input d2,
    input d3,
    output sout
);

wire aux_not_1;
// IC instance: FF0 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_2_q;
wire w_level1_d_flip_flop_inst_2_q_bar;
// IC instance: FF1 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_3_q;
wire w_level1_d_flip_flop_inst_3_q_bar;
// IC instance: FF2 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_4_q;
wire w_level1_d_flip_flop_inst_4_q_bar;
// IC instance: FF3 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_5_q;
wire w_level1_d_flip_flop_inst_5_q_bar;
// IC instance: BusMux_Load (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_6_out0;
wire w_level4_bus_mux_4bit_inst_6_out1;
wire w_level4_bus_mux_4bit_inst_6_out2;
wire w_level4_bus_mux_4bit_inst_6_out3;
// IC instance: BusMux_Shift (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_7_out0;
wire w_level4_bus_mux_4bit_inst_7_out1;
wire w_level4_bus_mux_4bit_inst_7_out2;
wire w_level4_bus_mux_4bit_inst_7_out3;
// IC instance: mux_sel0 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_8_p_output;
// IC instance: mux_sel1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_9_p_output;
// IC instance: mux_sel2 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_10_p_output;
// IC instance: mux_sel3 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_11_p_output;

// Internal logic
assign aux_not_1 = ~load;
level1_d_flip_flop level1_d_flip_flop_inst_2 (
    .d(w_level2_mux_2to1_inst_8_p_output),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_2_q),
    .q_bar(w_level1_d_flip_flop_inst_2_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_3 (
    .d(w_level2_mux_2to1_inst_9_p_output),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_3_q),
    .q_bar(w_level1_d_flip_flop_inst_3_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_4 (
    .d(w_level2_mux_2to1_inst_10_p_output),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_4_q),
    .q_bar(w_level1_d_flip_flop_inst_4_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_5 (
    .d(w_level2_mux_2to1_inst_11_p_output),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_5_q),
    .q_bar(w_level1_d_flip_flop_inst_5_q_bar)
);
level4_bus_mux_4bit level4_bus_mux_4bit_inst_6 (
    .in00(1'b0),
    .in01(1'b0),
    .in02(1'b0),
    .in03(1'b0),
    .in10(d0),
    .in11(d1),
    .in12(d2),
    .in13(d3),
    .sel(load),
    .out0(w_level4_bus_mux_4bit_inst_6_out0),
    .out1(w_level4_bus_mux_4bit_inst_6_out1),
    .out2(w_level4_bus_mux_4bit_inst_6_out2),
    .out3(w_level4_bus_mux_4bit_inst_6_out3)
);
level4_bus_mux_4bit level4_bus_mux_4bit_inst_7 (
    .in00(1'b0),
    .in01(1'b0),
    .in02(1'b0),
    .in03(1'b0),
    .in10(w_level1_d_flip_flop_inst_3_q),
    .in11(w_level1_d_flip_flop_inst_4_q),
    .in12(w_level1_d_flip_flop_inst_5_q),
    .in13(1'b0),
    .sel(aux_not_1),
    .out0(w_level4_bus_mux_4bit_inst_7_out0),
    .out1(w_level4_bus_mux_4bit_inst_7_out1),
    .out2(w_level4_bus_mux_4bit_inst_7_out2),
    .out3(w_level4_bus_mux_4bit_inst_7_out3)
);
level2_mux_2to1 level2_mux_2to1_inst_8 (
    .data0(w_level4_bus_mux_4bit_inst_7_out0),
    .data1(w_level4_bus_mux_4bit_inst_6_out0),
    .sel0(load),
    .p_output(w_level2_mux_2to1_inst_8_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_9 (
    .data0(w_level4_bus_mux_4bit_inst_7_out1),
    .data1(w_level4_bus_mux_4bit_inst_6_out1),
    .sel0(load),
    .p_output(w_level2_mux_2to1_inst_9_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_10 (
    .data0(w_level4_bus_mux_4bit_inst_7_out2),
    .data1(w_level4_bus_mux_4bit_inst_6_out2),
    .sel0(load),
    .p_output(w_level2_mux_2to1_inst_10_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_11 (
    .data0(w_level4_bus_mux_4bit_inst_7_out3),
    .data1(w_level4_bus_mux_4bit_inst_6_out3),
    .sel0(load),
    .p_output(w_level2_mux_2to1_inst_11_p_output)
);

assign sout = w_level1_d_flip_flop_inst_2_q;
endmodule

module level4_shift_register_piso (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,

/* ========= Outputs ========== */
output led8_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_SHIFT_REGISTER_PISO (level4_shift_register_piso_ic)
wire w_level4_shift_register_piso_ic_inst_1_sout;


// Assigning aux variables. //
level4_shift_register_piso_ic level4_shift_register_piso_ic_inst_1 (
    .clk(input_switch1),
    .load(input_switch2),
    .d0(input_switch3),
    .d1(input_switch4),
    .d2(input_switch5),
    .d3(input_switch6),
    .sout(w_level4_shift_register_piso_ic_inst_1_sout)
);

// Writing output data. //
assign led8_1 = w_level4_shift_register_piso_ic_inst_1_sout;
endmodule
