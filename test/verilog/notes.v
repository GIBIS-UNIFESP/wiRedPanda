// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: notes
// Generated: Fri Sep 26 14:59:14 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 38/1000 FFs, 9/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module notes (
    // ========= Input Ports =========
    input wire input_clock1_1,

    // ========= Output Ports =========
    output wire output_buzzer1_g6_2,
    output wire output_buzzer2_f6_3,
    output wire output_buzzer3_d6_4,
    output wire output_buzzer4_b7_5,
    output wire output_buzzer5_c6_6,
    output wire output_buzzer6_a7_7,
    output wire output_buzzer7_e6_8,
    output wire output_buzzer8_c7_9
);

    // ========= Internal Signals =========
// ============== BEGIN IC: JKFLIPFLOP ==============
// IC inputs: 5, IC outputs: 2
// Nesting depth: 0
    wire ic_jkflipflop_ic_jkflipflop_node_10;
    wire ic_jkflipflop_ic_jkflipflop_node_11;
    wire ic_jkflipflop_ic_jkflipflop_node_13;
    wire ic_jkflipflop_ic_jkflipflop_node_14;
    wire ic_jkflipflop_ic_jkflipflop_node_15;
    wire ic_jkflipflop_ic_jkflipflop_node_16;
    wire ic_jkflipflop_ic_jkflipflop_not_18;
    wire ic_jkflipflop_ic_jkflipflop_and_19;
    wire ic_jkflipflop_ic_jkflipflop_node_20;
    wire ic_jkflipflop_ic_jkflipflop_or_21;
    wire ic_jkflipflop_ic_jkflipflop_node_22;
    wire ic_jkflipflop_ic_jkflipflop_node_23;
    wire ic_jkflipflop_ic_jkflipflop_node_24;
    wire ic_jkflipflop_ic_jkflipflop_node_25;
    wire ic_jkflipflop_ic_jkflipflop_and_26;
    wire ic_jkflipflop_ic_jkflipflop_node_27;
    wire ic_jkflipflop_ic_jkflipflop_node_28;
    assign ic_jkflipflop_ic_jkflipflop_node_10 = ic_jkflipflop_ic_jkflipflop_node_20; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_11 = ic_jkflipflop_ic_jkflipflop_node_22; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_13 = ic_jkflipflop_ic_jkflipflop_node_16; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_14 = ic_jkflipflop_ic_jkflipflop_node_15; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_15 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_16 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_18 = ~ic_jkflipflop_ic_jkflipflop_node_23; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_19 = (ic_jkflipflop_ic_jkflipflop_node_25 & ic_jkflipflop_ic_jkflipflop_node_14); // And
    assign ic_jkflipflop_ic_jkflipflop_node_20 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_21 = (ic_jkflipflop_ic_jkflipflop_and_26 | ic_jkflipflop_ic_jkflipflop_and_19); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_22 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_23 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_24 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_25 = ic_jkflipflop_ic_jkflipflop_node_24; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_26 = (ic_jkflipflop_ic_jkflipflop_node_13 & ic_jkflipflop_ic_jkflipflop_not_18); // And
    assign ic_jkflipflop_ic_jkflipflop_node_27 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_28 = 1'b0; // Node
