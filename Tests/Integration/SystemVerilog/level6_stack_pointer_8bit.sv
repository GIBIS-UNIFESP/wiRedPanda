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

// Module for SPMux[0] (generated from level2_priority_mux_3to1.panda)
module level2_priority_mux_3to1 (
    input data0,
    input sel0,
    input data1,
    input sel1,
    input data2,
    output out
);

reg aux_mux_1 = 1'b0;
reg aux_mux_2 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel1})
            1'd0: aux_mux_1 = data2;
            1'd1: aux_mux_1 = data1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({sel0})
            1'd0: aux_mux_2 = aux_mux_1;
            1'd1: aux_mux_2 = data0;
            default: aux_mux_2 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign out = aux_mux_2;
endmodule

// Module for Adder (generated from level6_ripple_adder_8bit.panda)
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

// Module for LEVEL6_STACK_POINTER_8BIT (generated from level6_stack_pointer_8bit.panda)
module level6_stack_pointer_8bit_ic (
    input loadvalue0,
    input clock,
    input loadvalue1,
    input load,
    input loadvalue2,
    input push,
    input loadvalue3,
    input pop,
    input loadvalue4,
    input reset,
    input loadvalue5,
    input enable,
    input loadvalue6,
    input loadvalue7,
    output sp0,
    output sp1,
    output sp2,
    output sp3,
    output sp4,
    output sp5,
    output sp6,
    output sp7
);

// IC instance: Adder (level6_ripple_adder_8bit)
wire w_level6_ripple_adder_8bit_inst_1_sum0;
wire w_level6_ripple_adder_8bit_inst_1_sum1;
wire w_level6_ripple_adder_8bit_inst_1_sum2;
wire w_level6_ripple_adder_8bit_inst_1_sum3;
wire w_level6_ripple_adder_8bit_inst_1_sum4;
wire w_level6_ripple_adder_8bit_inst_1_sum5;
wire w_level6_ripple_adder_8bit_inst_1_sum6;
wire w_level6_ripple_adder_8bit_inst_1_sum7;
wire w_level6_ripple_adder_8bit_inst_1_carryout;
// IC instance: SP[0] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_2_q;
wire w_level1_d_flip_flop_inst_2_q_bar;
// IC instance: SP[1] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_3_q;
wire w_level1_d_flip_flop_inst_3_q_bar;
// IC instance: SP[2] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_4_q;
wire w_level1_d_flip_flop_inst_4_q_bar;
// IC instance: SP[3] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_5_q;
wire w_level1_d_flip_flop_inst_5_q_bar;
// IC instance: SP[4] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_6_q;
wire w_level1_d_flip_flop_inst_6_q_bar;
// IC instance: SP[5] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_7_q;
wire w_level1_d_flip_flop_inst_7_q_bar;
// IC instance: SP[6] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_8_q;
wire w_level1_d_flip_flop_inst_8_q_bar;
// IC instance: SP[7] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_9_q;
wire w_level1_d_flip_flop_inst_9_q_bar;
// IC instance: SPMux[0] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_18_out;
// IC instance: SPMux[1] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_19_out;
// IC instance: SPMux[2] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_20_out;
// IC instance: SPMux[3] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_21_out;
// IC instance: SPMux[4] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_22_out;
// IC instance: SPMux[5] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_23_out;
// IC instance: SPMux[6] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_24_out;
// IC instance: SPMux[7] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_25_out;

