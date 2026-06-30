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

// Module for Selector5way[0] (generated from level3_alu_selector_5way.panda)
module level3_alu_selector_5way (
    input result0,
    input result1,
    input result2,
    input result3,
    input result4,
    input op0,
    input op1,
    input op2,
    output out
);

// IC instance: l1_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_1_p_output;
// IC instance: l1_mux2 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_2_p_output;
// IC instance: l2_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_3_p_output;
// IC instance: l3_mux (level2_mux_2to1)
wire w_level2_mux_2to1_inst_4_p_output;

// Internal logic
level2_mux_2to1 level2_mux_2to1_inst_1 (
    .data0(result0),
    .data1(result1),
    .sel0(op0),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_1_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_2 (
    .data0(result2),
    .data1(result3),
    .sel0(op0),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_2_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_3 (
    .data0(w_level2_mux_2to1_inst_1_p_output),
    .data1(w_level2_mux_2to1_inst_2_p_output),
    .sel0(op1),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_3_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_4 (
    .data0(w_level2_mux_2to1_inst_3_p_output),
    .data1(result4),
    .sel0(op2),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_4_p_output)
);

assign out = w_level2_mux_2to1_inst_4_p_output;
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

// Module for ALU_Low (generated from level4_ripple_alu_4bit.panda)
module level4_ripple_alu_4bit (
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
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_9_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_10 (
    .data0(1'b0),
    .data1(b1),
    .sel0(a1),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_10_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_11 (
    .data0(1'b0),
    .data1(b2),
    .sel0(a2),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_11_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_12 (
    .data0(1'b0),
    .data1(b3),
    .sel0(a3),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_12_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_13 (
    .data0(a0),
    .data1(1'b1),
    .sel0(b0),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_13_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_14 (
    .data0(a1),
    .data1(1'b1),
    .sel0(b1),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_14_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_15 (
    .data0(a2),
    .data1(1'b1),
    .sel0(b2),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_15_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_16 (
    .data0(a3),
    .data1(1'b1),
    .sel0(b3),
    .enable(1'b1),
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

// Module for ALU_8bit (generated from level6_alu_8bit.panda)
module level6_alu_8bit (
    input a0,
    input a1,
    input a2,
    input a3,
    input a4,
    input a5,
    input a6,
    input a7,
    input opcode0,
    input opcode1,
    input opcode2,
    input b0,
    input b1,
    input b2,
    input b3,
    input b4,
    input b5,
    input b6,
    input b7,
    input carryin,
    input subcarryin,
    input shrin,
    input shlin,
    output result0,
    output result1,
    output zero,
    output result2,
    output negative,
    output result3,
    output carry,
    output result4,
    output subcarryout,
    output result5,
    output result6,
    output result7
);

// IC instance: ALU_Low (level4_ripple_alu_4bit)
wire w_level4_ripple_alu_4bit_inst_1_result_add0;
wire w_level4_ripple_alu_4bit_inst_1_result_add1;
wire w_level4_ripple_alu_4bit_inst_1_result_add2;
wire w_level4_ripple_alu_4bit_inst_1_result_add3;
wire w_level4_ripple_alu_4bit_inst_1_result_sub0;
wire w_level4_ripple_alu_4bit_inst_1_result_sub1;
wire w_level4_ripple_alu_4bit_inst_1_result_sub2;
wire w_level4_ripple_alu_4bit_inst_1_result_sub3;
wire w_level4_ripple_alu_4bit_inst_1_result_and0;
wire w_level4_ripple_alu_4bit_inst_1_result_and1;
wire w_level4_ripple_alu_4bit_inst_1_result_and2;
wire w_level4_ripple_alu_4bit_inst_1_result_and3;
wire w_level4_ripple_alu_4bit_inst_1_result_or0;
wire w_level4_ripple_alu_4bit_inst_1_result_or1;
wire w_level4_ripple_alu_4bit_inst_1_result_or2;
wire w_level4_ripple_alu_4bit_inst_1_result_or3;
wire w_level4_ripple_alu_4bit_inst_1_carryout;
wire w_level4_ripple_alu_4bit_inst_1_subcarryout;
// IC instance: ALU_High (level4_ripple_alu_4bit)
wire w_level4_ripple_alu_4bit_inst_2_result_add0;
wire w_level4_ripple_alu_4bit_inst_2_result_add1;
wire w_level4_ripple_alu_4bit_inst_2_result_add2;
wire w_level4_ripple_alu_4bit_inst_2_result_add3;
wire w_level4_ripple_alu_4bit_inst_2_result_sub0;
wire w_level4_ripple_alu_4bit_inst_2_result_sub1;
wire w_level4_ripple_alu_4bit_inst_2_result_sub2;
wire w_level4_ripple_alu_4bit_inst_2_result_sub3;
wire w_level4_ripple_alu_4bit_inst_2_result_and0;
wire w_level4_ripple_alu_4bit_inst_2_result_and1;
wire w_level4_ripple_alu_4bit_inst_2_result_and2;
wire w_level4_ripple_alu_4bit_inst_2_result_and3;
wire w_level4_ripple_alu_4bit_inst_2_result_or0;
wire w_level4_ripple_alu_4bit_inst_2_result_or1;
wire w_level4_ripple_alu_4bit_inst_2_result_or2;
wire w_level4_ripple_alu_4bit_inst_2_result_or3;
wire w_level4_ripple_alu_4bit_inst_2_carryout;
wire w_level4_ripple_alu_4bit_inst_2_subcarryout;
wire aux_xor_3;
wire aux_xor_4;
wire aux_xor_5;
wire aux_xor_6;
wire aux_xor_7;
wire aux_xor_8;
wire aux_xor_9;
wire aux_xor_10;
wire aux_not_11;
wire aux_not_12;
wire aux_not_13;
wire aux_not_14;
wire aux_not_15;
wire aux_not_16;
wire aux_not_17;
wire aux_not_18;
// IC instance: Selector5way[0] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_19_out;
// IC instance: Selector5way[1] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_20_out;
// IC instance: Selector5way[2] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_21_out;
// IC instance: Selector5way[3] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_22_out;
// IC instance: Selector5way[4] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_23_out;
// IC instance: Selector5way[5] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_24_out;
// IC instance: Selector5way[6] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_25_out;
// IC instance: Selector5way[7] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_26_out;
wire aux_not_27;
wire aux_not_28;
wire aux_and_29;
wire aux_and_30;
wire aux_and_31;
wire aux_and_32;
wire aux_and_33;
wire aux_and_34;
reg aux_mux_35 = 1'b0;
reg aux_mux_36 = 1'b0;
reg aux_mux_37 = 1'b0;
wire aux_or_38;
reg aux_mux_39 = 1'b0;
reg aux_mux_40 = 1'b0;
reg aux_mux_41 = 1'b0;
wire aux_or_42;
reg aux_mux_43 = 1'b0;
reg aux_mux_44 = 1'b0;
reg aux_mux_45 = 1'b0;
wire aux_or_46;
reg aux_mux_47 = 1'b0;
reg aux_mux_48 = 1'b0;
reg aux_mux_49 = 1'b0;
wire aux_or_50;
reg aux_mux_51 = 1'b0;
reg aux_mux_52 = 1'b0;
reg aux_mux_53 = 1'b0;
wire aux_or_54;
reg aux_mux_55 = 1'b0;
reg aux_mux_56 = 1'b0;
reg aux_mux_57 = 1'b0;
wire aux_or_58;
reg aux_mux_59 = 1'b0;
reg aux_mux_60 = 1'b0;
reg aux_mux_61 = 1'b0;
wire aux_or_62;
reg aux_mux_63 = 1'b0;
reg aux_mux_64 = 1'b0;
reg aux_mux_65 = 1'b0;
wire aux_or_66;
wire aux_nor_67;

// Internal logic
level4_ripple_alu_4bit level4_ripple_alu_4bit_inst_1 (
    .a0(a0),
    .a1(a1),
    .a2(a2),
    .a3(a3),
    .b0(b0),
    .b1(b1),
    .b2(b2),
    .b3(b3),
    .carryin(carryin),
    .subcarryin(subcarryin),
    .result_add0(w_level4_ripple_alu_4bit_inst_1_result_add0),
    .result_add1(w_level4_ripple_alu_4bit_inst_1_result_add1),
    .result_add2(w_level4_ripple_alu_4bit_inst_1_result_add2),
    .result_add3(w_level4_ripple_alu_4bit_inst_1_result_add3),
    .result_sub0(w_level4_ripple_alu_4bit_inst_1_result_sub0),
    .result_sub1(w_level4_ripple_alu_4bit_inst_1_result_sub1),
    .result_sub2(w_level4_ripple_alu_4bit_inst_1_result_sub2),
    .result_sub3(w_level4_ripple_alu_4bit_inst_1_result_sub3),
    .result_and0(w_level4_ripple_alu_4bit_inst_1_result_and0),
    .result_and1(w_level4_ripple_alu_4bit_inst_1_result_and1),
    .result_and2(w_level4_ripple_alu_4bit_inst_1_result_and2),
    .result_and3(w_level4_ripple_alu_4bit_inst_1_result_and3),
    .result_or0(w_level4_ripple_alu_4bit_inst_1_result_or0),
    .result_or1(w_level4_ripple_alu_4bit_inst_1_result_or1),
    .result_or2(w_level4_ripple_alu_4bit_inst_1_result_or2),
    .result_or3(w_level4_ripple_alu_4bit_inst_1_result_or3),
    .carryout(w_level4_ripple_alu_4bit_inst_1_carryout),
    .subcarryout(w_level4_ripple_alu_4bit_inst_1_subcarryout)
);
level4_ripple_alu_4bit level4_ripple_alu_4bit_inst_2 (
    .a0(a4),
    .a1(a5),
    .a2(a6),
    .a3(a7),
    .b0(b4),
    .b1(b5),
    .b2(b6),
    .b3(b7),
    .carryin(w_level4_ripple_alu_4bit_inst_1_carryout),
    .subcarryin(w_level4_ripple_alu_4bit_inst_1_subcarryout),
    .result_add0(w_level4_ripple_alu_4bit_inst_2_result_add0),
    .result_add1(w_level4_ripple_alu_4bit_inst_2_result_add1),
    .result_add2(w_level4_ripple_alu_4bit_inst_2_result_add2),
    .result_add3(w_level4_ripple_alu_4bit_inst_2_result_add3),
    .result_sub0(w_level4_ripple_alu_4bit_inst_2_result_sub0),
    .result_sub1(w_level4_ripple_alu_4bit_inst_2_result_sub1),
    .result_sub2(w_level4_ripple_alu_4bit_inst_2_result_sub2),
    .result_sub3(w_level4_ripple_alu_4bit_inst_2_result_sub3),
    .result_and0(w_level4_ripple_alu_4bit_inst_2_result_and0),
    .result_and1(w_level4_ripple_alu_4bit_inst_2_result_and1),
    .result_and2(w_level4_ripple_alu_4bit_inst_2_result_and2),
    .result_and3(w_level4_ripple_alu_4bit_inst_2_result_and3),
    .result_or0(w_level4_ripple_alu_4bit_inst_2_result_or0),
    .result_or1(w_level4_ripple_alu_4bit_inst_2_result_or1),
    .result_or2(w_level4_ripple_alu_4bit_inst_2_result_or2),
    .result_or3(w_level4_ripple_alu_4bit_inst_2_result_or3),
    .carryout(w_level4_ripple_alu_4bit_inst_2_carryout),
    .subcarryout(w_level4_ripple_alu_4bit_inst_2_subcarryout)
);
assign aux_xor_3 = (a0 ^ b0);
assign aux_xor_4 = (a1 ^ b1);
assign aux_xor_5 = (a2 ^ b2);
assign aux_xor_6 = (a3 ^ b3);
assign aux_xor_7 = (a4 ^ b4);
assign aux_xor_8 = (a5 ^ b5);
assign aux_xor_9 = (a6 ^ b6);
assign aux_xor_10 = (a7 ^ b7);
assign aux_not_11 = ~a0;
assign aux_not_12 = ~a1;
assign aux_not_13 = ~a2;
assign aux_not_14 = ~a3;
assign aux_not_15 = ~a4;
assign aux_not_16 = ~a5;
assign aux_not_17 = ~a6;
assign aux_not_18 = ~a7;
level3_alu_selector_5way level3_alu_selector_5way_inst_19 (
    .result0(w_level4_ripple_alu_4bit_inst_1_result_add0),
    .result1(w_level4_ripple_alu_4bit_inst_1_result_sub0),
    .result2(w_level4_ripple_alu_4bit_inst_1_result_and0),
    .result3(w_level4_ripple_alu_4bit_inst_1_result_or0),
    .result4(aux_xor_3),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_19_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_20 (
    .result0(w_level4_ripple_alu_4bit_inst_1_result_add1),
    .result1(w_level4_ripple_alu_4bit_inst_1_result_sub1),
    .result2(w_level4_ripple_alu_4bit_inst_1_result_and1),
    .result3(w_level4_ripple_alu_4bit_inst_1_result_or1),
    .result4(aux_xor_4),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_20_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_21 (
    .result0(w_level4_ripple_alu_4bit_inst_1_result_add2),
    .result1(w_level4_ripple_alu_4bit_inst_1_result_sub2),
    .result2(w_level4_ripple_alu_4bit_inst_1_result_and2),
    .result3(w_level4_ripple_alu_4bit_inst_1_result_or2),
    .result4(aux_xor_5),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_21_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_22 (
    .result0(w_level4_ripple_alu_4bit_inst_1_result_add3),
    .result1(w_level4_ripple_alu_4bit_inst_1_result_sub3),
    .result2(w_level4_ripple_alu_4bit_inst_1_result_and3),
    .result3(w_level4_ripple_alu_4bit_inst_1_result_or3),
    .result4(aux_xor_6),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_22_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_23 (
    .result0(w_level4_ripple_alu_4bit_inst_2_result_add0),
    .result1(w_level4_ripple_alu_4bit_inst_2_result_sub0),
    .result2(w_level4_ripple_alu_4bit_inst_2_result_and0),
    .result3(w_level4_ripple_alu_4bit_inst_2_result_or0),
    .result4(aux_xor_7),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_23_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_24 (
    .result0(w_level4_ripple_alu_4bit_inst_2_result_add1),
    .result1(w_level4_ripple_alu_4bit_inst_2_result_sub1),
    .result2(w_level4_ripple_alu_4bit_inst_2_result_and1),
    .result3(w_level4_ripple_alu_4bit_inst_2_result_or1),
    .result4(aux_xor_8),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_24_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_25 (
    .result0(w_level4_ripple_alu_4bit_inst_2_result_add2),
    .result1(w_level4_ripple_alu_4bit_inst_2_result_sub2),
    .result2(w_level4_ripple_alu_4bit_inst_2_result_and2),
    .result3(w_level4_ripple_alu_4bit_inst_2_result_or2),
    .result4(aux_xor_9),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_25_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_26 (
    .result0(w_level4_ripple_alu_4bit_inst_2_result_add3),
    .result1(w_level4_ripple_alu_4bit_inst_2_result_sub3),
    .result2(w_level4_ripple_alu_4bit_inst_2_result_and3),
    .result3(w_level4_ripple_alu_4bit_inst_2_result_or3),
    .result4(aux_xor_10),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_26_out)
);
assign aux_not_27 = ~opcode1;
assign aux_not_28 = ~opcode0;
assign aux_and_29 = (opcode2 & aux_not_27);
assign aux_and_30 = (aux_and_29 & opcode0);
assign aux_and_31 = (opcode2 & opcode1);
assign aux_and_32 = (aux_and_31 & aux_not_28);
assign aux_and_33 = (opcode2 & opcode1);
assign aux_and_34 = (aux_and_33 & opcode0);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_30})
            1'd0: aux_mux_35 = w_level3_alu_selector_5way_inst_19_out;
            1'd1: aux_mux_35 = aux_not_11;
            default: aux_mux_35 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_32})
            1'd0: aux_mux_36 = a1;
            1'd1: aux_mux_36 = shlin;
            default: aux_mux_36 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_38})
            1'd0: aux_mux_37 = aux_mux_35;
            1'd1: aux_mux_37 = aux_mux_36;
            default: aux_mux_37 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_38 = (aux_and_32 | aux_and_34);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_30})
            1'd0: aux_mux_39 = w_level3_alu_selector_5way_inst_20_out;
            1'd1: aux_mux_39 = aux_not_12;
            default: aux_mux_39 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_32})
            1'd0: aux_mux_40 = a2;
            1'd1: aux_mux_40 = a0;
            default: aux_mux_40 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_42})
            1'd0: aux_mux_41 = aux_mux_39;
            1'd1: aux_mux_41 = aux_mux_40;
            default: aux_mux_41 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_42 = (aux_and_32 | aux_and_34);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_30})
            1'd0: aux_mux_43 = w_level3_alu_selector_5way_inst_21_out;
            1'd1: aux_mux_43 = aux_not_13;
            default: aux_mux_43 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_32})
            1'd0: aux_mux_44 = a3;
            1'd1: aux_mux_44 = a1;
            default: aux_mux_44 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_46})
            1'd0: aux_mux_45 = aux_mux_43;
            1'd1: aux_mux_45 = aux_mux_44;
            default: aux_mux_45 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_46 = (aux_and_32 | aux_and_34);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_30})
            1'd0: aux_mux_47 = w_level3_alu_selector_5way_inst_22_out;
            1'd1: aux_mux_47 = aux_not_14;
            default: aux_mux_47 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_32})
            1'd0: aux_mux_48 = a4;
            1'd1: aux_mux_48 = a2;
            default: aux_mux_48 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_50})
            1'd0: aux_mux_49 = aux_mux_47;
            1'd1: aux_mux_49 = aux_mux_48;
            default: aux_mux_49 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_50 = (aux_and_32 | aux_and_34);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_30})
            1'd0: aux_mux_51 = w_level3_alu_selector_5way_inst_23_out;
            1'd1: aux_mux_51 = aux_not_15;
            default: aux_mux_51 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_32})
            1'd0: aux_mux_52 = a5;
            1'd1: aux_mux_52 = a3;
            default: aux_mux_52 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_54})
            1'd0: aux_mux_53 = aux_mux_51;
            1'd1: aux_mux_53 = aux_mux_52;
            default: aux_mux_53 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_54 = (aux_and_32 | aux_and_34);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_30})
            1'd0: aux_mux_55 = w_level3_alu_selector_5way_inst_24_out;
            1'd1: aux_mux_55 = aux_not_16;
            default: aux_mux_55 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_32})
            1'd0: aux_mux_56 = a6;
            1'd1: aux_mux_56 = a4;
            default: aux_mux_56 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_58})
            1'd0: aux_mux_57 = aux_mux_55;
            1'd1: aux_mux_57 = aux_mux_56;
            default: aux_mux_57 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_58 = (aux_and_32 | aux_and_34);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_30})
            1'd0: aux_mux_59 = w_level3_alu_selector_5way_inst_25_out;
            1'd1: aux_mux_59 = aux_not_17;
            default: aux_mux_59 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_32})
            1'd0: aux_mux_60 = a7;
            1'd1: aux_mux_60 = a5;
            default: aux_mux_60 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_62})
            1'd0: aux_mux_61 = aux_mux_59;
            1'd1: aux_mux_61 = aux_mux_60;
            default: aux_mux_61 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_62 = (aux_and_32 | aux_and_34);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_30})
            1'd0: aux_mux_63 = w_level3_alu_selector_5way_inst_26_out;
            1'd1: aux_mux_63 = aux_not_18;
            default: aux_mux_63 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_32})
            1'd0: aux_mux_64 = shrin;
            1'd1: aux_mux_64 = a6;
            default: aux_mux_64 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_66})
            1'd0: aux_mux_65 = aux_mux_63;
            1'd1: aux_mux_65 = aux_mux_64;
            default: aux_mux_65 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_66 = (aux_and_32 | aux_and_34);
