// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for FF0 (generated from level1_d_flip_flop.panda)
module level1_d_flip_flop (
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

// Module for LEVEL5_UP_DOWN_COUNTER_4BIT (generated from level5_up_down_counter_4bit.panda)
module level5_up_down_counter_4bit_ic (
    input clk,
    input direction,
    input enable,
    output q0,
    output q1,
    output q2,
    output q3
);

wire aux_not_3;
wire aux_not_4;
wire aux_not_5;
wire aux_not_6;
wire aux_and_7;
wire aux_and_8;
wire aux_not_9;
wire aux_not_10;
wire aux_not_11;
wire aux_and_12;
wire aux_and_13;
wire aux_and_14;
wire aux_and_15;
wire aux_and_16;
wire aux_and_17;
wire aux_and_18;
wire aux_and_19;
wire aux_or_20;
wire aux_or_21;
wire aux_or_22;
wire aux_or_23;
wire aux_and_24;
wire aux_and_25;
wire aux_not_26;
wire aux_not_27;
wire aux_not_28;
wire aux_and_29;
wire aux_and_30;
wire aux_and_31;
wire aux_and_32;
wire aux_and_33;
wire aux_and_34;
wire aux_and_35;
wire aux_and_36;
wire aux_or_37;
wire aux_or_38;
wire aux_or_39;
wire aux_or_40;
reg aux_mux_41 = 1'b0;
reg aux_mux_42 = 1'b0;
reg aux_mux_43 = 1'b0;
reg aux_mux_44 = 1'b0;
reg aux_mux_45 = 1'b0;
reg aux_mux_46 = 1'b0;
reg aux_mux_47 = 1'b0;
reg aux_mux_48 = 1'b0;
// IC instance: FF0 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_49_q;
wire w_level1_d_flip_flop_inst_49_q_bar;
// IC instance: FF1 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_50_q;
wire w_level1_d_flip_flop_inst_50_q_bar;
// IC instance: FF2 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_51_q;
wire w_level1_d_flip_flop_inst_51_q_bar;
// IC instance: FF3 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_52_q;
wire w_level1_d_flip_flop_inst_52_q_bar;

// Internal logic
assign aux_not_3 = ~w_level1_d_flip_flop_inst_49_q;
assign aux_not_4 = ~w_level1_d_flip_flop_inst_50_q;
assign aux_not_5 = ~w_level1_d_flip_flop_inst_51_q;
assign aux_not_6 = ~w_level1_d_flip_flop_inst_52_q;
assign aux_and_7 = (w_level1_d_flip_flop_inst_49_q & w_level1_d_flip_flop_inst_50_q);
assign aux_and_8 = (aux_and_7 & w_level1_d_flip_flop_inst_51_q);
assign aux_not_9 = ~w_level1_d_flip_flop_inst_49_q;
assign aux_not_10 = ~aux_and_7;
assign aux_not_11 = ~aux_and_8;
assign aux_and_12 = (aux_not_3 & 1'b1);
assign aux_and_13 = (w_level1_d_flip_flop_inst_49_q & aux_not_9);
assign aux_and_14 = (aux_not_4 & w_level1_d_flip_flop_inst_49_q);
assign aux_and_15 = (w_level1_d_flip_flop_inst_50_q & aux_not_9);
assign aux_and_16 = (aux_not_5 & aux_and_7);
assign aux_and_17 = (w_level1_d_flip_flop_inst_51_q & aux_not_10);
assign aux_and_18 = (aux_not_6 & aux_and_8);
assign aux_and_19 = (w_level1_d_flip_flop_inst_52_q & aux_not_11);
assign aux_or_20 = (aux_and_12 | aux_and_13);
assign aux_or_21 = (aux_and_14 | aux_and_15);
assign aux_or_22 = (aux_and_16 | aux_and_17);
assign aux_or_23 = (aux_and_18 | aux_and_19);
assign aux_and_24 = (aux_not_3 & aux_not_4);
assign aux_and_25 = (aux_and_24 & aux_not_5);
assign aux_not_26 = ~aux_not_3;
assign aux_not_27 = ~aux_and_24;
assign aux_not_28 = ~aux_and_25;
assign aux_and_29 = (aux_not_3 & 1'b1);
assign aux_and_30 = (w_level1_d_flip_flop_inst_49_q & aux_not_3);
assign aux_and_31 = (aux_not_4 & aux_not_3);
assign aux_and_32 = (w_level1_d_flip_flop_inst_50_q & aux_not_26);
assign aux_and_33 = (aux_not_5 & aux_and_24);
assign aux_and_34 = (w_level1_d_flip_flop_inst_51_q & aux_not_27);
assign aux_and_35 = (aux_not_6 & aux_and_25);
assign aux_and_36 = (w_level1_d_flip_flop_inst_52_q & aux_not_28);
assign aux_or_37 = (aux_and_29 | aux_and_30);
assign aux_or_38 = (aux_and_31 | aux_and_32);
assign aux_or_39 = (aux_and_33 | aux_and_34);
assign aux_or_40 = (aux_and_35 | aux_and_36);
    //Multiplexer
    always @(*)
    begin
        case({direction})
            1'd0: aux_mux_41 = aux_or_37;
            1'd1: aux_mux_41 = aux_or_20;
            default: aux_mux_41 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({direction})
            1'd0: aux_mux_42 = aux_or_38;
            1'd1: aux_mux_42 = aux_or_21;
            default: aux_mux_42 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({direction})
            1'd0: aux_mux_43 = aux_or_39;
            1'd1: aux_mux_43 = aux_or_22;
            default: aux_mux_43 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({direction})
            1'd0: aux_mux_44 = aux_or_40;
            1'd1: aux_mux_44 = aux_or_23;
            default: aux_mux_44 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({enable})
            1'd0: aux_mux_45 = w_level1_d_flip_flop_inst_49_q;
            1'd1: aux_mux_45 = aux_mux_41;
            default: aux_mux_45 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({enable})
            1'd0: aux_mux_46 = w_level1_d_flip_flop_inst_50_q;
            1'd1: aux_mux_46 = aux_mux_42;
            default: aux_mux_46 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({enable})
            1'd0: aux_mux_47 = w_level1_d_flip_flop_inst_51_q;
            1'd1: aux_mux_47 = aux_mux_43;
            default: aux_mux_47 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({enable})
            1'd0: aux_mux_48 = w_level1_d_flip_flop_inst_52_q;
            1'd1: aux_mux_48 = aux_mux_44;
            default: aux_mux_48 = 1'b0;
        endcase
    end
    //End of Multiplexer
level1_d_flip_flop level1_d_flip_flop_inst_49 (
    .d(aux_mux_45),
    .clock(clk),
    .preset(1'b0),
    .clear(1'b0),
    .q(w_level1_d_flip_flop_inst_49_q),
    .q_bar(w_level1_d_flip_flop_inst_49_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_50 (
    .d(aux_mux_46),
    .clock(clk),
    .preset(1'b0),
    .clear(1'b0),
    .q(w_level1_d_flip_flop_inst_50_q),
    .q_bar(w_level1_d_flip_flop_inst_50_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_51 (
    .d(aux_mux_47),
    .clock(clk),
    .preset(1'b0),
    .clear(1'b0),
    .q(w_level1_d_flip_flop_inst_51_q),
    .q_bar(w_level1_d_flip_flop_inst_51_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_52 (
    .d(aux_mux_48),
    .clock(clk),
    .preset(1'b0),
    .clear(1'b0),
    .q(w_level1_d_flip_flop_inst_52_q),
    .q_bar(w_level1_d_flip_flop_inst_52_q_bar)
);

assign q0 = w_level1_d_flip_flop_inst_49_q;
assign q1 = w_level1_d_flip_flop_inst_50_q;
assign q2 = w_level1_d_flip_flop_inst_51_q;
assign q3 = w_level1_d_flip_flop_inst_52_q;
endmodule

module level5_up_down_counter_4bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,

/* ========= Outputs ========== */
output led5_1,
output led6_1,
output led7_1,
output led8_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL5_UP_DOWN_COUNTER_4BIT (level5_up_down_counter_4bit_ic)
wire w_level5_up_down_counter_4bit_ic_inst_1_q0;
wire w_level5_up_down_counter_4bit_ic_inst_1_q1;
wire w_level5_up_down_counter_4bit_ic_inst_1_q2;
wire w_level5_up_down_counter_4bit_ic_inst_1_q3;


// Assigning aux variables. //
level5_up_down_counter_4bit_ic level5_up_down_counter_4bit_ic_inst_1 (
    .clk(input_switch1),
    .direction(input_switch2),
    .enable(input_switch3),
    .q0(w_level5_up_down_counter_4bit_ic_inst_1_q0),
    .q1(w_level5_up_down_counter_4bit_ic_inst_1_q1),
    .q2(w_level5_up_down_counter_4bit_ic_inst_1_q2),
    .q3(w_level5_up_down_counter_4bit_ic_inst_1_q3)
);

// Writing output data. //
assign led5_1 = w_level5_up_down_counter_4bit_ic_inst_1_q0;
assign led6_1 = w_level5_up_down_counter_4bit_ic_inst_1_q1;
assign led7_1 = w_level5_up_down_counter_4bit_ic_inst_1_q2;
assign led8_1 = w_level5_up_down_counter_4bit_ic_inst_1_q3;
endmodule
