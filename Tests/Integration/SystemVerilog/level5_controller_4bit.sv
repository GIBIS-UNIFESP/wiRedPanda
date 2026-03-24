// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL5_CONTROLLER_4BIT (generated from level5_controller_4bit.panda)
module level5_controller_4bit_ic (
    input opcode0,
    input opcode1,
    input opcode2,
    input opcode3,
    output ctrl0,
    output ctrl1,
    output ctrl2,
    output ctrl3
);

wire aux_or_1;

// Internal logic
assign aux_or_1 = (opcode3 | opcode2);

assign ctrl0 = opcode0;
assign ctrl1 = opcode1;
assign ctrl2 = aux_or_1;
assign ctrl3 = opcode3;
endmodule

module level5_controller_4bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,

/* ========= Outputs ========== */
output led6_1,
output led7_1,
output led8_1,
output led9_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL5_CONTROLLER_4BIT (level5_controller_4bit_ic)
wire w_level5_controller_4bit_ic_inst_1_ctrl0;
wire w_level5_controller_4bit_ic_inst_1_ctrl1;
wire w_level5_controller_4bit_ic_inst_1_ctrl2;
wire w_level5_controller_4bit_ic_inst_1_ctrl3;


// Assigning aux variables. //
level5_controller_4bit_ic level5_controller_4bit_ic_inst_1 (
    .opcode0(input_switch1),
    .opcode1(input_switch2),
    .opcode2(input_switch3),
    .opcode3(input_switch4),
    .ctrl0(w_level5_controller_4bit_ic_inst_1_ctrl0),
    .ctrl1(w_level5_controller_4bit_ic_inst_1_ctrl1),
    .ctrl2(w_level5_controller_4bit_ic_inst_1_ctrl2),
    .ctrl3(w_level5_controller_4bit_ic_inst_1_ctrl3)
);

// Writing output data. //
assign led6_1 = w_level5_controller_4bit_ic_inst_1_ctrl0;
assign led7_1 = w_level5_controller_4bit_ic_inst_1_ctrl1;
assign led8_1 = w_level5_controller_4bit_ic_inst_1_ctrl2;
assign led9_1 = w_level5_controller_4bit_ic_inst_1_ctrl3;
endmodule
