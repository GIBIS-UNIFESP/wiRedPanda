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

// Module for Mux[0] (generated from level2_mux_2to1.panda)
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
    .p_output(w_level2_mux_2to1_inst_1_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_2 (
    .data0(in01),
    .data1(in11),
    .sel0(sel),
    .p_output(w_level2_mux_2to1_inst_2_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_3 (
    .data0(in02),
    .data1(in12),
    .sel0(sel),
    .p_output(w_level2_mux_2to1_inst_3_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_4 (
    .data0(in03),
    .data1(in13),
    .sel0(sel),
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

// Module for LEVEL8_FETCH_STAGE (generated from level8_fetch_stage.panda)
module level8_fetch_stage_ic (
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
    .writeenable(1'b1),
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

module level8_fetch_stage (
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

/* ========= Outputs ========== */
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
output led63_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL8_FETCH_STAGE (level8_fetch_stage_ic)
wire w_level8_fetch_stage_ic_inst_1_pc0;
wire w_level8_fetch_stage_ic_inst_1_instruction0;
wire w_level8_fetch_stage_ic_inst_1_opcode0;
wire w_level8_fetch_stage_ic_inst_1_registeraddr0;
wire w_level8_fetch_stage_ic_inst_1_rawinstr0;
wire w_level8_fetch_stage_ic_inst_1_pc1;
wire w_level8_fetch_stage_ic_inst_1_instruction1;
wire w_level8_fetch_stage_ic_inst_1_opcode1;
wire w_level8_fetch_stage_ic_inst_1_registeraddr1;
wire w_level8_fetch_stage_ic_inst_1_rawinstr1;
wire w_level8_fetch_stage_ic_inst_1_pc2;
wire w_level8_fetch_stage_ic_inst_1_instruction2;
wire w_level8_fetch_stage_ic_inst_1_opcode2;
wire w_level8_fetch_stage_ic_inst_1_registeraddr2;
wire w_level8_fetch_stage_ic_inst_1_rawinstr2;
wire w_level8_fetch_stage_ic_inst_1_pc3;
wire w_level8_fetch_stage_ic_inst_1_instruction3;
wire w_level8_fetch_stage_ic_inst_1_opcode3;
wire w_level8_fetch_stage_ic_inst_1_rawinstr3;
wire w_level8_fetch_stage_ic_inst_1_pc4;
wire w_level8_fetch_stage_ic_inst_1_instruction4;
wire w_level8_fetch_stage_ic_inst_1_opcode4;
wire w_level8_fetch_stage_ic_inst_1_rawinstr4;
wire w_level8_fetch_stage_ic_inst_1_pc5;
wire w_level8_fetch_stage_ic_inst_1_instruction5;
wire w_level8_fetch_stage_ic_inst_1_rawinstr5;
wire w_level8_fetch_stage_ic_inst_1_pc6;
wire w_level8_fetch_stage_ic_inst_1_instruction6;
wire w_level8_fetch_stage_ic_inst_1_rawinstr6;
wire w_level8_fetch_stage_ic_inst_1_pc7;
wire w_level8_fetch_stage_ic_inst_1_instruction7;
wire w_level8_fetch_stage_ic_inst_1_rawinstr7;


// Assigning aux variables. //
level8_fetch_stage_ic level8_fetch_stage_ic_inst_1 (
    .pcdata0(input_switch1),
    .pcdata1(input_switch2),
    .pcdata2(input_switch3),
    .pcdata3(input_switch4),
    .pcdata4(input_switch5),
    .pcdata5(input_switch6),
    .pcdata6(input_switch7),
    .pcdata7(input_switch8),
    .clock(input_switch9),
    .reset(input_switch10),
    .pcload(input_switch11),
    .pcinc(input_switch12),
    .instrload(input_switch13),
    .progaddr0(input_switch14),
    .progaddr1(input_switch15),
    .progaddr2(input_switch16),
    .progaddr3(input_switch17),
    .progaddr4(input_switch18),
    .progaddr5(input_switch19),
    .progaddr6(input_switch20),
    .progaddr7(input_switch21),
    .progwrite(input_switch22),
    .progdata0(input_switch23),
    .progdata1(input_switch24),
    .progdata2(input_switch25),
    .progdata3(input_switch26),
    .progdata4(input_switch27),
    .progdata5(input_switch28),
    .progdata6(input_switch29),
    .progdata7(input_switch30),
    .pc0(w_level8_fetch_stage_ic_inst_1_pc0),
    .instruction0(w_level8_fetch_stage_ic_inst_1_instruction0),
    .opcode0(w_level8_fetch_stage_ic_inst_1_opcode0),
    .registeraddr0(w_level8_fetch_stage_ic_inst_1_registeraddr0),
    .rawinstr0(w_level8_fetch_stage_ic_inst_1_rawinstr0),
    .pc1(w_level8_fetch_stage_ic_inst_1_pc1),
    .instruction1(w_level8_fetch_stage_ic_inst_1_instruction1),
    .opcode1(w_level8_fetch_stage_ic_inst_1_opcode1),
    .registeraddr1(w_level8_fetch_stage_ic_inst_1_registeraddr1),
    .rawinstr1(w_level8_fetch_stage_ic_inst_1_rawinstr1),
    .pc2(w_level8_fetch_stage_ic_inst_1_pc2),
    .instruction2(w_level8_fetch_stage_ic_inst_1_instruction2),
    .opcode2(w_level8_fetch_stage_ic_inst_1_opcode2),
    .registeraddr2(w_level8_fetch_stage_ic_inst_1_registeraddr2),
    .rawinstr2(w_level8_fetch_stage_ic_inst_1_rawinstr2),
    .pc3(w_level8_fetch_stage_ic_inst_1_pc3),
    .instruction3(w_level8_fetch_stage_ic_inst_1_instruction3),
    .opcode3(w_level8_fetch_stage_ic_inst_1_opcode3),
    .rawinstr3(w_level8_fetch_stage_ic_inst_1_rawinstr3),
    .pc4(w_level8_fetch_stage_ic_inst_1_pc4),
    .instruction4(w_level8_fetch_stage_ic_inst_1_instruction4),
    .opcode4(w_level8_fetch_stage_ic_inst_1_opcode4),
    .rawinstr4(w_level8_fetch_stage_ic_inst_1_rawinstr4),
    .pc5(w_level8_fetch_stage_ic_inst_1_pc5),
    .instruction5(w_level8_fetch_stage_ic_inst_1_instruction5),
    .rawinstr5(w_level8_fetch_stage_ic_inst_1_rawinstr5),
    .pc6(w_level8_fetch_stage_ic_inst_1_pc6),
    .instruction6(w_level8_fetch_stage_ic_inst_1_instruction6),
    .rawinstr6(w_level8_fetch_stage_ic_inst_1_rawinstr6),
    .pc7(w_level8_fetch_stage_ic_inst_1_pc7),
    .instruction7(w_level8_fetch_stage_ic_inst_1_instruction7),
    .rawinstr7(w_level8_fetch_stage_ic_inst_1_rawinstr7)
);

// Writing output data. //
assign led32_1 = w_level8_fetch_stage_ic_inst_1_pc0;
assign led33_1 = w_level8_fetch_stage_ic_inst_1_instruction0;
assign led34_1 = w_level8_fetch_stage_ic_inst_1_opcode0;
assign led35_1 = w_level8_fetch_stage_ic_inst_1_registeraddr0;
assign led36_1 = w_level8_fetch_stage_ic_inst_1_rawinstr0;
assign led37_1 = w_level8_fetch_stage_ic_inst_1_pc1;
assign led38_1 = w_level8_fetch_stage_ic_inst_1_instruction1;
assign led39_1 = w_level8_fetch_stage_ic_inst_1_opcode1;
assign led40_1 = w_level8_fetch_stage_ic_inst_1_registeraddr1;
assign led41_1 = w_level8_fetch_stage_ic_inst_1_rawinstr1;
assign led42_1 = w_level8_fetch_stage_ic_inst_1_pc2;
assign led43_1 = w_level8_fetch_stage_ic_inst_1_instruction2;
assign led44_1 = w_level8_fetch_stage_ic_inst_1_opcode2;
assign led45_1 = w_level8_fetch_stage_ic_inst_1_registeraddr2;
assign led46_1 = w_level8_fetch_stage_ic_inst_1_rawinstr2;
assign led47_1 = w_level8_fetch_stage_ic_inst_1_pc3;
assign led48_1 = w_level8_fetch_stage_ic_inst_1_instruction3;
assign led49_1 = w_level8_fetch_stage_ic_inst_1_opcode3;
assign led50_1 = w_level8_fetch_stage_ic_inst_1_rawinstr3;
assign led51_1 = w_level8_fetch_stage_ic_inst_1_pc4;
assign led52_1 = w_level8_fetch_stage_ic_inst_1_instruction4;
assign led53_1 = w_level8_fetch_stage_ic_inst_1_opcode4;
assign led54_1 = w_level8_fetch_stage_ic_inst_1_rawinstr4;
assign led55_1 = w_level8_fetch_stage_ic_inst_1_pc5;
assign led56_1 = w_level8_fetch_stage_ic_inst_1_instruction5;
assign led57_1 = w_level8_fetch_stage_ic_inst_1_rawinstr5;
assign led58_1 = w_level8_fetch_stage_ic_inst_1_pc6;
assign led59_1 = w_level8_fetch_stage_ic_inst_1_instruction6;
assign led60_1 = w_level8_fetch_stage_ic_inst_1_rawinstr6;
assign led61_1 = w_level8_fetch_stage_ic_inst_1_pc7;
assign led62_1 = w_level8_fetch_stage_ic_inst_1_instruction7;
assign led63_1 = w_level8_fetch_stage_ic_inst_1_rawinstr7;
endmodule
