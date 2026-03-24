// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Write_Decoder (generated from level2_decoder_2to4.panda)
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
wire aux_and_2;
wire aux_and_3;
wire aux_and_4;
wire aux_and_5;
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
reg aux_d_flip_flop_14_0_q = 1'b0;
reg aux_d_flip_flop_14_1_q = 1'b1;
reg aux_d_flip_flop_15_0_q = 1'b0;
reg aux_d_flip_flop_15_1_q = 1'b1;
reg aux_d_flip_flop_16_0_q = 1'b0;
reg aux_d_flip_flop_16_1_q = 1'b1;
reg aux_d_flip_flop_17_0_q = 1'b0;
reg aux_d_flip_flop_17_1_q = 1'b1;
reg aux_mux_18 = 1'b0;
reg aux_mux_19 = 1'b0;
reg aux_mux_20 = 1'b0;
reg aux_mux_21 = 1'b0;
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
reg aux_d_flip_flop_30_0_q = 1'b0;
reg aux_d_flip_flop_30_1_q = 1'b1;
reg aux_d_flip_flop_31_0_q = 1'b0;
reg aux_d_flip_flop_31_1_q = 1'b1;
reg aux_d_flip_flop_32_0_q = 1'b0;
reg aux_d_flip_flop_32_1_q = 1'b1;
reg aux_d_flip_flop_33_0_q = 1'b0;
reg aux_d_flip_flop_33_1_q = 1'b1;
reg aux_mux_34 = 1'b0;
reg aux_mux_35 = 1'b0;
reg aux_mux_36 = 1'b0;
reg aux_mux_37 = 1'b0;
reg aux_mux_38 = 1'b0;
reg aux_mux_39 = 1'b0;
reg aux_mux_40 = 1'b0;
reg aux_mux_41 = 1'b0;
reg aux_mux_42 = 1'b0;
reg aux_mux_43 = 1'b0;
reg aux_mux_44 = 1'b0;
reg aux_mux_45 = 1'b0;

