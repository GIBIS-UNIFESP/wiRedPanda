// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for HA1 (generated from level2_half_adder.panda)
module level2_half_adder (
    input a,
    input b,
    output sum,
    output carry
);

wire aux_xor_1;
wire aux_and_2;

// Internal logic
assign aux_xor_1 = (a ^ b);
assign aux_and_2 = (a & b);

assign sum = aux_xor_1;
assign carry = aux_and_2;
endmodule

// Module for Mux[0] (generated from level2_mux_2to1.panda)
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

// Module for BusMux_LoadHold (generated from level4_bus_mux_4bit.panda)
module level4_bus_mux_4bit (
    input in00,
    input in01,
    input in02,
    input in03,
    input in10,
    input in11,
    input in12,
    input in13,
    input sel,
    output out0,
    output out1,
    output out2,
    output out3
);

// IC instance: Mux[0] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_1_p_output;
// IC instance: Mux[1] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_2_p_output;
// IC instance: Mux[2] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_3_p_output;
// IC instance: Mux[3] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_4_p_output;

// Internal logic
level2_mux_2to1 level2_mux_2to1_inst_1 (
    .data0(in00),
    .data1(in10),
    .sel0(sel),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_1_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_2 (
    .data0(in01),
    .data1(in11),
    .sel0(sel),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_2_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_3 (
    .data0(in02),
    .data1(in12),
    .sel0(sel),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_3_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_4 (
    .data0(in03),
    .data1(in13),
    .sel0(sel),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_4_p_output)
);

assign out0 = w_level2_mux_2to1_inst_1_p_output;
assign out1 = w_level2_mux_2to1_inst_2_p_output;
assign out2 = w_level2_mux_2to1_inst_3_p_output;
assign out3 = w_level2_mux_2to1_inst_4_p_output;
endmodule

// Module for Register4bit (generated from level4_register_4bit.panda)
module level4_register_4bit (
    input clk,
    input enable,
    input reset,
    input d0,
    input d1,
    input d2,
    input d3,
    output q0,
    output q1,
    output q2,
    output q3
);

// IC instance: BusMux_LoadHold (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_1_out0;
wire w_level4_bus_mux_4bit_inst_1_out1;
wire w_level4_bus_mux_4bit_inst_1_out2;
wire w_level4_bus_mux_4bit_inst_1_out3;
reg aux_d_flip_flop_2_0_q = 1'b0;
reg aux_d_flip_flop_2_1_q = 1'b1;
reg aux_d_flip_flop_3_0_q = 1'b0;
reg aux_d_flip_flop_3_1_q = 1'b1;
reg aux_d_flip_flop_4_0_q = 1'b0;
reg aux_d_flip_flop_4_1_q = 1'b1;
reg aux_d_flip_flop_5_0_q = 1'b0;
reg aux_d_flip_flop_5_1_q = 1'b1;
wire aux_not_6;
wire aux_not_7;

// Internal logic
level4_bus_mux_4bit level4_bus_mux_4bit_inst_1 (
    .in00(d0),
    .in01(d1),
    .in02(d2),
    .in03(d3),
    .in10(aux_d_flip_flop_2_0_q),
    .in11(aux_d_flip_flop_3_0_q),
    .in12(aux_d_flip_flop_4_0_q),
    .in13(aux_d_flip_flop_5_0_q),
    .sel(aux_not_7),
    .out0(w_level4_bus_mux_4bit_inst_1_out0),
    .out1(w_level4_bus_mux_4bit_inst_1_out1),
    .out2(w_level4_bus_mux_4bit_inst_1_out2),
    .out3(w_level4_bus_mux_4bit_inst_1_out3)
);
    //D FlipFlop
    always @(posedge clk or negedge aux_not_6)
    begin
        if (~aux_not_6)
        begin
            aux_d_flip_flop_2_0_q <= 1'b0;
            aux_d_flip_flop_2_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_2_0_q <= w_level4_bus_mux_4bit_inst_1_out0;
            aux_d_flip_flop_2_1_q <= ~w_level4_bus_mux_4bit_inst_1_out0;
        end
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clk or negedge aux_not_6)
    begin
        if (~aux_not_6)
        begin
            aux_d_flip_flop_3_0_q <= 1'b0;
            aux_d_flip_flop_3_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_3_0_q <= w_level4_bus_mux_4bit_inst_1_out1;
            aux_d_flip_flop_3_1_q <= ~w_level4_bus_mux_4bit_inst_1_out1;
        end
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clk or negedge aux_not_6)
    begin
        if (~aux_not_6)
        begin
            aux_d_flip_flop_4_0_q <= 1'b0;
            aux_d_flip_flop_4_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_4_0_q <= w_level4_bus_mux_4bit_inst_1_out2;
            aux_d_flip_flop_4_1_q <= ~w_level4_bus_mux_4bit_inst_1_out2;
        end
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clk or negedge aux_not_6)
    begin
        if (~aux_not_6)
        begin
            aux_d_flip_flop_5_0_q <= 1'b0;
            aux_d_flip_flop_5_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_5_0_q <= w_level4_bus_mux_4bit_inst_1_out3;
            aux_d_flip_flop_5_1_q <= ~w_level4_bus_mux_4bit_inst_1_out3;
        end
    end
    //End of D FlipFlop
