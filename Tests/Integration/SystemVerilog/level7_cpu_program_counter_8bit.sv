// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for FA[0] (generated from level2_full_adder_1bit.panda)
module level2_full_adder_1bit (
    input a,
    input b,
    input cin,
    output sum,
    output cout
);

wire aux_xor_1;
wire aux_and_2;
wire aux_xor_3;
wire aux_and_4;
wire aux_or_5;

// Internal logic
assign aux_xor_1 = (a ^ b);
assign aux_and_2 = (a & b);
assign aux_xor_3 = (aux_xor_1 ^ cin);
assign aux_and_4 = (aux_xor_1 & cin);
assign aux_or_5 = (aux_and_2 | aux_and_4);

assign sum = aux_xor_3;
assign cout = aux_or_5;
endmodule

// Module for Reg[0] (generated from level3_register_1bit.panda)
module level3_register_1bit (
    input data,
    input clock,
    input writeenable,
    input reset,
    output q,
    output notq
);

wire aux_not_1;
wire aux_not_2;
reg aux_mux_3 = 1'b0;
reg aux_d_flip_flop_4_0_q = 1'b0;
reg aux_d_flip_flop_4_1_q = 1'b1;

// Internal logic
assign aux_not_1 = ~reset;
assign aux_not_2 = ~writeenable;
    //Multiplexer
    always @(*)
    begin
        case({aux_not_2})
            1'd0: aux_mux_3 = data;
            1'd1: aux_mux_3 = aux_d_flip_flop_4_0_q;
            default: aux_mux_3 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_4_0_q <= 1'b0;
            aux_d_flip_flop_4_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_4_0_q <= aux_mux_3;
            aux_d_flip_flop_4_1_q <= ~aux_mux_3;
        end
    end
    //End of D FlipFlop

assign q = aux_d_flip_flop_4_0_q;
assign notq = aux_d_flip_flop_4_1_q;
endmodule

// Module for Register8bit (generated from level6_register_8bit.panda)
module level6_register_8bit (
    input data0,
    input clock,
    input data1,
    input writeenable,
    input data2,
    input reset,
    input data3,
    input data4,
    input data5,
    input data6,
    input data7,
    output q0,
    output q1,
    output q2,
    output q3,
    output q4,
    output q5,
    output q6,
    output q7
);

// IC instance: Reg[0] (level3_register_1bit)
wire w_level3_register_1bit_inst_1_q;
wire w_level3_register_1bit_inst_1_notq;
// IC instance: Reg[1] (level3_register_1bit)
wire w_level3_register_1bit_inst_2_q;
wire w_level3_register_1bit_inst_2_notq;
// IC instance: Reg[2] (level3_register_1bit)
wire w_level3_register_1bit_inst_3_q;
wire w_level3_register_1bit_inst_3_notq;
// IC instance: Reg[3] (level3_register_1bit)
wire w_level3_register_1bit_inst_4_q;
wire w_level3_register_1bit_inst_4_notq;
// IC instance: Reg[4] (level3_register_1bit)
wire w_level3_register_1bit_inst_5_q;
wire w_level3_register_1bit_inst_5_notq;
// IC instance: Reg[5] (level3_register_1bit)
wire w_level3_register_1bit_inst_6_q;
wire w_level3_register_1bit_inst_6_notq;
// IC instance: Reg[6] (level3_register_1bit)
wire w_level3_register_1bit_inst_7_q;
wire w_level3_register_1bit_inst_7_notq;
// IC instance: Reg[7] (level3_register_1bit)
wire w_level3_register_1bit_inst_8_q;
wire w_level3_register_1bit_inst_8_notq;

// Internal logic
level3_register_1bit level3_register_1bit_inst_1 (
    .data(data0),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_1_q),
    .notq(w_level3_register_1bit_inst_1_notq)
);
level3_register_1bit level3_register_1bit_inst_2 (
    .data(data1),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_2_q),
    .notq(w_level3_register_1bit_inst_2_notq)
);
level3_register_1bit level3_register_1bit_inst_3 (
    .data(data2),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_3_q),
    .notq(w_level3_register_1bit_inst_3_notq)
);
level3_register_1bit level3_register_1bit_inst_4 (
    .data(data3),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_4_q),
    .notq(w_level3_register_1bit_inst_4_notq)
);
level3_register_1bit level3_register_1bit_inst_5 (
    .data(data4),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_5_q),
    .notq(w_level3_register_1bit_inst_5_notq)
);
level3_register_1bit level3_register_1bit_inst_6 (
    .data(data5),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_6_q),
    .notq(w_level3_register_1bit_inst_6_notq)
);
level3_register_1bit level3_register_1bit_inst_7 (
    .data(data6),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_7_q),
    .notq(w_level3_register_1bit_inst_7_notq)
);
level3_register_1bit level3_register_1bit_inst_8 (
    .data(data7),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_8_q),
    .notq(w_level3_register_1bit_inst_8_notq)
);

