// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for FF0 (generated from level1_d_flip_flop.panda)
module level1_d_flip_flop (
    input d,
    input clock,
    input preset,
    input clear,
    output q,
    output q_bar
);

/* verilator lint_off UNOPTFLAT */ // intentional latch feedback
wire aux_not_1;
wire aux_not_2;
wire aux_not_3;
wire aux_or_4;
wire aux_or_5;
wire aux_or_6;
wire aux_or_7;
wire aux_not_8;
wire aux_and_9;
wire aux_and_10;
reg aux_nor_11 = 1'b0;
reg aux_nor_12 = 1'b0;
wire aux_and_13;
wire aux_and_14;
reg aux_nor_15 = 1'b0;
reg aux_nor_16 = 1'b0;

// Internal logic
assign aux_not_1 = ~clock;
assign aux_not_2 = ~preset;
assign aux_not_3 = ~clear;
assign aux_or_4 = (aux_and_14 | aux_not_3);
assign aux_or_5 = (aux_and_13 | aux_not_2);
assign aux_or_6 = (aux_and_10 | aux_not_3);
assign aux_or_7 = (aux_and_9 | aux_not_2);
assign aux_not_8 = ~d;
assign aux_and_9 = (d & aux_not_1);
assign aux_and_10 = (aux_not_8 & aux_not_1);
always @(*) aux_nor_11 = ~(aux_or_6 | aux_nor_12);
always @(*) aux_nor_12 = ~(aux_or_7 | aux_nor_11);
assign aux_and_13 = (aux_nor_11 & clock);
assign aux_and_14 = (aux_nor_12 & clock);
always @(*) aux_nor_15 = ~(aux_or_4 | aux_nor_16);
always @(*) aux_nor_16 = ~(aux_or_5 | aux_nor_15);

assign q = aux_nor_15;
assign q_bar = aux_nor_16;
/* verilator lint_on UNOPTFLAT */
endmodule

// Module for hold_mux0 (generated from level2_mux_2to1.panda)
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

// Module for LEVEL4_BINARY_COUNTER_4BIT (generated from level4_binary_counter_4bit.panda)
module level4_binary_counter_4bit_ic (
    input reset,
    input load,
    input countenable,
    input data0,
    input clk,
    input data1,
    input data2,
    input data3,
    output q0,
    output q1,
    output q2,
    output q3
);

wire aux_not_2;
wire aux_not_3;
wire aux_not_4;
wire aux_not_5;
wire aux_and_6;
wire aux_and_7;
wire aux_not_8;
wire aux_not_9;
wire aux_not_10;
wire aux_and_11;
wire aux_and_12;
wire aux_and_13;
wire aux_and_14;
wire aux_and_15;
wire aux_and_16;
wire aux_or_17;
wire aux_or_18;
wire aux_or_19;
// IC instance: FF0 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_20_q;
wire w_level1_d_flip_flop_inst_20_q_bar;
// IC instance: FF1 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_21_q;
wire w_level1_d_flip_flop_inst_21_q_bar;
// IC instance: FF2 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_22_q;
wire w_level1_d_flip_flop_inst_22_q_bar;
// IC instance: FF3 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_23_q;
wire w_level1_d_flip_flop_inst_23_q_bar;
// IC instance: hold_mux0 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_24_p_output;
// IC instance: load_mux0 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_25_p_output;
// IC instance: hold_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_26_p_output;
// IC instance: load_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_27_p_output;
// IC instance: hold_mux2 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_28_p_output;
// IC instance: load_mux2 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_29_p_output;
// IC instance: hold_mux3 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_30_p_output;
// IC instance: load_mux3 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_31_p_output;
wire aux_not_32;

