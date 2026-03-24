// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Behavioral module for level1_sr_latch_ic (generated from level1_sr_latch.panda)
module level1_sr_latch_ic (
    input s,
    input r,
    output reg q,
    output reg q_bar
);
    initial begin
        q = 1'b1;
        q_bar = 1'b0;
    end
    always_latch
    begin
        if (s && r)
        begin
            q = 1'b0;
            q_bar = 1'b0;
        end
        else if (s != r)
        begin
            q = s;
            q_bar = r;
        end
    end
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