assign q0 = w_level3_register_1bit_inst_1_q;
assign q1 = w_level3_register_1bit_inst_2_q;
assign q2 = w_level3_register_1bit_inst_3_q;
assign q3 = w_level3_register_1bit_inst_4_q;
assign q4 = w_level3_register_1bit_inst_5_q;
assign q5 = w_level3_register_1bit_inst_6_q;
assign q6 = w_level3_register_1bit_inst_7_q;
assign q7 = w_level3_register_1bit_inst_8_q;
endmodule

// Module for Adder8bit (generated from level6_ripple_adder_8bit.panda)
module level6_ripple_adder_8bit (
    input a0,
    input b0,
    input carryin,
    input a1,
    input b1,
    input a2,
    input b2,
    input a3,
    input b3,
    input a4,
    input b4,
    input a5,
    input b5,
    input a6,
    input b6,
    input a7,
    input b7,
    output sum0,
    output sum1,
    output sum2,
    output sum3,
    output sum4,
    output sum5,
    output sum6,
    output sum7,
    output carryout
);

// IC instance: FA[0] (level2_full_adder_1bit)
wire w_level2_full_adder_1bit_inst_1_sum;
wire w_level2_full_adder_1bit_inst_1_cout;
// IC instance: FA[1] (level2_full_adder_1bit)
wire w_level2_full_adder_1bit_inst_2_sum;
wire w_level2_full_adder_1bit_inst_2_cout;
// IC instance: FA[2] (level2_full_adder_1bit)
wire w_level2_full_adder_1bit_inst_3_sum;
wire w_level2_full_adder_1bit_inst_3_cout;
// IC instance: FA[3] (level2_full_adder_1bit)
wire w_level2_full_adder_1bit_inst_4_sum;
wire w_level2_full_adder_1bit_inst_4_cout;
// IC instance: FA[4] (level2_full_adder_1bit)
wire w_level2_full_adder_1bit_inst_5_sum;
wire w_level2_full_adder_1bit_inst_5_cout;
// IC instance: FA[5] (level2_full_adder_1bit)
wire w_level2_full_adder_1bit_inst_6_sum;
wire w_level2_full_adder_1bit_inst_6_cout;
// IC instance: FA[6] (level2_full_adder_1bit)
wire w_level2_full_adder_1bit_inst_7_sum;
wire w_level2_full_adder_1bit_inst_7_cout;
// IC instance: FA[7] (level2_full_adder_1bit)
wire w_level2_full_adder_1bit_inst_8_sum;
wire w_level2_full_adder_1bit_inst_8_cout;

