// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: display_4bits_counter
// Generated: Fri Sep 26 18:49:24 2025
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
    assign node_325 = node_320; // Node
    wire ic_display_4bits_ic_node_330_0;
    wire ic_display_4bits_ic_node_331_0;
    wire ic_display_4bits_ic_node_332_0;
    wire ic_display_4bits_ic_node_333_0;
    wire ic_display_4bits_ic_node_334_0;
    wire ic_display_4bits_ic_node_335_0;
    wire ic_display_4bits_ic_not_336_0;
    wire ic_display_4bits_ic_node_337_0;
    wire ic_display_4bits_ic_node_338_0;
    wire ic_display_4bits_ic_node_339_0;
    wire ic_display_4bits_ic_not_340_0;
    wire ic_display_4bits_ic_not_341_0;
    wire ic_display_4bits_ic_node_342_0;
    wire ic_display_4bits_ic_node_343_0;
    wire ic_display_4bits_ic_node_344_0;
    wire ic_display_4bits_ic_node_345_0;
    wire ic_display_4bits_ic_node_346_0;
    wire ic_display_4bits_ic_node_347_0;
    wire ic_display_4bits_ic_node_348_0;
    wire ic_display_4bits_ic_node_349_0;
    wire ic_display_4bits_ic_node_350_0;
    wire ic_display_4bits_ic_node_351_0;
    wire ic_display_4bits_ic_node_352_0;
    wire ic_display_4bits_ic_node_353_0;
    wire ic_display_4bits_ic_node_354_0;
    wire ic_display_4bits_ic_and_355_0;
    wire ic_display_4bits_ic_node_356_0;
    wire ic_display_4bits_ic_node_357_0;
    wire ic_display_4bits_ic_node_358_0;
    wire ic_display_4bits_ic_node_359_0;
    wire ic_display_4bits_ic_node_360_0;
    wire ic_display_4bits_ic_node_361_0;
    wire ic_display_4bits_ic_and_362_0;
    wire ic_display_4bits_ic_and_363_0;
    wire ic_display_4bits_ic_node_364_0;
    wire ic_display_4bits_ic_node_365_0;
    wire ic_display_4bits_ic_node_366_0;
    wire ic_display_4bits_ic_node_367_0;
    wire ic_display_4bits_ic_node_368_0;
    wire ic_display_4bits_ic_node_369_0;
    wire ic_display_4bits_ic_node_370_0;
    wire ic_display_4bits_ic_node_371_0;
    wire ic_display_4bits_ic_node_372_0;
    wire ic_display_4bits_ic_and_373_0;
    wire ic_display_4bits_ic_node_374_0;
    wire ic_display_4bits_ic_and_375_0;
    wire ic_display_4bits_ic_node_376_0;
    wire ic_display_4bits_ic_node_377_0;
    wire ic_display_4bits_ic_node_378_0;
    wire ic_display_4bits_ic_and_379_0;
    wire ic_display_4bits_ic_and_380_0;
    wire ic_display_4bits_ic_and_381_0;
    wire ic_display_4bits_ic_and_382_0;
    wire ic_display_4bits_ic_or_383_0;
    wire ic_display_4bits_ic_or_384_0;
    wire ic_display_4bits_ic_or_385_0;
    wire ic_display_4bits_ic_or_386_0;
    wire ic_display_4bits_ic_or_387_0;
    wire ic_display_4bits_ic_or_388_0;
    wire ic_display_4bits_ic_or_389_0;
    wire ic_display_4bits_ic_not_390_0;
    wire ic_display_4bits_ic_node_391_0;
    wire ic_display_4bits_ic_node_392_0;
    wire ic_display_4bits_ic_node_393_0;
    wire ic_display_4bits_ic_node_394_0;
    wire ic_display_4bits_ic_node_395_0;
    wire ic_display_4bits_ic_node_396_0;
    wire ic_display_4bits_ic_node_397_0;
    wire ic_display_4bits_ic_node_398_0;
    wire ic_display_4bits_ic_node_399_0;
    wire ic_display_4bits_ic_node_400_0;
    wire ic_display_4bits_ic_node_401_0;
    wire ic_display_4bits_ic_node_402_0;
    wire ic_display_4bits_ic_node_403_0;
    wire ic_display_4bits_ic_node_404_0;
    wire ic_display_4bits_ic_node_405_0;
    wire ic_display_4bits_ic_node_406_0;
    assign node_324 = 1'b0; // Node
    assign node_323 = 1'b0; // Node
    assign node_322 = 1'b0; // Node
    assign node_321 = 1'b0; // Node
    wire ic_jkflipflop_ic_node_407_0;
    wire ic_jkflipflop_ic_node_408_0;
    wire ic_jkflipflop_ic_node_409_0;
    wire ic_jkflipflop_ic_node_410_0;
    wire ic_jkflipflop_ic_node_411_0;
    wire ic_jkflipflop_ic_node_412_0;
    wire ic_dflipflop_ic_node_413_0;
    wire ic_dflipflop_ic_node_414_0;
    wire ic_dflipflop_ic_node_415_0;
    wire ic_dflipflop_ic_node_416_0;
    wire ic_dflipflop_ic_nand_417_0;
    wire ic_dflipflop_ic_node_418_0;
    wire ic_dflipflop_ic_nand_419_0;
    wire ic_dflipflop_ic_not_420_0;
    wire ic_dflipflop_ic_nand_421_0;
    wire ic_dflipflop_ic_nand_422_0;
    wire ic_dflipflop_ic_nand_423_0;
    wire ic_dflipflop_ic_nand_424_0;
    wire ic_dflipflop_ic_node_425_0;
    wire ic_dflipflop_ic_not_426_0;
    wire ic_dflipflop_ic_nand_427_0;
    wire ic_dflipflop_ic_nand_428_0;
    wire ic_dflipflop_ic_node_429_0;
    wire ic_dflipflop_ic_node_430_0;
    wire ic_dflipflop_ic_node_431_0;
    wire ic_dflipflop_ic_not_432_0;
    wire ic_dflipflop_ic_node_433_0;
    wire ic_dflipflop_ic_node_434_0;
    wire ic_dflipflop_ic_node_435_0;
    wire ic_dflipflop_ic_node_436_0;
    wire ic_dflipflop_ic_node_437_0;
    wire ic_jkflipflop_ic_node_438_0;
    wire ic_jkflipflop_ic_node_439_0;
    wire ic_jkflipflop_ic_node_440_0;
    wire ic_jkflipflop_ic_node_441_0;
    wire ic_jkflipflop_ic_node_442_0;
    wire ic_jkflipflop_ic_node_443_0;
    wire ic_jkflipflop_ic_not_444_0;
    wire ic_jkflipflop_ic_node_445_0;
    wire ic_jkflipflop_ic_and_446_0;
    wire ic_jkflipflop_ic_and_447_0;
    wire ic_jkflipflop_ic_or_448_0;
    wire ic_jkflipflop_ic_node_449_0;
    wire ic_jkflipflop_ic_node_450_0;
    assign node_327 = node_326; // Node
    assign node_326 = node_319; // Node
    assign node_319 = node_318; // Node
    assign node_318 = node_317; // Node
    assign not_316 = ~and_315; // Not
    assign and_315 = (node_328 & node_329 & node_313 & node_314); // And
    assign node_314 = 1'b0; // Node
    assign node_313 = 1'b0; // Node
    assign node_329 = 1'b0; // Node
    assign node_328 = 1'b0; // Node
    wire ic_jkflipflop_ic_node_451_0;
    wire ic_jkflipflop_ic_node_452_0;
    wire ic_jkflipflop_ic_node_453_0;
    wire ic_jkflipflop_ic_node_454_0;
    wire ic_jkflipflop_ic_node_455_0;
    wire ic_jkflipflop_ic_node_456_0;
    wire ic_dflipflop_ic_node_457_0;
    wire ic_dflipflop_ic_node_458_0;
    wire ic_dflipflop_ic_node_459_0;
    wire ic_dflipflop_ic_node_460_0;
    wire ic_dflipflop_ic_nand_461_0;
    wire ic_dflipflop_ic_node_462_0;
    wire ic_dflipflop_ic_nand_463_0;
    wire ic_dflipflop_ic_not_464_0;
    wire ic_dflipflop_ic_nand_465_0;
    wire ic_dflipflop_ic_nand_466_0;
    wire ic_dflipflop_ic_nand_467_0;
    wire ic_dflipflop_ic_nand_468_0;
    wire ic_dflipflop_ic_node_469_0;
    wire ic_dflipflop_ic_not_470_0;
    wire ic_dflipflop_ic_nand_471_0;
    wire ic_dflipflop_ic_nand_472_0;
    wire ic_dflipflop_ic_node_473_0;
    wire ic_dflipflop_ic_node_474_0;
    wire ic_dflipflop_ic_node_475_0;
    wire ic_dflipflop_ic_not_476_0;
    wire ic_dflipflop_ic_node_477_0;
    wire ic_dflipflop_ic_node_478_0;
    wire ic_dflipflop_ic_node_479_0;
    wire ic_dflipflop_ic_node_480_0;
    wire ic_dflipflop_ic_node_481_0;
    wire ic_jkflipflop_ic_node_482_0;
    wire ic_jkflipflop_ic_node_483_0;
    wire ic_jkflipflop_ic_node_484_0;
    wire ic_jkflipflop_ic_node_485_0;
    wire ic_jkflipflop_ic_node_486_0;
    wire ic_jkflipflop_ic_node_487_0;
    wire ic_jkflipflop_ic_not_488_0;
    wire ic_jkflipflop_ic_node_489_0;
    wire ic_jkflipflop_ic_and_490_0;
    wire ic_jkflipflop_ic_and_491_0;
    wire ic_jkflipflop_ic_or_492_0;
    wire ic_jkflipflop_ic_node_493_0;
    wire ic_jkflipflop_ic_node_494_0;
    wire ic_jkflipflop_ic_node_495_0;
    wire ic_jkflipflop_ic_node_496_0;
    wire ic_jkflipflop_ic_node_497_0;
    wire ic_jkflipflop_ic_node_498_0;
    wire ic_jkflipflop_ic_node_499_0;
    wire ic_jkflipflop_ic_node_500_0;
    wire ic_dflipflop_ic_node_501_0;
    wire ic_dflipflop_ic_node_502_0;
    wire ic_dflipflop_ic_node_503_0;
    wire ic_dflipflop_ic_node_504_0;
    wire ic_dflipflop_ic_nand_505_0;
    wire ic_dflipflop_ic_node_506_0;
    wire ic_dflipflop_ic_nand_507_0;
    wire ic_dflipflop_ic_not_508_0;
    wire ic_dflipflop_ic_nand_509_0;
    wire ic_dflipflop_ic_nand_510_0;
    wire ic_dflipflop_ic_nand_511_0;
    wire ic_dflipflop_ic_nand_512_0;
    wire ic_dflipflop_ic_node_513_0;
    wire ic_dflipflop_ic_not_514_0;
    wire ic_dflipflop_ic_nand_515_0;
    wire ic_dflipflop_ic_nand_516_0;
    wire ic_dflipflop_ic_node_517_0;
    wire ic_dflipflop_ic_node_518_0;
    wire ic_dflipflop_ic_node_519_0;
    wire ic_dflipflop_ic_not_520_0;
    wire ic_dflipflop_ic_node_521_0;
    wire ic_dflipflop_ic_node_522_0;
    wire ic_dflipflop_ic_node_523_0;
    wire ic_dflipflop_ic_node_524_0;
    wire ic_dflipflop_ic_node_525_0;
    wire ic_jkflipflop_ic_node_526_0;
    wire ic_jkflipflop_ic_node_527_0;
    wire ic_jkflipflop_ic_node_528_0;
    wire ic_jkflipflop_ic_node_529_0;
    wire ic_jkflipflop_ic_node_530_0;
    wire ic_jkflipflop_ic_node_531_0;
    wire ic_jkflipflop_ic_not_532_0;
    wire ic_jkflipflop_ic_node_533_0;
    wire ic_jkflipflop_ic_and_534_0;
    wire ic_jkflipflop_ic_and_535_0;
    wire ic_jkflipflop_ic_or_536_0;
    wire ic_jkflipflop_ic_node_537_0;
    wire ic_jkflipflop_ic_node_538_0;
    wire ic_jkflipflop_ic_node_539_0;
    wire ic_jkflipflop_ic_node_540_0;
    wire ic_jkflipflop_ic_node_541_0;
    wire ic_jkflipflop_ic_node_542_0;
    wire ic_jkflipflop_ic_node_543_0;
    wire ic_jkflipflop_ic_node_544_0;
    wire ic_dflipflop_ic_node_545_0;
    wire ic_dflipflop_ic_node_546_0;
    wire ic_dflipflop_ic_node_547_0;
    wire ic_dflipflop_ic_node_548_0;
    wire ic_dflipflop_ic_nand_549_0;
    wire ic_dflipflop_ic_node_550_0;
    wire ic_dflipflop_ic_nand_551_0;
    wire ic_dflipflop_ic_not_552_0;
    wire ic_dflipflop_ic_nand_553_0;
    wire ic_dflipflop_ic_nand_554_0;
    wire ic_dflipflop_ic_nand_555_0;
    wire ic_dflipflop_ic_nand_556_0;
    wire ic_dflipflop_ic_node_557_0;
    wire ic_dflipflop_ic_not_558_0;
    wire ic_dflipflop_ic_nand_559_0;
    wire ic_dflipflop_ic_nand_560_0;
    wire ic_dflipflop_ic_node_561_0;
    wire ic_dflipflop_ic_node_562_0;
    wire ic_dflipflop_ic_node_563_0;
    wire ic_dflipflop_ic_not_564_0;
    wire ic_dflipflop_ic_node_565_0;
    wire ic_dflipflop_ic_node_566_0;
    wire ic_dflipflop_ic_node_567_0;
    wire ic_dflipflop_ic_node_568_0;
    wire ic_dflipflop_ic_node_569_0;
    wire ic_jkflipflop_ic_node_570_0;
    wire ic_jkflipflop_ic_node_571_0;
    wire ic_jkflipflop_ic_node_572_0;
    wire ic_jkflipflop_ic_node_573_0;
    wire ic_jkflipflop_ic_node_574_0;
    wire ic_jkflipflop_ic_node_575_0;
    wire ic_jkflipflop_ic_not_576_0;
    wire ic_jkflipflop_ic_node_577_0;
    wire ic_jkflipflop_ic_and_578_0;
    wire ic_jkflipflop_ic_and_579_0;
    wire ic_jkflipflop_ic_or_580_0;
    wire ic_jkflipflop_ic_node_581_0;
    wire ic_jkflipflop_ic_node_582_0;
    assign not_312 = ~input_push_button1_1; // Not

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
