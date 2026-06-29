// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL1_SR_LATCH (generated from level1_sr_latch.panda)
module level1_sr_latch_ic (
    input s,
    input r,
    output q,
    output q_bar
);

/* verilator lint_off UNOPTFLAT */ // intentional latch feedback
reg aux_nor_1 = 1'b0;
reg aux_nor_2 = 1'b0;

// Internal logic
always @(*) aux_nor_1 = ~(r | aux_nor_2);
always @(*) aux_nor_2 = ~(s | aux_nor_1);

assign q = aux_nor_1;
assign q_bar = aux_nor_2;
/* verilator lint_on UNOPTFLAT */
endmodule

module level1_sr_latch (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,

/* ========= Outputs ========== */
output led4_1,
output led5_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL1_SR_LATCH (level1_sr_latch_ic)
wire w_level1_sr_latch_ic_inst_1_q;
wire w_level1_sr_latch_ic_inst_1_q_bar;


// Assigning aux variables. //
level1_sr_latch_ic level1_sr_latch_ic_inst_1 (
    .s(input_switch1),
    .r(input_switch2),
    .q(w_level1_sr_latch_ic_inst_1_q),
    .q_bar(w_level1_sr_latch_ic_inst_1_q_bar)
);

// Writing output data. //
assign led4_1 = w_level1_sr_latch_ic_inst_1_q;
assign led5_1 = w_level1_sr_latch_ic_inst_1_q_bar;
endmodule
