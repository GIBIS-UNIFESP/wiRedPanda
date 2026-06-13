// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for AddrDecoder (generated from level2_decoder_3to8.panda)
module level2_decoder_3to8 (
    input addr0,
    input addr1,
    input addr2,
    input enable,
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
assign aux_and_4 = (aux_not_1 & aux_not_2 & aux_not_3 & enable);
assign aux_and_5 = (addr0 & aux_not_2 & aux_not_3 & enable);
assign aux_and_6 = (aux_not_1 & addr1 & aux_not_3 & enable);
assign aux_and_7 = (addr0 & addr1 & aux_not_3 & enable);
assign aux_and_8 = (aux_not_1 & aux_not_2 & addr2 & enable);
assign aux_and_9 = (addr0 & aux_not_2 & addr2 & enable);
assign aux_and_10 = (aux_not_1 & addr1 & addr2 & enable);
assign aux_and_11 = (addr0 & addr1 & addr2 & enable);

assign out0 = aux_and_4;
assign out1 = aux_and_5;
assign out2 = aux_and_6;
assign out3 = aux_and_7;
assign out4 = aux_and_8;
assign out5 = aux_and_9;
assign out6 = aux_and_10;
assign out7 = aux_and_11;
endmodule

// Module for HA1 (generated from level2_half_adder.panda)
module level2_half_adder (
    input a,
    input b,
    output sum,
    output carry
);

wire aux_xor_1;
wire aux_and_2;

// Internal logic
assign aux_xor_1 = (a ^ b);
assign aux_and_2 = (a & b);

assign sum = aux_xor_1;
assign carry = aux_and_2;
endmodule

// Module for Mux[0] (generated from level2_mux_2to1.panda)
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
    input enable,
    output p_output
);

reg aux_mux_1 = 1'b0;
wire aux_and_2;

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
wire w_level2_mux_2to1_inst_2_p_output;
// IC instance: l1_mux2 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_3_p_output;
// IC instance: l2_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_4_p_output;
// IC instance: l3_mux (level2_mux_2to1)
wire w_level2_mux_2to1_inst_5_p_output;

// Internal logic
level2_mux_2to1 level2_mux_2to1_inst_2 (
    .data0(result0),
    .data1(result1),
    .sel0(op0),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_2_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_3 (
    .data0(result2),
    .data1(result3),
    .sel0(op0),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_3_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_4 (
    .data0(w_level2_mux_2to1_inst_2_p_output),
    .data1(w_level2_mux_2to1_inst_3_p_output),
    .sel0(op1),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_4_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_5 (
    .data0(w_level2_mux_2to1_inst_4_p_output),
    .data1(result4),
    .sel0(op2),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_5_p_output)
);

assign out = w_level2_mux_2to1_inst_5_p_output;
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

// Module for BusMux[0-3] (generated from level4_bus_mux_4bit.panda)
module level4_bus_mux_4bit (
    input in00,
    input in01,
    input in02,
    input in03,
    input in10,
    input in11,
    input in12,
    input in13,
    input sel,
    output out0,
    output out1,
    output out2,
    output out3
);

// IC instance: Mux[0] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_1_p_output;
// IC instance: Mux[1] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_2_p_output;
// IC instance: Mux[2] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_3_p_output;
// IC instance: Mux[3] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_4_p_output;

// Internal logic
level2_mux_2to1 level2_mux_2to1_inst_1 (
    .data0(in00),
    .data1(in10),
    .sel0(sel),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_1_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_2 (
    .data0(in01),
    .data1(in11),
    .sel0(sel),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_2_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_3 (
    .data0(in02),
    .data1(in12),
    .sel0(sel),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_3_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_4 (
    .data0(in03),
    .data1(in13),
    .sel0(sel),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_4_p_output)
);

assign out0 = w_level2_mux_2to1_inst_1_p_output;
assign out1 = w_level2_mux_2to1_inst_2_p_output;
assign out2 = w_level2_mux_2to1_inst_3_p_output;
assign out3 = w_level2_mux_2to1_inst_4_p_output;
endmodule

// Module for AddrMux (generated from level4_bus_mux_8bit.panda)
module level4_bus_mux_8bit (
    input in00,
    input in01,
    input in02,
    input in03,
    input in04,
    input in05,
    input in06,
    input in07,
    input in10,
    input in11,
    input in12,
    input in13,
    input in14,
    input in15,
    input in16,
    input in17,
    input sel,
    output out0,
    output out1,
    output out2,
    output out3,
    output out4,
    output out5,
    output out6,
    output out7
);

// IC instance: BusMux[0-3] (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_1_out0;
wire w_level4_bus_mux_4bit_inst_1_out1;
wire w_level4_bus_mux_4bit_inst_1_out2;
wire w_level4_bus_mux_4bit_inst_1_out3;
// IC instance: BusMux[4-7] (level4_bus_mux_4bit)
wire w_level4_bus_mux_4bit_inst_2_out0;
wire w_level4_bus_mux_4bit_inst_2_out1;
wire w_level4_bus_mux_4bit_inst_2_out2;
wire w_level4_bus_mux_4bit_inst_2_out3;

// Internal logic
level4_bus_mux_4bit level4_bus_mux_4bit_inst_1 (
    .in00(in00),
    .in01(in01),
    .in02(in02),
    .in03(in03),
    .in10(in10),
    .in11(in11),
    .in12(in12),
    .in13(in13),
    .sel(sel),
    .out0(w_level4_bus_mux_4bit_inst_1_out0),
    .out1(w_level4_bus_mux_4bit_inst_1_out1),
    .out2(w_level4_bus_mux_4bit_inst_1_out2),
    .out3(w_level4_bus_mux_4bit_inst_1_out3)
);
level4_bus_mux_4bit level4_bus_mux_4bit_inst_2 (
    .in00(in04),
    .in01(in05),
    .in02(in06),
    .in03(in07),
    .in10(in14),
    .in11(in15),
    .in12(in16),
    .in13(in17),
    .sel(sel),
    .out0(w_level4_bus_mux_4bit_inst_2_out0),
    .out1(w_level4_bus_mux_4bit_inst_2_out1),
    .out2(w_level4_bus_mux_4bit_inst_2_out2),
    .out3(w_level4_bus_mux_4bit_inst_2_out3)
);

assign out0 = w_level4_bus_mux_4bit_inst_1_out0;
assign out1 = w_level4_bus_mux_4bit_inst_1_out1;
assign out2 = w_level4_bus_mux_4bit_inst_1_out2;
assign out3 = w_level4_bus_mux_4bit_inst_1_out3;
assign out4 = w_level4_bus_mux_4bit_inst_2_out0;
assign out5 = w_level4_bus_mux_4bit_inst_2_out1;
assign out6 = w_level4_bus_mux_4bit_inst_2_out2;
assign out7 = w_level4_bus_mux_4bit_inst_2_out3;
endmodule

// Module for RAM_Bit0 (generated from level4_ram_8x1.panda)
module level4_ram_8x1 (
    input address0,
    input address1,
    input address2,
    input datain,
    input writeenable,
    input clock,
    input reset,
    output dataout
);

wire aux_not_1;
reg aux_mux_2 = 1'b0;
reg aux_d_flip_flop_3_0_q = 1'b0;
reg aux_d_flip_flop_3_1_q = 1'b1;
reg aux_mux_4 = 1'b0;
reg aux_d_flip_flop_5_0_q = 1'b0;
reg aux_d_flip_flop_5_1_q = 1'b1;
reg aux_mux_6 = 1'b0;
reg aux_d_flip_flop_7_0_q = 1'b0;
reg aux_d_flip_flop_7_1_q = 1'b1;
reg aux_mux_8 = 1'b0;
reg aux_d_flip_flop_9_0_q = 1'b0;
reg aux_d_flip_flop_9_1_q = 1'b1;
reg aux_mux_10 = 1'b0;
reg aux_d_flip_flop_11_0_q = 1'b0;
reg aux_d_flip_flop_11_1_q = 1'b1;
reg aux_mux_12 = 1'b0;
reg aux_d_flip_flop_13_0_q = 1'b0;
reg aux_d_flip_flop_13_1_q = 1'b1;
reg aux_mux_14 = 1'b0;
reg aux_d_flip_flop_15_0_q = 1'b0;
reg aux_d_flip_flop_15_1_q = 1'b1;
reg aux_mux_16 = 1'b0;
reg aux_d_flip_flop_17_0_q = 1'b0;
reg aux_d_flip_flop_17_1_q = 1'b1;
// IC instance: AddrDecoder (level2_decoder_3to8)
wire w_level2_decoder_3to8_inst_18_out0;
wire w_level2_decoder_3to8_inst_18_out1;
wire w_level2_decoder_3to8_inst_18_out2;
wire w_level2_decoder_3to8_inst_18_out3;
wire w_level2_decoder_3to8_inst_18_out4;
wire w_level2_decoder_3to8_inst_18_out5;
wire w_level2_decoder_3to8_inst_18_out6;
wire w_level2_decoder_3to8_inst_18_out7;
// IC instance: ReadMux (level2_mux_8to1)
wire w_level2_mux_8to1_inst_19_p_output;

// Internal logic
assign aux_not_1 = ~reset;
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_18_out0})
            1'd0: aux_mux_2 = aux_d_flip_flop_3_0_q;
            1'd1: aux_mux_2 = datain;
            default: aux_mux_2 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_3_0_q <= 1'b0;
            aux_d_flip_flop_3_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_3_0_q <= aux_mux_2;
            aux_d_flip_flop_3_1_q <= ~aux_mux_2;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_18_out1})
            1'd0: aux_mux_4 = aux_d_flip_flop_5_0_q;
            1'd1: aux_mux_4 = datain;
            default: aux_mux_4 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_5_0_q <= 1'b0;
            aux_d_flip_flop_5_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_5_0_q <= aux_mux_4;
            aux_d_flip_flop_5_1_q <= ~aux_mux_4;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_18_out2})
            1'd0: aux_mux_6 = aux_d_flip_flop_7_0_q;
            1'd1: aux_mux_6 = datain;
            default: aux_mux_6 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_7_0_q <= 1'b0;
            aux_d_flip_flop_7_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_7_0_q <= aux_mux_6;
            aux_d_flip_flop_7_1_q <= ~aux_mux_6;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_18_out3})
            1'd0: aux_mux_8 = aux_d_flip_flop_9_0_q;
            1'd1: aux_mux_8 = datain;
            default: aux_mux_8 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_9_0_q <= 1'b0;
            aux_d_flip_flop_9_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_9_0_q <= aux_mux_8;
            aux_d_flip_flop_9_1_q <= ~aux_mux_8;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_18_out4})
            1'd0: aux_mux_10 = aux_d_flip_flop_11_0_q;
            1'd1: aux_mux_10 = datain;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_11_0_q <= 1'b0;
            aux_d_flip_flop_11_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_11_0_q <= aux_mux_10;
            aux_d_flip_flop_11_1_q <= ~aux_mux_10;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_18_out5})
            1'd0: aux_mux_12 = aux_d_flip_flop_13_0_q;
            1'd1: aux_mux_12 = datain;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_13_0_q <= 1'b0;
            aux_d_flip_flop_13_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_13_0_q <= aux_mux_12;
            aux_d_flip_flop_13_1_q <= ~aux_mux_12;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_18_out6})
            1'd0: aux_mux_14 = aux_d_flip_flop_15_0_q;
            1'd1: aux_mux_14 = datain;
            default: aux_mux_14 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_15_0_q <= 1'b0;
            aux_d_flip_flop_15_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_15_0_q <= aux_mux_14;
            aux_d_flip_flop_15_1_q <= ~aux_mux_14;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_18_out7})
            1'd0: aux_mux_16 = aux_d_flip_flop_17_0_q;
            1'd1: aux_mux_16 = datain;
            default: aux_mux_16 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_17_0_q <= 1'b0;
            aux_d_flip_flop_17_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_17_0_q <= aux_mux_16;
            aux_d_flip_flop_17_1_q <= ~aux_mux_16;
        end
    end
    //End of D FlipFlop
