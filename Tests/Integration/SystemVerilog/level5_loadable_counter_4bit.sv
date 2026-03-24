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

// Module for LEVEL5_LOADABLE_COUNTER_4BIT (generated from level5_loadable_counter_4bit.panda)
module level5_loadable_counter_4bit_ic (
    input clk,
    input load,
    input d0,
    input d1,
    input d2,
    input d3,
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
reg aux_mux_23 = 1'b0;
reg aux_mux_24 = 1'b0;
reg aux_mux_25 = 1'b0;
reg aux_mux_26 = 1'b0;
// IC instance: FF0 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_27_q;
wire w_level1_d_flip_flop_inst_27_q_bar;
// IC instance: FF1 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_28_q;
wire w_level1_d_flip_flop_inst_28_q_bar;
// IC instance: FF2 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_29_q;
wire w_level1_d_flip_flop_inst_29_q_bar;
// IC instance: FF3 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_30_q;
wire w_level1_d_flip_flop_inst_30_q_bar;

// Internal logic
assign aux_not_2 = ~w_level1_d_flip_flop_inst_27_q;
assign aux_not_3 = ~w_level1_d_flip_flop_inst_28_q;
assign aux_not_4 = ~w_level1_d_flip_flop_inst_29_q;
assign aux_not_5 = ~w_level1_d_flip_flop_inst_30_q;
assign aux_and_6 = (w_level1_d_flip_flop_inst_27_q & w_level1_d_flip_flop_inst_28_q);
assign aux_and_7 = (aux_and_6 & w_level1_d_flip_flop_inst_29_q);
assign aux_not_8 = ~w_level1_d_flip_flop_inst_27_q;
assign aux_not_9 = ~aux_and_6;
assign aux_not_10 = ~aux_and_7;
assign aux_and_11 = (aux_not_2 & 1'b1);
assign aux_and_12 = (w_level1_d_flip_flop_inst_27_q & aux_not_8);
assign aux_and_13 = (aux_not_3 & w_level1_d_flip_flop_inst_27_q);
assign aux_and_14 = (w_level1_d_flip_flop_inst_28_q & aux_not_8);
assign aux_and_15 = (aux_not_4 & aux_and_6);
assign aux_and_16 = (w_level1_d_flip_flop_inst_29_q & aux_not_9);
assign aux_and_17 = (aux_not_5 & aux_and_7);
assign aux_and_18 = (w_level1_d_flip_flop_inst_30_q & aux_not_10);
assign aux_or_19 = (aux_and_11 | aux_and_12);
assign aux_or_20 = (aux_and_13 | aux_and_14);
assign aux_or_21 = (aux_and_15 | aux_and_16);
assign aux_or_22 = (aux_and_17 | aux_and_18);
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_23 = aux_or_19;
            1'd1: aux_mux_23 = d0;
            default: aux_mux_23 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_24 = aux_or_20;
            1'd1: aux_mux_24 = d1;
            default: aux_mux_24 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_25 = aux_or_21;
            1'd1: aux_mux_25 = d2;
            default: aux_mux_25 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_26 = aux_or_22;
            1'd1: aux_mux_26 = d3;
            default: aux_mux_26 = 1'b0;
        endcase
    end
    //End of Multiplexer
level1_d_flip_flop level1_d_flip_flop_inst_27 (
    .d(aux_mux_23),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_27_q),
    .q_bar(w_level1_d_flip_flop_inst_27_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_28 (
    .d(aux_mux_24),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_28_q),
    .q_bar(w_level1_d_flip_flop_inst_28_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_29 (
    .d(aux_mux_25),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_29_q),
    .q_bar(w_level1_d_flip_flop_inst_29_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_30 (
    .d(aux_mux_26),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_30_q),
    .q_bar(w_level1_d_flip_flop_inst_30_q_bar)
);

assign q0 = w_level1_d_flip_flop_inst_27_q;
assign q1 = w_level1_d_flip_flop_inst_28_q;
assign q2 = w_level1_d_flip_flop_inst_29_q;
assign q3 = w_level1_d_flip_flop_inst_30_q;
endmodule

module level5_loadable_counter_4bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,

/* ========= Outputs ========== */
output led8_1,
output led9_1,
output led10_1,
output led11_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL5_LOADABLE_COUNTER_4BIT (level5_loadable_counter_4bit_ic)
wire w_level5_loadable_counter_4bit_ic_inst_1_q0;
wire w_level5_loadable_counter_4bit_ic_inst_1_q1;
wire w_level5_loadable_counter_4bit_ic_inst_1_q2;
wire w_level5_loadable_counter_4bit_ic_inst_1_q3;


// Assigning aux variables. //
level5_loadable_counter_4bit_ic level5_loadable_counter_4bit_ic_inst_1 (
    .clk(input_switch1),
    .load(input_switch2),
    .d0(input_switch3),
    .d1(input_switch4),
    .d2(input_switch5),
    .d3(input_switch6),
    .q0(w_level5_loadable_counter_4bit_ic_inst_1_q0),
    .q1(w_level5_loadable_counter_4bit_ic_inst_1_q1),
    .q2(w_level5_loadable_counter_4bit_ic_inst_1_q2),
    .q3(w_level5_loadable_counter_4bit_ic_inst_1_q3)
);

// Writing output data. //
assign led8_1 = w_level5_loadable_counter_4bit_ic_inst_1_q0;
assign led9_1 = w_level5_loadable_counter_4bit_ic_inst_1_q1;
assign led10_1 = w_level5_loadable_counter_4bit_ic_inst_1_q2;
assign led11_1 = w_level5_loadable_counter_4bit_ic_inst_1_q3;
endmodule
