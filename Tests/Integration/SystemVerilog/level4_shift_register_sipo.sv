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

// Module for LEVEL4_SHIFT_REGISTER_SIPO (generated from level4_shift_register_sipo.panda)
module level4_shift_register_sipo_ic (
    input clk,
    input sin,
    output q0,
    output q1,
    output q2,
    output q3
);

// IC instance: FF0 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_1_q;
wire w_level1_d_flip_flop_inst_1_q_bar;
// IC instance: FF1 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_2_q;
wire w_level1_d_flip_flop_inst_2_q_bar;
// IC instance: FF2 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_3_q;
wire w_level1_d_flip_flop_inst_3_q_bar;
// IC instance: FF3 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_4_q;
wire w_level1_d_flip_flop_inst_4_q_bar;

// Internal logic
level1_d_flip_flop level1_d_flip_flop_inst_1 (
    .d(sin),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_1_q),
    .q_bar(w_level1_d_flip_flop_inst_1_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_2 (
    .d(w_level1_d_flip_flop_inst_1_q),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_2_q),
    .q_bar(w_level1_d_flip_flop_inst_2_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_3 (
    .d(w_level1_d_flip_flop_inst_2_q),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_3_q),
    .q_bar(w_level1_d_flip_flop_inst_3_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_4 (
    .d(w_level1_d_flip_flop_inst_3_q),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_4_q),
    .q_bar(w_level1_d_flip_flop_inst_4_q_bar)
);

assign q0 = w_level1_d_flip_flop_inst_1_q;
assign q1 = w_level1_d_flip_flop_inst_2_q;
assign q2 = w_level1_d_flip_flop_inst_3_q;
assign q3 = w_level1_d_flip_flop_inst_4_q;
endmodule

module level4_shift_register_sipo (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,

/* ========= Outputs ========== */
output led4_1,
output led5_1,
output led6_1,
output led7_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_SHIFT_REGISTER_SIPO (level4_shift_register_sipo_ic)
wire w_level4_shift_register_sipo_ic_inst_1_q0;
wire w_level4_shift_register_sipo_ic_inst_1_q1;
wire w_level4_shift_register_sipo_ic_inst_1_q2;
wire w_level4_shift_register_sipo_ic_inst_1_q3;


// Assigning aux variables. //
level4_shift_register_sipo_ic level4_shift_register_sipo_ic_inst_1 (
    .clk(input_switch1),
    .sin(input_switch2),
    .q0(w_level4_shift_register_sipo_ic_inst_1_q0),
    .q1(w_level4_shift_register_sipo_ic_inst_1_q1),
    .q2(w_level4_shift_register_sipo_ic_inst_1_q2),
    .q3(w_level4_shift_register_sipo_ic_inst_1_q3)
);

// Writing output data. //
assign led4_1 = w_level4_shift_register_sipo_ic_inst_1_q0;
assign led5_1 = w_level4_shift_register_sipo_ic_inst_1_q1;
assign led6_1 = w_level4_shift_register_sipo_ic_inst_1_q2;
assign led7_1 = w_level4_shift_register_sipo_ic_inst_1_q3;
endmodule
