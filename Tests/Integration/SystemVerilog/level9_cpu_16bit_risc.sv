// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for AddrDecoder (generated from level2_decoder_3to8.panda)
module level2_decoder_3to8 (
    input addr0,
    input addr1,
    input addr2,
    output out0,
    output out1,
    output out2,
    output out3,
    output out4,
    output out5,
    output out6,
    output out7
);

wire aux_not_1;
wire aux_not_2;
wire aux_not_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;
wire aux_and_9;
wire aux_and_10;
wire aux_and_11;

// Internal logic
assign aux_not_1 = ~addr0;
assign aux_not_2 = ~addr1;
assign aux_not_3 = ~addr2;
assign aux_and_4 = (aux_not_1 & aux_not_2 & aux_not_3);
assign aux_and_5 = (addr0 & aux_not_2 & aux_not_3);
assign aux_and_6 = (aux_not_1 & addr1 & aux_not_3);
assign aux_and_7 = (addr0 & addr1 & aux_not_3);
assign aux_and_8 = (aux_not_1 & aux_not_2 & addr2);
assign aux_and_9 = (addr0 & aux_not_2 & addr2);
assign aux_and_10 = (aux_not_1 & addr1 & addr2);
assign aux_and_11 = (addr0 & addr1 & addr2);

assign out0 = aux_and_4;
assign out1 = aux_and_5;
assign out2 = aux_and_6;
assign out3 = aux_and_7;
assign out4 = aux_and_8;
assign out5 = aux_and_9;
assign out6 = aux_and_10;
assign out7 = aux_and_11;
endmodule

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

// Module for ReadMux (generated from level2_mux_8to1.panda)
module level2_mux_8to1 (
    input data0,
    input data1,
    input data2,
    input data3,
    input data4,
    input data5,
    input data6,
    input data7,
    input sel0,
    input sel1,
    input sel2,
    output p_output
);

reg aux_mux_1 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel2, sel1, sel0})
            3'd0: aux_mux_1 = data0;
            3'd1: aux_mux_1 = data1;
            3'd2: aux_mux_1 = data2;
            3'd3: aux_mux_1 = data3;
            3'd4: aux_mux_1 = data4;
            3'd5: aux_mux_1 = data5;
            3'd6: aux_mux_1 = data6;
            3'd7: aux_mux_1 = data7;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign p_output = aux_mux_1;
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

reg aux_mux_1 = 1'b0;
reg aux_mux_2 = 1'b0;
reg aux_mux_3 = 1'b0;
reg aux_mux_4 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({op0})
            1'd0: aux_mux_1 = result0;
            1'd1: aux_mux_1 = result1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({op0})
            1'd0: aux_mux_2 = result2;
            1'd1: aux_mux_2 = result3;
            default: aux_mux_2 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({op1})
            1'd0: aux_mux_3 = aux_mux_1;
            1'd1: aux_mux_3 = aux_mux_2;
            default: aux_mux_3 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({op2})
            1'd0: aux_mux_4 = aux_mux_3;
            1'd1: aux_mux_4 = result4;
            default: aux_mux_4 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign out = aux_mux_4;
endmodule

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

// Module for RAM_Bit0 (generated from level4_ram_8x1.panda)
module level4_ram_8x1 (
    input address0,
    input address1,
    input address2,
    input datain,
    input writeenable,
    input clock,
    output dataout
);

wire aux_and_1;
wire aux_and_2;
wire aux_and_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;
reg aux_mux_9 = 1'b0;
reg aux_d_flip_flop_10_0_q = 1'b0;
reg aux_d_flip_flop_10_1_q = 1'b1;
reg aux_mux_11 = 1'b0;
reg aux_d_flip_flop_12_0_q = 1'b0;
reg aux_d_flip_flop_12_1_q = 1'b1;
reg aux_mux_13 = 1'b0;
reg aux_d_flip_flop_14_0_q = 1'b0;
reg aux_d_flip_flop_14_1_q = 1'b1;
reg aux_mux_15 = 1'b0;
reg aux_d_flip_flop_16_0_q = 1'b0;
reg aux_d_flip_flop_16_1_q = 1'b1;
reg aux_mux_17 = 1'b0;
reg aux_d_flip_flop_18_0_q = 1'b0;
reg aux_d_flip_flop_18_1_q = 1'b1;
reg aux_mux_19 = 1'b0;
reg aux_d_flip_flop_20_0_q = 1'b0;
reg aux_d_flip_flop_20_1_q = 1'b1;
reg aux_mux_21 = 1'b0;
reg aux_d_flip_flop_22_0_q = 1'b0;
reg aux_d_flip_flop_22_1_q = 1'b1;
reg aux_mux_23 = 1'b0;
reg aux_d_flip_flop_24_0_q = 1'b0;
reg aux_d_flip_flop_24_1_q = 1'b1;
// IC instance: AddrDecoder (level2_decoder_3to8)
wire w_level2_decoder_3to8_inst_25_out0;
wire w_level2_decoder_3to8_inst_25_out1;
wire w_level2_decoder_3to8_inst_25_out2;
wire w_level2_decoder_3to8_inst_25_out3;
wire w_level2_decoder_3to8_inst_25_out4;
wire w_level2_decoder_3to8_inst_25_out5;
wire w_level2_decoder_3to8_inst_25_out6;
wire w_level2_decoder_3to8_inst_25_out7;
// IC instance: ReadMux (level2_mux_8to1)
wire w_level2_mux_8to1_inst_26_p_output;

// Internal logic
assign aux_and_1 = (w_level2_decoder_3to8_inst_25_out0 & writeenable);
assign aux_and_2 = (w_level2_decoder_3to8_inst_25_out1 & writeenable);
assign aux_and_3 = (w_level2_decoder_3to8_inst_25_out2 & writeenable);
assign aux_and_4 = (w_level2_decoder_3to8_inst_25_out3 & writeenable);
assign aux_and_5 = (w_level2_decoder_3to8_inst_25_out4 & writeenable);
assign aux_and_6 = (w_level2_decoder_3to8_inst_25_out5 & writeenable);
assign aux_and_7 = (w_level2_decoder_3to8_inst_25_out6 & writeenable);
assign aux_and_8 = (w_level2_decoder_3to8_inst_25_out7 & writeenable);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_1})
            1'd0: aux_mux_9 = aux_d_flip_flop_10_0_q;
            1'd1: aux_mux_9 = datain;
            default: aux_mux_9 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_10_0_q <= aux_mux_9;
            aux_d_flip_flop_10_1_q <= ~aux_mux_9;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_11 = aux_d_flip_flop_12_0_q;
            1'd1: aux_mux_11 = datain;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_12_0_q <= aux_mux_11;
            aux_d_flip_flop_12_1_q <= ~aux_mux_11;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_13 = aux_d_flip_flop_14_0_q;
            1'd1: aux_mux_13 = datain;
            default: aux_mux_13 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_14_0_q <= aux_mux_13;
            aux_d_flip_flop_14_1_q <= ~aux_mux_13;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_15 = aux_d_flip_flop_16_0_q;
            1'd1: aux_mux_15 = datain;
            default: aux_mux_15 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_16_0_q <= aux_mux_15;
            aux_d_flip_flop_16_1_q <= ~aux_mux_15;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_17 = aux_d_flip_flop_18_0_q;
            1'd1: aux_mux_17 = datain;
            default: aux_mux_17 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_18_0_q <= aux_mux_17;
            aux_d_flip_flop_18_1_q <= ~aux_mux_17;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_19 = aux_d_flip_flop_20_0_q;
            1'd1: aux_mux_19 = datain;
            default: aux_mux_19 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_20_0_q <= aux_mux_19;
            aux_d_flip_flop_20_1_q <= ~aux_mux_19;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_21 = aux_d_flip_flop_22_0_q;
            1'd1: aux_mux_21 = datain;
            default: aux_mux_21 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_22_0_q <= aux_mux_21;
            aux_d_flip_flop_22_1_q <= ~aux_mux_21;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_23 = aux_d_flip_flop_24_0_q;
            1'd1: aux_mux_23 = datain;
            default: aux_mux_23 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_24_0_q <= aux_mux_23;
            aux_d_flip_flop_24_1_q <= ~aux_mux_23;
    end
    //End of D FlipFlop
level2_decoder_3to8 level2_decoder_3to8_inst_25 (
    .addr0(address0),
    .addr1(address1),
    .addr2(address2),
    .out0(w_level2_decoder_3to8_inst_25_out0),
    .out1(w_level2_decoder_3to8_inst_25_out1),
    .out2(w_level2_decoder_3to8_inst_25_out2),
    .out3(w_level2_decoder_3to8_inst_25_out3),
    .out4(w_level2_decoder_3to8_inst_25_out4),
    .out5(w_level2_decoder_3to8_inst_25_out5),
    .out6(w_level2_decoder_3to8_inst_25_out6),
    .out7(w_level2_decoder_3to8_inst_25_out7)
);
level2_mux_8to1 level2_mux_8to1_inst_26 (
    .data0(aux_d_flip_flop_10_0_q),
    .data1(aux_d_flip_flop_12_0_q),
    .data2(aux_d_flip_flop_14_0_q),
    .data3(aux_d_flip_flop_16_0_q),
    .data4(aux_d_flip_flop_18_0_q),
    .data5(aux_d_flip_flop_20_0_q),
    .data6(aux_d_flip_flop_22_0_q),
    .data7(aux_d_flip_flop_24_0_q),
    .sel0(address0),
    .sel1(address1),
    .sel2(address2),
    .p_output(w_level2_mux_8to1_inst_26_p_output)
);

assign dataout = w_level2_mux_8to1_inst_26_p_output;
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

