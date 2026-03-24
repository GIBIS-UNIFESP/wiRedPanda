// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Reg[0] (generated from level3_register_1bit.panda)
module level3_register_1bit (
    input data,
    input clock,
    input writeenable,
    input reset,
    output q,
    output notq
);

wire aux_not_1;
wire aux_not_2;
reg aux_mux_3 = 1'b0;
reg aux_d_flip_flop_4_0_q = 1'b0;
reg aux_d_flip_flop_4_1_q = 1'b1;

// Internal logic
assign aux_not_1 = ~reset;
assign aux_not_2 = ~writeenable;
    //Multiplexer
    always @(*)
    begin
        case({aux_not_2})
            1'd0: aux_mux_3 = data;
            1'd1: aux_mux_3 = aux_d_flip_flop_4_0_q;
            default: aux_mux_3 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_4_0_q <= 1'b0;
            aux_d_flip_flop_4_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_4_0_q <= aux_mux_3;
            aux_d_flip_flop_4_1_q <= ~aux_mux_3;
        end
    end
    //End of D FlipFlop

assign q = aux_d_flip_flop_4_0_q;
assign notq = aux_d_flip_flop_4_1_q;
endmodule

// Module for InstructionReg (generated from level6_register_8bit.panda)
module level6_register_8bit (
    input data0,
    input clock,
    input data1,
    input writeenable,
    input data2,
    input reset,
    input data3,
    input data4,
    input data5,
    input data6,
    input data7,
    output q0,
    output q1,
    output q2,
    output q3,
    output q4,
    output q5,
    output q6,
    output q7
);

// IC instance: Reg[0] (level3_register_1bit)
wire w_level3_register_1bit_inst_1_q;
wire w_level3_register_1bit_inst_1_notq;
// IC instance: Reg[1] (level3_register_1bit)
wire w_level3_register_1bit_inst_2_q;
wire w_level3_register_1bit_inst_2_notq;
// IC instance: Reg[2] (level3_register_1bit)
wire w_level3_register_1bit_inst_3_q;
wire w_level3_register_1bit_inst_3_notq;
// IC instance: Reg[3] (level3_register_1bit)
wire w_level3_register_1bit_inst_4_q;
wire w_level3_register_1bit_inst_4_notq;
// IC instance: Reg[4] (level3_register_1bit)
wire w_level3_register_1bit_inst_5_q;
wire w_level3_register_1bit_inst_5_notq;
// IC instance: Reg[5] (level3_register_1bit)
wire w_level3_register_1bit_inst_6_q;
wire w_level3_register_1bit_inst_6_notq;
// IC instance: Reg[6] (level3_register_1bit)
wire w_level3_register_1bit_inst_7_q;
wire w_level3_register_1bit_inst_7_notq;
// IC instance: Reg[7] (level3_register_1bit)
wire w_level3_register_1bit_inst_8_q;
wire w_level3_register_1bit_inst_8_notq;

// Internal logic
level3_register_1bit level3_register_1bit_inst_1 (
    .data(data0),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_1_q),
    .notq(w_level3_register_1bit_inst_1_notq)
);
level3_register_1bit level3_register_1bit_inst_2 (
    .data(data1),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_2_q),
    .notq(w_level3_register_1bit_inst_2_notq)
);
level3_register_1bit level3_register_1bit_inst_3 (
    .data(data2),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_3_q),
    .notq(w_level3_register_1bit_inst_3_notq)
);
level3_register_1bit level3_register_1bit_inst_4 (
    .data(data3),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_4_q),
    .notq(w_level3_register_1bit_inst_4_notq)
);
level3_register_1bit level3_register_1bit_inst_5 (
    .data(data4),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_5_q),
    .notq(w_level3_register_1bit_inst_5_notq)
);
level3_register_1bit level3_register_1bit_inst_6 (
    .data(data5),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_6_q),
    .notq(w_level3_register_1bit_inst_6_notq)
);
level3_register_1bit level3_register_1bit_inst_7 (
    .data(data6),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_7_q),
    .notq(w_level3_register_1bit_inst_7_notq)
);
level3_register_1bit level3_register_1bit_inst_8 (
    .data(data7),
    .clock(clock),
    .writeenable(writeenable),
    .reset(reset),
    .q(w_level3_register_1bit_inst_8_q),
    .notq(w_level3_register_1bit_inst_8_notq)
);

