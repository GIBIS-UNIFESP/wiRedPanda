// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: ic
// Generated: Fri Sep 26 21:36:32 2025
// Target FPGA: Generic-Small
// Resource Usage: 58/1000 LUTs, 215/1000 FFs, 37/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module ic (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_7_segment_display1_g_middle_1,
    output wire output_7_segment_display1_f_upper_left_2,
    output wire output_7_segment_display1_e_lower_left_3,
    output wire output_7_segment_display1_d_bottom_4,
    output wire output_7_segment_display1_a_top_5,
    output wire output_7_segment_display1_b_upper_right_6,
    output wire output_7_segment_display1_dp_dot_7,
    output wire output_7_segment_display1_c_lower_right_8,
    output wire output_7_segment_display2_g_middle_9,
    output wire output_7_segment_display2_f_upper_left_10,
    output wire output_7_segment_display2_e_lower_left_11,
    output wire output_7_segment_display2_d_bottom_12,
    output wire output_7_segment_display2_a_top_13,
    output wire output_7_segment_display2_b_upper_right_14,
    output wire output_7_segment_display2_dp_dot_15,
    output wire output_7_segment_display2_c_lower_right_16,
    output wire output_7_segment_display3_g_middle_17,
    output wire output_7_segment_display3_f_upper_left_18,
    output wire output_7_segment_display3_e_lower_left_19,
    output wire output_7_segment_display3_d_bottom_20,
    output wire output_7_segment_display3_a_top_21,
    output wire output_7_segment_display3_b_upper_right_22,
    output wire output_7_segment_display3_dp_dot_23,
    output wire output_7_segment_display3_c_lower_right_24,
    output wire output_led4_0_25,
    output wire output_led5_0_26,
    output wire output_led6_0_27,
    output wire output_led7_0_28
);

    // ========= Internal Signals =========
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_node_39_0;
    wire ic_jkflipflop_ic_node_40_0;
    wire ic_jkflipflop_ic_node_41_0;
    wire ic_jkflipflop_ic_node_42_0;
    wire ic_jkflipflop_ic_node_43_0;
    wire ic_jkflipflop_ic_node_44_0;
    wire ic_dflipflop_ic_node_45_0;
    wire ic_dflipflop_ic_node_46_0;
    wire ic_dflipflop_ic_node_47_0;
    wire ic_dflipflop_ic_node_48_0;
    wire ic_dflipflop_ic_nand_49_0;
    wire ic_dflipflop_ic_node_50_0;
    wire ic_dflipflop_ic_nand_51_0;
    wire ic_dflipflop_ic_not_52_0;
    wire ic_dflipflop_ic_nand_53_0;
    wire ic_dflipflop_ic_nand_54_0;
    wire ic_dflipflop_ic_nand_55_0;
    wire ic_dflipflop_ic_nand_56_0;
    wire ic_dflipflop_ic_node_57_0;
    wire ic_dflipflop_ic_not_58_0;
    wire ic_dflipflop_ic_nand_59_0;
    wire ic_dflipflop_ic_nand_60_0;
    wire ic_dflipflop_ic_node_61_0;
    wire ic_dflipflop_ic_node_62_0;
    wire ic_dflipflop_ic_node_63_0;
    wire ic_dflipflop_ic_not_64_0;
    wire ic_dflipflop_ic_node_65_0;
    wire ic_dflipflop_ic_node_66_0;
    wire ic_dflipflop_ic_node_67_0;
    wire ic_dflipflop_ic_node_68_0;
    wire ic_dflipflop_ic_node_69_0;
    wire ic_jkflipflop_ic_node_70_0;
    wire ic_jkflipflop_ic_node_71_0;
    wire ic_jkflipflop_ic_node_72_0;
    wire ic_jkflipflop_ic_node_73_0;
    wire ic_jkflipflop_ic_node_74_0;
    wire ic_jkflipflop_ic_node_75_0;
    wire ic_jkflipflop_ic_not_76_0;
    wire ic_jkflipflop_ic_node_77_0;
    wire ic_jkflipflop_ic_and_78_0;
    wire ic_jkflipflop_ic_and_79_0;
    wire ic_jkflipflop_ic_or_80_0;
    wire ic_jkflipflop_ic_node_81_0;
    wire ic_jkflipflop_ic_node_82_0;
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: INPUT ==============
// IC inputs: 2, IC outputs: 4
// Nesting depth: 0
    wire ic_input_ic_node_87_0;
    wire ic_input_ic_node_88_0;
    wire ic_input_ic_not_89_0;
    wire ic_input_ic_xor_90_0;
    wire ic_input_ic_or_91_0;
    wire ic_input_ic_and_92_0;
    wire ic_input_ic_node_93_0;
    wire ic_input_ic_node_94_0;
    wire ic_input_ic_node_95_0;
    wire ic_input_ic_node_96_0;
