// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for AddrDecoder (generated from level2_decoder_2to4.panda)
module level2_decoder_2to4 (
    input addr0,
    input addr1,
    output out0,
    output out1,
    output out2,
    output out3
);

wire aux_not_1;
wire aux_not_2;
wire aux_and_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;

// Internal logic
assign aux_not_1 = ~addr0;
assign aux_not_2 = ~addr1;
assign aux_and_3 = (aux_not_1 & aux_not_2);
assign aux_and_4 = (addr0 & aux_not_2);
assign aux_and_5 = (aux_not_1 & addr1);
assign aux_and_6 = (addr0 & addr1);

assign out0 = aux_and_3;
assign out1 = aux_and_4;
assign out2 = aux_and_5;
assign out3 = aux_and_6;
endmodule

// Module for ReadMux (generated from level2_mux_4to1.panda)
module level2_mux_4to1 (
    input data0,
    input data1,
    input data2,
    input data3,
    input sel0,
    input sel1,
    output p_output
);

reg aux_mux_1 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel1, sel0})
            2'd0: aux_mux_1 = data0;
            2'd1: aux_mux_1 = data1;
            2'd2: aux_mux_1 = data2;
            2'd3: aux_mux_1 = data3;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign p_output = aux_mux_1;
endmodule

// Module for LEVEL4_RAM_4X1 (generated from level4_ram_4x1.panda)
module level4_ram_4x1_ic (
    input address0,
    input address1,
    input datain,
    input writeenable,
    input clock,
    output dataout
);

wire aux_and_1;
wire aux_and_2;
wire aux_and_3;
wire aux_and_4;
reg aux_mux_5 = 1'b0;
reg aux_d_flip_flop_6_0_q = 1'b0;
reg aux_d_flip_flop_6_1_q = 1'b1;
reg aux_mux_7 = 1'b0;
reg aux_d_flip_flop_8_0_q = 1'b0;
reg aux_d_flip_flop_8_1_q = 1'b1;
reg aux_mux_9 = 1'b0;
reg aux_d_flip_flop_10_0_q = 1'b0;
reg aux_d_flip_flop_10_1_q = 1'b1;
reg aux_mux_11 = 1'b0;
reg aux_d_flip_flop_12_0_q = 1'b0;
reg aux_d_flip_flop_12_1_q = 1'b1;
// IC instance: AddrDecoder (level2_decoder_2to4)
wire w_level2_decoder_2to4_inst_13_out0;
wire w_level2_decoder_2to4_inst_13_out1;
wire w_level2_decoder_2to4_inst_13_out2;
wire w_level2_decoder_2to4_inst_13_out3;
// IC instance: ReadMux (level2_mux_4to1)
wire w_level2_mux_4to1_inst_14_p_output;

// Internal logic
assign aux_and_1 = (w_level2_decoder_2to4_inst_13_out0 & writeenable);
assign aux_and_2 = (w_level2_decoder_2to4_inst_13_out1 & writeenable);
assign aux_and_3 = (w_level2_decoder_2to4_inst_13_out2 & writeenable);
assign aux_and_4 = (w_level2_decoder_2to4_inst_13_out3 & writeenable);
    //Multiplexer
    always @(*)
    begin
        case({aux_and_1})
            1'd0: aux_mux_5 = aux_d_flip_flop_6_0_q;
            1'd1: aux_mux_5 = datain;
            default: aux_mux_5 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_6_0_q <= aux_mux_5;
            aux_d_flip_flop_6_1_q <= ~aux_mux_5;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_7 = aux_d_flip_flop_8_0_q;
            1'd1: aux_mux_7 = datain;
            default: aux_mux_7 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_8_0_q <= aux_mux_7;
            aux_d_flip_flop_8_1_q <= ~aux_mux_7;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
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
        case({aux_and_4})
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
level2_decoder_2to4 level2_decoder_2to4_inst_13 (
    .addr0(address0),
    .addr1(address1),
    .out0(w_level2_decoder_2to4_inst_13_out0),
    .out1(w_level2_decoder_2to4_inst_13_out1),
    .out2(w_level2_decoder_2to4_inst_13_out2),
    .out3(w_level2_decoder_2to4_inst_13_out3)
);
level2_mux_4to1 level2_mux_4to1_inst_14 (
    .data0(aux_d_flip_flop_6_0_q),
    .data1(aux_d_flip_flop_8_0_q),
    .data2(aux_d_flip_flop_10_0_q),
    .data3(aux_d_flip_flop_12_0_q),
    .sel0(address0),
    .sel1(address1),
    .p_output(w_level2_mux_4to1_inst_14_p_output)
);

assign dataout = w_level2_mux_4to1_inst_14_p_output;
endmodule

module level4_ram_4x1 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,

/* ========= Outputs ========== */
output led7_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL4_RAM_4X1 (level4_ram_4x1_ic)
wire w_level4_ram_4x1_ic_inst_1_dataout;


// Assigning aux variables. //
level4_ram_4x1_ic level4_ram_4x1_ic_inst_1 (
    .address0(input_switch1),
    .address1(input_switch2),
    .datain(input_switch3),
    .writeenable(input_switch4),
    .clock(input_switch5),
    .dataout(w_level4_ram_4x1_ic_inst_1_dataout)
);

// Writing output data. //
assign led7_1 = w_level4_ram_4x1_ic_inst_1_dataout;
endmodule
