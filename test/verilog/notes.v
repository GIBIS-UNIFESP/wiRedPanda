// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: notes
// Generated: Fri Sep 26 21:22:27 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 38/1000 FFs, 9/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module notes (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_buzzer1_g6_1,
    output wire output_buzzer2_f6_2,
    output wire output_buzzer3_d6_3,
    output wire output_buzzer4_b7_4,
    output wire output_buzzer5_c6_5,
    output wire output_buzzer6_a7_6,
    output wire output_buzzer7_e6_7,
    output wire output_buzzer8_c7_8
);

    // ========= Internal Signals =========
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_node_19_0;
    wire ic_jkflipflop_ic_node_20_0;
    wire ic_jkflipflop_ic_node_21_0;
    wire ic_jkflipflop_ic_node_22_0;
    wire ic_jkflipflop_ic_node_23_0;
    wire ic_jkflipflop_ic_node_24_0;
    wire ic_dflipflop_ic_node_25_0;
    wire ic_dflipflop_ic_node_26_0;
    wire ic_dflipflop_ic_node_27_0;
    wire ic_dflipflop_ic_node_28_0;
    wire ic_dflipflop_ic_nand_29_0;
    wire ic_dflipflop_ic_node_30_0;
    wire ic_dflipflop_ic_nand_31_0;
    wire ic_dflipflop_ic_not_32_0;
    wire ic_dflipflop_ic_nand_33_0;
    wire ic_dflipflop_ic_nand_34_0;
    wire ic_dflipflop_ic_nand_35_0;
    wire ic_dflipflop_ic_nand_36_0;
    wire ic_dflipflop_ic_node_37_0;
    wire ic_dflipflop_ic_not_38_0;
    wire ic_dflipflop_ic_nand_39_0;
    wire ic_dflipflop_ic_nand_40_0;
    wire ic_dflipflop_ic_node_41_0;
    wire ic_dflipflop_ic_node_42_0;
    wire ic_dflipflop_ic_node_43_0;
    wire ic_dflipflop_ic_not_44_0;
    wire ic_dflipflop_ic_node_45_0;
    wire ic_dflipflop_ic_node_46_0;
    wire ic_dflipflop_ic_node_47_0;
    wire ic_dflipflop_ic_node_48_0;
    wire ic_dflipflop_ic_node_49_0;
    wire ic_jkflipflop_ic_node_50_0;
    wire ic_jkflipflop_ic_node_51_0;
    wire ic_jkflipflop_ic_node_52_0;
    wire ic_jkflipflop_ic_node_53_0;
    wire ic_jkflipflop_ic_node_54_0;
    wire ic_jkflipflop_ic_node_55_0;
    wire ic_jkflipflop_ic_not_56_0;
    wire ic_jkflipflop_ic_node_57_0;
    wire ic_jkflipflop_ic_and_58_0;
    wire ic_jkflipflop_ic_and_59_0;
    wire ic_jkflipflop_ic_or_60_0;
    wire ic_jkflipflop_ic_node_61_0;
    wire ic_jkflipflop_ic_node_62_0;
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: DECODER ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
    wire ic_decoder_ic_node_69_0;
    wire ic_decoder_ic_node_70_0;
    wire ic_decoder_ic_node_71_0;
    wire ic_decoder_ic_node_72_0;
    wire ic_decoder_ic_node_73_0;
    wire ic_decoder_ic_node_74_0;
    wire ic_decoder_ic_not_75_0;
    wire ic_decoder_ic_not_76_0;
    wire ic_decoder_ic_not_77_0;
    wire ic_decoder_ic_node_78_0;
    wire ic_decoder_ic_node_79_0;
    wire ic_decoder_ic_node_80_0;
    wire ic_decoder_ic_node_81_0;
    wire ic_decoder_ic_node_82_0;
    wire ic_decoder_ic_node_83_0;
    wire ic_decoder_ic_node_84_0;
    wire ic_decoder_ic_node_85_0;
    wire ic_decoder_ic_node_86_0;
    wire ic_decoder_ic_node_87_0;
    wire ic_decoder_ic_node_88_0;
    wire ic_decoder_ic_node_89_0;
    wire ic_decoder_ic_node_90_0;
    wire ic_decoder_ic_node_91_0;
    wire ic_decoder_ic_node_92_0;
    wire ic_decoder_ic_node_93_0;
    wire ic_decoder_ic_node_94_0;
    wire ic_decoder_ic_node_95_0;
    wire ic_decoder_ic_node_96_0;
    wire ic_decoder_ic_node_97_0;
    wire ic_decoder_ic_node_98_0;
    wire ic_decoder_ic_node_99_0;
    wire ic_decoder_ic_node_100_0;
    wire ic_decoder_ic_node_101_0;
    wire ic_decoder_ic_and_102_0;
    wire ic_decoder_ic_and_103_0;
    wire ic_decoder_ic_and_104_0;
    wire ic_decoder_ic_and_105_0;
    wire ic_decoder_ic_and_106_0;
    wire ic_decoder_ic_and_107_0;
    wire ic_decoder_ic_and_108_0;
    wire ic_decoder_ic_and_109_0;
    wire ic_decoder_ic_node_110_0;
    wire ic_decoder_ic_node_111_0;
    wire ic_decoder_ic_node_112_0;
    wire ic_decoder_ic_node_113_0;
    wire ic_decoder_ic_node_114_0;
    wire ic_decoder_ic_node_115_0;
    wire ic_decoder_ic_node_116_0;
    wire ic_decoder_ic_node_117_0;
    wire ic_decoder_ic_node_118_0;
    wire ic_decoder_ic_node_119_0;
    wire ic_decoder_ic_node_120_0;
    wire ic_decoder_ic_node_121_0;
    wire ic_decoder_ic_node_122_0;
    wire ic_decoder_ic_node_123_0;
