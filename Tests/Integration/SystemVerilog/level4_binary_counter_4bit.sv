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

// Module for LEVEL4_BINARY_COUNTER_4BIT (generated from level4_binary_counter_4bit.panda)
module level4_binary_counter_4bit_ic (
    input clk,
    output q0,
    output q1,
    output q2,
    output q3
);

wire aux_not_2;
wire aux_not_3;
wire aux_not_4;
wire aux_not_5;
wire aux_and_6;
wire aux_and_7;
wire aux_not_8;
wire aux_not_9;
wire aux_not_10;
wire aux_and_11;
wire aux_and_12;
wire aux_and_13;
wire aux_and_14;
wire aux_and_15;
wire aux_and_16;
wire aux_and_17;
wire aux_and_18;
wire aux_or_19;
wire aux_or_20;
wire aux_or_21;
wire aux_or_22;
// IC instance: FF0 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_23_q;
wire w_level1_d_flip_flop_inst_23_q_bar;
// IC instance: FF1 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_24_q;
wire w_level1_d_flip_flop_inst_24_q_bar;
// IC instance: FF2 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_25_q;
wire w_level1_d_flip_flop_inst_25_q_bar;
// IC instance: FF3 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_26_q;
wire w_level1_d_flip_flop_inst_26_q_bar;

// Internal logic
assign aux_not_2 = ~w_level1_d_flip_flop_inst_23_q;
assign aux_not_3 = ~w_level1_d_flip_flop_inst_24_q;
assign aux_not_4 = ~w_level1_d_flip_flop_inst_25_q;
assign aux_not_5 = ~w_level1_d_flip_flop_inst_26_q;
assign aux_and_6 = (w_level1_d_flip_flop_inst_23_q & w_level1_d_flip_flop_inst_24_q);
assign aux_and_7 = (aux_and_6 & w_level1_d_flip_flop_inst_25_q);
assign aux_not_8 = ~w_level1_d_flip_flop_inst_23_q;
assign aux_not_9 = ~aux_and_6;
assign aux_not_10 = ~aux_and_7;
assign aux_and_11 = (1'b0 & 1'b0);
assign aux_and_12 = (1'b0 & 1'b0);
assign aux_and_13 = (aux_not_3 & w_level1_d_flip_flop_inst_23_q);
assign aux_and_14 = (w_level1_d_flip_flop_inst_24_q & aux_not_8);
assign aux_and_15 = (aux_not_4 & aux_and_6);
assign aux_and_16 = (w_level1_d_flip_flop_inst_25_q & aux_not_9);
assign aux_and_17 = (aux_not_5 & aux_and_7);
assign aux_and_18 = (w_level1_d_flip_flop_inst_26_q & aux_not_10);
assign aux_or_19 = (1'b0 | 1'b0);
assign aux_or_20 = (aux_and_13 | aux_and_14);
assign aux_or_21 = (aux_and_15 | aux_and_16);
assign aux_or_22 = (aux_and_17 | aux_and_18);
level1_d_flip_flop level1_d_flip_flop_inst_23 (
    .d(aux_not_2),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_23_q),
    .q_bar(w_level1_d_flip_flop_inst_23_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_24 (
    .d(aux_or_20),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_24_q),
    .q_bar(w_level1_d_flip_flop_inst_24_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_25 (
    .d(aux_or_21),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_25_q),
    .q_bar(w_level1_d_flip_flop_inst_25_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_26 (
    .d(aux_or_22),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_26_q),
    .q_bar(w_level1_d_flip_flop_inst_26_q_bar)
);

assign q0 = w_level1_d_flip_flop_inst_23_q;
assign q1 = w_level1_d_flip_flop_inst_24_q;
assign q2 = w_level1_d_flip_flop_inst_25_q;
assign q3 = w_level1_d_flip_flop_inst_26_q;
endmodule

module level4_binary_counter_4bit (
/* ========= Inputs ========== */
input input_switch1,

/* ========= Outputs ========== */
output led3_1,
output led4_1,
output led5_1,
output led6_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_BINARY_COUNTER_4BIT (level4_binary_counter_4bit_ic)
wire w_level4_binary_counter_4bit_ic_inst_1_q0;
wire w_level4_binary_counter_4bit_ic_inst_1_q1;
wire w_level4_binary_counter_4bit_ic_inst_1_q2;
wire w_level4_binary_counter_4bit_ic_inst_1_q3;


// Assigning aux variables. //
level4_binary_counter_4bit_ic level4_binary_counter_4bit_ic_inst_1 (
    .clk(input_switch1),
    .q0(w_level4_binary_counter_4bit_ic_inst_1_q0),
    .q1(w_level4_binary_counter_4bit_ic_inst_1_q1),
    .q2(w_level4_binary_counter_4bit_ic_inst_1_q2),
    .q3(w_level4_binary_counter_4bit_ic_inst_1_q3)
);

// Writing output data. //
assign led3_1 = w_level4_binary_counter_4bit_ic_inst_1_q0;
assign led4_1 = w_level4_binary_counter_4bit_ic_inst_1_q1;
assign led5_1 = w_level4_binary_counter_4bit_ic_inst_1_q2;
assign led6_1 = w_level4_binary_counter_4bit_ic_inst_1_q3;
endmodule
