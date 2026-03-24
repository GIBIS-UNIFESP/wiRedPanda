// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Behavioral module for level1_d_latch_ic (generated from level1_d_latch.panda)
module level1_d_latch_ic (
    input d,
    input enable,
    output reg q,
    output reg q_bar
);
    initial begin
        q = 1'b1;
        q_bar = 1'b0;
    end
    always_latch
    begin
        if (enable)
        begin
            q = d;
            q_bar = ~d;
        end
    end
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
