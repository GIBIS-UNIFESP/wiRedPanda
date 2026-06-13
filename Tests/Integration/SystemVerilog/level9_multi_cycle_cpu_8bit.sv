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

// Module for Decoder4to16 (generated from level2_decoder_4to16.panda)
module level2_decoder_4to16 (
    input addr0,
    input addr1,
    input addr2,
    input addr3,
    input enable,
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
assign aux_and_5 = (aux_not_1 & aux_not_2 & aux_not_3 & aux_not_4 & enable);
assign aux_and_6 = (addr0 & aux_not_2 & aux_not_3 & aux_not_4 & enable);
assign aux_and_7 = (aux_not_1 & addr1 & aux_not_3 & aux_not_4 & enable);
assign aux_and_8 = (addr0 & addr1 & aux_not_3 & aux_not_4 & enable);
assign aux_and_9 = (aux_not_1 & aux_not_2 & addr2 & aux_not_4 & enable);
assign aux_and_10 = (addr0 & aux_not_2 & addr2 & aux_not_4 & enable);
assign aux_and_11 = (aux_not_1 & addr1 & addr2 & aux_not_4 & enable);
assign aux_and_12 = (addr0 & addr1 & addr2 & aux_not_4 & enable);
assign aux_and_13 = (aux_not_1 & aux_not_2 & aux_not_3 & addr3 & enable);
assign aux_and_14 = (addr0 & aux_not_2 & aux_not_3 & addr3 & enable);
assign aux_and_15 = (aux_not_1 & addr1 & aux_not_3 & addr3 & enable);
assign aux_and_16 = (addr0 & addr1 & aux_not_3 & addr3 & enable);
assign aux_and_17 = (aux_not_1 & aux_not_2 & addr2 & addr3 & enable);
assign aux_and_18 = (addr0 & aux_not_2 & addr2 & addr3 & enable);
assign aux_and_19 = (aux_not_1 & addr1 & addr2 & addr3 & enable);
assign aux_and_20 = (addr0 & addr1 & addr2 & addr3 & enable);

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

// Module for RegFile (generated from level6_register_file_8x8.panda)
module level6_register_file_8x8 (
    input write_addr0,
    input write_addr1,
    input write_addr2,
    input read_addr10,
    input read_addr11,
    input read_addr12,
    input read_addr20,
    input read_addr21,
    input read_addr22,
    input data_in0,
    input data_in1,
    input data_in2,
    input data_in3,
    input data_in4,
    input data_in5,
    input data_in6,
    input data_in7,
    input write_enable,
    input clock,
    output read_data10,
    output read_data11,
    output read_data12,
    output read_data13,
    output read_data14,
    output read_data15,
    output read_data16,
    output read_data17,
    output read_data20,
    output read_data21,
    output read_data22,
    output read_data23,
    output read_data24,
    output read_data25,
    output read_data26,
    output read_data27
);

// IC instance: Write_Decoder (level2_decoder_3to8)
wire w_level2_decoder_3to8_inst_1_out0;
wire w_level2_decoder_3to8_inst_1_out1;
wire w_level2_decoder_3to8_inst_1_out2;
wire w_level2_decoder_3to8_inst_1_out3;
wire w_level2_decoder_3to8_inst_1_out4;
wire w_level2_decoder_3to8_inst_1_out5;
wire w_level2_decoder_3to8_inst_1_out6;
wire w_level2_decoder_3to8_inst_1_out7;
reg aux_d_flip_flop_2_0_q = 1'b0;
reg aux_d_flip_flop_2_1_q = 1'b1;
reg aux_d_flip_flop_3_0_q = 1'b0;
reg aux_d_flip_flop_3_1_q = 1'b1;
reg aux_d_flip_flop_4_0_q = 1'b0;
reg aux_d_flip_flop_4_1_q = 1'b1;
reg aux_d_flip_flop_5_0_q = 1'b0;
reg aux_d_flip_flop_5_1_q = 1'b1;
reg aux_d_flip_flop_6_0_q = 1'b0;
reg aux_d_flip_flop_6_1_q = 1'b1;
reg aux_d_flip_flop_7_0_q = 1'b0;
reg aux_d_flip_flop_7_1_q = 1'b1;
reg aux_d_flip_flop_8_0_q = 1'b0;
reg aux_d_flip_flop_8_1_q = 1'b1;
reg aux_d_flip_flop_9_0_q = 1'b0;
reg aux_d_flip_flop_9_1_q = 1'b1;
reg aux_mux_10 = 1'b0;
reg aux_mux_11 = 1'b0;
reg aux_mux_12 = 1'b0;
reg aux_mux_13 = 1'b0;
reg aux_mux_14 = 1'b0;
reg aux_mux_15 = 1'b0;
reg aux_mux_16 = 1'b0;
reg aux_mux_17 = 1'b0;
reg aux_d_flip_flop_18_0_q = 1'b0;
reg aux_d_flip_flop_18_1_q = 1'b1;
reg aux_d_flip_flop_19_0_q = 1'b0;
reg aux_d_flip_flop_19_1_q = 1'b1;
reg aux_d_flip_flop_20_0_q = 1'b0;
reg aux_d_flip_flop_20_1_q = 1'b1;
reg aux_d_flip_flop_21_0_q = 1'b0;
reg aux_d_flip_flop_21_1_q = 1'b1;
reg aux_d_flip_flop_22_0_q = 1'b0;
reg aux_d_flip_flop_22_1_q = 1'b1;
reg aux_d_flip_flop_23_0_q = 1'b0;
reg aux_d_flip_flop_23_1_q = 1'b1;
reg aux_d_flip_flop_24_0_q = 1'b0;
reg aux_d_flip_flop_24_1_q = 1'b1;
reg aux_d_flip_flop_25_0_q = 1'b0;
reg aux_d_flip_flop_25_1_q = 1'b1;
reg aux_mux_26 = 1'b0;
reg aux_mux_27 = 1'b0;
reg aux_mux_28 = 1'b0;
reg aux_mux_29 = 1'b0;
reg aux_mux_30 = 1'b0;
reg aux_mux_31 = 1'b0;
reg aux_mux_32 = 1'b0;
reg aux_mux_33 = 1'b0;
reg aux_d_flip_flop_34_0_q = 1'b0;
reg aux_d_flip_flop_34_1_q = 1'b1;
reg aux_d_flip_flop_35_0_q = 1'b0;
reg aux_d_flip_flop_35_1_q = 1'b1;
reg aux_d_flip_flop_36_0_q = 1'b0;
reg aux_d_flip_flop_36_1_q = 1'b1;
reg aux_d_flip_flop_37_0_q = 1'b0;
reg aux_d_flip_flop_37_1_q = 1'b1;
reg aux_d_flip_flop_38_0_q = 1'b0;
reg aux_d_flip_flop_38_1_q = 1'b1;
reg aux_d_flip_flop_39_0_q = 1'b0;
reg aux_d_flip_flop_39_1_q = 1'b1;
reg aux_d_flip_flop_40_0_q = 1'b0;
reg aux_d_flip_flop_40_1_q = 1'b1;
reg aux_d_flip_flop_41_0_q = 1'b0;
reg aux_d_flip_flop_41_1_q = 1'b1;
reg aux_mux_42 = 1'b0;
reg aux_mux_43 = 1'b0;
reg aux_mux_44 = 1'b0;
reg aux_mux_45 = 1'b0;
reg aux_mux_46 = 1'b0;
reg aux_mux_47 = 1'b0;
reg aux_mux_48 = 1'b0;
reg aux_mux_49 = 1'b0;
reg aux_d_flip_flop_50_0_q = 1'b0;
reg aux_d_flip_flop_50_1_q = 1'b1;
reg aux_d_flip_flop_51_0_q = 1'b0;
reg aux_d_flip_flop_51_1_q = 1'b1;
reg aux_d_flip_flop_52_0_q = 1'b0;
reg aux_d_flip_flop_52_1_q = 1'b1;
reg aux_d_flip_flop_53_0_q = 1'b0;
reg aux_d_flip_flop_53_1_q = 1'b1;
reg aux_d_flip_flop_54_0_q = 1'b0;
reg aux_d_flip_flop_54_1_q = 1'b1;
reg aux_d_flip_flop_55_0_q = 1'b0;
reg aux_d_flip_flop_55_1_q = 1'b1;
reg aux_d_flip_flop_56_0_q = 1'b0;
reg aux_d_flip_flop_56_1_q = 1'b1;
reg aux_d_flip_flop_57_0_q = 1'b0;
reg aux_d_flip_flop_57_1_q = 1'b1;
reg aux_mux_58 = 1'b0;
reg aux_mux_59 = 1'b0;
reg aux_mux_60 = 1'b0;
reg aux_mux_61 = 1'b0;
reg aux_mux_62 = 1'b0;
reg aux_mux_63 = 1'b0;
reg aux_mux_64 = 1'b0;
reg aux_mux_65 = 1'b0;
reg aux_d_flip_flop_66_0_q = 1'b0;
reg aux_d_flip_flop_66_1_q = 1'b1;
reg aux_d_flip_flop_67_0_q = 1'b0;
reg aux_d_flip_flop_67_1_q = 1'b1;
reg aux_d_flip_flop_68_0_q = 1'b0;
reg aux_d_flip_flop_68_1_q = 1'b1;
reg aux_d_flip_flop_69_0_q = 1'b0;
reg aux_d_flip_flop_69_1_q = 1'b1;
reg aux_d_flip_flop_70_0_q = 1'b0;
reg aux_d_flip_flop_70_1_q = 1'b1;
reg aux_d_flip_flop_71_0_q = 1'b0;
reg aux_d_flip_flop_71_1_q = 1'b1;
reg aux_d_flip_flop_72_0_q = 1'b0;
reg aux_d_flip_flop_72_1_q = 1'b1;
reg aux_d_flip_flop_73_0_q = 1'b0;
reg aux_d_flip_flop_73_1_q = 1'b1;
reg aux_mux_74 = 1'b0;
reg aux_mux_75 = 1'b0;
reg aux_mux_76 = 1'b0;
reg aux_mux_77 = 1'b0;
reg aux_mux_78 = 1'b0;
reg aux_mux_79 = 1'b0;
reg aux_mux_80 = 1'b0;
reg aux_mux_81 = 1'b0;
reg aux_d_flip_flop_82_0_q = 1'b0;
reg aux_d_flip_flop_82_1_q = 1'b1;
reg aux_d_flip_flop_83_0_q = 1'b0;
reg aux_d_flip_flop_83_1_q = 1'b1;
reg aux_d_flip_flop_84_0_q = 1'b0;
reg aux_d_flip_flop_84_1_q = 1'b1;
reg aux_d_flip_flop_85_0_q = 1'b0;
reg aux_d_flip_flop_85_1_q = 1'b1;
reg aux_d_flip_flop_86_0_q = 1'b0;
reg aux_d_flip_flop_86_1_q = 1'b1;
reg aux_d_flip_flop_87_0_q = 1'b0;
reg aux_d_flip_flop_87_1_q = 1'b1;
reg aux_d_flip_flop_88_0_q = 1'b0;
reg aux_d_flip_flop_88_1_q = 1'b1;
reg aux_d_flip_flop_89_0_q = 1'b0;
reg aux_d_flip_flop_89_1_q = 1'b1;
reg aux_mux_90 = 1'b0;
reg aux_mux_91 = 1'b0;
reg aux_mux_92 = 1'b0;
reg aux_mux_93 = 1'b0;
reg aux_mux_94 = 1'b0;
reg aux_mux_95 = 1'b0;
reg aux_mux_96 = 1'b0;
reg aux_mux_97 = 1'b0;
reg aux_d_flip_flop_98_0_q = 1'b0;
reg aux_d_flip_flop_98_1_q = 1'b1;
reg aux_d_flip_flop_99_0_q = 1'b0;
reg aux_d_flip_flop_99_1_q = 1'b1;
reg aux_d_flip_flop_100_0_q = 1'b0;
reg aux_d_flip_flop_100_1_q = 1'b1;
reg aux_d_flip_flop_101_0_q = 1'b0;
reg aux_d_flip_flop_101_1_q = 1'b1;
reg aux_d_flip_flop_102_0_q = 1'b0;
reg aux_d_flip_flop_102_1_q = 1'b1;
reg aux_d_flip_flop_103_0_q = 1'b0;
reg aux_d_flip_flop_103_1_q = 1'b1;
reg aux_d_flip_flop_104_0_q = 1'b0;
reg aux_d_flip_flop_104_1_q = 1'b1;
reg aux_d_flip_flop_105_0_q = 1'b0;
reg aux_d_flip_flop_105_1_q = 1'b1;
reg aux_mux_106 = 1'b0;
reg aux_mux_107 = 1'b0;
reg aux_mux_108 = 1'b0;
reg aux_mux_109 = 1'b0;
reg aux_mux_110 = 1'b0;
reg aux_mux_111 = 1'b0;
reg aux_mux_112 = 1'b0;
reg aux_mux_113 = 1'b0;
reg aux_d_flip_flop_114_0_q = 1'b0;
reg aux_d_flip_flop_114_1_q = 1'b1;
reg aux_d_flip_flop_115_0_q = 1'b0;
reg aux_d_flip_flop_115_1_q = 1'b1;
reg aux_d_flip_flop_116_0_q = 1'b0;
reg aux_d_flip_flop_116_1_q = 1'b1;
reg aux_d_flip_flop_117_0_q = 1'b0;
reg aux_d_flip_flop_117_1_q = 1'b1;
reg aux_d_flip_flop_118_0_q = 1'b0;
reg aux_d_flip_flop_118_1_q = 1'b1;
reg aux_d_flip_flop_119_0_q = 1'b0;
reg aux_d_flip_flop_119_1_q = 1'b1;
reg aux_d_flip_flop_120_0_q = 1'b0;
reg aux_d_flip_flop_120_1_q = 1'b1;
reg aux_d_flip_flop_121_0_q = 1'b0;
reg aux_d_flip_flop_121_1_q = 1'b1;
reg aux_mux_122 = 1'b0;
reg aux_mux_123 = 1'b0;
reg aux_mux_124 = 1'b0;
reg aux_mux_125 = 1'b0;
reg aux_mux_126 = 1'b0;
reg aux_mux_127 = 1'b0;
reg aux_mux_128 = 1'b0;
reg aux_mux_129 = 1'b0;
reg aux_mux_130 = 1'b0;
reg aux_mux_131 = 1'b0;
reg aux_mux_132 = 1'b0;
reg aux_mux_133 = 1'b0;
reg aux_mux_134 = 1'b0;
reg aux_mux_135 = 1'b0;
reg aux_mux_136 = 1'b0;
reg aux_mux_137 = 1'b0;
reg aux_mux_138 = 1'b0;
reg aux_mux_139 = 1'b0;
reg aux_mux_140 = 1'b0;
reg aux_mux_141 = 1'b0;
reg aux_mux_142 = 1'b0;
reg aux_mux_143 = 1'b0;
reg aux_mux_144 = 1'b0;
reg aux_mux_145 = 1'b0;

// Internal logic
level2_decoder_3to8 level2_decoder_3to8_inst_1 (
    .addr0(write_addr0),
    .addr1(write_addr1),
    .addr2(write_addr2),
    .enable(write_enable),
    .out0(w_level2_decoder_3to8_inst_1_out0),
    .out1(w_level2_decoder_3to8_inst_1_out1),
    .out2(w_level2_decoder_3to8_inst_1_out2),
    .out3(w_level2_decoder_3to8_inst_1_out3),
    .out4(w_level2_decoder_3to8_inst_1_out4),
    .out5(w_level2_decoder_3to8_inst_1_out5),
    .out6(w_level2_decoder_3to8_inst_1_out6),
    .out7(w_level2_decoder_3to8_inst_1_out7)
);
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_2_0_q <= aux_mux_10;
            aux_d_flip_flop_2_1_q <= ~aux_mux_10;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_3_0_q <= aux_mux_11;
            aux_d_flip_flop_3_1_q <= ~aux_mux_11;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_4_0_q <= aux_mux_12;
            aux_d_flip_flop_4_1_q <= ~aux_mux_12;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_5_0_q <= aux_mux_13;
            aux_d_flip_flop_5_1_q <= ~aux_mux_13;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_6_0_q <= aux_mux_14;
            aux_d_flip_flop_6_1_q <= ~aux_mux_14;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_7_0_q <= aux_mux_15;
            aux_d_flip_flop_7_1_q <= ~aux_mux_15;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_8_0_q <= aux_mux_16;
            aux_d_flip_flop_8_1_q <= ~aux_mux_16;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_9_0_q <= aux_mux_17;
            aux_d_flip_flop_9_1_q <= ~aux_mux_17;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out0})
            1'd0: aux_mux_10 = aux_d_flip_flop_2_0_q;
            1'd1: aux_mux_10 = data_in0;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out0})
            1'd0: aux_mux_11 = aux_d_flip_flop_3_0_q;
            1'd1: aux_mux_11 = data_in1;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out0})
            1'd0: aux_mux_12 = aux_d_flip_flop_4_0_q;
            1'd1: aux_mux_12 = data_in2;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out0})
            1'd0: aux_mux_13 = aux_d_flip_flop_5_0_q;
            1'd1: aux_mux_13 = data_in3;
            default: aux_mux_13 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out0})
            1'd0: aux_mux_14 = aux_d_flip_flop_6_0_q;
            1'd1: aux_mux_14 = data_in4;
            default: aux_mux_14 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out0})
            1'd0: aux_mux_15 = aux_d_flip_flop_7_0_q;
            1'd1: aux_mux_15 = data_in5;
            default: aux_mux_15 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out0})
            1'd0: aux_mux_16 = aux_d_flip_flop_8_0_q;
            1'd1: aux_mux_16 = data_in6;
            default: aux_mux_16 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out0})
            1'd0: aux_mux_17 = aux_d_flip_flop_9_0_q;
            1'd1: aux_mux_17 = data_in7;
            default: aux_mux_17 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_18_0_q <= aux_mux_26;
            aux_d_flip_flop_18_1_q <= ~aux_mux_26;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_19_0_q <= aux_mux_27;
            aux_d_flip_flop_19_1_q <= ~aux_mux_27;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_20_0_q <= aux_mux_28;
            aux_d_flip_flop_20_1_q <= ~aux_mux_28;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_21_0_q <= aux_mux_29;
            aux_d_flip_flop_21_1_q <= ~aux_mux_29;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_22_0_q <= aux_mux_30;
            aux_d_flip_flop_22_1_q <= ~aux_mux_30;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_23_0_q <= aux_mux_31;
            aux_d_flip_flop_23_1_q <= ~aux_mux_31;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_24_0_q <= aux_mux_32;
            aux_d_flip_flop_24_1_q <= ~aux_mux_32;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_25_0_q <= aux_mux_33;
            aux_d_flip_flop_25_1_q <= ~aux_mux_33;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out1})
            1'd0: aux_mux_26 = aux_d_flip_flop_18_0_q;
            1'd1: aux_mux_26 = data_in0;
            default: aux_mux_26 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out1})
            1'd0: aux_mux_27 = aux_d_flip_flop_19_0_q;
            1'd1: aux_mux_27 = data_in1;
            default: aux_mux_27 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out1})
            1'd0: aux_mux_28 = aux_d_flip_flop_20_0_q;
            1'd1: aux_mux_28 = data_in2;
            default: aux_mux_28 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out1})
            1'd0: aux_mux_29 = aux_d_flip_flop_21_0_q;
            1'd1: aux_mux_29 = data_in3;
            default: aux_mux_29 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out1})
            1'd0: aux_mux_30 = aux_d_flip_flop_22_0_q;
            1'd1: aux_mux_30 = data_in4;
            default: aux_mux_30 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out1})
            1'd0: aux_mux_31 = aux_d_flip_flop_23_0_q;
            1'd1: aux_mux_31 = data_in5;
            default: aux_mux_31 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out1})
            1'd0: aux_mux_32 = aux_d_flip_flop_24_0_q;
            1'd1: aux_mux_32 = data_in6;
            default: aux_mux_32 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out1})
            1'd0: aux_mux_33 = aux_d_flip_flop_25_0_q;
            1'd1: aux_mux_33 = data_in7;
            default: aux_mux_33 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_34_0_q <= aux_mux_42;
            aux_d_flip_flop_34_1_q <= ~aux_mux_42;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_35_0_q <= aux_mux_43;
            aux_d_flip_flop_35_1_q <= ~aux_mux_43;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_36_0_q <= aux_mux_44;
            aux_d_flip_flop_36_1_q <= ~aux_mux_44;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_37_0_q <= aux_mux_45;
            aux_d_flip_flop_37_1_q <= ~aux_mux_45;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_38_0_q <= aux_mux_46;
            aux_d_flip_flop_38_1_q <= ~aux_mux_46;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_39_0_q <= aux_mux_47;
            aux_d_flip_flop_39_1_q <= ~aux_mux_47;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_40_0_q <= aux_mux_48;
            aux_d_flip_flop_40_1_q <= ~aux_mux_48;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_41_0_q <= aux_mux_49;
            aux_d_flip_flop_41_1_q <= ~aux_mux_49;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out2})
            1'd0: aux_mux_42 = aux_d_flip_flop_34_0_q;
            1'd1: aux_mux_42 = data_in0;
            default: aux_mux_42 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out2})
            1'd0: aux_mux_43 = aux_d_flip_flop_35_0_q;
            1'd1: aux_mux_43 = data_in1;
            default: aux_mux_43 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out2})
            1'd0: aux_mux_44 = aux_d_flip_flop_36_0_q;
            1'd1: aux_mux_44 = data_in2;
            default: aux_mux_44 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out2})
            1'd0: aux_mux_45 = aux_d_flip_flop_37_0_q;
            1'd1: aux_mux_45 = data_in3;
            default: aux_mux_45 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out2})
            1'd0: aux_mux_46 = aux_d_flip_flop_38_0_q;
            1'd1: aux_mux_46 = data_in4;
            default: aux_mux_46 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out2})
            1'd0: aux_mux_47 = aux_d_flip_flop_39_0_q;
            1'd1: aux_mux_47 = data_in5;
            default: aux_mux_47 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out2})
            1'd0: aux_mux_48 = aux_d_flip_flop_40_0_q;
            1'd1: aux_mux_48 = data_in6;
            default: aux_mux_48 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out2})
            1'd0: aux_mux_49 = aux_d_flip_flop_41_0_q;
            1'd1: aux_mux_49 = data_in7;
            default: aux_mux_49 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_50_0_q <= aux_mux_58;
            aux_d_flip_flop_50_1_q <= ~aux_mux_58;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_51_0_q <= aux_mux_59;
            aux_d_flip_flop_51_1_q <= ~aux_mux_59;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_52_0_q <= aux_mux_60;
            aux_d_flip_flop_52_1_q <= ~aux_mux_60;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_53_0_q <= aux_mux_61;
            aux_d_flip_flop_53_1_q <= ~aux_mux_61;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_54_0_q <= aux_mux_62;
            aux_d_flip_flop_54_1_q <= ~aux_mux_62;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_55_0_q <= aux_mux_63;
            aux_d_flip_flop_55_1_q <= ~aux_mux_63;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_56_0_q <= aux_mux_64;
            aux_d_flip_flop_56_1_q <= ~aux_mux_64;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_57_0_q <= aux_mux_65;
            aux_d_flip_flop_57_1_q <= ~aux_mux_65;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out3})
            1'd0: aux_mux_58 = aux_d_flip_flop_50_0_q;
            1'd1: aux_mux_58 = data_in0;
            default: aux_mux_58 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out3})
            1'd0: aux_mux_59 = aux_d_flip_flop_51_0_q;
            1'd1: aux_mux_59 = data_in1;
            default: aux_mux_59 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out3})
            1'd0: aux_mux_60 = aux_d_flip_flop_52_0_q;
            1'd1: aux_mux_60 = data_in2;
            default: aux_mux_60 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out3})
            1'd0: aux_mux_61 = aux_d_flip_flop_53_0_q;
            1'd1: aux_mux_61 = data_in3;
            default: aux_mux_61 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out3})
            1'd0: aux_mux_62 = aux_d_flip_flop_54_0_q;
            1'd1: aux_mux_62 = data_in4;
            default: aux_mux_62 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out3})
            1'd0: aux_mux_63 = aux_d_flip_flop_55_0_q;
            1'd1: aux_mux_63 = data_in5;
            default: aux_mux_63 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out3})
            1'd0: aux_mux_64 = aux_d_flip_flop_56_0_q;
            1'd1: aux_mux_64 = data_in6;
            default: aux_mux_64 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out3})
            1'd0: aux_mux_65 = aux_d_flip_flop_57_0_q;
            1'd1: aux_mux_65 = data_in7;
            default: aux_mux_65 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_66_0_q <= aux_mux_74;
            aux_d_flip_flop_66_1_q <= ~aux_mux_74;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_67_0_q <= aux_mux_75;
            aux_d_flip_flop_67_1_q <= ~aux_mux_75;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_68_0_q <= aux_mux_76;
            aux_d_flip_flop_68_1_q <= ~aux_mux_76;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_69_0_q <= aux_mux_77;
            aux_d_flip_flop_69_1_q <= ~aux_mux_77;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_70_0_q <= aux_mux_78;
            aux_d_flip_flop_70_1_q <= ~aux_mux_78;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_71_0_q <= aux_mux_79;
            aux_d_flip_flop_71_1_q <= ~aux_mux_79;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_72_0_q <= aux_mux_80;
            aux_d_flip_flop_72_1_q <= ~aux_mux_80;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_73_0_q <= aux_mux_81;
            aux_d_flip_flop_73_1_q <= ~aux_mux_81;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out4})
            1'd0: aux_mux_74 = aux_d_flip_flop_66_0_q;
            1'd1: aux_mux_74 = data_in0;
            default: aux_mux_74 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out4})
            1'd0: aux_mux_75 = aux_d_flip_flop_67_0_q;
            1'd1: aux_mux_75 = data_in1;
            default: aux_mux_75 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out4})
            1'd0: aux_mux_76 = aux_d_flip_flop_68_0_q;
            1'd1: aux_mux_76 = data_in2;
            default: aux_mux_76 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out4})
            1'd0: aux_mux_77 = aux_d_flip_flop_69_0_q;
            1'd1: aux_mux_77 = data_in3;
            default: aux_mux_77 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out4})
            1'd0: aux_mux_78 = aux_d_flip_flop_70_0_q;
            1'd1: aux_mux_78 = data_in4;
            default: aux_mux_78 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out4})
            1'd0: aux_mux_79 = aux_d_flip_flop_71_0_q;
            1'd1: aux_mux_79 = data_in5;
            default: aux_mux_79 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out4})
            1'd0: aux_mux_80 = aux_d_flip_flop_72_0_q;
            1'd1: aux_mux_80 = data_in6;
            default: aux_mux_80 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out4})
            1'd0: aux_mux_81 = aux_d_flip_flop_73_0_q;
            1'd1: aux_mux_81 = data_in7;
            default: aux_mux_81 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_82_0_q <= aux_mux_90;
            aux_d_flip_flop_82_1_q <= ~aux_mux_90;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_83_0_q <= aux_mux_91;
            aux_d_flip_flop_83_1_q <= ~aux_mux_91;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_84_0_q <= aux_mux_92;
            aux_d_flip_flop_84_1_q <= ~aux_mux_92;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_85_0_q <= aux_mux_93;
            aux_d_flip_flop_85_1_q <= ~aux_mux_93;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_86_0_q <= aux_mux_94;
            aux_d_flip_flop_86_1_q <= ~aux_mux_94;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_87_0_q <= aux_mux_95;
            aux_d_flip_flop_87_1_q <= ~aux_mux_95;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_88_0_q <= aux_mux_96;
            aux_d_flip_flop_88_1_q <= ~aux_mux_96;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_89_0_q <= aux_mux_97;
            aux_d_flip_flop_89_1_q <= ~aux_mux_97;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out5})
            1'd0: aux_mux_90 = aux_d_flip_flop_82_0_q;
            1'd1: aux_mux_90 = data_in0;
            default: aux_mux_90 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out5})
            1'd0: aux_mux_91 = aux_d_flip_flop_83_0_q;
            1'd1: aux_mux_91 = data_in1;
            default: aux_mux_91 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out5})
            1'd0: aux_mux_92 = aux_d_flip_flop_84_0_q;
            1'd1: aux_mux_92 = data_in2;
            default: aux_mux_92 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out5})
            1'd0: aux_mux_93 = aux_d_flip_flop_85_0_q;
            1'd1: aux_mux_93 = data_in3;
            default: aux_mux_93 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out5})
            1'd0: aux_mux_94 = aux_d_flip_flop_86_0_q;
            1'd1: aux_mux_94 = data_in4;
            default: aux_mux_94 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out5})
            1'd0: aux_mux_95 = aux_d_flip_flop_87_0_q;
            1'd1: aux_mux_95 = data_in5;
            default: aux_mux_95 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out5})
            1'd0: aux_mux_96 = aux_d_flip_flop_88_0_q;
            1'd1: aux_mux_96 = data_in6;
            default: aux_mux_96 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out5})
            1'd0: aux_mux_97 = aux_d_flip_flop_89_0_q;
            1'd1: aux_mux_97 = data_in7;
            default: aux_mux_97 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_98_0_q <= aux_mux_106;
            aux_d_flip_flop_98_1_q <= ~aux_mux_106;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_99_0_q <= aux_mux_107;
            aux_d_flip_flop_99_1_q <= ~aux_mux_107;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_100_0_q <= aux_mux_108;
            aux_d_flip_flop_100_1_q <= ~aux_mux_108;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_101_0_q <= aux_mux_109;
            aux_d_flip_flop_101_1_q <= ~aux_mux_109;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_102_0_q <= aux_mux_110;
            aux_d_flip_flop_102_1_q <= ~aux_mux_110;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_103_0_q <= aux_mux_111;
            aux_d_flip_flop_103_1_q <= ~aux_mux_111;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_104_0_q <= aux_mux_112;
            aux_d_flip_flop_104_1_q <= ~aux_mux_112;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_105_0_q <= aux_mux_113;
            aux_d_flip_flop_105_1_q <= ~aux_mux_113;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out6})
            1'd0: aux_mux_106 = aux_d_flip_flop_98_0_q;
            1'd1: aux_mux_106 = data_in0;
            default: aux_mux_106 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out6})
            1'd0: aux_mux_107 = aux_d_flip_flop_99_0_q;
            1'd1: aux_mux_107 = data_in1;
            default: aux_mux_107 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out6})
            1'd0: aux_mux_108 = aux_d_flip_flop_100_0_q;
            1'd1: aux_mux_108 = data_in2;
            default: aux_mux_108 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out6})
            1'd0: aux_mux_109 = aux_d_flip_flop_101_0_q;
            1'd1: aux_mux_109 = data_in3;
            default: aux_mux_109 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out6})
            1'd0: aux_mux_110 = aux_d_flip_flop_102_0_q;
            1'd1: aux_mux_110 = data_in4;
            default: aux_mux_110 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out6})
            1'd0: aux_mux_111 = aux_d_flip_flop_103_0_q;
            1'd1: aux_mux_111 = data_in5;
            default: aux_mux_111 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out6})
            1'd0: aux_mux_112 = aux_d_flip_flop_104_0_q;
            1'd1: aux_mux_112 = data_in6;
            default: aux_mux_112 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out6})
            1'd0: aux_mux_113 = aux_d_flip_flop_105_0_q;
            1'd1: aux_mux_113 = data_in7;
            default: aux_mux_113 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_114_0_q <= aux_mux_122;
            aux_d_flip_flop_114_1_q <= ~aux_mux_122;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_115_0_q <= aux_mux_123;
            aux_d_flip_flop_115_1_q <= ~aux_mux_123;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_116_0_q <= aux_mux_124;
            aux_d_flip_flop_116_1_q <= ~aux_mux_124;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_117_0_q <= aux_mux_125;
            aux_d_flip_flop_117_1_q <= ~aux_mux_125;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_118_0_q <= aux_mux_126;
            aux_d_flip_flop_118_1_q <= ~aux_mux_126;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_119_0_q <= aux_mux_127;
            aux_d_flip_flop_119_1_q <= ~aux_mux_127;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_120_0_q <= aux_mux_128;
            aux_d_flip_flop_120_1_q <= ~aux_mux_128;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_121_0_q <= aux_mux_129;
            aux_d_flip_flop_121_1_q <= ~aux_mux_129;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out7})
            1'd0: aux_mux_122 = aux_d_flip_flop_114_0_q;
            1'd1: aux_mux_122 = data_in0;
            default: aux_mux_122 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out7})
            1'd0: aux_mux_123 = aux_d_flip_flop_115_0_q;
            1'd1: aux_mux_123 = data_in1;
            default: aux_mux_123 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out7})
            1'd0: aux_mux_124 = aux_d_flip_flop_116_0_q;
            1'd1: aux_mux_124 = data_in2;
            default: aux_mux_124 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out7})
            1'd0: aux_mux_125 = aux_d_flip_flop_117_0_q;
            1'd1: aux_mux_125 = data_in3;
            default: aux_mux_125 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out7})
            1'd0: aux_mux_126 = aux_d_flip_flop_118_0_q;
            1'd1: aux_mux_126 = data_in4;
            default: aux_mux_126 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out7})
            1'd0: aux_mux_127 = aux_d_flip_flop_119_0_q;
            1'd1: aux_mux_127 = data_in5;
            default: aux_mux_127 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out7})
            1'd0: aux_mux_128 = aux_d_flip_flop_120_0_q;
            1'd1: aux_mux_128 = data_in6;
            default: aux_mux_128 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_3to8_inst_1_out7})
            1'd0: aux_mux_129 = aux_d_flip_flop_121_0_q;
            1'd1: aux_mux_129 = data_in7;
            default: aux_mux_129 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_130 = aux_d_flip_flop_2_0_q;
            3'd1: aux_mux_130 = aux_d_flip_flop_18_0_q;
            3'd2: aux_mux_130 = aux_d_flip_flop_34_0_q;
            3'd3: aux_mux_130 = aux_d_flip_flop_50_0_q;
            3'd4: aux_mux_130 = aux_d_flip_flop_66_0_q;
            3'd5: aux_mux_130 = aux_d_flip_flop_82_0_q;
            3'd6: aux_mux_130 = aux_d_flip_flop_98_0_q;
            3'd7: aux_mux_130 = aux_d_flip_flop_114_0_q;
            default: aux_mux_130 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_131 = aux_d_flip_flop_2_0_q;
            3'd1: aux_mux_131 = aux_d_flip_flop_18_0_q;
            3'd2: aux_mux_131 = aux_d_flip_flop_34_0_q;
            3'd3: aux_mux_131 = aux_d_flip_flop_50_0_q;
            3'd4: aux_mux_131 = aux_d_flip_flop_66_0_q;
            3'd5: aux_mux_131 = aux_d_flip_flop_82_0_q;
            3'd6: aux_mux_131 = aux_d_flip_flop_98_0_q;
            3'd7: aux_mux_131 = aux_d_flip_flop_114_0_q;
            default: aux_mux_131 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_132 = aux_d_flip_flop_3_0_q;
            3'd1: aux_mux_132 = aux_d_flip_flop_19_0_q;
            3'd2: aux_mux_132 = aux_d_flip_flop_35_0_q;
            3'd3: aux_mux_132 = aux_d_flip_flop_51_0_q;
            3'd4: aux_mux_132 = aux_d_flip_flop_67_0_q;
            3'd5: aux_mux_132 = aux_d_flip_flop_83_0_q;
            3'd6: aux_mux_132 = aux_d_flip_flop_99_0_q;
            3'd7: aux_mux_132 = aux_d_flip_flop_115_0_q;
            default: aux_mux_132 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_133 = aux_d_flip_flop_3_0_q;
            3'd1: aux_mux_133 = aux_d_flip_flop_19_0_q;
            3'd2: aux_mux_133 = aux_d_flip_flop_35_0_q;
            3'd3: aux_mux_133 = aux_d_flip_flop_51_0_q;
            3'd4: aux_mux_133 = aux_d_flip_flop_67_0_q;
            3'd5: aux_mux_133 = aux_d_flip_flop_83_0_q;
            3'd6: aux_mux_133 = aux_d_flip_flop_99_0_q;
            3'd7: aux_mux_133 = aux_d_flip_flop_115_0_q;
            default: aux_mux_133 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_134 = aux_d_flip_flop_4_0_q;
            3'd1: aux_mux_134 = aux_d_flip_flop_20_0_q;
            3'd2: aux_mux_134 = aux_d_flip_flop_36_0_q;
            3'd3: aux_mux_134 = aux_d_flip_flop_52_0_q;
            3'd4: aux_mux_134 = aux_d_flip_flop_68_0_q;
            3'd5: aux_mux_134 = aux_d_flip_flop_84_0_q;
            3'd6: aux_mux_134 = aux_d_flip_flop_100_0_q;
            3'd7: aux_mux_134 = aux_d_flip_flop_116_0_q;
            default: aux_mux_134 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_135 = aux_d_flip_flop_4_0_q;
            3'd1: aux_mux_135 = aux_d_flip_flop_20_0_q;
            3'd2: aux_mux_135 = aux_d_flip_flop_36_0_q;
            3'd3: aux_mux_135 = aux_d_flip_flop_52_0_q;
            3'd4: aux_mux_135 = aux_d_flip_flop_68_0_q;
            3'd5: aux_mux_135 = aux_d_flip_flop_84_0_q;
            3'd6: aux_mux_135 = aux_d_flip_flop_100_0_q;
            3'd7: aux_mux_135 = aux_d_flip_flop_116_0_q;
            default: aux_mux_135 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_136 = aux_d_flip_flop_5_0_q;
            3'd1: aux_mux_136 = aux_d_flip_flop_21_0_q;
            3'd2: aux_mux_136 = aux_d_flip_flop_37_0_q;
            3'd3: aux_mux_136 = aux_d_flip_flop_53_0_q;
            3'd4: aux_mux_136 = aux_d_flip_flop_69_0_q;
            3'd5: aux_mux_136 = aux_d_flip_flop_85_0_q;
            3'd6: aux_mux_136 = aux_d_flip_flop_101_0_q;
            3'd7: aux_mux_136 = aux_d_flip_flop_117_0_q;
            default: aux_mux_136 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_137 = aux_d_flip_flop_5_0_q;
            3'd1: aux_mux_137 = aux_d_flip_flop_21_0_q;
            3'd2: aux_mux_137 = aux_d_flip_flop_37_0_q;
            3'd3: aux_mux_137 = aux_d_flip_flop_53_0_q;
            3'd4: aux_mux_137 = aux_d_flip_flop_69_0_q;
            3'd5: aux_mux_137 = aux_d_flip_flop_85_0_q;
            3'd6: aux_mux_137 = aux_d_flip_flop_101_0_q;
            3'd7: aux_mux_137 = aux_d_flip_flop_117_0_q;
            default: aux_mux_137 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_138 = aux_d_flip_flop_6_0_q;
            3'd1: aux_mux_138 = aux_d_flip_flop_22_0_q;
            3'd2: aux_mux_138 = aux_d_flip_flop_38_0_q;
            3'd3: aux_mux_138 = aux_d_flip_flop_54_0_q;
            3'd4: aux_mux_138 = aux_d_flip_flop_70_0_q;
            3'd5: aux_mux_138 = aux_d_flip_flop_86_0_q;
            3'd6: aux_mux_138 = aux_d_flip_flop_102_0_q;
            3'd7: aux_mux_138 = aux_d_flip_flop_118_0_q;
            default: aux_mux_138 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_139 = aux_d_flip_flop_6_0_q;
            3'd1: aux_mux_139 = aux_d_flip_flop_22_0_q;
            3'd2: aux_mux_139 = aux_d_flip_flop_38_0_q;
            3'd3: aux_mux_139 = aux_d_flip_flop_54_0_q;
            3'd4: aux_mux_139 = aux_d_flip_flop_70_0_q;
            3'd5: aux_mux_139 = aux_d_flip_flop_86_0_q;
            3'd6: aux_mux_139 = aux_d_flip_flop_102_0_q;
            3'd7: aux_mux_139 = aux_d_flip_flop_118_0_q;
            default: aux_mux_139 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_140 = aux_d_flip_flop_7_0_q;
            3'd1: aux_mux_140 = aux_d_flip_flop_23_0_q;
            3'd2: aux_mux_140 = aux_d_flip_flop_39_0_q;
            3'd3: aux_mux_140 = aux_d_flip_flop_55_0_q;
            3'd4: aux_mux_140 = aux_d_flip_flop_71_0_q;
            3'd5: aux_mux_140 = aux_d_flip_flop_87_0_q;
            3'd6: aux_mux_140 = aux_d_flip_flop_103_0_q;
            3'd7: aux_mux_140 = aux_d_flip_flop_119_0_q;
            default: aux_mux_140 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_141 = aux_d_flip_flop_7_0_q;
            3'd1: aux_mux_141 = aux_d_flip_flop_23_0_q;
            3'd2: aux_mux_141 = aux_d_flip_flop_39_0_q;
            3'd3: aux_mux_141 = aux_d_flip_flop_55_0_q;
            3'd4: aux_mux_141 = aux_d_flip_flop_71_0_q;
            3'd5: aux_mux_141 = aux_d_flip_flop_87_0_q;
            3'd6: aux_mux_141 = aux_d_flip_flop_103_0_q;
            3'd7: aux_mux_141 = aux_d_flip_flop_119_0_q;
            default: aux_mux_141 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_142 = aux_d_flip_flop_8_0_q;
            3'd1: aux_mux_142 = aux_d_flip_flop_24_0_q;
            3'd2: aux_mux_142 = aux_d_flip_flop_40_0_q;
            3'd3: aux_mux_142 = aux_d_flip_flop_56_0_q;
            3'd4: aux_mux_142 = aux_d_flip_flop_72_0_q;
            3'd5: aux_mux_142 = aux_d_flip_flop_88_0_q;
            3'd6: aux_mux_142 = aux_d_flip_flop_104_0_q;
            3'd7: aux_mux_142 = aux_d_flip_flop_120_0_q;
            default: aux_mux_142 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_143 = aux_d_flip_flop_8_0_q;
            3'd1: aux_mux_143 = aux_d_flip_flop_24_0_q;
            3'd2: aux_mux_143 = aux_d_flip_flop_40_0_q;
            3'd3: aux_mux_143 = aux_d_flip_flop_56_0_q;
            3'd4: aux_mux_143 = aux_d_flip_flop_72_0_q;
            3'd5: aux_mux_143 = aux_d_flip_flop_88_0_q;
            3'd6: aux_mux_143 = aux_d_flip_flop_104_0_q;
            3'd7: aux_mux_143 = aux_d_flip_flop_120_0_q;
            default: aux_mux_143 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_144 = aux_d_flip_flop_9_0_q;
            3'd1: aux_mux_144 = aux_d_flip_flop_25_0_q;
            3'd2: aux_mux_144 = aux_d_flip_flop_41_0_q;
            3'd3: aux_mux_144 = aux_d_flip_flop_57_0_q;
            3'd4: aux_mux_144 = aux_d_flip_flop_73_0_q;
            3'd5: aux_mux_144 = aux_d_flip_flop_89_0_q;
            3'd6: aux_mux_144 = aux_d_flip_flop_105_0_q;
            3'd7: aux_mux_144 = aux_d_flip_flop_121_0_q;
            default: aux_mux_144 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_145 = aux_d_flip_flop_9_0_q;
            3'd1: aux_mux_145 = aux_d_flip_flop_25_0_q;
            3'd2: aux_mux_145 = aux_d_flip_flop_41_0_q;
            3'd3: aux_mux_145 = aux_d_flip_flop_57_0_q;
            3'd4: aux_mux_145 = aux_d_flip_flop_73_0_q;
            3'd5: aux_mux_145 = aux_d_flip_flop_89_0_q;
            3'd6: aux_mux_145 = aux_d_flip_flop_105_0_q;
            3'd7: aux_mux_145 = aux_d_flip_flop_121_0_q;
            default: aux_mux_145 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign read_data10 = aux_mux_130;