// Module for ALU_Low (generated from level6_alu_8bit.panda)
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
    output result0,
    output result1,
    output zero,
    output result2,
    output negative,
    output result3,
    output carry,
    output result4,
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
wire aux_xor_5;
wire aux_xor_6;
wire aux_xor_7;
wire aux_xor_8;
wire aux_xor_9;
wire aux_xor_10;
wire aux_xor_11;
wire aux_xor_12;
wire aux_not_13;
wire aux_not_14;
wire aux_not_15;
wire aux_not_16;
wire aux_not_17;
wire aux_not_18;
wire aux_not_19;
wire aux_not_20;
// IC instance: Selector5way[0] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_23_out;
// IC instance: Selector5way[1] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_24_out;
// IC instance: Selector5way[2] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_25_out;
// IC instance: Selector5way[3] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_26_out;
// IC instance: Selector5way[4] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_27_out;
// IC instance: Selector5way[5] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_28_out;
// IC instance: Selector5way[6] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_29_out;
// IC instance: Selector5way[7] (level3_alu_selector_5way)
wire w_level3_alu_selector_5way_inst_30_out;
reg aux_mux_31 = 1'b0;
reg aux_mux_32 = 1'b0;
reg aux_mux_33 = 1'b0;
reg aux_mux_34 = 1'b0;
reg aux_mux_35 = 1'b0;
reg aux_mux_36 = 1'b0;
reg aux_mux_37 = 1'b0;
reg aux_mux_38 = 1'b0;
wire aux_not_39;
wire aux_not_40;
wire aux_and_41;
wire aux_and_42;
wire aux_and_43;
wire aux_and_44;
wire aux_and_45;
wire aux_and_46;
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
reg aux_mux_67 = 1'b0;
reg aux_mux_68 = 1'b0;
reg aux_mux_69 = 1'b0;
wire aux_or_70;
reg aux_mux_71 = 1'b0;
reg aux_mux_72 = 1'b0;
reg aux_mux_73 = 1'b0;
wire aux_or_74;
reg aux_mux_75 = 1'b0;
reg aux_mux_76 = 1'b0;
reg aux_mux_77 = 1'b0;
wire aux_or_78;
wire aux_nor_79;

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
    .carryin(1'b0),
    .subcarryin(1'b1),
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
assign aux_xor_5 = (a0 ^ b0);
assign aux_xor_6 = (a1 ^ b1);
assign aux_xor_7 = (a2 ^ b2);
assign aux_xor_8 = (a3 ^ b3);
assign aux_xor_9 = (a4 ^ b4);
assign aux_xor_10 = (a5 ^ b5);
assign aux_xor_11 = (a6 ^ b6);
assign aux_xor_12 = (a7 ^ b7);
assign aux_not_13 = ~a0;
assign aux_not_14 = ~a1;
assign aux_not_15 = ~a2;
assign aux_not_16 = ~a3;
assign aux_not_17 = ~a4;
assign aux_not_18 = ~a5;
assign aux_not_19 = ~a6;
assign aux_not_20 = ~a7;
level3_alu_selector_5way level3_alu_selector_5way_inst_23 (
    .result0(w_level4_ripple_alu_4bit_inst_1_result_add0),
    .result1(w_level4_ripple_alu_4bit_inst_1_result_sub0),
    .result2(w_level4_ripple_alu_4bit_inst_1_result_and0),
    .result3(w_level4_ripple_alu_4bit_inst_1_result_or0),
    .result4(aux_xor_5),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_23_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_24 (
    .result0(w_level4_ripple_alu_4bit_inst_1_result_add1),
    .result1(w_level4_ripple_alu_4bit_inst_1_result_sub1),
    .result2(w_level4_ripple_alu_4bit_inst_1_result_and1),
    .result3(w_level4_ripple_alu_4bit_inst_1_result_or1),
    .result4(aux_xor_6),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_24_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_25 (
    .result0(w_level4_ripple_alu_4bit_inst_1_result_add2),
    .result1(w_level4_ripple_alu_4bit_inst_1_result_sub2),
    .result2(w_level4_ripple_alu_4bit_inst_1_result_and2),
    .result3(w_level4_ripple_alu_4bit_inst_1_result_or2),
    .result4(aux_xor_7),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_25_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_26 (
    .result0(w_level4_ripple_alu_4bit_inst_1_result_add3),
    .result1(w_level4_ripple_alu_4bit_inst_1_result_sub3),
    .result2(w_level4_ripple_alu_4bit_inst_1_result_and3),
    .result3(w_level4_ripple_alu_4bit_inst_1_result_or3),
    .result4(aux_xor_8),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_26_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_27 (
    .result0(w_level4_ripple_alu_4bit_inst_2_result_add0),
    .result1(w_level4_ripple_alu_4bit_inst_2_result_sub0),
    .result2(w_level4_ripple_alu_4bit_inst_2_result_and0),
    .result3(w_level4_ripple_alu_4bit_inst_2_result_or0),
    .result4(aux_xor_9),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_27_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_28 (
    .result0(w_level4_ripple_alu_4bit_inst_2_result_add1),
    .result1(w_level4_ripple_alu_4bit_inst_2_result_sub1),
    .result2(w_level4_ripple_alu_4bit_inst_2_result_and1),
    .result3(w_level4_ripple_alu_4bit_inst_2_result_or1),
    .result4(aux_xor_10),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_28_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_29 (
    .result0(w_level4_ripple_alu_4bit_inst_2_result_add2),
    .result1(w_level4_ripple_alu_4bit_inst_2_result_sub2),
    .result2(w_level4_ripple_alu_4bit_inst_2_result_and2),
    .result3(w_level4_ripple_alu_4bit_inst_2_result_or2),
    .result4(aux_xor_11),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_29_out)
);
level3_alu_selector_5way level3_alu_selector_5way_inst_30 (
    .result0(w_level4_ripple_alu_4bit_inst_2_result_add3),
    .result1(w_level4_ripple_alu_4bit_inst_2_result_sub3),
    .result2(w_level4_ripple_alu_4bit_inst_2_result_and3),
    .result3(w_level4_ripple_alu_4bit_inst_2_result_or3),
    .result4(aux_xor_12),
    .op0(opcode0),
    .op1(opcode1),
    .op2(opcode2),
    .out(w_level3_alu_selector_5way_inst_30_out)
);
    //Multiplexer
    always @(*)
    begin
        case({1'b0})
            1'd0: aux_mux_31 = w_level3_alu_selector_5way_inst_23_out;
            1'd1: aux_mux_31 = aux_not_13;
            default: aux_mux_31 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({1'b0})
            1'd0: aux_mux_32 = w_level3_alu_selector_5way_inst_24_out;
            1'd1: aux_mux_32 = aux_not_14;
            default: aux_mux_32 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({1'b0})
            1'd0: aux_mux_33 = w_level3_alu_selector_5way_inst_25_out;
            1'd1: aux_mux_33 = aux_not_15;
            default: aux_mux_33 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({1'b0})
            1'd0: aux_mux_34 = w_level3_alu_selector_5way_inst_26_out;
            1'd1: aux_mux_34 = aux_not_16;
            default: aux_mux_34 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({1'b0})
            1'd0: aux_mux_35 = w_level3_alu_selector_5way_inst_27_out;
            1'd1: aux_mux_35 = aux_not_17;
            default: aux_mux_35 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({1'b0})
            1'd0: aux_mux_36 = w_level3_alu_selector_5way_inst_28_out;
            1'd1: aux_mux_36 = aux_not_18;
            default: aux_mux_36 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({1'b0})
            1'd0: aux_mux_37 = w_level3_alu_selector_5way_inst_29_out;
            1'd1: aux_mux_37 = aux_not_19;
            default: aux_mux_37 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({1'b0})
            1'd0: aux_mux_38 = w_level3_alu_selector_5way_inst_30_out;
            1'd1: aux_mux_38 = aux_not_20;
            default: aux_mux_38 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_not_39 = ~opcode1;
assign aux_not_40 = ~opcode0;
assign aux_and_41 = (opcode2 & aux_not_39);
assign aux_and_42 = (aux_and_41 & opcode0);
assign aux_and_43 = (opcode2 & opcode1);
assign aux_and_44 = (aux_and_43 & aux_not_40);
assign aux_and_45 = (opcode2 & opcode1);
assign aux_and_46 = (aux_and_45 & opcode0);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_42})
            1'd0: aux_mux_47 = w_level3_alu_selector_5way_inst_23_out;
            1'd1: aux_mux_47 = aux_not_13;
            default: aux_mux_47 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_44})
            1'd0: aux_mux_48 = a1;
            1'd1: aux_mux_48 = 1'b0;
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
assign aux_or_50 = (aux_and_44 | aux_and_46);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_42})
            1'd0: aux_mux_51 = w_level3_alu_selector_5way_inst_24_out;
            1'd1: aux_mux_51 = aux_not_14;
            default: aux_mux_51 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_44})
            1'd0: aux_mux_52 = a2;
            1'd1: aux_mux_52 = a0;
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
assign aux_or_54 = (aux_and_44 | aux_and_46);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_42})
            1'd0: aux_mux_55 = w_level3_alu_selector_5way_inst_25_out;
            1'd1: aux_mux_55 = aux_not_15;
            default: aux_mux_55 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_44})
            1'd0: aux_mux_56 = a3;
            1'd1: aux_mux_56 = a1;
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
assign aux_or_58 = (aux_and_44 | aux_and_46);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_42})
            1'd0: aux_mux_59 = w_level3_alu_selector_5way_inst_26_out;
            1'd1: aux_mux_59 = aux_not_16;
            default: aux_mux_59 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_44})
            1'd0: aux_mux_60 = a4;
            1'd1: aux_mux_60 = a2;
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
assign aux_or_62 = (aux_and_44 | aux_and_46);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_42})
            1'd0: aux_mux_63 = w_level3_alu_selector_5way_inst_27_out;
            1'd1: aux_mux_63 = aux_not_17;
            default: aux_mux_63 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_44})
            1'd0: aux_mux_64 = a5;
            1'd1: aux_mux_64 = a3;
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
assign aux_or_66 = (aux_and_44 | aux_and_46);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_42})
            1'd0: aux_mux_67 = w_level3_alu_selector_5way_inst_28_out;
            1'd1: aux_mux_67 = aux_not_18;
            default: aux_mux_67 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_44})
            1'd0: aux_mux_68 = a6;
            1'd1: aux_mux_68 = a4;
            default: aux_mux_68 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_70})
            1'd0: aux_mux_69 = aux_mux_67;
            1'd1: aux_mux_69 = aux_mux_68;
            default: aux_mux_69 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_70 = (aux_and_44 | aux_and_46);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_42})
            1'd0: aux_mux_71 = w_level3_alu_selector_5way_inst_29_out;
            1'd1: aux_mux_71 = aux_not_19;
            default: aux_mux_71 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_44})
            1'd0: aux_mux_72 = a7;
            1'd1: aux_mux_72 = a5;
            default: aux_mux_72 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_74})
            1'd0: aux_mux_73 = aux_mux_71;
            1'd1: aux_mux_73 = aux_mux_72;
            default: aux_mux_73 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_74 = (aux_and_44 | aux_and_46);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_42})
            1'd0: aux_mux_75 = w_level3_alu_selector_5way_inst_30_out;
            1'd1: aux_mux_75 = aux_not_20;
            default: aux_mux_75 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_44})
            1'd0: aux_mux_76 = 1'b0;
            1'd1: aux_mux_76 = a6;
            default: aux_mux_76 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_or_78})
            1'd0: aux_mux_77 = aux_mux_75;
            1'd1: aux_mux_77 = aux_mux_76;
            default: aux_mux_77 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_78 = (aux_and_44 | aux_and_46);
