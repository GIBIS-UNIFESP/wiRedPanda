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

// Module for LEVEL4_RAM_8X1 (generated from level4_ram_8x1.panda)
module level4_ram_8x1_ic (
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

module level4_ram_8x1 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,

/* ========= Outputs ========== */
output led9_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_RAM_8X1 (level4_ram_8x1_ic)
wire w_level4_ram_8x1_ic_inst_1_dataout;


// Assigning aux variables. //
level4_ram_8x1_ic level4_ram_8x1_ic_inst_1 (
    .address0(input_switch1),
    .address1(input_switch2),
    .address2(input_switch3),
    .datain(input_switch4),
    .writeenable(input_switch5),
    .clock(input_switch6),
    .reset(input_switch7),
    .dataout(w_level4_ram_8x1_ic_inst_1_dataout)
);

// Writing output data. //
assign led9_1 = w_level4_ram_8x1_ic_inst_1_dataout;
endmodule
