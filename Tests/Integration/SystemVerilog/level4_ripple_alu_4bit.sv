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

// Module for mux_and[0] (generated from level2_mux_2to1.panda)
module level2_mux_2to1 (
    input data0,
    input data1,
    input sel0,
    output p_output
);

reg aux_mux_1 = 1'b0;

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

assign p_output = aux_mux_1;
endmodule

// Module for Adder (generated from level4_ripple_adder_4bit.panda)
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

// Module for LEVEL4_RIPPLE_ALU_4BIT (generated from level4_ripple_alu_4bit.panda)
module level4_ripple_alu_4bit_ic (
    input a0,
    input a1,
    input a2,
    input a3,
    input b0,
    input b1,
    input b2,
    input b3,
    input carryin,
    input subcarryin,
    output result_add0,
    output result_add1,
    output result_add2,
    output result_add3,
    output result_sub0,
    output result_sub1,
    output result_sub2,
    output result_sub3,
    output result_and0,
    output result_and1,
    output result_and2,
    output result_and3,
    output result_or0,
    output result_or1,
    output result_or2,
    output result_or3,
    output carryout,
    output subcarryout
);

// IC instance: Adder (level4_ripple_adder_4bit)
wire w_level4_ripple_adder_4bit_inst_3_sum0;
wire w_level4_ripple_adder_4bit_inst_3_sum1;
wire w_level4_ripple_adder_4bit_inst_3_sum2;
wire w_level4_ripple_adder_4bit_inst_3_carryout;
wire w_level4_ripple_adder_4bit_inst_3_sum3;
// IC instance: Subtractor (Adder with ~B) (level4_ripple_adder_4bit)
wire w_level4_ripple_adder_4bit_inst_4_sum0;
wire w_level4_ripple_adder_4bit_inst_4_sum1;
wire w_level4_ripple_adder_4bit_inst_4_sum2;
wire w_level4_ripple_adder_4bit_inst_4_carryout;
wire w_level4_ripple_adder_4bit_inst_4_sum3;
wire aux_not_5;
wire aux_not_6;
wire aux_not_7;
wire aux_not_8;
// IC instance: mux_and[0] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_9_p_output;
// IC instance: mux_and[1] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_10_p_output;
// IC instance: mux_and[2] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_11_p_output;
// IC instance: mux_and[3] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_12_p_output;
// IC instance: mux_or[0] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_13_p_output;
// IC instance: mux_or[1] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_14_p_output;
// IC instance: mux_or[2] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_15_p_output;
// IC instance: mux_or[3] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_16_p_output;

