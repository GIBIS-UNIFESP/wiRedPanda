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

// Module for LEVEL7_FLAG_REGISTER (generated from level7_flag_register.panda)
module level7_flag_register_ic (
    input flagin0_zero,
    input flagin1_sign,
    input flagin2_carry,
    input flagin3_overflow,
    input load,
    input clock,
    output flagout0_zero,
    output flagout1_sign,
    output flagout2_carry,
    output flagout3_overflow
);

reg aux_mux_2 = 1'b0;
reg aux_mux_3 = 1'b0;
reg aux_mux_4 = 1'b0;
reg aux_mux_5 = 1'b0;
// IC instance: FlagFF0 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_6_q;
wire w_level1_d_flip_flop_inst_6_q_bar;
// IC instance: FlagFF1 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_7_q;
wire w_level1_d_flip_flop_inst_7_q_bar;
// IC instance: FlagFF2 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_8_q;
wire w_level1_d_flip_flop_inst_8_q_bar;
// IC instance: FlagFF3 (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_9_q;
wire w_level1_d_flip_flop_inst_9_q_bar;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_2 = w_level1_d_flip_flop_inst_6_q;
            1'd1: aux_mux_2 = flagin0_zero;
            default: aux_mux_2 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_3 = w_level1_d_flip_flop_inst_7_q;
            1'd1: aux_mux_3 = flagin1_sign;
            default: aux_mux_3 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_4 = w_level1_d_flip_flop_inst_8_q;
            1'd1: aux_mux_4 = flagin2_carry;
            default: aux_mux_4 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_5 = w_level1_d_flip_flop_inst_9_q;
            1'd1: aux_mux_5 = flagin3_overflow;
            default: aux_mux_5 = 1'b0;
        endcase
    end
    //End of Multiplexer
level1_d_flip_flop level1_d_flip_flop_inst_6 (
    .d(aux_mux_2),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_6_q),
    .q_bar(w_level1_d_flip_flop_inst_6_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_7 (
    .d(aux_mux_3),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_7_q),
    .q_bar(w_level1_d_flip_flop_inst_7_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_8 (
    .d(aux_mux_4),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_8_q),
    .q_bar(w_level1_d_flip_flop_inst_8_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_9 (
    .d(aux_mux_5),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_9_q),
    .q_bar(w_level1_d_flip_flop_inst_9_q_bar)
);

assign flagout0_zero = w_level1_d_flip_flop_inst_6_q;
assign flagout1_sign = w_level1_d_flip_flop_inst_7_q;
assign flagout2_carry = w_level1_d_flip_flop_inst_8_q;
assign flagout3_overflow = w_level1_d_flip_flop_inst_9_q;
endmodule

module level7_flag_register (
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
// IC instance: LEVEL7_FLAG_REGISTER (level7_flag_register_ic)
wire w_level7_flag_register_ic_inst_1_flagout0_zero;
wire w_level7_flag_register_ic_inst_1_flagout1_sign;
wire w_level7_flag_register_ic_inst_1_flagout2_carry;
wire w_level7_flag_register_ic_inst_1_flagout3_overflow;


// Assigning aux variables. //
level7_flag_register_ic level7_flag_register_ic_inst_1 (
    .flagin0_zero(input_switch1),
    .flagin1_sign(input_switch2),
    .flagin2_carry(input_switch3),
    .flagin3_overflow(input_switch4),
    .load(input_switch5),
    .clock(input_switch6),
    .flagout0_zero(w_level7_flag_register_ic_inst_1_flagout0_zero),
    .flagout1_sign(w_level7_flag_register_ic_inst_1_flagout1_sign),
    .flagout2_carry(w_level7_flag_register_ic_inst_1_flagout2_carry),
    .flagout3_overflow(w_level7_flag_register_ic_inst_1_flagout3_overflow)
);

// Writing output data. //
assign led8_1 = w_level7_flag_register_ic_inst_1_flagout0_zero;
assign led9_1 = w_level7_flag_register_ic_inst_1_flagout1_sign;
assign led10_1 = w_level7_flag_register_ic_inst_1_flagout2_carry;
assign led11_1 = w_level7_flag_register_ic_inst_1_flagout3_overflow;
endmodule