assign aux_not_6 = ~reset;
assign aux_not_7 = ~enable;

assign q0 = aux_d_flip_flop_2_0_q;
assign q1 = aux_d_flip_flop_3_0_q;
assign q2 = aux_d_flip_flop_4_0_q;
assign q3 = aux_d_flip_flop_5_0_q;
endmodule

// Module for FA[0] (generated from level2_full_adder_1bit.panda)
module level2_full_adder_1bit (
    input a,
    input b,
    input cin,
    output sum,
    output cout
);

// IC instance: HA1 (level2_half_adder)
wire w_level2_half_adder_inst_1_sum;
wire w_level2_half_adder_inst_1_carry;
// IC instance: HA2 (level2_half_adder)
wire w_level2_half_adder_inst_2_sum;
wire w_level2_half_adder_inst_2_carry;
wire aux_or_3;

// Internal logic
level2_half_adder level2_half_adder_inst_1 (
    .a(a),
    .b(b),
    .sum(w_level2_half_adder_inst_1_sum),
    .carry(w_level2_half_adder_inst_1_carry)
);
level2_half_adder level2_half_adder_inst_2 (
    .a(w_level2_half_adder_inst_1_sum),
    .b(cin),
    .sum(w_level2_half_adder_inst_2_sum),
    .carry(w_level2_half_adder_inst_2_carry)
);
assign aux_or_3 = (w_level2_half_adder_inst_1_carry | w_level2_half_adder_inst_2_carry);

assign sum = w_level2_half_adder_inst_2_sum;
assign cout = aux_or_3;
endmodule