assign aux_nor_79 = ~(w_level3_alu_selector_5way_inst_23_out | w_level3_alu_selector_5way_inst_24_out | w_level3_alu_selector_5way_inst_25_out | w_level3_alu_selector_5way_inst_26_out | w_level3_alu_selector_5way_inst_27_out | w_level3_alu_selector_5way_inst_28_out | w_level3_alu_selector_5way_inst_29_out | w_level3_alu_selector_5way_inst_30_out);

assign result0 = aux_mux_49;
assign result1 = aux_mux_53;
assign zero = aux_nor_79;
assign result2 = aux_mux_57;
assign negative = w_level3_alu_selector_5way_inst_30_out;
assign result3 = aux_mux_61;
assign carry = w_level4_ripple_alu_4bit_inst_2_carryout;
assign result4 = aux_mux_65;
assign result5 = aux_mux_69;
assign result6 = aux_mux_73;
assign result7 = aux_mux_77;
endmodule

// Module for InstructionMemory (generated from level6_ram_256x8.panda)
module level6_ram_256x8 (
    input address0,
    input address1,
    input address2,
    input address3,
    input address4,
    input address5,
    input address6,
    input address7,
    input datain0,
    input datain1,
    input datain2,
    input datain3,
    input datain4,
    input datain5,
    input datain6,
    input datain7,
    input writeenable,
    input clock,
    output dataout0,
    output dataout1,
    output dataout2,
    output dataout3,
    output dataout4,
    output dataout5,
    output dataout6,
    output dataout7
);

// IC instance: RAM_Bit0 (level4_ram_8x1)
wire w_level4_ram_8x1_inst_1_dataout;
// IC instance: RAM_Bit1 (level4_ram_8x1)
wire w_level4_ram_8x1_inst_2_dataout;
// IC instance: RAM_Bit2 (level4_ram_8x1)
wire w_level4_ram_8x1_inst_3_dataout;
// IC instance: RAM_Bit3 (level4_ram_8x1)
wire w_level4_ram_8x1_inst_4_dataout;
// IC instance: RAM_Bit4 (level4_ram_8x1)
wire w_level4_ram_8x1_inst_5_dataout;
// IC instance: RAM_Bit5 (level4_ram_8x1)
wire w_level4_ram_8x1_inst_6_dataout;
// IC instance: RAM_Bit6 (level4_ram_8x1)
wire w_level4_ram_8x1_inst_7_dataout;
// IC instance: RAM_Bit7 (level4_ram_8x1)
wire w_level4_ram_8x1_inst_8_dataout;

// Internal logic
level4_ram_8x1 level4_ram_8x1_inst_1 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain0),
    .writeenable(writeenable),
    .clock(clock),
    .dataout(w_level4_ram_8x1_inst_1_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_2 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain1),
    .writeenable(writeenable),
    .clock(clock),
    .dataout(w_level4_ram_8x1_inst_2_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_3 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain2),
    .writeenable(writeenable),
    .clock(clock),
    .dataout(w_level4_ram_8x1_inst_3_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_4 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain3),
    .writeenable(writeenable),
    .clock(clock),
    .dataout(w_level4_ram_8x1_inst_4_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_5 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain4),
    .writeenable(writeenable),
    .clock(clock),
    .dataout(w_level4_ram_8x1_inst_5_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_6 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain5),
    .writeenable(writeenable),
    .clock(clock),
    .dataout(w_level4_ram_8x1_inst_6_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_7 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain6),
    .writeenable(writeenable),
    .clock(clock),
    .dataout(w_level4_ram_8x1_inst_7_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_8 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain7),
    .writeenable(writeenable),
    .clock(clock),
    .dataout(w_level4_ram_8x1_inst_8_dataout)
);

assign dataout0 = w_level4_ram_8x1_inst_1_dataout;
assign dataout1 = w_level4_ram_8x1_inst_2_dataout;
assign dataout2 = w_level4_ram_8x1_inst_3_dataout;
assign dataout3 = w_level4_ram_8x1_inst_4_dataout;
assign dataout4 = w_level4_ram_8x1_inst_5_dataout;
assign dataout5 = w_level4_ram_8x1_inst_6_dataout;
assign dataout6 = w_level4_ram_8x1_inst_7_dataout;
assign dataout7 = w_level4_ram_8x1_inst_8_dataout;
endmodule

// Module for Register8bit (generated from level6_register_8bit.panda)
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

// Module for Adder8bit (generated from level6_ripple_adder_8bit.panda)
module level6_ripple_adder_8bit (
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

// Module for ALU_16bit (generated from level7_alu_16bit.panda)
module level7_alu_16bit (
    input operanda0,
    input operandb0,
    input aluop0,
    input operanda1,
    input operandb1,
    input aluop1,
    input operanda2,
    input operandb2,
    input aluop2,
    input operanda3,
    input operandb3,
    input operanda4,
    input operandb4,
    input operanda5,
    input operandb5,
    input operanda6,
    input operandb6,
    input operanda7,
    input operandb7,
    input operanda8,
    input operandb8,
    input operanda9,
    input operandb9,
    input operanda10,
    input operandb10,
    input operanda11,
    input operandb11,
    input operanda12,
    input operandb12,
    input operanda13,
    input operandb13,
    input operanda14,
    input operandb14,
    input operanda15,
    input operandb15,
    output result0,
    output zero,
    output result1,
    output sign,
    output result2,
    output carry,
    output result3,
    output result4,
    output result5,
    output result6,
    output result7,
    output result8,
    output result9,
    output result10,
    output result11,
    output result12,
    output result13,
    output result14,
    output result15
);

// IC instance: ALU_Low (level6_alu_8bit)
wire w_level6_alu_8bit_inst_1_result0;
wire w_level6_alu_8bit_inst_1_result1;
wire w_level6_alu_8bit_inst_1_zero;
wire w_level6_alu_8bit_inst_1_result2;
wire w_level6_alu_8bit_inst_1_negative;
wire w_level6_alu_8bit_inst_1_result3;
wire w_level6_alu_8bit_inst_1_carry;
wire w_level6_alu_8bit_inst_1_result4;
wire w_level6_alu_8bit_inst_1_result5;
wire w_level6_alu_8bit_inst_1_result6;
wire w_level6_alu_8bit_inst_1_result7;
// IC instance: ALU_High (level6_alu_8bit)
wire w_level6_alu_8bit_inst_2_result0;
wire w_level6_alu_8bit_inst_2_result1;
wire w_level6_alu_8bit_inst_2_zero;
wire w_level6_alu_8bit_inst_2_result2;
wire w_level6_alu_8bit_inst_2_negative;
wire w_level6_alu_8bit_inst_2_result3;
wire w_level6_alu_8bit_inst_2_carry;
wire w_level6_alu_8bit_inst_2_result4;
wire w_level6_alu_8bit_inst_2_result5;
wire w_level6_alu_8bit_inst_2_result6;
wire w_level6_alu_8bit_inst_2_result7;
wire aux_nor_3;

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
    .opcode0(aluop0),
    .opcode1(aluop1),
    .opcode2(aluop2),
    .b0(operandb0),
    .b1(operandb1),
    .b2(operandb2),
    .b3(operandb3),
    .b4(operandb4),
    .b5(operandb5),
    .b6(operandb6),
    .b7(operandb7),
    .result0(w_level6_alu_8bit_inst_1_result0),
    .result1(w_level6_alu_8bit_inst_1_result1),
    .zero(w_level6_alu_8bit_inst_1_zero),
    .result2(w_level6_alu_8bit_inst_1_result2),
    .negative(w_level6_alu_8bit_inst_1_negative),
    .result3(w_level6_alu_8bit_inst_1_result3),
    .carry(w_level6_alu_8bit_inst_1_carry),
    .result4(w_level6_alu_8bit_inst_1_result4),
    .result5(w_level6_alu_8bit_inst_1_result5),
    .result6(w_level6_alu_8bit_inst_1_result6),
    .result7(w_level6_alu_8bit_inst_1_result7)
);
level6_alu_8bit level6_alu_8bit_inst_2 (
    .a0(operanda8),
    .a1(operanda9),
    .a2(operanda10),
    .a3(operanda11),
    .a4(operanda12),
    .a5(operanda13),
    .a6(operanda14),
    .a7(operanda15),
    .opcode0(aluop0),
    .opcode1(aluop1),
    .opcode2(aluop2),
    .b0(operandb8),
    .b1(operandb9),
    .b2(operandb10),
    .b3(operandb11),
    .b4(operandb12),
    .b5(operandb13),
    .b6(operandb14),
    .b7(operandb15),
    .result0(w_level6_alu_8bit_inst_2_result0),
    .result1(w_level6_alu_8bit_inst_2_result1),
    .zero(w_level6_alu_8bit_inst_2_zero),
    .result2(w_level6_alu_8bit_inst_2_result2),
    .negative(w_level6_alu_8bit_inst_2_negative),
    .result3(w_level6_alu_8bit_inst_2_result3),
    .carry(w_level6_alu_8bit_inst_2_carry),
    .result4(w_level6_alu_8bit_inst_2_result4),
    .result5(w_level6_alu_8bit_inst_2_result5),
    .result6(w_level6_alu_8bit_inst_2_result6),
    .result7(w_level6_alu_8bit_inst_2_result7)
);
assign aux_nor_3 = ~(w_level6_alu_8bit_inst_1_zero | w_level6_alu_8bit_inst_2_zero);

