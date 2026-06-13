// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL1_JK_FLIP_FLOP (generated from level1_jk_flip_flop.panda)
module level1_jk_flip_flop_ic (
    input j,
    input k,
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
reg aux_or_4 = 1'b0;
reg aux_or_5 = 1'b0;
reg aux_or_6 = 1'b0;
reg aux_or_7 = 1'b0;
reg aux_and_8 = 1'b0;
reg aux_and_9 = 1'b0;
reg aux_and_10 = 1'b0;
reg aux_and_11 = 1'b0;
reg aux_nor_12 = 1'b0;
reg aux_nor_13 = 1'b0;
reg aux_and_14 = 1'b0;
reg aux_and_15 = 1'b0;
reg aux_nor_16 = 1'b0;
reg aux_nor_17 = 1'b0;

// Internal logic
assign aux_not_1 = ~clock;
assign aux_not_2 = ~preset;
assign aux_not_3 = ~clear;
always @(*) aux_or_4 = (aux_and_15 | aux_not_3);
always @(*) aux_or_5 = (aux_and_14 | aux_not_2);
always @(*) aux_or_6 = (aux_and_10 | aux_not_2);
always @(*) aux_or_7 = (aux_and_11 | aux_not_3);
always @(*) aux_and_8 = (j & aux_nor_17);
always @(*) aux_and_9 = (k & aux_nor_16);
always @(*) aux_and_10 = (aux_and_8 & clock);
always @(*) aux_and_11 = (aux_and_9 & clock);
always @(*) aux_nor_12 = ~(aux_or_7 | aux_nor_13);
always @(*) aux_nor_13 = ~(aux_or_6 | aux_nor_12);
always @(*) aux_and_14 = (aux_nor_12 & aux_not_1);
always @(*) aux_and_15 = (aux_nor_13 & aux_not_1);
always @(*) aux_nor_16 = ~(aux_or_4 | aux_nor_17);
always @(*) aux_nor_17 = ~(aux_or_5 | aux_nor_16);
/* verilator lint_on UNOPTFLAT */

assign q = aux_nor_16;
assign q_bar = aux_nor_17;
endmodule

module level1_jk_flip_flop (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,

/* ========= Outputs ========== */
output led7_1,
output led8_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL1_JK_FLIP_FLOP (level1_jk_flip_flop_ic)
wire w_level1_jk_flip_flop_ic_inst_1_q;
wire w_level1_jk_flip_flop_ic_inst_1_q_bar;


// Assigning aux variables. //
level1_jk_flip_flop_ic level1_jk_flip_flop_ic_inst_1 (
    .j(input_switch1),
    .k(input_switch2),
    .clock(input_switch3),
    .preset(input_switch4),
    .clear(input_switch5),
    .q(w_level1_jk_flip_flop_ic_inst_1_q),
    .q_bar(w_level1_jk_flip_flop_ic_inst_1_q_bar)
);

// Writing output data. //
assign led7_1 = w_level1_jk_flip_flop_ic_inst_1_q;
assign led8_1 = w_level1_jk_flip_flop_ic_inst_1_q_bar;
endmodule