// Internal logic
level2_full_adder_1bit level2_full_adder_1bit_inst_1 (
    .a(a0),
    .b(b0),
    .cin(carryin),
    .sum(w_level2_full_adder_1bit_inst_1_sum),
    .cout(w_level2_full_adder_1bit_inst_1_cout)
);
level2_full_adder_1bit level2_full_adder_1bit_inst_2 (
    .a(a1),
    .b(b1),
    .cin(w_level2_full_adder_1bit_inst_1_cout),
    .sum(w_level2_full_adder_1bit_inst_2_sum),
    .cout(w_level2_full_adder_1bit_inst_2_cout)
);
level2_full_adder_1bit level2_full_adder_1bit_inst_3 (
    .a(a2),
    .b(b2),
    .cin(w_level2_full_adder_1bit_inst_2_cout),
    .sum(w_level2_full_adder_1bit_inst_3_sum),
    .cout(w_level2_full_adder_1bit_inst_3_cout)
);
level2_full_adder_1bit level2_full_adder_1bit_inst_4 (
    .a(a3),
    .b(b3),
    .cin(w_level2_full_adder_1bit_inst_3_cout),
    .sum(w_level2_full_adder_1bit_inst_4_sum),
    .cout(w_level2_full_adder_1bit_inst_4_cout)
);
level2_full_adder_1bit level2_full_adder_1bit_inst_5 (
    .a(a4),
    .b(b4),
    .cin(w_level2_full_adder_1bit_inst_4_cout),
    .sum(w_level2_full_adder_1bit_inst_5_sum),
    .cout(w_level2_full_adder_1bit_inst_5_cout)
);
level2_full_adder_1bit level2_full_adder_1bit_inst_6 (
    .a(a5),
    .b(b5),
    .cin(w_level2_full_adder_1bit_inst_5_cout),
    .sum(w_level2_full_adder_1bit_inst_6_sum),
    .cout(w_level2_full_adder_1bit_inst_6_cout)
);
level2_full_adder_1bit level2_full_adder_1bit_inst_7 (
    .a(a6),
    .b(b6),
    .cin(w_level2_full_adder_1bit_inst_6_cout),
    .sum(w_level2_full_adder_1bit_inst_7_sum),
    .cout(w_level2_full_adder_1bit_inst_7_cout)
);
level2_full_adder_1bit level2_full_adder_1bit_inst_8 (
    .a(a7),
    .b(b7),
    .cin(w_level2_full_adder_1bit_inst_7_cout),
    .sum(w_level2_full_adder_1bit_inst_8_sum),
    .cout(w_level2_full_adder_1bit_inst_8_cout)
);

assign sum0 = w_level2_full_adder_1bit_inst_1_sum;
assign sum1 = w_level2_full_adder_1bit_inst_2_sum;
assign sum2 = w_level2_full_adder_1bit_inst_3_sum;
assign sum3 = w_level2_full_adder_1bit_inst_4_sum;
assign sum4 = w_level2_full_adder_1bit_inst_5_sum;
assign sum5 = w_level2_full_adder_1bit_inst_6_sum;
assign sum6 = w_level2_full_adder_1bit_inst_7_sum;
assign sum7 = w_level2_full_adder_1bit_inst_8_sum;
assign carryout = w_level2_full_adder_1bit_inst_8_cout;
endmodule

// Module for PC (generated from level6_program_counter_8bit_arithmetic.panda)
module level6_program_counter_8bit_arithmetic (
    input loadvalue0,
    input loadvalue1,
    input loadvalue2,
    input loadvalue3,
    input loadvalue4,
    input loadvalue5,
    input loadvalue6,
    input loadvalue7,
    input load,
    input inc,
    input reset,
    input clock,
    output pc0,
    output pc_plus_10,
    output pc1,
    output pc_plus_11,
    output pc2,
    output pc_plus_12,
    output pc3,
    output pc_plus_13,
    output pc4,
    output pc_plus_14,
    output pc5,
    output pc_plus_15,
    output pc6,
    output pc_plus_16,
    output pc7,
    output pc_plus_17
);

// IC instance: Register8bit (level6_register_8bit)
wire w_level6_register_8bit_inst_1_q0;
wire w_level6_register_8bit_inst_1_q1;
wire w_level6_register_8bit_inst_1_q2;
wire w_level6_register_8bit_inst_1_q3;
wire w_level6_register_8bit_inst_1_q4;
wire w_level6_register_8bit_inst_1_q5;
wire w_level6_register_8bit_inst_1_q6;
wire w_level6_register_8bit_inst_1_q7;
// IC instance: Adder8bit (level6_ripple_adder_8bit)
wire w_level6_ripple_adder_8bit_inst_2_sum0;
wire w_level6_ripple_adder_8bit_inst_2_sum1;
wire w_level6_ripple_adder_8bit_inst_2_sum2;
wire w_level6_ripple_adder_8bit_inst_2_sum3;
wire w_level6_ripple_adder_8bit_inst_2_sum4;
wire w_level6_ripple_adder_8bit_inst_2_sum5;
wire w_level6_ripple_adder_8bit_inst_2_sum6;
wire w_level6_ripple_adder_8bit_inst_2_sum7;
wire w_level6_ripple_adder_8bit_inst_2_carryout;
wire aux_not_4;
wire aux_and_5;
reg aux_mux_6 = 1'b0;
reg aux_mux_7 = 1'b0;
reg aux_mux_8 = 1'b0;
reg aux_mux_9 = 1'b0;
reg aux_mux_10 = 1'b0;
reg aux_mux_11 = 1'b0;
reg aux_mux_12 = 1'b0;
reg aux_mux_13 = 1'b0;
reg aux_mux_14 = 1'b0;
reg aux_mux_15 = 1'b0;
reg aux_mux_16 = 1'b0;
reg aux_mux_17 = 1'b0;
reg aux_mux_18 = 1'b0;
reg aux_mux_19 = 1'b0;
reg aux_mux_20 = 1'b0;
reg aux_mux_21 = 1'b0;
wire aux_or_22;

