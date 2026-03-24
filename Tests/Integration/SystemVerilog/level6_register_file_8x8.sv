// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for Write_Decoder (generated from level2_decoder_3to8.panda)
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
wire aux_and_2;
wire aux_and_3;
wire aux_and_4;
wire aux_and_5;
wire aux_and_6;
wire aux_and_7;
wire aux_and_8;
wire aux_and_9;
reg aux_d_flip_flop_10_0_q = 1'b0;
reg aux_d_flip_flop_10_1_q = 1'b1;
reg aux_d_flip_flop_11_0_q = 1'b0;
reg aux_d_flip_flop_11_1_q = 1'b1;
reg aux_d_flip_flop_12_0_q = 1'b0;
reg aux_d_flip_flop_12_1_q = 1'b1;
reg aux_d_flip_flop_13_0_q = 1'b0;
reg aux_d_flip_flop_13_1_q = 1'b1;
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
reg aux_d_flip_flop_42_0_q = 1'b0;
reg aux_d_flip_flop_42_1_q = 1'b1;
reg aux_d_flip_flop_43_0_q = 1'b0;
reg aux_d_flip_flop_43_1_q = 1'b1;
reg aux_d_flip_flop_44_0_q = 1'b0;
reg aux_d_flip_flop_44_1_q = 1'b1;
reg aux_d_flip_flop_45_0_q = 1'b0;
reg aux_d_flip_flop_45_1_q = 1'b1;
reg aux_d_flip_flop_46_0_q = 1'b0;
reg aux_d_flip_flop_46_1_q = 1'b1;
reg aux_d_flip_flop_47_0_q = 1'b0;
reg aux_d_flip_flop_47_1_q = 1'b1;
reg aux_d_flip_flop_48_0_q = 1'b0;
reg aux_d_flip_flop_48_1_q = 1'b1;
reg aux_d_flip_flop_49_0_q = 1'b0;
reg aux_d_flip_flop_49_1_q = 1'b1;
reg aux_mux_50 = 1'b0;
reg aux_mux_51 = 1'b0;
reg aux_mux_52 = 1'b0;
reg aux_mux_53 = 1'b0;
reg aux_mux_54 = 1'b0;
reg aux_mux_55 = 1'b0;
reg aux_mux_56 = 1'b0;
reg aux_mux_57 = 1'b0;
reg aux_d_flip_flop_58_0_q = 1'b0;
reg aux_d_flip_flop_58_1_q = 1'b1;
reg aux_d_flip_flop_59_0_q = 1'b0;
reg aux_d_flip_flop_59_1_q = 1'b1;
reg aux_d_flip_flop_60_0_q = 1'b0;
reg aux_d_flip_flop_60_1_q = 1'b1;
reg aux_d_flip_flop_61_0_q = 1'b0;
reg aux_d_flip_flop_61_1_q = 1'b1;
reg aux_d_flip_flop_62_0_q = 1'b0;
reg aux_d_flip_flop_62_1_q = 1'b1;
reg aux_d_flip_flop_63_0_q = 1'b0;
reg aux_d_flip_flop_63_1_q = 1'b1;
reg aux_d_flip_flop_64_0_q = 1'b0;
reg aux_d_flip_flop_64_1_q = 1'b1;
reg aux_d_flip_flop_65_0_q = 1'b0;
reg aux_d_flip_flop_65_1_q = 1'b1;
reg aux_mux_66 = 1'b0;
reg aux_mux_67 = 1'b0;
reg aux_mux_68 = 1'b0;
reg aux_mux_69 = 1'b0;
reg aux_mux_70 = 1'b0;
reg aux_mux_71 = 1'b0;
reg aux_mux_72 = 1'b0;
reg aux_mux_73 = 1'b0;
reg aux_d_flip_flop_74_0_q = 1'b0;
reg aux_d_flip_flop_74_1_q = 1'b1;
reg aux_d_flip_flop_75_0_q = 1'b0;
reg aux_d_flip_flop_75_1_q = 1'b1;
reg aux_d_flip_flop_76_0_q = 1'b0;
reg aux_d_flip_flop_76_1_q = 1'b1;
reg aux_d_flip_flop_77_0_q = 1'b0;
reg aux_d_flip_flop_77_1_q = 1'b1;
reg aux_d_flip_flop_78_0_q = 1'b0;
reg aux_d_flip_flop_78_1_q = 1'b1;
reg aux_d_flip_flop_79_0_q = 1'b0;
reg aux_d_flip_flop_79_1_q = 1'b1;
reg aux_d_flip_flop_80_0_q = 1'b0;
reg aux_d_flip_flop_80_1_q = 1'b1;
reg aux_d_flip_flop_81_0_q = 1'b0;
reg aux_d_flip_flop_81_1_q = 1'b1;
reg aux_mux_82 = 1'b0;
reg aux_mux_83 = 1'b0;
reg aux_mux_84 = 1'b0;
reg aux_mux_85 = 1'b0;
reg aux_mux_86 = 1'b0;
reg aux_mux_87 = 1'b0;
reg aux_mux_88 = 1'b0;
reg aux_mux_89 = 1'b0;
reg aux_d_flip_flop_90_0_q = 1'b0;
reg aux_d_flip_flop_90_1_q = 1'b1;
reg aux_d_flip_flop_91_0_q = 1'b0;
reg aux_d_flip_flop_91_1_q = 1'b1;
reg aux_d_flip_flop_92_0_q = 1'b0;
reg aux_d_flip_flop_92_1_q = 1'b1;
reg aux_d_flip_flop_93_0_q = 1'b0;
reg aux_d_flip_flop_93_1_q = 1'b1;
reg aux_d_flip_flop_94_0_q = 1'b0;
reg aux_d_flip_flop_94_1_q = 1'b1;
reg aux_d_flip_flop_95_0_q = 1'b0;
reg aux_d_flip_flop_95_1_q = 1'b1;
reg aux_d_flip_flop_96_0_q = 1'b0;
reg aux_d_flip_flop_96_1_q = 1'b1;
reg aux_d_flip_flop_97_0_q = 1'b0;
reg aux_d_flip_flop_97_1_q = 1'b1;
reg aux_mux_98 = 1'b0;
reg aux_mux_99 = 1'b0;
reg aux_mux_100 = 1'b0;
reg aux_mux_101 = 1'b0;
reg aux_mux_102 = 1'b0;
reg aux_mux_103 = 1'b0;
reg aux_mux_104 = 1'b0;
reg aux_mux_105 = 1'b0;
reg aux_d_flip_flop_106_0_q = 1'b0;
reg aux_d_flip_flop_106_1_q = 1'b1;
reg aux_d_flip_flop_107_0_q = 1'b0;
reg aux_d_flip_flop_107_1_q = 1'b1;
reg aux_d_flip_flop_108_0_q = 1'b0;
reg aux_d_flip_flop_108_1_q = 1'b1;
reg aux_d_flip_flop_109_0_q = 1'b0;
reg aux_d_flip_flop_109_1_q = 1'b1;
reg aux_d_flip_flop_110_0_q = 1'b0;
reg aux_d_flip_flop_110_1_q = 1'b1;
reg aux_d_flip_flop_111_0_q = 1'b0;
reg aux_d_flip_flop_111_1_q = 1'b1;
reg aux_d_flip_flop_112_0_q = 1'b0;
reg aux_d_flip_flop_112_1_q = 1'b1;
reg aux_d_flip_flop_113_0_q = 1'b0;
reg aux_d_flip_flop_113_1_q = 1'b1;
reg aux_mux_114 = 1'b0;
reg aux_mux_115 = 1'b0;
reg aux_mux_116 = 1'b0;
reg aux_mux_117 = 1'b0;
reg aux_mux_118 = 1'b0;
reg aux_mux_119 = 1'b0;
reg aux_mux_120 = 1'b0;
reg aux_mux_121 = 1'b0;
reg aux_d_flip_flop_122_0_q = 1'b0;
reg aux_d_flip_flop_122_1_q = 1'b1;
reg aux_d_flip_flop_123_0_q = 1'b0;
reg aux_d_flip_flop_123_1_q = 1'b1;
reg aux_d_flip_flop_124_0_q = 1'b0;
reg aux_d_flip_flop_124_1_q = 1'b1;
reg aux_d_flip_flop_125_0_q = 1'b0;
reg aux_d_flip_flop_125_1_q = 1'b1;
reg aux_d_flip_flop_126_0_q = 1'b0;
reg aux_d_flip_flop_126_1_q = 1'b1;
reg aux_d_flip_flop_127_0_q = 1'b0;
reg aux_d_flip_flop_127_1_q = 1'b1;
reg aux_d_flip_flop_128_0_q = 1'b0;
reg aux_d_flip_flop_128_1_q = 1'b1;
reg aux_d_flip_flop_129_0_q = 1'b0;
reg aux_d_flip_flop_129_1_q = 1'b1;
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
reg aux_mux_146 = 1'b0;
reg aux_mux_147 = 1'b0;
reg aux_mux_148 = 1'b0;
reg aux_mux_149 = 1'b0;
reg aux_mux_150 = 1'b0;
reg aux_mux_151 = 1'b0;
reg aux_mux_152 = 1'b0;
reg aux_mux_153 = 1'b0;

