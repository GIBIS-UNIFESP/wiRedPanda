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

// Module for LEVEL4_RIPPLE_ADDER_4BIT (generated from level4_ripple_adder_4bit.panda)
module level4_ripple_adder_4bit_ic (
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

module level4_ripple_adder_4bit (
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

/* ========= Outputs ========== */
output led11_1,
output led12_1,
output led13_1,
output led14_1,
output led15_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_RIPPLE_ADDER_4BIT (level4_ripple_adder_4bit_ic)
wire w_level4_ripple_adder_4bit_ic_inst_1_sum0;
wire w_level4_ripple_adder_4bit_ic_inst_1_sum1;
wire w_level4_ripple_adder_4bit_ic_inst_1_sum2;
wire w_level4_ripple_adder_4bit_ic_inst_1_carryout;
wire w_level4_ripple_adder_4bit_ic_inst_1_sum3;


// Assigning aux variables. //
level4_ripple_adder_4bit_ic level4_ripple_adder_4bit_ic_inst_1 (
    .a0(input_switch1),
    .a1(input_switch2),
    .b0(input_switch3),
    .a2(input_switch4),
    .b1(input_switch5),
    .carryin(input_switch6),
    .a3(input_switch7),
    .b2(input_switch8),
    .b3(input_switch9),
    .sum0(w_level4_ripple_adder_4bit_ic_inst_1_sum0),
    .sum1(w_level4_ripple_adder_4bit_ic_inst_1_sum1),
    .sum2(w_level4_ripple_adder_4bit_ic_inst_1_sum2),
    .carryout(w_level4_ripple_adder_4bit_ic_inst_1_carryout),
    .sum3(w_level4_ripple_adder_4bit_ic_inst_1_sum3)
);

// Writing output data. //
assign led11_1 = w_level4_ripple_adder_4bit_ic_inst_1_sum0;
assign led12_1 = w_level4_ripple_adder_4bit_ic_inst_1_sum1;
assign led13_1 = w_level4_ripple_adder_4bit_ic_inst_1_sum2;
assign led14_1 = w_level4_ripple_adder_4bit_ic_inst_1_carryout;
assign led15_1 = w_level4_ripple_adder_4bit_ic_inst_1_sum3;
endmodule