// Internal logic
level2_decoder_2to4 level2_decoder_2to4_inst_1 (
    .addr0(write_addr0),
    .addr1(write_addr1),
    .out0(w_level2_decoder_2to4_inst_1_out0),
    .out1(w_level2_decoder_2to4_inst_1_out1),
    .out2(w_level2_decoder_2to4_inst_1_out2),
    .out3(w_level2_decoder_2to4_inst_1_out3)
);
assign aux_and_2 = (w_level2_decoder_2to4_inst_1_out0 & write_enable);
assign aux_and_3 = (w_level2_decoder_2to4_inst_1_out1 & write_enable);
assign aux_and_4 = (w_level2_decoder_2to4_inst_1_out2 & write_enable);
assign aux_and_5 = (w_level2_decoder_2to4_inst_1_out3 & write_enable);
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_6_0_q <= aux_mux_10;
            aux_d_flip_flop_6_1_q <= ~aux_mux_10;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_7_0_q <= aux_mux_11;
            aux_d_flip_flop_7_1_q <= ~aux_mux_11;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_8_0_q <= aux_mux_12;
            aux_d_flip_flop_8_1_q <= ~aux_mux_12;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_9_0_q <= aux_mux_13;
            aux_d_flip_flop_9_1_q <= ~aux_mux_13;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_10 = aux_d_flip_flop_6_0_q;
            1'd1: aux_mux_10 = data_in0;
            default: aux_mux_10 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_11 = aux_d_flip_flop_7_0_q;
            1'd1: aux_mux_11 = data_in1;
            default: aux_mux_11 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_12 = aux_d_flip_flop_8_0_q;
            1'd1: aux_mux_12 = data_in2;
            default: aux_mux_12 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_13 = aux_d_flip_flop_9_0_q;
            1'd1: aux_mux_13 = data_in3;
            default: aux_mux_13 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_14_0_q <= aux_mux_18;
            aux_d_flip_flop_14_1_q <= ~aux_mux_18;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_15_0_q <= aux_mux_19;
            aux_d_flip_flop_15_1_q <= ~aux_mux_19;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_16_0_q <= aux_mux_20;
            aux_d_flip_flop_16_1_q <= ~aux_mux_20;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_17_0_q <= aux_mux_21;
            aux_d_flip_flop_17_1_q <= ~aux_mux_21;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_18 = aux_d_flip_flop_14_0_q;
            1'd1: aux_mux_18 = data_in0;
            default: aux_mux_18 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_19 = aux_d_flip_flop_15_0_q;
            1'd1: aux_mux_19 = data_in1;
            default: aux_mux_19 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_20 = aux_d_flip_flop_16_0_q;
            1'd1: aux_mux_20 = data_in2;
            default: aux_mux_20 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_21 = aux_d_flip_flop_17_0_q;
            1'd1: aux_mux_21 = data_in3;
            default: aux_mux_21 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_22_0_q <= aux_mux_26;
            aux_d_flip_flop_22_1_q <= ~aux_mux_26;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_23_0_q <= aux_mux_27;
            aux_d_flip_flop_23_1_q <= ~aux_mux_27;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_24_0_q <= aux_mux_28;
            aux_d_flip_flop_24_1_q <= ~aux_mux_28;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_25_0_q <= aux_mux_29;
            aux_d_flip_flop_25_1_q <= ~aux_mux_29;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_26 = aux_d_flip_flop_22_0_q;
            1'd1: aux_mux_26 = data_in0;
            default: aux_mux_26 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_27 = aux_d_flip_flop_23_0_q;
            1'd1: aux_mux_27 = data_in1;
            default: aux_mux_27 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_28 = aux_d_flip_flop_24_0_q;
            1'd1: aux_mux_28 = data_in2;
            default: aux_mux_28 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_29 = aux_d_flip_flop_25_0_q;
            1'd1: aux_mux_29 = data_in3;
            default: aux_mux_29 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_30_0_q <= aux_mux_34;
            aux_d_flip_flop_30_1_q <= ~aux_mux_34;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_31_0_q <= aux_mux_35;
            aux_d_flip_flop_31_1_q <= ~aux_mux_35;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_32_0_q <= aux_mux_36;
            aux_d_flip_flop_32_1_q <= ~aux_mux_36;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_33_0_q <= aux_mux_37;
            aux_d_flip_flop_33_1_q <= ~aux_mux_37;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_34 = aux_d_flip_flop_30_0_q;
            1'd1: aux_mux_34 = data_in0;
            default: aux_mux_34 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_35 = aux_d_flip_flop_31_0_q;
            1'd1: aux_mux_35 = data_in1;
            default: aux_mux_35 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_36 = aux_d_flip_flop_32_0_q;
            1'd1: aux_mux_36 = data_in2;
            default: aux_mux_36 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_37 = aux_d_flip_flop_33_0_q;
            1'd1: aux_mux_37 = data_in3;
            default: aux_mux_37 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr11, read_addr10})
            2'd0: aux_mux_38 = aux_d_flip_flop_6_0_q;
            2'd1: aux_mux_38 = aux_d_flip_flop_14_0_q;
            2'd2: aux_mux_38 = aux_d_flip_flop_22_0_q;
            2'd3: aux_mux_38 = aux_d_flip_flop_30_0_q;
            default: aux_mux_38 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr21, read_addr20})
            2'd0: aux_mux_39 = aux_d_flip_flop_6_0_q;
            2'd1: aux_mux_39 = aux_d_flip_flop_14_0_q;
            2'd2: aux_mux_39 = aux_d_flip_flop_22_0_q;
            2'd3: aux_mux_39 = aux_d_flip_flop_30_0_q;
            default: aux_mux_39 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr11, read_addr10})
            2'd0: aux_mux_40 = aux_d_flip_flop_7_0_q;
            2'd1: aux_mux_40 = aux_d_flip_flop_15_0_q;
            2'd2: aux_mux_40 = aux_d_flip_flop_23_0_q;
            2'd3: aux_mux_40 = aux_d_flip_flop_31_0_q;
            default: aux_mux_40 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr21, read_addr20})
            2'd0: aux_mux_41 = aux_d_flip_flop_7_0_q;
            2'd1: aux_mux_41 = aux_d_flip_flop_15_0_q;
            2'd2: aux_mux_41 = aux_d_flip_flop_23_0_q;
            2'd3: aux_mux_41 = aux_d_flip_flop_31_0_q;
            default: aux_mux_41 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr11, read_addr10})
            2'd0: aux_mux_42 = aux_d_flip_flop_8_0_q;
            2'd1: aux_mux_42 = aux_d_flip_flop_16_0_q;
            2'd2: aux_mux_42 = aux_d_flip_flop_24_0_q;
            2'd3: aux_mux_42 = aux_d_flip_flop_32_0_q;
            default: aux_mux_42 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr21, read_addr20})
            2'd0: aux_mux_43 = aux_d_flip_flop_8_0_q;
            2'd1: aux_mux_43 = aux_d_flip_flop_16_0_q;
            2'd2: aux_mux_43 = aux_d_flip_flop_24_0_q;
            2'd3: aux_mux_43 = aux_d_flip_flop_32_0_q;
            default: aux_mux_43 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr11, read_addr10})
            2'd0: aux_mux_44 = aux_d_flip_flop_9_0_q;
            2'd1: aux_mux_44 = aux_d_flip_flop_17_0_q;
            2'd2: aux_mux_44 = aux_d_flip_flop_25_0_q;
            2'd3: aux_mux_44 = aux_d_flip_flop_33_0_q;
            default: aux_mux_44 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr21, read_addr20})
            2'd0: aux_mux_45 = aux_d_flip_flop_9_0_q;
            2'd1: aux_mux_45 = aux_d_flip_flop_17_0_q;
            2'd2: aux_mux_45 = aux_d_flip_flop_25_0_q;
            2'd3: aux_mux_45 = aux_d_flip_flop_33_0_q;
            default: aux_mux_45 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign read_data10 = aux_mux_38;
assign read_data11 = aux_mux_40;
assign read_data12 = aux_mux_42;
assign read_data13 = aux_mux_44;
assign read_data20 = aux_mux_39;
assign read_data21 = aux_mux_41;
assign read_data22 = aux_mux_43;
assign read_data23 = aux_mux_45;
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
