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
wire aux_not_2;
wire aux_not_3;
wire aux_or_4;
wire aux_or_5;
wire aux_or_6;
wire aux_or_7;
wire aux_not_8;
wire aux_and_9;
wire aux_and_10;
reg aux_nor_11 = 1'b0;
reg aux_nor_12 = 1'b0;
wire aux_and_13;
wire aux_and_14;
reg aux_nor_15 = 1'b0;
reg aux_nor_16 = 1'b0;

// Internal logic
assign aux_not_1 = ~clock;
assign aux_not_2 = ~preset;
assign aux_not_3 = ~clear;
assign aux_or_4 = (aux_and_14 | aux_not_3);
assign aux_or_5 = (aux_and_13 | aux_not_2);
assign aux_or_6 = (aux_and_10 | aux_not_3);
assign aux_or_7 = (aux_and_9 | aux_not_2);
assign aux_not_8 = ~d;
assign aux_and_9 = (d & aux_not_1);
assign aux_and_10 = (aux_not_8 & aux_not_1);
always @(*) aux_nor_11 = ~(aux_or_6 | aux_nor_12);
always @(*) aux_nor_12 = ~(aux_or_7 | aux_nor_11);
assign aux_and_13 = (aux_nor_11 & clock);
assign aux_and_14 = (aux_nor_12 & clock);
always @(*) aux_nor_15 = ~(aux_or_4 | aux_nor_16);
always @(*) aux_nor_16 = ~(aux_or_5 | aux_nor_15);

assign q = aux_nor_15;
assign q_bar = aux_nor_16;
/* verilator lint_on UNOPTFLAT */
endmodule

// Module for hold_mux0 (generated from level2_mux_2to1.panda)
module level2_mux_2to1 (
    input data0,
    input data1,
    input sel0,
    input enable,
    output p_output
);

reg aux_mux_1 = 1'b0;
wire aux_and_2;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel0})
            1'd0: aux_mux_1 = data0;
            1'd1: aux_mux_1 = data1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_and_2 = (aux_mux_1 & enable);

assign p_output = aux_and_2;
endmodule

// Module for LEVEL4_RING_COUNTER_4BIT (generated from level4_ring_counter_4bit.panda)
module level4_ring_counter_4bit_ic (
    input load,
    input countenable,
    input clock,
    input data0,
    input data1,
    input init,
    input data2,
    input data3,
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
// IC instance: hold_mux0 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_6_p_output;
// IC instance: load_mux0 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_7_p_output;
// IC instance: hold_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_8_p_output;
// IC instance: load_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_9_p_output;
// IC instance: hold_mux2 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_10_p_output;
// IC instance: load_mux2 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_11_p_output;
// IC instance: hold_mux3 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_12_p_output;
// IC instance: load_mux3 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_13_p_output;
wire aux_not_14;

// Internal logic
level1_d_flip_flop level1_d_flip_flop_inst_1 (
    .d(w_level2_mux_2to1_inst_7_p_output),
    .clock(clock),
    .preset(aux_not_14),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_1_q),
    .q_bar(w_level1_d_flip_flop_inst_1_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_2 (
    .d(w_level2_mux_2to1_inst_9_p_output),
    .clock(clock),
    .preset(1'b1),
    .clear(aux_not_14),
    .q(w_level1_d_flip_flop_inst_2_q),
    .q_bar(w_level1_d_flip_flop_inst_2_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_3 (
    .d(w_level2_mux_2to1_inst_11_p_output),
    .clock(clock),
    .preset(1'b1),
    .clear(aux_not_14),
    .q(w_level1_d_flip_flop_inst_3_q),
    .q_bar(w_level1_d_flip_flop_inst_3_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_4 (
    .d(w_level2_mux_2to1_inst_13_p_output),
    .clock(clock),
    .preset(1'b1),
    .clear(aux_not_14),
    .q(w_level1_d_flip_flop_inst_4_q),
    .q_bar(w_level1_d_flip_flop_inst_4_q_bar)
);
level2_mux_2to1 level2_mux_2to1_inst_6 (
    .data0(w_level1_d_flip_flop_inst_1_q),
    .data1(w_level1_d_flip_flop_inst_4_q),
    .sel0(countenable),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_6_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_7 (
    .data0(w_level2_mux_2to1_inst_6_p_output),
    .data1(data0),
    .sel0(load),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_7_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_8 (
    .data0(w_level1_d_flip_flop_inst_2_q),
    .data1(w_level1_d_flip_flop_inst_1_q),
    .sel0(countenable),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_8_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_9 (
    .data0(w_level2_mux_2to1_inst_8_p_output),
    .data1(data1),
    .sel0(load),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_9_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_10 (
    .data0(w_level1_d_flip_flop_inst_3_q),
    .data1(w_level1_d_flip_flop_inst_2_q),
    .sel0(countenable),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_10_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_11 (
    .data0(w_level2_mux_2to1_inst_10_p_output),
    .data1(data2),
    .sel0(load),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_11_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_12 (
    .data0(w_level1_d_flip_flop_inst_4_q),
    .data1(w_level1_d_flip_flop_inst_3_q),
    .sel0(countenable),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_12_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_13 (
    .data0(w_level2_mux_2to1_inst_12_p_output),
    .data1(data3),
    .sel0(load),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_13_p_output)
);
assign aux_not_14 = ~init;

assign q0 = w_level1_d_flip_flop_inst_1_q;
assign q1 = w_level1_d_flip_flop_inst_2_q;
assign q2 = w_level1_d_flip_flop_inst_3_q;
assign q3 = w_level1_d_flip_flop_inst_4_q;
endmodule

module level4_ring_counter_4bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,
input input_switch8,

/* ========= Outputs ========== */
output led10_1,
output led11_1,
output led12_1,
output led13_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_RING_COUNTER_4BIT (level4_ring_counter_4bit_ic)
wire w_level4_ring_counter_4bit_ic_inst_1_q0;
wire w_level4_ring_counter_4bit_ic_inst_1_q1;
wire w_level4_ring_counter_4bit_ic_inst_1_q2;
wire w_level4_ring_counter_4bit_ic_inst_1_q3;


// Assigning aux variables. //
level4_ring_counter_4bit_ic level4_ring_counter_4bit_ic_inst_1 (
    .load(input_switch1),
    .countenable(input_switch2),
    .clock(input_switch3),
    .data0(input_switch4),
    .data1(input_switch5),
    .init(input_switch6),
    .data2(input_switch7),
    .data3(input_switch8),
    .q0(w_level4_ring_counter_4bit_ic_inst_1_q0),
    .q1(w_level4_ring_counter_4bit_ic_inst_1_q1),
    .q2(w_level4_ring_counter_4bit_ic_inst_1_q2),
    .q3(w_level4_ring_counter_4bit_ic_inst_1_q3)
);

// Writing output data. //
assign led10_1 = w_level4_ring_counter_4bit_ic_inst_1_q0;
assign led11_1 = w_level4_ring_counter_4bit_ic_inst_1_q1;
assign led12_1 = w_level4_ring_counter_4bit_ic_inst_1_q2;
assign led13_1 = w_level4_ring_counter_4bit_ic_inst_1_q3;
endmodule