assign aux_nor_67 = ~(aux_mux_37 | aux_mux_41 | aux_mux_45 | aux_mux_49 | aux_mux_53 | aux_mux_57 | aux_mux_61 | aux_mux_65);

assign result0 = aux_mux_37;
assign result1 = aux_mux_41;
assign zero = aux_nor_67;
assign result2 = aux_mux_45;
assign negative = aux_mux_65;
assign result3 = aux_mux_49;
assign carry = w_level4_ripple_alu_4bit_inst_2_carryout;
assign result4 = aux_mux_53;
assign subcarryout = w_level4_ripple_alu_4bit_inst_2_subcarryout;
assign result5 = aux_mux_57;
assign result6 = aux_mux_61;
assign result7 = aux_mux_65;
endmodule

// Module for Datapath (generated from level7_execution_datapath.panda)
module level7_execution_datapath (
    input operanda0,
    input operanda1,
    input operanda2,
    input operanda3,
    input operanda4,
    input operanda5,
    input operanda6,
    input operanda7,
    input opcode0,
    input opcode1,
    input opcode2,
    input operandb0,
    input operandb1,
    input operandb2,
    input operandb3,
    input operandb4,
    input operandb5,
    input operandb6,
    input operandb7,
    output result0,
    output zero,
    output result1,
    output result2,
    output sign,
    output result3,
    output result4,
    output result5,
    output result6,
    output result7
);

