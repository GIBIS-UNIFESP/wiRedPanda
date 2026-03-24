// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Behavioral module for level1_d_flip_flop_ic (generated from level1_d_flip_flop.panda)
module level1_d_flip_flop_ic (
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
