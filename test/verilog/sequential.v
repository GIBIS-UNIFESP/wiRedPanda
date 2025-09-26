// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: sequential
// Generated: Fri Sep 26 18:49:30 2025
// Target FPGA: Generic-Small
// Resource Usage: 45/1000 LUTs, 74/1000 FFs, 8/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module sequential (
    // ========= Input Ports =========
    input wire input_push_button1_reset_1,
    input wire input_clock2_slow_clk_2,
    input wire input_clock3_fast_clk_3,

    // ========= Output Ports =========
    output wire output_led1_load_shift_0_4,
    output wire output_led2_l1_0_5,
    output wire output_led3_l3_0_6,
    output wire output_led4_l2_0_7,
    output wire output_led5_l0_0_8
);

    // ========= Internal Signals =========
// ============== BEGIN IC: SERIALIZE ==============
// IC inputs: 6, IC outputs: 2
// Nesting depth: 0
    wire ic_serialize_ic_node_21_0;
    wire ic_serialize_ic_node_22_0;
    wire ic_serialize_ic_node_23_0;
    wire ic_serialize_ic_node_24_0;
    wire ic_serialize_ic_node_25_0;
    wire ic_serialize_ic_not_26_0;
    wire ic_serialize_ic_node_27_0;
    wire ic_serialize_ic_node_28_0;
    wire ic_serialize_ic_node_29_0;
    wire ic_serialize_ic_node_30_0;
    reg ic_serialize_ic_d_flip_flop_31_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_32_1_1 = 1'b0;
    wire ic_serialize_ic_and_33_0;
    wire ic_serialize_ic_and_34_0;
    wire ic_serialize_ic_node_35_0;
    wire ic_serialize_ic_or_36_0;
    wire ic_serialize_ic_node_37_0;
    wire ic_serialize_ic_node_38_0;
    reg ic_serialize_ic_d_flip_flop_39_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_40_1_1 = 1'b0;
    wire ic_serialize_ic_node_41_0;
    wire ic_serialize_ic_and_42_0;
    wire ic_serialize_ic_and_43_0;
    wire ic_serialize_ic_node_44_0;
    wire ic_serialize_ic_or_45_0;
    wire ic_serialize_ic_node_46_0;
    wire ic_serialize_ic_node_47_0;
    reg ic_serialize_ic_d_flip_flop_48_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_49_1_1 = 1'b0;
    wire ic_serialize_ic_node_50_0;
    wire ic_serialize_ic_and_51_0;
    wire ic_serialize_ic_and_52_0;
    wire ic_serialize_ic_node_53_0;
    wire ic_serialize_ic_or_54_0;
    wire ic_serialize_ic_node_55_0;
    wire ic_serialize_ic_node_56_0;
    reg ic_serialize_ic_d_flip_flop_57_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_58_1_1 = 1'b0;
    wire ic_serialize_ic_node_59_0;
    wire ic_serialize_ic_and_60_0;
    wire ic_serialize_ic_node_61_0;
    wire ic_serialize_ic_node_62_0;
// ============== END IC: SERIALIZE ==============
// ============== BEGIN IC: REGISTER ==============
// IC inputs: 2, IC outputs: 4
// Nesting depth: 0
    wire ic_register_ic_node_67_0;
    wire ic_register_ic_node_68_0;
    wire ic_register_ic_node_69_0;
    wire ic_register_ic_node_70_0;
    reg ic_register_ic_d_flip_flop_71_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_72_1_1 = 1'b0;
    wire ic_register_ic_node_73_0;
    reg ic_register_ic_d_flip_flop_74_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_75_1_1 = 1'b0;
    reg ic_register_ic_d_flip_flop_76_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_77_1_1 = 1'b0;
    wire ic_register_ic_node_78_0;
    reg ic_register_ic_d_flip_flop_79_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_80_1_1 = 1'b0;
    wire ic_register_ic_node_81_0;
    wire ic_register_ic_node_82_0;
    wire ic_register_ic_node_83_0;
    wire ic_register_ic_node_84_0;
