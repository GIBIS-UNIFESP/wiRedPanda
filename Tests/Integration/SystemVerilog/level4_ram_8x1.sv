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

// Module for LEVEL4_RAM_8X1 (generated from level4_ram_8x1.panda)
module level4_ram_8x1_ic (
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

module level4_ram_8x1 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,

/* ========= Outputs ========== */
output led8_1
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
    .dataout(w_level4_ram_8x1_ic_inst_1_dataout)
);

// Writing output data. //
assign led8_1 = w_level4_ram_8x1_ic_inst_1_dataout;
endmodule
