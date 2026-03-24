// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Behavioral module for level1_d_flip_flop (generated from level1_d_flip_flop.panda)
module level1_d_flip_flop (
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