assign q0 = w_level3_register_1bit_inst_1_q;
assign q1 = w_level3_register_1bit_inst_2_q;
assign q2 = w_level3_register_1bit_inst_3_q;
assign q3 = w_level3_register_1bit_inst_4_q;
assign q4 = w_level3_register_1bit_inst_5_q;
assign q5 = w_level3_register_1bit_inst_6_q;
assign q6 = w_level3_register_1bit_inst_7_q;
assign q7 = w_level3_register_1bit_inst_8_q;
endmodule

// Module for LEVEL7_INSTRUCTION_REGISTER_8BIT (generated from level7_instruction_register_8bit.panda)
module level7_instruction_register_8bit_ic (
    input data0,
    input data1,
    input data2,
    input data3,
    input data4,
    input data5,
    input data6,
    input data7,
    input clock,
    input load,
    input reset,
    output instruction0,
    output instruction1,
    output instruction2,
    output instruction3,
    output instruction4,
    output instruction5,
    output instruction6,
    output instruction7,
    output opcode0,
    output opcode1,
    output opcode2,
    output opcode3,
    output opcode4,
    output registeraddr0,
    output registeraddr1,
    output registeraddr2
);

// IC instance: InstructionReg (level6_register_8bit)
wire w_level6_register_8bit_inst_1_q0;
wire w_level6_register_8bit_inst_1_q1;
wire w_level6_register_8bit_inst_1_q2;
wire w_level6_register_8bit_inst_1_q3;
wire w_level6_register_8bit_inst_1_q4;
wire w_level6_register_8bit_inst_1_q5;
wire w_level6_register_8bit_inst_1_q6;
wire w_level6_register_8bit_inst_1_q7;

// Internal logic
level6_register_8bit level6_register_8bit_inst_1 (
    .data0(data0),
    .clock(clock),
    .data1(data1),
    .writeenable(load),
    .data2(data2),
    .reset(reset),
    .data3(data3),
    .data4(data4),
    .data5(data5),
    .data6(data6),
    .data7(data7),
    .q0(w_level6_register_8bit_inst_1_q0),
    .q1(w_level6_register_8bit_inst_1_q1),
    .q2(w_level6_register_8bit_inst_1_q2),
    .q3(w_level6_register_8bit_inst_1_q3),
    .q4(w_level6_register_8bit_inst_1_q4),
    .q5(w_level6_register_8bit_inst_1_q5),
    .q6(w_level6_register_8bit_inst_1_q6),
    .q7(w_level6_register_8bit_inst_1_q7)
);

assign instruction0 = w_level6_register_8bit_inst_1_q0;
assign instruction1 = w_level6_register_8bit_inst_1_q1;
assign instruction2 = w_level6_register_8bit_inst_1_q2;
assign instruction3 = w_level6_register_8bit_inst_1_q3;
assign instruction4 = w_level6_register_8bit_inst_1_q4;
assign instruction5 = w_level6_register_8bit_inst_1_q5;
assign instruction6 = w_level6_register_8bit_inst_1_q6;
assign instruction7 = w_level6_register_8bit_inst_1_q7;
assign opcode0 = w_level6_register_8bit_inst_1_q3;
assign opcode1 = w_level6_register_8bit_inst_1_q4;
assign opcode2 = w_level6_register_8bit_inst_1_q5;
assign opcode3 = w_level6_register_8bit_inst_1_q6;
assign opcode4 = w_level6_register_8bit_inst_1_q7;
assign registeraddr0 = w_level6_register_8bit_inst_1_q0;
assign registeraddr1 = w_level6_register_8bit_inst_1_q1;
assign registeraddr2 = w_level6_register_8bit_inst_1_q2;
endmodule

