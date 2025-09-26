// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits_counter
// Generated: Fri Sep 26 19:56:48 2025
// Target FPGA: Generic-Small
// Resource Usage: 42/1000 LUTs, 35/1000 FFs, 11/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module display_4bits_counter (
    // ========= Input Ports =========
    input wire input_push_button1_1,
    input wire input_clock2_2,

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_3,
    output wire output_7_segment_display1_f_upper_left_4,
    output wire output_7_segment_display1_e_lower_left_5,
    output wire output_7_segment_display1_d_bottom_6,
    output wire output_7_segment_display1_a_top_7,
    output wire output_7_segment_display1_b_upper_right_8,
    output wire output_7_segment_display1_dp_dot_9,
    output wire output_7_segment_display1_c_lower_right_10
);

    // ========= Internal Signals =========
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_node_21_0;
    wire ic_jkflipflop_ic_node_22_0;
    wire ic_jkflipflop_ic_node_23_0;
    wire ic_jkflipflop_ic_node_24_0;
    wire ic_jkflipflop_ic_node_25_0;
    wire ic_jkflipflop_ic_node_26_0;
    wire ic_dflipflop_ic_node_27_0;
    wire ic_dflipflop_ic_node_28_0;
    wire ic_dflipflop_ic_node_29_0;
    wire ic_dflipflop_ic_node_30_0;
    wire ic_dflipflop_ic_nand_31_0;
    wire ic_dflipflop_ic_node_32_0;
    wire ic_dflipflop_ic_nand_33_0;
    wire ic_dflipflop_ic_not_34_0;
    wire ic_dflipflop_ic_nand_35_0;
    wire ic_dflipflop_ic_nand_36_0;
    wire ic_dflipflop_ic_nand_37_0;
    wire ic_dflipflop_ic_nand_38_0;
    wire ic_dflipflop_ic_node_39_0;
    wire ic_dflipflop_ic_not_40_0;
    wire ic_dflipflop_ic_nand_41_0;
    wire ic_dflipflop_ic_nand_42_0;
    wire ic_dflipflop_ic_node_43_0;
    wire ic_dflipflop_ic_node_44_0;
    wire ic_dflipflop_ic_node_45_0;
    wire ic_dflipflop_ic_not_46_0;
    wire ic_dflipflop_ic_node_47_0;
    wire ic_dflipflop_ic_node_48_0;
    wire ic_dflipflop_ic_node_49_0;
    wire ic_dflipflop_ic_node_50_0;
    wire ic_dflipflop_ic_node_51_0;
    wire ic_jkflipflop_ic_node_52_0;
    wire ic_jkflipflop_ic_node_53_0;
    wire ic_jkflipflop_ic_node_54_0;
    wire ic_jkflipflop_ic_node_55_0;
    wire ic_jkflipflop_ic_node_56_0;
    wire ic_jkflipflop_ic_node_57_0;
    wire ic_jkflipflop_ic_not_58_0;
    wire ic_jkflipflop_ic_node_59_0;
    wire ic_jkflipflop_ic_and_60_0;
    wire ic_jkflipflop_ic_and_61_0;
    wire ic_jkflipflop_ic_or_62_0;
    wire ic_jkflipflop_ic_node_63_0;
    wire ic_jkflipflop_ic_node_64_0;
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_node_75_0;
    wire ic_jkflipflop_ic_node_76_0;
    wire ic_jkflipflop_ic_node_77_0;
    wire ic_jkflipflop_ic_node_78_0;
    wire ic_jkflipflop_ic_node_79_0;
    wire ic_jkflipflop_ic_node_80_0;
    wire ic_dflipflop_ic_node_81_0;
    wire ic_dflipflop_ic_node_82_0;
    wire ic_dflipflop_ic_node_83_0;
    wire ic_dflipflop_ic_node_84_0;
    wire ic_dflipflop_ic_nand_85_0;
    wire ic_dflipflop_ic_node_86_0;
    wire ic_dflipflop_ic_nand_87_0;
    wire ic_dflipflop_ic_not_88_0;
    wire ic_dflipflop_ic_nand_89_0;
    wire ic_dflipflop_ic_nand_90_0;
    wire ic_dflipflop_ic_nand_91_0;
    wire ic_dflipflop_ic_nand_92_0;
    wire ic_dflipflop_ic_node_93_0;
    wire ic_dflipflop_ic_not_94_0;
    wire ic_dflipflop_ic_nand_95_0;
    wire ic_dflipflop_ic_nand_96_0;
    wire ic_dflipflop_ic_node_97_0;
    wire ic_dflipflop_ic_node_98_0;
    wire ic_dflipflop_ic_node_99_0;
    wire ic_dflipflop_ic_not_100_0;
    wire ic_dflipflop_ic_node_101_0;
    wire ic_dflipflop_ic_node_102_0;
    wire ic_dflipflop_ic_node_103_0;
    wire ic_dflipflop_ic_node_104_0;
    wire ic_dflipflop_ic_node_105_0;
    wire ic_jkflipflop_ic_node_106_0;
    wire ic_jkflipflop_ic_node_107_0;
    wire ic_jkflipflop_ic_node_108_0;
    wire ic_jkflipflop_ic_node_109_0;
    wire ic_jkflipflop_ic_node_110_0;
    wire ic_jkflipflop_ic_node_111_0;
    wire ic_jkflipflop_ic_not_112_0;
    wire ic_jkflipflop_ic_node_113_0;
    wire ic_jkflipflop_ic_and_114_0;
    wire ic_jkflipflop_ic_and_115_0;
    wire ic_jkflipflop_ic_or_116_0;
    wire ic_jkflipflop_ic_node_117_0;
    wire ic_jkflipflop_ic_node_118_0;
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_node_129_0;
    wire ic_jkflipflop_ic_node_130_0;
    wire ic_jkflipflop_ic_node_131_0;
    wire ic_jkflipflop_ic_node_132_0;
    wire ic_jkflipflop_ic_node_133_0;
    wire ic_jkflipflop_ic_node_134_0;
    wire ic_dflipflop_ic_node_135_0;
    wire ic_dflipflop_ic_node_136_0;
    wire ic_dflipflop_ic_node_137_0;
    wire ic_dflipflop_ic_node_138_0;
    wire ic_dflipflop_ic_nand_139_0;
    wire ic_dflipflop_ic_node_140_0;
    wire ic_dflipflop_ic_nand_141_0;
    wire ic_dflipflop_ic_not_142_0;
    wire ic_dflipflop_ic_nand_143_0;
    wire ic_dflipflop_ic_nand_144_0;
    wire ic_dflipflop_ic_nand_145_0;
    wire ic_dflipflop_ic_nand_146_0;
    wire ic_dflipflop_ic_node_147_0;
    wire ic_dflipflop_ic_not_148_0;
    wire ic_dflipflop_ic_nand_149_0;
    wire ic_dflipflop_ic_nand_150_0;
    wire ic_dflipflop_ic_node_151_0;
    wire ic_dflipflop_ic_node_152_0;
    wire ic_dflipflop_ic_node_153_0;
    wire ic_dflipflop_ic_not_154_0;
    wire ic_dflipflop_ic_node_155_0;
    wire ic_dflipflop_ic_node_156_0;
    wire ic_dflipflop_ic_node_157_0;
    wire ic_dflipflop_ic_node_158_0;
    wire ic_dflipflop_ic_node_159_0;
    wire ic_jkflipflop_ic_node_160_0;
    wire ic_jkflipflop_ic_node_161_0;
    wire ic_jkflipflop_ic_node_162_0;
    wire ic_jkflipflop_ic_node_163_0;
    wire ic_jkflipflop_ic_node_164_0;
    wire ic_jkflipflop_ic_node_165_0;
    wire ic_jkflipflop_ic_not_166_0;
    wire ic_jkflipflop_ic_node_167_0;
    wire ic_jkflipflop_ic_and_168_0;
    wire ic_jkflipflop_ic_and_169_0;
    wire ic_jkflipflop_ic_or_170_0;
    wire ic_jkflipflop_ic_node_171_0;
    wire ic_jkflipflop_ic_node_172_0;
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_node_183_0;
    wire ic_jkflipflop_ic_node_184_0;
    wire ic_jkflipflop_ic_node_185_0;
    wire ic_jkflipflop_ic_node_186_0;
    wire ic_jkflipflop_ic_node_187_0;
    wire ic_jkflipflop_ic_node_188_0;
    wire ic_dflipflop_ic_node_189_0;
    wire ic_dflipflop_ic_node_190_0;
    wire ic_dflipflop_ic_node_191_0;
    wire ic_dflipflop_ic_node_192_0;
    wire ic_dflipflop_ic_nand_193_0;
    wire ic_dflipflop_ic_node_194_0;
    wire ic_dflipflop_ic_nand_195_0;
    wire ic_dflipflop_ic_not_196_0;
    wire ic_dflipflop_ic_nand_197_0;
    wire ic_dflipflop_ic_nand_198_0;
    wire ic_dflipflop_ic_nand_199_0;
    wire ic_dflipflop_ic_nand_200_0;
    wire ic_dflipflop_ic_node_201_0;
    wire ic_dflipflop_ic_not_202_0;
    wire ic_dflipflop_ic_nand_203_0;
    wire ic_dflipflop_ic_nand_204_0;
    wire ic_dflipflop_ic_node_205_0;
    wire ic_dflipflop_ic_node_206_0;
    wire ic_dflipflop_ic_node_207_0;
    wire ic_dflipflop_ic_not_208_0;
    wire ic_dflipflop_ic_node_209_0;
    wire ic_dflipflop_ic_node_210_0;
    wire ic_dflipflop_ic_node_211_0;
    wire ic_dflipflop_ic_node_212_0;
    wire ic_dflipflop_ic_node_213_0;
    wire ic_jkflipflop_ic_node_214_0;
    wire ic_jkflipflop_ic_node_215_0;
    wire ic_jkflipflop_ic_node_216_0;
    wire ic_jkflipflop_ic_node_217_0;
    wire ic_jkflipflop_ic_node_218_0;
    wire ic_jkflipflop_ic_node_219_0;
    wire ic_jkflipflop_ic_not_220_0;
    wire ic_jkflipflop_ic_node_221_0;
    wire ic_jkflipflop_ic_and_222_0;
    wire ic_jkflipflop_ic_and_223_0;
    wire ic_jkflipflop_ic_or_224_0;
    wire ic_jkflipflop_ic_node_225_0;
    wire ic_jkflipflop_ic_node_226_0;
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: DISPLAY-4BITS ==============
// IC inputs: 4, IC outputs: 8
// Nesting depth: 0
    wire ic_display_4bits_ic_node_235_0;
    wire ic_display_4bits_ic_node_236_0;
    wire ic_display_4bits_ic_node_237_0;
    wire ic_display_4bits_ic_node_238_0;
    wire ic_display_4bits_ic_node_239_0;
    wire ic_display_4bits_ic_node_240_0;
    wire ic_display_4bits_ic_not_241_0;
    wire ic_display_4bits_ic_node_242_0;
    wire ic_display_4bits_ic_node_243_0;
    wire ic_display_4bits_ic_node_244_0;
    wire ic_display_4bits_ic_not_245_0;
    wire ic_display_4bits_ic_not_246_0;
    wire ic_display_4bits_ic_node_247_0;
    wire ic_display_4bits_ic_node_248_0;
    wire ic_display_4bits_ic_node_249_0;
    wire ic_display_4bits_ic_node_250_0;
    wire ic_display_4bits_ic_node_251_0;
    wire ic_display_4bits_ic_node_252_0;
    wire ic_display_4bits_ic_node_253_0;
    wire ic_display_4bits_ic_node_254_0;
    wire ic_display_4bits_ic_node_255_0;
    wire ic_display_4bits_ic_node_256_0;
    wire ic_display_4bits_ic_node_257_0;
    wire ic_display_4bits_ic_node_258_0;
    wire ic_display_4bits_ic_node_259_0;
    wire ic_display_4bits_ic_and_260_0;
    wire ic_display_4bits_ic_node_261_0;
    wire ic_display_4bits_ic_node_262_0;
    wire ic_display_4bits_ic_node_263_0;
    wire ic_display_4bits_ic_node_264_0;
    wire ic_display_4bits_ic_node_265_0;
    wire ic_display_4bits_ic_node_266_0;
    wire ic_display_4bits_ic_and_267_0;
    wire ic_display_4bits_ic_and_268_0;
    wire ic_display_4bits_ic_node_269_0;
    wire ic_display_4bits_ic_node_270_0;
    wire ic_display_4bits_ic_node_271_0;
    wire ic_display_4bits_ic_node_272_0;
    wire ic_display_4bits_ic_node_273_0;
    wire ic_display_4bits_ic_node_274_0;
    wire ic_display_4bits_ic_node_275_0;
    wire ic_display_4bits_ic_node_276_0;
    wire ic_display_4bits_ic_node_277_0;
    wire ic_display_4bits_ic_and_278_0;
    wire ic_display_4bits_ic_node_279_0;
    wire ic_display_4bits_ic_and_280_0;
    wire ic_display_4bits_ic_node_281_0;
    wire ic_display_4bits_ic_node_282_0;
    wire ic_display_4bits_ic_node_283_0;
    wire ic_display_4bits_ic_and_284_0;
    wire ic_display_4bits_ic_and_285_0;
    wire ic_display_4bits_ic_and_286_0;
    wire ic_display_4bits_ic_and_287_0;
    wire ic_display_4bits_ic_or_288_0;
    wire ic_display_4bits_ic_or_289_0;
    wire ic_display_4bits_ic_or_290_0;
    wire ic_display_4bits_ic_or_291_0;
    wire ic_display_4bits_ic_or_292_0;
    wire ic_display_4bits_ic_or_293_0;
    wire ic_display_4bits_ic_or_294_0;
    wire ic_display_4bits_ic_not_295_0;
    wire ic_display_4bits_ic_node_296_0;
    wire ic_display_4bits_ic_node_297_0;
    wire ic_display_4bits_ic_node_298_0;
    wire ic_display_4bits_ic_node_299_0;
    wire ic_display_4bits_ic_node_300_0;
    wire ic_display_4bits_ic_node_301_0;
    wire ic_display_4bits_ic_node_302_0;
    wire ic_display_4bits_ic_node_303_0;
    wire ic_display_4bits_ic_node_304_0;
    wire ic_display_4bits_ic_node_305_0;
    wire ic_display_4bits_ic_node_306_0;
    wire ic_display_4bits_ic_node_307_0;
    wire ic_display_4bits_ic_node_308_0;
    wire ic_display_4bits_ic_node_309_0;
    wire ic_display_4bits_ic_node_310_0;
    wire ic_display_4bits_ic_node_311_0;
