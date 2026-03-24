// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Behavioral module for level1_d_flip_flop (generated from level1_d_flip_flop.panda)
module level1_d_flip_flop (
    input d,
    input clock,
    input preset,
    input clear,
    output reg q,
    output reg q_bar
);
    initial begin
        q = 1'b1;
        q_bar = 1'b0;
    end
    always @(posedge clock or negedge preset or negedge clear)
    begin
        if (~preset)
        begin
            q <= 1'b1;
            q_bar <= 1'b0;
        end
        else if (~clear)
        begin
            q <= 1'b0;
            q_bar <= 1'b1;
        end
        else
        begin
            q <= d;
            q_bar <= ~d;
        end
    end
endmodule

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

// Module for AddrMux[0] (generated from level2_mux_2to1.panda)
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

// Module for SPMux[0] (generated from level2_priority_mux_3to1.panda)
module level2_priority_mux_3to1 (
    input data0,
    input sel0,
    input data1,
    input sel1,
    input data2,
    output out
);

reg aux_mux_1 = 1'b0;
reg aux_mux_2 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel1})
            1'd0: aux_mux_1 = data2;
            1'd1: aux_mux_1 = data1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({sel0})
            1'd0: aux_mux_2 = aux_mux_1;
            1'd1: aux_mux_2 = data0;
            default: aux_mux_2 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign out = aux_mux_2;
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

// Module for StackRAM (generated from level6_ram_256x8.panda)
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

// Module for Adder (generated from level6_ripple_adder_8bit.panda)
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

// Module for StackPointer (generated from level6_stack_pointer_8bit.panda)
module level6_stack_pointer_8bit (
    input loadvalue0,
    input clock,
    input loadvalue1,
    input load,
    input loadvalue2,
    input push,
    input loadvalue3,
    input pop,
    input loadvalue4,
    input reset,
    input loadvalue5,
    input enable,
    input loadvalue6,
    input loadvalue7,
    output sp0,
    output sp1,
    output sp2,
    output sp3,
    output sp4,
    output sp5,
    output sp6,
    output sp7
);

// IC instance: Adder (level6_ripple_adder_8bit)
wire w_level6_ripple_adder_8bit_inst_1_sum0;
wire w_level6_ripple_adder_8bit_inst_1_sum1;
wire w_level6_ripple_adder_8bit_inst_1_sum2;
wire w_level6_ripple_adder_8bit_inst_1_sum3;
wire w_level6_ripple_adder_8bit_inst_1_sum4;
wire w_level6_ripple_adder_8bit_inst_1_sum5;
wire w_level6_ripple_adder_8bit_inst_1_sum6;
wire w_level6_ripple_adder_8bit_inst_1_sum7;
wire w_level6_ripple_adder_8bit_inst_1_carryout;
// IC instance: SP[0] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_2_q;
wire w_level1_d_flip_flop_inst_2_q_bar;
// IC instance: SP[1] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_3_q;
wire w_level1_d_flip_flop_inst_3_q_bar;
// IC instance: SP[2] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_4_q;
wire w_level1_d_flip_flop_inst_4_q_bar;
// IC instance: SP[3] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_5_q;
wire w_level1_d_flip_flop_inst_5_q_bar;
// IC instance: SP[4] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_6_q;
wire w_level1_d_flip_flop_inst_6_q_bar;
// IC instance: SP[5] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_7_q;
wire w_level1_d_flip_flop_inst_7_q_bar;
// IC instance: SP[6] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_8_q;
wire w_level1_d_flip_flop_inst_8_q_bar;
// IC instance: SP[7] (level1_d_flip_flop)
wire w_level1_d_flip_flop_inst_9_q;
wire w_level1_d_flip_flop_inst_9_q_bar;
// IC instance: SPMux[0] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_18_out;
// IC instance: SPMux[1] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_19_out;
// IC instance: SPMux[2] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_20_out;
// IC instance: SPMux[3] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_21_out;
// IC instance: SPMux[4] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_22_out;
// IC instance: SPMux[5] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_23_out;
// IC instance: SPMux[6] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_24_out;
// IC instance: SPMux[7] (level2_priority_mux_3to1)
wire w_level2_priority_mux_3to1_inst_25_out;