// ============== END IC: INPUT ==============
// ============== BEGIN IC: DISPLAY-4BITS ==============
// IC inputs: 4, IC outputs: 8
// Nesting depth: 0
    wire ic_display_4bits_ic_node_105_0;
    wire ic_display_4bits_ic_node_106_0;
    wire ic_display_4bits_ic_node_107_0;
    wire ic_display_4bits_ic_node_108_0;
    wire ic_display_4bits_ic_node_109_0;
    wire ic_display_4bits_ic_node_110_0;
    wire ic_display_4bits_ic_not_111_0;
    wire ic_display_4bits_ic_node_112_0;
    wire ic_display_4bits_ic_node_113_0;
    wire ic_display_4bits_ic_node_114_0;
    wire ic_display_4bits_ic_not_115_0;
    wire ic_display_4bits_ic_not_116_0;
    wire ic_display_4bits_ic_node_117_0;
    wire ic_display_4bits_ic_node_118_0;
    wire ic_display_4bits_ic_node_119_0;
    wire ic_display_4bits_ic_node_120_0;
    wire ic_display_4bits_ic_node_121_0;
    wire ic_display_4bits_ic_node_122_0;
    wire ic_display_4bits_ic_node_123_0;
    wire ic_display_4bits_ic_node_124_0;
    wire ic_display_4bits_ic_node_125_0;
    wire ic_display_4bits_ic_node_126_0;
    wire ic_display_4bits_ic_node_127_0;
    wire ic_display_4bits_ic_node_128_0;
    wire ic_display_4bits_ic_node_129_0;
    wire ic_display_4bits_ic_and_130_0;
    wire ic_display_4bits_ic_node_131_0;
    wire ic_display_4bits_ic_node_132_0;
    wire ic_display_4bits_ic_node_133_0;
    wire ic_display_4bits_ic_node_134_0;
    wire ic_display_4bits_ic_node_135_0;
    wire ic_display_4bits_ic_node_136_0;
    wire ic_display_4bits_ic_and_137_0;
    wire ic_display_4bits_ic_and_138_0;
    wire ic_display_4bits_ic_node_139_0;
    wire ic_display_4bits_ic_node_140_0;
    wire ic_display_4bits_ic_node_141_0;
    wire ic_display_4bits_ic_node_142_0;
    wire ic_display_4bits_ic_node_143_0;
    wire ic_display_4bits_ic_node_144_0;
    wire ic_display_4bits_ic_node_145_0;
    wire ic_display_4bits_ic_node_146_0;
    wire ic_display_4bits_ic_node_147_0;
    wire ic_display_4bits_ic_and_148_0;
    wire ic_display_4bits_ic_node_149_0;
    wire ic_display_4bits_ic_and_150_0;
    wire ic_display_4bits_ic_node_151_0;
    wire ic_display_4bits_ic_node_152_0;
    wire ic_display_4bits_ic_node_153_0;
    wire ic_display_4bits_ic_and_154_0;
    wire ic_display_4bits_ic_and_155_0;
    wire ic_display_4bits_ic_and_156_0;
    wire ic_display_4bits_ic_and_157_0;
    wire ic_display_4bits_ic_or_158_0;
    wire ic_display_4bits_ic_or_159_0;
    wire ic_display_4bits_ic_or_160_0;
    wire ic_display_4bits_ic_or_161_0;
    wire ic_display_4bits_ic_or_162_0;
    wire ic_display_4bits_ic_or_163_0;
    wire ic_display_4bits_ic_or_164_0;
    wire ic_display_4bits_ic_not_165_0;
    wire ic_display_4bits_ic_node_166_0;
    wire ic_display_4bits_ic_node_167_0;
    wire ic_display_4bits_ic_node_168_0;
    wire ic_display_4bits_ic_node_169_0;
    wire ic_display_4bits_ic_node_170_0;
    wire ic_display_4bits_ic_node_171_0;
    wire ic_display_4bits_ic_node_172_0;
    wire ic_display_4bits_ic_node_173_0;
    wire ic_display_4bits_ic_node_174_0;
    wire ic_display_4bits_ic_node_175_0;
    wire ic_display_4bits_ic_node_176_0;
    wire ic_display_4bits_ic_node_177_0;
    wire ic_display_4bits_ic_node_178_0;
    wire ic_display_4bits_ic_node_179_0;
    wire ic_display_4bits_ic_node_180_0;
    wire ic_display_4bits_ic_node_181_0;
