// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL1_D_LATCH (generated from level1_d_latch.panda)
module level1_d_latch_ic (
    input d,
    input enable,
    output q,
    output q_bar
);

/* verilator lint_off UNOPTFLAT */ // intentional latch feedback
wire aux_not_1;
wire aux_and_2;
wire aux_and_3;
reg aux_nor_4 = 1'b0;
reg aux_nor_5 = 1'b0;

// Internal logic
assign aux_not_1 = ~d;
assign aux_and_2 = (d & enable);
assign aux_and_3 = (aux_not_1 & enable);
always @(*) aux_nor_4 = ~(aux_and_3 | aux_nor_5);
always @(*) aux_nor_5 = ~(aux_and_2 | aux_nor_4);
/* verilator lint_on UNOPTFLAT */

assign q = aux_nor_4;
assign q_bar = aux_nor_5;
endmodule

module level1_d_latch (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,

/* ========= Outputs ========== */
output led4_1,
output led5_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL1_D_LATCH (level1_d_latch_ic)
wire w_level1_d_latch_ic_inst_1_q;
wire w_level1_d_latch_ic_inst_1_q_bar;


// Assigning aux variables. //
level1_d_latch_ic level1_d_latch_ic_inst_1 (
    .d(input_switch1),
    .enable(input_switch2),
    .q(w_level1_d_latch_ic_inst_1_q),
    .q_bar(w_level1_d_latch_ic_inst_1_q_bar)
);

// Writing output data. //
assign led4_1 = w_level1_d_latch_ic_inst_1_q;
assign led5_1 = w_level1_d_latch_ic_inst_1_q_bar;
endmodule