// Internal logic
level6_ripple_adder_8bit level6_ripple_adder_8bit_inst_1 (
    .a0(w_level1_d_flip_flop_inst_2_q),
    .b0(1'b0),
    .carryin(1'b0),
    .a1(w_level1_d_flip_flop_inst_3_q),
    .b1(1'b0),
    .a2(w_level1_d_flip_flop_inst_4_q),
    .b2(1'b0),
    .a3(w_level1_d_flip_flop_inst_5_q),
    .b3(1'b0),
    .a4(w_level1_d_flip_flop_inst_6_q),
    .b4(1'b0),
    .a5(w_level1_d_flip_flop_inst_7_q),
    .b5(1'b0),
    .a6(w_level1_d_flip_flop_inst_8_q),
    .b6(1'b0),
    .a7(w_level1_d_flip_flop_inst_9_q),
    .b7(1'b0),
    .sum0(w_level6_ripple_adder_8bit_inst_1_sum0),
    .sum1(w_level6_ripple_adder_8bit_inst_1_sum1),
    .sum2(w_level6_ripple_adder_8bit_inst_1_sum2),
    .sum3(w_level6_ripple_adder_8bit_inst_1_sum3),
    .sum4(w_level6_ripple_adder_8bit_inst_1_sum4),
    .sum5(w_level6_ripple_adder_8bit_inst_1_sum5),
    .sum6(w_level6_ripple_adder_8bit_inst_1_sum6),
    .sum7(w_level6_ripple_adder_8bit_inst_1_sum7),
    .carryout(w_level6_ripple_adder_8bit_inst_1_carryout)
);
level1_d_flip_flop level1_d_flip_flop_inst_2 (
    .d(w_level2_priority_mux_3to1_inst_18_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_2_q),
    .q_bar(w_level1_d_flip_flop_inst_2_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_3 (
    .d(w_level2_priority_mux_3to1_inst_19_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_3_q),
    .q_bar(w_level1_d_flip_flop_inst_3_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_4 (
    .d(w_level2_priority_mux_3to1_inst_20_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_4_q),
    .q_bar(w_level1_d_flip_flop_inst_4_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_5 (
    .d(w_level2_priority_mux_3to1_inst_21_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_5_q),
    .q_bar(w_level1_d_flip_flop_inst_5_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_6 (
    .d(w_level2_priority_mux_3to1_inst_22_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_6_q),
    .q_bar(w_level1_d_flip_flop_inst_6_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_7 (
    .d(w_level2_priority_mux_3to1_inst_23_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_7_q),
    .q_bar(w_level1_d_flip_flop_inst_7_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_8 (
    .d(w_level2_priority_mux_3to1_inst_24_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_8_q),
    .q_bar(w_level1_d_flip_flop_inst_8_q_bar)
);
level1_d_flip_flop level1_d_flip_flop_inst_9 (
    .d(w_level2_priority_mux_3to1_inst_25_out),
    .clock(clock),
    .preset(1'b1),
    .clear(1'b1),
    .q(w_level1_d_flip_flop_inst_9_q),
    .q_bar(w_level1_d_flip_flop_inst_9_q_bar)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_18 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue0),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum0),
    .out(w_level2_priority_mux_3to1_inst_18_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_19 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue1),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum1),
    .out(w_level2_priority_mux_3to1_inst_19_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_20 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue2),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum2),
    .out(w_level2_priority_mux_3to1_inst_20_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_21 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue3),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum3),
    .out(w_level2_priority_mux_3to1_inst_21_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_22 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue4),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum4),
    .out(w_level2_priority_mux_3to1_inst_22_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_23 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue5),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum5),
    .out(w_level2_priority_mux_3to1_inst_23_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_24 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue6),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum6),
    .out(w_level2_priority_mux_3to1_inst_24_out)
);
level2_priority_mux_3to1 level2_priority_mux_3to1_inst_25 (
    .data0(1'b1),
    .sel0(reset),
    .data1(loadvalue7),
    .sel1(load),
    .data2(w_level6_ripple_adder_8bit_inst_1_sum7),
    .out(w_level2_priority_mux_3to1_inst_25_out)
);

assign sp0 = w_level1_d_flip_flop_inst_2_q;
assign sp1 = w_level1_d_flip_flop_inst_3_q;
assign sp2 = w_level1_d_flip_flop_inst_4_q;
assign sp3 = w_level1_d_flip_flop_inst_5_q;
assign sp4 = w_level1_d_flip_flop_inst_6_q;
assign sp5 = w_level1_d_flip_flop_inst_7_q;
assign sp6 = w_level1_d_flip_flop_inst_8_q;
assign sp7 = w_level1_d_flip_flop_inst_9_q;
endmodule

// Module for LEVEL6_STACK_MEMORY_INTERFACE (generated from level6_stack_memory_interface.panda)
module level6_stack_memory_interface_ic (
    input sp_loadvalue0,
    input sp_loadvalue1,
    input sp_loadvalue2,
    input sp_loadvalue3,
    input sp_loadvalue4,
    input sp_loadvalue5,
    input sp_loadvalue6,
    input sp_loadvalue7,
    input sp_load,
    input sp_push,
    input sp_pop,
    input sp_reset,
    input address0,
    input address1,
    input address2,
    input address3,
    input address4,
    input address5,
    input address6,
    input address7,
    input addressselect,
    input datain0,
    input datain1,
    input datain2,
    input datain3,
    input datain4,
    input datain5,
    input datain6,
    input datain7,
    input memwrite,
    input memread,
    input clock,
    input enable,
    output sp0,
    output sp1,
    output sp2,
    output sp3,
    output sp4,
    output sp5,
    output sp6,
    output sp7,
    output dataout0,
    output dataout1,
    output dataout2,
    output dataout3,
    output dataout4,
    output dataout5,
    output dataout6,
    output dataout7,
    output finaladdress0,
    output finaladdress1,
    output finaladdress2,
    output finaladdress3,
    output finaladdress4,
    output finaladdress5,
    output finaladdress6,
    output finaladdress7
);

// IC instance: StackPointer (level6_stack_pointer_8bit)
wire w_level6_stack_pointer_8bit_inst_1_sp0;
wire w_level6_stack_pointer_8bit_inst_1_sp1;
wire w_level6_stack_pointer_8bit_inst_1_sp2;
wire w_level6_stack_pointer_8bit_inst_1_sp3;
wire w_level6_stack_pointer_8bit_inst_1_sp4;
wire w_level6_stack_pointer_8bit_inst_1_sp5;
wire w_level6_stack_pointer_8bit_inst_1_sp6;
wire w_level6_stack_pointer_8bit_inst_1_sp7;
// IC instance: StackRAM (level6_ram_256x8)
wire w_level6_ram_256x8_inst_2_dataout0;
wire w_level6_ram_256x8_inst_2_dataout1;
wire w_level6_ram_256x8_inst_2_dataout2;
wire w_level6_ram_256x8_inst_2_dataout3;
wire w_level6_ram_256x8_inst_2_dataout4;
wire w_level6_ram_256x8_inst_2_dataout5;
wire w_level6_ram_256x8_inst_2_dataout6;
wire w_level6_ram_256x8_inst_2_dataout7;
// IC instance: AddrMux[0] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_3_p_output;
// IC instance: AddrMux[1] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_4_p_output;
// IC instance: AddrMux[2] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_5_p_output;
// IC instance: AddrMux[3] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_6_p_output;
// IC instance: AddrMux[4] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_7_p_output;
// IC instance: AddrMux[5] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_8_p_output;
// IC instance: AddrMux[6] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_9_p_output;
// IC instance: AddrMux[7] (level2_mux_2to1)
wire w_level2_mux_2to1_inst_10_p_output;

// Internal logic
level6_stack_pointer_8bit level6_stack_pointer_8bit_inst_1 (
    .loadvalue0(sp_loadvalue0),
    .clock(clock),
    .loadvalue1(sp_loadvalue1),
    .load(sp_load),
    .loadvalue2(sp_loadvalue2),
    .push(sp_push),
    .loadvalue3(sp_loadvalue3),
    .pop(sp_pop),
    .loadvalue4(sp_loadvalue4),
    .reset(sp_reset),
    .loadvalue5(sp_loadvalue5),
    .enable(1'b0),
    .loadvalue6(sp_loadvalue6),
    .loadvalue7(sp_loadvalue7),
    .sp0(w_level6_stack_pointer_8bit_inst_1_sp0),
    .sp1(w_level6_stack_pointer_8bit_inst_1_sp1),
    .sp2(w_level6_stack_pointer_8bit_inst_1_sp2),
    .sp3(w_level6_stack_pointer_8bit_inst_1_sp3),
    .sp4(w_level6_stack_pointer_8bit_inst_1_sp4),
    .sp5(w_level6_stack_pointer_8bit_inst_1_sp5),
    .sp6(w_level6_stack_pointer_8bit_inst_1_sp6),
    .sp7(w_level6_stack_pointer_8bit_inst_1_sp7)
);
level6_ram_256x8 level6_ram_256x8_inst_2 (
    .address0(w_level2_mux_2to1_inst_3_p_output),
    .address1(w_level2_mux_2to1_inst_4_p_output),
    .address2(w_level2_mux_2to1_inst_5_p_output),
    .address3(w_level2_mux_2to1_inst_6_p_output),
    .address4(w_level2_mux_2to1_inst_7_p_output),
    .address5(w_level2_mux_2to1_inst_8_p_output),
    .address6(w_level2_mux_2to1_inst_9_p_output),
    .address7(w_level2_mux_2to1_inst_10_p_output),
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
    .dataout0(w_level6_ram_256x8_inst_2_dataout0),
    .dataout1(w_level6_ram_256x8_inst_2_dataout1),
    .dataout2(w_level6_ram_256x8_inst_2_dataout2),
    .dataout3(w_level6_ram_256x8_inst_2_dataout3),
    .dataout4(w_level6_ram_256x8_inst_2_dataout4),
    .dataout5(w_level6_ram_256x8_inst_2_dataout5),
    .dataout6(w_level6_ram_256x8_inst_2_dataout6),
    .dataout7(w_level6_ram_256x8_inst_2_dataout7)
);
level2_mux_2to1 level2_mux_2to1_inst_3 (
    .data0(address0),
    .data1(w_level6_stack_pointer_8bit_inst_1_sp0),
    .sel0(addressselect),
    .p_output(w_level2_mux_2to1_inst_3_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_4 (
    .data0(address1),
    .data1(w_level6_stack_pointer_8bit_inst_1_sp1),
    .sel0(addressselect),
    .p_output(w_level2_mux_2to1_inst_4_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_5 (
    .data0(address2),
    .data1(w_level6_stack_pointer_8bit_inst_1_sp2),
    .sel0(addressselect),
    .p_output(w_level2_mux_2to1_inst_5_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_6 (
    .data0(address3),
    .data1(w_level6_stack_pointer_8bit_inst_1_sp3),
    .sel0(addressselect),
    .p_output(w_level2_mux_2to1_inst_6_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_7 (
    .data0(address4),
    .data1(w_level6_stack_pointer_8bit_inst_1_sp4),
    .sel0(addressselect),
    .p_output(w_level2_mux_2to1_inst_7_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_8 (
    .data0(address5),
    .data1(w_level6_stack_pointer_8bit_inst_1_sp5),
    .sel0(addressselect),
    .p_output(w_level2_mux_2to1_inst_8_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_9 (
    .data0(address6),
    .data1(w_level6_stack_pointer_8bit_inst_1_sp6),
    .sel0(addressselect),
    .p_output(w_level2_mux_2to1_inst_9_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_10 (
    .data0(address7),
    .data1(w_level6_stack_pointer_8bit_inst_1_sp7),
    .sel0(addressselect),
    .p_output(w_level2_mux_2to1_inst_10_p_output)
);

assign sp0 = w_level6_stack_pointer_8bit_inst_1_sp0;
assign sp1 = w_level6_stack_pointer_8bit_inst_1_sp1;
assign sp2 = w_level6_stack_pointer_8bit_inst_1_sp2;
assign sp3 = w_level6_stack_pointer_8bit_inst_1_sp3;
assign sp4 = w_level6_stack_pointer_8bit_inst_1_sp4;
assign sp5 = w_level6_stack_pointer_8bit_inst_1_sp5;
assign sp6 = w_level6_stack_pointer_8bit_inst_1_sp6;
assign sp7 = w_level6_stack_pointer_8bit_inst_1_sp7;
assign dataout0 = w_level6_ram_256x8_inst_2_dataout0;
assign dataout1 = w_level6_ram_256x8_inst_2_dataout1;
assign dataout2 = w_level6_ram_256x8_inst_2_dataout2;
assign dataout3 = w_level6_ram_256x8_inst_2_dataout3;
assign dataout4 = w_level6_ram_256x8_inst_2_dataout4;
assign dataout5 = w_level6_ram_256x8_inst_2_dataout5;
assign dataout6 = w_level6_ram_256x8_inst_2_dataout6;
assign dataout7 = w_level6_ram_256x8_inst_2_dataout7;
assign finaladdress0 = w_level2_mux_2to1_inst_3_p_output;
assign finaladdress1 = w_level2_mux_2to1_inst_4_p_output;
assign finaladdress2 = w_level2_mux_2to1_inst_5_p_output;
assign finaladdress3 = w_level2_mux_2to1_inst_6_p_output;
assign finaladdress4 = w_level2_mux_2to1_inst_7_p_output;
assign finaladdress5 = w_level2_mux_2to1_inst_8_p_output;
assign finaladdress6 = w_level2_mux_2to1_inst_9_p_output;
assign finaladdress7 = w_level2_mux_2to1_inst_10_p_output;
endmodule

module level6_stack_memory_interface (
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

/* ========= Outputs ========== */
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
// IC instance: LEVEL6_STACK_MEMORY_INTERFACE (level6_stack_memory_interface_ic)
wire w_level6_stack_memory_interface_ic_inst_1_sp0;
wire w_level6_stack_memory_interface_ic_inst_1_sp1;
wire w_level6_stack_memory_interface_ic_inst_1_sp2;
wire w_level6_stack_memory_interface_ic_inst_1_sp3;
wire w_level6_stack_memory_interface_ic_inst_1_sp4;
wire w_level6_stack_memory_interface_ic_inst_1_sp5;
wire w_level6_stack_memory_interface_ic_inst_1_sp6;
wire w_level6_stack_memory_interface_ic_inst_1_sp7;
wire w_level6_stack_memory_interface_ic_inst_1_dataout0;
wire w_level6_stack_memory_interface_ic_inst_1_dataout1;
wire w_level6_stack_memory_interface_ic_inst_1_dataout2;
wire w_level6_stack_memory_interface_ic_inst_1_dataout3;
wire w_level6_stack_memory_interface_ic_inst_1_dataout4;
wire w_level6_stack_memory_interface_ic_inst_1_dataout5;
wire w_level6_stack_memory_interface_ic_inst_1_dataout6;
wire w_level6_stack_memory_interface_ic_inst_1_dataout7;
wire w_level6_stack_memory_interface_ic_inst_1_finaladdress0;
wire w_level6_stack_memory_interface_ic_inst_1_finaladdress1;
wire w_level6_stack_memory_interface_ic_inst_1_finaladdress2;
wire w_level6_stack_memory_interface_ic_inst_1_finaladdress3;
wire w_level6_stack_memory_interface_ic_inst_1_finaladdress4;
wire w_level6_stack_memory_interface_ic_inst_1_finaladdress5;
wire w_level6_stack_memory_interface_ic_inst_1_finaladdress6;
wire w_level6_stack_memory_interface_ic_inst_1_finaladdress7;


// Assigning aux variables. //
level6_stack_memory_interface_ic level6_stack_memory_interface_ic_inst_1 (
    .sp_loadvalue0(input_switch1),
    .sp_loadvalue1(input_switch2),
    .sp_loadvalue2(input_switch3),
    .sp_loadvalue3(input_switch4),
    .sp_loadvalue4(input_switch5),
    .sp_loadvalue5(input_switch6),
    .sp_loadvalue6(input_switch7),
    .sp_loadvalue7(input_switch8),
    .sp_load(input_switch9),
    .sp_push(input_switch10),
    .sp_pop(input_switch11),
    .sp_reset(input_switch12),
    .address0(input_switch13),
    .address1(input_switch14),
    .address2(input_switch15),
    .address3(input_switch16),
    .address4(input_switch17),
    .address5(input_switch18),
    .address6(input_switch19),
    .address7(input_switch20),
    .addressselect(input_switch21),
    .datain0(input_switch22),
    .datain1(input_switch23),
    .datain2(input_switch24),
    .datain3(input_switch25),
    .datain4(input_switch26),
    .datain5(input_switch27),
    .datain6(input_switch28),
    .datain7(input_switch29),
    .memwrite(input_switch30),
    .memread(input_switch31),
    .clock(input_switch32),
    .enable(input_switch33),
    .sp0(w_level6_stack_memory_interface_ic_inst_1_sp0),
    .sp1(w_level6_stack_memory_interface_ic_inst_1_sp1),
    .sp2(w_level6_stack_memory_interface_ic_inst_1_sp2),
    .sp3(w_level6_stack_memory_interface_ic_inst_1_sp3),
    .sp4(w_level6_stack_memory_interface_ic_inst_1_sp4),
    .sp5(w_level6_stack_memory_interface_ic_inst_1_sp5),
    .sp6(w_level6_stack_memory_interface_ic_inst_1_sp6),
    .sp7(w_level6_stack_memory_interface_ic_inst_1_sp7),
    .dataout0(w_level6_stack_memory_interface_ic_inst_1_dataout0),
    .dataout1(w_level6_stack_memory_interface_ic_inst_1_dataout1),
    .dataout2(w_level6_stack_memory_interface_ic_inst_1_dataout2),
    .dataout3(w_level6_stack_memory_interface_ic_inst_1_dataout3),
    .dataout4(w_level6_stack_memory_interface_ic_inst_1_dataout4),
    .dataout5(w_level6_stack_memory_interface_ic_inst_1_dataout5),
    .dataout6(w_level6_stack_memory_interface_ic_inst_1_dataout6),
    .dataout7(w_level6_stack_memory_interface_ic_inst_1_dataout7),
    .finaladdress0(w_level6_stack_memory_interface_ic_inst_1_finaladdress0),
    .finaladdress1(w_level6_stack_memory_interface_ic_inst_1_finaladdress1),
    .finaladdress2(w_level6_stack_memory_interface_ic_inst_1_finaladdress2),
    .finaladdress3(w_level6_stack_memory_interface_ic_inst_1_finaladdress3),
    .finaladdress4(w_level6_stack_memory_interface_ic_inst_1_finaladdress4),
    .finaladdress5(w_level6_stack_memory_interface_ic_inst_1_finaladdress5),
    .finaladdress6(w_level6_stack_memory_interface_ic_inst_1_finaladdress6),
    .finaladdress7(w_level6_stack_memory_interface_ic_inst_1_finaladdress7)
);

// Writing output data. //
assign led35_1 = w_level6_stack_memory_interface_ic_inst_1_sp0;
assign led36_1 = w_level6_stack_memory_interface_ic_inst_1_sp1;
assign led37_1 = w_level6_stack_memory_interface_ic_inst_1_sp2;
assign led38_1 = w_level6_stack_memory_interface_ic_inst_1_sp3;
assign led39_1 = w_level6_stack_memory_interface_ic_inst_1_sp4;
assign led40_1 = w_level6_stack_memory_interface_ic_inst_1_sp5;
assign led41_1 = w_level6_stack_memory_interface_ic_inst_1_sp6;
assign led42_1 = w_level6_stack_memory_interface_ic_inst_1_sp7;
assign led43_1 = w_level6_stack_memory_interface_ic_inst_1_dataout0;
assign led44_1 = w_level6_stack_memory_interface_ic_inst_1_dataout1;
assign led45_1 = w_level6_stack_memory_interface_ic_inst_1_dataout2;
assign led46_1 = w_level6_stack_memory_interface_ic_inst_1_dataout3;
assign led47_1 = w_level6_stack_memory_interface_ic_inst_1_dataout4;
assign led48_1 = w_level6_stack_memory_interface_ic_inst_1_dataout5;
assign led49_1 = w_level6_stack_memory_interface_ic_inst_1_dataout6;
assign led50_1 = w_level6_stack_memory_interface_ic_inst_1_dataout7;
assign led51_1 = w_level6_stack_memory_interface_ic_inst_1_finaladdress0;
assign led52_1 = w_level6_stack_memory_interface_ic_inst_1_finaladdress1;
assign led53_1 = w_level6_stack_memory_interface_ic_inst_1_finaladdress2;
assign led54_1 = w_level6_stack_memory_interface_ic_inst_1_finaladdress3;
assign led55_1 = w_level6_stack_memory_interface_ic_inst_1_finaladdress4;
assign led56_1 = w_level6_stack_memory_interface_ic_inst_1_finaladdress5;
assign led57_1 = w_level6_stack_memory_interface_ic_inst_1_finaladdress6;
assign led58_1 = w_level6_stack_memory_interface_ic_inst_1_finaladdress7;
endmodule
