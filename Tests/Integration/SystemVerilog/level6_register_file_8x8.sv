// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Write_Decoder (generated from level2_decoder_3to8.panda)
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

// Module for LEVEL6_REGISTER_FILE_8X8 (generated from level6_register_file_8x8.panda)
module level6_register_file_8x8_ic (
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

module level6_register_file_8x8 (
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

/* ========= Outputs ========== */
output led21_1,
output led22_1,
output led23_1,
output led24_1,
output led25_1,
output led26_1,
output led27_1,
output led28_1,
output led29_1,
output led30_1,
output led31_1,
output led32_1,
output led33_1,
output led34_1,
output led35_1,
output led36_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL6_REGISTER_FILE_8X8 (level6_register_file_8x8_ic)
wire w_level6_register_file_8x8_ic_inst_1_read_data10;
wire w_level6_register_file_8x8_ic_inst_1_read_data11;
wire w_level6_register_file_8x8_ic_inst_1_read_data12;
wire w_level6_register_file_8x8_ic_inst_1_read_data13;
wire w_level6_register_file_8x8_ic_inst_1_read_data14;
wire w_level6_register_file_8x8_ic_inst_1_read_data15;
wire w_level6_register_file_8x8_ic_inst_1_read_data16;
wire w_level6_register_file_8x8_ic_inst_1_read_data17;
wire w_level6_register_file_8x8_ic_inst_1_read_data20;
wire w_level6_register_file_8x8_ic_inst_1_read_data21;
wire w_level6_register_file_8x8_ic_inst_1_read_data22;
wire w_level6_register_file_8x8_ic_inst_1_read_data23;
wire w_level6_register_file_8x8_ic_inst_1_read_data24;
wire w_level6_register_file_8x8_ic_inst_1_read_data25;
wire w_level6_register_file_8x8_ic_inst_1_read_data26;
wire w_level6_register_file_8x8_ic_inst_1_read_data27;


// Assigning aux variables. //
level6_register_file_8x8_ic level6_register_file_8x8_ic_inst_1 (
    .write_addr0(input_switch1),
    .write_addr1(input_switch2),
    .write_addr2(input_switch3),
    .read_addr10(input_switch4),
    .read_addr11(input_switch5),
    .read_addr12(input_switch6),
    .read_addr20(input_switch7),
    .read_addr21(input_switch8),
    .read_addr22(input_switch9),
    .data_in0(input_switch10),
    .data_in1(input_switch11),
    .data_in2(input_switch12),
    .data_in3(input_switch13),
    .data_in4(input_switch14),
    .data_in5(input_switch15),
    .data_in6(input_switch16),
    .data_in7(input_switch17),
    .write_enable(input_switch18),
    .clock(input_switch19),
    .read_data10(w_level6_register_file_8x8_ic_inst_1_read_data10),
    .read_data11(w_level6_register_file_8x8_ic_inst_1_read_data11),
    .read_data12(w_level6_register_file_8x8_ic_inst_1_read_data12),
    .read_data13(w_level6_register_file_8x8_ic_inst_1_read_data13),
    .read_data14(w_level6_register_file_8x8_ic_inst_1_read_data14),
    .read_data15(w_level6_register_file_8x8_ic_inst_1_read_data15),
    .read_data16(w_level6_register_file_8x8_ic_inst_1_read_data16),
    .read_data17(w_level6_register_file_8x8_ic_inst_1_read_data17),
    .read_data20(w_level6_register_file_8x8_ic_inst_1_read_data20),
    .read_data21(w_level6_register_file_8x8_ic_inst_1_read_data21),
    .read_data22(w_level6_register_file_8x8_ic_inst_1_read_data22),
    .read_data23(w_level6_register_file_8x8_ic_inst_1_read_data23),
    .read_data24(w_level6_register_file_8x8_ic_inst_1_read_data24),
    .read_data25(w_level6_register_file_8x8_ic_inst_1_read_data25),
    .read_data26(w_level6_register_file_8x8_ic_inst_1_read_data26),
    .read_data27(w_level6_register_file_8x8_ic_inst_1_read_data27)
);

// Writing output data. //
assign led21_1 = w_level6_register_file_8x8_ic_inst_1_read_data10;
assign led22_1 = w_level6_register_file_8x8_ic_inst_1_read_data11;
assign led23_1 = w_level6_register_file_8x8_ic_inst_1_read_data12;
assign led24_1 = w_level6_register_file_8x8_ic_inst_1_read_data13;
assign led25_1 = w_level6_register_file_8x8_ic_inst_1_read_data14;
assign led26_1 = w_level6_register_file_8x8_ic_inst_1_read_data15;
assign led27_1 = w_level6_register_file_8x8_ic_inst_1_read_data16;
assign led28_1 = w_level6_register_file_8x8_ic_inst_1_read_data17;
assign led29_1 = w_level6_register_file_8x8_ic_inst_1_read_data20;
assign led30_1 = w_level6_register_file_8x8_ic_inst_1_read_data21;
assign led31_1 = w_level6_register_file_8x8_ic_inst_1_read_data22;
assign led32_1 = w_level6_register_file_8x8_ic_inst_1_read_data23;
assign led33_1 = w_level6_register_file_8x8_ic_inst_1_read_data24;
assign led34_1 = w_level6_register_file_8x8_ic_inst_1_read_data25;
assign led35_1 = w_level6_register_file_8x8_ic_inst_1_read_data26;
assign led36_1 = w_level6_register_file_8x8_ic_inst_1_read_data27;
endmodule