// ============== END IC: DECODER ==============
    reg jk_flip_flop_124_0_q = 1'b0;
    reg jk_flip_flop_124_1_q = 1'b0;
    reg jk_flip_flop_125_0_q = 1'b0;
    reg jk_flip_flop_125_1_q = 1'b0;
    reg jk_flip_flop_126_0_q = 1'b0;
    reg jk_flip_flop_126_1_q = 1'b0;

    // ========= Logic Assignments =========
    wire ic_decoder_ic_node_127_0;
    wire ic_decoder_ic_node_128_0;
    wire ic_decoder_ic_node_129_0;
    wire ic_decoder_ic_node_130_0;
    wire ic_decoder_ic_node_131_0;
    wire ic_decoder_ic_node_132_0;
    wire ic_decoder_ic_not_133_0;
    wire ic_decoder_ic_not_134_0;
    wire ic_decoder_ic_not_135_0;
    wire ic_decoder_ic_node_136_0;
    wire ic_decoder_ic_node_137_0;
    wire ic_decoder_ic_node_138_0;
    wire ic_decoder_ic_node_139_0;
    wire ic_decoder_ic_node_140_0;
    wire ic_decoder_ic_node_141_0;
    wire ic_decoder_ic_node_142_0;
    wire ic_decoder_ic_node_143_0;
    wire ic_decoder_ic_node_144_0;
    wire ic_decoder_ic_node_145_0;
    wire ic_decoder_ic_node_146_0;
    wire ic_decoder_ic_node_147_0;
    wire ic_decoder_ic_node_148_0;
    wire ic_decoder_ic_node_149_0;
    wire ic_decoder_ic_node_150_0;
    wire ic_decoder_ic_node_151_0;
    wire ic_decoder_ic_node_152_0;
    wire ic_decoder_ic_node_153_0;
    wire ic_decoder_ic_node_154_0;
    wire ic_decoder_ic_node_155_0;
    wire ic_decoder_ic_node_156_0;
    wire ic_decoder_ic_node_157_0;
    wire ic_decoder_ic_node_158_0;
    wire ic_decoder_ic_node_159_0;
    wire ic_decoder_ic_and_160_0;
    wire ic_decoder_ic_and_161_0;
    wire ic_decoder_ic_and_162_0;
    wire ic_decoder_ic_and_163_0;
    wire ic_decoder_ic_and_164_0;
    wire ic_decoder_ic_and_165_0;
    wire ic_decoder_ic_and_166_0;
    wire ic_decoder_ic_and_167_0;
    wire ic_decoder_ic_node_168_0;
    wire ic_decoder_ic_node_169_0;
    wire ic_decoder_ic_node_170_0;
    wire ic_decoder_ic_node_171_0;
    wire ic_decoder_ic_node_172_0;
    wire ic_decoder_ic_node_173_0;
    wire ic_decoder_ic_node_174_0;
    wire ic_decoder_ic_node_175_0;
    wire ic_decoder_ic_node_176_0;
    wire ic_decoder_ic_node_177_0;
    wire ic_decoder_ic_node_178_0;
    wire ic_decoder_ic_node_179_0;
    wire ic_decoder_ic_node_180_0;
    wire ic_decoder_ic_node_181_0;
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_125_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_126_0_q <= 1'b0; jk_flip_flop_126_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_126_0_q <= 1'b1; jk_flip_flop_126_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_126_0_q <= jk_flip_flop_126_1_q; jk_flip_flop_126_1_q <= jk_flip_flop_126_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_124_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_125_0_q <= 1'b0; jk_flip_flop_125_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_125_0_q <= 1'b1; jk_flip_flop_125_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_125_0_q <= jk_flip_flop_125_1_q; jk_flip_flop_125_1_q <= jk_flip_flop_125_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        jk_flip_flop_124_0_q = 1'b0;
        jk_flip_flop_124_1_q = 1'b1;
    end

    wire ic_jkflipflop_ic_node_182_0;
    wire ic_jkflipflop_ic_node_183_0;
    wire ic_jkflipflop_ic_node_184_0;
    wire ic_jkflipflop_ic_node_185_0;
    wire ic_jkflipflop_ic_node_186_0;
    wire ic_jkflipflop_ic_node_187_0;
    wire ic_dflipflop_ic_node_188_0;
    wire ic_dflipflop_ic_node_189_0;
    wire ic_dflipflop_ic_node_190_0;
    wire ic_dflipflop_ic_node_191_0;
    wire ic_dflipflop_ic_nand_192_0;
    wire ic_dflipflop_ic_node_193_0;
    wire ic_dflipflop_ic_nand_194_0;
    wire ic_dflipflop_ic_not_195_0;
    wire ic_dflipflop_ic_nand_196_0;
    wire ic_dflipflop_ic_nand_197_0;
    wire ic_dflipflop_ic_nand_198_0;
    wire ic_dflipflop_ic_nand_199_0;
    wire ic_dflipflop_ic_node_200_0;
    wire ic_dflipflop_ic_not_201_0;
    wire ic_dflipflop_ic_nand_202_0;
    wire ic_dflipflop_ic_nand_203_0;
    wire ic_dflipflop_ic_node_204_0;
    wire ic_dflipflop_ic_node_205_0;
    wire ic_dflipflop_ic_node_206_0;
    wire ic_dflipflop_ic_not_207_0;
    wire ic_dflipflop_ic_node_208_0;
    wire ic_dflipflop_ic_node_209_0;
    wire ic_dflipflop_ic_node_210_0;
    wire ic_dflipflop_ic_node_211_0;
    wire ic_dflipflop_ic_node_212_0;
    wire ic_jkflipflop_ic_node_213_0;
    wire ic_jkflipflop_ic_node_214_0;
    wire ic_jkflipflop_ic_node_215_0;
    wire ic_jkflipflop_ic_node_216_0;
    wire ic_jkflipflop_ic_node_217_0;
    wire ic_jkflipflop_ic_node_218_0;
    wire ic_jkflipflop_ic_not_219_0;
    wire ic_jkflipflop_ic_node_220_0;
    wire ic_jkflipflop_ic_and_221_0;
    wire ic_jkflipflop_ic_and_222_0;
    wire ic_jkflipflop_ic_or_223_0;
    wire ic_jkflipflop_ic_node_224_0;
    wire ic_jkflipflop_ic_node_225_0;

    // ========= Output Assignments =========
    assign output_buzzer1_g6_1 = 1'b0; // Buzzer
    assign output_buzzer2_f6_2 = 1'b0; // Buzzer
    assign output_buzzer3_d6_3 = 1'b0; // Buzzer
    assign output_buzzer4_b7_4 = 1'b0; // Buzzer
    assign output_buzzer5_c6_5 = 1'b0; // Buzzer
    assign output_buzzer6_a7_6 = 1'b0; // Buzzer
    assign output_buzzer7_e6_7 = 1'b0; // Buzzer
    assign output_buzzer8_c7_8 = 1'b0; // Buzzer

endmodule // notes

// ====================================================================
// Module notes generation completed successfully
// Elements processed: 14
// Inputs: 0, Outputs: 8
// Warnings: 5
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 4 is not connected
//   IC JKFLIPFLOP output 1 is not connected
// ====================================================================
