// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Decoder4to16 (generated from level2_decoder_4to16.panda)
module level2_decoder_4to16 (
    input addr0,
    input addr1,
    input addr2,
    input addr3,
    output out0,
    output out1,
    output out2,
    output out3,
    output out4,
    output out5,
    output out6,
    output out7,
    output out8,
    output out9,
    output out10,
    output out11,
    output out12,
    output out13,
    output out14,
    output out15
);

wire aux_not_1;
wire aux_not_2;
wire aux_not_3;
wire aux_not_4;
wire aux_and_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;
wire aux_and_9;
wire aux_and_10;
wire aux_and_11;
wire aux_and_12;
wire aux_and_13;
wire aux_and_14;
wire aux_and_15;
wire aux_and_16;
wire aux_and_17;
wire aux_and_18;
wire aux_and_19;
wire aux_and_20;

// Internal logic
assign aux_not_1 = ~addr0;
assign aux_not_2 = ~addr1;
assign aux_not_3 = ~addr2;
assign aux_not_4 = ~addr3;
assign aux_and_5 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4);
assign aux_and_6 = (addr0 & aux_not_2 & aux_not_3 & aux_not_4);
assign aux_and_7 = (aux_not_1 & addr1 & aux_not_3 & aux_not_4);
assign aux_and_8 = (addr0 & addr1 & aux_not_3 & aux_not_4);
assign aux_and_9 = (aux_not_1 & aux_not_2 & addr2 & aux_not_4);
assign aux_and_10 = (addr0 & aux_not_2 & addr2 & aux_not_4);
assign aux_and_11 = (aux_not_1 & addr1 & addr2 & aux_not_4);
assign aux_and_12 = (addr0 & addr1 & addr2 & aux_not_4);
assign aux_and_13 = (aux_not_1 & aux_not_2 & aux_not_3 & addr3);
assign aux_and_14 = (addr0 & aux_not_2 & aux_not_3 & addr3);
assign aux_and_15 = (aux_not_1 & addr1 & aux_not_3 & addr3);
assign aux_and_16 = (addr0 & addr1 & aux_not_3 & addr3);
assign aux_and_17 = (aux_not_1 & aux_not_2 & addr2 & addr3);
assign aux_and_18 = (addr0 & aux_not_2 & addr2 & addr3);
assign aux_and_19 = (aux_not_1 & addr1 & addr2 & addr3);
assign aux_and_20 = (addr0 & addr1 & addr2 & addr3);

assign out0 = aux_and_5;
assign out1 = aux_and_6;
assign out2 = aux_and_7;
assign out3 = aux_and_8;
assign out4 = aux_and_9;
assign out5 = aux_and_10;
assign out6 = aux_and_11;
assign out7 = aux_and_12;
assign out8 = aux_and_13;
assign out9 = aux_and_14;
assign out10 = aux_and_15;
assign out11 = aux_and_16;
assign out12 = aux_and_17;
assign out13 = aux_and_18;
assign out14 = aux_and_19;
assign out15 = aux_and_20;
endmodule

// Module for LEVEL5_INSTRUCTION_DECODER_4BIT (generated from level5_instruction_decoder_4bit.panda)
module level5_instruction_decoder_4bit_ic (
    input instr0,
    input instr1,
    input instr2,
    input instr3,
    output op0,
    output op1,
    output op2,
    output op3,
    output op4,
    output op5,
    output op6,
    output op7,
    output op8,
    output op9,
    output op10,
    output op11,
    output op12,
    output op13,
    output op14,
    output op15
);

// IC instance: Decoder4to16 (level2_decoder_4to16)
wire w_level2_decoder_4to16_inst_1_out0;
wire w_level2_decoder_4to16_inst_1_out1;
wire w_level2_decoder_4to16_inst_1_out2;
wire w_level2_decoder_4to16_inst_1_out3;
wire w_level2_decoder_4to16_inst_1_out4;
wire w_level2_decoder_4to16_inst_1_out5;
wire w_level2_decoder_4to16_inst_1_out6;
wire w_level2_decoder_4to16_inst_1_out7;
wire w_level2_decoder_4to16_inst_1_out8;
wire w_level2_decoder_4to16_inst_1_out9;
wire w_level2_decoder_4to16_inst_1_out10;
wire w_level2_decoder_4to16_inst_1_out11;
wire w_level2_decoder_4to16_inst_1_out12;
wire w_level2_decoder_4to16_inst_1_out13;
wire w_level2_decoder_4to16_inst_1_out14;
wire w_level2_decoder_4to16_inst_1_out15;

