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
wire aux_and_23;
wire aux_and_24;
wire aux_not_25;
wire aux_not_26;
wire aux_not_27;
wire aux_and_28;
wire aux_and_29;
wire aux_and_30;
wire aux_and_31;
wire aux_and_32;
wire aux_and_33;
wire aux_and_34;
wire aux_and_35;
wire aux_or_36;
wire aux_or_37;
wire aux_or_38;
wire aux_or_39;
reg aux_mux_40 = 1'b0;
reg aux_mux_41 = 1'b0;
reg aux_mux_42 = 1'b0;
reg aux_mux_43 = 1'b0;
reg aux_mux_44 = 1'b0;
reg aux_mux_45 = 1'b0;
reg aux_mux_46 = 1'b0;
reg aux_mux_47 = 1'b0;
// IC instance: FF0 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_48_q;
wire w_level1_d_flip_flop_inst_48_q_bar;
// IC instance: FF1 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_49_q;
wire w_level1_d_flip_flop_inst_49_q_bar;
// IC instance: FF2 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_50_q;
wire w_level1_d_flip_flop_inst_50_q_bar;
// IC instance: FF3 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_51_q;
wire w_level1_d_flip_flop_inst_51_q_bar;

// Internal logic
assign aux_not_2 = ~w_level1_d_flip_flop_inst_48_q;
assign aux_not_3 = ~w_level1_d_flip_flop_inst_49_q;
assign aux_not_4 = ~w_level1_d_flip_flop_inst_50_q;
assign aux_not_5 = ~w_level1_d_flip_flop_inst_51_q;
assign aux_and_6 = (w_level1_d_flip_flop_inst_48_q & w_level1_d_flip_flop_inst_49_q);
assign aux_and_7 = (aux_and_6 & w_level1_d_flip_flop_inst_50_q);
assign aux_not_8 = ~w_level1_d_flip_flop_inst_48_q;
assign aux_not_9 = ~aux_and_6;
assign aux_not_10 = ~aux_and_7;
assign aux_and_11 = (aux_not_2 & 1'b1);
assign aux_and_12 = (w_level1_d_flip_flop_inst_48_q & aux_not_8);
assign aux_and_13 = (aux_not_3 & w_level1_d_flip_flop_inst_48_q);
assign aux_and_14 = (w_level1_d_flip_flop_inst_49_q & aux_not_8);
assign aux_and_15 = (aux_not_4 & aux_and_6);
assign aux_and_16 = (w_level1_d_flip_flop_inst_50_q & aux_not_9);
assign aux_and_17 = (aux_not_5 & aux_and_7);
assign aux_and_18 = (w_level1_d_flip_flop_inst_51_q & aux_not_10);
assign aux_or_19 = (aux_and_11 | aux_and_12);
assign aux_or_20 = (aux_and_13 | aux_and_14);
assign aux_or_21 = (aux_and_15 | aux_and_16);
assign aux_or_22 = (aux_and_17 | aux_and_18);
assign aux_and_23 = (aux_not_2 & aux_not_3);
assign aux_and_24 = (aux_and_23 & aux_not_4);
assign aux_not_25 = ~aux_not_2;
assign aux_not_26 = ~aux_and_23;
assign aux_not_27 = ~aux_and_24;
assign aux_and_28 = (aux_not_2 & 1'b1);
assign aux_and_29 = (w_level1_d_flip_flop_inst_48_q & aux_not_2);
assign aux_and_30 = (aux_not_3 & aux_not_2);
assign aux_and_31 = (w_level1_d_flip_flop_inst_49_q & aux_not_25);
assign aux_and_32 = (aux_not_4 & aux_and_23);
assign aux_and_33 = (w_level1_d_flip_flop_inst_50_q & aux_not_26);
assign aux_and_34 = (aux_not_5 & aux_and_24);
assign aux_and_35 = (w_level1_d_flip_flop_inst_51_q & aux_not_27);
assign aux_or_36 = (aux_and_28 | aux_and_29);
assign aux_or_37 = (aux_and_30 | aux_and_31);
assign aux_or_38 = (aux_and_32 | aux_and_33);
assign aux_or_39 = (aux_and_34 | aux_and_35);
    //Multiplexer
    always @(*)
    begin
        case({direction})
            1'd0: aux_mux_40 = aux_or_36;
            1'd1: aux_mux_40 = aux_or_19;
            default: aux_mux_40 = 1'b0;
        endcase
    end
    //End of Multiplexer
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
        case({enable})
            1'd0: aux_mux_44 = w_level1_d_flip_flop_inst_48_q;
            1'd1: aux_mux_44 = aux_mux_40;
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
level1_d_flip_flop level1_d_flip_flop_inst_48 (
    .d(aux_mux_44),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_48_q),
    .q_bar(w_level1_d_flip_flop_inst_48_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_49 (
    .d(aux_mux_45),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_49_q),
    .q_bar(w_level1_d_flip_flop_inst_49_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_50 (
    .d(aux_mux_46),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_50_q),
    .q_bar(w_level1_d_flip_flop_inst_50_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_51 (
    .d(aux_mux_47),
    .clock(clk),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_51_q),
    .q_bar(w_level1_d_flip_flop_inst_51_q_bar)
);

assign q0 = w_level1_d_flip_flop_inst_48_q;
assign q1 = w_level1_d_flip_flop_inst_49_q;
assign q2 = w_level1_d_flip_flop_inst_50_q;
assign q3 = w_level1_d_flip_flop_inst_51_q;
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