// Module for Adder4bit (generated from level4_ripple_adder_4bit.panda)
module level4_ripple_adder_4bit (
    input a0,
    input a1,
    input b0,
    input a2,
    input b1,
    input carryin,
    input a3,
    input b2,
    input b3,
    output sum0,
    output sum1,
    output sum2,
    output carryout,
    output sum3
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

assign sum0 = w_level2_full_adder_1bit_inst_1_sum;
assign sum1 = w_level2_full_adder_1bit_inst_2_sum;
assign sum2 = w_level2_full_adder_1bit_inst_3_sum;
assign carryout = w_level2_full_adder_1bit_inst_4_cout;
assign sum3 = w_level2_full_adder_1bit_inst_4_sum;
endmodule

// Module for LEVEL5_PROGRAM_COUNTER_4BIT (generated from level5_program_counter_4bit.panda)
module level5_program_counter_4bit_ic (
    input loadvalue0,
    input loadvalue1,
    input loadvalue2,
    input loadvalue3,
    input load,
    input inc,
    input reset,
    input clock,
    output pc0,
    output pc1,
    output pc2,
    output pc3,
    output pc_plus_10,
    output pc_plus_11,
    output pc_plus_12,
    output pc_plus_13
);

// IC instance: Register4bit (level4_register_4bit)
wire w_level4_register_4bit_inst_1_q0;
wire w_level4_register_4bit_inst_1_q1;
wire w_level4_register_4bit_inst_1_q2;
wire w_level4_register_4bit_inst_1_q3;
// IC instance: Adder4bit (level4_ripple_adder_4bit)
wire w_level4_ripple_adder_4bit_inst_2_sum0;
wire w_level4_ripple_adder_4bit_inst_2_sum1;
wire w_level4_ripple_adder_4bit_inst_2_sum2;
wire w_level4_ripple_adder_4bit_inst_2_carryout;
wire w_level4_ripple_adder_4bit_inst_2_sum3;
wire aux_not_3;
wire aux_and_4;
reg aux_mux_5 = 1'b0;
reg aux_mux_6 = 1'b0;
reg aux_mux_7 = 1'b0;
reg aux_mux_8 = 1'b0;
reg aux_mux_9 = 1'b0;
reg aux_mux_10 = 1'b0;
reg aux_mux_11 = 1'b0;
reg aux_mux_12 = 1'b0;

// Internal logic
level4_register_4bit level4_register_4bit_inst_1 (
    .clk(clock),
    .enable(1'b1),
    .reset(reset),
    .d0(aux_mux_6),
    .d1(aux_mux_8),
    .d2(aux_mux_10),
    .d3(aux_mux_12),
    .q0(w_level4_register_4bit_inst_1_q0),
    .q1(w_level4_register_4bit_inst_1_q1),
    .q2(w_level4_register_4bit_inst_1_q2),
    .q3(w_level4_register_4bit_inst_1_q3)
);
level4_ripple_adder_4bit level4_ripple_adder_4bit_inst_2 (
    .a0(w_level4_register_4bit_inst_1_q0),
    .a1(w_level4_register_4bit_inst_1_q1),
    .b0(1'b1),
    .a2(w_level4_register_4bit_inst_1_q2),
    .b1(1'b0),
    .carryin(1'b0),
    .a3(w_level4_register_4bit_inst_1_q3),
    .b2(1'b0),
    .b3(1'b0),
    .sum0(w_level4_ripple_adder_4bit_inst_2_sum0),
    .sum1(w_level4_ripple_adder_4bit_inst_2_sum1),
    .sum2(w_level4_ripple_adder_4bit_inst_2_sum2),
    .carryout(w_level4_ripple_adder_4bit_inst_2_carryout),
    .sum3(w_level4_ripple_adder_4bit_inst_2_sum3)
);
assign aux_not_3 = ~load;
assign aux_and_4 = (inc & aux_not_3);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_5 = w_level4_register_4bit_inst_1_q0;
            1'd1: aux_mux_5 = w_level4_ripple_adder_4bit_inst_2_sum0;
            default: aux_mux_5 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_6 = aux_mux_5;
            1'd1: aux_mux_6 = loadvalue0;
            default: aux_mux_6 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_7 = w_level4_register_4bit_inst_1_q1;
            1'd1: aux_mux_7 = w_level4_ripple_adder_4bit_inst_2_sum1;
            default: aux_mux_7 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_8 = aux_mux_7;
            1'd1: aux_mux_8 = loadvalue1;
            default: aux_mux_8 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_9 = w_level4_register_4bit_inst_1_q2;
            1'd1: aux_mux_9 = w_level4_ripple_adder_4bit_inst_2_sum2;
            default: aux_mux_9 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_10 = aux_mux_9;
            1'd1: aux_mux_10 = loadvalue2;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_11 = w_level4_register_4bit_inst_1_q3;
            1'd1: aux_mux_11 = w_level4_ripple_adder_4bit_inst_2_sum3;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_12 = aux_mux_11;
            1'd1: aux_mux_12 = loadvalue3;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign pc0 = w_level4_register_4bit_inst_1_q0;
assign pc1 = w_level4_register_4bit_inst_1_q1;
assign pc2 = w_level4_register_4bit_inst_1_q2;
assign pc3 = w_level4_register_4bit_inst_1_q3;
assign pc_plus_10 = w_level4_ripple_adder_4bit_inst_2_sum0;
assign pc_plus_11 = w_level4_ripple_adder_4bit_inst_2_sum1;
assign pc_plus_12 = w_level4_ripple_adder_4bit_inst_2_sum2;
assign pc_plus_13 = w_level4_ripple_adder_4bit_inst_2_sum3;
endmodule

module level5_program_counter_4bit (
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
output led13_1,
output led14_1,
output led15_1,
output led16_1,
output led17_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL5_PROGRAM_COUNTER_4BIT (level5_program_counter_4bit_ic)
wire w_level5_program_counter_4bit_ic_inst_1_pc0;
wire w_level5_program_counter_4bit_ic_inst_1_pc1;
wire w_level5_program_counter_4bit_ic_inst_1_pc2;
wire w_level5_program_counter_4bit_ic_inst_1_pc3;
wire w_level5_program_counter_4bit_ic_inst_1_pc_plus_10;
wire w_level5_program_counter_4bit_ic_inst_1_pc_plus_11;
wire w_level5_program_counter_4bit_ic_inst_1_pc_plus_12;
wire w_level5_program_counter_4bit_ic_inst_1_pc_plus_13;


// Assigning aux variables. //
level5_program_counter_4bit_ic level5_program_counter_4bit_ic_inst_1 (
    .loadvalue0(input_switch1),
    .loadvalue1(input_switch2),
    .loadvalue2(input_switch3),
    .loadvalue3(input_switch4),
    .load(input_switch5),
    .inc(input_switch6),
    .reset(input_switch7),
    .clock(input_switch8),
    .pc0(w_level5_program_counter_4bit_ic_inst_1_pc0),
    .pc1(w_level5_program_counter_4bit_ic_inst_1_pc1),
    .pc2(w_level5_program_counter_4bit_ic_inst_1_pc2),
    .pc3(w_level5_program_counter_4bit_ic_inst_1_pc3),
    .pc_plus_10(w_level5_program_counter_4bit_ic_inst_1_pc_plus_10),
    .pc_plus_11(w_level5_program_counter_4bit_ic_inst_1_pc_plus_11),
    .pc_plus_12(w_level5_program_counter_4bit_ic_inst_1_pc_plus_12),
    .pc_plus_13(w_level5_program_counter_4bit_ic_inst_1_pc_plus_13)
);

// Writing output data. //
assign led10_1 = w_level5_program_counter_4bit_ic_inst_1_pc0;
assign led11_1 = w_level5_program_counter_4bit_ic_inst_1_pc1;
assign led12_1 = w_level5_program_counter_4bit_ic_inst_1_pc2;
assign led13_1 = w_level5_program_counter_4bit_ic_inst_1_pc3;
assign led14_1 = w_level5_program_counter_4bit_ic_inst_1_pc_plus_10;
assign led15_1 = w_level5_program_counter_4bit_ic_inst_1_pc_plus_11;
assign led16_1 = w_level5_program_counter_4bit_ic_inst_1_pc_plus_12;
assign led17_1 = w_level5_program_counter_4bit_ic_inst_1_pc_plus_13;
endmodule