// Internal logic
level2_decoder_4to16 level2_decoder_4to16_inst_1 (
    .addr0(instr0),
    .addr1(instr1),
    .addr2(instr2),
    .addr3(instr3),
    .out0(w_level2_decoder_4to16_inst_1_out0),
    .out1(w_level2_decoder_4to16_inst_1_out1),
    .out2(w_level2_decoder_4to16_inst_1_out2),
    .out3(w_level2_decoder_4to16_inst_1_out3),
    .out4(w_level2_decoder_4to16_inst_1_out4),
    .out5(w_level2_decoder_4to16_inst_1_out5),
    .out6(w_level2_decoder_4to16_inst_1_out6),
    .out7(w_level2_decoder_4to16_inst_1_out7),
    .out8(w_level2_decoder_4to16_inst_1_out8),
    .out9(w_level2_decoder_4to16_inst_1_out9),
    .out10(w_level2_decoder_4to16_inst_1_out10),
    .out11(w_level2_decoder_4to16_inst_1_out11),
    .out12(w_level2_decoder_4to16_inst_1_out12),
    .out13(w_level2_decoder_4to16_inst_1_out13),
    .out14(w_level2_decoder_4to16_inst_1_out14),
    .out15(w_level2_decoder_4to16_inst_1_out15)
);

assign op0 = w_level2_decoder_4to16_inst_1_out0;
assign op1 = w_level2_decoder_4to16_inst_1_out1;
assign op2 = w_level2_decoder_4to16_inst_1_out2;
assign op3 = w_level2_decoder_4to16_inst_1_out3;
assign op4 = w_level2_decoder_4to16_inst_1_out4;
assign op5 = w_level2_decoder_4to16_inst_1_out5;
assign op6 = w_level2_decoder_4to16_inst_1_out6;
assign op7 = w_level2_decoder_4to16_inst_1_out7;
assign op8 = w_level2_decoder_4to16_inst_1_out8;
assign op9 = w_level2_decoder_4to16_inst_1_out9;
assign op10 = w_level2_decoder_4to16_inst_1_out10;
assign op11 = w_level2_decoder_4to16_inst_1_out11;
assign op12 = w_level2_decoder_4to16_inst_1_out12;
assign op13 = w_level2_decoder_4to16_inst_1_out13;
assign op14 = w_level2_decoder_4to16_inst_1_out14;
assign op15 = w_level2_decoder_4to16_inst_1_out15;
endmodule

module level5_instruction_decoder_4bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,

/* ========= Outputs ========== */
output led6_1,
output led7_1,
output led8_1,
output led9_1,
output led10_1,
output led11_1,
output led12_1,
output led13_1,
output led14_1,
output led15_1,
output led16_1,
output led17_1,
output led18_1,
output led19_1,
output led20_1,
output led21_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL5_INSTRUCTION_DECODER_4BIT (level5_instruction_decoder_4bit_ic)
wire w_level5_instruction_decoder_4bit_ic_inst_1_op0;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op1;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op2;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op3;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op4;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op5;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op6;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op7;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op8;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op9;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op10;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op11;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op12;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op13;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op14;
wire w_level5_instruction_decoder_4bit_ic_inst_1_op15;


// Assigning aux variables. //
level5_instruction_decoder_4bit_ic level5_instruction_decoder_4bit_ic_inst_1 (
    .instr0(input_switch1),
    .instr1(input_switch2),
    .instr2(input_switch3),
    .instr3(input_switch4),
    .op0(w_level5_instruction_decoder_4bit_ic_inst_1_op0),
    .op1(w_level5_instruction_decoder_4bit_ic_inst_1_op1),
    .op2(w_level5_instruction_decoder_4bit_ic_inst_1_op2),
    .op3(w_level5_instruction_decoder_4bit_ic_inst_1_op3),
    .op4(w_level5_instruction_decoder_4bit_ic_inst_1_op4),
    .op5(w_level5_instruction_decoder_4bit_ic_inst_1_op5),
    .op6(w_level5_instruction_decoder_4bit_ic_inst_1_op6),
    .op7(w_level5_instruction_decoder_4bit_ic_inst_1_op7),
    .op8(w_level5_instruction_decoder_4bit_ic_inst_1_op8),
    .op9(w_level5_instruction_decoder_4bit_ic_inst_1_op9),
    .op10(w_level5_instruction_decoder_4bit_ic_inst_1_op10),
    .op11(w_level5_instruction_decoder_4bit_ic_inst_1_op11),
    .op12(w_level5_instruction_decoder_4bit_ic_inst_1_op12),
    .op13(w_level5_instruction_decoder_4bit_ic_inst_1_op13),
    .op14(w_level5_instruction_decoder_4bit_ic_inst_1_op14),
    .op15(w_level5_instruction_decoder_4bit_ic_inst_1_op15)
);

// Writing output data. //
assign led6_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op0;
assign led7_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op1;
assign led8_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op2;
assign led9_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op3;
assign led10_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op4;
assign led11_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op5;
assign led12_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op6;
assign led13_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op7;
assign led14_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op8;
assign led15_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op9;
assign led16_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op10;
assign led17_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op11;
assign led18_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op12;
assign led19_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op13;
assign led20_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op14;
assign led21_1 = w_level5_instruction_decoder_4bit_ic_inst_1_op15;
endmodule
