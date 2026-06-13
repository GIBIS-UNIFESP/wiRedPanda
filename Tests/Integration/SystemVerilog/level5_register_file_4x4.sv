// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Write_Decoder (generated from level2_decoder_2to4.panda)
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

// Module for LEVEL5_REGISTER_FILE_4X4 (generated from level5_register_file_4x4.panda)
module level5_register_file_4x4_ic (
    input write_addr0,
    input write_addr1,
    input read_addr10,
    input read_addr11,
    input read_addr20,
    input read_addr21,
    input data_in0,
    input data_in1,
    input data_in2,
    input data_in3,
    input write_enable,
    input clock,
    output read_data10,
    output read_data11,
    output read_data12,
    output read_data13,
    output read_data20,
    output read_data21,
    output read_data22,
    output read_data23
);

// IC instance: Write_Decoder (level2_decoder_2to4)
wire w_level2_decoder_2to4_inst_1_out0;
wire w_level2_decoder_2to4_inst_1_out1;
wire w_level2_decoder_2to4_inst_1_out2;
wire w_level2_decoder_2to4_inst_1_out3;
reg aux_d_flip_flop_2_0_q = 1'b0;
reg aux_d_flip_flop_2_1_q = 1'b1;
reg aux_d_flip_flop_3_0_q = 1'b0;
reg aux_d_flip_flop_3_1_q = 1'b1;
reg aux_d_flip_flop_4_0_q = 1'b0;
reg aux_d_flip_flop_4_1_q = 1'b1;
reg aux_d_flip_flop_5_0_q = 1'b0;
reg aux_d_flip_flop_5_1_q = 1'b1;
reg aux_mux_6 = 1'b0;
reg aux_mux_7 = 1'b0;
reg aux_mux_8 = 1'b0;
reg aux_mux_9 = 1'b0;
reg aux_d_flip_flop_10_0_q = 1'b0;
reg aux_d_flip_flop_10_1_q = 1'b1;
reg aux_d_flip_flop_11_0_q = 1'b0;
reg aux_d_flip_flop_11_1_q = 1'b1;
reg aux_d_flip_flop_12_0_q = 1'b0;
reg aux_d_flip_flop_12_1_q = 1'b1;
reg aux_d_flip_flop_13_0_q = 1'b0;
reg aux_d_flip_flop_13_1_q = 1'b1;
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
reg aux_mux_22 = 1'b0;
reg aux_mux_23 = 1'b0;
reg aux_mux_24 = 1'b0;
reg aux_mux_25 = 1'b0;
reg aux_d_flip_flop_26_0_q = 1'b0;
reg aux_d_flip_flop_26_1_q = 1'b1;
reg aux_d_flip_flop_27_0_q = 1'b0;
reg aux_d_flip_flop_27_1_q = 1'b1;
reg aux_d_flip_flop_28_0_q = 1'b0;
reg aux_d_flip_flop_28_1_q = 1'b1;
reg aux_d_flip_flop_29_0_q = 1'b0;
reg aux_d_flip_flop_29_1_q = 1'b1;
reg aux_mux_30 = 1'b0;
reg aux_mux_31 = 1'b0;
reg aux_mux_32 = 1'b0;
reg aux_mux_33 = 1'b0;
reg aux_mux_34 = 1'b0;
reg aux_mux_35 = 1'b0;
reg aux_mux_36 = 1'b0;
reg aux_mux_37 = 1'b0;
reg aux_mux_38 = 1'b0;
reg aux_mux_39 = 1'b0;
reg aux_mux_40 = 1'b0;
reg aux_mux_41 = 1'b0;