// ============== END IC: DISPLAY-4BITS ==============
    wire not_312;
    wire node_313;
    wire node_314;
    wire and_315;
    wire not_316;
    wire node_317;
    wire node_318;
    wire node_319;
    wire node_320;
    wire node_321;
    wire node_322;
    wire node_323;
    wire node_324;
    wire node_325;
    wire node_326;
    wire node_327;
    wire node_328;
    wire node_329;

    // ========= Logic Assignments =========

    // ========= Internal Sequential Register =========
    reg output_7_segment_display1_g_middle_3_behavioral_reg = 1'b0; // Internal sequential register

    // ========= Behavioral Sequential Logic (replaces gate-level feedback) =========
    // Industry-standard behavioral sequential logic
    always @(posedge input_clock2_2) begin
begin // Synchronous operation
            output_7_segment_display1_g_middle_3_behavioral_reg <= input_push_button1_1;
        end
    end

    assign output_7_segment_display1_g_middle_3 = output_7_segment_display1_g_middle_3_behavioral_reg; // Connect behavioral register to output
    assign output_7_segment_display1_f_upper_left_4 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_e_lower_left_5 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_d_bottom_6 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_a_top_7 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_b_upper_right_8 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_dp_dot_9 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output
    assign output_7_segment_display1_c_lower_right_10 = ~output_7_segment_display1_g_middle_3_behavioral_reg; // Complementary output

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_3 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_4 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_5 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_6 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_8 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_9 = node_325; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_10 = 1'b0; // 7-Segment Display

endmodule // display_4bits_counter

// ====================================================================
// Module display_4bits_counter generation completed successfully
// Elements processed: 26
// Inputs: 2, Outputs: 8
// Warnings: 12
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP output 1 is not connected
//   IC DISPLAY-4BITS output 6 is not connected
// ====================================================================
