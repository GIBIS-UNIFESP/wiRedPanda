// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: sequential
// Generated: Fri Sep 26 21:22:29 2025
// Target FPGA: Generic-Small
// Resource Usage: 45/1000 LUTs, 74/1000 FFs, 8/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module sequential (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_load_shift_0_1,
    output wire output_led2_l1_0_2,
    output wire output_led3_l3_0_3,
    output wire output_led4_l2_0_4,
    output wire output_led5_l0_0_5
);

    // ========= Internal Signals =========
// ============== BEGIN IC: SERIALIZE ==============
// IC inputs: 6, IC outputs: 2
// Nesting depth: 0
    wire ic_serialize_ic_node_18_0;
    wire ic_serialize_ic_node_19_0;
    wire ic_serialize_ic_node_20_0;
    wire ic_serialize_ic_node_21_0;
    wire ic_serialize_ic_node_22_0;
    wire ic_serialize_ic_not_23_0;
    wire ic_serialize_ic_node_24_0;
    wire ic_serialize_ic_node_25_0;
    wire ic_serialize_ic_node_26_0;
    wire ic_serialize_ic_node_27_0;
    reg ic_serialize_ic_d_flip_flop_28_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_29_1_1 = 1'b0;
    wire ic_serialize_ic_and_30_0;
    wire ic_serialize_ic_and_31_0;
    wire ic_serialize_ic_node_32_0;
    wire ic_serialize_ic_or_33_0;
    wire ic_serialize_ic_node_34_0;
    wire ic_serialize_ic_node_35_0;
    reg ic_serialize_ic_d_flip_flop_36_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_37_1_1 = 1'b0;
    wire ic_serialize_ic_node_38_0;
    wire ic_serialize_ic_and_39_0;
    wire ic_serialize_ic_and_40_0;
    wire ic_serialize_ic_node_41_0;
    wire ic_serialize_ic_or_42_0;
    wire ic_serialize_ic_node_43_0;
    wire ic_serialize_ic_node_44_0;
    reg ic_serialize_ic_d_flip_flop_45_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_46_1_1 = 1'b0;
    wire ic_serialize_ic_node_47_0;
    wire ic_serialize_ic_and_48_0;
    wire ic_serialize_ic_and_49_0;
    wire ic_serialize_ic_node_50_0;
    wire ic_serialize_ic_or_51_0;
    wire ic_serialize_ic_node_52_0;
    wire ic_serialize_ic_node_53_0;
    reg ic_serialize_ic_d_flip_flop_54_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_55_1_1 = 1'b0;
    wire ic_serialize_ic_node_56_0;
    wire ic_serialize_ic_and_57_0;
    wire ic_serialize_ic_node_58_0;
    wire ic_serialize_ic_node_59_0;
// ============== END IC: SERIALIZE ==============
// ============== BEGIN IC: REGISTER ==============
// IC inputs: 2, IC outputs: 4
// Nesting depth: 0
    wire ic_register_ic_node_64_0;
    wire ic_register_ic_node_65_0;
    wire ic_register_ic_node_66_0;
    wire ic_register_ic_node_67_0;
    reg ic_register_ic_d_flip_flop_68_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_69_1_1 = 1'b0;
    wire ic_register_ic_node_70_0;
    reg ic_register_ic_d_flip_flop_71_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_72_1_1 = 1'b0;
    reg ic_register_ic_d_flip_flop_73_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_74_1_1 = 1'b0;
    wire ic_register_ic_node_75_0;
    reg ic_register_ic_d_flip_flop_76_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_77_1_1 = 1'b0;
    wire ic_register_ic_node_78_0;
    wire ic_register_ic_node_79_0;
    wire ic_register_ic_node_80_0;
    wire ic_register_ic_node_81_0;