// Internal logic
assign aux_not_2 = ~w_level1_d_flip_flop_inst_20_q;
assign aux_not_3 = ~w_level1_d_flip_flop_inst_21_q;
assign aux_not_4 = ~w_level1_d_flip_flop_inst_22_q;
assign aux_not_5 = ~w_level1_d_flip_flop_inst_23_q;
assign aux_and_6 = (w_level1_d_flip_flop_inst_20_q & w_level1_d_flip_flop_inst_21_q);
assign aux_and_7 = (aux_and_6 & w_level1_d_flip_flop_inst_22_q);
assign aux_not_8 = ~w_level1_d_flip_flop_inst_20_q;
assign aux_not_9 = ~aux_and_6;
assign aux_not_10 = ~aux_and_7;
assign aux_and_11 = (aux_not_3 & w_level1_d_flip_flop_inst_20_q);
assign aux_and_12 = (w_level1_d_flip_flop_inst_21_q & aux_not_8);
assign aux_and_13 = (aux_not_4 & aux_and_6);
assign aux_and_14 = (w_level1_d_flip_flop_inst_22_q & aux_not_9);
assign aux_and_15 = (aux_not_5 & aux_and_7);
assign aux_and_16 = (w_level1_d_flip_flop_inst_23_q & aux_not_10);
assign aux_or_17 = (aux_and_11 | aux_and_12);
assign aux_or_18 = (aux_and_13 | aux_and_14);
assign aux_or_19 = (aux_and_15 | aux_and_16);
level1_d_flip_flop level1_d_flip_flop_inst_20 (
    .d(w_level2_mux_2to1_inst_25_p_output),
    .clock(clk),
    .preset(1'b1),
    .clear(aux_not_32),
    .q(w_level1_d_flip_flop_inst_20_q),
    .q_bar(w_level1_d_flip_flop_inst_20_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_21 (
    .d(w_level2_mux_2to1_inst_27_p_output),
    .clock(clk),
    .preset(1'b1),
    .clear(aux_not_32),
    .q(w_level1_d_flip_flop_inst_21_q),
    .q_bar(w_level1_d_flip_flop_inst_21_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_22 (
    .d(w_level2_mux_2to1_inst_29_p_output),
    .clock(clk),
    .preset(1'b1),
    .clear(aux_not_32),
    .q(w_level1_d_flip_flop_inst_22_q),
    .q_bar(w_level1_d_flip_flop_inst_22_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_23 (
    .d(w_level2_mux_2to1_inst_31_p_output),
    .clock(clk),
    .preset(1'b1),
    .clear(aux_not_32),
    .q(w_level1_d_flip_flop_inst_23_q),
    .q_bar(w_level1_d_flip_flop_inst_23_q_bar)
);
level2_mux_2to1 level2_mux_2to1_inst_24 (
    .data0(w_level1_d_flip_flop_inst_20_q),
    .data1(aux_not_2),
    .sel0(countenable),
    .p_output(w_level2_mux_2to1_inst_24_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_25 (
    .data0(w_level2_mux_2to1_inst_24_p_output),
    .data1(data0),
    .sel0(load),
    .p_output(w_level2_mux_2to1_inst_25_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_26 (
    .data0(w_level1_d_flip_flop_inst_21_q),
    .data1(aux_or_17),
    .sel0(countenable),
    .p_output(w_level2_mux_2to1_inst_26_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_27 (
    .data0(w_level2_mux_2to1_inst_26_p_output),
    .data1(data1),
    .sel0(load),
    .p_output(w_level2_mux_2to1_inst_27_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_28 (
    .data0(w_level1_d_flip_flop_inst_22_q),
    .data1(aux_or_18),
    .sel0(countenable),
    .p_output(w_level2_mux_2to1_inst_28_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_29 (
    .data0(w_level2_mux_2to1_inst_28_p_output),
    .data1(data2),
    .sel0(load),
    .p_output(w_level2_mux_2to1_inst_29_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_30 (
    .data0(w_level1_d_flip_flop_inst_23_q),
    .data1(aux_or_19),
    .sel0(countenable),
    .p_output(w_level2_mux_2to1_inst_30_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_31 (
    .data0(w_level2_mux_2to1_inst_30_p_output),
    .data1(data3),
    .sel0(load),
    .p_output(w_level2_mux_2to1_inst_31_p_output)
);
assign aux_not_32 = ~reset;

assign q0 = w_level1_d_flip_flop_inst_20_q;
assign q1 = w_level1_d_flip_flop_inst_21_q;
assign q2 = w_level1_d_flip_flop_inst_22_q;
assign q3 = w_level1_d_flip_flop_inst_23_q;
endmodule

module level4_binary_counter_4bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,
input input_switch8,

/* ========= Outputs ========== */
output led10_1,
output led11_1,
output led12_1,
output led13_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_BINARY_COUNTER_4BIT (level4_binary_counter_4bit_ic)
wire w_level4_binary_counter_4bit_ic_inst_1_q0;
wire w_level4_binary_counter_4bit_ic_inst_1_q1;
wire w_level4_binary_counter_4bit_ic_inst_1_q2;
wire w_level4_binary_counter_4bit_ic_inst_1_q3;


// Assigning aux variables. //
level4_binary_counter_4bit_ic level4_binary_counter_4bit_ic_inst_1 (
    .reset(input_switch1),
    .load(input_switch2),
    .countenable(input_switch3),
    .data0(input_switch4),
    .clk(input_switch5),
    .data1(input_switch6),
    .data2(input_switch7),
    .data3(input_switch8),
    .q0(w_level4_binary_counter_4bit_ic_inst_1_q0),
    .q1(w_level4_binary_counter_4bit_ic_inst_1_q1),
    .q2(w_level4_binary_counter_4bit_ic_inst_1_q2),
    .q3(w_level4_binary_counter_4bit_ic_inst_1_q3)
);

// Writing output data. //
assign led10_1 = w_level4_binary_counter_4bit_ic_inst_1_q0;
assign led11_1 = w_level4_binary_counter_4bit_ic_inst_1_q1;
assign led12_1 = w_level4_binary_counter_4bit_ic_inst_1_q2;
assign led13_1 = w_level4_binary_counter_4bit_ic_inst_1_q3;
endmodule