// ============== END IC: DISPLAY-4BITS ==============
// ============== BEGIN IC: DISPLAY-4BITS ==============
// IC inputs: 4, IC outputs: 8
// Nesting depth: 0
    wire ic_display_4bits_ic_node_190_0;
    wire ic_display_4bits_ic_node_191_0;
    wire ic_display_4bits_ic_node_192_0;
    wire ic_display_4bits_ic_node_193_0;
    wire ic_display_4bits_ic_node_194_0;
    wire ic_display_4bits_ic_node_195_0;
    wire ic_display_4bits_ic_not_196_0;
    wire ic_display_4bits_ic_node_197_0;
    wire ic_display_4bits_ic_node_198_0;
    wire ic_display_4bits_ic_node_199_0;
    wire ic_display_4bits_ic_not_200_0;
    wire ic_display_4bits_ic_not_201_0;
    wire ic_display_4bits_ic_node_202_0;
    wire ic_display_4bits_ic_node_203_0;
    wire ic_display_4bits_ic_node_204_0;
    wire ic_display_4bits_ic_node_205_0;
    wire ic_display_4bits_ic_node_206_0;
    wire ic_display_4bits_ic_node_207_0;
    wire ic_display_4bits_ic_node_208_0;
    wire ic_display_4bits_ic_node_209_0;
    wire ic_display_4bits_ic_node_210_0;
    wire ic_display_4bits_ic_node_211_0;
    wire ic_display_4bits_ic_node_212_0;
    wire ic_display_4bits_ic_node_213_0;
    wire ic_display_4bits_ic_node_214_0;
    wire ic_display_4bits_ic_and_215_0;
    wire ic_display_4bits_ic_node_216_0;
    wire ic_display_4bits_ic_node_217_0;
    wire ic_display_4bits_ic_node_218_0;
    wire ic_display_4bits_ic_node_219_0;
    wire ic_display_4bits_ic_node_220_0;
    wire ic_display_4bits_ic_node_221_0;
    wire ic_display_4bits_ic_and_222_0;
    wire ic_display_4bits_ic_and_223_0;
    wire ic_display_4bits_ic_node_224_0;
    wire ic_display_4bits_ic_node_225_0;
    wire ic_display_4bits_ic_node_226_0;
    wire ic_display_4bits_ic_node_227_0;
    wire ic_display_4bits_ic_node_228_0;
    wire ic_display_4bits_ic_node_229_0;
    wire ic_display_4bits_ic_node_230_0;
    wire ic_display_4bits_ic_node_231_0;
    wire ic_display_4bits_ic_node_232_0;
    wire ic_display_4bits_ic_and_233_0;
    wire ic_display_4bits_ic_node_234_0;
    wire ic_display_4bits_ic_and_235_0;
    wire ic_display_4bits_ic_node_236_0;
    wire ic_display_4bits_ic_node_237_0;
    wire ic_display_4bits_ic_node_238_0;
    wire ic_display_4bits_ic_and_239_0;
    wire ic_display_4bits_ic_and_240_0;
    wire ic_display_4bits_ic_and_241_0;
    wire ic_display_4bits_ic_and_242_0;
    wire ic_display_4bits_ic_or_243_0;
    wire ic_display_4bits_ic_or_244_0;
    wire ic_display_4bits_ic_or_245_0;
    wire ic_display_4bits_ic_or_246_0;
    wire ic_display_4bits_ic_or_247_0;
    wire ic_display_4bits_ic_or_248_0;
    wire ic_display_4bits_ic_or_249_0;
    wire ic_display_4bits_ic_not_250_0;
    wire ic_display_4bits_ic_node_251_0;
    wire ic_display_4bits_ic_node_252_0;
    wire ic_display_4bits_ic_node_253_0;
    wire ic_display_4bits_ic_node_254_0;
    wire ic_display_4bits_ic_node_255_0;
    wire ic_display_4bits_ic_node_256_0;
    wire ic_display_4bits_ic_node_257_0;
    wire ic_display_4bits_ic_node_258_0;
    wire ic_display_4bits_ic_node_259_0;
    wire ic_display_4bits_ic_node_260_0;
    wire ic_display_4bits_ic_node_261_0;
    wire ic_display_4bits_ic_node_262_0;
    wire ic_display_4bits_ic_node_263_0;
    wire ic_display_4bits_ic_node_264_0;
    wire ic_display_4bits_ic_node_265_0;
    wire ic_display_4bits_ic_node_266_0;