// ============== END IC: REGISTER ==============
    wire not_85;
    wire node_86;
    wire node_87;
    reg jk_flip_flop_88_0_q = 1'b0;
    reg jk_flip_flop_88_1_q = 1'b0;
    wire node_89;
    reg jk_flip_flop_90_0_q = 1'b0;
    reg jk_flip_flop_90_1_q = 1'b0;
    wire node_91;
    reg jk_flip_flop_92_0_q = 1'b0;
    reg jk_flip_flop_92_1_q = 1'b0;
    wire node_93;
    reg jk_flip_flop_94_0_q = 1'b0;
    reg jk_flip_flop_94_1_q = 1'b0;
    wire node_95;
    wire node_96;
    wire node_97;
    wire node_98;
    wire node_99;
    wire node_100;
    wire node_101;
    wire node_102;
    wire not_103;
    wire node_104;
    wire node_105;
    wire and_106;
    wire node_107;
    wire node_108;
    wire and_109;
    wire and_110;
    wire and_111;
    wire and_112;

    // ========= Logic Assignments =========
    assign and_112 = (1'b0 & node_108); // And
    assign and_111 = (1'b0 & node_107); // And
    assign and_110 = (1'b0 & node_105); // And
    assign and_109 = (1'b0 & node_102); // And
    assign node_108 = node_107; // Node
    wire ic_register_ic_node_113_0;
    wire ic_register_ic_node_114_0;
    wire ic_register_ic_node_115_0;
    wire ic_register_ic_node_116_0;
    reg ic_register_ic_d_flip_flop_117_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_118_1_1 = 1'b0;
    wire ic_register_ic_node_119_0;
    reg ic_register_ic_d_flip_flop_120_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_121_1_1 = 1'b0;
    reg ic_register_ic_d_flip_flop_122_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_123_1_1 = 1'b0;
    wire ic_register_ic_node_124_0;
    reg ic_register_ic_d_flip_flop_125_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_126_1_1 = 1'b0;
    wire ic_register_ic_node_127_0;
    wire ic_register_ic_node_128_0;
    wire ic_register_ic_node_129_0;
    wire ic_register_ic_node_130_0;
    assign node_107 = node_105; // Node
    assign and_106 = (not_103 & node_104); // And
    assign node_105 = node_102; // Node
    assign node_104 = node_101; // Node
    assign not_103 = ~1'b0; // Not
    assign node_102 = node_100; // Node
    wire ic_serialize_ic_node_131_0;
    wire ic_serialize_ic_node_132_0;
    wire ic_serialize_ic_node_133_0;
    wire ic_serialize_ic_node_134_0;
    wire ic_serialize_ic_node_135_0;
    wire ic_serialize_ic_not_136_0;
    wire ic_serialize_ic_node_137_0;
    wire ic_serialize_ic_node_138_0;
    wire ic_serialize_ic_node_139_0;
    wire ic_serialize_ic_node_140_0;
    reg ic_serialize_ic_d_flip_flop_141_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_142_1_1 = 1'b0;
    wire ic_serialize_ic_and_143_0;
    wire ic_serialize_ic_and_144_0;
    wire ic_serialize_ic_node_145_0;
    wire ic_serialize_ic_or_146_0;
    wire ic_serialize_ic_node_147_0;
    wire ic_serialize_ic_node_148_0;
    reg ic_serialize_ic_d_flip_flop_149_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_150_1_1 = 1'b0;
    wire ic_serialize_ic_node_151_0;
    wire ic_serialize_ic_and_152_0;
    wire ic_serialize_ic_and_153_0;
    wire ic_serialize_ic_node_154_0;
    wire ic_serialize_ic_or_155_0;
    wire ic_serialize_ic_node_156_0;
    wire ic_serialize_ic_node_157_0;
    reg ic_serialize_ic_d_flip_flop_158_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_159_1_1 = 1'b0;
    wire ic_serialize_ic_node_160_0;
    wire ic_serialize_ic_and_161_0;
    wire ic_serialize_ic_and_162_0;
    wire ic_serialize_ic_node_163_0;
    wire ic_serialize_ic_or_164_0;
    wire ic_serialize_ic_node_165_0;
    wire ic_serialize_ic_node_166_0;
    reg ic_serialize_ic_d_flip_flop_167_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_168_1_1 = 1'b0;
    wire ic_serialize_ic_node_169_0;
    wire ic_serialize_ic_and_170_0;
    wire ic_serialize_ic_node_171_0;
    wire ic_serialize_ic_node_172_0;
    assign node_100 = node_95; // Node
    assign node_99 = jk_flip_flop_94_1_q; // Node
    assign node_98 = jk_flip_flop_92_1_q; // Node
    assign node_97 = jk_flip_flop_90_1_q; // Node
    assign node_96 = jk_flip_flop_88_1_q; // Node
    assign node_95 = input_clock2_slow_clk_2; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_92_0_q or negedge node_93) begin
        if (!node_93) begin
            jk_flip_flop_94_0_q <= 1'b1;
            jk_flip_flop_94_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_94_0_q <= 1'b0; jk_flip_flop_94_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_94_0_q <= 1'b1; jk_flip_flop_94_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_94_0_q <= jk_flip_flop_94_1_q; jk_flip_flop_94_1_q <= jk_flip_flop_94_0_q; end // toggle
            endcase
        end
    end

    assign node_93 = node_91; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_90_0_q or negedge node_91) begin
        if (!node_91) begin
            jk_flip_flop_92_0_q <= 1'b1;
            jk_flip_flop_92_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_92_0_q <= 1'b0; jk_flip_flop_92_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_92_0_q <= 1'b1; jk_flip_flop_92_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_92_0_q <= jk_flip_flop_92_1_q; jk_flip_flop_92_1_q <= jk_flip_flop_92_0_q; end // toggle
            endcase
        end
    end

    assign node_91 = node_89; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_88_0_q or negedge node_89) begin
        if (!node_89) begin
            jk_flip_flop_90_0_q <= 1'b1;
            jk_flip_flop_90_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_90_0_q <= 1'b0; jk_flip_flop_90_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_90_0_q <= 1'b1; jk_flip_flop_90_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_90_0_q <= jk_flip_flop_90_1_q; jk_flip_flop_90_1_q <= jk_flip_flop_90_0_q; end // toggle
            endcase
        end
    end

    assign node_89 = node_87; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock2_slow_clk_2 or negedge node_87) begin
        if (!node_87) begin
            jk_flip_flop_88_0_q <= 1'b1;
            jk_flip_flop_88_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_88_0_q <= 1'b0; jk_flip_flop_88_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_88_0_q <= 1'b1; jk_flip_flop_88_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_88_0_q <= jk_flip_flop_88_1_q; jk_flip_flop_88_1_q <= jk_flip_flop_88_0_q; end // toggle
            endcase
        end
    end

    assign node_87 = node_86; // Node
    assign not_85 = ~input_push_button1_reset_1; // Not

    // ========= Output Assignments =========
    assign output_led1_load_shift_0_4 = node_100; // LED
    assign output_led2_l1_0_5 = and_111; // LED
    assign output_led3_l3_0_6 = and_109; // LED
    assign output_led4_l2_0_7 = and_110; // LED
    assign output_led5_l0_0_8 = and_112; // LED

endmodule // sequential

// ====================================================================
// Module sequential generation completed successfully
// Elements processed: 38
// Inputs: 3, Outputs: 5
// ====================================================================