assign result0 = w_level6_alu_8bit_inst_1_result0;
assign zero = aux_nor_3;
assign result1 = w_level6_alu_8bit_inst_1_result1;
assign sign = w_level6_alu_8bit_inst_2_negative;
assign result2 = w_level6_alu_8bit_inst_1_result2;
assign carry = w_level6_alu_8bit_inst_2_carry;
assign result3 = w_level6_alu_8bit_inst_1_result3;
assign result4 = w_level6_alu_8bit_inst_1_result4;
assign result5 = w_level6_alu_8bit_inst_1_result5;
assign result6 = w_level6_alu_8bit_inst_1_result6;
assign result7 = w_level6_alu_8bit_inst_1_result7;
assign result8 = w_level6_alu_8bit_inst_2_result0;
assign result9 = w_level6_alu_8bit_inst_2_result1;
assign result10 = w_level6_alu_8bit_inst_2_result2;
assign result11 = w_level6_alu_8bit_inst_2_result3;
assign result12 = w_level6_alu_8bit_inst_2_result4;
assign result13 = w_level6_alu_8bit_inst_2_result5;
assign result14 = w_level6_alu_8bit_inst_2_result6;
assign result15 = w_level6_alu_8bit_inst_2_result7;
endmodule

// Module for InstrMem_Low (generated from level7_instruction_memory_interface.panda)
module level7_instruction_memory_interface (
    input address0,
    input address1,
    input address2,
    input address3,
    input address4,
    input address5,
    input address6,
    input address7,
    input clock,
    input datain0,
    input datain1,
    input datain2,
    input datain3,
    input datain4,
    input datain5,
    input datain6,
    input datain7,
    input writeenable,
    output instruction0,
    output instruction1,
    output instruction2,
    output instruction3,
    output instruction4,
    output instruction5,
    output instruction6,
    output instruction7
);

// IC instance: InstructionMemory (level6_ram_256x8)
wire w_level6_ram_256x8_inst_1_dataout0;
wire w_level6_ram_256x8_inst_1_dataout1;
wire w_level6_ram_256x8_inst_1_dataout2;
wire w_level6_ram_256x8_inst_1_dataout3;
wire w_level6_ram_256x8_inst_1_dataout4;
wire w_level6_ram_256x8_inst_1_dataout5;
wire w_level6_ram_256x8_inst_1_dataout6;
wire w_level6_ram_256x8_inst_1_dataout7;

// Internal logic
level6_ram_256x8 level6_ram_256x8_inst_1 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .address3(address3),
    .address4(address4),
    .address5(address5),
    .address6(address6),
    .address7(address7),
    .datain0(datain0),
    .datain1(datain1),
    .datain2(datain2),
    .datain3(datain3),
    .datain4(datain4),
    .datain5(datain5),
    .datain6(datain6),
    .datain7(datain7),
    .writeenable(writeenable),
    .clock(clock),
    .dataout0(w_level6_ram_256x8_inst_1_dataout0),
    .dataout1(w_level6_ram_256x8_inst_1_dataout1),
    .dataout2(w_level6_ram_256x8_inst_1_dataout2),
    .dataout3(w_level6_ram_256x8_inst_1_dataout3),
    .dataout4(w_level6_ram_256x8_inst_1_dataout4),
    .dataout5(w_level6_ram_256x8_inst_1_dataout5),
    .dataout6(w_level6_ram_256x8_inst_1_dataout6),
    .dataout7(w_level6_ram_256x8_inst_1_dataout7)
);

assign instruction0 = w_level6_ram_256x8_inst_1_dataout0;
assign instruction1 = w_level6_ram_256x8_inst_1_dataout1;
assign instruction2 = w_level6_ram_256x8_inst_1_dataout2;
assign instruction3 = w_level6_ram_256x8_inst_1_dataout3;
assign instruction4 = w_level6_ram_256x8_inst_1_dataout4;
assign instruction5 = w_level6_ram_256x8_inst_1_dataout5;
assign instruction6 = w_level6_ram_256x8_inst_1_dataout6;
assign instruction7 = w_level6_ram_256x8_inst_1_dataout7;
endmodule

// Module for PC (generated from level6_program_counter_8bit_arithmetic.panda)
module level6_program_counter_8bit_arithmetic (
    input loadvalue0,
    input loadvalue1,
    input loadvalue2,
    input loadvalue3,
    input loadvalue4,
    input loadvalue5,
    input loadvalue6,
    input loadvalue7,
    input load,
    input inc,
    input reset,
    input clock,
    output pc0,
    output pc_plus_10,
    output pc1,
    output pc_plus_11,
    output pc2,
    output pc_plus_12,
    output pc3,
    output pc_plus_13,
    output pc4,
    output pc_plus_14,
    output pc5,
    output pc_plus_15,
    output pc6,
    output pc_plus_16,
    output pc7,
    output pc_plus_17
);

// IC instance: Register8bit (level6_register_8bit)
wire w_level6_register_8bit_inst_1_q0;
wire w_level6_register_8bit_inst_1_q1;
wire w_level6_register_8bit_inst_1_q2;
wire w_level6_register_8bit_inst_1_q3;
wire w_level6_register_8bit_inst_1_q4;
wire w_level6_register_8bit_inst_1_q5;
wire w_level6_register_8bit_inst_1_q6;
wire w_level6_register_8bit_inst_1_q7;
// IC instance: Adder8bit (level6_ripple_adder_8bit)
wire w_level6_ripple_adder_8bit_inst_2_sum0;
wire w_level6_ripple_adder_8bit_inst_2_sum1;
wire w_level6_ripple_adder_8bit_inst_2_sum2;
wire w_level6_ripple_adder_8bit_inst_2_sum3;
wire w_level6_ripple_adder_8bit_inst_2_sum4;
wire w_level6_ripple_adder_8bit_inst_2_sum5;
wire w_level6_ripple_adder_8bit_inst_2_sum6;
wire w_level6_ripple_adder_8bit_inst_2_sum7;
wire w_level6_ripple_adder_8bit_inst_2_carryout;
wire aux_not_4;
wire aux_and_5;
reg aux_mux_6 = 1'b0;
reg aux_mux_7 = 1'b0;
reg aux_mux_8 = 1'b0;
reg aux_mux_9 = 1'b0;
reg aux_mux_10 = 1'b0;
reg aux_mux_11 = 1'b0;
reg aux_mux_12 = 1'b0;
reg aux_mux_13 = 1'b0;
reg aux_mux_14 = 1'b0;
reg aux_mux_15 = 1'b0;
reg aux_mux_16 = 1'b0;
reg aux_mux_17 = 1'b0;
reg aux_mux_18 = 1'b0;
reg aux_mux_19 = 1'b0;
reg aux_mux_20 = 1'b0;
reg aux_mux_21 = 1'b0;
wire aux_or_22;