// ============== END IC: DISPLAY-4BITS ==============
// ============== BEGIN IC: DISPLAY-3BITS ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
    wire ic_display_3bits_ic_node_273_0;
    wire ic_display_3bits_ic_node_274_0;
    wire ic_display_3bits_ic_node_275_0;
    wire ic_display_3bits_ic_gnd_276_0;
    wire ic_display_3bits_ic_or_277_0;
    wire ic_display_3bits_ic_or_278_0;
    wire ic_display_3bits_ic_or_279_0;
    wire ic_display_3bits_ic_not_280_0;
    wire ic_display_3bits_ic_not_281_0;
    wire ic_display_3bits_ic_not_282_0;
    wire ic_display_3bits_ic_and_283_0;
    wire ic_display_3bits_ic_and_284_0;
    wire ic_display_3bits_ic_and_285_0;
    wire ic_display_3bits_ic_and_286_0;
    wire ic_display_3bits_ic_xnor_287_0;
    wire ic_display_3bits_ic_and_288_0;
    wire ic_display_3bits_ic_and_289_0;
    wire ic_display_3bits_ic_and_290_0;
    wire ic_display_3bits_ic_and_291_0;
    wire ic_display_3bits_ic_and_292_0;
    wire ic_display_3bits_ic_xnor_293_0;
    wire ic_display_3bits_ic_gnd_294_0;
    wire ic_display_3bits_ic_and_295_0;
    wire ic_display_3bits_ic_node_296_0;
    wire ic_display_3bits_ic_or_297_0;
    wire ic_display_3bits_ic_or_298_0;
    wire ic_display_3bits_ic_nand_299_0;
    wire ic_display_3bits_ic_or_300_0;
    wire ic_display_3bits_ic_or_301_0;
    wire ic_display_3bits_ic_or_302_0;
    wire ic_display_3bits_ic_or_303_0;
    wire ic_display_3bits_ic_node_304_0;
    wire ic_display_3bits_ic_node_305_0;
    wire ic_display_3bits_ic_node_306_0;
    wire ic_display_3bits_ic_node_307_0;
    wire ic_display_3bits_ic_node_308_0;
    wire ic_display_3bits_ic_node_309_0;
    wire ic_display_3bits_ic_node_310_0;
    wire ic_display_3bits_ic_node_311_0;
