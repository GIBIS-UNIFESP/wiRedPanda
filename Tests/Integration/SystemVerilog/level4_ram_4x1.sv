// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for AddrDecoder (generated from level2_decoder_2to4.panda)
module level2_decoder_2to4 (
    input addr0,
    input addr1,
    input enable,
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
assign aux_and_3 = (aux_not_1 & aux_not_2 & enable);
assign aux_and_4 = (addr0 & aux_not_2 & enable);
assign aux_and_5 = (aux_not_1 & addr1 & enable);
assign aux_and_6 = (addr0 & addr1 & enable);

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
    input enable,
    output p_output
);

reg aux_mux_1 = 1'b0;
wire aux_and_2;

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
assign aux_and_2 = (aux_mux_1 & enable);

assign p_output = aux_and_2;
endmodule

// Module for LEVEL4_RAM_4X1 (generated from level4_ram_4x1.panda)
module level4_ram_4x1_ic (
    input address0,
    input address1,
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
// IC instance: AddrDecoder (level2_decoder_2to4)
wire w_level2_decoder_2to4_inst_10_out0;
wire w_level2_decoder_2to4_inst_10_out1;
wire w_level2_decoder_2to4_inst_10_out2;
wire w_level2_decoder_2to4_inst_10_out3;
// IC instance: ReadMux (level2_mux_4to1)
wire w_level2_mux_4to1_inst_11_p_output;

// Internal logic
assign aux_not_1 = ~reset;
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_10_out0})
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
        case({w_level2_decoder_2to4_inst_10_out1})
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
        case({w_level2_decoder_2to4_inst_10_out2})
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
        case({w_level2_decoder_2to4_inst_10_out3})
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
level2_decoder_2to4 level2_decoder_2to4_inst_10 (
    .addr0(address0),
    .addr1(address1),
    .enable(writeenable),
    .out0(w_level2_decoder_2to4_inst_10_out0),
    .out1(w_level2_decoder_2to4_inst_10_out1),
    .out2(w_level2_decoder_2to4_inst_10_out2),
    .out3(w_level2_decoder_2to4_inst_10_out3)
);
level2_mux_4to1 level2_mux_4to1_inst_11 (
    .data0(aux_d_flip_flop_3_0_q),
    .data1(aux_d_flip_flop_5_0_q),
    .data2(aux_d_flip_flop_7_0_q),
    .data3(aux_d_flip_flop_9_0_q),
    .sel0(address0),
    .sel1(address1),
    .enable(1'b1),
    .p_output(w_level2_mux_4to1_inst_11_p_output)
);

assign dataout = w_level2_mux_4to1_inst_11_p_output;
endmodule

module level4_ram_4x1 (
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
// IC instance: LEVEL4_RAM_4X1 (level4_ram_4x1_ic)
wire w_level4_ram_4x1_ic_inst_1_dataout;


// Assigning aux variables. //
level4_ram_4x1_ic level4_ram_4x1_ic_inst_1 (
    .address0(input_switch1),
    .address1(input_switch2),
    .datain(input_switch3),
    .writeenable(input_switch4),
    .clock(input_switch5),
    .reset(input_switch6),
    .dataout(w_level4_ram_4x1_ic_inst_1_dataout)
);

// Writing output data. //
assign led8_1 = w_level4_ram_4x1_ic_inst_1_dataout;
endmodule