level2_decoder_3to8 level2_decoder_3to8_inst_18 (
    .addr0(address0),
    .addr1(address1),
    .addr2(address2),
    .enable(writeenable),
    .out0(w_level2_decoder_3to8_inst_18_out0),
    .out1(w_level2_decoder_3to8_inst_18_out1),
    .out2(w_level2_decoder_3to8_inst_18_out2),
    .out3(w_level2_decoder_3to8_inst_18_out3),
    .out4(w_level2_decoder_3to8_inst_18_out4),
    .out5(w_level2_decoder_3to8_inst_18_out5),
    .out6(w_level2_decoder_3to8_inst_18_out6),
    .out7(w_level2_decoder_3to8_inst_18_out7)
);
level2_mux_8to1 level2_mux_8to1_inst_19 (
    .data0(aux_d_flip_flop_3_0_q),
    .data1(aux_d_flip_flop_5_0_q),
    .data2(aux_d_flip_flop_7_0_q),
    .data3(aux_d_flip_flop_9_0_q),
    .data4(aux_d_flip_flop_11_0_q),
    .data5(aux_d_flip_flop_13_0_q),
    .data6(aux_d_flip_flop_15_0_q),
    .data7(aux_d_flip_flop_17_0_q),
    .sel0(address0),
    .sel1(address1),
    .sel2(address2),
    .enable(1'b1),
    .p_output(w_level2_mux_8to1_inst_19_p_output)
);

assign dataout = w_level2_mux_8to1_inst_19_p_output;
endmodule

// Module for InstructionMemory (generated from level6_ram_8x8.panda)
module level6_ram_8x8 (
    input address0,
    input address1,
    input address2,
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
    input reset,
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
    .reset(reset),
    .dataout(w_level4_ram_8x1_inst_1_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_2 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain1),
    .writeenable(writeenable),
    .clock(clock),
    .reset(reset),
    .dataout(w_level4_ram_8x1_inst_2_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_3 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain2),
    .writeenable(writeenable),
    .clock(clock),
    .reset(reset),
    .dataout(w_level4_ram_8x1_inst_3_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_4 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain3),
    .writeenable(writeenable),
    .clock(clock),
    .reset(reset),
    .dataout(w_level4_ram_8x1_inst_4_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_5 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain4),
    .writeenable(writeenable),
    .clock(clock),
    .reset(reset),
    .dataout(w_level4_ram_8x1_inst_5_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_6 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain5),
    .writeenable(writeenable),
    .clock(clock),
    .reset(reset),
    .dataout(w_level4_ram_8x1_inst_6_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_7 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain6),
    .writeenable(writeenable),
    .clock(clock),
    .reset(reset),
    .dataout(w_level4_ram_8x1_inst_7_dataout)
);
level4_ram_8x1 level4_ram_8x1_inst_8 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain(datain7),
    .writeenable(writeenable),
    .clock(clock),
    .reset(reset),
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

// IC instance: InstructionMemory (level6_ram_8x8)
wire w_level6_ram_8x8_inst_1_dataout0;
wire w_level6_ram_8x8_inst_1_dataout1;
wire w_level6_ram_8x8_inst_1_dataout2;
wire w_level6_ram_8x8_inst_1_dataout3;
wire w_level6_ram_8x8_inst_1_dataout4;
wire w_level6_ram_8x8_inst_1_dataout5;
wire w_level6_ram_8x8_inst_1_dataout6;
wire w_level6_ram_8x8_inst_1_dataout7;

// Internal logic
level6_ram_8x8 level6_ram_8x8_inst_1 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
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
    .reset(1'b0),
    .dataout0(w_level6_ram_8x8_inst_1_dataout0),
    .dataout1(w_level6_ram_8x8_inst_1_dataout1),
    .dataout2(w_level6_ram_8x8_inst_1_dataout2),
    .dataout3(w_level6_ram_8x8_inst_1_dataout3),
    .dataout4(w_level6_ram_8x8_inst_1_dataout4),
    .dataout5(w_level6_ram_8x8_inst_1_dataout5),
    .dataout6(w_level6_ram_8x8_inst_1_dataout6),
    .dataout7(w_level6_ram_8x8_inst_1_dataout7)
);

assign instruction0 = w_level6_ram_8x8_inst_1_dataout0;
assign instruction1 = w_level6_ram_8x8_inst_1_dataout1;
assign instruction2 = w_level6_ram_8x8_inst_1_dataout2;
assign instruction3 = w_level6_ram_8x8_inst_1_dataout3;
assign instruction4 = w_level6_ram_8x8_inst_1_dataout4;
assign instruction5 = w_level6_ram_8x8_inst_1_dataout5;
assign instruction6 = w_level6_ram_8x8_inst_1_dataout6;
assign instruction7 = w_level6_ram_8x8_inst_1_dataout7;
endmodule

// Module for FA[0] (generated from level2_full_adder_1bit.panda)
module level2_full_adder_1bit (
    input a,
    input b,
    input cin,
    output sum,
    output cout
);

// IC instance: HA1 (level2_half_adder)
wire w_level2_half_adder_inst_1_sum;
wire w_level2_half_adder_inst_1_carry;
// IC instance: HA2 (level2_half_adder)
wire w_level2_half_adder_inst_2_sum;
wire w_level2_half_adder_inst_2_carry;
wire aux_or_3;

// Internal logic
level2_half_adder level2_half_adder_inst_1 (
    .a(a),
    .b(b),
    .sum(w_level2_half_adder_inst_1_sum),
    .carry(w_level2_half_adder_inst_1_carry)
);
level2_half_adder level2_half_adder_inst_2 (
    .a(w_level2_half_adder_inst_1_sum),
    .b(cin),
    .sum(w_level2_half_adder_inst_2_sum),
    .carry(w_level2_half_adder_inst_2_carry)
);
assign aux_or_3 = (w_level2_half_adder_inst_1_carry | w_level2_half_adder_inst_2_carry);

assign sum = w_level2_half_adder_inst_2_sum;
assign cout = aux_or_3;
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
wire w_level6_alu_8bit_inst_1_subcarryout;
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
wire w_level6_alu_8bit_inst_2_subcarryout;
wire w_level6_alu_8bit_inst_2_result5;
wire w_level6_alu_8bit_inst_2_result6;
wire w_level6_alu_8bit_inst_2_result7;
wire aux_and_3;

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
    .carryin(1'b0),
    .subcarryin(1'b1),
    .shrin(operanda8),
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
    .carryin(w_level6_alu_8bit_inst_1_carry),
    .subcarryin(w_level6_alu_8bit_inst_1_subcarryout),
    .shrin(1'b0),
    .shlin(operanda7),
    .result0(w_level6_alu_8bit_inst_2_result0),
    .result1(w_level6_alu_8bit_inst_2_result1),
    .zero(w_level6_alu_8bit_inst_2_zero),
    .result2(w_level6_alu_8bit_inst_2_result2),
    .negative(w_level6_alu_8bit_inst_2_negative),
    .result3(w_level6_alu_8bit_inst_2_result3),
    .carry(w_level6_alu_8bit_inst_2_carry),
    .result4(w_level6_alu_8bit_inst_2_result4),
    .subcarryout(w_level6_alu_8bit_inst_2_subcarryout),
    .result5(w_level6_alu_8bit_inst_2_result5),
    .result6(w_level6_alu_8bit_inst_2_result6),
    .result7(w_level6_alu_8bit_inst_2_result7)
);
assign aux_and_3 = (w_level6_alu_8bit_inst_1_zero & w_level6_alu_8bit_inst_2_zero);

assign result0 = w_level6_alu_8bit_inst_1_result0;
assign zero = aux_and_3;
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
wire aux_not_5;
wire aux_and_6;
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
reg aux_mux_22 = 1'b0;
wire aux_or_23;

// Internal logic
level6_register_8bit level6_register_8bit_inst_1 (
    .data0(aux_mux_15),
    .clock(clock),
    .data1(aux_mux_16),
    .writeenable(aux_or_23),
    .data2(aux_mux_17),
    .reset(reset),
    .data3(aux_mux_18),
    .data4(aux_mux_19),
    .data5(aux_mux_20),
    .data6(aux_mux_21),
    .data7(aux_mux_22),
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
assign aux_not_5 = ~load;
assign aux_and_6 = (inc & aux_not_5);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_7 = w_level6_register_8bit_inst_1_q0;
            1'd1: aux_mux_7 = w_level6_ripple_adder_8bit_inst_2_sum0;
            default: aux_mux_7 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_8 = w_level6_register_8bit_inst_1_q1;
            1'd1: aux_mux_8 = w_level6_ripple_adder_8bit_inst_2_sum1;
            default: aux_mux_8 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_9 = w_level6_register_8bit_inst_1_q2;
            1'd1: aux_mux_9 = w_level6_ripple_adder_8bit_inst_2_sum2;
            default: aux_mux_9 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_10 = w_level6_register_8bit_inst_1_q3;
            1'd1: aux_mux_10 = w_level6_ripple_adder_8bit_inst_2_sum3;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_11 = w_level6_register_8bit_inst_1_q4;
            1'd1: aux_mux_11 = w_level6_ripple_adder_8bit_inst_2_sum4;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_12 = w_level6_register_8bit_inst_1_q5;
            1'd1: aux_mux_12 = w_level6_ripple_adder_8bit_inst_2_sum5;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_13 = w_level6_register_8bit_inst_1_q6;
            1'd1: aux_mux_13 = w_level6_ripple_adder_8bit_inst_2_sum6;
            default: aux_mux_13 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_14 = w_level6_register_8bit_inst_1_q7;
            1'd1: aux_mux_14 = w_level6_ripple_adder_8bit_inst_2_sum7;
            default: aux_mux_14 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_15 = aux_mux_7;
            1'd1: aux_mux_15 = loadvalue0;
            default: aux_mux_15 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_16 = aux_mux_8;
            1'd1: aux_mux_16 = loadvalue1;
            default: aux_mux_16 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_17 = aux_mux_9;
            1'd1: aux_mux_17 = loadvalue2;
            default: aux_mux_17 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_18 = aux_mux_10;
            1'd1: aux_mux_18 = loadvalue3;
            default: aux_mux_18 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_19 = aux_mux_11;
            1'd1: aux_mux_19 = loadvalue4;
            default: aux_mux_19 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_20 = aux_mux_12;
            1'd1: aux_mux_20 = loadvalue5;
            default: aux_mux_20 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_21 = aux_mux_13;
            1'd1: aux_mux_21 = loadvalue6;
            default: aux_mux_21 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({load})
            1'd0: aux_mux_22 = aux_mux_14;
            1'd1: aux_mux_22 = loadvalue7;
            default: aux_mux_22 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_or_23 = (load | inc);

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
    input progaddr0,
    input progaddr1,
    input progaddr2,
    input progaddr3,
    input progaddr4,
    input progaddr5,
    input progaddr6,
    input progaddr7,
    input progwrite,
    input progdata0,
    input progdata1,
    input progdata2,
    input progdata3,
    input progdata4,
    input progdata5,
    input progdata6,
    input progdata7,
    input progdata8,
    input progdata9,
    input progdata10,
    input progdata11,
    input progdata12,
    input progdata13,
    input progdata14,
    input progdata15,
    output instruction0,
    output rawinstr0,
    output pc0,
    output opcode0,
    output destreg0,
    output srcbits0,
    output instruction1,
    output rawinstr1,
    output pc1,
    output opcode1,
    output destreg1,
    output srcbits1,
    output instruction2,
    output rawinstr2,
    output pc2,
    output opcode2,
    output destreg2,
    output srcbits2,
    output instruction3,
    output rawinstr3,
    output pc3,
    output opcode3,
    output destreg3,
    output srcbits3,
    output instruction4,
    output rawinstr4,
    output pc4,
    output opcode4,
    output destreg4,
    output srcbits4,
    output instruction5,
    output rawinstr5,
    output pc5,
    output srcbits5,
    output instruction6,
    output rawinstr6,
    output pc6,
    output instruction7,
    output rawinstr7,
    output pc7,
    output instruction8,
    output rawinstr8,
    output instruction9,
    output rawinstr9,
    output instruction10,
    output rawinstr10,
    output instruction11,
    output rawinstr11,
    output instruction12,
    output rawinstr12,
    output instruction13,
    output rawinstr13,
    output instruction14,
    output rawinstr14,
    output instruction15,
    output rawinstr15
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
// IC instance: AddrMux (level4_bus_mux_8bit)
wire w_level4_bus_mux_8bit_inst_2_out0;
wire w_level4_bus_mux_8bit_inst_2_out1;
wire w_level4_bus_mux_8bit_inst_2_out2;
wire w_level4_bus_mux_8bit_inst_2_out3;
wire w_level4_bus_mux_8bit_inst_2_out4;
wire w_level4_bus_mux_8bit_inst_2_out5;
wire w_level4_bus_mux_8bit_inst_2_out6;
wire w_level4_bus_mux_8bit_inst_2_out7;
// IC instance: InstrMem_Low (level7_instruction_memory_interface)
wire w_level7_instruction_memory_interface_inst_3_instruction0;
wire w_level7_instruction_memory_interface_inst_3_instruction1;
wire w_level7_instruction_memory_interface_inst_3_instruction2;
wire w_level7_instruction_memory_interface_inst_3_instruction3;
wire w_level7_instruction_memory_interface_inst_3_instruction4;
wire w_level7_instruction_memory_interface_inst_3_instruction5;
wire w_level7_instruction_memory_interface_inst_3_instruction6;
wire w_level7_instruction_memory_interface_inst_3_instruction7;
// IC instance: InstrMem_High (level7_instruction_memory_interface)
wire w_level7_instruction_memory_interface_inst_4_instruction0;
wire w_level7_instruction_memory_interface_inst_4_instruction1;
wire w_level7_instruction_memory_interface_inst_4_instruction2;
wire w_level7_instruction_memory_interface_inst_4_instruction3;
wire w_level7_instruction_memory_interface_inst_4_instruction4;
wire w_level7_instruction_memory_interface_inst_4_instruction5;
wire w_level7_instruction_memory_interface_inst_4_instruction6;
wire w_level7_instruction_memory_interface_inst_4_instruction7;
// IC instance: IR_Low (level6_register_8bit)
wire w_level6_register_8bit_inst_5_q0;
wire w_level6_register_8bit_inst_5_q1;
wire w_level6_register_8bit_inst_5_q2;
wire w_level6_register_8bit_inst_5_q3;
wire w_level6_register_8bit_inst_5_q4;
wire w_level6_register_8bit_inst_5_q5;
wire w_level6_register_8bit_inst_5_q6;
wire w_level6_register_8bit_inst_5_q7;
// IC instance: IR_High (level6_register_8bit)
wire w_level6_register_8bit_inst_6_q0;
wire w_level6_register_8bit_inst_6_q1;
wire w_level6_register_8bit_inst_6_q2;
wire w_level6_register_8bit_inst_6_q3;
wire w_level6_register_8bit_inst_6_q4;
wire w_level6_register_8bit_inst_6_q5;
wire w_level6_register_8bit_inst_6_q6;
wire w_level6_register_8bit_inst_6_q7;

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
    .address0(w_level7_cpu_program_counter_8bit_inst_1_address0),
    .address1(w_level7_cpu_program_counter_8bit_inst_1_address1),
    .address2(w_level7_cpu_program_counter_8bit_inst_1_address2),
    .address3(w_level7_cpu_program_counter_8bit_inst_1_address3),
    .address4(w_level7_cpu_program_counter_8bit_inst_1_address4),
    .address5(w_level7_cpu_program_counter_8bit_inst_1_address5),
    .address6(w_level7_cpu_program_counter_8bit_inst_1_address6),
    .address7(w_level7_cpu_program_counter_8bit_inst_1_address7)
);
level4_bus_mux_8bit level4_bus_mux_8bit_inst_2 (
    .in00(w_level7_cpu_program_counter_8bit_inst_1_address0),
    .in01(w_level7_cpu_program_counter_8bit_inst_1_address1),
    .in02(w_level7_cpu_program_counter_8bit_inst_1_address2),
    .in03(w_level7_cpu_program_counter_8bit_inst_1_address3),
    .in04(w_level7_cpu_program_counter_8bit_inst_1_address4),
    .in05(w_level7_cpu_program_counter_8bit_inst_1_address5),
    .in06(w_level7_cpu_program_counter_8bit_inst_1_address6),
    .in07(w_level7_cpu_program_counter_8bit_inst_1_address7),
    .in10(progaddr0),
    .in11(progaddr1),
    .in12(progaddr2),
    .in13(progaddr3),
    .in14(progaddr4),
    .in15(progaddr5),
    .in16(progaddr6),
    .in17(progaddr7),
    .sel(progwrite),
    .out0(w_level4_bus_mux_8bit_inst_2_out0),
    .out1(w_level4_bus_mux_8bit_inst_2_out1),
    .out2(w_level4_bus_mux_8bit_inst_2_out2),
    .out3(w_level4_bus_mux_8bit_inst_2_out3),
    .out4(w_level4_bus_mux_8bit_inst_2_out4),
    .out5(w_level4_bus_mux_8bit_inst_2_out5),
    .out6(w_level4_bus_mux_8bit_inst_2_out6),
    .out7(w_level4_bus_mux_8bit_inst_2_out7)
);
level7_instruction_memory_interface level7_instruction_memory_interface_inst_3 (
    .address0(w_level4_bus_mux_8bit_inst_2_out0),
    .address1(w_level4_bus_mux_8bit_inst_2_out1),
    .address2(w_level4_bus_mux_8bit_inst_2_out2),
    .address3(w_level4_bus_mux_8bit_inst_2_out3),
    .address4(w_level4_bus_mux_8bit_inst_2_out4),
    .address5(w_level4_bus_mux_8bit_inst_2_out5),
    .address6(w_level4_bus_mux_8bit_inst_2_out6),
    .address7(w_level4_bus_mux_8bit_inst_2_out7),
    .clock(clock),
    .datain0(progdata0),
    .datain1(progdata1),
    .datain2(progdata2),
    .datain3(progdata3),
    .datain4(progdata4),
    .datain5(progdata5),
    .datain6(progdata6),
    .datain7(progdata7),
    .writeenable(progwrite),
    .instruction0(w_level7_instruction_memory_interface_inst_3_instruction0),
    .instruction1(w_level7_instruction_memory_interface_inst_3_instruction1),
    .instruction2(w_level7_instruction_memory_interface_inst_3_instruction2),
    .instruction3(w_level7_instruction_memory_interface_inst_3_instruction3),
    .instruction4(w_level7_instruction_memory_interface_inst_3_instruction4),
    .instruction5(w_level7_instruction_memory_interface_inst_3_instruction5),
    .instruction6(w_level7_instruction_memory_interface_inst_3_instruction6),
    .instruction7(w_level7_instruction_memory_interface_inst_3_instruction7)
);
level7_instruction_memory_interface level7_instruction_memory_interface_inst_4 (
    .address0(w_level4_bus_mux_8bit_inst_2_out0),
    .address1(w_level4_bus_mux_8bit_inst_2_out1),
    .address2(w_level4_bus_mux_8bit_inst_2_out2),
    .address3(w_level4_bus_mux_8bit_inst_2_out3),
    .address4(w_level4_bus_mux_8bit_inst_2_out4),
    .address5(w_level4_bus_mux_8bit_inst_2_out5),
    .address6(w_level4_bus_mux_8bit_inst_2_out6),
    .address7(w_level4_bus_mux_8bit_inst_2_out7),
    .clock(clock),
    .datain0(progdata8),
    .datain1(progdata9),
    .datain2(progdata10),
    .datain3(progdata11),
    .datain4(progdata12),
    .datain5(progdata13),
    .datain6(progdata14),
    .datain7(progdata15),
    .writeenable(progwrite),
    .instruction0(w_level7_instruction_memory_interface_inst_4_instruction0),
    .instruction1(w_level7_instruction_memory_interface_inst_4_instruction1),
    .instruction2(w_level7_instruction_memory_interface_inst_4_instruction2),
    .instruction3(w_level7_instruction_memory_interface_inst_4_instruction3),
    .instruction4(w_level7_instruction_memory_interface_inst_4_instruction4),
    .instruction5(w_level7_instruction_memory_interface_inst_4_instruction5),
    .instruction6(w_level7_instruction_memory_interface_inst_4_instruction6),
    .instruction7(w_level7_instruction_memory_interface_inst_4_instruction7)
);
level6_register_8bit level6_register_8bit_inst_5 (
    .data0(w_level7_instruction_memory_interface_inst_3_instruction0),
    .clock(clock),
    .data1(w_level7_instruction_memory_interface_inst_3_instruction1),
    .writeenable(instrload),
    .data2(w_level7_instruction_memory_interface_inst_3_instruction2),
    .reset(reset),
    .data3(w_level7_instruction_memory_interface_inst_3_instruction3),
    .data4(w_level7_instruction_memory_interface_inst_3_instruction4),
    .data5(w_level7_instruction_memory_interface_inst_3_instruction5),
    .data6(w_level7_instruction_memory_interface_inst_3_instruction6),
    .data7(w_level7_instruction_memory_interface_inst_3_instruction7),
    .q0(w_level6_register_8bit_inst_5_q0),
    .q1(w_level6_register_8bit_inst_5_q1),
    .q2(w_level6_register_8bit_inst_5_q2),
    .q3(w_level6_register_8bit_inst_5_q3),
    .q4(w_level6_register_8bit_inst_5_q4),
    .q5(w_level6_register_8bit_inst_5_q5),
    .q6(w_level6_register_8bit_inst_5_q6),
    .q7(w_level6_register_8bit_inst_5_q7)
);
level6_register_8bit level6_register_8bit_inst_6 (
    .data0(w_level7_instruction_memory_interface_inst_4_instruction0),
    .clock(clock),
    .data1(w_level7_instruction_memory_interface_inst_4_instruction1),
    .writeenable(instrload),
    .data2(w_level7_instruction_memory_interface_inst_4_instruction2),
    .reset(reset),
    .data3(w_level7_instruction_memory_interface_inst_4_instruction3),
    .data4(w_level7_instruction_memory_interface_inst_4_instruction4),
    .data5(w_level7_instruction_memory_interface_inst_4_instruction5),
    .data6(w_level7_instruction_memory_interface_inst_4_instruction6),
    .data7(w_level7_instruction_memory_interface_inst_4_instruction7),
    .q0(w_level6_register_8bit_inst_6_q0),
    .q1(w_level6_register_8bit_inst_6_q1),
    .q2(w_level6_register_8bit_inst_6_q2),
    .q3(w_level6_register_8bit_inst_6_q3),
    .q4(w_level6_register_8bit_inst_6_q4),
    .q5(w_level6_register_8bit_inst_6_q5),
    .q6(w_level6_register_8bit_inst_6_q6),
    .q7(w_level6_register_8bit_inst_6_q7)
);

assign instruction0 = w_level6_register_8bit_inst_5_q0;
assign rawinstr0 = w_level7_instruction_memory_interface_inst_3_instruction0;
assign pc0 = w_level7_cpu_program_counter_8bit_inst_1_address0;
assign opcode0 = w_level6_register_8bit_inst_6_q3;
assign destreg0 = w_level6_register_8bit_inst_5_q6;
assign srcbits0 = w_level6_register_8bit_inst_5_q0;
assign instruction1 = w_level6_register_8bit_inst_5_q1;
assign rawinstr1 = w_level7_instruction_memory_interface_inst_3_instruction1;
assign pc1 = w_level7_cpu_program_counter_8bit_inst_1_address1;
assign opcode1 = w_level6_register_8bit_inst_6_q4;
assign destreg1 = w_level6_register_8bit_inst_5_q7;
assign srcbits1 = w_level6_register_8bit_inst_5_q1;
assign instruction2 = w_level6_register_8bit_inst_5_q2;
assign rawinstr2 = w_level7_instruction_memory_interface_inst_3_instruction2;
assign pc2 = w_level7_cpu_program_counter_8bit_inst_1_address2;
assign opcode2 = w_level6_register_8bit_inst_6_q5;
assign destreg2 = w_level6_register_8bit_inst_6_q0;
assign srcbits2 = w_level6_register_8bit_inst_5_q2;
assign instruction3 = w_level6_register_8bit_inst_5_q3;
assign rawinstr3 = w_level7_instruction_memory_interface_inst_3_instruction3;
assign pc3 = w_level7_cpu_program_counter_8bit_inst_1_address3;
assign opcode3 = w_level6_register_8bit_inst_6_q6;
assign destreg3 = w_level6_register_8bit_inst_6_q1;
assign srcbits3 = w_level6_register_8bit_inst_5_q3;
assign instruction4 = w_level6_register_8bit_inst_5_q4;
assign rawinstr4 = w_level7_instruction_memory_interface_inst_3_instruction4;
assign pc4 = w_level7_cpu_program_counter_8bit_inst_1_address4;
assign opcode4 = w_level6_register_8bit_inst_6_q7;
assign destreg4 = w_level6_register_8bit_inst_6_q2;
assign srcbits4 = w_level6_register_8bit_inst_5_q4;
assign instruction5 = w_level6_register_8bit_inst_5_q5;
assign rawinstr5 = w_level7_instruction_memory_interface_inst_3_instruction5;
assign pc5 = w_level7_cpu_program_counter_8bit_inst_1_address5;
assign srcbits5 = w_level6_register_8bit_inst_5_q5;
assign instruction6 = w_level6_register_8bit_inst_5_q6;
assign rawinstr6 = w_level7_instruction_memory_interface_inst_3_instruction6;
assign pc6 = w_level7_cpu_program_counter_8bit_inst_1_address6;
assign instruction7 = w_level6_register_8bit_inst_5_q7;
assign rawinstr7 = w_level7_instruction_memory_interface_inst_3_instruction7;
assign pc7 = w_level7_cpu_program_counter_8bit_inst_1_address7;
assign instruction8 = w_level6_register_8bit_inst_6_q0;
assign rawinstr8 = w_level7_instruction_memory_interface_inst_4_instruction0;
assign instruction9 = w_level6_register_8bit_inst_6_q1;
assign rawinstr9 = w_level7_instruction_memory_interface_inst_4_instruction1;
assign instruction10 = w_level6_register_8bit_inst_6_q2;
assign rawinstr10 = w_level7_instruction_memory_interface_inst_4_instruction2;
assign instruction11 = w_level6_register_8bit_inst_6_q3;
assign rawinstr11 = w_level7_instruction_memory_interface_inst_4_instruction3;
assign instruction12 = w_level6_register_8bit_inst_6_q4;
assign rawinstr12 = w_level7_instruction_memory_interface_inst_4_instruction4;
assign instruction13 = w_level6_register_8bit_inst_6_q5;
assign rawinstr13 = w_level7_instruction_memory_interface_inst_4_instruction5;
assign instruction14 = w_level6_register_8bit_inst_6_q6;
assign rawinstr14 = w_level7_instruction_memory_interface_inst_4_instruction6;
assign instruction15 = w_level6_register_8bit_inst_6_q7;
assign rawinstr15 = w_level7_instruction_memory_interface_inst_4_instruction7;
endmodule

// Module for LEVEL9_CPU_16BIT_RISC (generated from level9_cpu_16bit_risc.panda)
module level9_cpu_16bit_risc_ic (
    input clock,
    input reset,
    input progaddr0,
    input progaddr1,
    input progaddr2,
    input progaddr3,
    input progaddr4,
    input progaddr5,
    input progaddr6,
    input progaddr7,
    input progwrite,
    input progdata0,
    input progdata1,
    input progdata2,
    input progdata3,
    input progdata4,
    input progdata5,
    input progdata6,
    input progdata7,
    input progdata8,
    input progdata9,
    input progdata10,
    input progdata11,
    input progdata12,
    input progdata13,
    input progdata14,
    input progdata15,
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
wire w_level9_fetch_stage_16bit_inst_3_instruction0;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr0;
wire w_level9_fetch_stage_16bit_inst_3_pc0;
wire w_level9_fetch_stage_16bit_inst_3_opcode0;
wire w_level9_fetch_stage_16bit_inst_3_destreg0;
wire w_level9_fetch_stage_16bit_inst_3_srcbits0;
wire w_level9_fetch_stage_16bit_inst_3_instruction1;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr1;
wire w_level9_fetch_stage_16bit_inst_3_pc1;
wire w_level9_fetch_stage_16bit_inst_3_opcode1;
wire w_level9_fetch_stage_16bit_inst_3_destreg1;
wire w_level9_fetch_stage_16bit_inst_3_srcbits1;
wire w_level9_fetch_stage_16bit_inst_3_instruction2;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr2;
wire w_level9_fetch_stage_16bit_inst_3_pc2;
wire w_level9_fetch_stage_16bit_inst_3_opcode2;
wire w_level9_fetch_stage_16bit_inst_3_destreg2;
wire w_level9_fetch_stage_16bit_inst_3_srcbits2;
wire w_level9_fetch_stage_16bit_inst_3_instruction3;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr3;
wire w_level9_fetch_stage_16bit_inst_3_pc3;
wire w_level9_fetch_stage_16bit_inst_3_opcode3;
wire w_level9_fetch_stage_16bit_inst_3_destreg3;
wire w_level9_fetch_stage_16bit_inst_3_srcbits3;
wire w_level9_fetch_stage_16bit_inst_3_instruction4;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr4;
wire w_level9_fetch_stage_16bit_inst_3_pc4;
wire w_level9_fetch_stage_16bit_inst_3_opcode4;
wire w_level9_fetch_stage_16bit_inst_3_destreg4;
wire w_level9_fetch_stage_16bit_inst_3_srcbits4;
wire w_level9_fetch_stage_16bit_inst_3_instruction5;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr5;
wire w_level9_fetch_stage_16bit_inst_3_pc5;
wire w_level9_fetch_stage_16bit_inst_3_srcbits5;
wire w_level9_fetch_stage_16bit_inst_3_instruction6;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr6;
wire w_level9_fetch_stage_16bit_inst_3_pc6;
wire w_level9_fetch_stage_16bit_inst_3_instruction7;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr7;
wire w_level9_fetch_stage_16bit_inst_3_pc7;
wire w_level9_fetch_stage_16bit_inst_3_instruction8;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr8;
wire w_level9_fetch_stage_16bit_inst_3_instruction9;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr9;
wire w_level9_fetch_stage_16bit_inst_3_instruction10;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr10;
wire w_level9_fetch_stage_16bit_inst_3_instruction11;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr11;
wire w_level9_fetch_stage_16bit_inst_3_instruction12;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr12;
wire w_level9_fetch_stage_16bit_inst_3_instruction13;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr13;
wire w_level9_fetch_stage_16bit_inst_3_instruction14;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr14;
wire w_level9_fetch_stage_16bit_inst_3_instruction15;
wire w_level9_fetch_stage_16bit_inst_3_rawinstr15;
// IC instance: ALU_16bit (level7_alu_16bit)
wire w_level7_alu_16bit_inst_4_result0;
wire w_level7_alu_16bit_inst_4_zero;
wire w_level7_alu_16bit_inst_4_result1;
wire w_level7_alu_16bit_inst_4_sign;
wire w_level7_alu_16bit_inst_4_result2;
wire w_level7_alu_16bit_inst_4_carry;
wire w_level7_alu_16bit_inst_4_result3;
wire w_level7_alu_16bit_inst_4_result4;
wire w_level7_alu_16bit_inst_4_result5;
wire w_level7_alu_16bit_inst_4_result6;
wire w_level7_alu_16bit_inst_4_result7;
wire w_level7_alu_16bit_inst_4_result8;
wire w_level7_alu_16bit_inst_4_result9;
wire w_level7_alu_16bit_inst_4_result10;
wire w_level7_alu_16bit_inst_4_result11;
wire w_level7_alu_16bit_inst_4_result12;
wire w_level7_alu_16bit_inst_4_result13;
wire w_level7_alu_16bit_inst_4_result14;
wire w_level7_alu_16bit_inst_4_result15;

// Internal logic
level9_fetch_stage_16bit level9_fetch_stage_16bit_inst_3 (
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
    .instrload(1'b1),
    .progaddr0(progaddr0),
    .progaddr1(progaddr1),
    .progaddr2(progaddr2),
    .progaddr3(progaddr3),
    .progaddr4(progaddr4),
    .progaddr5(progaddr5),
    .progaddr6(progaddr6),
    .progaddr7(progaddr7),
    .progwrite(progwrite),
    .progdata0(progdata0),
    .progdata1(progdata1),
    .progdata2(progdata2),
    .progdata3(progdata3),
    .progdata4(progdata4),
    .progdata5(progdata5),
    .progdata6(progdata6),
    .progdata7(progdata7),
    .progdata8(progdata8),
    .progdata9(progdata9),
    .progdata10(progdata10),
    .progdata11(progdata11),
    .progdata12(progdata12),
    .progdata13(progdata13),
    .progdata14(progdata14),
    .progdata15(progdata15),
    .instruction0(w_level9_fetch_stage_16bit_inst_3_instruction0),
    .rawinstr0(w_level9_fetch_stage_16bit_inst_3_rawinstr0),
    .pc0(w_level9_fetch_stage_16bit_inst_3_pc0),
    .opcode0(w_level9_fetch_stage_16bit_inst_3_opcode0),
    .destreg0(w_level9_fetch_stage_16bit_inst_3_destreg0),
    .srcbits0(w_level9_fetch_stage_16bit_inst_3_srcbits0),
    .instruction1(w_level9_fetch_stage_16bit_inst_3_instruction1),
    .rawinstr1(w_level9_fetch_stage_16bit_inst_3_rawinstr1),
    .pc1(w_level9_fetch_stage_16bit_inst_3_pc1),
    .opcode1(w_level9_fetch_stage_16bit_inst_3_opcode1),
    .destreg1(w_level9_fetch_stage_16bit_inst_3_destreg1),
    .srcbits1(w_level9_fetch_stage_16bit_inst_3_srcbits1),
    .instruction2(w_level9_fetch_stage_16bit_inst_3_instruction2),
    .rawinstr2(w_level9_fetch_stage_16bit_inst_3_rawinstr2),
    .pc2(w_level9_fetch_stage_16bit_inst_3_pc2),
    .opcode2(w_level9_fetch_stage_16bit_inst_3_opcode2),
    .destreg2(w_level9_fetch_stage_16bit_inst_3_destreg2),
    .srcbits2(w_level9_fetch_stage_16bit_inst_3_srcbits2),
    .instruction3(w_level9_fetch_stage_16bit_inst_3_instruction3),
    .rawinstr3(w_level9_fetch_stage_16bit_inst_3_rawinstr3),
    .pc3(w_level9_fetch_stage_16bit_inst_3_pc3),
    .opcode3(w_level9_fetch_stage_16bit_inst_3_opcode3),
    .destreg3(w_level9_fetch_stage_16bit_inst_3_destreg3),
    .srcbits3(w_level9_fetch_stage_16bit_inst_3_srcbits3),
    .instruction4(w_level9_fetch_stage_16bit_inst_3_instruction4),
    .rawinstr4(w_level9_fetch_stage_16bit_inst_3_rawinstr4),
    .pc4(w_level9_fetch_stage_16bit_inst_3_pc4),
    .opcode4(w_level9_fetch_stage_16bit_inst_3_opcode4),
    .destreg4(w_level9_fetch_stage_16bit_inst_3_destreg4),
    .srcbits4(w_level9_fetch_stage_16bit_inst_3_srcbits4),
    .instruction5(w_level9_fetch_stage_16bit_inst_3_instruction5),
    .rawinstr5(w_level9_fetch_stage_16bit_inst_3_rawinstr5),
    .pc5(w_level9_fetch_stage_16bit_inst_3_pc5),
    .srcbits5(w_level9_fetch_stage_16bit_inst_3_srcbits5),
    .instruction6(w_level9_fetch_stage_16bit_inst_3_instruction6),
    .rawinstr6(w_level9_fetch_stage_16bit_inst_3_rawinstr6),
    .pc6(w_level9_fetch_stage_16bit_inst_3_pc6),
    .instruction7(w_level9_fetch_stage_16bit_inst_3_instruction7),
    .rawinstr7(w_level9_fetch_stage_16bit_inst_3_rawinstr7),
    .pc7(w_level9_fetch_stage_16bit_inst_3_pc7),
    .instruction8(w_level9_fetch_stage_16bit_inst_3_instruction8),
    .rawinstr8(w_level9_fetch_stage_16bit_inst_3_rawinstr8),
    .instruction9(w_level9_fetch_stage_16bit_inst_3_instruction9),
    .rawinstr9(w_level9_fetch_stage_16bit_inst_3_rawinstr9),
    .instruction10(w_level9_fetch_stage_16bit_inst_3_instruction10),
    .rawinstr10(w_level9_fetch_stage_16bit_inst_3_rawinstr10),
    .instruction11(w_level9_fetch_stage_16bit_inst_3_instruction11),
    .rawinstr11(w_level9_fetch_stage_16bit_inst_3_rawinstr11),
    .instruction12(w_level9_fetch_stage_16bit_inst_3_instruction12),
    .rawinstr12(w_level9_fetch_stage_16bit_inst_3_rawinstr12),
    .instruction13(w_level9_fetch_stage_16bit_inst_3_instruction13),
    .rawinstr13(w_level9_fetch_stage_16bit_inst_3_rawinstr13),
    .instruction14(w_level9_fetch_stage_16bit_inst_3_instruction14),
    .rawinstr14(w_level9_fetch_stage_16bit_inst_3_rawinstr14),
    .instruction15(w_level9_fetch_stage_16bit_inst_3_instruction15),
    .rawinstr15(w_level9_fetch_stage_16bit_inst_3_rawinstr15)
);
level7_alu_16bit level7_alu_16bit_inst_4 (
    .operanda0(w_level9_fetch_stage_16bit_inst_3_rawinstr0),
    .operandb0(w_level9_fetch_stage_16bit_inst_3_rawinstr6),
    .aluop0(w_level9_fetch_stage_16bit_inst_3_rawinstr11),
    .operanda1(w_level9_fetch_stage_16bit_inst_3_rawinstr1),
    .operandb1(w_level9_fetch_stage_16bit_inst_3_rawinstr7),
    .aluop1(w_level9_fetch_stage_16bit_inst_3_rawinstr12),
    .operanda2(w_level9_fetch_stage_16bit_inst_3_rawinstr2),
    .operandb2(w_level9_fetch_stage_16bit_inst_3_rawinstr8),
    .aluop2(w_level9_fetch_stage_16bit_inst_3_rawinstr13),
    .operanda3(w_level9_fetch_stage_16bit_inst_3_rawinstr3),
    .operandb3(w_level9_fetch_stage_16bit_inst_3_rawinstr9),
    .operanda4(w_level9_fetch_stage_16bit_inst_3_rawinstr4),
    .operandb4(w_level9_fetch_stage_16bit_inst_3_rawinstr10),
    .operanda5(w_level9_fetch_stage_16bit_inst_3_rawinstr5),
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
    .result0(w_level7_alu_16bit_inst_4_result0),
    .zero(w_level7_alu_16bit_inst_4_zero),
    .result1(w_level7_alu_16bit_inst_4_result1),
    .sign(w_level7_alu_16bit_inst_4_sign),
    .result2(w_level7_alu_16bit_inst_4_result2),
    .carry(w_level7_alu_16bit_inst_4_carry),
    .result3(w_level7_alu_16bit_inst_4_result3),
    .result4(w_level7_alu_16bit_inst_4_result4),
    .result5(w_level7_alu_16bit_inst_4_result5),
    .result6(w_level7_alu_16bit_inst_4_result6),
    .result7(w_level7_alu_16bit_inst_4_result7),
    .result8(w_level7_alu_16bit_inst_4_result8),
    .result9(w_level7_alu_16bit_inst_4_result9),
    .result10(w_level7_alu_16bit_inst_4_result10),
    .result11(w_level7_alu_16bit_inst_4_result11),
    .result12(w_level7_alu_16bit_inst_4_result12),
    .result13(w_level7_alu_16bit_inst_4_result13),
    .result14(w_level7_alu_16bit_inst_4_result14),
    .result15(w_level7_alu_16bit_inst_4_result15)
);

assign pc0 = w_level9_fetch_stage_16bit_inst_3_pc0;
assign pc1 = w_level9_fetch_stage_16bit_inst_3_pc1;
assign pc2 = w_level9_fetch_stage_16bit_inst_3_pc2;
assign pc3 = w_level9_fetch_stage_16bit_inst_3_pc3;
assign pc4 = w_level9_fetch_stage_16bit_inst_3_pc4;
assign pc5 = w_level9_fetch_stage_16bit_inst_3_pc5;
assign pc6 = w_level9_fetch_stage_16bit_inst_3_pc6;
assign pc7 = w_level9_fetch_stage_16bit_inst_3_pc7;
assign instr0 = w_level9_fetch_stage_16bit_inst_3_instruction0;
assign opcode0 = w_level9_fetch_stage_16bit_inst_3_opcode0;
assign instr1 = w_level9_fetch_stage_16bit_inst_3_instruction1;
assign opcode1 = w_level9_fetch_stage_16bit_inst_3_opcode1;
assign instr2 = w_level9_fetch_stage_16bit_inst_3_instruction2;
assign opcode2 = w_level9_fetch_stage_16bit_inst_3_opcode2;
assign result0 = w_level7_alu_16bit_inst_4_result0;
assign instr3 = w_level9_fetch_stage_16bit_inst_3_instruction3;
assign opcode3 = w_level9_fetch_stage_16bit_inst_3_opcode3;
assign result1 = w_level7_alu_16bit_inst_4_result1;
assign instr4 = w_level9_fetch_stage_16bit_inst_3_instruction4;
assign opcode4 = w_level9_fetch_stage_16bit_inst_3_opcode4;
assign result2 = w_level7_alu_16bit_inst_4_result2;
assign instr5 = w_level9_fetch_stage_16bit_inst_3_instruction5;
assign result3 = w_level7_alu_16bit_inst_4_result3;
assign instr6 = w_level9_fetch_stage_16bit_inst_3_instruction6;
assign result4 = w_level7_alu_16bit_inst_4_result4;
assign instr7 = w_level9_fetch_stage_16bit_inst_3_instruction7;
assign result5 = w_level7_alu_16bit_inst_4_result5;
assign instr8 = w_level9_fetch_stage_16bit_inst_3_instruction8;
assign result6 = w_level7_alu_16bit_inst_4_result6;
assign instr9 = w_level9_fetch_stage_16bit_inst_3_instruction9;
assign result7 = w_level7_alu_16bit_inst_4_result7;
assign instr10 = w_level9_fetch_stage_16bit_inst_3_instruction10;
assign result8 = w_level7_alu_16bit_inst_4_result8;
assign instr11 = w_level9_fetch_stage_16bit_inst_3_instruction11;
assign result9 = w_level7_alu_16bit_inst_4_result9;
assign instr12 = w_level9_fetch_stage_16bit_inst_3_instruction12;
assign result10 = w_level7_alu_16bit_inst_4_result10;
assign instr13 = w_level9_fetch_stage_16bit_inst_3_instruction13;
assign result11 = w_level7_alu_16bit_inst_4_result11;
assign instr14 = w_level9_fetch_stage_16bit_inst_3_instruction14;
assign result12 = w_level7_alu_16bit_inst_4_result12;
assign instr15 = w_level9_fetch_stage_16bit_inst_3_instruction15;
assign result13 = w_level7_alu_16bit_inst_4_result13;
assign result14 = w_level7_alu_16bit_inst_4_result14;
assign result15 = w_level7_alu_16bit_inst_4_result15;
endmodule

module level9_cpu_16bit_risc (
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
input input_switch20,
input input_switch21,
input input_switch22,
input input_switch23,
input input_switch24,
input input_switch25,
input input_switch26,
input input_switch27,

/* ========= Outputs ========== */
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
output led48_1,
output led49_1,
output led50_1,
output led51_1,
output led52_1,
output led53_1,
output led54_1,
output led55_1,
output led56_1,
output led57_1,
output led58_1,
output led59_1,
output led60_1,
output led61_1,
output led62_1,
output led63_1,
output led64_1,
output led65_1,
output led66_1,
output led67_1,
output led68_1,
output led69_1,
output led70_1,
output led71_1,
output led72_1,
output led73_1
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
    .progaddr0(input_switch3),
    .progaddr1(input_switch4),
    .progaddr2(input_switch5),
    .progaddr3(input_switch6),
    .progaddr4(input_switch7),
    .progaddr5(input_switch8),
    .progaddr6(input_switch9),
    .progaddr7(input_switch10),
    .progwrite(input_switch11),
    .progdata0(input_switch12),
    .progdata1(input_switch13),
    .progdata2(input_switch14),
    .progdata3(input_switch15),
    .progdata4(input_switch16),
    .progdata5(input_switch17),
    .progdata6(input_switch18),
    .progdata7(input_switch19),
    .progdata8(input_switch20),
    .progdata9(input_switch21),
    .progdata10(input_switch22),
    .progdata11(input_switch23),
    .progdata12(input_switch24),
    .progdata13(input_switch25),
    .progdata14(input_switch26),
    .progdata15(input_switch27),
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
assign led29_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc0;
assign led30_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc1;
assign led31_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc2;
assign led32_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc3;
assign led33_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc4;
assign led34_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc5;
assign led35_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc6;
assign led36_1 = w_level9_cpu_16bit_risc_ic_inst_1_pc7;
assign led37_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr0;
assign led38_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode0;
assign led39_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr1;
assign led40_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode1;
assign led41_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr2;
assign led42_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode2;
assign led43_1 = w_level9_cpu_16bit_risc_ic_inst_1_result0;
assign led44_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr3;
assign led45_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode3;
assign led46_1 = w_level9_cpu_16bit_risc_ic_inst_1_result1;
assign led47_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr4;
assign led48_1 = w_level9_cpu_16bit_risc_ic_inst_1_opcode4;
assign led49_1 = w_level9_cpu_16bit_risc_ic_inst_1_result2;
assign led50_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr5;
assign led51_1 = w_level9_cpu_16bit_risc_ic_inst_1_result3;
assign led52_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr6;
assign led53_1 = w_level9_cpu_16bit_risc_ic_inst_1_result4;
assign led54_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr7;
assign led55_1 = w_level9_cpu_16bit_risc_ic_inst_1_result5;
assign led56_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr8;
assign led57_1 = w_level9_cpu_16bit_risc_ic_inst_1_result6;
assign led58_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr9;
assign led59_1 = w_level9_cpu_16bit_risc_ic_inst_1_result7;
assign led60_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr10;
assign led61_1 = w_level9_cpu_16bit_risc_ic_inst_1_result8;
assign led62_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr11;
assign led63_1 = w_level9_cpu_16bit_risc_ic_inst_1_result9;
assign led64_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr12;
assign led65_1 = w_level9_cpu_16bit_risc_ic_inst_1_result10;
assign led66_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr13;
assign led67_1 = w_level9_cpu_16bit_risc_ic_inst_1_result11;
assign led68_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr14;
assign led69_1 = w_level9_cpu_16bit_risc_ic_inst_1_result12;
assign led70_1 = w_level9_cpu_16bit_risc_ic_inst_1_instr15;
assign led71_1 = w_level9_cpu_16bit_risc_ic_inst_1_result13;
assign led72_1 = w_level9_cpu_16bit_risc_ic_inst_1_result14;
assign led73_1 = w_level9_cpu_16bit_risc_ic_inst_1_result15;
endmodule
