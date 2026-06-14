// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL1_D_FLIP_FLOP (generated from level1_d_flip_flop.panda)
module level1_d_flip_flop_ic (
    input d,
    input clock,
    input preset,
    input clear,
    output q,
    output q_bar
);

/* verilator lint_off UNOPTFLAT */ // intentional latch feedback
wire aux_not_1;
wire aux_or_2;
wire aux_or_3;
wire aux_or_4;
wire aux_or_5;
wire aux_not_6;
wire aux_and_7;
wire aux_and_8;
reg aux_nor_9 = 1'b0;
reg aux_nor_10 = 1'b0;
wire aux_and_11;
wire aux_and_12;
reg aux_nor_13 = 1'b0;
reg aux_nor_14 = 1'b0;

// Internal logic
assign aux_not_1 = ~clock;
assign aux_or_2 = (aux_and_12 | clear);
assign aux_or_3 = (aux_and_11 | preset);
assign aux_or_4 = (aux_and_8 | clear);
assign aux_or_5 = (aux_and_7 | preset);
assign aux_not_6 = ~d;
assign aux_and_7 = (d & aux_not_1);
assign aux_and_8 = (aux_not_6 & aux_not_1);
always @(*) aux_nor_9 = ~(aux_or_4 | aux_nor_10);
always @(*) aux_nor_10 = ~(aux_or_5 | aux_nor_9);
assign aux_and_11 = (aux_nor_9 & clock);
assign aux_and_12 = (aux_nor_10 & clock);
always @(*) aux_nor_13 = ~(aux_or_2 | aux_nor_14);
always @(*) aux_nor_14 = ~(aux_or_3 | aux_nor_13);
/* verilator lint_on UNOPTFLAT */

assign q = aux_nor_13;
assign q_bar = aux_nor_14;
endmodule

module level1_d_flip_flop (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,

/* ========= Outputs ========== */
output led6_1,
output led7_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL1_D_FLIP_FLOP (level1_d_flip_flop_ic)
wire w_level1_d_flip_flop_ic_inst_1_q;
wire w_level1_d_flip_flop_ic_inst_1_q_bar;


// Assigning aux variables. //
level1_d_flip_flop_ic level1_d_flip_flop_ic_inst_1 (
    .d(input_switch1),
    .clock(input_switch2),
    .preset(input_switch3),
    .clear(input_switch4),
    .q(w_level1_d_flip_flop_ic_inst_1_q),
    .q_bar(w_level1_d_flip_flop_ic_inst_1_q_bar)
);

// Writing output data. //
assign led6_1 = w_level1_d_flip_flop_ic_inst_1_q;
assign led7_1 = w_level1_d_flip_flop_ic_inst_1_q_bar;
endmodule
