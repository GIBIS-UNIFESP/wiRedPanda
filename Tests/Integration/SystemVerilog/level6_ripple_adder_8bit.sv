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

// Module for LEVEL6_RIPPLE_ADDER_8BIT (generated from level6_ripple_adder_8bit.panda)
module level6_ripple_adder_8bit_ic (
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

module level6_ripple_adder_8bit (
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

/* ========= Outputs ========== */
output led19_1,
output led20_1,
output led21_1,
output led22_1,
output led23_1,
output led24_1,
output led25_1,
output led26_1,
output led27_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL6_RIPPLE_ADDER_8BIT (level6_ripple_adder_8bit_ic)
wire w_level6_ripple_adder_8bit_ic_inst_1_sum0;
wire w_level6_ripple_adder_8bit_ic_inst_1_sum1;
wire w_level6_ripple_adder_8bit_ic_inst_1_sum2;
wire w_level6_ripple_adder_8bit_ic_inst_1_sum3;
wire w_level6_ripple_adder_8bit_ic_inst_1_sum4;
wire w_level6_ripple_adder_8bit_ic_inst_1_sum5;
wire w_level6_ripple_adder_8bit_ic_inst_1_sum6;
wire w_level6_ripple_adder_8bit_ic_inst_1_sum7;
wire w_level6_ripple_adder_8bit_ic_inst_1_carryout;


// Assigning aux variables. //
level6_ripple_adder_8bit_ic level6_ripple_adder_8bit_ic_inst_1 (
    .a0(input_switch1),
    .b0(input_switch2),
    .carryin(input_switch3),
    .a1(input_switch4),
    .b1(input_switch5),
    .a2(input_switch6),
    .b2(input_switch7),
    .a3(input_switch8),
    .b3(input_switch9),
    .a4(input_switch10),
    .b4(input_switch11),
    .a5(input_switch12),
    .b5(input_switch13),
    .a6(input_switch14),
    .b6(input_switch15),
    .a7(input_switch16),
    .b7(input_switch17),
    .sum0(w_level6_ripple_adder_8bit_ic_inst_1_sum0),
    .sum1(w_level6_ripple_adder_8bit_ic_inst_1_sum1),
    .sum2(w_level6_ripple_adder_8bit_ic_inst_1_sum2),
    .sum3(w_level6_ripple_adder_8bit_ic_inst_1_sum3),
    .sum4(w_level6_ripple_adder_8bit_ic_inst_1_sum4),
    .sum5(w_level6_ripple_adder_8bit_ic_inst_1_sum5),
    .sum6(w_level6_ripple_adder_8bit_ic_inst_1_sum6),
    .sum7(w_level6_ripple_adder_8bit_ic_inst_1_sum7),
    .carryout(w_level6_ripple_adder_8bit_ic_inst_1_carryout)
);

// Writing output data. //
assign led19_1 = w_level6_ripple_adder_8bit_ic_inst_1_sum0;
assign led20_1 = w_level6_ripple_adder_8bit_ic_inst_1_sum1;
assign led21_1 = w_level6_ripple_adder_8bit_ic_inst_1_sum2;
assign led22_1 = w_level6_ripple_adder_8bit_ic_inst_1_sum3;
assign led23_1 = w_level6_ripple_adder_8bit_ic_inst_1_sum4;
assign led24_1 = w_level6_ripple_adder_8bit_ic_inst_1_sum5;
assign led25_1 = w_level6_ripple_adder_8bit_ic_inst_1_sum6;
assign led26_1 = w_level6_ripple_adder_8bit_ic_inst_1_sum7;
assign led27_1 = w_level6_ripple_adder_8bit_ic_inst_1_carryout;
endmodule
