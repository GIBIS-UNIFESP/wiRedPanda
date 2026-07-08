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

// IC instance: AddrDecoder (level2_decoder_3to8)
wire w_level2_decoder_3to8_inst_1_out0;
wire w_level2_decoder_3to8_inst_1_out1;
wire w_level2_decoder_3to8_inst_1_out2;
wire w_level2_decoder_3to8_inst_1_out3;
wire w_level2_decoder_3to8_inst_1_out4;
wire w_level2_decoder_3to8_inst_1_out5;
wire w_level2_decoder_3to8_inst_1_out6;
wire w_level2_decoder_3to8_inst_1_out7;
wire aux_not_2;
wire aux_and_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;
wire aux_and_9;
wire aux_and_10;
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
reg aux_mux_25 = 1'b0;
reg aux_d_flip_flop_26_0_q = 1'b0;
reg aux_d_flip_flop_26_1_q = 1'b1;
// IC instance: ReadMux (level2_mux_8to1)
wire w_level2_mux_8to1_inst_27_p_output;

// Internal logic
level2_decoder_3to8 level2_decoder_3to8_inst_1 (
    .addr0(address0),
    .addr1(address1),
    .addr2(address2),
    .enable(1'b1),
    .out0(w_level2_decoder_3to8_inst_1_out0),
    .out1(w_level2_decoder_3to8_inst_1_out1),
    .out2(w_level2_decoder_3to8_inst_1_out2),
    .out3(w_level2_decoder_3to8_inst_1_out3),
    .out4(w_level2_decoder_3to8_inst_1_out4),
    .out5(w_level2_decoder_3to8_inst_1_out5),
    .out6(w_level2_decoder_3to8_inst_1_out6),
    .out7(w_level2_decoder_3to8_inst_1_out7)
);
assign aux_not_2 = ~reset;
assign aux_and_3 = (w_level2_decoder_3to8_inst_1_out0 & writeenable);
assign aux_and_4 = (w_level2_decoder_3to8_inst_1_out1 & writeenable);
assign aux_and_5 = (w_level2_decoder_3to8_inst_1_out2 & writeenable);
assign aux_and_6 = (w_level2_decoder_3to8_inst_1_out3 & writeenable);
assign aux_and_7 = (w_level2_decoder_3to8_inst_1_out4 & writeenable);
assign aux_and_8 = (w_level2_decoder_3to8_inst_1_out5 & writeenable);
assign aux_and_9 = (w_level2_decoder_3to8_inst_1_out6 & writeenable);
assign aux_and_10 = (w_level2_decoder_3to8_inst_1_out7 & writeenable);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_11 = aux_d_flip_flop_12_0_q;
            1'd1: aux_mux_11 = datain;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_2)
    begin
        if (~aux_not_2)
        begin
            aux_d_flip_flop_12_0_q <= 1'b0;
            aux_d_flip_flop_12_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_12_0_q <= aux_mux_11;
            aux_d_flip_flop_12_1_q <= ~aux_mux_11;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_13 = aux_d_flip_flop_14_0_q;
            1'd1: aux_mux_13 = datain;
            default: aux_mux_13 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_2)
    begin
        if (~aux_not_2)
        begin
            aux_d_flip_flop_14_0_q <= 1'b0;
            aux_d_flip_flop_14_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_14_0_q <= aux_mux_13;
            aux_d_flip_flop_14_1_q <= ~aux_mux_13;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_15 = aux_d_flip_flop_16_0_q;
            1'd1: aux_mux_15 = datain;
            default: aux_mux_15 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_2)
    begin
        if (~aux_not_2)
        begin
            aux_d_flip_flop_16_0_q <= 1'b0;
            aux_d_flip_flop_16_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_16_0_q <= aux_mux_15;
            aux_d_flip_flop_16_1_q <= ~aux_mux_15;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_17 = aux_d_flip_flop_18_0_q;
            1'd1: aux_mux_17 = datain;
            default: aux_mux_17 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_2)
    begin
        if (~aux_not_2)
        begin
            aux_d_flip_flop_18_0_q <= 1'b0;
            aux_d_flip_flop_18_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_18_0_q <= aux_mux_17;
            aux_d_flip_flop_18_1_q <= ~aux_mux_17;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_19 = aux_d_flip_flop_20_0_q;
            1'd1: aux_mux_19 = datain;
            default: aux_mux_19 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_2)
    begin
        if (~aux_not_2)
        begin
            aux_d_flip_flop_20_0_q <= 1'b0;
            aux_d_flip_flop_20_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_20_0_q <= aux_mux_19;
            aux_d_flip_flop_20_1_q <= ~aux_mux_19;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_21 = aux_d_flip_flop_22_0_q;
            1'd1: aux_mux_21 = datain;
            default: aux_mux_21 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_2)
    begin
        if (~aux_not_2)
        begin
            aux_d_flip_flop_22_0_q <= 1'b0;
            aux_d_flip_flop_22_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_22_0_q <= aux_mux_21;
            aux_d_flip_flop_22_1_q <= ~aux_mux_21;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_23 = aux_d_flip_flop_24_0_q;
            1'd1: aux_mux_23 = datain;
            default: aux_mux_23 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_2)
    begin
        if (~aux_not_2)
        begin
            aux_d_flip_flop_24_0_q <= 1'b0;
            aux_d_flip_flop_24_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_24_0_q <= aux_mux_23;
            aux_d_flip_flop_24_1_q <= ~aux_mux_23;
        end
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_10})
            1'd0: aux_mux_25 = aux_d_flip_flop_26_0_q;
            1'd1: aux_mux_25 = datain;
            default: aux_mux_25 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_2)
    begin
        if (~aux_not_2)
        begin
            aux_d_flip_flop_26_0_q <= 1'b0;
            aux_d_flip_flop_26_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_26_0_q <= aux_mux_25;
            aux_d_flip_flop_26_1_q <= ~aux_mux_25;
        end
    end
    //End of D FlipFlop