// Internal logic
level4_ripple_adder_4bit level4_ripple_adder_4bit_inst_3 (
    .a0(a0),
    .a1(a1),
    .b0(b0),
    .a2(a2),
    .b1(b1),
    .carryin(carryin),
    .a3(a3),
    .b2(b2),
    .b3(b3),
    .sum0(w_level4_ripple_adder_4bit_inst_3_sum0),
    .sum1(w_level4_ripple_adder_4bit_inst_3_sum1),
    .sum2(w_level4_ripple_adder_4bit_inst_3_sum2),
    .carryout(w_level4_ripple_adder_4bit_inst_3_carryout),
    .sum3(w_level4_ripple_adder_4bit_inst_3_sum3)
);
level4_ripple_adder_4bit level4_ripple_adder_4bit_inst_4 (
    .a0(a0),
    .a1(a1),
    .b0(aux_not_5),
    .a2(a2),
    .b1(aux_not_6),
    .carryin(subcarryin),
    .a3(a3),
    .b2(aux_not_7),
    .b3(aux_not_8),
    .sum0(w_level4_ripple_adder_4bit_inst_4_sum0),
    .sum1(w_level4_ripple_adder_4bit_inst_4_sum1),
    .sum2(w_level4_ripple_adder_4bit_inst_4_sum2),
    .carryout(w_level4_ripple_adder_4bit_inst_4_carryout),
    .sum3(w_level4_ripple_adder_4bit_inst_4_sum3)
);
assign aux_not_5 = ~b0;
assign aux_not_6 = ~b1;
assign aux_not_7 = ~b2;
assign aux_not_8 = ~b3;
level2_mux_2to1 level2_mux_2to1_inst_9 (
    .data0(1'b0),
    .data1(b0),
    .sel0(a0),
    .p_output(w_level2_mux_2to1_inst_9_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_10 (
    .data0(1'b0),
    .data1(b1),
    .sel0(a1),
    .p_output(w_level2_mux_2to1_inst_10_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_11 (
    .data0(1'b0),
    .data1(b2),
    .sel0(a2),
    .p_output(w_level2_mux_2to1_inst_11_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_12 (
    .data0(1'b0),
    .data1(b3),
    .sel0(a3),
    .p_output(w_level2_mux_2to1_inst_12_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_13 (
    .data0(a0),
    .data1(1'b1),
    .sel0(b0),
    .p_output(w_level2_mux_2to1_inst_13_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_14 (
    .data0(a1),
    .data1(1'b1),
    .sel0(b1),
    .p_output(w_level2_mux_2to1_inst_14_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_15 (
    .data0(a2),
    .data1(1'b1),
    .sel0(b2),
    .p_output(w_level2_mux_2to1_inst_15_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_16 (
    .data0(a3),
    .data1(1'b1),
    .sel0(b3),
    .p_output(w_level2_mux_2to1_inst_16_p_output)
);

assign result_add0 = w_level4_ripple_adder_4bit_inst_3_sum0;
assign result_add1 = w_level4_ripple_adder_4bit_inst_3_sum1;
assign result_add2 = w_level4_ripple_adder_4bit_inst_3_sum2;
assign result_add3 = w_level4_ripple_adder_4bit_inst_3_sum3;
assign result_sub0 = w_level4_ripple_adder_4bit_inst_4_sum0;
assign result_sub1 = w_level4_ripple_adder_4bit_inst_4_sum1;
assign result_sub2 = w_level4_ripple_adder_4bit_inst_4_sum2;
assign result_sub3 = w_level4_ripple_adder_4bit_inst_4_sum3;
assign result_and0 = w_level2_mux_2to1_inst_9_p_output;
assign result_and1 = w_level2_mux_2to1_inst_10_p_output;
assign result_and2 = w_level2_mux_2to1_inst_11_p_output;
assign result_and3 = w_level2_mux_2to1_inst_12_p_output;
assign result_or0 = w_level2_mux_2to1_inst_13_p_output;
assign result_or1 = w_level2_mux_2to1_inst_14_p_output;
assign result_or2 = w_level2_mux_2to1_inst_15_p_output;
assign result_or3 = w_level2_mux_2to1_inst_16_p_output;
assign carryout = w_level4_ripple_adder_4bit_inst_3_carryout;
assign subcarryout = w_level4_ripple_adder_4bit_inst_4_carryout;
endmodule

module level4_ripple_alu_4bit (
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

/* ========= Outputs ========== */
output led12_1,
output led13_1,
output led14_1,
output led15_1,
output led16_1,
output led17_1,
output led18_1,
output led19_1,
output led20_1,
output led21_1,
output led22_1,
output led23_1,
output led24_1,
output led25_1,
output led26_1,
output led27_1,
output led28_1,
output led29_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_RIPPLE_ALU_4BIT (level4_ripple_alu_4bit_ic)
wire w_level4_ripple_alu_4bit_ic_inst_1_result_add0;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_add1;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_add2;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_add3;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_sub0;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_sub1;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_sub2;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_sub3;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_and0;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_and1;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_and2;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_and3;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_or0;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_or1;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_or2;
wire w_level4_ripple_alu_4bit_ic_inst_1_result_or3;
wire w_level4_ripple_alu_4bit_ic_inst_1_carryout;
wire w_level4_ripple_alu_4bit_ic_inst_1_subcarryout;


// Assigning aux variables. //
level4_ripple_alu_4bit_ic level4_ripple_alu_4bit_ic_inst_1 (
    .a0(input_switch1),
    .a1(input_switch2),
    .a2(input_switch3),
    .a3(input_switch4),
    .b0(input_switch5),
    .b1(input_switch6),
    .b2(input_switch7),
    .b3(input_switch8),
    .carryin(input_switch9),
    .subcarryin(input_switch10),
    .result_add0(w_level4_ripple_alu_4bit_ic_inst_1_result_add0),
    .result_add1(w_level4_ripple_alu_4bit_ic_inst_1_result_add1),
    .result_add2(w_level4_ripple_alu_4bit_ic_inst_1_result_add2),
    .result_add3(w_level4_ripple_alu_4bit_ic_inst_1_result_add3),
    .result_sub0(w_level4_ripple_alu_4bit_ic_inst_1_result_sub0),
    .result_sub1(w_level4_ripple_alu_4bit_ic_inst_1_result_sub1),
    .result_sub2(w_level4_ripple_alu_4bit_ic_inst_1_result_sub2),
    .result_sub3(w_level4_ripple_alu_4bit_ic_inst_1_result_sub3),
    .result_and0(w_level4_ripple_alu_4bit_ic_inst_1_result_and0),
    .result_and1(w_level4_ripple_alu_4bit_ic_inst_1_result_and1),
    .result_and2(w_level4_ripple_alu_4bit_ic_inst_1_result_and2),
    .result_and3(w_level4_ripple_alu_4bit_ic_inst_1_result_and3),
    .result_or0(w_level4_ripple_alu_4bit_ic_inst_1_result_or0),
    .result_or1(w_level4_ripple_alu_4bit_ic_inst_1_result_or1),
    .result_or2(w_level4_ripple_alu_4bit_ic_inst_1_result_or2),
    .result_or3(w_level4_ripple_alu_4bit_ic_inst_1_result_or3),
    .carryout(w_level4_ripple_alu_4bit_ic_inst_1_carryout),
    .subcarryout(w_level4_ripple_alu_4bit_ic_inst_1_subcarryout)
);

// Writing output data. //
assign led12_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_add0;
assign led13_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_add1;
assign led14_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_add2;
assign led15_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_add3;
assign led16_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_sub0;
assign led17_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_sub1;
assign led18_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_sub2;
assign led19_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_sub3;
assign led20_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_and0;
assign led21_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_and1;
assign led22_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_and2;
assign led23_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_and3;
assign led24_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_or0;
assign led25_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_or1;
assign led26_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_or2;
assign led27_1 = w_level4_ripple_alu_4bit_ic_inst_1_result_or3;
assign led28_1 = w_level4_ripple_alu_4bit_ic_inst_1_carryout;
assign led29_1 = w_level4_ripple_alu_4bit_ic_inst_1_subcarryout;
endmodule