// Internal logic
level6_register_8bit level6_register_8bit_inst_1 (
    .data0(aux_mux_14),
    .clock(clock),
    .data1(aux_mux_15),
    .writeenable(aux_or_22),
    .data2(aux_mux_16),
    .reset(reset),
    .data3(aux_mux_17),
    .data4(aux_mux_18),
    .data5(aux_mux_19),
    .data6(aux_mux_20),
    .data7(aux_mux_21),
    .q0(w_level6_register_8bit_inst_1_q0),
    .q1(w_level6_register_8bit_inst_1_q1),
    .q2(w_level6_register_8bit_inst_1_q2),
    .q3(w_level6_register_8bit_inst_1_q3),
    .q4(w_level6_register_8bit_inst_1_q4),
    .q5(w_level6_register_8bit_inst_1_q5),
    .q6(w_level6_register_8bit_inst_1_q6),
    .q7(w_level6_register_8bit_inst_1_q7)
);
level6_ripple_adder_8bit level6_ripple_adder_8bit_inst_2 (
    .a0(w_level6_register_8bit_inst_1_q0),
    .b0(1'b1),
    .carryin(1'b0),
    .a1(w_level6_register_8bit_inst_1_q1),
    .b1(1'b0),
    .a2(w_level6_register_8bit_inst_1_q2),
    .b2(1'b0),
    .a3(w_level6_register_8bit_inst_1_q3),
    .b3(1'b0),
    .a4(w_level6_register_8bit_inst_1_q4),
    .b4(1'b0),
    .a5(w_level6_register_8bit_inst_1_q5),
    .b5(1'b0),
    .a6(w_level6_register_8bit_inst_1_q6),
    .b6(1'b0),
    .a7(w_level6_register_8bit_inst_1_q7),
    .b7(1'b0),
    .sum0(w_level6_ripple_adder_8bit_inst_2_sum0),
    .sum1(w_level6_ripple_adder_8bit_inst_2_sum1),
    .sum2(w_level6_ripple_adder_8bit_inst_2_sum2),
    .sum3(w_level6_ripple_adder_8bit_inst_2_sum3),
    .sum4(w_level6_ripple_adder_8bit_inst_2_sum4),
    .sum5(w_level6_ripple_adder_8bit_inst_2_sum5),
    .sum6(w_level6_ripple_adder_8bit_inst_2_sum6),
    .sum7(w_level6_ripple_adder_8bit_inst_2_sum7),
    .carryout(w_level6_ripple_adder_8bit_inst_2_carryout)
);
assign aux_not_4 = ~load;
assign aux_and_5 = (inc & aux_not_4);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_6 = w_level6_register_8bit_inst_1_q0;
            1'd1: aux_mux_6 = w_level6_ripple_adder_8bit_inst_2_sum0;
            default: aux_mux_6 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_7 = w_level6_register_8bit_inst_1_q1;
            1'd1: aux_mux_7 = w_level6_ripple_adder_8bit_inst_2_sum1;
            default: aux_mux_7 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_8 = w_level6_register_8bit_inst_1_q2;
            1'd1: aux_mux_8 = w_level6_ripple_adder_8bit_inst_2_sum2;
            default: aux_mux_8 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_9 = w_level6_register_8bit_inst_1_q3;
            1'd1: aux_mux_9 = w_level6_ripple_adder_8bit_inst_2_sum3;
            default: aux_mux_9 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_10 = w_level6_register_8bit_inst_1_q4;
            1'd1: aux_mux_10 = w_level6_ripple_adder_8bit_inst_2_sum4;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_11 = w_level6_register_8bit_inst_1_q5;
            1'd1: aux_mux_11 = w_level6_ripple_adder_8bit_inst_2_sum5;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_12 = w_level6_register_8bit_inst_1_q6;
            1'd1: aux_mux_12 = w_level6_ripple_adder_8bit_inst_2_sum6;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_13 = w_level6_register_8bit_inst_1_q7;
            1'd1: aux_mux_13 = w_level6_ripple_adder_8bit_inst_2_sum7;
            default: aux_mux_13 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_14 = aux_mux_6;
            1'd1: aux_mux_14 = loadvalue0;
            default: aux_mux_14 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_15 = aux_mux_7;
            1'd1: aux_mux_15 = loadvalue1;
            default: aux_mux_15 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_16 = aux_mux_8;
            1'd1: aux_mux_16 = loadvalue2;
            default: aux_mux_16 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_17 = aux_mux_9;
            1'd1: aux_mux_17 = loadvalue3;
            default: aux_mux_17 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_18 = aux_mux_10;
            1'd1: aux_mux_18 = loadvalue4;
            default: aux_mux_18 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_19 = aux_mux_11;
            1'd1: aux_mux_19 = loadvalue5;
            default: aux_mux_19 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_20 = aux_mux_12;
            1'd1: aux_mux_20 = loadvalue6;
            default: aux_mux_20 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_21 = aux_mux_13;
            1'd1: aux_mux_21 = loadvalue7;
            default: aux_mux_21 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_22 = (load | inc);

assign pc0 = w_level6_register_8bit_inst_1_q0;
assign pc_plus_10 = w_level6_ripple_adder_8bit_inst_2_sum0;
assign pc1 = w_level6_register_8bit_inst_1_q1;
assign pc_plus_11 = w_level6_ripple_adder_8bit_inst_2_sum1;
assign pc2 = w_level6_register_8bit_inst_1_q2;
assign pc_plus_12 = w_level6_ripple_adder_8bit_inst_2_sum2;
assign pc3 = w_level6_register_8bit_inst_1_q3;
assign pc_plus_13 = w_level6_ripple_adder_8bit_inst_2_sum3;
assign pc4 = w_level6_register_8bit_inst_1_q4;
assign pc_plus_14 = w_level6_ripple_adder_8bit_inst_2_sum4;
assign pc5 = w_level6_register_8bit_inst_1_q5;
assign pc_plus_15 = w_level6_ripple_adder_8bit_inst_2_sum5;
assign pc6 = w_level6_register_8bit_inst_1_q6;
assign pc_plus_16 = w_level6_ripple_adder_8bit_inst_2_sum6;
assign pc7 = w_level6_register_8bit_inst_1_q7;
assign pc_plus_17 = w_level6_ripple_adder_8bit_inst_2_sum7;
endmodule

// Module for LEVEL7_CPU_PROGRAM_COUNTER_8BIT (generated from level7_cpu_program_counter_8bit.panda)
module level7_cpu_program_counter_8bit_ic (
    input data0,
    input data1,
    input data2,
    input data3,
    input data4,
    input data5,
    input data6,
    input data7,
    input clock,
    input load,
    input inc,
    input reset,
    input writeenable,
    output address0,
    output address1,
    output address2,
    output address3,
    output address4,
    output address5,
    output address6,
    output address7
);

// IC instance: PC (level6_program_counter_8bit_arithmetic)
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc0;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_10;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc1;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_11;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc2;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_12;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc3;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_13;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc4;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_14;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc5;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_15;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc6;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_16;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc7;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_17;

// Internal logic
level6_program_counter_8bit_arithmetic level6_program_counter_8bit_arithmetic_inst_1 (
    .loadvalue0(data0),
    .loadvalue1(data1),
    .loadvalue2(data2),
    .loadvalue3(data3),
    .loadvalue4(data4),
    .loadvalue5(data5),
    .loadvalue6(data6),
    .loadvalue7(data7),
    .load(load),
    .inc(inc),
    .reset(reset),
    .clock(clock),
    .pc0(w_level6_program_counter_8bit_arithmetic_inst_1_pc0),
    .pc_plus_10(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_10),
    .pc1(w_level6_program_counter_8bit_arithmetic_inst_1_pc1),
    .pc_plus_11(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_11),
    .pc2(w_level6_program_counter_8bit_arithmetic_inst_1_pc2),
    .pc_plus_12(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_12),
    .pc3(w_level6_program_counter_8bit_arithmetic_inst_1_pc3),
    .pc_plus_13(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_13),
    .pc4(w_level6_program_counter_8bit_arithmetic_inst_1_pc4),
    .pc_plus_14(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_14),
    .pc5(w_level6_program_counter_8bit_arithmetic_inst_1_pc5),
    .pc_plus_15(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_15),
    .pc6(w_level6_program_counter_8bit_arithmetic_inst_1_pc6),
    .pc_plus_16(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_16),
    .pc7(w_level6_program_counter_8bit_arithmetic_inst_1_pc7),
    .pc_plus_17(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_17)
);

assign address0 = w_level6_program_counter_8bit_arithmetic_inst_1_pc0;
assign address1 = w_level6_program_counter_8bit_arithmetic_inst_1_pc1;
assign address2 = w_level6_program_counter_8bit_arithmetic_inst_1_pc2;
assign address3 = w_level6_program_counter_8bit_arithmetic_inst_1_pc3;
assign address4 = w_level6_program_counter_8bit_arithmetic_inst_1_pc4;
assign address5 = w_level6_program_counter_8bit_arithmetic_inst_1_pc5;
assign address6 = w_level6_program_counter_8bit_arithmetic_inst_1_pc6;
assign address7 = w_level6_program_counter_8bit_arithmetic_inst_1_pc7;
endmodule

module level7_cpu_program_counter_8bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,
input input_switch8,
input input_switch9,
input input_switch10,
input input_switch11,
input input_switch12,
input input_switch13,

/* ========= Outputs ========== */
output led15_1,
output led16_1,
output led17_1,
output led18_1,
output led19_1,
output led20_1,
output led21_1,
output led22_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL7_CPU_PROGRAM_COUNTER_8BIT (level7_cpu_program_counter_8bit_ic)
wire w_level7_cpu_program_counter_8bit_ic_inst_1_address0;
wire w_level7_cpu_program_counter_8bit_ic_inst_1_address1;
wire w_level7_cpu_program_counter_8bit_ic_inst_1_address2;
wire w_level7_cpu_program_counter_8bit_ic_inst_1_address3;
wire w_level7_cpu_program_counter_8bit_ic_inst_1_address4;
wire w_level7_cpu_program_counter_8bit_ic_inst_1_address5;
wire w_level7_cpu_program_counter_8bit_ic_inst_1_address6;
wire w_level7_cpu_program_counter_8bit_ic_inst_1_address7;


// Assigning aux variables. //
level7_cpu_program_counter_8bit_ic level7_cpu_program_counter_8bit_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .data2(input_switch3),
    .data3(input_switch4),
    .data4(input_switch5),
    .data5(input_switch6),
    .data6(input_switch7),
    .data7(input_switch8),
    .clock(input_switch9),
    .load(input_switch10),
    .inc(input_switch11),
    .reset(input_switch12),
    .writeenable(input_switch13),
    .address0(w_level7_cpu_program_counter_8bit_ic_inst_1_address0),
    .address1(w_level7_cpu_program_counter_8bit_ic_inst_1_address1),
    .address2(w_level7_cpu_program_counter_8bit_ic_inst_1_address2),
    .address3(w_level7_cpu_program_counter_8bit_ic_inst_1_address3),
    .address4(w_level7_cpu_program_counter_8bit_ic_inst_1_address4),
    .address5(w_level7_cpu_program_counter_8bit_ic_inst_1_address5),
    .address6(w_level7_cpu_program_counter_8bit_ic_inst_1_address6),
    .address7(w_level7_cpu_program_counter_8bit_ic_inst_1_address7)
);

// Writing output data. //
assign led15_1 = w_level7_cpu_program_counter_8bit_ic_inst_1_address0;
assign led16_1 = w_level7_cpu_program_counter_8bit_ic_inst_1_address1;
assign led17_1 = w_level7_cpu_program_counter_8bit_ic_inst_1_address2;
assign led18_1 = w_level7_cpu_program_counter_8bit_ic_inst_1_address3;
assign led19_1 = w_level7_cpu_program_counter_8bit_ic_inst_1_address4;
assign led20_1 = w_level7_cpu_program_counter_8bit_ic_inst_1_address5;
assign led21_1 = w_level7_cpu_program_counter_8bit_ic_inst_1_address6;
assign led22_1 = w_level7_cpu_program_counter_8bit_ic_inst_1_address7;
endmodule
