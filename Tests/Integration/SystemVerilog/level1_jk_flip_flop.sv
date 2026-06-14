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
reg aux_or_2 = 1'b0;
reg aux_or_3 = 1'b0;
reg aux_or_4 = 1'b0;
reg aux_or_5 = 1'b0;
reg aux_and_6 = 1'b0;
reg aux_and_7 = 1'b0;
reg aux_and_8 = 1'b0;
reg aux_and_9 = 1'b0;
reg aux_nor_10 = 1'b0;
reg aux_nor_11 = 1'b0;
reg aux_and_12 = 1'b0;
reg aux_and_13 = 1'b0;
reg aux_nor_14 = 1'b0;
reg aux_nor_15 = 1'b0;

// Internal logic
assign aux_not_1 = ~clock;
always @(*) aux_or_2 = (aux_and_13 | clear);
always @(*) aux_or_3 = (aux_and_12 | preset);
always @(*) aux_or_4 = (aux_and_8 | preset);
always @(*) aux_or_5 = (aux_and_9 | clear);
always @(*) aux_and_6 = (j & aux_nor_15);
always @(*) aux_and_7 = (k & aux_nor_14);
always @(*) aux_and_8 = (aux_and_6 & clock);
always @(*) aux_and_9 = (aux_and_7 & clock);
always @(*) aux_nor_10 = ~(aux_or_5 | aux_nor_11);
always @(*) aux_nor_11 = ~(aux_or_4 | aux_nor_10);
always @(*) aux_and_12 = (aux_nor_10 & aux_not_1);
always @(*) aux_and_13 = (aux_nor_11 & aux_not_1);
always @(*) aux_nor_14 = ~(aux_or_2 | aux_nor_15);
always @(*) aux_nor_15 = ~(aux_or_3 | aux_nor_14);
/* verilator lint_on UNOPTFLAT */

assign q = aux_nor_14;
assign q_bar = aux_nor_15;
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