// Internal logic
level6_register_8bit level6_register_8bit_inst_1 (
    .data0(aux_mux_14),
    .clock(clock),
    .data1(aux_mux_15),
    .writeenable(aux_or_22),
    .data2(aux_mux_16),
    .reset(reset),
    .data3(aux_mux_17),
    .data4(aux_mux_18),
    .data5(aux_mux_19),
    .data6(aux_mux_20),
    .data7(aux_mux_21),
    .q0(w_level6_register_8bit_inst_1_q0),
    .q1(w_level6_register_8bit_inst_1_q1),
    .q2(w_level6_register_8bit_inst_1_q2),
    .q3(w_level6_register_8bit_inst_1_q3),
    .q4(w_level6_register_8bit_inst_1_q4),
    .q5(w_level6_register_8bit_inst_1_q5),
    .q6(w_level6_register_8bit_inst_1_q6),
    .q7(w_level6_register_8bit_inst_1_q7)
);
level6_ripple_adder_8bit level6_ripple_adder_8bit_inst_2 (
    .a0(w_level6_register_8bit_inst_1_q0),
    .b0(1'b1),
    .carryin(1'b0),
    .a1(w_level6_register_8bit_inst_1_q1),
    .b1(1'b0),
    .a2(w_level6_register_8bit_inst_1_q2),
    .b2(1'b0),
    .a3(w_level6_register_8bit_inst_1_q3),
    .b3(1'b0),
    .a4(w_level6_register_8bit_inst_1_q4),
    .b4(1'b0),
    .a5(w_level6_register_8bit_inst_1_q5),
    .b5(1'b0),
    .a6(w_level6_register_8bit_inst_1_q6),
    .b6(1'b0),
    .a7(w_level6_register_8bit_inst_1_q7),
    .b7(1'b0),
    .sum0(w_level6_ripple_adder_8bit_inst_2_sum0),
    .sum1(w_level6_ripple_adder_8bit_inst_2_sum1),
    .sum2(w_level6_ripple_adder_8bit_inst_2_sum2),
    .sum3(w_level6_ripple_adder_8bit_inst_2_sum3),
    .sum4(w_level6_ripple_adder_8bit_inst_2_sum4),
    .sum5(w_level6_ripple_adder_8bit_inst_2_sum5),
    .sum6(w_level6_ripple_adder_8bit_inst_2_sum6),
    .sum7(w_level6_ripple_adder_8bit_inst_2_sum7),
    .carryout(w_level6_ripple_adder_8bit_inst_2_carryout)
);
assign aux_not_4 = ~load;
assign aux_and_5 = (inc & aux_not_4);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_6 = w_level6_register_8bit_inst_1_q0;
            1'd1: aux_mux_6 = w_level6_ripple_adder_8bit_inst_2_sum0;
            default: aux_mux_6 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_7 = w_level6_register_8bit_inst_1_q1;
            1'd1: aux_mux_7 = w_level6_ripple_adder_8bit_inst_2_sum1;
            default: aux_mux_7 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_8 = w_level6_register_8bit_inst_1_q2;
            1'd1: aux_mux_8 = w_level6_ripple_adder_8bit_inst_2_sum2;
            default: aux_mux_8 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_9 = w_level6_register_8bit_inst_1_q3;
            1'd1: aux_mux_9 = w_level6_ripple_adder_8bit_inst_2_sum3;
            default: aux_mux_9 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_10 = w_level6_register_8bit_inst_1_q4;
            1'd1: aux_mux_10 = w_level6_ripple_adder_8bit_inst_2_sum4;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_11 = w_level6_register_8bit_inst_1_q5;
            1'd1: aux_mux_11 = w_level6_ripple_adder_8bit_inst_2_sum5;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_12 = w_level6_register_8bit_inst_1_q6;
            1'd1: aux_mux_12 = w_level6_ripple_adder_8bit_inst_2_sum6;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_13 = w_level6_register_8bit_inst_1_q7;
            1'd1: aux_mux_13 = w_level6_ripple_adder_8bit_inst_2_sum7;
            default: aux_mux_13 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_14 = aux_mux_6;
            1'd1: aux_mux_14 = loadvalue0;
            default: aux_mux_14 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_15 = aux_mux_7;
            1'd1: aux_mux_15 = loadvalue1;
            default: aux_mux_15 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_16 = aux_mux_8;
            1'd1: aux_mux_16 = loadvalue2;
            default: aux_mux_16 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_17 = aux_mux_9;
            1'd1: aux_mux_17 = loadvalue3;
            default: aux_mux_17 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_18 = aux_mux_10;
            1'd1: aux_mux_18 = loadvalue4;
            default: aux_mux_18 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_19 = aux_mux_11;
            1'd1: aux_mux_19 = loadvalue5;
            default: aux_mux_19 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_20 = aux_mux_12;
            1'd1: aux_mux_20 = loadvalue6;
            default: aux_mux_20 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_21 = aux_mux_13;
            1'd1: aux_mux_21 = loadvalue7;
            default: aux_mux_21 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_22 = (load | inc);

assign pc0 = w_level6_register_8bit_inst_1_q0;
assign pc_plus_10 = w_level6_ripple_adder_8bit_inst_2_sum0;
assign pc1 = w_level6_register_8bit_inst_1_q1;
assign pc_plus_11 = w_level6_ripple_adder_8bit_inst_2_sum1;
assign pc2 = w_level6_register_8bit_inst_1_q2;
assign pc_plus_12 = w_level6_ripple_adder_8bit_inst_2_sum2;
assign pc3 = w_level6_register_8bit_inst_1_q3;
assign pc_plus_13 = w_level6_ripple_adder_8bit_inst_2_sum3;
assign pc4 = w_level6_register_8bit_inst_1_q4;
assign pc_plus_14 = w_level6_ripple_adder_8bit_inst_2_sum4;
assign pc5 = w_level6_register_8bit_inst_1_q5;
assign pc_plus_15 = w_level6_ripple_adder_8bit_inst_2_sum5;
assign pc6 = w_level6_register_8bit_inst_1_q6;
assign pc_plus_16 = w_level6_ripple_adder_8bit_inst_2_sum6;
assign pc7 = w_level6_register_8bit_inst_1_q7;
assign pc_plus_17 = w_level6_ripple_adder_8bit_inst_2_sum7;
endmodule

// Module for PC (generated from level7_cpu_program_counter_8bit.panda)
module level7_cpu_program_counter_8bit (
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
    input inc,
    input reset,
    input writeenable,
    output address0,
    output address1,
    output address2,
    output address3,
    output address4,
    output address5,
    output address6,
    output address7
);

// IC instance: PC (level6_program_counter_8bit_arithmetic)
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc0;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_10;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc1;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_11;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc2;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_12;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc3;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_13;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc4;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_14;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc5;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_15;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc6;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_16;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc7;
wire w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_17;

// Internal logic
level6_program_counter_8bit_arithmetic level6_program_counter_8bit_arithmetic_inst_1 (
    .loadvalue0(data0),
    .loadvalue1(data1),
    .loadvalue2(data2),
    .loadvalue3(data3),
    .loadvalue4(data4),
    .loadvalue5(data5),
    .loadvalue6(data6),
    .loadvalue7(data7),
    .load(load),
    .inc(inc),
    .reset(reset),
    .clock(clock),
    .pc0(w_level6_program_counter_8bit_arithmetic_inst_1_pc0),
    .pc_plus_10(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_10),
    .pc1(w_level6_program_counter_8bit_arithmetic_inst_1_pc1),
    .pc_plus_11(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_11),
    .pc2(w_level6_program_counter_8bit_arithmetic_inst_1_pc2),
    .pc_plus_12(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_12),
    .pc3(w_level6_program_counter_8bit_arithmetic_inst_1_pc3),
    .pc_plus_13(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_13),
    .pc4(w_level6_program_counter_8bit_arithmetic_inst_1_pc4),
    .pc_plus_14(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_14),
    .pc5(w_level6_program_counter_8bit_arithmetic_inst_1_pc5),
    .pc_plus_15(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_15),
    .pc6(w_level6_program_counter_8bit_arithmetic_inst_1_pc6),
    .pc_plus_16(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_16),
    .pc7(w_level6_program_counter_8bit_arithmetic_inst_1_pc7),
    .pc_plus_17(w_level6_program_counter_8bit_arithmetic_inst_1_pc_plus_17)
);

assign address0 = w_level6_program_counter_8bit_arithmetic_inst_1_pc0;
assign address1 = w_level6_program_counter_8bit_arithmetic_inst_1_pc1;
assign address2 = w_level6_program_counter_8bit_arithmetic_inst_1_pc2;
assign address3 = w_level6_program_counter_8bit_arithmetic_inst_1_pc3;
assign address4 = w_level6_program_counter_8bit_arithmetic_inst_1_pc4;
assign address5 = w_level6_program_counter_8bit_arithmetic_inst_1_pc5;
assign address6 = w_level6_program_counter_8bit_arithmetic_inst_1_pc6;
assign address7 = w_level6_program_counter_8bit_arithmetic_inst_1_pc7;
endmodule

// Module for Fetch_16bit (generated from level9_fetch_stage_16bit.panda)
module level9_fetch_stage_16bit (
    input pcdata0,
    input pcdata1,
    input pcdata2,
    input pcdata3,
    input pcdata4,
    input pcdata5,
    input pcdata6,
    input pcdata7,
    input clock,
    input reset,
    input pcload,
    input pcinc,
    input instrload,
    output instruction0,
    output pc0,
    output opcode0,
    output destreg0,
    output srcbits0,
    output instruction1,
    output pc1,
    output opcode1,
    output destreg1,
    output srcbits1,
    output instruction2,
    output pc2,
    output opcode2,
    output destreg2,
    output srcbits2,
    output instruction3,
    output pc3,
    output opcode3,
    output destreg3,
    output srcbits3,
    output instruction4,
    output pc4,
    output opcode4,
    output destreg4,
    output srcbits4,
    output instruction5,
    output pc5,
    output srcbits5,
    output instruction6,
    output pc6,
    output instruction7,
    output pc7,
    output instruction8,
    output instruction9,
    output instruction10,
    output instruction11,
    output instruction12,
    output instruction13,
    output instruction14,
    output instruction15
);

// IC instance: PC (level7_cpu_program_counter_8bit)
wire w_level7_cpu_program_counter_8bit_inst_1_address0;
wire w_level7_cpu_program_counter_8bit_inst_1_address1;
wire w_level7_cpu_program_counter_8bit_inst_1_address2;
wire w_level7_cpu_program_counter_8bit_inst_1_address3;
wire w_level7_cpu_program_counter_8bit_inst_1_address4;
wire w_level7_cpu_program_counter_8bit_inst_1_address5;
wire w_level7_cpu_program_counter_8bit_inst_1_address6;
wire w_level7_cpu_program_counter_8bit_inst_1_address7;
// IC instance: InstrMem_Low (level7_instruction_memory_interface)
wire w_level7_instruction_memory_interface_inst_2_instruction0;
wire w_level7_instruction_memory_interface_inst_2_instruction1;
wire w_level7_instruction_memory_interface_inst_2_instruction2;
wire w_level7_instruction_memory_interface_inst_2_instruction3;
wire w_level7_instruction_memory_interface_inst_2_instruction4;
wire w_level7_instruction_memory_interface_inst_2_instruction5;
wire w_level7_instruction_memory_interface_inst_2_instruction6;
wire w_level7_instruction_memory_interface_inst_2_instruction7;
// IC instance: InstrMem_High (level7_instruction_memory_interface)
wire w_level7_instruction_memory_interface_inst_3_instruction0;
wire w_level7_instruction_memory_interface_inst_3_instruction1;
wire w_level7_instruction_memory_interface_inst_3_instruction2;
wire w_level7_instruction_memory_interface_inst_3_instruction3;
wire w_level7_instruction_memory_interface_inst_3_instruction4;
wire w_level7_instruction_memory_interface_inst_3_instruction5;
wire w_level7_instruction_memory_interface_inst_3_instruction6;
wire w_level7_instruction_memory_interface_inst_3_instruction7;

// Internal logic
level7_cpu_program_counter_8bit level7_cpu_program_counter_8bit_inst_1 (
    .data0(pcdata0),
    .data1(pcdata1),
    .data2(pcdata2),
    .data3(pcdata3),
    .data4(pcdata4),
    .data5(pcdata5),
    .data6(pcdata6),
    .data7(pcdata7),
    .clock(clock),
    .load(pcload),
    .inc(pcinc),
    .reset(reset),
    .writeenable(1'b0),
    .address0(w_level7_cpu_program_counter_8bit_inst_1_address0),
    .address1(w_level7_cpu_program_counter_8bit_inst_1_address1),
    .address2(w_level7_cpu_program_counter_8bit_inst_1_address2),
    .address3(w_level7_cpu_program_counter_8bit_inst_1_address3),
    .address4(w_level7_cpu_program_counter_8bit_inst_1_address4),
    .address5(w_level7_cpu_program_counter_8bit_inst_1_address5),
    .address6(w_level7_cpu_program_counter_8bit_inst_1_address6),
    .address7(w_level7_cpu_program_counter_8bit_inst_1_address7)
);
level7_instruction_memory_interface level7_instruction_memory_interface_inst_2 (
    .address0(w_level7_cpu_program_counter_8bit_inst_1_address0),
    .address1(w_level7_cpu_program_counter_8bit_inst_1_address1),
    .address2(w_level7_cpu_program_counter_8bit_inst_1_address2),
    .address3(w_level7_cpu_program_counter_8bit_inst_1_address3),
    .address4(w_level7_cpu_program_counter_8bit_inst_1_address4),
    .address5(w_level7_cpu_program_counter_8bit_inst_1_address5),
    .address6(w_level7_cpu_program_counter_8bit_inst_1_address6),
    .address7(w_level7_cpu_program_counter_8bit_inst_1_address7),
    .clock(clock),
    .datain0(1'b0),
    .datain1(1'b0),
    .datain2(1'b0),
    .datain3(1'b0),
    .datain4(1'b0),
    .datain5(1'b0),
    .datain6(1'b0),
    .datain7(1'b0),
    .writeenable(1'b0),
    .instruction0(w_level7_instruction_memory_interface_inst_2_instruction0),
    .instruction1(w_level7_instruction_memory_interface_inst_2_instruction1),
    .instruction2(w_level7_instruction_memory_interface_inst_2_instruction2),
    .instruction3(w_level7_instruction_memory_interface_inst_2_instruction3),
    .instruction4(w_level7_instruction_memory_interface_inst_2_instruction4),
    .instruction5(w_level7_instruction_memory_interface_inst_2_instruction5),
    .instruction6(w_level7_instruction_memory_interface_inst_2_instruction6),
    .instruction7(w_level7_instruction_memory_interface_inst_2_instruction7)
);
level7_instruction_memory_interface level7_instruction_memory_interface_inst_3 (
    .address0(w_level7_cpu_program_counter_8bit_inst_1_address0),
    .address1(w_level7_cpu_program_counter_8bit_inst_1_address1),
    .address2(w_level7_cpu_program_counter_8bit_inst_1_address2),
    .address3(w_level7_cpu_program_counter_8bit_inst_1_address3),
    .address4(w_level7_cpu_program_counter_8bit_inst_1_address4),
    .address5(w_level7_cpu_program_counter_8bit_inst_1_address5),
    .address6(w_level7_cpu_program_counter_8bit_inst_1_address6),
    .address7(w_level7_cpu_program_counter_8bit_inst_1_address7),
    .clock(clock),
    .datain0(1'b0),
    .datain1(1'b0),
    .datain2(1'b0),
    .datain3(1'b0),
    .datain4(1'b0),
    .datain5(1'b0),
    .datain6(1'b0),
    .datain7(1'b0),
    .writeenable(1'b0),
    .instruction0(w_level7_instruction_memory_interface_inst_3_instruction0),
    .instruction1(w_level7_instruction_memory_interface_inst_3_instruction1),
    .instruction2(w_level7_instruction_memory_interface_inst_3_instruction2),
    .instruction3(w_level7_instruction_memory_interface_inst_3_instruction3),
    .instruction4(w_level7_instruction_memory_interface_inst_3_instruction4),
    .instruction5(w_level7_instruction_memory_interface_inst_3_instruction5),
    .instruction6(w_level7_instruction_memory_interface_inst_3_instruction6),
    .instruction7(w_level7_instruction_memory_interface_inst_3_instruction7)
);

assign instruction0 = w_level7_instruction_memory_interface_inst_2_instruction0;
assign pc0 = w_level7_cpu_program_counter_8bit_inst_1_address0;
assign opcode0 = w_level7_instruction_memory_interface_inst_3_instruction7;
assign destreg0 = w_level7_instruction_memory_interface_inst_3_instruction2;
assign srcbits0 = w_level7_instruction_memory_interface_inst_2_instruction5;
assign instruction1 = w_level7_instruction_memory_interface_inst_2_instruction1;
assign pc1 = w_level7_cpu_program_counter_8bit_inst_1_address1;
assign opcode1 = w_level7_instruction_memory_interface_inst_3_instruction6;
assign destreg1 = w_level7_instruction_memory_interface_inst_3_instruction1;
assign srcbits1 = w_level7_instruction_memory_interface_inst_2_instruction4;
assign instruction2 = w_level7_instruction_memory_interface_inst_2_instruction2;
assign pc2 = w_level7_cpu_program_counter_8bit_inst_1_address2;
assign opcode2 = w_level7_instruction_memory_interface_inst_3_instruction5;
assign destreg2 = w_level7_instruction_memory_interface_inst_3_instruction0;
assign srcbits2 = w_level7_instruction_memory_interface_inst_2_instruction3;
assign instruction3 = w_level7_instruction_memory_interface_inst_2_instruction3;
assign pc3 = w_level7_cpu_program_counter_8bit_inst_1_address3;
assign opcode3 = w_level7_instruction_memory_interface_inst_3_instruction4;
assign destreg3 = w_level7_instruction_memory_interface_inst_2_instruction7;
assign srcbits3 = w_level7_instruction_memory_interface_inst_2_instruction2;
assign instruction4 = w_level7_instruction_memory_interface_inst_2_instruction4;
assign pc4 = w_level7_cpu_program_counter_8bit_inst_1_address4;
assign opcode4 = w_level7_instruction_memory_interface_inst_3_instruction3;
assign destreg4 = w_level7_instruction_memory_interface_inst_2_instruction6;
assign srcbits4 = w_level7_instruction_memory_interface_inst_2_instruction1;
assign instruction5 = w_level7_instruction_memory_interface_inst_2_instruction5;
assign pc5 = w_level7_cpu_program_counter_8bit_inst_1_address5;
assign srcbits5 = w_level7_instruction_memory_interface_inst_2_instruction0;
assign instruction6 = w_level7_instruction_memory_interface_inst_2_instruction6;
assign pc6 = w_level7_cpu_program_counter_8bit_inst_1_address6;
assign instruction7 = w_level7_instruction_memory_interface_inst_2_instruction7;
assign pc7 = w_level7_cpu_program_counter_8bit_inst_1_address7;
assign instruction8 = w_level7_instruction_memory_interface_inst_3_instruction0;
assign instruction9 = w_level7_instruction_memory_interface_inst_3_instruction1;
assign instruction10 = w_level7_instruction_memory_interface_inst_3_instruction2;
assign instruction11 = w_level7_instruction_memory_interface_inst_3_instruction3;
assign instruction12 = w_level7_instruction_memory_interface_inst_3_instruction4;
assign instruction13 = w_level7_instruction_memory_interface_inst_3_instruction5;
assign instruction14 = w_level7_instruction_memory_interface_inst_3_instruction6;
assign instruction15 = w_level7_instruction_memory_interface_inst_3_instruction7;
endmodule

// Module for LEVEL9_CPU_16BIT_RISC (generated from level9_cpu_16bit_risc.panda)
module level9_cpu_16bit_risc_ic (
    input clock,
    input reset,
    output pc0,
    output pc1,
    output pc2,
    output pc3,
    output pc4,
    output pc5,
    output pc6,
    output pc7,
    output instr0,
    output opcode0,
    output instr1,
    output opcode1,
    output instr2,
    output opcode2,
    output result0,
    output instr3,
    output opcode3,
    output result1,
    output instr4,
    output opcode4,
    output result2,
    output instr5,
    output result3,
    output instr6,
    output result4,
    output instr7,
    output result5,
    output instr8,
    output result6,
    output instr9,
    output result7,
    output instr10,
    output result8,
    output instr11,
    output result9,
    output instr12,
    output result10,
    output instr13,
    output result11,
    output instr14,
    output result12,
    output instr15,
    output result13,
    output result14,
    output result15
);

// IC instance: Fetch_16bit (level9_fetch_stage_16bit)
wire w_level9_fetch_stage_16bit_inst_2_instruction0;
wire w_level9_fetch_stage_16bit_inst_2_pc0;
wire w_level9_fetch_stage_16bit_inst_2_opcode0;
wire w_level9_fetch_stage_16bit_inst_2_destreg0;
wire w_level9_fetch_stage_16bit_inst_2_srcbits0;
wire w_level9_fetch_stage_16bit_inst_2_instruction1;
wire w_level9_fetch_stage_16bit_inst_2_pc1;
wire w_level9_fetch_stage_16bit_inst_2_opcode1;
wire w_level9_fetch_stage_16bit_inst_2_destreg1;
wire w_level9_fetch_stage_16bit_inst_2_srcbits1;
wire w_level9_fetch_stage_16bit_inst_2_instruction2;
wire w_level9_fetch_stage_16bit_inst_2_pc2;
wire w_level9_fetch_stage_16bit_inst_2_opcode2;
wire w_level9_fetch_stage_16bit_inst_2_destreg2;
wire w_level9_fetch_stage_16bit_inst_2_srcbits2;
wire w_level9_fetch_stage_16bit_inst_2_instruction3;
wire w_level9_fetch_stage_16bit_inst_2_pc3;
wire w_level9_fetch_stage_16bit_inst_2_opcode3;
wire w_level9_fetch_stage_16bit_inst_2_destreg3;
wire w_level9_fetch_stage_16bit_inst_2_srcbits3;
wire w_level9_fetch_stage_16bit_inst_2_instruction4;
wire w_level9_fetch_stage_16bit_inst_2_pc4;
wire w_level9_fetch_stage_16bit_inst_2_opcode4;
wire w_level9_fetch_stage_16bit_inst_2_destreg4;
wire w_level9_fetch_stage_16bit_inst_2_srcbits4;
wire w_level9_fetch_stage_16bit_inst_2_instruction5;
wire w_level9_fetch_stage_16bit_inst_2_pc5;
wire w_level9_fetch_stage_16bit_inst_2_srcbits5;
wire w_level9_fetch_stage_16bit_inst_2_instruction6;
wire w_level9_fetch_stage_16bit_inst_2_pc6;
wire w_level9_fetch_stage_16bit_inst_2_instruction7;
wire w_level9_fetch_stage_16bit_inst_2_pc7;
wire w_level9_fetch_stage_16bit_inst_2_instruction8;
wire w_level9_fetch_stage_16bit_inst_2_instruction9;
wire w_level9_fetch_stage_16bit_inst_2_instruction10;
wire w_level9_fetch_stage_16bit_inst_2_instruction11;
wire w_level9_fetch_stage_16bit_inst_2_instruction12;
wire w_level9_fetch_stage_16bit_inst_2_instruction13;
wire w_level9_fetch_stage_16bit_inst_2_instruction14;
wire w_level9_fetch_stage_16bit_inst_2_instruction15;
// IC instance: ALU_16bit (level7_alu_16bit)
wire w_level7_alu_16bit_inst_3_result0;
wire w_level7_alu_16bit_inst_3_zero;
wire w_level7_alu_16bit_inst_3_result1;
wire w_level7_alu_16bit_inst_3_sign;
wire w_level7_alu_16bit_inst_3_result2;
wire w_level7_alu_16bit_inst_3_carry;
wire w_level7_alu_16bit_inst_3_result3;
wire w_level7_alu_16bit_inst_3_result4;
wire w_level7_alu_16bit_inst_3_result5;
wire w_level7_alu_16bit_inst_3_result6;
wire w_level7_alu_16bit_inst_3_result7;
wire w_level7_alu_16bit_inst_3_result8;
wire w_level7_alu_16bit_inst_3_result9;
wire w_level7_alu_16bit_inst_3_result10;
wire w_level7_alu_16bit_inst_3_result11;
wire w_level7_alu_16bit_inst_3_result12;
wire w_level7_alu_16bit_inst_3_result13;
wire w_level7_alu_16bit_inst_3_result14;
wire w_level7_alu_16bit_inst_3_result15;

// Internal logic
level9_fetch_stage_16bit level9_fetch_stage_16bit_inst_2 (
    .pcdata0(1'b0),
    .pcdata1(1'b0),
    .pcdata2(1'b0),
    .pcdata3(1'b0),
    .pcdata4(1'b0),
    .pcdata5(1'b0),
    .pcdata6(1'b0),
    .pcdata7(1'b0),
    .clock(clock),
    .reset(reset),
    .pcload(1'b0),
    .pcinc(1'b1),
    .instrload(1'b0),
    .instruction0(w_level9_fetch_stage_16bit_inst_2_instruction0),
    .pc0(w_level9_fetch_stage_16bit_inst_2_pc0),
    .opcode0(w_level9_fetch_stage_16bit_inst_2_opcode0),
    .destreg0(w_level9_fetch_stage_16bit_inst_2_destreg0),
    .srcbits0(w_level9_fetch_stage_16bit_inst_2_srcbits0),
    .instruction1(w_level9_fetch_stage_16bit_inst_2_instruction1),
    .pc1(w_level9_fetch_stage_16bit_inst_2_pc1),
    .opcode1(w_level9_fetch_stage_16bit_inst_2_opcode1),
    .destreg1(w_level9_fetch_stage_16bit_inst_2_destreg1),
    .srcbits1(w_level9_fetch_stage_16bit_inst_2_srcbits1),
    .instruction2(w_level9_fetch_stage_16bit_inst_2_instruction2),
    .pc2(w_level9_fetch_stage_16bit_inst_2_pc2),
    .opcode2(w_level9_fetch_stage_16bit_inst_2_opcode2),
    .destreg2(w_level9_fetch_stage_16bit_inst_2_destreg2),
    .srcbits2(w_level9_fetch_stage_16bit_inst_2_srcbits2),
    .instruction3(w_level9_fetch_stage_16bit_inst_2_instruction3),
    .pc3(w_level9_fetch_stage_16bit_inst_2_pc3),
    .opcode3(w_level9_fetch_stage_16bit_inst_2_opcode3),
    .destreg3(w_level9_fetch_stage_16bit_inst_2_destreg3),
    .srcbits3(w_level9_fetch_stage_16bit_inst_2_srcbits3),
    .instruction4(w_level9_fetch_stage_16bit_inst_2_instruction4),
    .pc4(w_level9_fetch_stage_16bit_inst_2_pc4),
    .opcode4(w_level9_fetch_stage_16bit_inst_2_opcode4),
    .destreg4(w_level9_fetch_stage_16bit_inst_2_destreg4),
    .srcbits4(w_level9_fetch_stage_16bit_inst_2_srcbits4),
    .instruction5(w_level9_fetch_stage_16bit_inst_2_instruction5),
    .pc5(w_level9_fetch_stage_16bit_inst_2_pc5),
    .srcbits5(w_level9_fetch_stage_16bit_inst_2_srcbits5),
    .instruction6(w_level9_fetch_stage_16bit_inst_2_instruction6),
    .pc6(w_level9_fetch_stage_16bit_inst_2_pc6),
    .instruction7(w_level9_fetch_stage_16bit_inst_2_instruction7),
    .pc7(w_level9_fetch_stage_16bit_inst_2_pc7),
    .instruction8(w_level9_fetch_stage_16bit_inst_2_instruction8),
    .instruction9(w_level9_fetch_stage_16bit_inst_2_instruction9),
    .instruction10(w_level9_fetch_stage_16bit_inst_2_instruction10),
    .instruction11(w_level9_fetch_stage_16bit_inst_2_instruction11),
    .instruction12(w_level9_fetch_stage_16bit_inst_2_instruction12),
    .instruction13(w_level9_fetch_stage_16bit_inst_2_instruction13),
    .instruction14(w_level9_fetch_stage_16bit_inst_2_instruction14),
    .instruction15(w_level9_fetch_stage_16bit_inst_2_instruction15)
);
level7_alu_16bit level7_alu_16bit_inst_3 (
    .operanda0(1'b0),
    .operandb0(1'b0),
    .aluop0(1'b0),
    .operanda1(1'b0),
    .operandb1(1'b0),
    .aluop1(1'b0),
    .operanda2(1'b0),
    .operandb2(1'b0),
    .aluop2(1'b0),
    .operanda3(1'b0),
    .operandb3(1'b0),
    .operanda4(1'b0),
    .operandb4(1'b0),
    .operanda5(1'b0),
    .operandb5(1'b0),
    .operanda6(1'b0),
    .operandb6(1'b0),
    .operanda7(1'b0),
    .operandb7(1'b0),
    .operanda8(1'b0),
    .operandb8(1'b0),
    .operanda9(1'b0),
    .operandb9(1'b0),
    .operanda10(1'b0),
    .operandb10(1'b0),
    .operanda11(1'b0),
    .operandb11(1'b0),
    .operanda12(1'b0),
    .operandb12(1'b0),
    .operanda13(1'b0),
    .operandb13(1'b0),
    .operanda14(1'b0),
    .operandb14(1'b0),
    .operanda15(1'b0),
    .operandb15(1'b0),
    .result0(w_level7_alu_16bit_inst_3_result0),
    .zero(w_level7_alu_16bit_inst_3_zero),
    .result1(w_level7_alu_16bit_inst_3_result1),
    .sign(w_level7_alu_16bit_inst_3_sign),
    .result2(w_level7_alu_16bit_inst_3_result2),
    .carry(w_level7_alu_16bit_inst_3_carry),
    .result3(w_level7_alu_16bit_inst_3_result3),
    .result4(w_level7_alu_16bit_inst_3_result4),
    .result5(w_level7_alu_16bit_inst_3_result5),
    .result6(w_level7_alu_16bit_inst_3_result6),
    .result7(w_level7_alu_16bit_inst_3_result7),
    .result8(w_level7_alu_16bit_inst_3_result8),
    .result9(w_level7_alu_16bit_inst_3_result9),
    .result10(w_level7_alu_16bit_inst_3_result10),
    .result11(w_level7_alu_16bit_inst_3_result11),
    .result12(w_level7_alu_16bit_inst_3_result12),
    .result13(w_level7_alu_16bit_inst_3_result13),
    .result14(w_level7_alu_16bit_inst_3_result14),
    .result15(w_level7_alu_16bit_inst_3_result15)
);

assign pc0 = w_level9_fetch_stage_16bit_inst_2_pc0;
assign pc1 = w_level9_fetch_stage_16bit_inst_2_pc1;
assign pc2 = w_level9_fetch_stage_16bit_inst_2_pc2;
assign pc3 = w_level9_fetch_stage_16bit_inst_2_pc3;
assign pc4 = w_level9_fetch_stage_16bit_inst_2_pc4;
assign pc5 = w_level9_fetch_stage_16bit_inst_2_pc5;
assign pc6 = w_level9_fetch_stage_16bit_inst_2_pc6;
assign pc7 = w_level9_fetch_stage_16bit_inst_2_pc7;
assign instr0 = w_level9_fetch_stage_16bit_inst_2_instruction0;
assign opcode0 = w_level9_fetch_stage_16bit_inst_2_opcode0;
assign instr1 = w_level9_fetch_stage_16bit_inst_2_instruction1;
assign opcode1 = w_level9_fetch_stage_16bit_inst_2_opcode1;
assign instr2 = w_level9_fetch_stage_16bit_inst_2_instruction2;
assign opcode2 = w_level9_fetch_stage_16bit_inst_2_opcode2;
assign result0 = w_level7_alu_16bit_inst_3_result0;
assign instr3 = w_level9_fetch_stage_16bit_inst_2_instruction3;
assign opcode3 = w_level9_fetch_stage_16bit_inst_2_opcode3;
assign result1 = w_level7_alu_16bit_inst_3_result1;
assign instr4 = w_level9_fetch_stage_16bit_inst_2_instruction4;
assign opcode4 = w_level9_fetch_stage_16bit_inst_2_opcode4;
assign result2 = w_level7_alu_16bit_inst_3_result2;
assign instr5 = w_level9_fetch_stage_16bit_inst_2_instruction5;
assign result3 = w_level7_alu_16bit_inst_3_result3;
assign instr6 = w_level9_fetch_stage_16bit_inst_2_instruction6;
assign result4 = w_level7_alu_16bit_inst_3_result4;
assign instr7 = w_level9_fetch_stage_16bit_inst_2_instruction7;
assign result5 = w_level7_alu_16bit_inst_3_result5;
assign instr8 = w_level9_fetch_stage_16bit_inst_2_instruction8;
assign result6 = w_level7_alu_16bit_inst_3_result6;
assign instr9 = w_level9_fetch_stage_16bit_inst_2_instruction9;
assign result7 = w_level7_alu_16bit_inst_3_result7;
assign instr10 = w_level9_fetch_stage_16bit_inst_2_instruction10;
assign result8 = w_level7_alu_16bit_inst_3_result8;
assign instr11 = w_level9_fetch_stage_16bit_inst_2_instruction11;
assign result9 = w_level7_alu_16bit_inst_3_result9;
assign instr12 = w_level9_fetch_stage_16bit_inst_2_instruction12;
assign result10 = w_level7_alu_16bit_inst_3_result10;
assign instr13 = w_level9_fetch_stage_16bit_inst_2_instruction13;
assign result11 = w_level7_alu_16bit_inst_3_result11;
assign instr14 = w_level9_fetch_stage_16bit_inst_2_instruction14;
assign result12 = w_level7_alu_16bit_inst_3_result12;
assign instr15 = w_level9_fetch_stage_16bit_inst_2_instruction15;
assign result13 = w_level7_alu_16bit_inst_3_result13;
assign result14 = w_level7_alu_16bit_inst_3_result14;
assign result15 = w_level7_alu_16bit_inst_3_result15;
endmodule

module level9_cpu_16bit_risc (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,

/* ========= Outputs ========== */
output led4_1,
output led5_1,
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
output led21_1,
output led22_1,
output led23_1,
output led24_1,
output led25_1,
output led26_1,
output led27_1,
output led28_1,
output led29_1,
output led30_1,
output led31_1,
output led32_1,
output led33_1,
output led34_1,
output led35_1,
output led36_1,
output led37_1,
output led38_1,
output led39_1,
output led40_1,
output led41_1,
output led42_1,
output led43_1,
output led44_1,
output led45_1,
output led46_1,
output led47_1,
output led48_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL9_CPU_16BIT_RISC (level9_cpu_16bit_risc_ic)
wire w_level9_cpu_16bit_risc_ic_inst_1_pc0;
wire w_level9_cpu_16bit_risc_ic_inst_1_pc1;
wire w_level9_cpu_16bit_risc_ic_inst_1_pc2;
wire w_level9_cpu_16bit_risc_ic_inst_1_pc3;
wire w_level9_cpu_16bit_risc_ic_inst_1_pc4;
wire w_level9_cpu_16bit_risc_ic_inst_1_pc5;
wire w_level9_cpu_16bit_risc_ic_inst_1_pc6;
wire w_level9_cpu_16bit_risc_ic_inst_1_pc7;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr0;
wire w_level9_cpu_16bit_risc_ic_inst_1_opcode0;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr1;
wire w_level9_cpu_16bit_risc_ic_inst_1_opcode1;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr2;
wire w_level9_cpu_16bit_risc_ic_inst_1_opcode2;
wire w_level9_cpu_16bit_risc_ic_inst_1_result0;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr3;
wire w_level9_cpu_16bit_risc_ic_inst_1_opcode3;
wire w_level9_cpu_16bit_risc_ic_inst_1_result1;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr4;
wire w_level9_cpu_16bit_risc_ic_inst_1_opcode4;
wire w_level9_cpu_16bit_risc_ic_inst_1_result2;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr5;
wire w_level9_cpu_16bit_risc_ic_inst_1_result3;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr6;
wire w_level9_cpu_16bit_risc_ic_inst_1_result4;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr7;
wire w_level9_cpu_16bit_risc_ic_inst_1_result5;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr8;
wire w_level9_cpu_16bit_risc_ic_inst_1_result6;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr9;
wire w_level9_cpu_16bit_risc_ic_inst_1_result7;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr10;
wire w_level9_cpu_16bit_risc_ic_inst_1_result8;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr11;
wire w_level9_cpu_16bit_risc_ic_inst_1_result9;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr12;
wire w_level9_cpu_16bit_risc_ic_inst_1_result10;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr13;
wire w_level9_cpu_16bit_risc_ic_inst_1_result11;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr14;
wire w_level9_cpu_16bit_risc_ic_inst_1_result12;
wire w_level9_cpu_16bit_risc_ic_inst_1_instr15;
wire w_level9_cpu_16bit_risc_ic_inst_1_result13;
wire w_level9_cpu_16bit_risc_ic_inst_1_result14;
wire w_level9_cpu_16bit_risc_ic_inst_1_result15;


// Assigning aux variables. //
level9_cpu_16bit_risc_ic level9_cpu_16bit_risc_ic_inst_1 (
    .clock(input_switch1),
    .reset(input_switch2),
    .pc0(w_level9_cpu_16bit_risc_ic_inst_1_pc0),
    .pc1(w_level9_cpu_16bit_risc_ic_inst_1_pc1),
    .pc2(w_level9_cpu_16bit_risc_ic_inst_1_pc2),
    .pc3(w_level9_cpu_16bit_risc_ic_inst_1_pc3),
    .pc4(w_level9_cpu_16bit_risc_ic_inst_1_pc4),
    .pc5(w_level9_cpu_16bit_risc_ic_inst_1_pc5),
    .pc6(w_level9_cpu_16bit_risc_ic_inst_1_pc6),
    .pc7(w_level9_cpu_16bit_risc_ic_inst_1_pc7),
    .instr0(w_level9_cpu_16bit_risc_ic_inst_1_instr0),
    .opcode0(w_level9_cpu_16bit_risc_ic_inst_1_opcode0),
    .instr1(w_level9_cpu_16bit_risc_ic_inst_1_instr1),
    .opcode1(w_level9_cpu_16bit_risc_ic_inst_1_opcode1),
    .instr2(w_level9_cpu_16bit_risc_ic_inst_1_instr2),
    .opcode2(w_level9_cpu_16bit_risc_ic_inst_1_opcode2),
    .result0(w_level9_cpu_16bit_risc_ic_inst_1_result0),
    .instr3(w_level9_cpu_16bit_risc_ic_inst_1_instr3),
    .opcode3(w_level9_cpu_16bit_risc_ic_inst_1_opcode3),
    .result1(w_level9_cpu_16bit_risc_ic_inst_1_result1),
    .instr4(w_level9_cpu_16bit_risc_ic_inst_1_instr4),
    .opcode4(w_level9_cpu_16bit_risc_ic_inst_1_opcode4),
    .result2(w_level9_cpu_16bit_risc_ic_inst_1_result2),
    .instr5(w_level9_cpu_16bit_risc_ic_inst_1_instr5),
    .result3(w_level9_cpu_16bit_risc_ic_inst_1_result3),
    .instr6(w_level9_cpu_16bit_risc_ic_inst_1_instr6),
    .result4(w_level9_cpu_16bit_risc_ic_inst_1_result4),
    .instr7(w_level9_cpu_16bit_risc_ic_inst_1_instr7),
    .result5(w_level9_cpu_16bit_risc_ic_inst_1_result5),
    .instr8(w_level9_cpu_16bit_risc_ic_inst_1_instr8),
    .result6(w_level9_cpu_16bit_risc_ic_inst_1_result6),
    .instr9(w_level9_cpu_16bit_risc_ic_inst_1_instr9),
    .result7(w_level9_cpu_16bit_risc_ic_inst_1_result7),
    .instr10(w_level9_cpu_16bit_risc_ic_inst_1_instr10),
    .result8(w_level9_cpu_16bit_risc_ic_inst_1_result8),
    .instr11(w_level9_cpu_16bit_risc_ic_inst_1_instr11),
    .result9(w_level9_cpu_16bit_risc_ic_inst_1_result9),
    .instr12(w_level9_cpu_16bit_risc_ic_inst_1_instr12),
    .result10(w_level9_cpu_16bit_risc_ic_inst_1_result10),
    .instr13(w_level9_cpu_16bit_risc_ic_inst_1_instr13),
    .result11(w_level9_cpu_16bit_risc_ic_inst_1_result11),
    .instr14(w_level9_cpu_16bit_risc_ic_inst_1_instr14),
    .result12(w_level9_cpu_16bit_risc_ic_inst_1_result12),
    .instr15(w_level9_cpu_16bit_risc_ic_inst_1_instr15),
    .result13(w_level9_cpu_16bit_risc_ic_inst_1_result13),
    .result14(w_level9_cpu_16bit_risc_ic_inst_1_result14),
    .result15(w_level9_cpu_16bit_risc_ic_inst_1_result15)
);

// Writing output data. //
assign led4_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc0;
assign led5_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc1;
assign led6_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc2;
assign led7_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc3;
assign led8_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc4;
assign led9_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc5;
assign led10_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc6;
assign led11_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc7;
assign led12_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr0;
assign led13_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode0;
assign led14_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr1;
assign led15_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode1;
assign led16_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr2;
assign led17_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode2;
assign led18_1 = w_level9_cpu_16bit_risc_ic_inst_1_result0;
assign led19_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr3;
assign led20_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode3;
assign led21_1 = w_level9_cpu_16bit_risc_ic_inst_1_result1;
assign led22_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr4;
assign led23_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode4;
assign led24_1 = w_level9_cpu_16bit_risc_ic_inst_1_result2;
assign led25_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr5;
assign led26_1 = w_level9_cpu_16bit_risc_ic_inst_1_result3;
assign led27_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr6;
assign led28_1 = w_level9_cpu_16bit_risc_ic_inst_1_result4;
assign led29_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr7;
assign led30_1 = w_level9_cpu_16bit_risc_ic_inst_1_result5;
assign led31_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr8;
assign led32_1 = w_level9_cpu_16bit_risc_ic_inst_1_result6;
assign led33_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr9;
assign led34_1 = w_level9_cpu_16bit_risc_ic_inst_1_result7;
assign led35_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr10;
assign led36_1 = w_level9_cpu_16bit_risc_ic_inst_1_result8;
assign led37_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr11;
assign led38_1 = w_level9_cpu_16bit_risc_ic_inst_1_result9;
assign led39_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr12;
assign led40_1 = w_level9_cpu_16bit_risc_ic_inst_1_result10;
assign led41_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr13;
assign led42_1 = w_level9_cpu_16bit_risc_ic_inst_1_result11;
assign led43_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr14;
assign led44_1 = w_level9_cpu_16bit_risc_ic_inst_1_result12;
assign led45_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr15;
assign led46_1 = w_level9_cpu_16bit_risc_ic_inst_1_result13;
assign led47_1 = w_level9_cpu_16bit_risc_ic_inst_1_result14;
assign led48_1 = w_level9_cpu_16bit_risc_ic_inst_1_result15;
endmodule