module level7_instruction_register_8bit (
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

/* ========= Outputs ========== */
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
output led28_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL7_INSTRUCTION_REGISTER_8BIT (level7_instruction_register_8bit_ic)
wire w_level7_instruction_register_8bit_ic_inst_1_instruction0;
wire w_level7_instruction_register_8bit_ic_inst_1_instruction1;
wire w_level7_instruction_register_8bit_ic_inst_1_instruction2;
wire w_level7_instruction_register_8bit_ic_inst_1_instruction3;
wire w_level7_instruction_register_8bit_ic_inst_1_instruction4;
wire w_level7_instruction_register_8bit_ic_inst_1_instruction5;
wire w_level7_instruction_register_8bit_ic_inst_1_instruction6;
wire w_level7_instruction_register_8bit_ic_inst_1_instruction7;
wire w_level7_instruction_register_8bit_ic_inst_1_opcode0;
wire w_level7_instruction_register_8bit_ic_inst_1_opcode1;
wire w_level7_instruction_register_8bit_ic_inst_1_opcode2;
wire w_level7_instruction_register_8bit_ic_inst_1_opcode3;
wire w_level7_instruction_register_8bit_ic_inst_1_opcode4;
wire w_level7_instruction_register_8bit_ic_inst_1_registeraddr0;
wire w_level7_instruction_register_8bit_ic_inst_1_registeraddr1;
wire w_level7_instruction_register_8bit_ic_inst_1_registeraddr2;


// Assigning aux variables. //
level7_instruction_register_8bit_ic level7_instruction_register_8bit_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .data2(input_switch3),
    .data3(input_switch4),
    .data4(input_switch5),
    .data5(input_switch6),
    .data6(input_switch7),
    .data7(input_switch8),
    .clock(input_switch9),
    .load(input_switch10),
    .reset(input_switch11),
    .instruction0(w_level7_instruction_register_8bit_ic_inst_1_instruction0),
    .instruction1(w_level7_instruction_register_8bit_ic_inst_1_instruction1),
    .instruction2(w_level7_instruction_register_8bit_ic_inst_1_instruction2),
    .instruction3(w_level7_instruction_register_8bit_ic_inst_1_instruction3),
    .instruction4(w_level7_instruction_register_8bit_ic_inst_1_instruction4),
    .instruction5(w_level7_instruction_register_8bit_ic_inst_1_instruction5),
    .instruction6(w_level7_instruction_register_8bit_ic_inst_1_instruction6),
    .instruction7(w_level7_instruction_register_8bit_ic_inst_1_instruction7),
    .opcode0(w_level7_instruction_register_8bit_ic_inst_1_opcode0),
    .opcode1(w_level7_instruction_register_8bit_ic_inst_1_opcode1),
    .opcode2(w_level7_instruction_register_8bit_ic_inst_1_opcode2),
    .opcode3(w_level7_instruction_register_8bit_ic_inst_1_opcode3),
    .opcode4(w_level7_instruction_register_8bit_ic_inst_1_opcode4),
    .registeraddr0(w_level7_instruction_register_8bit_ic_inst_1_registeraddr0),
    .registeraddr1(w_level7_instruction_register_8bit_ic_inst_1_registeraddr1),
    .registeraddr2(w_level7_instruction_register_8bit_ic_inst_1_registeraddr2)
);

// Writing output data. //
assign led13_1 = w_level7_instruction_register_8bit_ic_inst_1_instruction0;
assign led14_1 = w_level7_instruction_register_8bit_ic_inst_1_instruction1;
assign led15_1 = w_level7_instruction_register_8bit_ic_inst_1_instruction2;
assign led16_1 = w_level7_instruction_register_8bit_ic_inst_1_instruction3;
assign led17_1 = w_level7_instruction_register_8bit_ic_inst_1_instruction4;
assign led18_1 = w_level7_instruction_register_8bit_ic_inst_1_instruction5;
assign led19_1 = w_level7_instruction_register_8bit_ic_inst_1_instruction6;
assign led20_1 = w_level7_instruction_register_8bit_ic_inst_1_instruction7;
assign led21_1 = w_level7_instruction_register_8bit_ic_inst_1_opcode0;
assign led22_1 = w_level7_instruction_register_8bit_ic_inst_1_opcode1;
assign led23_1 = w_level7_instruction_register_8bit_ic_inst_1_opcode2;
assign led24_1 = w_level7_instruction_register_8bit_ic_inst_1_opcode3;
assign led25_1 = w_level7_instruction_register_8bit_ic_inst_1_opcode4;
assign led26_1 = w_level7_instruction_register_8bit_ic_inst_1_registeraddr0;
assign led27_1 = w_level7_instruction_register_8bit_ic_inst_1_registeraddr1;
assign led28_1 = w_level7_instruction_register_8bit_ic_inst_1_registeraddr2;
endmodule