// ============== END IC: DISPLAY-3BITS ==============
    reg jk_flip_flop_312_0_q = 1'b0;
    reg jk_flip_flop_312_1_q = 1'b0;
    reg jk_flip_flop_313_0_q = 1'b0;
    reg jk_flip_flop_313_1_q = 1'b0;
    reg jk_flip_flop_314_0_q = 1'b0;
    reg jk_flip_flop_314_1_q = 1'b0;
    reg jk_flip_flop_315_0_q = 1'b0;
    reg jk_flip_flop_315_1_q = 1'b0;

    // ========= Logic Assignments =========
    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        jk_flip_flop_315_0_q = 1'b0;
        jk_flip_flop_315_1_q = 1'b1;
    end

    wire ic_display_3bits_ic_node_316_0;
    wire ic_display_3bits_ic_node_317_0;
    wire ic_display_3bits_ic_node_318_0;
    wire ic_display_3bits_ic_gnd_319_0;
    wire ic_display_3bits_ic_or_320_0;
    wire ic_display_3bits_ic_or_321_0;
    wire ic_display_3bits_ic_or_322_0;
    wire ic_display_3bits_ic_not_323_0;
    wire ic_display_3bits_ic_not_324_0;
    wire ic_display_3bits_ic_not_325_0;
    wire ic_display_3bits_ic_and_326_0;
    wire ic_display_3bits_ic_and_327_0;
    wire ic_display_3bits_ic_and_328_0;
    wire ic_display_3bits_ic_and_329_0;
    wire ic_display_3bits_ic_xnor_330_0;
    wire ic_display_3bits_ic_and_331_0;
    wire ic_display_3bits_ic_and_332_0;
    wire ic_display_3bits_ic_and_333_0;
    wire ic_display_3bits_ic_and_334_0;
    wire ic_display_3bits_ic_and_335_0;
    wire ic_display_3bits_ic_xnor_336_0;
    wire ic_display_3bits_ic_gnd_337_0;
    wire ic_display_3bits_ic_and_338_0;
    wire ic_display_3bits_ic_node_339_0;
    wire ic_display_3bits_ic_or_340_0;
    wire ic_display_3bits_ic_or_341_0;
    wire ic_display_3bits_ic_nand_342_0;
    wire ic_display_3bits_ic_or_343_0;
    wire ic_display_3bits_ic_or_344_0;
    wire ic_display_3bits_ic_or_345_0;
    wire ic_display_3bits_ic_or_346_0;
    wire ic_display_3bits_ic_node_347_0;
    wire ic_display_3bits_ic_node_348_0;
    wire ic_display_3bits_ic_node_349_0;
    wire ic_display_3bits_ic_node_350_0;
    wire ic_display_3bits_ic_node_351_0;
    wire ic_display_3bits_ic_node_352_0;
    wire ic_display_3bits_ic_node_353_0;
    wire ic_display_3bits_ic_node_354_0;
    wire ic_display_4bits_ic_node_355_0;
    wire ic_display_4bits_ic_node_356_0;
    wire ic_display_4bits_ic_node_357_0;
    wire ic_display_4bits_ic_node_358_0;
    wire ic_display_4bits_ic_node_359_0;
    wire ic_display_4bits_ic_node_360_0;
    wire ic_display_4bits_ic_not_361_0;
    wire ic_display_4bits_ic_node_362_0;
    wire ic_display_4bits_ic_node_363_0;
    wire ic_display_4bits_ic_node_364_0;
    wire ic_display_4bits_ic_not_365_0;
    wire ic_display_4bits_ic_not_366_0;
    wire ic_display_4bits_ic_node_367_0;
    wire ic_display_4bits_ic_node_368_0;
    wire ic_display_4bits_ic_node_369_0;
    wire ic_display_4bits_ic_node_370_0;
    wire ic_display_4bits_ic_node_371_0;
    wire ic_display_4bits_ic_node_372_0;
    wire ic_display_4bits_ic_node_373_0;
    wire ic_display_4bits_ic_node_374_0;
    wire ic_display_4bits_ic_node_375_0;
    wire ic_display_4bits_ic_node_376_0;
    wire ic_display_4bits_ic_node_377_0;
    wire ic_display_4bits_ic_node_378_0;
    wire ic_display_4bits_ic_node_379_0;
    wire ic_display_4bits_ic_and_380_0;
    wire ic_display_4bits_ic_node_381_0;
    wire ic_display_4bits_ic_node_382_0;
    wire ic_display_4bits_ic_node_383_0;
    wire ic_display_4bits_ic_node_384_0;
    wire ic_display_4bits_ic_node_385_0;
    wire ic_display_4bits_ic_node_386_0;
    wire ic_display_4bits_ic_and_387_0;
    wire ic_display_4bits_ic_and_388_0;
    wire ic_display_4bits_ic_node_389_0;
    wire ic_display_4bits_ic_node_390_0;
    wire ic_display_4bits_ic_node_391_0;
    wire ic_display_4bits_ic_node_392_0;
    wire ic_display_4bits_ic_node_393_0;
    wire ic_display_4bits_ic_node_394_0;
    wire ic_display_4bits_ic_node_395_0;
    wire ic_display_4bits_ic_node_396_0;
    wire ic_display_4bits_ic_node_397_0;
    wire ic_display_4bits_ic_and_398_0;
    wire ic_display_4bits_ic_node_399_0;
    wire ic_display_4bits_ic_and_400_0;
    wire ic_display_4bits_ic_node_401_0;
    wire ic_display_4bits_ic_node_402_0;
    wire ic_display_4bits_ic_node_403_0;
    wire ic_display_4bits_ic_and_404_0;
    wire ic_display_4bits_ic_and_405_0;
    wire ic_display_4bits_ic_and_406_0;
    wire ic_display_4bits_ic_and_407_0;
    wire ic_display_4bits_ic_or_408_0;
    wire ic_display_4bits_ic_or_409_0;
    wire ic_display_4bits_ic_or_410_0;
    wire ic_display_4bits_ic_or_411_0;
    wire ic_display_4bits_ic_or_412_0;
    wire ic_display_4bits_ic_or_413_0;
    wire ic_display_4bits_ic_or_414_0;
    wire ic_display_4bits_ic_not_415_0;
    wire ic_display_4bits_ic_node_416_0;
    wire ic_display_4bits_ic_node_417_0;
    wire ic_display_4bits_ic_node_418_0;
    wire ic_display_4bits_ic_node_419_0;
    wire ic_display_4bits_ic_node_420_0;
    wire ic_display_4bits_ic_node_421_0;
    wire ic_display_4bits_ic_node_422_0;
    wire ic_display_4bits_ic_node_423_0;
    wire ic_display_4bits_ic_node_424_0;
    wire ic_display_4bits_ic_node_425_0;
    wire ic_display_4bits_ic_node_426_0;
    wire ic_display_4bits_ic_node_427_0;
    wire ic_display_4bits_ic_node_428_0;
    wire ic_display_4bits_ic_node_429_0;
    wire ic_display_4bits_ic_node_430_0;
    wire ic_display_4bits_ic_node_431_0;
    wire ic_display_4bits_ic_node_432_0;
    wire ic_display_4bits_ic_node_433_0;
    wire ic_display_4bits_ic_node_434_0;
    wire ic_display_4bits_ic_node_435_0;
    wire ic_display_4bits_ic_node_436_0;
    wire ic_display_4bits_ic_node_437_0;
    wire ic_display_4bits_ic_not_438_0;
    wire ic_display_4bits_ic_node_439_0;
    wire ic_display_4bits_ic_node_440_0;
    wire ic_display_4bits_ic_node_441_0;
    wire ic_display_4bits_ic_not_442_0;
    wire ic_display_4bits_ic_not_443_0;
    wire ic_display_4bits_ic_node_444_0;
    wire ic_display_4bits_ic_node_445_0;
    wire ic_display_4bits_ic_node_446_0;
    wire ic_display_4bits_ic_node_447_0;
    wire ic_display_4bits_ic_node_448_0;
    wire ic_display_4bits_ic_node_449_0;
    wire ic_display_4bits_ic_node_450_0;
    wire ic_display_4bits_ic_node_451_0;
    wire ic_display_4bits_ic_node_452_0;
    wire ic_display_4bits_ic_node_453_0;
    wire ic_display_4bits_ic_node_454_0;
    wire ic_display_4bits_ic_node_455_0;
    wire ic_display_4bits_ic_node_456_0;
    wire ic_display_4bits_ic_and_457_0;
    wire ic_display_4bits_ic_node_458_0;
    wire ic_display_4bits_ic_node_459_0;
    wire ic_display_4bits_ic_node_460_0;
    wire ic_display_4bits_ic_node_461_0;
    wire ic_display_4bits_ic_node_462_0;
    wire ic_display_4bits_ic_node_463_0;
    wire ic_display_4bits_ic_and_464_0;
    wire ic_display_4bits_ic_and_465_0;
    wire ic_display_4bits_ic_node_466_0;
    wire ic_display_4bits_ic_node_467_0;
    wire ic_display_4bits_ic_node_468_0;
    wire ic_display_4bits_ic_node_469_0;
    wire ic_display_4bits_ic_node_470_0;
    wire ic_display_4bits_ic_node_471_0;
    wire ic_display_4bits_ic_node_472_0;
    wire ic_display_4bits_ic_node_473_0;
    wire ic_display_4bits_ic_node_474_0;
    wire ic_display_4bits_ic_and_475_0;
    wire ic_display_4bits_ic_node_476_0;
    wire ic_display_4bits_ic_and_477_0;
    wire ic_display_4bits_ic_node_478_0;
    wire ic_display_4bits_ic_node_479_0;
    wire ic_display_4bits_ic_node_480_0;
    wire ic_display_4bits_ic_and_481_0;
    wire ic_display_4bits_ic_and_482_0;
    wire ic_display_4bits_ic_and_483_0;
    wire ic_display_4bits_ic_and_484_0;
    wire ic_display_4bits_ic_or_485_0;
    wire ic_display_4bits_ic_or_486_0;
    wire ic_display_4bits_ic_or_487_0;
    wire ic_display_4bits_ic_or_488_0;
    wire ic_display_4bits_ic_or_489_0;
    wire ic_display_4bits_ic_or_490_0;
    wire ic_display_4bits_ic_or_491_0;
    wire ic_display_4bits_ic_not_492_0;
    wire ic_display_4bits_ic_node_493_0;
    wire ic_display_4bits_ic_node_494_0;
    wire ic_display_4bits_ic_node_495_0;
    wire ic_display_4bits_ic_node_496_0;
    wire ic_display_4bits_ic_node_497_0;
    wire ic_display_4bits_ic_node_498_0;
    wire ic_display_4bits_ic_node_499_0;
    wire ic_display_4bits_ic_node_500_0;
    wire ic_display_4bits_ic_node_501_0;
    wire ic_display_4bits_ic_node_502_0;
    wire ic_display_4bits_ic_node_503_0;
    wire ic_display_4bits_ic_node_504_0;
    wire ic_display_4bits_ic_node_505_0;
    wire ic_display_4bits_ic_node_506_0;
    wire ic_display_4bits_ic_node_507_0;
    wire ic_display_4bits_ic_node_508_0;
    wire ic_input_ic_node_509_0;
    wire ic_input_ic_node_510_0;
    wire ic_input_ic_not_511_0;
    wire ic_input_ic_xor_512_0;
    wire ic_input_ic_or_513_0;
    wire ic_input_ic_and_514_0;
    wire ic_input_ic_node_515_0;
    wire ic_input_ic_node_516_0;
    wire ic_input_ic_node_517_0;
    wire ic_input_ic_node_518_0;
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_313_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_314_0_q <= 1'b0; jk_flip_flop_314_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_314_0_q <= 1'b1; jk_flip_flop_314_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_314_0_q <= jk_flip_flop_314_1_q; jk_flip_flop_314_1_q <= jk_flip_flop_314_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_312_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_313_0_q <= 1'b0; jk_flip_flop_313_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_313_0_q <= 1'b1; jk_flip_flop_313_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_313_0_q <= jk_flip_flop_313_1_q; jk_flip_flop_313_1_q <= jk_flip_flop_313_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        jk_flip_flop_312_0_q = 1'b0;
        jk_flip_flop_312_1_q = 1'b1;
    end

    wire ic_jkflipflop_ic_node_519_0;
    wire ic_jkflipflop_ic_node_520_0;
    wire ic_jkflipflop_ic_node_521_0;
    wire ic_jkflipflop_ic_node_522_0;
    wire ic_jkflipflop_ic_node_523_0;
    wire ic_jkflipflop_ic_node_524_0;
    wire ic_dflipflop_ic_node_525_0;
    wire ic_dflipflop_ic_node_526_0;
    wire ic_dflipflop_ic_node_527_0;
    wire ic_dflipflop_ic_node_528_0;
    wire ic_dflipflop_ic_nand_529_0;
    wire ic_dflipflop_ic_node_530_0;
    wire ic_dflipflop_ic_nand_531_0;
    wire ic_dflipflop_ic_not_532_0;
    wire ic_dflipflop_ic_nand_533_0;
    wire ic_dflipflop_ic_nand_534_0;
    wire ic_dflipflop_ic_nand_535_0;
    wire ic_dflipflop_ic_nand_536_0;
    wire ic_dflipflop_ic_node_537_0;
    wire ic_dflipflop_ic_not_538_0;
    wire ic_dflipflop_ic_nand_539_0;
    wire ic_dflipflop_ic_nand_540_0;
    wire ic_dflipflop_ic_node_541_0;
    wire ic_dflipflop_ic_node_542_0;
    wire ic_dflipflop_ic_node_543_0;
    wire ic_dflipflop_ic_not_544_0;
    wire ic_dflipflop_ic_node_545_0;
    wire ic_dflipflop_ic_node_546_0;
    wire ic_dflipflop_ic_node_547_0;
    wire ic_dflipflop_ic_node_548_0;
    wire ic_dflipflop_ic_node_549_0;
    wire ic_jkflipflop_ic_node_550_0;
    wire ic_jkflipflop_ic_node_551_0;
    wire ic_jkflipflop_ic_node_552_0;
    wire ic_jkflipflop_ic_node_553_0;
    wire ic_jkflipflop_ic_node_554_0;
    wire ic_jkflipflop_ic_node_555_0;
    wire ic_jkflipflop_ic_not_556_0;
    wire ic_jkflipflop_ic_node_557_0;
    wire ic_jkflipflop_ic_and_558_0;
    wire ic_jkflipflop_ic_and_559_0;
    wire ic_jkflipflop_ic_or_560_0;
    wire ic_jkflipflop_ic_node_561_0;
    wire ic_jkflipflop_ic_node_562_0;

    // ========= Output Assignments =========
    assign output_7_segment_display1_g_middle_1 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_f_upper_left_2 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_e_lower_left_3 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_d_bottom_4 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_a_top_5 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_b_upper_right_6 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_dp_dot_7 = 1'b0; // 7-Segment Display
    assign output_7_segment_display1_c_lower_right_8 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_g_middle_9 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_f_upper_left_10 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_e_lower_left_11 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_d_bottom_12 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_a_top_13 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_b_upper_right_14 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_dp_dot_15 = 1'b0; // 7-Segment Display
    assign output_7_segment_display2_c_lower_right_16 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_g_middle_17 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_f_upper_left_18 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_e_lower_left_19 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_d_bottom_20 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_a_top_21 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_b_upper_right_22 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_dp_dot_23 = 1'b0; // 7-Segment Display
    assign output_7_segment_display3_c_lower_right_24 = 1'b0; // 7-Segment Display
    assign output_led4_0_25 = ic_input_ic_node_517_0; // LED
    assign output_led5_0_26 = ic_input_ic_node_516_0; // LED
    assign output_led6_0_27 = ic_input_ic_node_518_0; // LED
    assign output_led7_0_28 = jk_flip_flop_315_0_q; // LED

endmodule // ic

// ====================================================================
// Module ic generation completed successfully
// Elements processed: 22
// Inputs: 0, Outputs: 28
// Warnings: 5
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 4 is not connected
//   IC INPUT output 3 is not connected
// ====================================================================