// Internal logic
level2_decoder_2to4 level2_decoder_2to4_inst_1 (
    .addr0(write_addr0),
    .addr1(write_addr1),
    .enable(write_enable),
    .out0(w_level2_decoder_2to4_inst_1_out0),
    .out1(w_level2_decoder_2to4_inst_1_out1),
    .out2(w_level2_decoder_2to4_inst_1_out2),
    .out3(w_level2_decoder_2to4_inst_1_out3)
);
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_2_0_q <= aux_mux_6;
            aux_d_flip_flop_2_1_q <= ~aux_mux_6;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_3_0_q <= aux_mux_7;
            aux_d_flip_flop_3_1_q <= ~aux_mux_7;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_4_0_q <= aux_mux_8;
            aux_d_flip_flop_4_1_q <= ~aux_mux_8;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_5_0_q <= aux_mux_9;
            aux_d_flip_flop_5_1_q <= ~aux_mux_9;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out0})
            1'd0: aux_mux_6 = aux_d_flip_flop_2_0_q;
            1'd1: aux_mux_6 = data_in0;
            default: aux_mux_6 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out0})
            1'd0: aux_mux_7 = aux_d_flip_flop_3_0_q;
            1'd1: aux_mux_7 = data_in1;
            default: aux_mux_7 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out0})
            1'd0: aux_mux_8 = aux_d_flip_flop_4_0_q;
            1'd1: aux_mux_8 = data_in2;
            default: aux_mux_8 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out0})
            1'd0: aux_mux_9 = aux_d_flip_flop_5_0_q;
            1'd1: aux_mux_9 = data_in3;
            default: aux_mux_9 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_10_0_q <= aux_mux_14;
            aux_d_flip_flop_10_1_q <= ~aux_mux_14;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_11_0_q <= aux_mux_15;
            aux_d_flip_flop_11_1_q <= ~aux_mux_15;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_12_0_q <= aux_mux_16;
            aux_d_flip_flop_12_1_q <= ~aux_mux_16;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_13_0_q <= aux_mux_17;
            aux_d_flip_flop_13_1_q <= ~aux_mux_17;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out1})
            1'd0: aux_mux_14 = aux_d_flip_flop_10_0_q;
            1'd1: aux_mux_14 = data_in0;
            default: aux_mux_14 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out1})
            1'd0: aux_mux_15 = aux_d_flip_flop_11_0_q;
            1'd1: aux_mux_15 = data_in1;
            default: aux_mux_15 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out1})
            1'd0: aux_mux_16 = aux_d_flip_flop_12_0_q;
            1'd1: aux_mux_16 = data_in2;
            default: aux_mux_16 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out1})
            1'd0: aux_mux_17 = aux_d_flip_flop_13_0_q;
            1'd1: aux_mux_17 = data_in3;
            default: aux_mux_17 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_18_0_q <= aux_mux_22;
            aux_d_flip_flop_18_1_q <= ~aux_mux_22;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_19_0_q <= aux_mux_23;
            aux_d_flip_flop_19_1_q <= ~aux_mux_23;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_20_0_q <= aux_mux_24;
            aux_d_flip_flop_20_1_q <= ~aux_mux_24;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_21_0_q <= aux_mux_25;
            aux_d_flip_flop_21_1_q <= ~aux_mux_25;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out2})
            1'd0: aux_mux_22 = aux_d_flip_flop_18_0_q;
            1'd1: aux_mux_22 = data_in0;
            default: aux_mux_22 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out2})
            1'd0: aux_mux_23 = aux_d_flip_flop_19_0_q;
            1'd1: aux_mux_23 = data_in1;
            default: aux_mux_23 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out2})
            1'd0: aux_mux_24 = aux_d_flip_flop_20_0_q;
            1'd1: aux_mux_24 = data_in2;
            default: aux_mux_24 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out2})
            1'd0: aux_mux_25 = aux_d_flip_flop_21_0_q;
            1'd1: aux_mux_25 = data_in3;
            default: aux_mux_25 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_26_0_q <= aux_mux_30;
            aux_d_flip_flop_26_1_q <= ~aux_mux_30;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_27_0_q <= aux_mux_31;
            aux_d_flip_flop_27_1_q <= ~aux_mux_31;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_28_0_q <= aux_mux_32;
            aux_d_flip_flop_28_1_q <= ~aux_mux_32;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_29_0_q <= aux_mux_33;
            aux_d_flip_flop_29_1_q <= ~aux_mux_33;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out3})
            1'd0: aux_mux_30 = aux_d_flip_flop_26_0_q;
            1'd1: aux_mux_30 = data_in0;
            default: aux_mux_30 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out3})
            1'd0: aux_mux_31 = aux_d_flip_flop_27_0_q;
            1'd1: aux_mux_31 = data_in1;
            default: aux_mux_31 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out3})
            1'd0: aux_mux_32 = aux_d_flip_flop_28_0_q;
            1'd1: aux_mux_32 = data_in2;
            default: aux_mux_32 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({w_level2_decoder_2to4_inst_1_out3})
            1'd0: aux_mux_33 = aux_d_flip_flop_29_0_q;
            1'd1: aux_mux_33 = data_in3;
            default: aux_mux_33 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr11, read_addr10})
            2'd0: aux_mux_34 = aux_d_flip_flop_2_0_q;
            2'd1: aux_mux_34 = aux_d_flip_flop_10_0_q;
            2'd2: aux_mux_34 = aux_d_flip_flop_18_0_q;
            2'd3: aux_mux_34 = aux_d_flip_flop_26_0_q;
            default: aux_mux_34 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr21, read_addr20})
            2'd0: aux_mux_35 = aux_d_flip_flop_2_0_q;
            2'd1: aux_mux_35 = aux_d_flip_flop_10_0_q;
            2'd2: aux_mux_35 = aux_d_flip_flop_18_0_q;
            2'd3: aux_mux_35 = aux_d_flip_flop_26_0_q;
            default: aux_mux_35 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr11, read_addr10})
            2'd0: aux_mux_36 = aux_d_flip_flop_3_0_q;
            2'd1: aux_mux_36 = aux_d_flip_flop_11_0_q;
            2'd2: aux_mux_36 = aux_d_flip_flop_19_0_q;
            2'd3: aux_mux_36 = aux_d_flip_flop_27_0_q;
            default: aux_mux_36 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr21, read_addr20})
            2'd0: aux_mux_37 = aux_d_flip_flop_3_0_q;
            2'd1: aux_mux_37 = aux_d_flip_flop_11_0_q;
            2'd2: aux_mux_37 = aux_d_flip_flop_19_0_q;
            2'd3: aux_mux_37 = aux_d_flip_flop_27_0_q;
            default: aux_mux_37 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr11, read_addr10})
            2'd0: aux_mux_38 = aux_d_flip_flop_4_0_q;
            2'd1: aux_mux_38 = aux_d_flip_flop_12_0_q;
            2'd2: aux_mux_38 = aux_d_flip_flop_20_0_q;
            2'd3: aux_mux_38 = aux_d_flip_flop_28_0_q;
            default: aux_mux_38 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr21, read_addr20})
            2'd0: aux_mux_39 = aux_d_flip_flop_4_0_q;
            2'd1: aux_mux_39 = aux_d_flip_flop_12_0_q;
            2'd2: aux_mux_39 = aux_d_flip_flop_20_0_q;
            2'd3: aux_mux_39 = aux_d_flip_flop_28_0_q;
            default: aux_mux_39 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr11, read_addr10})
            2'd0: aux_mux_40 = aux_d_flip_flop_5_0_q;
            2'd1: aux_mux_40 = aux_d_flip_flop_13_0_q;
            2'd2: aux_mux_40 = aux_d_flip_flop_21_0_q;
            2'd3: aux_mux_40 = aux_d_flip_flop_29_0_q;
            default: aux_mux_40 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr21, read_addr20})
            2'd0: aux_mux_41 = aux_d_flip_flop_5_0_q;
            2'd1: aux_mux_41 = aux_d_flip_flop_13_0_q;
            2'd2: aux_mux_41 = aux_d_flip_flop_21_0_q;
            2'd3: aux_mux_41 = aux_d_flip_flop_29_0_q;
            default: aux_mux_41 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign read_data10 = aux_mux_34;