// ============== END IC: JKFLIPFLOP ==============
// ============== BEGIN IC: DECODER ==============
// IC inputs: 3, IC outputs: 8
// Nesting depth: 0
    wire ic_decoder_ic_decoder_node_29;
    wire ic_decoder_ic_decoder_node_30;
    wire ic_decoder_ic_decoder_node_31;
    wire ic_decoder_ic_decoder_node_32;
    wire ic_decoder_ic_decoder_node_33;
    wire ic_decoder_ic_decoder_node_34;
    wire ic_decoder_ic_decoder_node_35;
    wire ic_decoder_ic_decoder_node_36;
    wire ic_decoder_ic_decoder_not_37;
    wire ic_decoder_ic_decoder_not_38;
    wire ic_decoder_ic_decoder_not_39;
    wire ic_decoder_ic_decoder_node_40;
    wire ic_decoder_ic_decoder_node_41;
    wire ic_decoder_ic_decoder_node_42;
    wire ic_decoder_ic_decoder_node_43;
    wire ic_decoder_ic_decoder_node_44;
    wire ic_decoder_ic_decoder_node_45;
    wire ic_decoder_ic_decoder_node_46;
    wire ic_decoder_ic_decoder_node_47;
    wire ic_decoder_ic_decoder_node_48;
    wire ic_decoder_ic_decoder_node_49;
    wire ic_decoder_ic_decoder_node_50;
    wire ic_decoder_ic_decoder_node_51;
    wire ic_decoder_ic_decoder_and_52;
    wire ic_decoder_ic_decoder_and_53;
    wire ic_decoder_ic_decoder_and_54;
    wire ic_decoder_ic_decoder_and_55;
    wire ic_decoder_ic_decoder_and_56;
    wire ic_decoder_ic_decoder_and_57;
    wire ic_decoder_ic_decoder_and_58;
    wire ic_decoder_ic_decoder_and_59;
    wire ic_decoder_ic_decoder_node_60;
    wire ic_decoder_ic_decoder_node_61;
    wire ic_decoder_ic_decoder_node_62;
    wire ic_decoder_ic_decoder_node_63;
    wire ic_decoder_ic_decoder_node_64;
    wire ic_decoder_ic_decoder_node_65;
    wire ic_decoder_ic_decoder_node_66;
    wire ic_decoder_ic_decoder_node_67;
    wire ic_decoder_ic_decoder_node_68;
    wire ic_decoder_ic_decoder_node_69;
    wire ic_decoder_ic_decoder_node_70;
    wire ic_decoder_ic_decoder_node_71;
    wire ic_decoder_ic_decoder_node_72;
    wire ic_decoder_ic_decoder_node_73;
    wire ic_decoder_ic_decoder_node_74;
    wire ic_decoder_ic_decoder_node_75;
    wire ic_decoder_ic_decoder_node_76;
    wire ic_decoder_ic_decoder_node_77;
    wire ic_decoder_ic_decoder_node_78;
    wire ic_decoder_ic_decoder_node_79;
    wire ic_decoder_ic_decoder_node_80;
    wire ic_decoder_ic_decoder_node_81;
    wire ic_decoder_ic_decoder_node_82;
    wire ic_decoder_ic_decoder_node_83;
    assign ic_decoder_ic_decoder_node_29 = ic_decoder_ic_decoder_and_59; // Node
    assign ic_decoder_ic_decoder_node_30 = ic_decoder_ic_decoder_and_58; // Node
    assign ic_decoder_ic_decoder_node_31 = ic_decoder_ic_decoder_and_57; // Node
    assign ic_decoder_ic_decoder_node_32 = ic_decoder_ic_decoder_and_56; // Node
    assign ic_decoder_ic_decoder_node_33 = ic_decoder_ic_decoder_and_55; // Node
    assign ic_decoder_ic_decoder_node_34 = ic_decoder_ic_decoder_and_54; // Node
    assign ic_decoder_ic_decoder_node_35 = ic_decoder_ic_decoder_and_53; // Node
    assign ic_decoder_ic_decoder_node_36 = ic_decoder_ic_decoder_and_52; // Node
    assign ic_decoder_ic_decoder_not_37 = ~ic_decoder_ic_decoder_node_40; // Not
    assign ic_decoder_ic_decoder_not_38 = ~ic_decoder_ic_decoder_node_41; // Not
    assign ic_decoder_ic_decoder_not_39 = ~ic_decoder_ic_decoder_node_42; // Not
    assign ic_decoder_ic_decoder_node_40 = ic_decoder_ic_decoder_node_65; // Node
    assign ic_decoder_ic_decoder_node_41 = ic_decoder_ic_decoder_node_63; // Node
    assign ic_decoder_ic_decoder_node_42 = ic_decoder_ic_decoder_node_64; // Node
    assign ic_decoder_ic_decoder_node_43 = ic_decoder_ic_decoder_node_83; // Node
    assign ic_decoder_ic_decoder_node_44 = ic_decoder_ic_decoder_node_71; // Node
    assign ic_decoder_ic_decoder_node_45 = ic_decoder_ic_decoder_node_82; // Node
    assign ic_decoder_ic_decoder_node_46 = ic_decoder_ic_decoder_node_76; // Node
    assign ic_decoder_ic_decoder_node_47 = ic_decoder_ic_decoder_node_81; // Node
    assign ic_decoder_ic_decoder_node_48 = ic_decoder_ic_decoder_node_78; // Node
    assign ic_decoder_ic_decoder_node_49 = ic_decoder_ic_decoder_not_37; // Node
    assign ic_decoder_ic_decoder_node_50 = ic_decoder_ic_decoder_not_38; // Node
    assign ic_decoder_ic_decoder_node_51 = ic_decoder_ic_decoder_not_39; // Node
    assign ic_decoder_ic_decoder_and_52 = (ic_decoder_ic_decoder_node_51 & ic_decoder_ic_decoder_node_50 & ic_decoder_ic_decoder_node_49); // And
    assign ic_decoder_ic_decoder_and_53 = (ic_decoder_ic_decoder_node_62 & ic_decoder_ic_decoder_node_61 & ic_decoder_ic_decoder_node_60); // And
    assign ic_decoder_ic_decoder_and_54 = (ic_decoder_ic_decoder_node_68 & ic_decoder_ic_decoder_node_67 & ic_decoder_ic_decoder_node_66); // And
    assign ic_decoder_ic_decoder_and_55 = (ic_decoder_ic_decoder_node_71 & ic_decoder_ic_decoder_node_70 & ic_decoder_ic_decoder_node_69); // And
    assign ic_decoder_ic_decoder_and_56 = (ic_decoder_ic_decoder_node_72 & ic_decoder_ic_decoder_node_73 & ic_decoder_ic_decoder_node_74); // And
    assign ic_decoder_ic_decoder_and_57 = (ic_decoder_ic_decoder_node_77 & ic_decoder_ic_decoder_node_76 & ic_decoder_ic_decoder_node_75); // And
    assign ic_decoder_ic_decoder_and_58 = (ic_decoder_ic_decoder_node_80 & ic_decoder_ic_decoder_node_79 & ic_decoder_ic_decoder_node_78); // And
    assign ic_decoder_ic_decoder_and_59 = (ic_decoder_ic_decoder_node_83 & ic_decoder_ic_decoder_node_82 & ic_decoder_ic_decoder_node_81); // And
    assign ic_decoder_ic_decoder_node_60 = ic_decoder_ic_decoder_node_42; // Node
    assign ic_decoder_ic_decoder_node_61 = ic_decoder_ic_decoder_node_50; // Node
    assign ic_decoder_ic_decoder_node_62 = ic_decoder_ic_decoder_node_49; // Node
    assign ic_decoder_ic_decoder_node_63 = 1'b0; // Node
    assign ic_decoder_ic_decoder_node_64 = 1'b0; // Node
    assign ic_decoder_ic_decoder_node_65 = 1'b0; // Node
    assign ic_decoder_ic_decoder_node_66 = ic_decoder_ic_decoder_node_51; // Node
    assign ic_decoder_ic_decoder_node_67 = ic_decoder_ic_decoder_node_41; // Node
    assign ic_decoder_ic_decoder_node_68 = ic_decoder_ic_decoder_node_62; // Node
    assign ic_decoder_ic_decoder_node_69 = ic_decoder_ic_decoder_node_60; // Node
    assign ic_decoder_ic_decoder_node_70 = ic_decoder_ic_decoder_node_67; // Node
    assign ic_decoder_ic_decoder_node_71 = ic_decoder_ic_decoder_node_68; // Node
    assign ic_decoder_ic_decoder_node_72 = ic_decoder_ic_decoder_node_40; // Node
    assign ic_decoder_ic_decoder_node_73 = ic_decoder_ic_decoder_node_61; // Node
    assign ic_decoder_ic_decoder_node_74 = ic_decoder_ic_decoder_node_66; // Node
    assign ic_decoder_ic_decoder_node_75 = ic_decoder_ic_decoder_node_69; // Node
    assign ic_decoder_ic_decoder_node_76 = ic_decoder_ic_decoder_node_73; // Node
    assign ic_decoder_ic_decoder_node_77 = ic_decoder_ic_decoder_node_72; // Node
    assign ic_decoder_ic_decoder_node_78 = ic_decoder_ic_decoder_node_74; // Node
    assign ic_decoder_ic_decoder_node_79 = ic_decoder_ic_decoder_node_70; // Node
    assign ic_decoder_ic_decoder_node_80 = ic_decoder_ic_decoder_node_77; // Node
    assign ic_decoder_ic_decoder_node_81 = ic_decoder_ic_decoder_node_75; // Node
    assign ic_decoder_ic_decoder_node_82 = ic_decoder_ic_decoder_node_79; // Node
    assign ic_decoder_ic_decoder_node_83 = ic_decoder_ic_decoder_node_80; // Node