// Internal logic
level6_ripple_adder_8bit level6_ripple_adder_8bit_inst_1 (
    .a0(w_level1_d_flip_flop_inst_2_q),
    .b0(1'b0),
    .carryin(1'b0),
    .a1(w_level1_d_flip_flop_inst_3_q),
    .b1(1'b0),
    .a2(w_level1_d_flip_flop_inst_4_q),
    .b2(1'b0),
    .a3(w_level1_d_flip_flop_inst_5_q),
    .b3(1'b0),
    .a4(w_level1_d_flip_flop_inst_6_q),
    .b4(1'b0),
    .a5(w_level1_d_flip_flop_inst_7_q),
    .b5(1'b0),
    .a6(w_level1_d_flip_flop_inst_8_q),
    .b6(1'b0),
    .a7(w_level1_d_flip_flop_inst_9_q),
    .b7(1'b0),
    .sum0(w_level6_ripple_adder_8bit_inst_1_sum0),
    .sum1(w_level6_ripple_adder_8bit_inst_1_sum1),
    .sum2(w_level6_ripple_adder_8bit_inst_1_sum2),
    .sum3(w_level6_ripple_adder_8bit_inst_1_sum3),
    .sum4(w_level6_ripple_adder_8bit_inst_1_sum4),
    .sum5(w_level6_ripple_adder_8bit_inst_1_sum5),
    .sum6(w_level6_ripple_adder_8bit_inst_1_sum6),
    .sum7(w_level6_ripple_adder_8bit_inst_1_sum7),
    .carryout(w_level6_ripple_adder_8bit_inst_1_carryout)
);
level1_d_flip_flop level1_d_flip_flop_inst_2 (
    .d(w_level2_priority_mux_3to1_inst_18_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_2_q),
    .q_bar(w_level1_d_flip_flop_inst_2_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_3 (
    .d(w_level2_priority_mux_3to1_inst_19_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_3_q),
    .q_bar(w_level1_d_flip_flop_inst_3_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_4 (
    .d(w_level2_priority_mux_3to1_inst_20_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_4_q),
    .q_bar(w_level1_d_flip_flop_inst_4_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_5 (
    .d(w_level2_priority_mux_3to1_inst_21_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_5_q),
    .q_bar(w_level1_d_flip_flop_inst_5_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_6 (
    .d(w_level2_priority_mux_3to1_inst_22_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_6_q),
    .q_bar(w_level1_d_flip_flop_inst_6_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_7 (
    .d(w_level2_priority_mux_3to1_inst_23_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_7_q),
    .q_bar(w_level1_d_flip_flop_inst_7_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_8 (
    .d(w_level2_priority_mux_3to1_inst_24_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_8_q),
    .q_bar(w_level1_d_flip_flop_inst_8_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_9 (
    .d(w_level2_priority_mux_3to1_inst_25_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_9_q),
    .q_bar(w_level1_d_flip_flop_inst_9_q_bar)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_18 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue0),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum0),
    .out(w_level2_priority_mux_3to1_inst_18_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_19 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue1),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum1),
    .out(w_level2_priority_mux_3to1_inst_19_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_20 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue2),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum2),
    .out(w_level2_priority_mux_3to1_inst_20_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_21 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue3),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum3),
    .out(w_level2_priority_mux_3to1_inst_21_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_22 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue4),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum4),
    .out(w_level2_priority_mux_3to1_inst_22_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_23 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue5),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum5),
    .out(w_level2_priority_mux_3to1_inst_23_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_24 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue6),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum6),
    .out(w_level2_priority_mux_3to1_inst_24_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_25 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue7),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum7),
    .out(w_level2_priority_mux_3to1_inst_25_out)
);

assign sp0 = w_level1_d_flip_flop_inst_2_q;
assign sp1 = w_level1_d_flip_flop_inst_3_q;
assign sp2 = w_level1_d_flip_flop_inst_4_q;
assign sp3 = w_level1_d_flip_flop_inst_5_q;
assign sp4 = w_level1_d_flip_flop_inst_6_q;
assign sp5 = w_level1_d_flip_flop_inst_7_q;
assign sp6 = w_level1_d_flip_flop_inst_8_q;
assign sp7 = w_level1_d_flip_flop_inst_9_q;
endmodule

module level6_stack_pointer_8bit (
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
input input_switch14,

/* ========= Outputs ========== */
output led16_1,
output led17_1,
output led18_1,
output led19_1,
output led20_1,
output led21_1,
output led22_1,
output led23_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL6_STACK_POINTER_8BIT (level6_stack_pointer_8bit_ic)
wire w_level6_stack_pointer_8bit_ic_inst_1_sp0;
wire w_level6_stack_pointer_8bit_ic_inst_1_sp1;
wire w_level6_stack_pointer_8bit_ic_inst_1_sp2;
wire w_level6_stack_pointer_8bit_ic_inst_1_sp3;
wire w_level6_stack_pointer_8bit_ic_inst_1_sp4;
wire w_level6_stack_pointer_8bit_ic_inst_1_sp5;
wire w_level6_stack_pointer_8bit_ic_inst_1_sp6;
wire w_level6_stack_pointer_8bit_ic_inst_1_sp7;


// Assigning aux variables. //
level6_stack_pointer_8bit_ic level6_stack_pointer_8bit_ic_inst_1 (
    .loadvalue0(input_switch1),
    .clock(input_switch2),
    .loadvalue1(input_switch3),
    .load(input_switch4),
    .loadvalue2(input_switch5),
    .push(input_switch6),
    .loadvalue3(input_switch7),
    .pop(input_switch8),
    .loadvalue4(input_switch9),
    .reset(input_switch10),
    .loadvalue5(input_switch11),
    .enable(input_switch12),
    .loadvalue6(input_switch13),
    .loadvalue7(input_switch14),
    .sp0(w_level6_stack_pointer_8bit_ic_inst_1_sp0),
    .sp1(w_level6_stack_pointer_8bit_ic_inst_1_sp1),
    .sp2(w_level6_stack_pointer_8bit_ic_inst_1_sp2),
    .sp3(w_level6_stack_pointer_8bit_ic_inst_1_sp3),
    .sp4(w_level6_stack_pointer_8bit_ic_inst_1_sp4),
    .sp5(w_level6_stack_pointer_8bit_ic_inst_1_sp5),
    .sp6(w_level6_stack_pointer_8bit_ic_inst_1_sp6),
    .sp7(w_level6_stack_pointer_8bit_ic_inst_1_sp7)
);

// Writing output data. //
assign led16_1 = w_level6_stack_pointer_8bit_ic_inst_1_sp0;
assign led17_1 = w_level6_stack_pointer_8bit_ic_inst_1_sp1;
assign led18_1 = w_level6_stack_pointer_8bit_ic_inst_1_sp2;
assign led19_1 = w_level6_stack_pointer_8bit_ic_inst_1_sp3;
assign led20_1 = w_level6_stack_pointer_8bit_ic_inst_1_sp4;
assign led21_1 = w_level6_stack_pointer_8bit_ic_inst_1_sp5;
assign led22_1 = w_level6_stack_pointer_8bit_ic_inst_1_sp6;
assign led23_1 = w_level6_stack_pointer_8bit_ic_inst_1_sp7;
endmodule
