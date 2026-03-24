// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL8_DECODE_STAGE (generated from level8_decode_stage.panda)
module level8_decode_stage_ic (
    input opcode0,
    input opcode1,
    input opcode2,
    input opcode3,
    input opcode4,
    input clock,
    input reset,
    output aluop0,
    output regwrite,
    output memread,
    output memwrite,
    output aluop1,
    output aluop2
);

wire aux_not_1;
wire aux_not_2;
wire aux_and_3;
wire aux_and_4;

// Internal logic
assign aux_not_1 = ~opcode4;
assign aux_not_2 = ~opcode3;
assign aux_and_3 = (opcode4 & aux_not_2);
assign aux_and_4 = (opcode4 & opcode3);

assign aluop0 = opcode0;
assign regwrite = aux_not_1;
assign memread = aux_and_3;
assign memwrite = aux_and_4;
assign aluop1 = opcode1;
assign aluop2 = opcode2;
endmodule

module level8_decode_stage (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,

/* ========= Outputs ========== */
output led9_1,
output led10_1,
output led11_1,
output led12_1,
output led13_1,
output led14_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL8_DECODE_STAGE (level8_decode_stage_ic)
wire w_level8_decode_stage_ic_inst_1_aluop0;
wire w_level8_decode_stage_ic_inst_1_regwrite;
wire w_level8_decode_stage_ic_inst_1_memread;
wire w_level8_decode_stage_ic_inst_1_memwrite;
wire w_level8_decode_stage_ic_inst_1_aluop1;
wire w_level8_decode_stage_ic_inst_1_aluop2;


// Assigning aux variables. //
level8_decode_stage_ic level8_decode_stage_ic_inst_1 (
    .opcode0(input_switch1),
    .opcode1(input_switch2),
    .opcode2(input_switch3),
    .opcode3(input_switch4),
    .opcode4(input_switch5),
    .clock(input_switch6),
    .reset(input_switch7),
    .aluop0(w_level8_decode_stage_ic_inst_1_aluop0),
    .regwrite(w_level8_decode_stage_ic_inst_1_regwrite),
    .memread(w_level8_decode_stage_ic_inst_1_memread),
    .memwrite(w_level8_decode_stage_ic_inst_1_memwrite),
    .aluop1(w_level8_decode_stage_ic_inst_1_aluop1),
    .aluop2(w_level8_decode_stage_ic_inst_1_aluop2)
);

// Writing output data. //
assign led9_1 = w_level8_decode_stage_ic_inst_1_aluop0;
assign led10_1 = w_level8_decode_stage_ic_inst_1_regwrite;
assign led11_1 = w_level8_decode_stage_ic_inst_1_memread;
assign led12_1 = w_level8_decode_stage_ic_inst_1_memwrite;
assign led13_1 = w_level8_decode_stage_ic_inst_1_aluop1;
assign led14_1 = w_level8_decode_stage_ic_inst_1_aluop2;
endmodule
