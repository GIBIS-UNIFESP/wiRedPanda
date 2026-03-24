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

// Module for LEVEL6_REGISTER_8BIT (generated from level6_register_8bit.panda)
module level6_register_8bit_ic (
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

module level6_register_8bit (
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
output led20_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL6_REGISTER_8BIT (level6_register_8bit_ic)
wire w_level6_register_8bit_ic_inst_1_q0;
wire w_level6_register_8bit_ic_inst_1_q1;
wire w_level6_register_8bit_ic_inst_1_q2;
wire w_level6_register_8bit_ic_inst_1_q3;
wire w_level6_register_8bit_ic_inst_1_q4;
wire w_level6_register_8bit_ic_inst_1_q5;
wire w_level6_register_8bit_ic_inst_1_q6;
wire w_level6_register_8bit_ic_inst_1_q7;


// Assigning aux variables. //
level6_register_8bit_ic level6_register_8bit_ic_inst_1 (
    .data0(input_switch1),
    .clock(input_switch2),
    .data1(input_switch3),
    .writeenable(input_switch4),
    .data2(input_switch5),
    .reset(input_switch6),
    .data3(input_switch7),
    .data4(input_switch8),
    .data5(input_switch9),
    .data6(input_switch10),
    .data7(input_switch11),
    .q0(w_level6_register_8bit_ic_inst_1_q0),
    .q1(w_level6_register_8bit_ic_inst_1_q1),
    .q2(w_level6_register_8bit_ic_inst_1_q2),
    .q3(w_level6_register_8bit_ic_inst_1_q3),
    .q4(w_level6_register_8bit_ic_inst_1_q4),
    .q5(w_level6_register_8bit_ic_inst_1_q5),
    .q6(w_level6_register_8bit_ic_inst_1_q6),
    .q7(w_level6_register_8bit_ic_inst_1_q7)
);

// Writing output data. //
assign led13_1 = w_level6_register_8bit_ic_inst_1_q0;
assign led14_1 = w_level6_register_8bit_ic_inst_1_q1;
assign led15_1 = w_level6_register_8bit_ic_inst_1_q2;
assign led16_1 = w_level6_register_8bit_ic_inst_1_q3;
assign led17_1 = w_level6_register_8bit_ic_inst_1_q4;
assign led18_1 = w_level6_register_8bit_ic_inst_1_q5;
assign led19_1 = w_level6_register_8bit_ic_inst_1_q6;
assign led20_1 = w_level6_register_8bit_ic_inst_1_q7;
endmodule