assign read_data11 = aux_mux_36;
assign read_data12 = aux_mux_38;
assign read_data13 = aux_mux_40;
assign read_data20 = aux_mux_35;
assign read_data21 = aux_mux_37;
assign read_data22 = aux_mux_39;
assign read_data23 = aux_mux_41;
endmodule

module level5_register_file_4x4 (
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

/* ========= Outputs ========== */
output led14_1,
output led15_1,
output led16_1,
output led17_1,
output led18_1,
output led19_1,
output led20_1,
output led21_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL5_REGISTER_FILE_4X4 (level5_register_file_4x4_ic)
wire w_level5_register_file_4x4_ic_inst_1_read_data10;
wire w_level5_register_file_4x4_ic_inst_1_read_data11;
wire w_level5_register_file_4x4_ic_inst_1_read_data12;
wire w_level5_register_file_4x4_ic_inst_1_read_data13;
wire w_level5_register_file_4x4_ic_inst_1_read_data20;
wire w_level5_register_file_4x4_ic_inst_1_read_data21;
wire w_level5_register_file_4x4_ic_inst_1_read_data22;
wire w_level5_register_file_4x4_ic_inst_1_read_data23;


// Assigning aux variables. //
level5_register_file_4x4_ic level5_register_file_4x4_ic_inst_1 (
    .write_addr0(input_switch1),
    .write_addr1(input_switch2),
    .read_addr10(input_switch3),
    .read_addr11(input_switch4),
    .read_addr20(input_switch5),
    .read_addr21(input_switch6),
    .data_in0(input_switch7),
    .data_in1(input_switch8),
    .data_in2(input_switch9),
    .data_in3(input_switch10),
    .write_enable(input_switch11),
    .clock(input_switch12),
    .read_data10(w_level5_register_file_4x4_ic_inst_1_read_data10),
    .read_data11(w_level5_register_file_4x4_ic_inst_1_read_data11),
    .read_data12(w_level5_register_file_4x4_ic_inst_1_read_data12),
    .read_data13(w_level5_register_file_4x4_ic_inst_1_read_data13),
    .read_data20(w_level5_register_file_4x4_ic_inst_1_read_data20),
    .read_data21(w_level5_register_file_4x4_ic_inst_1_read_data21),
    .read_data22(w_level5_register_file_4x4_ic_inst_1_read_data22),
    .read_data23(w_level5_register_file_4x4_ic_inst_1_read_data23)
);

// Writing output data. //
assign led14_1 = w_level5_register_file_4x4_ic_inst_1_read_data10;
assign led15_1 = w_level5_register_file_4x4_ic_inst_1_read_data11;
assign led16_1 = w_level5_register_file_4x4_ic_inst_1_read_data12;
assign led17_1 = w_level5_register_file_4x4_ic_inst_1_read_data13;
assign led18_1 = w_level5_register_file_4x4_ic_inst_1_read_data20;
assign led19_1 = w_level5_register_file_4x4_ic_inst_1_read_data21;
assign led20_1 = w_level5_register_file_4x4_ic_inst_1_read_data22;
assign led21_1 = w_level5_register_file_4x4_ic_inst_1_read_data23;
endmodule