assign read_data11 = aux_mux_132;
assign read_data12 = aux_mux_134;
assign read_data13 = aux_mux_136;
assign read_data14 = aux_mux_138;
assign read_data15 = aux_mux_140;
assign read_data16 = aux_mux_142;
assign read_data17 = aux_mux_144;
assign read_data20 = aux_mux_131;
assign read_data21 = aux_mux_133;
assign read_data22 = aux_mux_135;
assign read_data23 = aux_mux_137;
assign read_data24 = aux_mux_139;
assign read_data25 = aux_mux_141;
assign read_data26 = aux_mux_143;
assign read_data27 = aux_mux_145;
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

// Module for InstrMem (generated from level7_instruction_memory_interface.panda)
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

// Module for InstrReg (generated from level7_instruction_register_8bit.panda)
module level7_instruction_register_8bit (
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

// Module for Decode (generated from level8_decode_stage.panda)
module level8_decode_stage (
    input opcode0,
    input opcode1,
    input opcode2,
    input opcode3,
    input opcode4,
    output aluop0,
    output regwrite,
    output memread,
    output memwrite,
    output aluop1,
    output aluop2,
    output instrdecodedlines0,
    output instrdecodedlines1,
    output instrdecodedlines2,
    output instrdecodedlines3,
    output instrdecodedlines4,
    output instrdecodedlines5,
    output instrdecodedlines6,
    output instrdecodedlines7,
    output instrdecodedlines8,
    output instrdecodedlines9,
    output instrdecodedlines10,
    output instrdecodedlines11,
    output instrdecodedlines12,
    output instrdecodedlines13,
    output instrdecodedlines14,
    output instrdecodedlines15,
    output instrdecodedlines16,
    output instrdecodedlines17,
    output instrdecodedlines18,
    output instrdecodedlines19,
    output instrdecodedlines20,
    output instrdecodedlines21,
    output instrdecodedlines22,
    output instrdecodedlines23,
    output instrdecodedlines24,
    output instrdecodedlines25,
    output instrdecodedlines26,
    output instrdecodedlines27,
    output instrdecodedlines28,
    output instrdecodedlines29,
    output instrdecodedlines30,
    output instrdecodedlines31
);

wire aux_not_1;
wire aux_not_2;
wire aux_and_3;
wire aux_and_4;
// IC instance: Decoder4to16 (level2_decoder_4to16)
wire w_level2_decoder_4to16_inst_5_out0;
wire w_level2_decoder_4to16_inst_5_out1;
wire w_level2_decoder_4to16_inst_5_out2;
wire w_level2_decoder_4to16_inst_5_out3;
wire w_level2_decoder_4to16_inst_5_out4;
wire w_level2_decoder_4to16_inst_5_out5;
wire w_level2_decoder_4to16_inst_5_out6;
wire w_level2_decoder_4to16_inst_5_out7;
wire w_level2_decoder_4to16_inst_5_out8;
wire w_level2_decoder_4to16_inst_5_out9;
wire w_level2_decoder_4to16_inst_5_out10;
wire w_level2_decoder_4to16_inst_5_out11;
wire w_level2_decoder_4to16_inst_5_out12;
wire w_level2_decoder_4to16_inst_5_out13;
wire w_level2_decoder_4to16_inst_5_out14;
wire w_level2_decoder_4to16_inst_5_out15;
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
wire aux_and_21;
wire aux_and_22;
wire aux_and_23;
wire aux_and_24;
wire aux_and_25;
wire aux_and_26;
wire aux_and_27;
wire aux_and_28;
wire aux_and_29;
wire aux_and_30;
wire aux_and_31;
wire aux_and_32;
wire aux_and_33;
wire aux_and_34;
wire aux_and_35;
wire aux_and_36;
wire aux_and_37;
wire aux_and_38;

// Internal logic
assign aux_not_1 = ~opcode4;
assign aux_not_2 = ~opcode3;
assign aux_and_3 = (opcode4 & aux_not_2);
assign aux_and_4 = (opcode4 & opcode3);
level2_decoder_4to16 level2_decoder_4to16_inst_5 (
    .addr0(opcode0),
    .addr1(opcode1),
    .addr2(opcode2),
    .addr3(opcode3),
    .enable(1'b1),
    .out0(w_level2_decoder_4to16_inst_5_out0),
    .out1(w_level2_decoder_4to16_inst_5_out1),
    .out2(w_level2_decoder_4to16_inst_5_out2),
    .out3(w_level2_decoder_4to16_inst_5_out3),
    .out4(w_level2_decoder_4to16_inst_5_out4),
    .out5(w_level2_decoder_4to16_inst_5_out5),
    .out6(w_level2_decoder_4to16_inst_5_out6),
    .out7(w_level2_decoder_4to16_inst_5_out7),
    .out8(w_level2_decoder_4to16_inst_5_out8),
    .out9(w_level2_decoder_4to16_inst_5_out9),
    .out10(w_level2_decoder_4to16_inst_5_out10),
    .out11(w_level2_decoder_4to16_inst_5_out11),
    .out12(w_level2_decoder_4to16_inst_5_out12),
    .out13(w_level2_decoder_4to16_inst_5_out13),
    .out14(w_level2_decoder_4to16_inst_5_out14),
    .out15(w_level2_decoder_4to16_inst_5_out15)
);
assign aux_and_7 = (w_level2_decoder_4to16_inst_5_out0 & aux_not_1);
assign aux_and_8 = (w_level2_decoder_4to16_inst_5_out1 & aux_not_1);
assign aux_and_9 = (w_level2_decoder_4to16_inst_5_out2 & aux_not_1);
assign aux_and_10 = (w_level2_decoder_4to16_inst_5_out3 & aux_not_1);
assign aux_and_11 = (w_level2_decoder_4to16_inst_5_out4 & aux_not_1);
assign aux_and_12 = (w_level2_decoder_4to16_inst_5_out5 & aux_not_1);
assign aux_and_13 = (w_level2_decoder_4to16_inst_5_out6 & aux_not_1);
assign aux_and_14 = (w_level2_decoder_4to16_inst_5_out7 & aux_not_1);
assign aux_and_15 = (w_level2_decoder_4to16_inst_5_out8 & aux_not_1);
assign aux_and_16 = (w_level2_decoder_4to16_inst_5_out9 & aux_not_1);
assign aux_and_17 = (w_level2_decoder_4to16_inst_5_out10 & aux_not_1);
assign aux_and_18 = (w_level2_decoder_4to16_inst_5_out11 & aux_not_1);
assign aux_and_19 = (w_level2_decoder_4to16_inst_5_out12 & aux_not_1);
assign aux_and_20 = (w_level2_decoder_4to16_inst_5_out13 & aux_not_1);
assign aux_and_21 = (w_level2_decoder_4to16_inst_5_out14 & aux_not_1);
assign aux_and_22 = (w_level2_decoder_4to16_inst_5_out15 & aux_not_1);
assign aux_and_23 = (w_level2_decoder_4to16_inst_5_out0 & opcode4);
assign aux_and_24 = (w_level2_decoder_4to16_inst_5_out1 & opcode4);
assign aux_and_25 = (w_level2_decoder_4to16_inst_5_out2 & opcode4);
assign aux_and_26 = (w_level2_decoder_4to16_inst_5_out3 & opcode4);
assign aux_and_27 = (w_level2_decoder_4to16_inst_5_out4 & opcode4);
assign aux_and_28 = (w_level2_decoder_4to16_inst_5_out5 & opcode4);
assign aux_and_29 = (w_level2_decoder_4to16_inst_5_out6 & opcode4);
assign aux_and_30 = (w_level2_decoder_4to16_inst_5_out7 & opcode4);
assign aux_and_31 = (w_level2_decoder_4to16_inst_5_out8 & opcode4);
assign aux_and_32 = (w_level2_decoder_4to16_inst_5_out9 & opcode4);
assign aux_and_33 = (w_level2_decoder_4to16_inst_5_out10 & opcode4);
assign aux_and_34 = (w_level2_decoder_4to16_inst_5_out11 & opcode4);
assign aux_and_35 = (w_level2_decoder_4to16_inst_5_out12 & opcode4);
assign aux_and_36 = (w_level2_decoder_4to16_inst_5_out13 & opcode4);
assign aux_and_37 = (w_level2_decoder_4to16_inst_5_out14 & opcode4);
assign aux_and_38 = (w_level2_decoder_4to16_inst_5_out15 & opcode4);

assign aluop0 = opcode0;
assign regwrite = aux_not_1;
assign memread = aux_and_3;
assign memwrite = aux_and_4;
assign aluop1 = opcode1;
assign aluop2 = opcode2;
assign instrdecodedlines0 = aux_and_7;
assign instrdecodedlines1 = aux_and_8;
assign instrdecodedlines2 = aux_and_9;
assign instrdecodedlines3 = aux_and_10;
assign instrdecodedlines4 = aux_and_11;
assign instrdecodedlines5 = aux_and_12;
assign instrdecodedlines6 = aux_and_13;
assign instrdecodedlines7 = aux_and_14;
assign instrdecodedlines8 = aux_and_15;
assign instrdecodedlines9 = aux_and_16;
assign instrdecodedlines10 = aux_and_17;
assign instrdecodedlines11 = aux_and_18;
assign instrdecodedlines12 = aux_and_19;
assign instrdecodedlines13 = aux_and_20;
assign instrdecodedlines14 = aux_and_21;
assign instrdecodedlines15 = aux_and_22;
assign instrdecodedlines16 = aux_and_23;
assign instrdecodedlines17 = aux_and_24;
assign instrdecodedlines18 = aux_and_25;
assign instrdecodedlines19 = aux_and_26;
assign instrdecodedlines20 = aux_and_27;
assign instrdecodedlines21 = aux_and_28;
assign instrdecodedlines22 = aux_and_29;
assign instrdecodedlines23 = aux_and_30;
assign instrdecodedlines24 = aux_and_31;
assign instrdecodedlines25 = aux_and_32;
assign instrdecodedlines26 = aux_and_33;
assign instrdecodedlines27 = aux_and_34;
assign instrdecodedlines28 = aux_and_35;
assign instrdecodedlines29 = aux_and_36;
assign instrdecodedlines30 = aux_and_37;
assign instrdecodedlines31 = aux_and_38;
endmodule

// Module for Execute (generated from level8_execute_stage.panda)
module level8_execute_stage (
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

// Module for Memory (generated from level8_memory_stage.panda)
module level8_memory_stage (
    input address0,
    input address1,
    input address2,
    input address3,
    input address4,
    input address5,
    input address6,
    input address7,
    input memread,
    input datain0,
    input datain1,
    input datain2,
    input datain3,
    input datain4,
    input datain5,
    input datain6,
    input datain7,
    input memwrite,
    input result0,
    input result1,
    input result2,
    input result3,
    input result4,
    input result5,
    input result6,
    input result7,
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

// IC instance: RAM (level6_ram_8x8)
wire w_level6_ram_8x8_inst_1_dataout0;
wire w_level6_ram_8x8_inst_1_dataout1;
wire w_level6_ram_8x8_inst_1_dataout2;
wire w_level6_ram_8x8_inst_1_dataout3;
wire w_level6_ram_8x8_inst_1_dataout4;
wire w_level6_ram_8x8_inst_1_dataout5;
wire w_level6_ram_8x8_inst_1_dataout6;
wire w_level6_ram_8x8_inst_1_dataout7;
reg aux_mux_2 = 1'b0;
reg aux_mux_3 = 1'b0;
reg aux_mux_4 = 1'b0;
reg aux_mux_5 = 1'b0;
reg aux_mux_6 = 1'b0;
reg aux_mux_7 = 1'b0;
reg aux_mux_8 = 1'b0;
reg aux_mux_9 = 1'b0;

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
    .writeenable(memwrite),
    .clock(clock),
    .reset(reset),
    .dataout0(w_level6_ram_8x8_inst_1_dataout0),
    .dataout1(w_level6_ram_8x8_inst_1_dataout1),
    .dataout2(w_level6_ram_8x8_inst_1_dataout2),
    .dataout3(w_level6_ram_8x8_inst_1_dataout3),
    .dataout4(w_level6_ram_8x8_inst_1_dataout4),
    .dataout5(w_level6_ram_8x8_inst_1_dataout5),
    .dataout6(w_level6_ram_8x8_inst_1_dataout6),
    .dataout7(w_level6_ram_8x8_inst_1_dataout7)
);
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_2 = result0;
            1'd1: aux_mux_2 = w_level6_ram_8x8_inst_1_dataout0;
            default: aux_mux_2 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_3 = result1;
            1'd1: aux_mux_3 = w_level6_ram_8x8_inst_1_dataout1;
            default: aux_mux_3 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_4 = result2;
            1'd1: aux_mux_4 = w_level6_ram_8x8_inst_1_dataout2;
            default: aux_mux_4 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_5 = result3;
            1'd1: aux_mux_5 = w_level6_ram_8x8_inst_1_dataout3;
            default: aux_mux_5 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_6 = result4;
            1'd1: aux_mux_6 = w_level6_ram_8x8_inst_1_dataout4;
            default: aux_mux_6 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_7 = result5;
            1'd1: aux_mux_7 = w_level6_ram_8x8_inst_1_dataout5;
            default: aux_mux_7 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_8 = result6;
            1'd1: aux_mux_8 = w_level6_ram_8x8_inst_1_dataout6;
            default: aux_mux_8 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_9 = result7;
            1'd1: aux_mux_9 = w_level6_ram_8x8_inst_1_dataout7;
            default: aux_mux_9 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign dataout0 = aux_mux_2;
assign dataout1 = aux_mux_3;
assign dataout2 = aux_mux_4;
assign dataout3 = aux_mux_5;
assign dataout4 = aux_mux_6;
assign dataout5 = aux_mux_7;
assign dataout6 = aux_mux_8;
assign dataout7 = aux_mux_9;
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

// Module for Fetch (generated from level8_fetch_stage.panda)
module level8_fetch_stage (
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
    output pc0,
    output instruction0,
    output opcode0,
    output registeraddr0,
    output rawinstr0,
    output pc1,
    output instruction1,
    output opcode1,
    output registeraddr1,
    output rawinstr1,
    output pc2,
    output instruction2,
    output opcode2,
    output registeraddr2,
    output rawinstr2,
    output pc3,
    output instruction3,
    output opcode3,
    output rawinstr3,
    output pc4,
    output instruction4,
    output opcode4,
    output rawinstr4,
    output pc5,
    output instruction5,
    output rawinstr5,
    output pc6,
    output instruction6,
    output rawinstr6,
    output pc7,
    output instruction7,
    output rawinstr7
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
// IC instance: InstrMem (level7_instruction_memory_interface)
wire w_level7_instruction_memory_interface_inst_2_instruction0;
wire w_level7_instruction_memory_interface_inst_2_instruction1;
wire w_level7_instruction_memory_interface_inst_2_instruction2;
wire w_level7_instruction_memory_interface_inst_2_instruction3;
wire w_level7_instruction_memory_interface_inst_2_instruction4;
wire w_level7_instruction_memory_interface_inst_2_instruction5;
wire w_level7_instruction_memory_interface_inst_2_instruction6;
wire w_level7_instruction_memory_interface_inst_2_instruction7;
// IC instance: AddrMux (level4_bus_mux_8bit)
wire w_level4_bus_mux_8bit_inst_3_out0;
wire w_level4_bus_mux_8bit_inst_3_out1;
wire w_level4_bus_mux_8bit_inst_3_out2;
wire w_level4_bus_mux_8bit_inst_3_out3;
wire w_level4_bus_mux_8bit_inst_3_out4;
wire w_level4_bus_mux_8bit_inst_3_out5;
wire w_level4_bus_mux_8bit_inst_3_out6;
wire w_level4_bus_mux_8bit_inst_3_out7;
// IC instance: InstrReg (level7_instruction_register_8bit)
wire w_level7_instruction_register_8bit_inst_4_instruction0;
wire w_level7_instruction_register_8bit_inst_4_instruction1;
wire w_level7_instruction_register_8bit_inst_4_instruction2;
wire w_level7_instruction_register_8bit_inst_4_instruction3;
wire w_level7_instruction_register_8bit_inst_4_instruction4;
wire w_level7_instruction_register_8bit_inst_4_instruction5;
wire w_level7_instruction_register_8bit_inst_4_instruction6;
wire w_level7_instruction_register_8bit_inst_4_instruction7;
wire w_level7_instruction_register_8bit_inst_4_opcode0;
wire w_level7_instruction_register_8bit_inst_4_opcode1;
wire w_level7_instruction_register_8bit_inst_4_opcode2;
wire w_level7_instruction_register_8bit_inst_4_opcode3;
wire w_level7_instruction_register_8bit_inst_4_opcode4;
wire w_level7_instruction_register_8bit_inst_4_registeraddr0;
wire w_level7_instruction_register_8bit_inst_4_registeraddr1;
wire w_level7_instruction_register_8bit_inst_4_registeraddr2;

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
level7_instruction_memory_interface level7_instruction_memory_interface_inst_2 (
    .address0(w_level4_bus_mux_8bit_inst_3_out0),
    .address1(w_level4_bus_mux_8bit_inst_3_out1),
    .address2(w_level4_bus_mux_8bit_inst_3_out2),
    .address3(w_level4_bus_mux_8bit_inst_3_out3),
    .address4(w_level4_bus_mux_8bit_inst_3_out4),
    .address5(w_level4_bus_mux_8bit_inst_3_out5),
    .address6(w_level4_bus_mux_8bit_inst_3_out6),
    .address7(w_level4_bus_mux_8bit_inst_3_out7),
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
    .instruction0(w_level7_instruction_memory_interface_inst_2_instruction0),
    .instruction1(w_level7_instruction_memory_interface_inst_2_instruction1),
    .instruction2(w_level7_instruction_memory_interface_inst_2_instruction2),
    .instruction3(w_level7_instruction_memory_interface_inst_2_instruction3),
    .instruction4(w_level7_instruction_memory_interface_inst_2_instruction4),
    .instruction5(w_level7_instruction_memory_interface_inst_2_instruction5),
    .instruction6(w_level7_instruction_memory_interface_inst_2_instruction6),
    .instruction7(w_level7_instruction_memory_interface_inst_2_instruction7)
);
level4_bus_mux_8bit level4_bus_mux_8bit_inst_3 (
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
    .out0(w_level4_bus_mux_8bit_inst_3_out0),
    .out1(w_level4_bus_mux_8bit_inst_3_out1),
    .out2(w_level4_bus_mux_8bit_inst_3_out2),
    .out3(w_level4_bus_mux_8bit_inst_3_out3),
    .out4(w_level4_bus_mux_8bit_inst_3_out4),
    .out5(w_level4_bus_mux_8bit_inst_3_out5),
    .out6(w_level4_bus_mux_8bit_inst_3_out6),
    .out7(w_level4_bus_mux_8bit_inst_3_out7)
);
level7_instruction_register_8bit level7_instruction_register_8bit_inst_4 (
    .data0(w_level7_instruction_memory_interface_inst_2_instruction0),
    .data1(w_level7_instruction_memory_interface_inst_2_instruction1),
    .data2(w_level7_instruction_memory_interface_inst_2_instruction2),
    .data3(w_level7_instruction_memory_interface_inst_2_instruction3),
    .data4(w_level7_instruction_memory_interface_inst_2_instruction4),
    .data5(w_level7_instruction_memory_interface_inst_2_instruction5),
    .data6(w_level7_instruction_memory_interface_inst_2_instruction6),
    .data7(w_level7_instruction_memory_interface_inst_2_instruction7),
    .clock(clock),
    .load(instrload),
    .reset(reset),
    .instruction0(w_level7_instruction_register_8bit_inst_4_instruction0),
    .instruction1(w_level7_instruction_register_8bit_inst_4_instruction1),
    .instruction2(w_level7_instruction_register_8bit_inst_4_instruction2),
    .instruction3(w_level7_instruction_register_8bit_inst_4_instruction3),
    .instruction4(w_level7_instruction_register_8bit_inst_4_instruction4),
    .instruction5(w_level7_instruction_register_8bit_inst_4_instruction5),
    .instruction6(w_level7_instruction_register_8bit_inst_4_instruction6),
    .instruction7(w_level7_instruction_register_8bit_inst_4_instruction7),
    .opcode0(w_level7_instruction_register_8bit_inst_4_opcode0),
    .opcode1(w_level7_instruction_register_8bit_inst_4_opcode1),
    .opcode2(w_level7_instruction_register_8bit_inst_4_opcode2),
    .opcode3(w_level7_instruction_register_8bit_inst_4_opcode3),
    .opcode4(w_level7_instruction_register_8bit_inst_4_opcode4),
    .registeraddr0(w_level7_instruction_register_8bit_inst_4_registeraddr0),
    .registeraddr1(w_level7_instruction_register_8bit_inst_4_registeraddr1),
    .registeraddr2(w_level7_instruction_register_8bit_inst_4_registeraddr2)
);

assign pc0 = w_level7_cpu_program_counter_8bit_inst_1_address0;
assign instruction0 = w_level7_instruction_register_8bit_inst_4_instruction0;
assign opcode0 = w_level7_instruction_register_8bit_inst_4_opcode0;
assign registeraddr0 = w_level7_instruction_register_8bit_inst_4_registeraddr0;
assign rawinstr0 = w_level7_instruction_memory_interface_inst_2_instruction0;
assign pc1 = w_level7_cpu_program_counter_8bit_inst_1_address1;
assign instruction1 = w_level7_instruction_register_8bit_inst_4_instruction1;
assign opcode1 = w_level7_instruction_register_8bit_inst_4_opcode1;
assign registeraddr1 = w_level7_instruction_register_8bit_inst_4_registeraddr1;
assign rawinstr1 = w_level7_instruction_memory_interface_inst_2_instruction1;
assign pc2 = w_level7_cpu_program_counter_8bit_inst_1_address2;
assign instruction2 = w_level7_instruction_register_8bit_inst_4_instruction2;
assign opcode2 = w_level7_instruction_register_8bit_inst_4_opcode2;
assign registeraddr2 = w_level7_instruction_register_8bit_inst_4_registeraddr2;
assign rawinstr2 = w_level7_instruction_memory_interface_inst_2_instruction2;
assign pc3 = w_level7_cpu_program_counter_8bit_inst_1_address3;
assign instruction3 = w_level7_instruction_register_8bit_inst_4_instruction3;
assign opcode3 = w_level7_instruction_register_8bit_inst_4_opcode3;
assign rawinstr3 = w_level7_instruction_memory_interface_inst_2_instruction3;
assign pc4 = w_level7_cpu_program_counter_8bit_inst_1_address4;
assign instruction4 = w_level7_instruction_register_8bit_inst_4_instruction4;
assign opcode4 = w_level7_instruction_register_8bit_inst_4_opcode4;
assign rawinstr4 = w_level7_instruction_memory_interface_inst_2_instruction4;
assign pc5 = w_level7_cpu_program_counter_8bit_inst_1_address5;
assign instruction5 = w_level7_instruction_register_8bit_inst_4_instruction5;
assign rawinstr5 = w_level7_instruction_memory_interface_inst_2_instruction5;
assign pc6 = w_level7_cpu_program_counter_8bit_inst_1_address6;
assign instruction6 = w_level7_instruction_register_8bit_inst_4_instruction6;
assign rawinstr6 = w_level7_instruction_memory_interface_inst_2_instruction6;
assign pc7 = w_level7_cpu_program_counter_8bit_inst_1_address7;
assign instruction7 = w_level7_instruction_register_8bit_inst_4_instruction7;
assign rawinstr7 = w_level7_instruction_memory_interface_inst_2_instruction7;
endmodule

// Module for LEVEL9_MULTI_CYCLE_CPU_8BIT (generated from level9_multi_cycle_cpu_8bit.panda)
module level9_multi_cycle_cpu_8bit_ic (
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
    input regprogaddr0,
    input regprogaddr1,
    input regprogaddr2,
    input regprogwrite,
    input regprogdata0,
    input regprogdata1,
    input regprogdata2,
    input regprogdata3,
    input regprogdata4,
    input regprogdata5,
    input regprogdata6,
    input regprogdata7,
    output cyclecounter0,
    output pc0,
    output cyclecounter1,
    output instruction0,
    output pc1,
    output instruction1,
    output pc2,
    output instruction2,
    output pc3,
    output instruction3,
    output pc4,
    output instruction4,
    output pc5,
    output instruction5,
    output pc6,
    output instruction6,
    output pc7,
    output instruction7,
    output result0,
    output result1,
    output result2,
    output result3,
    output result4,
    output result5,
    output result6,
    output result7
);

reg aux_d_flip_flop_1_0_q = 1'b0;
reg aux_d_flip_flop_1_1_q = 1'b1;
reg aux_d_flip_flop_2_0_q = 1'b0;
reg aux_d_flip_flop_2_1_q = 1'b1;
// IC instance: Fetch (level8_fetch_stage)
wire w_level8_fetch_stage_inst_4_pc0;
wire w_level8_fetch_stage_inst_4_instruction0;
wire w_level8_fetch_stage_inst_4_opcode0;
wire w_level8_fetch_stage_inst_4_registeraddr0;
wire w_level8_fetch_stage_inst_4_rawinstr0;
wire w_level8_fetch_stage_inst_4_pc1;
wire w_level8_fetch_stage_inst_4_instruction1;
wire w_level8_fetch_stage_inst_4_opcode1;
wire w_level8_fetch_stage_inst_4_registeraddr1;
wire w_level8_fetch_stage_inst_4_rawinstr1;
wire w_level8_fetch_stage_inst_4_pc2;
wire w_level8_fetch_stage_inst_4_instruction2;
wire w_level8_fetch_stage_inst_4_opcode2;
wire w_level8_fetch_stage_inst_4_registeraddr2;
wire w_level8_fetch_stage_inst_4_rawinstr2;
wire w_level8_fetch_stage_inst_4_pc3;
wire w_level8_fetch_stage_inst_4_instruction3;
wire w_level8_fetch_stage_inst_4_opcode3;
wire w_level8_fetch_stage_inst_4_rawinstr3;
wire w_level8_fetch_stage_inst_4_pc4;
wire w_level8_fetch_stage_inst_4_instruction4;
wire w_level8_fetch_stage_inst_4_opcode4;
wire w_level8_fetch_stage_inst_4_rawinstr4;
wire w_level8_fetch_stage_inst_4_pc5;
wire w_level8_fetch_stage_inst_4_instruction5;
wire w_level8_fetch_stage_inst_4_rawinstr5;
wire w_level8_fetch_stage_inst_4_pc6;
wire w_level8_fetch_stage_inst_4_instruction6;
wire w_level8_fetch_stage_inst_4_rawinstr6;
wire w_level8_fetch_stage_inst_4_pc7;
wire w_level8_fetch_stage_inst_4_instruction7;
wire w_level8_fetch_stage_inst_4_rawinstr7;
// IC instance: Decode (level8_decode_stage)
wire w_level8_decode_stage_inst_5_aluop0;
wire w_level8_decode_stage_inst_5_regwrite;
wire w_level8_decode_stage_inst_5_memread;
wire w_level8_decode_stage_inst_5_memwrite;
wire w_level8_decode_stage_inst_5_aluop1;
wire w_level8_decode_stage_inst_5_aluop2;
wire w_level8_decode_stage_inst_5_instrdecodedlines0;
wire w_level8_decode_stage_inst_5_instrdecodedlines1;
wire w_level8_decode_stage_inst_5_instrdecodedlines2;
wire w_level8_decode_stage_inst_5_instrdecodedlines3;
wire w_level8_decode_stage_inst_5_instrdecodedlines4;
wire w_level8_decode_stage_inst_5_instrdecodedlines5;
wire w_level8_decode_stage_inst_5_instrdecodedlines6;
wire w_level8_decode_stage_inst_5_instrdecodedlines7;
wire w_level8_decode_stage_inst_5_instrdecodedlines8;
wire w_level8_decode_stage_inst_5_instrdecodedlines9;
wire w_level8_decode_stage_inst_5_instrdecodedlines10;
wire w_level8_decode_stage_inst_5_instrdecodedlines11;
wire w_level8_decode_stage_inst_5_instrdecodedlines12;
wire w_level8_decode_stage_inst_5_instrdecodedlines13;
wire w_level8_decode_stage_inst_5_instrdecodedlines14;
wire w_level8_decode_stage_inst_5_instrdecodedlines15;
wire w_level8_decode_stage_inst_5_instrdecodedlines16;
wire w_level8_decode_stage_inst_5_instrdecodedlines17;
wire w_level8_decode_stage_inst_5_instrdecodedlines18;
wire w_level8_decode_stage_inst_5_instrdecodedlines19;
wire w_level8_decode_stage_inst_5_instrdecodedlines20;
wire w_level8_decode_stage_inst_5_instrdecodedlines21;
wire w_level8_decode_stage_inst_5_instrdecodedlines22;
wire w_level8_decode_stage_inst_5_instrdecodedlines23;
wire w_level8_decode_stage_inst_5_instrdecodedlines24;
wire w_level8_decode_stage_inst_5_instrdecodedlines25;
wire w_level8_decode_stage_inst_5_instrdecodedlines26;
wire w_level8_decode_stage_inst_5_instrdecodedlines27;
wire w_level8_decode_stage_inst_5_instrdecodedlines28;
wire w_level8_decode_stage_inst_5_instrdecodedlines29;
wire w_level8_decode_stage_inst_5_instrdecodedlines30;
wire w_level8_decode_stage_inst_5_instrdecodedlines31;
// IC instance: Execute (level8_execute_stage)
wire w_level8_execute_stage_inst_6_result0;
wire w_level8_execute_stage_inst_6_zero;
wire w_level8_execute_stage_inst_6_result1;
wire w_level8_execute_stage_inst_6_sign;
wire w_level8_execute_stage_inst_6_result2;
wire w_level8_execute_stage_inst_6_result3;
wire w_level8_execute_stage_inst_6_result4;
wire w_level8_execute_stage_inst_6_result5;
wire w_level8_execute_stage_inst_6_result6;
wire w_level8_execute_stage_inst_6_result7;
// IC instance: Memory (level8_memory_stage)
wire w_level8_memory_stage_inst_7_dataout0;
wire w_level8_memory_stage_inst_7_dataout1;
wire w_level8_memory_stage_inst_7_dataout2;
wire w_level8_memory_stage_inst_7_dataout3;
wire w_level8_memory_stage_inst_7_dataout4;
wire w_level8_memory_stage_inst_7_dataout5;
wire w_level8_memory_stage_inst_7_dataout6;
wire w_level8_memory_stage_inst_7_dataout7;
// IC instance: RegFile (level6_register_file_8x8)
wire w_level6_register_file_8x8_inst_8_read_data10;
wire w_level6_register_file_8x8_inst_8_read_data11;
wire w_level6_register_file_8x8_inst_8_read_data12;
wire w_level6_register_file_8x8_inst_8_read_data13;
wire w_level6_register_file_8x8_inst_8_read_data14;
wire w_level6_register_file_8x8_inst_8_read_data15;
wire w_level6_register_file_8x8_inst_8_read_data16;
wire w_level6_register_file_8x8_inst_8_read_data17;
wire w_level6_register_file_8x8_inst_8_read_data20;
wire w_level6_register_file_8x8_inst_8_read_data21;
wire w_level6_register_file_8x8_inst_8_read_data22;
wire w_level6_register_file_8x8_inst_8_read_data23;
wire w_level6_register_file_8x8_inst_8_read_data24;
wire w_level6_register_file_8x8_inst_8_read_data25;
wire w_level6_register_file_8x8_inst_8_read_data26;
wire w_level6_register_file_8x8_inst_8_read_data27;
// IC instance: WriteDataMux (level4_bus_mux_8bit)
wire w_level4_bus_mux_8bit_inst_9_out0;
wire w_level4_bus_mux_8bit_inst_9_out1;
wire w_level4_bus_mux_8bit_inst_9_out2;
wire w_level4_bus_mux_8bit_inst_9_out3;
wire w_level4_bus_mux_8bit_inst_9_out4;
wire w_level4_bus_mux_8bit_inst_9_out5;
wire w_level4_bus_mux_8bit_inst_9_out6;
wire w_level4_bus_mux_8bit_inst_9_out7;
reg aux_mux_10 = 1'b0;
reg aux_mux_11 = 1'b0;
reg aux_mux_12 = 1'b0;
wire aux_not_13;
wire aux_not_14;
wire aux_and_15;
wire aux_and_16;
wire aux_not_17;
wire aux_and_18;
wire aux_and_19;
wire aux_or_20;
wire aux_xor_21;

// Internal logic
    //D FlipFlop
    always @(posedge clock or negedge aux_not_17)
    begin
        if (~aux_not_17)
        begin
            aux_d_flip_flop_1_0_q <= 1'b0;
            aux_d_flip_flop_1_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_1_0_q <= aux_not_13;
            aux_d_flip_flop_1_1_q <= ~aux_not_13;
        end
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock or negedge aux_not_17)
    begin
        if (~aux_not_17)
        begin
            aux_d_flip_flop_2_0_q <= 1'b0;
            aux_d_flip_flop_2_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_2_0_q <= aux_xor_21;
            aux_d_flip_flop_2_1_q <= ~aux_xor_21;
        end
    end
    //End of D FlipFlop
level8_fetch_stage level8_fetch_stage_inst_4 (
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
    .pcinc(aux_and_15),
    .instrload(aux_and_15),
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
    .pc0(w_level8_fetch_stage_inst_4_pc0),
    .instruction0(w_level8_fetch_stage_inst_4_instruction0),
    .opcode0(w_level8_fetch_stage_inst_4_opcode0),
    .registeraddr0(w_level8_fetch_stage_inst_4_registeraddr0),
    .rawinstr0(w_level8_fetch_stage_inst_4_rawinstr0),
    .pc1(w_level8_fetch_stage_inst_4_pc1),
    .instruction1(w_level8_fetch_stage_inst_4_instruction1),
    .opcode1(w_level8_fetch_stage_inst_4_opcode1),
    .registeraddr1(w_level8_fetch_stage_inst_4_registeraddr1),
    .rawinstr1(w_level8_fetch_stage_inst_4_rawinstr1),
    .pc2(w_level8_fetch_stage_inst_4_pc2),
    .instruction2(w_level8_fetch_stage_inst_4_instruction2),
    .opcode2(w_level8_fetch_stage_inst_4_opcode2),
    .registeraddr2(w_level8_fetch_stage_inst_4_registeraddr2),
    .rawinstr2(w_level8_fetch_stage_inst_4_rawinstr2),
    .pc3(w_level8_fetch_stage_inst_4_pc3),
    .instruction3(w_level8_fetch_stage_inst_4_instruction3),
    .opcode3(w_level8_fetch_stage_inst_4_opcode3),
    .rawinstr3(w_level8_fetch_stage_inst_4_rawinstr3),
    .pc4(w_level8_fetch_stage_inst_4_pc4),
    .instruction4(w_level8_fetch_stage_inst_4_instruction4),
    .opcode4(w_level8_fetch_stage_inst_4_opcode4),
    .rawinstr4(w_level8_fetch_stage_inst_4_rawinstr4),
    .pc5(w_level8_fetch_stage_inst_4_pc5),
    .instruction5(w_level8_fetch_stage_inst_4_instruction5),
    .rawinstr5(w_level8_fetch_stage_inst_4_rawinstr5),
    .pc6(w_level8_fetch_stage_inst_4_pc6),
    .instruction6(w_level8_fetch_stage_inst_4_instruction6),
    .rawinstr6(w_level8_fetch_stage_inst_4_rawinstr6),
    .pc7(w_level8_fetch_stage_inst_4_pc7),
    .instruction7(w_level8_fetch_stage_inst_4_instruction7),
    .rawinstr7(w_level8_fetch_stage_inst_4_rawinstr7)
);
level8_decode_stage level8_decode_stage_inst_5 (
    .opcode0(w_level8_fetch_stage_inst_4_opcode0),
    .opcode1(w_level8_fetch_stage_inst_4_opcode1),
    .opcode2(w_level8_fetch_stage_inst_4_opcode2),
    .opcode3(w_level8_fetch_stage_inst_4_opcode3),
    .opcode4(w_level8_fetch_stage_inst_4_opcode4),
    .aluop0(w_level8_decode_stage_inst_5_aluop0),
    .regwrite(w_level8_decode_stage_inst_5_regwrite),
    .memread(w_level8_decode_stage_inst_5_memread),
    .memwrite(w_level8_decode_stage_inst_5_memwrite),
    .aluop1(w_level8_decode_stage_inst_5_aluop1),
    .aluop2(w_level8_decode_stage_inst_5_aluop2),
    .instrdecodedlines0(w_level8_decode_stage_inst_5_instrdecodedlines0),
    .instrdecodedlines1(w_level8_decode_stage_inst_5_instrdecodedlines1),
    .instrdecodedlines2(w_level8_decode_stage_inst_5_instrdecodedlines2),
    .instrdecodedlines3(w_level8_decode_stage_inst_5_instrdecodedlines3),
    .instrdecodedlines4(w_level8_decode_stage_inst_5_instrdecodedlines4),
    .instrdecodedlines5(w_level8_decode_stage_inst_5_instrdecodedlines5),
    .instrdecodedlines6(w_level8_decode_stage_inst_5_instrdecodedlines6),
    .instrdecodedlines7(w_level8_decode_stage_inst_5_instrdecodedlines7),
    .instrdecodedlines8(w_level8_decode_stage_inst_5_instrdecodedlines8),
    .instrdecodedlines9(w_level8_decode_stage_inst_5_instrdecodedlines9),
    .instrdecodedlines10(w_level8_decode_stage_inst_5_instrdecodedlines10),
    .instrdecodedlines11(w_level8_decode_stage_inst_5_instrdecodedlines11),
    .instrdecodedlines12(w_level8_decode_stage_inst_5_instrdecodedlines12),
    .instrdecodedlines13(w_level8_decode_stage_inst_5_instrdecodedlines13),
    .instrdecodedlines14(w_level8_decode_stage_inst_5_instrdecodedlines14),
    .instrdecodedlines15(w_level8_decode_stage_inst_5_instrdecodedlines15),
    .instrdecodedlines16(w_level8_decode_stage_inst_5_instrdecodedlines16),
    .instrdecodedlines17(w_level8_decode_stage_inst_5_instrdecodedlines17),
    .instrdecodedlines18(w_level8_decode_stage_inst_5_instrdecodedlines18),
    .instrdecodedlines19(w_level8_decode_stage_inst_5_instrdecodedlines19),
    .instrdecodedlines20(w_level8_decode_stage_inst_5_instrdecodedlines20),
    .instrdecodedlines21(w_level8_decode_stage_inst_5_instrdecodedlines21),
    .instrdecodedlines22(w_level8_decode_stage_inst_5_instrdecodedlines22),
    .instrdecodedlines23(w_level8_decode_stage_inst_5_instrdecodedlines23),
    .instrdecodedlines24(w_level8_decode_stage_inst_5_instrdecodedlines24),
    .instrdecodedlines25(w_level8_decode_stage_inst_5_instrdecodedlines25),
    .instrdecodedlines26(w_level8_decode_stage_inst_5_instrdecodedlines26),
    .instrdecodedlines27(w_level8_decode_stage_inst_5_instrdecodedlines27),
    .instrdecodedlines28(w_level8_decode_stage_inst_5_instrdecodedlines28),
    .instrdecodedlines29(w_level8_decode_stage_inst_5_instrdecodedlines29),
    .instrdecodedlines30(w_level8_decode_stage_inst_5_instrdecodedlines30),
    .instrdecodedlines31(w_level8_decode_stage_inst_5_instrdecodedlines31)
);
level8_execute_stage level8_execute_stage_inst_6 (
    .operanda0(w_level6_register_file_8x8_inst_8_read_data10),
    .operanda1(w_level6_register_file_8x8_inst_8_read_data11),
    .operanda2(w_level6_register_file_8x8_inst_8_read_data12),
    .operanda3(w_level6_register_file_8x8_inst_8_read_data13),
    .operanda4(w_level6_register_file_8x8_inst_8_read_data14),
    .operanda5(w_level6_register_file_8x8_inst_8_read_data15),
    .operanda6(w_level6_register_file_8x8_inst_8_read_data16),
    .operanda7(w_level6_register_file_8x8_inst_8_read_data17),
    .aluop0(w_level8_decode_stage_inst_5_aluop0),
    .operandb0(w_level6_register_file_8x8_inst_8_read_data20),
    .operandb1(w_level6_register_file_8x8_inst_8_read_data21),
    .operandb2(w_level6_register_file_8x8_inst_8_read_data22),
    .operandb3(w_level6_register_file_8x8_inst_8_read_data23),
    .operandb4(w_level6_register_file_8x8_inst_8_read_data24),
    .operandb5(w_level6_register_file_8x8_inst_8_read_data25),
    .operandb6(w_level6_register_file_8x8_inst_8_read_data26),
    .operandb7(w_level6_register_file_8x8_inst_8_read_data27),
    .aluop1(w_level8_decode_stage_inst_5_aluop1),
    .aluop2(w_level8_decode_stage_inst_5_aluop2),
    .result0(w_level8_execute_stage_inst_6_result0),
    .zero(w_level8_execute_stage_inst_6_zero),
    .result1(w_level8_execute_stage_inst_6_result1),
    .sign(w_level8_execute_stage_inst_6_sign),
    .result2(w_level8_execute_stage_inst_6_result2),
    .result3(w_level8_execute_stage_inst_6_result3),
    .result4(w_level8_execute_stage_inst_6_result4),
    .result5(w_level8_execute_stage_inst_6_result5),
    .result6(w_level8_execute_stage_inst_6_result6),
    .result7(w_level8_execute_stage_inst_6_result7)
);
level8_memory_stage level8_memory_stage_inst_7 (
    .address0(w_level8_fetch_stage_inst_4_instruction0),
    .address1(w_level8_fetch_stage_inst_4_instruction1),
    .address2(w_level8_fetch_stage_inst_4_instruction2),
    .address3(1'b0),
    .address4(1'b0),
    .address5(1'b0),
    .address6(1'b0),
    .address7(1'b0),
    .memread(w_level8_decode_stage_inst_5_memread),
    .datain0(w_level6_register_file_8x8_inst_8_read_data10),
    .datain1(w_level6_register_file_8x8_inst_8_read_data11),
    .datain2(w_level6_register_file_8x8_inst_8_read_data12),
    .datain3(w_level6_register_file_8x8_inst_8_read_data13),
    .datain4(w_level6_register_file_8x8_inst_8_read_data14),
    .datain5(w_level6_register_file_8x8_inst_8_read_data15),
    .datain6(w_level6_register_file_8x8_inst_8_read_data16),
    .datain7(w_level6_register_file_8x8_inst_8_read_data17),
    .memwrite(aux_and_18),
    .result0(w_level8_execute_stage_inst_6_result0),
    .result1(w_level8_execute_stage_inst_6_result1),
    .result2(w_level8_execute_stage_inst_6_result2),
    .result3(w_level8_execute_stage_inst_6_result3),
    .result4(w_level8_execute_stage_inst_6_result4),
    .result5(w_level8_execute_stage_inst_6_result5),
    .result6(w_level8_execute_stage_inst_6_result6),
    .result7(w_level8_execute_stage_inst_6_result7),
    .clock(clock),
    .reset(reset),
    .dataout0(w_level8_memory_stage_inst_7_dataout0),
    .dataout1(w_level8_memory_stage_inst_7_dataout1),
    .dataout2(w_level8_memory_stage_inst_7_dataout2),
    .dataout3(w_level8_memory_stage_inst_7_dataout3),
    .dataout4(w_level8_memory_stage_inst_7_dataout4),
    .dataout5(w_level8_memory_stage_inst_7_dataout5),
    .dataout6(w_level8_memory_stage_inst_7_dataout6),
    .dataout7(w_level8_memory_stage_inst_7_dataout7)
);
level6_register_file_8x8 level6_register_file_8x8_inst_8 (
    .write_addr0(aux_mux_10),
    .write_addr1(aux_mux_11),
    .write_addr2(aux_mux_12),
    .read_addr10(1'b0),
    .read_addr11(1'b0),
    .read_addr12(1'b0),
    .read_addr20(w_level8_fetch_stage_inst_4_instruction0),
    .read_addr21(w_level8_fetch_stage_inst_4_instruction1),
    .read_addr22(w_level8_fetch_stage_inst_4_instruction2),
    .data_in0(w_level4_bus_mux_8bit_inst_9_out0),
    .data_in1(w_level4_bus_mux_8bit_inst_9_out1),
    .data_in2(w_level4_bus_mux_8bit_inst_9_out2),
    .data_in3(w_level4_bus_mux_8bit_inst_9_out3),
    .data_in4(w_level4_bus_mux_8bit_inst_9_out4),
    .data_in5(w_level4_bus_mux_8bit_inst_9_out5),
    .data_in6(w_level4_bus_mux_8bit_inst_9_out6),
    .data_in7(w_level4_bus_mux_8bit_inst_9_out7),
    .write_enable(aux_or_20),
    .clock(clock),
    .read_data10(w_level6_register_file_8x8_inst_8_read_data10),
    .read_data11(w_level6_register_file_8x8_inst_8_read_data11),
    .read_data12(w_level6_register_file_8x8_inst_8_read_data12),
    .read_data13(w_level6_register_file_8x8_inst_8_read_data13),
    .read_data14(w_level6_register_file_8x8_inst_8_read_data14),
    .read_data15(w_level6_register_file_8x8_inst_8_read_data15),
    .read_data16(w_level6_register_file_8x8_inst_8_read_data16),
    .read_data17(w_level6_register_file_8x8_inst_8_read_data17),
    .read_data20(w_level6_register_file_8x8_inst_8_read_data20),
    .read_data21(w_level6_register_file_8x8_inst_8_read_data21),
    .read_data22(w_level6_register_file_8x8_inst_8_read_data22),
    .read_data23(w_level6_register_file_8x8_inst_8_read_data23),
    .read_data24(w_level6_register_file_8x8_inst_8_read_data24),
    .read_data25(w_level6_register_file_8x8_inst_8_read_data25),
    .read_data26(w_level6_register_file_8x8_inst_8_read_data26),
    .read_data27(w_level6_register_file_8x8_inst_8_read_data27)
);
level4_bus_mux_8bit level4_bus_mux_8bit_inst_9 (
    .in00(w_level8_memory_stage_inst_7_dataout0),
    .in01(w_level8_memory_stage_inst_7_dataout1),
    .in02(w_level8_memory_stage_inst_7_dataout2),
    .in03(w_level8_memory_stage_inst_7_dataout3),
    .in04(w_level8_memory_stage_inst_7_dataout4),
    .in05(w_level8_memory_stage_inst_7_dataout5),
    .in06(w_level8_memory_stage_inst_7_dataout6),
    .in07(w_level8_memory_stage_inst_7_dataout7),
    .in10(regprogdata0),
    .in11(regprogdata1),
    .in12(regprogdata2),
    .in13(regprogdata3),
    .in14(regprogdata4),
    .in15(regprogdata5),
    .in16(regprogdata6),
    .in17(regprogdata7),
    .sel(regprogwrite),
    .out0(w_level4_bus_mux_8bit_inst_9_out0),
    .out1(w_level4_bus_mux_8bit_inst_9_out1),
    .out2(w_level4_bus_mux_8bit_inst_9_out2),
    .out3(w_level4_bus_mux_8bit_inst_9_out3),
    .out4(w_level4_bus_mux_8bit_inst_9_out4),
    .out5(w_level4_bus_mux_8bit_inst_9_out5),
    .out6(w_level4_bus_mux_8bit_inst_9_out6),
    .out7(w_level4_bus_mux_8bit_inst_9_out7)
);
    //Multiplexer
    always @(*)
    begin
        case({regprogwrite})
            1'd0: aux_mux_10 = 1'b0;
            1'd1: aux_mux_10 = regprogaddr0;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({regprogwrite})
            1'd0: aux_mux_11 = 1'b0;
            1'd1: aux_mux_11 = regprogaddr1;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({regprogwrite})
            1'd0: aux_mux_12 = 1'b0;
            1'd1: aux_mux_12 = regprogaddr2;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_not_13 = ~aux_d_flip_flop_1_0_q;
assign aux_not_14 = ~aux_d_flip_flop_2_0_q;
assign aux_and_15 = (aux_not_14 & aux_not_13);
assign aux_and_16 = (aux_d_flip_flop_2_0_q & aux_d_flip_flop_1_0_q);
assign aux_not_17 = ~reset;
assign aux_and_18 = (w_level8_decode_stage_inst_5_memwrite & aux_and_16);
assign aux_and_19 = (w_level8_decode_stage_inst_5_regwrite & aux_and_16);
assign aux_or_20 = (aux_and_19 | regprogwrite);
assign aux_xor_21 = (aux_d_flip_flop_1_0_q ^ aux_d_flip_flop_2_0_q);

assign cyclecounter0 = aux_d_flip_flop_1_0_q;
assign pc0 = w_level8_fetch_stage_inst_4_pc0;
assign cyclecounter1 = aux_d_flip_flop_2_0_q;
assign instruction0 = w_level8_fetch_stage_inst_4_instruction0;
assign pc1 = w_level8_fetch_stage_inst_4_pc1;
assign instruction1 = w_level8_fetch_stage_inst_4_instruction1;
assign pc2 = w_level8_fetch_stage_inst_4_pc2;
assign instruction2 = w_level8_fetch_stage_inst_4_instruction2;
assign pc3 = w_level8_fetch_stage_inst_4_pc3;
assign instruction3 = w_level8_fetch_stage_inst_4_instruction3;
assign pc4 = w_level8_fetch_stage_inst_4_pc4;
assign instruction4 = w_level8_fetch_stage_inst_4_instruction4;
assign pc5 = w_level8_fetch_stage_inst_4_pc5;
assign instruction5 = w_level8_fetch_stage_inst_4_instruction5;
assign pc6 = w_level8_fetch_stage_inst_4_pc6;
assign instruction6 = w_level8_fetch_stage_inst_4_instruction6;
assign pc7 = w_level8_fetch_stage_inst_4_pc7;
assign instruction7 = w_level8_fetch_stage_inst_4_instruction7;
assign result0 = w_level8_memory_stage_inst_7_dataout0;
assign result1 = w_level8_memory_stage_inst_7_dataout1;
assign result2 = w_level8_memory_stage_inst_7_dataout2;
assign result3 = w_level8_memory_stage_inst_7_dataout3;
assign result4 = w_level8_memory_stage_inst_7_dataout4;
assign result5 = w_level8_memory_stage_inst_7_dataout5;
assign result6 = w_level8_memory_stage_inst_7_dataout6;
assign result7 = w_level8_memory_stage_inst_7_dataout7;
endmodule

module level9_multi_cycle_cpu_8bit (
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
input input_switch28,
input input_switch29,
input input_switch30,
input input_switch31,

/* ========= Outputs ========== */
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
output led58_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL9_MULTI_CYCLE_CPU_8BIT (level9_multi_cycle_cpu_8bit_ic)
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_cyclecounter0;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc0;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_cyclecounter1;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction0;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc1;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction1;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc2;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction2;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc3;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction3;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc4;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction4;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc5;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction5;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc6;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction6;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc7;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction7;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_result0;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_result1;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_result2;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_result3;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_result4;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_result5;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_result6;
wire w_level9_multi_cycle_cpu_8bit_ic_inst_1_result7;


// Assigning aux variables. //
level9_multi_cycle_cpu_8bit_ic level9_multi_cycle_cpu_8bit_ic_inst_1 (
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
    .regprogaddr0(input_switch20),
    .regprogaddr1(input_switch21),
    .regprogaddr2(input_switch22),
    .regprogwrite(input_switch23),
    .regprogdata0(input_switch24),
    .regprogdata1(input_switch25),
    .regprogdata2(input_switch26),
    .regprogdata3(input_switch27),
    .regprogdata4(input_switch28),
    .regprogdata5(input_switch29),
    .regprogdata6(input_switch30),
    .regprogdata7(input_switch31),
    .cyclecounter0(w_level9_multi_cycle_cpu_8bit_ic_inst_1_cyclecounter0),
    .pc0(w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc0),
    .cyclecounter1(w_level9_multi_cycle_cpu_8bit_ic_inst_1_cyclecounter1),
    .instruction0(w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction0),
    .pc1(w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc1),
    .instruction1(w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction1),
    .pc2(w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc2),
    .instruction2(w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction2),
    .pc3(w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc3),
    .instruction3(w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction3),
    .pc4(w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc4),
    .instruction4(w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction4),
    .pc5(w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc5),
    .instruction5(w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction5),
    .pc6(w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc6),
    .instruction6(w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction6),
    .pc7(w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc7),
    .instruction7(w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction7),
    .result0(w_level9_multi_cycle_cpu_8bit_ic_inst_1_result0),
    .result1(w_level9_multi_cycle_cpu_8bit_ic_inst_1_result1),
    .result2(w_level9_multi_cycle_cpu_8bit_ic_inst_1_result2),
    .result3(w_level9_multi_cycle_cpu_8bit_ic_inst_1_result3),
    .result4(w_level9_multi_cycle_cpu_8bit_ic_inst_1_result4),
    .result5(w_level9_multi_cycle_cpu_8bit_ic_inst_1_result5),
    .result6(w_level9_multi_cycle_cpu_8bit_ic_inst_1_result6),
    .result7(w_level9_multi_cycle_cpu_8bit_ic_inst_1_result7)
);

// Writing output data. //
assign led33_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_cyclecounter0;
assign led34_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc0;
assign led35_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_cyclecounter1;
assign led36_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction0;
assign led37_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc1;
assign led38_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction1;
assign led39_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc2;
assign led40_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction2;
assign led41_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc3;
assign led42_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction3;
assign led43_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc4;
assign led44_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction4;
assign led45_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc5;
assign led46_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction5;
assign led47_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc6;
assign led48_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction6;
assign led49_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_pc7;
assign led50_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_instruction7;
assign led51_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_result0;
assign led52_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_result1;
assign led53_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_result2;
assign led54_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_result3;
assign led55_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_result4;
assign led56_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_result5;
assign led57_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_result6;
assign led58_1 = w_level9_multi_cycle_cpu_8bit_ic_inst_1_result7;
endmodule