// Internal logic
level2_decoder_3to8 level2_decoder_3to8_inst_1 (
    .addr0(write_addr0),
    .addr1(write_addr1),
    .addr2(write_addr2),
    .out0(w_level2_decoder_3to8_inst_1_out0),
    .out1(w_level2_decoder_3to8_inst_1_out1),
    .out2(w_level2_decoder_3to8_inst_1_out2),
    .out3(w_level2_decoder_3to8_inst_1_out3),
    .out4(w_level2_decoder_3to8_inst_1_out4),
    .out5(w_level2_decoder_3to8_inst_1_out5),
    .out6(w_level2_decoder_3to8_inst_1_out6),
    .out7(w_level2_decoder_3to8_inst_1_out7)
);
assign aux_and_2 = (w_level2_decoder_3to8_inst_1_out0 & write_enable);
assign aux_and_3 = (w_level2_decoder_3to8_inst_1_out1 & write_enable);
assign aux_and_4 = (w_level2_decoder_3to8_inst_1_out2 & write_enable);
assign aux_and_5 = (w_level2_decoder_3to8_inst_1_out3 & write_enable);
assign aux_and_6 = (w_level2_decoder_3to8_inst_1_out4 & write_enable);
assign aux_and_7 = (w_level2_decoder_3to8_inst_1_out5 & write_enable);
assign aux_and_8 = (w_level2_decoder_3to8_inst_1_out6 & write_enable);
assign aux_and_9 = (w_level2_decoder_3to8_inst_1_out7 & write_enable);
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_10_0_q <= aux_mux_18;
            aux_d_flip_flop_10_1_q <= ~aux_mux_18;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_11_0_q <= aux_mux_19;
            aux_d_flip_flop_11_1_q <= ~aux_mux_19;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_12_0_q <= aux_mux_20;
            aux_d_flip_flop_12_1_q <= ~aux_mux_20;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_13_0_q <= aux_mux_21;
            aux_d_flip_flop_13_1_q <= ~aux_mux_21;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_14_0_q <= aux_mux_22;
            aux_d_flip_flop_14_1_q <= ~aux_mux_22;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_15_0_q <= aux_mux_23;
            aux_d_flip_flop_15_1_q <= ~aux_mux_23;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_16_0_q <= aux_mux_24;
            aux_d_flip_flop_16_1_q <= ~aux_mux_24;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_17_0_q <= aux_mux_25;
            aux_d_flip_flop_17_1_q <= ~aux_mux_25;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_18 = aux_d_flip_flop_10_0_q;
            1'd1: aux_mux_18 = data_in0;
            default: aux_mux_18 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_19 = aux_d_flip_flop_11_0_q;
            1'd1: aux_mux_19 = data_in1;
            default: aux_mux_19 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_20 = aux_d_flip_flop_12_0_q;
            1'd1: aux_mux_20 = data_in2;
            default: aux_mux_20 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_21 = aux_d_flip_flop_13_0_q;
            1'd1: aux_mux_21 = data_in3;
            default: aux_mux_21 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_22 = aux_d_flip_flop_14_0_q;
            1'd1: aux_mux_22 = data_in4;
            default: aux_mux_22 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_23 = aux_d_flip_flop_15_0_q;
            1'd1: aux_mux_23 = data_in5;
            default: aux_mux_23 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_24 = aux_d_flip_flop_16_0_q;
            1'd1: aux_mux_24 = data_in6;
            default: aux_mux_24 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_2})
            1'd0: aux_mux_25 = aux_d_flip_flop_17_0_q;
            1'd1: aux_mux_25 = data_in7;
            default: aux_mux_25 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_26_0_q <= aux_mux_34;
            aux_d_flip_flop_26_1_q <= ~aux_mux_34;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_27_0_q <= aux_mux_35;
            aux_d_flip_flop_27_1_q <= ~aux_mux_35;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_28_0_q <= aux_mux_36;
            aux_d_flip_flop_28_1_q <= ~aux_mux_36;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_29_0_q <= aux_mux_37;
            aux_d_flip_flop_29_1_q <= ~aux_mux_37;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_30_0_q <= aux_mux_38;
            aux_d_flip_flop_30_1_q <= ~aux_mux_38;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_31_0_q <= aux_mux_39;
            aux_d_flip_flop_31_1_q <= ~aux_mux_39;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_32_0_q <= aux_mux_40;
            aux_d_flip_flop_32_1_q <= ~aux_mux_40;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_33_0_q <= aux_mux_41;
            aux_d_flip_flop_33_1_q <= ~aux_mux_41;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_34 = aux_d_flip_flop_26_0_q;
            1'd1: aux_mux_34 = data_in0;
            default: aux_mux_34 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_35 = aux_d_flip_flop_27_0_q;
            1'd1: aux_mux_35 = data_in1;
            default: aux_mux_35 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_36 = aux_d_flip_flop_28_0_q;
            1'd1: aux_mux_36 = data_in2;
            default: aux_mux_36 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_37 = aux_d_flip_flop_29_0_q;
            1'd1: aux_mux_37 = data_in3;
            default: aux_mux_37 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_38 = aux_d_flip_flop_30_0_q;
            1'd1: aux_mux_38 = data_in4;
            default: aux_mux_38 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_39 = aux_d_flip_flop_31_0_q;
            1'd1: aux_mux_39 = data_in5;
            default: aux_mux_39 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_40 = aux_d_flip_flop_32_0_q;
            1'd1: aux_mux_40 = data_in6;
            default: aux_mux_40 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_3})
            1'd0: aux_mux_41 = aux_d_flip_flop_33_0_q;
            1'd1: aux_mux_41 = data_in7;
            default: aux_mux_41 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_42_0_q <= aux_mux_50;
            aux_d_flip_flop_42_1_q <= ~aux_mux_50;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_43_0_q <= aux_mux_51;
            aux_d_flip_flop_43_1_q <= ~aux_mux_51;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_44_0_q <= aux_mux_52;
            aux_d_flip_flop_44_1_q <= ~aux_mux_52;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_45_0_q <= aux_mux_53;
            aux_d_flip_flop_45_1_q <= ~aux_mux_53;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_46_0_q <= aux_mux_54;
            aux_d_flip_flop_46_1_q <= ~aux_mux_54;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_47_0_q <= aux_mux_55;
            aux_d_flip_flop_47_1_q <= ~aux_mux_55;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_48_0_q <= aux_mux_56;
            aux_d_flip_flop_48_1_q <= ~aux_mux_56;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_49_0_q <= aux_mux_57;
            aux_d_flip_flop_49_1_q <= ~aux_mux_57;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_50 = aux_d_flip_flop_42_0_q;
            1'd1: aux_mux_50 = data_in0;
            default: aux_mux_50 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_51 = aux_d_flip_flop_43_0_q;
            1'd1: aux_mux_51 = data_in1;
            default: aux_mux_51 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_52 = aux_d_flip_flop_44_0_q;
            1'd1: aux_mux_52 = data_in2;
            default: aux_mux_52 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_53 = aux_d_flip_flop_45_0_q;
            1'd1: aux_mux_53 = data_in3;
            default: aux_mux_53 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_54 = aux_d_flip_flop_46_0_q;
            1'd1: aux_mux_54 = data_in4;
            default: aux_mux_54 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_55 = aux_d_flip_flop_47_0_q;
            1'd1: aux_mux_55 = data_in5;
            default: aux_mux_55 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_56 = aux_d_flip_flop_48_0_q;
            1'd1: aux_mux_56 = data_in6;
            default: aux_mux_56 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_4})
            1'd0: aux_mux_57 = aux_d_flip_flop_49_0_q;
            1'd1: aux_mux_57 = data_in7;
            default: aux_mux_57 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_58_0_q <= aux_mux_66;
            aux_d_flip_flop_58_1_q <= ~aux_mux_66;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_59_0_q <= aux_mux_67;
            aux_d_flip_flop_59_1_q <= ~aux_mux_67;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_60_0_q <= aux_mux_68;
            aux_d_flip_flop_60_1_q <= ~aux_mux_68;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_61_0_q <= aux_mux_69;
            aux_d_flip_flop_61_1_q <= ~aux_mux_69;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_62_0_q <= aux_mux_70;
            aux_d_flip_flop_62_1_q <= ~aux_mux_70;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_63_0_q <= aux_mux_71;
            aux_d_flip_flop_63_1_q <= ~aux_mux_71;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_64_0_q <= aux_mux_72;
            aux_d_flip_flop_64_1_q <= ~aux_mux_72;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_65_0_q <= aux_mux_73;
            aux_d_flip_flop_65_1_q <= ~aux_mux_73;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_66 = aux_d_flip_flop_58_0_q;
            1'd1: aux_mux_66 = data_in0;
            default: aux_mux_66 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_67 = aux_d_flip_flop_59_0_q;
            1'd1: aux_mux_67 = data_in1;
            default: aux_mux_67 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_68 = aux_d_flip_flop_60_0_q;
            1'd1: aux_mux_68 = data_in2;
            default: aux_mux_68 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_69 = aux_d_flip_flop_61_0_q;
            1'd1: aux_mux_69 = data_in3;
            default: aux_mux_69 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_70 = aux_d_flip_flop_62_0_q;
            1'd1: aux_mux_70 = data_in4;
            default: aux_mux_70 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_71 = aux_d_flip_flop_63_0_q;
            1'd1: aux_mux_71 = data_in5;
            default: aux_mux_71 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_72 = aux_d_flip_flop_64_0_q;
            1'd1: aux_mux_72 = data_in6;
            default: aux_mux_72 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_5})
            1'd0: aux_mux_73 = aux_d_flip_flop_65_0_q;
            1'd1: aux_mux_73 = data_in7;
            default: aux_mux_73 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_74_0_q <= aux_mux_82;
            aux_d_flip_flop_74_1_q <= ~aux_mux_82;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_75_0_q <= aux_mux_83;
            aux_d_flip_flop_75_1_q <= ~aux_mux_83;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_76_0_q <= aux_mux_84;
            aux_d_flip_flop_76_1_q <= ~aux_mux_84;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_77_0_q <= aux_mux_85;
            aux_d_flip_flop_77_1_q <= ~aux_mux_85;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_78_0_q <= aux_mux_86;
            aux_d_flip_flop_78_1_q <= ~aux_mux_86;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_79_0_q <= aux_mux_87;
            aux_d_flip_flop_79_1_q <= ~aux_mux_87;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_80_0_q <= aux_mux_88;
            aux_d_flip_flop_80_1_q <= ~aux_mux_88;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_81_0_q <= aux_mux_89;
            aux_d_flip_flop_81_1_q <= ~aux_mux_89;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_82 = aux_d_flip_flop_74_0_q;
            1'd1: aux_mux_82 = data_in0;
            default: aux_mux_82 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_83 = aux_d_flip_flop_75_0_q;
            1'd1: aux_mux_83 = data_in1;
            default: aux_mux_83 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_84 = aux_d_flip_flop_76_0_q;
            1'd1: aux_mux_84 = data_in2;
            default: aux_mux_84 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_85 = aux_d_flip_flop_77_0_q;
            1'd1: aux_mux_85 = data_in3;
            default: aux_mux_85 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_86 = aux_d_flip_flop_78_0_q;
            1'd1: aux_mux_86 = data_in4;
            default: aux_mux_86 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_87 = aux_d_flip_flop_79_0_q;
            1'd1: aux_mux_87 = data_in5;
            default: aux_mux_87 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_88 = aux_d_flip_flop_80_0_q;
            1'd1: aux_mux_88 = data_in6;
            default: aux_mux_88 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_6})
            1'd0: aux_mux_89 = aux_d_flip_flop_81_0_q;
            1'd1: aux_mux_89 = data_in7;
            default: aux_mux_89 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_90_0_q <= aux_mux_98;
            aux_d_flip_flop_90_1_q <= ~aux_mux_98;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_91_0_q <= aux_mux_99;
            aux_d_flip_flop_91_1_q <= ~aux_mux_99;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_92_0_q <= aux_mux_100;
            aux_d_flip_flop_92_1_q <= ~aux_mux_100;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_93_0_q <= aux_mux_101;
            aux_d_flip_flop_93_1_q <= ~aux_mux_101;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_94_0_q <= aux_mux_102;
            aux_d_flip_flop_94_1_q <= ~aux_mux_102;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_95_0_q <= aux_mux_103;
            aux_d_flip_flop_95_1_q <= ~aux_mux_103;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_96_0_q <= aux_mux_104;
            aux_d_flip_flop_96_1_q <= ~aux_mux_104;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_97_0_q <= aux_mux_105;
            aux_d_flip_flop_97_1_q <= ~aux_mux_105;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_98 = aux_d_flip_flop_90_0_q;
            1'd1: aux_mux_98 = data_in0;
            default: aux_mux_98 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_99 = aux_d_flip_flop_91_0_q;
            1'd1: aux_mux_99 = data_in1;
            default: aux_mux_99 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_100 = aux_d_flip_flop_92_0_q;
            1'd1: aux_mux_100 = data_in2;
            default: aux_mux_100 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_101 = aux_d_flip_flop_93_0_q;
            1'd1: aux_mux_101 = data_in3;
            default: aux_mux_101 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_102 = aux_d_flip_flop_94_0_q;
            1'd1: aux_mux_102 = data_in4;
            default: aux_mux_102 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_103 = aux_d_flip_flop_95_0_q;
            1'd1: aux_mux_103 = data_in5;
            default: aux_mux_103 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_104 = aux_d_flip_flop_96_0_q;
            1'd1: aux_mux_104 = data_in6;
            default: aux_mux_104 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_7})
            1'd0: aux_mux_105 = aux_d_flip_flop_97_0_q;
            1'd1: aux_mux_105 = data_in7;
            default: aux_mux_105 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_106_0_q <= aux_mux_114;
            aux_d_flip_flop_106_1_q <= ~aux_mux_114;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_107_0_q <= aux_mux_115;
            aux_d_flip_flop_107_1_q <= ~aux_mux_115;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_108_0_q <= aux_mux_116;
            aux_d_flip_flop_108_1_q <= ~aux_mux_116;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_109_0_q <= aux_mux_117;
            aux_d_flip_flop_109_1_q <= ~aux_mux_117;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_110_0_q <= aux_mux_118;
            aux_d_flip_flop_110_1_q <= ~aux_mux_118;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_111_0_q <= aux_mux_119;
            aux_d_flip_flop_111_1_q <= ~aux_mux_119;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_112_0_q <= aux_mux_120;
            aux_d_flip_flop_112_1_q <= ~aux_mux_120;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_113_0_q <= aux_mux_121;
            aux_d_flip_flop_113_1_q <= ~aux_mux_121;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_114 = aux_d_flip_flop_106_0_q;
            1'd1: aux_mux_114 = data_in0;
            default: aux_mux_114 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_115 = aux_d_flip_flop_107_0_q;
            1'd1: aux_mux_115 = data_in1;
            default: aux_mux_115 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_116 = aux_d_flip_flop_108_0_q;
            1'd1: aux_mux_116 = data_in2;
            default: aux_mux_116 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_117 = aux_d_flip_flop_109_0_q;
            1'd1: aux_mux_117 = data_in3;
            default: aux_mux_117 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_118 = aux_d_flip_flop_110_0_q;
            1'd1: aux_mux_118 = data_in4;
            default: aux_mux_118 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_119 = aux_d_flip_flop_111_0_q;
            1'd1: aux_mux_119 = data_in5;
            default: aux_mux_119 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_120 = aux_d_flip_flop_112_0_q;
            1'd1: aux_mux_120 = data_in6;
            default: aux_mux_120 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_8})
            1'd0: aux_mux_121 = aux_d_flip_flop_113_0_q;
            1'd1: aux_mux_121 = data_in7;
            default: aux_mux_121 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_122_0_q <= aux_mux_130;
            aux_d_flip_flop_122_1_q <= ~aux_mux_130;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_123_0_q <= aux_mux_131;
            aux_d_flip_flop_123_1_q <= ~aux_mux_131;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_124_0_q <= aux_mux_132;
            aux_d_flip_flop_124_1_q <= ~aux_mux_132;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_125_0_q <= aux_mux_133;
            aux_d_flip_flop_125_1_q <= ~aux_mux_133;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_126_0_q <= aux_mux_134;
            aux_d_flip_flop_126_1_q <= ~aux_mux_134;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_127_0_q <= aux_mux_135;
            aux_d_flip_flop_127_1_q <= ~aux_mux_135;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_128_0_q <= aux_mux_136;
            aux_d_flip_flop_128_1_q <= ~aux_mux_136;
    end
    //End of D FlipFlop
    //D FlipFlop
    always @(posedge clock)
    begin
            aux_d_flip_flop_129_0_q <= aux_mux_137;
            aux_d_flip_flop_129_1_q <= ~aux_mux_137;
    end
    //End of D FlipFlop
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_130 = aux_d_flip_flop_122_0_q;
            1'd1: aux_mux_130 = data_in0;
            default: aux_mux_130 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_131 = aux_d_flip_flop_123_0_q;
            1'd1: aux_mux_131 = data_in1;
            default: aux_mux_131 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_132 = aux_d_flip_flop_124_0_q;
            1'd1: aux_mux_132 = data_in2;
            default: aux_mux_132 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_133 = aux_d_flip_flop_125_0_q;
            1'd1: aux_mux_133 = data_in3;
            default: aux_mux_133 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_134 = aux_d_flip_flop_126_0_q;
            1'd1: aux_mux_134 = data_in4;
            default: aux_mux_134 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_135 = aux_d_flip_flop_127_0_q;
            1'd1: aux_mux_135 = data_in5;
            default: aux_mux_135 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_136 = aux_d_flip_flop_128_0_q;
            1'd1: aux_mux_136 = data_in6;
            default: aux_mux_136 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({aux_and_9})
            1'd0: aux_mux_137 = aux_d_flip_flop_129_0_q;
            1'd1: aux_mux_137 = data_in7;
            default: aux_mux_137 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_138 = aux_d_flip_flop_10_0_q;
            3'd1: aux_mux_138 = aux_d_flip_flop_26_0_q;
            3'd2: aux_mux_138 = aux_d_flip_flop_42_0_q;
            3'd3: aux_mux_138 = aux_d_flip_flop_58_0_q;
            3'd4: aux_mux_138 = aux_d_flip_flop_74_0_q;
            3'd5: aux_mux_138 = aux_d_flip_flop_90_0_q;
            3'd6: aux_mux_138 = aux_d_flip_flop_106_0_q;
            3'd7: aux_mux_138 = aux_d_flip_flop_122_0_q;
            default: aux_mux_138 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_139 = aux_d_flip_flop_10_0_q;
            3'd1: aux_mux_139 = aux_d_flip_flop_26_0_q;
            3'd2: aux_mux_139 = aux_d_flip_flop_42_0_q;
            3'd3: aux_mux_139 = aux_d_flip_flop_58_0_q;
            3'd4: aux_mux_139 = aux_d_flip_flop_74_0_q;
            3'd5: aux_mux_139 = aux_d_flip_flop_90_0_q;
            3'd6: aux_mux_139 = aux_d_flip_flop_106_0_q;
            3'd7: aux_mux_139 = aux_d_flip_flop_122_0_q;
            default: aux_mux_139 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_140 = aux_d_flip_flop_11_0_q;
            3'd1: aux_mux_140 = aux_d_flip_flop_27_0_q;
            3'd2: aux_mux_140 = aux_d_flip_flop_43_0_q;
            3'd3: aux_mux_140 = aux_d_flip_flop_59_0_q;
            3'd4: aux_mux_140 = aux_d_flip_flop_75_0_q;
            3'd5: aux_mux_140 = aux_d_flip_flop_91_0_q;
            3'd6: aux_mux_140 = aux_d_flip_flop_107_0_q;
            3'd7: aux_mux_140 = aux_d_flip_flop_123_0_q;
            default: aux_mux_140 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_141 = aux_d_flip_flop_11_0_q;
            3'd1: aux_mux_141 = aux_d_flip_flop_27_0_q;
            3'd2: aux_mux_141 = aux_d_flip_flop_43_0_q;
            3'd3: aux_mux_141 = aux_d_flip_flop_59_0_q;
            3'd4: aux_mux_141 = aux_d_flip_flop_75_0_q;
            3'd5: aux_mux_141 = aux_d_flip_flop_91_0_q;
            3'd6: aux_mux_141 = aux_d_flip_flop_107_0_q;
            3'd7: aux_mux_141 = aux_d_flip_flop_123_0_q;
            default: aux_mux_141 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_142 = aux_d_flip_flop_12_0_q;
            3'd1: aux_mux_142 = aux_d_flip_flop_28_0_q;
            3'd2: aux_mux_142 = aux_d_flip_flop_44_0_q;
            3'd3: aux_mux_142 = aux_d_flip_flop_60_0_q;
            3'd4: aux_mux_142 = aux_d_flip_flop_76_0_q;
            3'd5: aux_mux_142 = aux_d_flip_flop_92_0_q;
            3'd6: aux_mux_142 = aux_d_flip_flop_108_0_q;
            3'd7: aux_mux_142 = aux_d_flip_flop_124_0_q;
            default: aux_mux_142 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_143 = aux_d_flip_flop_12_0_q;
            3'd1: aux_mux_143 = aux_d_flip_flop_28_0_q;
            3'd2: aux_mux_143 = aux_d_flip_flop_44_0_q;
            3'd3: aux_mux_143 = aux_d_flip_flop_60_0_q;
            3'd4: aux_mux_143 = aux_d_flip_flop_76_0_q;
            3'd5: aux_mux_143 = aux_d_flip_flop_92_0_q;
            3'd6: aux_mux_143 = aux_d_flip_flop_108_0_q;
            3'd7: aux_mux_143 = aux_d_flip_flop_124_0_q;
            default: aux_mux_143 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_144 = aux_d_flip_flop_13_0_q;
            3'd1: aux_mux_144 = aux_d_flip_flop_29_0_q;
            3'd2: aux_mux_144 = aux_d_flip_flop_45_0_q;
            3'd3: aux_mux_144 = aux_d_flip_flop_61_0_q;
            3'd4: aux_mux_144 = aux_d_flip_flop_77_0_q;
            3'd5: aux_mux_144 = aux_d_flip_flop_93_0_q;
            3'd6: aux_mux_144 = aux_d_flip_flop_109_0_q;
            3'd7: aux_mux_144 = aux_d_flip_flop_125_0_q;
            default: aux_mux_144 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_145 = aux_d_flip_flop_13_0_q;
            3'd1: aux_mux_145 = aux_d_flip_flop_29_0_q;
            3'd2: aux_mux_145 = aux_d_flip_flop_45_0_q;
            3'd3: aux_mux_145 = aux_d_flip_flop_61_0_q;
            3'd4: aux_mux_145 = aux_d_flip_flop_77_0_q;
            3'd5: aux_mux_145 = aux_d_flip_flop_93_0_q;
            3'd6: aux_mux_145 = aux_d_flip_flop_109_0_q;
            3'd7: aux_mux_145 = aux_d_flip_flop_125_0_q;
            default: aux_mux_145 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_146 = aux_d_flip_flop_14_0_q;
            3'd1: aux_mux_146 = aux_d_flip_flop_30_0_q;
            3'd2: aux_mux_146 = aux_d_flip_flop_46_0_q;
            3'd3: aux_mux_146 = aux_d_flip_flop_62_0_q;
            3'd4: aux_mux_146 = aux_d_flip_flop_78_0_q;
            3'd5: aux_mux_146 = aux_d_flip_flop_94_0_q;
            3'd6: aux_mux_146 = aux_d_flip_flop_110_0_q;
            3'd7: aux_mux_146 = aux_d_flip_flop_126_0_q;
            default: aux_mux_146 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_147 = aux_d_flip_flop_14_0_q;
            3'd1: aux_mux_147 = aux_d_flip_flop_30_0_q;
            3'd2: aux_mux_147 = aux_d_flip_flop_46_0_q;
            3'd3: aux_mux_147 = aux_d_flip_flop_62_0_q;
            3'd4: aux_mux_147 = aux_d_flip_flop_78_0_q;
            3'd5: aux_mux_147 = aux_d_flip_flop_94_0_q;
            3'd6: aux_mux_147 = aux_d_flip_flop_110_0_q;
            3'd7: aux_mux_147 = aux_d_flip_flop_126_0_q;
            default: aux_mux_147 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_148 = aux_d_flip_flop_15_0_q;
            3'd1: aux_mux_148 = aux_d_flip_flop_31_0_q;
            3'd2: aux_mux_148 = aux_d_flip_flop_47_0_q;
            3'd3: aux_mux_148 = aux_d_flip_flop_63_0_q;
            3'd4: aux_mux_148 = aux_d_flip_flop_79_0_q;
            3'd5: aux_mux_148 = aux_d_flip_flop_95_0_q;
            3'd6: aux_mux_148 = aux_d_flip_flop_111_0_q;
            3'd7: aux_mux_148 = aux_d_flip_flop_127_0_q;
            default: aux_mux_148 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_149 = aux_d_flip_flop_15_0_q;
            3'd1: aux_mux_149 = aux_d_flip_flop_31_0_q;
            3'd2: aux_mux_149 = aux_d_flip_flop_47_0_q;
            3'd3: aux_mux_149 = aux_d_flip_flop_63_0_q;
            3'd4: aux_mux_149 = aux_d_flip_flop_79_0_q;
            3'd5: aux_mux_149 = aux_d_flip_flop_95_0_q;
            3'd6: aux_mux_149 = aux_d_flip_flop_111_0_q;
            3'd7: aux_mux_149 = aux_d_flip_flop_127_0_q;
            default: aux_mux_149 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_150 = aux_d_flip_flop_16_0_q;
            3'd1: aux_mux_150 = aux_d_flip_flop_32_0_q;
            3'd2: aux_mux_150 = aux_d_flip_flop_48_0_q;
            3'd3: aux_mux_150 = aux_d_flip_flop_64_0_q;
            3'd4: aux_mux_150 = aux_d_flip_flop_80_0_q;
            3'd5: aux_mux_150 = aux_d_flip_flop_96_0_q;
            3'd6: aux_mux_150 = aux_d_flip_flop_112_0_q;
            3'd7: aux_mux_150 = aux_d_flip_flop_128_0_q;
            default: aux_mux_150 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_151 = aux_d_flip_flop_16_0_q;
            3'd1: aux_mux_151 = aux_d_flip_flop_32_0_q;
            3'd2: aux_mux_151 = aux_d_flip_flop_48_0_q;
            3'd3: aux_mux_151 = aux_d_flip_flop_64_0_q;
            3'd4: aux_mux_151 = aux_d_flip_flop_80_0_q;
            3'd5: aux_mux_151 = aux_d_flip_flop_96_0_q;
            3'd6: aux_mux_151 = aux_d_flip_flop_112_0_q;
            3'd7: aux_mux_151 = aux_d_flip_flop_128_0_q;
            default: aux_mux_151 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr12, read_addr11, read_addr10})
            3'd0: aux_mux_152 = aux_d_flip_flop_17_0_q;
            3'd1: aux_mux_152 = aux_d_flip_flop_33_0_q;
            3'd2: aux_mux_152 = aux_d_flip_flop_49_0_q;
            3'd3: aux_mux_152 = aux_d_flip_flop_65_0_q;
            3'd4: aux_mux_152 = aux_d_flip_flop_81_0_q;
            3'd5: aux_mux_152 = aux_d_flip_flop_97_0_q;
            3'd6: aux_mux_152 = aux_d_flip_flop_113_0_q;
            3'd7: aux_mux_152 = aux_d_flip_flop_129_0_q;
            default: aux_mux_152 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({read_addr22, read_addr21, read_addr20})
            3'd0: aux_mux_153 = aux_d_flip_flop_17_0_q;
            3'd1: aux_mux_153 = aux_d_flip_flop_33_0_q;
            3'd2: aux_mux_153 = aux_d_flip_flop_49_0_q;
            3'd3: aux_mux_153 = aux_d_flip_flop_65_0_q;
            3'd4: aux_mux_153 = aux_d_flip_flop_81_0_q;
            3'd5: aux_mux_153 = aux_d_flip_flop_97_0_q;
            3'd6: aux_mux_153 = aux_d_flip_flop_113_0_q;
            3'd7: aux_mux_153 = aux_d_flip_flop_129_0_q;
            default: aux_mux_153 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign read_data10 = aux_mux_138;
assign read_data11 = aux_mux_140;
assign read_data12 = aux_mux_142;
assign read_data13 = aux_mux_144;
assign read_data14 = aux_mux_146;
assign read_data15 = aux_mux_148;
assign read_data16 = aux_mux_150;
assign read_data17 = aux_mux_152;
assign read_data20 = aux_mux_139;
assign read_data21 = aux_mux_141;
assign read_data22 = aux_mux_143;
assign read_data23 = aux_mux_145;
assign read_data24 = aux_mux_147;
assign read_data25 = aux_mux_149;
assign read_data26 = aux_mux_151;
assign read_data27 = aux_mux_153;
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
