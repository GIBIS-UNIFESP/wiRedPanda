// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Behavioral module for level1_jk_flip_flop_ic (generated from level1_jk_flip_flop.panda)
module level1_jk_flip_flop_ic (
    input j,
    input k,
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
            if (j && k)
            begin
                q <= q_bar;
                q_bar <= q;
            end
            else if (j && ~k)
            begin
                q <= 1'b1;
                q_bar <= 1'b0;
            end
            else if (~j && k)
            begin
                q <= 1'b0;
                q_bar <= 1'b1;
            end
        end
    end
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