level2_mux_8to1 level2_mux_8to1_inst_27 (
    .data0(aux_d_flip_flop_12_0_q),
    .data1(aux_d_flip_flop_14_0_q),
    .data2(aux_d_flip_flop_16_0_q),
    .data3(aux_d_flip_flop_18_0_q),
    .data4(aux_d_flip_flop_20_0_q),
    .data5(aux_d_flip_flop_22_0_q),
    .data6(aux_d_flip_flop_24_0_q),
    .data7(aux_d_flip_flop_26_0_q),
    .sel0(address0),
    .sel1(address1),
    .sel2(address2),
    .enable(1'b1),
    .p_output(w_level2_mux_8to1_inst_27_p_output)
);

assign dataout = w_level2_mux_8to1_inst_27_p_output;
endmodule

// Module for RAM_Low (generated from level6_ram_8x8.panda)
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

// Module for LEVEL9_MEMORY_STAGE_16BIT (generated from level9_memory_stage_16bit.panda)
module level9_memory_stage_16bit_ic (
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
    input datain8,
    input datain9,
    input datain10,
    input datain11,
    input datain12,
    input datain13,
    input datain14,
    input datain15,
    input memwrite,
    input result0,
    input result1,
    input result2,
    input result3,
    input result4,
    input result5,
    input result6,
    input result7,
    input result8,
    input result9,
    input result10,
    input result11,
    input result12,
    input result13,
    input result14,
    input result15,
    input clock,
    input reset,
    output dataout0,
    output dataout1,
    output dataout2,
    output dataout3,
    output dataout4,
    output dataout5,
    output dataout6,
    output dataout7,
    output dataout8,
    output dataout9,
    output dataout10,
    output dataout11,
    output dataout12,
    output dataout13,
    output dataout14,
    output dataout15
);

// IC instance: RAM_Low (level6_ram_8x8)
wire w_level6_ram_8x8_inst_1_dataout0;
wire w_level6_ram_8x8_inst_1_dataout1;
wire w_level6_ram_8x8_inst_1_dataout2;
wire w_level6_ram_8x8_inst_1_dataout3;
wire w_level6_ram_8x8_inst_1_dataout4;
wire w_level6_ram_8x8_inst_1_dataout5;
wire w_level6_ram_8x8_inst_1_dataout6;
wire w_level6_ram_8x8_inst_1_dataout7;
// IC instance: RAM_High (level6_ram_8x8)
wire w_level6_ram_8x8_inst_2_dataout0;
wire w_level6_ram_8x8_inst_2_dataout1;
wire w_level6_ram_8x8_inst_2_dataout2;
wire w_level6_ram_8x8_inst_2_dataout3;
wire w_level6_ram_8x8_inst_2_dataout4;
wire w_level6_ram_8x8_inst_2_dataout5;
wire w_level6_ram_8x8_inst_2_dataout6;
wire w_level6_ram_8x8_inst_2_dataout7;
reg aux_mux_3 = 1'b0;
reg aux_mux_4 = 1'b0;
reg aux_mux_5 = 1'b0;
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
level6_ram_8x8 level6_ram_8x8_inst_2 (
    .address0(address0),
    .address1(address1),
    .address2(address2),
    .datain0(datain8),
    .datain1(datain9),
    .datain2(datain10),
    .datain3(datain11),
    .datain4(datain12),
    .datain5(datain13),
    .datain6(datain14),
    .datain7(datain15),
    .writeenable(memwrite),
    .clock(clock),
    .reset(reset),
    .dataout0(w_level6_ram_8x8_inst_2_dataout0),
    .dataout1(w_level6_ram_8x8_inst_2_dataout1),
    .dataout2(w_level6_ram_8x8_inst_2_dataout2),
    .dataout3(w_level6_ram_8x8_inst_2_dataout3),
    .dataout4(w_level6_ram_8x8_inst_2_dataout4),
    .dataout5(w_level6_ram_8x8_inst_2_dataout5),
    .dataout6(w_level6_ram_8x8_inst_2_dataout6),
    .dataout7(w_level6_ram_8x8_inst_2_dataout7)
);
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_3 = result0;
            1'd1: aux_mux_3 = w_level6_ram_8x8_inst_1_dataout0;
            default: aux_mux_3 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_4 = result1;
            1'd1: aux_mux_4 = w_level6_ram_8x8_inst_1_dataout1;
            default: aux_mux_4 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_5 = result2;
            1'd1: aux_mux_5 = w_level6_ram_8x8_inst_1_dataout2;
            default: aux_mux_5 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_6 = result3;
            1'd1: aux_mux_6 = w_level6_ram_8x8_inst_1_dataout3;
            default: aux_mux_6 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_7 = result4;
            1'd1: aux_mux_7 = w_level6_ram_8x8_inst_1_dataout4;
            default: aux_mux_7 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_8 = result5;
            1'd1: aux_mux_8 = w_level6_ram_8x8_inst_1_dataout5;
            default: aux_mux_8 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_9 = result6;
            1'd1: aux_mux_9 = w_level6_ram_8x8_inst_1_dataout6;
            default: aux_mux_9 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_10 = result7;
            1'd1: aux_mux_10 = w_level6_ram_8x8_inst_1_dataout7;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_11 = result8;
            1'd1: aux_mux_11 = w_level6_ram_8x8_inst_2_dataout0;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_12 = result9;
            1'd1: aux_mux_12 = w_level6_ram_8x8_inst_2_dataout1;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_13 = result10;
            1'd1: aux_mux_13 = w_level6_ram_8x8_inst_2_dataout2;
            default: aux_mux_13 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_14 = result11;
            1'd1: aux_mux_14 = w_level6_ram_8x8_inst_2_dataout3;
            default: aux_mux_14 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_15 = result12;
            1'd1: aux_mux_15 = w_level6_ram_8x8_inst_2_dataout4;
            default: aux_mux_15 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_16 = result13;
            1'd1: aux_mux_16 = w_level6_ram_8x8_inst_2_dataout5;
            default: aux_mux_16 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_17 = result14;
            1'd1: aux_mux_17 = w_level6_ram_8x8_inst_2_dataout6;
            default: aux_mux_17 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({memread})
            1'd0: aux_mux_18 = result15;
            1'd1: aux_mux_18 = w_level6_ram_8x8_inst_2_dataout7;
            default: aux_mux_18 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign dataout0 = aux_mux_3;
assign dataout1 = aux_mux_4;
assign dataout2 = aux_mux_5;
assign dataout3 = aux_mux_6;
assign dataout4 = aux_mux_7;
assign dataout5 = aux_mux_8;
assign dataout6 = aux_mux_9;
assign dataout7 = aux_mux_10;
assign dataout8 = aux_mux_11;
assign dataout9 = aux_mux_12;
assign dataout10 = aux_mux_13;
assign dataout11 = aux_mux_14;
assign dataout12 = aux_mux_15;
assign dataout13 = aux_mux_16;
assign dataout14 = aux_mux_17;
assign dataout15 = aux_mux_18;
endmodule

module level9_memory_stage_16bit (
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
input input_switch32,
input input_switch33,
input input_switch34,
input input_switch35,
input input_switch36,
input input_switch37,
input input_switch38,
input input_switch39,
input input_switch40,
input input_switch41,
input input_switch42,
input input_switch43,
input input_switch44,

/* ========= Outputs ========== */
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
output led61_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL9_MEMORY_STAGE_16BIT (level9_memory_stage_16bit_ic)
wire w_level9_memory_stage_16bit_ic_inst_1_dataout0;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout1;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout2;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout3;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout4;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout5;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout6;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout7;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout8;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout9;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout10;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout11;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout12;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout13;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout14;
wire w_level9_memory_stage_16bit_ic_inst_1_dataout15;


// Assigning aux variables. //
level9_memory_stage_16bit_ic level9_memory_stage_16bit_ic_inst_1 (
    .address0(input_switch1),
    .address1(input_switch2),
    .address2(input_switch3),
    .address3(input_switch4),
    .address4(input_switch5),
    .address5(input_switch6),
    .address6(input_switch7),
    .address7(input_switch8),
    .memread(input_switch9),
    .datain0(input_switch10),
    .datain1(input_switch11),
    .datain2(input_switch12),
    .datain3(input_switch13),
    .datain4(input_switch14),
    .datain5(input_switch15),
    .datain6(input_switch16),
    .datain7(input_switch17),
    .datain8(input_switch18),
    .datain9(input_switch19),
    .datain10(input_switch20),
    .datain11(input_switch21),
    .datain12(input_switch22),
    .datain13(input_switch23),
    .datain14(input_switch24),
    .datain15(input_switch25),
    .memwrite(input_switch26),
    .result0(input_switch27),
    .result1(input_switch28),
    .result2(input_switch29),
    .result3(input_switch30),
    .result4(input_switch31),
    .result5(input_switch32),
    .result6(input_switch33),
    .result7(input_switch34),
    .result8(input_switch35),
    .result9(input_switch36),
    .result10(input_switch37),
    .result11(input_switch38),
    .result12(input_switch39),
    .result13(input_switch40),
    .result14(input_switch41),
    .result15(input_switch42),
    .clock(input_switch43),
    .reset(input_switch44),
    .dataout0(w_level9_memory_stage_16bit_ic_inst_1_dataout0),
    .dataout1(w_level9_memory_stage_16bit_ic_inst_1_dataout1),
    .dataout2(w_level9_memory_stage_16bit_ic_inst_1_dataout2),
    .dataout3(w_level9_memory_stage_16bit_ic_inst_1_dataout3),
    .dataout4(w_level9_memory_stage_16bit_ic_inst_1_dataout4),
    .dataout5(w_level9_memory_stage_16bit_ic_inst_1_dataout5),
    .dataout6(w_level9_memory_stage_16bit_ic_inst_1_dataout6),
    .dataout7(w_level9_memory_stage_16bit_ic_inst_1_dataout7),
    .dataout8(w_level9_memory_stage_16bit_ic_inst_1_dataout8),
    .dataout9(w_level9_memory_stage_16bit_ic_inst_1_dataout9),
    .dataout10(w_level9_memory_stage_16bit_ic_inst_1_dataout10),
    .dataout11(w_level9_memory_stage_16bit_ic_inst_1_dataout11),
    .dataout12(w_level9_memory_stage_16bit_ic_inst_1_dataout12),
    .dataout13(w_level9_memory_stage_16bit_ic_inst_1_dataout13),
    .dataout14(w_level9_memory_stage_16bit_ic_inst_1_dataout14),
    .dataout15(w_level9_memory_stage_16bit_ic_inst_1_dataout15)
);

// Writing output data. //
assign led46_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout0;
assign led47_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout1;
assign led48_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout2;
assign led49_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout3;
assign led50_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout4;
assign led51_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout5;
assign led52_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout6;
assign led53_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout7;
assign led54_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout8;
assign led55_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout9;
assign led56_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout10;
assign led57_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout11;
assign led58_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout12;
assign led59_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout13;
assign led60_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout14;
assign led61_1 = w_level9_memory_stage_16bit_ic_inst_1_dataout15;
endmodule