// ============== END IC: REGISTER ==============
    wire not_82;
    wire node_83;
    wire node_84;
    reg jk_flip_flop_85_0_q = 1'b0;
    reg jk_flip_flop_85_1_q = 1'b0;
    wire node_86;
    reg jk_flip_flop_87_0_q = 1'b0;
    reg jk_flip_flop_87_1_q = 1'b0;
    wire node_88;
    reg jk_flip_flop_89_0_q = 1'b0;
    reg jk_flip_flop_89_1_q = 1'b0;
    wire node_90;
    reg jk_flip_flop_91_0_q = 1'b0;
    reg jk_flip_flop_91_1_q = 1'b0;
    wire node_92;
    wire node_93;
    wire node_94;
    wire node_95;
    wire node_96;
    wire node_97;
    wire node_98;
    wire node_99;
    wire not_100;
    wire node_101;
    wire node_102;
    wire and_103;
    wire node_104;
    wire node_105;
    wire and_106;
    wire and_107;
    wire and_108;
    wire and_109;

    // ========= Logic Assignments =========
    assign and_109 = (1'b0 & node_105); // And
    assign and_108 = (1'b0 & node_104); // And
    assign and_107 = (1'b0 & node_102); // And
    assign and_106 = (1'b0 & node_99); // And
    assign node_105 = node_104; // Node
    wire ic_register_ic_node_110_0;
    wire ic_register_ic_node_111_0;
    wire ic_register_ic_node_112_0;
    wire ic_register_ic_node_113_0;
    reg ic_register_ic_d_flip_flop_114_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_115_1_1 = 1'b0;
    wire ic_register_ic_node_116_0;
    reg ic_register_ic_d_flip_flop_117_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_118_1_1 = 1'b0;
    reg ic_register_ic_d_flip_flop_119_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_120_1_1 = 1'b0;
    wire ic_register_ic_node_121_0;
    reg ic_register_ic_d_flip_flop_122_0_0 = 1'b0;
    reg ic_register_ic_d_flip_flop_123_1_1 = 1'b0;
    wire ic_register_ic_node_124_0;
    wire ic_register_ic_node_125_0;
    wire ic_register_ic_node_126_0;
    wire ic_register_ic_node_127_0;
    assign node_104 = node_102; // Node
    assign and_103 = (not_100 & node_101); // And
    assign node_102 = node_99; // Node
    assign node_101 = node_98; // Node
    assign not_100 = ~1'b0; // Not
    assign node_99 = node_97; // Node
    wire ic_serialize_ic_node_128_0;
    wire ic_serialize_ic_node_129_0;
    wire ic_serialize_ic_node_130_0;
    wire ic_serialize_ic_node_131_0;
    wire ic_serialize_ic_node_132_0;
    wire ic_serialize_ic_not_133_0;
    wire ic_serialize_ic_node_134_0;
    wire ic_serialize_ic_node_135_0;
    wire ic_serialize_ic_node_136_0;
    wire ic_serialize_ic_node_137_0;
    reg ic_serialize_ic_d_flip_flop_138_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_139_1_1 = 1'b0;
    wire ic_serialize_ic_and_140_0;
    wire ic_serialize_ic_and_141_0;
    wire ic_serialize_ic_node_142_0;
    wire ic_serialize_ic_or_143_0;
    wire ic_serialize_ic_node_144_0;
    wire ic_serialize_ic_node_145_0;
    reg ic_serialize_ic_d_flip_flop_146_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_147_1_1 = 1'b0;
    wire ic_serialize_ic_node_148_0;
    wire ic_serialize_ic_and_149_0;
    wire ic_serialize_ic_and_150_0;
    wire ic_serialize_ic_node_151_0;
    wire ic_serialize_ic_or_152_0;
    wire ic_serialize_ic_node_153_0;
    wire ic_serialize_ic_node_154_0;
    reg ic_serialize_ic_d_flip_flop_155_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_156_1_1 = 1'b0;
    wire ic_serialize_ic_node_157_0;
    wire ic_serialize_ic_and_158_0;
    wire ic_serialize_ic_and_159_0;
    wire ic_serialize_ic_node_160_0;
    wire ic_serialize_ic_or_161_0;
    wire ic_serialize_ic_node_162_0;
    wire ic_serialize_ic_node_163_0;
    reg ic_serialize_ic_d_flip_flop_164_0_0 = 1'b0;
    reg ic_serialize_ic_d_flip_flop_165_1_1 = 1'b0;
    wire ic_serialize_ic_node_166_0;
    wire ic_serialize_ic_and_167_0;
    wire ic_serialize_ic_node_168_0;
    wire ic_serialize_ic_node_169_0;
    assign node_97 = node_92; // Node
    assign node_96 = jk_flip_flop_91_1_q; // Node
    assign node_95 = jk_flip_flop_89_1_q; // Node
    assign node_94 = jk_flip_flop_87_1_q; // Node
    assign node_93 = jk_flip_flop_85_1_q; // Node
    assign node_92 = 1'b0; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_89_0_q or negedge node_90) begin
        if (!node_90) begin
            jk_flip_flop_91_0_q <= 1'b1;
            jk_flip_flop_91_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_91_0_q <= 1'b0; jk_flip_flop_91_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_91_0_q <= 1'b1; jk_flip_flop_91_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_91_0_q <= jk_flip_flop_91_1_q; jk_flip_flop_91_1_q <= jk_flip_flop_91_0_q; end // toggle
            endcase
        end
    end

    assign node_90 = node_88; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_87_0_q or negedge node_88) begin
        if (!node_88) begin
            jk_flip_flop_89_0_q <= 1'b1;
            jk_flip_flop_89_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_89_0_q <= 1'b0; jk_flip_flop_89_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_89_0_q <= 1'b1; jk_flip_flop_89_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_89_0_q <= jk_flip_flop_89_1_q; jk_flip_flop_89_1_q <= jk_flip_flop_89_0_q; end // toggle
            endcase
        end
    end

    assign node_88 = node_86; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_85_0_q or negedge node_86) begin
        if (!node_86) begin
            jk_flip_flop_87_0_q <= 1'b1;
            jk_flip_flop_87_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_87_0_q <= 1'b0; jk_flip_flop_87_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_87_0_q <= 1'b1; jk_flip_flop_87_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_87_0_q <= jk_flip_flop_87_1_q; jk_flip_flop_87_1_q <= jk_flip_flop_87_0_q; end // toggle
            endcase
        end
    end

    assign node_86 = node_84; // Node
    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        jk_flip_flop_85_0_q = 1'b0;
        jk_flip_flop_85_1_q = 1'b1;
    end

    assign node_84 = node_83; // Node
    assign not_82 = ~1'b0; // Not

    // ========= Output Assignments =========
    assign output_led1_load_shift_0_1 = node_97; // LED
    assign output_led2_l1_0_2 = and_108; // LED
    assign output_led3_l3_0_3 = and_106; // LED
    assign output_led4_l2_0_4 = and_107; // LED
    assign output_led5_l0_0_5 = and_109; // LED

endmodule // sequential

// ====================================================================
// Module sequential generation completed successfully
// Elements processed: 38
// Inputs: 0, Outputs: 5
// ====================================================================