// ============== END IC: DECODER ==============
    reg jk_flip_flop_84_0_q = 1'b0;
    reg jk_flip_flop_84_1_q = 1'b0;
    reg jk_flip_flop_85_0_q = 1'b0;
    reg jk_flip_flop_85_1_q = 1'b0;
    reg jk_flip_flop_86_0_q = 1'b0;
    reg jk_flip_flop_86_1_q = 1'b0;

    // ========= Logic Assignments =========
    assign ic_decoder_ic_decoder_node_29 = ic_decoder_ic_decoder_and_59; // Node
    assign ic_decoder_ic_decoder_node_30 = ic_decoder_ic_decoder_and_58; // Node
    assign ic_decoder_ic_decoder_node_31 = ic_decoder_ic_decoder_and_57; // Node
    assign ic_decoder_ic_decoder_node_32 = ic_decoder_ic_decoder_and_56; // Node
    assign ic_decoder_ic_decoder_node_33 = ic_decoder_ic_decoder_and_55; // Node
    assign ic_decoder_ic_decoder_node_34 = ic_decoder_ic_decoder_and_54; // Node
    assign ic_decoder_ic_decoder_node_35 = ic_decoder_ic_decoder_and_53; // Node
    assign ic_decoder_ic_decoder_node_36 = ic_decoder_ic_decoder_and_52; // Node
    assign ic_decoder_ic_decoder_not_37 = ~ic_decoder_ic_decoder_node_40; // Not
    assign ic_decoder_ic_decoder_not_38 = ~ic_decoder_ic_decoder_node_41; // Not
    assign ic_decoder_ic_decoder_not_39 = ~ic_decoder_ic_decoder_node_42; // Not
    assign ic_decoder_ic_decoder_node_40 = ic_decoder_ic_decoder_node_65; // Node
    assign ic_decoder_ic_decoder_node_41 = ic_decoder_ic_decoder_node_63; // Node
    assign ic_decoder_ic_decoder_node_42 = ic_decoder_ic_decoder_node_64; // Node
    assign ic_decoder_ic_decoder_node_43 = ic_decoder_ic_decoder_node_83; // Node
    assign ic_decoder_ic_decoder_node_44 = ic_decoder_ic_decoder_node_71; // Node
    assign ic_decoder_ic_decoder_node_45 = ic_decoder_ic_decoder_node_82; // Node
    assign ic_decoder_ic_decoder_node_46 = ic_decoder_ic_decoder_node_76; // Node
    assign ic_decoder_ic_decoder_node_47 = ic_decoder_ic_decoder_node_81; // Node
    assign ic_decoder_ic_decoder_node_48 = ic_decoder_ic_decoder_node_78; // Node
    assign ic_decoder_ic_decoder_node_49 = ic_decoder_ic_decoder_not_37; // Node
    assign ic_decoder_ic_decoder_node_50 = ic_decoder_ic_decoder_not_38; // Node
    assign ic_decoder_ic_decoder_node_51 = ic_decoder_ic_decoder_not_39; // Node
    assign ic_decoder_ic_decoder_and_52 = (ic_decoder_ic_decoder_node_51 & ic_decoder_ic_decoder_node_50 & ic_decoder_ic_decoder_node_49); // And
    assign ic_decoder_ic_decoder_and_53 = (ic_decoder_ic_decoder_node_62 & ic_decoder_ic_decoder_node_61 & ic_decoder_ic_decoder_node_60); // And
    assign ic_decoder_ic_decoder_and_54 = (ic_decoder_ic_decoder_node_68 & ic_decoder_ic_decoder_node_67 & ic_decoder_ic_decoder_node_66); // And
    assign ic_decoder_ic_decoder_and_55 = (ic_decoder_ic_decoder_node_71 & ic_decoder_ic_decoder_node_70 & ic_decoder_ic_decoder_node_69); // And
    assign ic_decoder_ic_decoder_and_56 = (ic_decoder_ic_decoder_node_72 & ic_decoder_ic_decoder_node_73 & ic_decoder_ic_decoder_node_74); // And
    assign ic_decoder_ic_decoder_and_57 = (ic_decoder_ic_decoder_node_77 & ic_decoder_ic_decoder_node_76 & ic_decoder_ic_decoder_node_75); // And
    assign ic_decoder_ic_decoder_and_58 = (ic_decoder_ic_decoder_node_80 & ic_decoder_ic_decoder_node_79 & ic_decoder_ic_decoder_node_78); // And
    assign ic_decoder_ic_decoder_and_59 = (ic_decoder_ic_decoder_node_83 & ic_decoder_ic_decoder_node_82 & ic_decoder_ic_decoder_node_81); // And
    assign ic_decoder_ic_decoder_node_60 = ic_decoder_ic_decoder_node_42; // Node
    assign ic_decoder_ic_decoder_node_61 = ic_decoder_ic_decoder_node_50; // Node
    assign ic_decoder_ic_decoder_node_62 = ic_decoder_ic_decoder_node_49; // Node
    assign ic_decoder_ic_decoder_node_63 = 1'b0; // Node
    assign ic_decoder_ic_decoder_node_64 = 1'b0; // Node
    assign ic_decoder_ic_decoder_node_65 = 1'b0; // Node
    assign ic_decoder_ic_decoder_node_66 = ic_decoder_ic_decoder_node_51; // Node
    assign ic_decoder_ic_decoder_node_67 = ic_decoder_ic_decoder_node_41; // Node
    assign ic_decoder_ic_decoder_node_68 = ic_decoder_ic_decoder_node_62; // Node
    assign ic_decoder_ic_decoder_node_69 = ic_decoder_ic_decoder_node_60; // Node
    assign ic_decoder_ic_decoder_node_70 = ic_decoder_ic_decoder_node_67; // Node
    assign ic_decoder_ic_decoder_node_71 = ic_decoder_ic_decoder_node_68; // Node
    assign ic_decoder_ic_decoder_node_72 = ic_decoder_ic_decoder_node_40; // Node
    assign ic_decoder_ic_decoder_node_73 = ic_decoder_ic_decoder_node_61; // Node
    assign ic_decoder_ic_decoder_node_74 = ic_decoder_ic_decoder_node_66; // Node
    assign ic_decoder_ic_decoder_node_75 = ic_decoder_ic_decoder_node_69; // Node
    assign ic_decoder_ic_decoder_node_76 = ic_decoder_ic_decoder_node_73; // Node
    assign ic_decoder_ic_decoder_node_77 = ic_decoder_ic_decoder_node_72; // Node
    assign ic_decoder_ic_decoder_node_78 = ic_decoder_ic_decoder_node_74; // Node
    assign ic_decoder_ic_decoder_node_79 = ic_decoder_ic_decoder_node_70; // Node
    assign ic_decoder_ic_decoder_node_80 = ic_decoder_ic_decoder_node_77; // Node
    assign ic_decoder_ic_decoder_node_81 = ic_decoder_ic_decoder_node_75; // Node
    assign ic_decoder_ic_decoder_node_82 = ic_decoder_ic_decoder_node_79; // Node
    assign ic_decoder_ic_decoder_node_83 = ic_decoder_ic_decoder_node_80; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_85_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_86_0_q <= 1'b0; jk_flip_flop_86_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_86_0_q <= 1'b1; jk_flip_flop_86_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_86_0_q <= jk_flip_flop_86_1_q; jk_flip_flop_86_1_q <= jk_flip_flop_86_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_84_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_85_0_q <= 1'b0; jk_flip_flop_85_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_85_0_q <= 1'b1; jk_flip_flop_85_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_85_0_q <= jk_flip_flop_85_1_q; jk_flip_flop_85_1_q <= jk_flip_flop_85_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        jk_flip_flop_84_0_q = 1'b0;
        jk_flip_flop_84_1_q = 1'b1;
    end

    assign ic_jkflipflop_ic_jkflipflop_node_10 = ic_jkflipflop_ic_jkflipflop_node_20; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_11 = ic_jkflipflop_ic_jkflipflop_node_22; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_13 = ic_jkflipflop_ic_jkflipflop_node_16; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_14 = ic_jkflipflop_ic_jkflipflop_node_15; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_15 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_16 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_not_18 = ~ic_jkflipflop_ic_jkflipflop_node_23; // Not
    assign ic_jkflipflop_ic_jkflipflop_and_19 = (ic_jkflipflop_ic_jkflipflop_node_25 & ic_jkflipflop_ic_jkflipflop_node_14); // And
    assign ic_jkflipflop_ic_jkflipflop_node_20 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_or_21 = (ic_jkflipflop_ic_jkflipflop_and_26 | ic_jkflipflop_ic_jkflipflop_and_19); // Or
    assign ic_jkflipflop_ic_jkflipflop_node_22 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_23 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_24 = 1'b1; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_25 = ic_jkflipflop_ic_jkflipflop_node_24; // Node
    assign ic_jkflipflop_ic_jkflipflop_and_26 = (ic_jkflipflop_ic_jkflipflop_node_13 & ic_jkflipflop_ic_jkflipflop_not_18); // And
    assign ic_jkflipflop_ic_jkflipflop_node_27 = 1'b0; // Node
    assign ic_jkflipflop_ic_jkflipflop_node_28 = 1'b0; // Node

    // ========= Output Assignments =========
    assign output_buzzer1_g6_2 = 1'b0; // Buzzer
    assign output_buzzer2_f6_3 = 1'b0; // Buzzer
    assign output_buzzer3_d6_4 = 1'b0; // Buzzer
    assign output_buzzer4_b7_5 = 1'b0; // Buzzer
    assign output_buzzer5_c6_6 = 1'b0; // Buzzer
    assign output_buzzer6_a7_7 = 1'b0; // Buzzer
    assign output_buzzer7_e6_8 = 1'b0; // Buzzer
    assign output_buzzer8_c7_9 = 1'b0; // Buzzer

endmodule // notes

// ====================================================================
// Module notes generation completed successfully
// Elements processed: 14
// Inputs: 1, Outputs: 8
// Warnings: 5
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 4 is not connected
//   IC JKFLIPFLOP output 1 is not connected
// ====================================================================