// IC instance: ALU_8bit (level6_alu_8bit)
wire w_level6_alu_8bit_inst_1_result0;
wire w_level6_alu_8bit_inst_1_result1;
wire w_level6_alu_8bit_inst_1_zero;
wire w_level6_alu_8bit_inst_1_result2;
wire w_level6_alu_8bit_inst_1_negative;
wire w_level6_alu_8bit_inst_1_result3;
wire w_level6_alu_8bit_inst_1_carry;
wire w_level6_alu_8bit_inst_1_result4;
wire w_level6_alu_8bit_inst_1_subcarryout;
wire w_level6_alu_8bit_inst_1_result5;
wire w_level6_alu_8bit_inst_1_result6;
wire w_level6_alu_8bit_inst_1_result7;
wire aux_nor_2;
wire aux_nor_3;
wire aux_nor_4;
wire aux_nor_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;

// Internal logic
level6_alu_8bit level6_alu_8bit_inst_1 (
    .a0(operanda0),
    .a1(operanda1),
    .a2(operanda2),
    .a3(operanda3),
    .a4(operanda4),
    .a5(operanda5),
    .a6(operanda6),
    .a7(operanda7),
    .opcode0(opcode0),
    .opcode1(opcode1),
    .opcode2(opcode2),
    .b0(operandb0),
    .b1(operandb1),
    .b2(operandb2),
    .b3(operandb3),
    .b4(operandb4),
    .b5(operandb5),
    .b6(operandb6),
    .b7(operandb7),
    .carryin(1'b0),
    .subcarryin(1'b1),
    .shrin(1'b0),
    .shlin(1'b0),
    .result0(w_level6_alu_8bit_inst_1_result0),
    .result1(w_level6_alu_8bit_inst_1_result1),
    .zero(w_level6_alu_8bit_inst_1_zero),
    .result2(w_level6_alu_8bit_inst_1_result2),
    .negative(w_level6_alu_8bit_inst_1_negative),
    .result3(w_level6_alu_8bit_inst_1_result3),
    .carry(w_level6_alu_8bit_inst_1_carry),
    .result4(w_level6_alu_8bit_inst_1_result4),
    .subcarryout(w_level6_alu_8bit_inst_1_subcarryout),
    .result5(w_level6_alu_8bit_inst_1_result5),
    .result6(w_level6_alu_8bit_inst_1_result6),
    .result7(w_level6_alu_8bit_inst_1_result7)
);
assign aux_nor_2 = ~(w_level6_alu_8bit_inst_1_result0 | w_level6_alu_8bit_inst_1_result1);
assign aux_nor_3 = ~(w_level6_alu_8bit_inst_1_result2 | w_level6_alu_8bit_inst_1_result3);
assign aux_nor_4 = ~(w_level6_alu_8bit_inst_1_result4 | w_level6_alu_8bit_inst_1_result5);
assign aux_nor_5 = ~(w_level6_alu_8bit_inst_1_result6 | w_level6_alu_8bit_inst_1_result7);
assign aux_and_6 = (aux_nor_2 & aux_nor_4);
assign aux_and_7 = (aux_nor_3 & aux_nor_5);
assign aux_and_8 = (aux_and_6 & aux_and_7);

assign result0 = w_level6_alu_8bit_inst_1_result0;
assign zero = aux_and_8;
assign result1 = w_level6_alu_8bit_inst_1_result1;
assign result2 = w_level6_alu_8bit_inst_1_result2;
assign sign = w_level6_alu_8bit_inst_1_result7;
assign result3 = w_level6_alu_8bit_inst_1_result3;
assign result4 = w_level6_alu_8bit_inst_1_result4;
assign result5 = w_level6_alu_8bit_inst_1_result5;
assign result6 = w_level6_alu_8bit_inst_1_result6;
assign result7 = w_level6_alu_8bit_inst_1_result7;
endmodule

// Module for LEVEL8_EXECUTE_STAGE (generated from level8_execute_stage.panda)
module level8_execute_stage_ic (
    input operanda0,
    input operanda1,
    input operanda2,
    input operanda3,
    input operanda4,
    input operanda5,
    input operanda6,
    input operanda7,
    input aluop0,
    input operandb0,
    input operandb1,
    input operandb2,
    input operandb3,
    input operandb4,
    input operandb5,
    input operandb6,
    input operandb7,
    input aluop1,
    input aluop2,
    output result0,
    output zero,
    output result1,
    output sign,
    output result2,
    output result3,
    output result4,
    output result5,
    output result6,
    output result7
);

// IC instance: Datapath (level7_execution_datapath)
wire w_level7_execution_datapath_inst_1_result0;
wire w_level7_execution_datapath_inst_1_zero;
wire w_level7_execution_datapath_inst_1_result1;
wire w_level7_execution_datapath_inst_1_result2;
wire w_level7_execution_datapath_inst_1_sign;
wire w_level7_execution_datapath_inst_1_result3;
wire w_level7_execution_datapath_inst_1_result4;
wire w_level7_execution_datapath_inst_1_result5;
wire w_level7_execution_datapath_inst_1_result6;
wire w_level7_execution_datapath_inst_1_result7;

// Internal logic
level7_execution_datapath level7_execution_datapath_inst_1 (
    .operanda0(operanda0),
    .operanda1(operanda1),
    .operanda2(operanda2),
    .operanda3(operanda3),
    .operanda4(operanda4),
    .operanda5(operanda5),
    .operanda6(operanda6),
    .operanda7(operanda7),
    .opcode0(aluop0),
    .opcode1(aluop1),
    .opcode2(aluop2),
    .operandb0(operandb0),
    .operandb1(operandb1),
    .operandb2(operandb2),
    .operandb3(operandb3),
    .operandb4(operandb4),
    .operandb5(operandb5),
    .operandb6(operandb6),
    .operandb7(operandb7),
    .result0(w_level7_execution_datapath_inst_1_result0),
    .zero(w_level7_execution_datapath_inst_1_zero),
    .result1(w_level7_execution_datapath_inst_1_result1),
    .result2(w_level7_execution_datapath_inst_1_result2),
    .sign(w_level7_execution_datapath_inst_1_sign),
    .result3(w_level7_execution_datapath_inst_1_result3),
    .result4(w_level7_execution_datapath_inst_1_result4),
    .result5(w_level7_execution_datapath_inst_1_result5),
    .result6(w_level7_execution_datapath_inst_1_result6),
    .result7(w_level7_execution_datapath_inst_1_result7)
);

assign result0 = w_level7_execution_datapath_inst_1_result0;
assign zero = w_level7_execution_datapath_inst_1_zero;
assign result1 = w_level7_execution_datapath_inst_1_result1;
assign sign = w_level7_execution_datapath_inst_1_sign;
assign result2 = w_level7_execution_datapath_inst_1_result2;
assign result3 = w_level7_execution_datapath_inst_1_result3;
assign result4 = w_level7_execution_datapath_inst_1_result4;
assign result5 = w_level7_execution_datapath_inst_1_result5;
assign result6 = w_level7_execution_datapath_inst_1_result6;
assign result7 = w_level7_execution_datapath_inst_1_result7;
endmodule

module level8_execute_stage (
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
input input_switch15,
input input_switch16,
input input_switch17,
input input_switch18,
input input_switch19,

/* ========= Outputs ========== */
output led21_1,
output led22_1,
output led23_1,
output led24_1,
output led25_1,
output led26_1,
output led27_1,
output led28_1,
output led29_1,
output led30_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL8_EXECUTE_STAGE (level8_execute_stage_ic)
wire w_level8_execute_stage_ic_inst_1_result0;
wire w_level8_execute_stage_ic_inst_1_zero;
wire w_level8_execute_stage_ic_inst_1_result1;
wire w_level8_execute_stage_ic_inst_1_sign;
wire w_level8_execute_stage_ic_inst_1_result2;
wire w_level8_execute_stage_ic_inst_1_result3;
wire w_level8_execute_stage_ic_inst_1_result4;
wire w_level8_execute_stage_ic_inst_1_result5;
wire w_level8_execute_stage_ic_inst_1_result6;
wire w_level8_execute_stage_ic_inst_1_result7;


// Assigning aux variables. //
level8_execute_stage_ic level8_execute_stage_ic_inst_1 (
    .operanda0(input_switch1),
    .operanda1(input_switch2),
    .operanda2(input_switch3),
    .operanda3(input_switch4),
    .operanda4(input_switch5),
    .operanda5(input_switch6),
    .operanda6(input_switch7),
    .operanda7(input_switch8),
    .aluop0(input_switch9),
    .operandb0(input_switch10),
    .operandb1(input_switch11),
    .operandb2(input_switch12),
    .operandb3(input_switch13),
    .operandb4(input_switch14),
    .operandb5(input_switch15),
    .operandb6(input_switch16),
    .operandb7(input_switch17),
    .aluop1(input_switch18),
    .aluop2(input_switch19),
    .result0(w_level8_execute_stage_ic_inst_1_result0),
    .zero(w_level8_execute_stage_ic_inst_1_zero),
    .result1(w_level8_execute_stage_ic_inst_1_result1),
    .sign(w_level8_execute_stage_ic_inst_1_sign),
    .result2(w_level8_execute_stage_ic_inst_1_result2),
    .result3(w_level8_execute_stage_ic_inst_1_result3),
    .result4(w_level8_execute_stage_ic_inst_1_result4),
    .result5(w_level8_execute_stage_ic_inst_1_result5),
    .result6(w_level8_execute_stage_ic_inst_1_result6),
    .result7(w_level8_execute_stage_ic_inst_1_result7)
);

// Writing output data. //
assign led21_1 = w_level8_execute_stage_ic_inst_1_result0;
assign led22_1 = w_level8_execute_stage_ic_inst_1_zero;
assign led23_1 = w_level8_execute_stage_ic_inst_1_result1;
assign led24_1 = w_level8_execute_stage_ic_inst_1_sign;
assign led25_1 = w_level8_execute_stage_ic_inst_1_result2;
assign led26_1 = w_level8_execute_stage_ic_inst_1_result3;
assign led27_1 = w_level8_execute_stage_ic_inst_1_result4;
assign led28_1 = w_level8_execute_stage_ic_inst_1_result5;
assign led29_1 = w_level8_execute_stage_ic_inst_1_result6;
assign led30_1 = w_level8_execute_stage_ic_inst_1_result7;
endmodule
