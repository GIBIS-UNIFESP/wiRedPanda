// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: sequential
// Generated: Fri Sep 26 14:59:15 2025
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
    wire ic_serialize_ic_serialize_node_17;
    wire ic_serialize_ic_serialize_node_18;
    wire ic_serialize_ic_serialize_node_19;
    wire ic_serialize_ic_serialize_node_20;
    wire ic_serialize_ic_serialize_and_26;
    wire ic_serialize_ic_serialize_node_27;
    wire ic_serialize_ic_serialize_not_28;
    wire ic_serialize_ic_serialize_and_29;
    wire ic_serialize_ic_serialize_or_30;
    wire ic_serialize_ic_serialize_and_31;
    reg ic_serialize_ic_serialize_d_flip_flop_32_0 = 1'b0;
    reg ic_serialize_ic_serialize_d_flip_flop_32_1 = 1'b0;
    wire ic_serialize_ic_serialize_and_33;
    wire ic_serialize_ic_serialize_or_34;
    reg ic_serialize_ic_serialize_d_flip_flop_35_0 = 1'b0;
    reg ic_serialize_ic_serialize_d_flip_flop_35_1 = 1'b0;
    wire ic_serialize_ic_serialize_and_36;
    reg ic_serialize_ic_serialize_d_flip_flop_37_0 = 1'b0;
    reg ic_serialize_ic_serialize_d_flip_flop_37_1 = 1'b0;
    wire ic_serialize_ic_serialize_node_38;
    wire ic_serialize_ic_serialize_node_39;
    wire ic_serialize_ic_serialize_node_40;
    wire ic_serialize_ic_serialize_node_41;
    wire ic_serialize_ic_serialize_and_42;
    reg ic_serialize_ic_serialize_d_flip_flop_43_0 = 1'b0;
    reg ic_serialize_ic_serialize_d_flip_flop_43_1 = 1'b0;
    wire ic_serialize_ic_serialize_and_44;
    wire ic_serialize_ic_serialize_or_45;
    assign ic_serialize_ic_serialize_node_17 = ic_serialize_ic_serialize_not_28; // Node
    assign ic_serialize_ic_serialize_node_18 = ic_serialize_ic_serialize_not_28; // Node
    assign ic_serialize_ic_serialize_node_19 = ic_serialize_ic_serialize_not_28; // Node
    assign ic_serialize_ic_serialize_node_20 = ic_serialize_ic_serialize_not_28; // Node
    assign ic_serialize_ic_serialize_and_26 = (ic_serialize_ic_serialize_node_17 & ic_serialize_ic_serialize_d_flip_flop_32_0); // And
    assign ic_serialize_ic_serialize_node_27 = ic_serialize_ic_serialize_and_26; // Node
    assign ic_serialize_ic_serialize_not_28 = ~input_clock2_slow_clk_2; // Not
    assign ic_serialize_ic_serialize_and_29 = (ic_serialize_ic_serialize_node_18 & ic_serialize_ic_serialize_d_flip_flop_35_0); // And
    assign ic_serialize_ic_serialize_or_30 = (ic_serialize_ic_serialize_and_31 | ic_serialize_ic_serialize_and_29); // Or
    assign ic_serialize_ic_serialize_and_31 = (input_clock2_slow_clk_2 & ic_serialize_ic_serialize_node_41); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock3_fast_clk_3) begin
        begin
            ic_serialize_ic_serialize_d_flip_flop_32_0 <= ic_serialize_ic_serialize_or_30;
            ic_serialize_ic_serialize_d_flip_flop_32_1 <= ~ic_serialize_ic_serialize_or_30;
        end
    end

    assign ic_serialize_ic_serialize_and_33 = (input_clock2_slow_clk_2 & ic_serialize_ic_serialize_node_40); // And
    assign ic_serialize_ic_serialize_or_34 = (ic_serialize_ic_serialize_and_33 | ic_serialize_ic_serialize_and_36); // Or
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock3_fast_clk_3) begin
        begin
            ic_serialize_ic_serialize_d_flip_flop_35_0 <= ic_serialize_ic_serialize_or_34;
            ic_serialize_ic_serialize_d_flip_flop_35_1 <= ~ic_serialize_ic_serialize_or_34;
        end
    end

    assign ic_serialize_ic_serialize_and_36 = (ic_serialize_ic_serialize_node_19 & ic_serialize_ic_serialize_d_flip_flop_37_0); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock3_fast_clk_3) begin
        begin
            ic_serialize_ic_serialize_d_flip_flop_37_0 <= ic_serialize_ic_serialize_or_45;
            ic_serialize_ic_serialize_d_flip_flop_37_1 <= ~ic_serialize_ic_serialize_or_45;
        end
    end

    assign ic_serialize_ic_serialize_node_38 = 1'b0; // Node
    assign ic_serialize_ic_serialize_node_39 = 1'b1; // Node
    assign ic_serialize_ic_serialize_node_40 = 1'b0; // Node
    assign ic_serialize_ic_serialize_node_41 = 1'b1; // Node
    assign ic_serialize_ic_serialize_and_42 = (ic_serialize_ic_serialize_node_20 & ic_serialize_ic_serialize_d_flip_flop_43_0); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock3_fast_clk_3) begin
        begin
            ic_serialize_ic_serialize_d_flip_flop_43_0 <= ic_serialize_ic_serialize_node_38;
            ic_serialize_ic_serialize_d_flip_flop_43_1 <= ~ic_serialize_ic_serialize_node_38;
        end
    end

    assign ic_serialize_ic_serialize_and_44 = (input_clock2_slow_clk_2 & ic_serialize_ic_serialize_node_39); // And
    assign ic_serialize_ic_serialize_or_45 = (ic_serialize_ic_serialize_and_44 | ic_serialize_ic_serialize_and_42); // Or
// ============== END IC: SERIALIZE ==============
// ============== BEGIN IC: REGISTER ==============
// IC inputs: 2, IC outputs: 4
// Nesting depth: 0
    reg ic_register_ic_register_d_flip_flop_47_0 = 1'b0;
    reg ic_register_ic_register_d_flip_flop_47_1 = 1'b0;
    wire ic_register_ic_register_node_49;
    reg ic_register_ic_register_d_flip_flop_50_0 = 1'b0;
    reg ic_register_ic_register_d_flip_flop_50_1 = 1'b0;
    wire ic_register_ic_register_node_51;
    reg ic_register_ic_register_d_flip_flop_52_0 = 1'b0;
    reg ic_register_ic_register_d_flip_flop_52_1 = 1'b0;
    wire ic_register_ic_register_node_54;
    wire ic_register_ic_register_node_55;
    wire ic_register_ic_register_node_57;
    reg ic_register_ic_register_d_flip_flop_60_0 = 1'b0;
    reg ic_register_ic_register_d_flip_flop_60_1 = 1'b0;
    wire ic_register_ic_register_d_flip_flop_47_0_clk_wire;
    assign ic_register_ic_register_d_flip_flop_47_0_clk_wire = (~1'b0 & input_clock3_fast_clk_3); // Clock expression wire
    // D FlipFlop: D-Flip-Flop
    always @(posedge ic_register_ic_register_d_flip_flop_47_0_clk_wire) begin
        begin
            ic_register_ic_register_d_flip_flop_47_0 <= ic_register_ic_register_d_flip_flop_52_0;
            ic_register_ic_register_d_flip_flop_47_1 <= ~ic_register_ic_register_d_flip_flop_52_0;
        end
    end

    assign ic_register_ic_register_node_49 = 1'b0; // Node
    wire ic_register_ic_register_d_flip_flop_50_0_clk_wire;
    assign ic_register_ic_register_d_flip_flop_50_0_clk_wire = (~1'b0 & input_clock3_fast_clk_3); // Clock expression wire
    // D FlipFlop: D-Flip-Flop
    always @(posedge ic_register_ic_register_d_flip_flop_50_0_clk_wire) begin
        begin
            ic_register_ic_register_d_flip_flop_50_0 <= ic_register_ic_register_d_flip_flop_47_0;
            ic_register_ic_register_d_flip_flop_50_1 <= ~ic_register_ic_register_d_flip_flop_47_0;
        end
    end

    assign ic_register_ic_register_node_51 = ic_register_ic_register_d_flip_flop_60_0; // Node
    wire ic_register_ic_register_d_flip_flop_52_0_clk_wire;
    assign ic_register_ic_register_d_flip_flop_52_0_clk_wire = (~1'b0 & input_clock3_fast_clk_3); // Clock expression wire
    // D FlipFlop: D-Flip-Flop
    always @(posedge ic_register_ic_register_d_flip_flop_52_0_clk_wire) begin
        begin
            ic_register_ic_register_d_flip_flop_52_0 <= ic_register_ic_register_d_flip_flop_60_0;
            ic_register_ic_register_d_flip_flop_52_1 <= ~ic_register_ic_register_d_flip_flop_60_0;
        end
    end

    assign ic_register_ic_register_node_54 = ic_register_ic_register_d_flip_flop_47_0; // Node
    assign ic_register_ic_register_node_55 = ic_register_ic_register_d_flip_flop_50_0; // Node
    assign ic_register_ic_register_node_57 = ic_register_ic_register_d_flip_flop_52_0; // Node
    wire ic_register_ic_register_d_flip_flop_60_0_clk_wire;
    assign ic_register_ic_register_d_flip_flop_60_0_clk_wire = (~1'b0 & input_clock3_fast_clk_3); // Clock expression wire
    // D FlipFlop: D-Flip-Flop
    always @(posedge ic_register_ic_register_d_flip_flop_60_0_clk_wire) begin
        begin
            ic_register_ic_register_d_flip_flop_60_0 <= ic_register_ic_register_node_49;
            ic_register_ic_register_d_flip_flop_60_1 <= ~ic_register_ic_register_node_49;
        end
    end

// ============== END IC: REGISTER ==============
    wire not_61;
    wire node_62;
    wire node_63;
    reg jk_flip_flop_64_0_q = 1'b0;
    reg jk_flip_flop_64_1_q = 1'b0;
    wire node_65;
    reg jk_flip_flop_66_0_q = 1'b0;
    reg jk_flip_flop_66_1_q = 1'b0;
    wire node_67;
    reg jk_flip_flop_68_0_q = 1'b0;
    reg jk_flip_flop_68_1_q = 1'b0;
    wire node_69;
    reg jk_flip_flop_70_0_q = 1'b0;
    reg jk_flip_flop_70_1_q = 1'b0;
    wire node_71;
    wire node_72;
    wire node_73;
    wire node_74;
    wire node_75;
    wire node_76;
    wire node_77;
    wire node_78;
    wire not_79;
    wire node_80;
    wire node_81;
    wire and_82;
    wire node_83;
    wire node_84;
    wire and_85;
    wire and_86;
    wire and_87;
    wire and_88;

    // ========= Logic Assignments =========
    assign and_88 = (1'b0 & node_84); // And
    assign and_87 = (1'b0 & node_83); // And
    assign and_86 = (1'b0 & node_81); // And
    assign and_85 = (1'b0 & node_78); // And
    assign node_84 = node_83; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge ic_register_ic_register_d_flip_flop_47_0_clk_wire) begin
        begin
            ic_register_ic_register_d_flip_flop_47_0 <= ic_register_ic_register_d_flip_flop_52_0;
            ic_register_ic_register_d_flip_flop_47_1 <= ~ic_register_ic_register_d_flip_flop_52_0;
        end
    end

    assign ic_register_ic_register_node_49 = 1'b0; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge ic_register_ic_register_d_flip_flop_50_0_clk_wire) begin
        begin
            ic_register_ic_register_d_flip_flop_50_0 <= ic_register_ic_register_d_flip_flop_47_0;
            ic_register_ic_register_d_flip_flop_50_1 <= ~ic_register_ic_register_d_flip_flop_47_0;
        end
    end

    assign ic_register_ic_register_node_51 = ic_register_ic_register_d_flip_flop_60_0; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge ic_register_ic_register_d_flip_flop_52_0_clk_wire) begin
        begin
            ic_register_ic_register_d_flip_flop_52_0 <= ic_register_ic_register_d_flip_flop_60_0;
            ic_register_ic_register_d_flip_flop_52_1 <= ~ic_register_ic_register_d_flip_flop_60_0;
        end
    end

    assign ic_register_ic_register_node_54 = ic_register_ic_register_d_flip_flop_47_0; // Node
    assign ic_register_ic_register_node_55 = ic_register_ic_register_d_flip_flop_50_0; // Node
    assign ic_register_ic_register_node_57 = ic_register_ic_register_d_flip_flop_52_0; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge ic_register_ic_register_d_flip_flop_60_0_clk_wire) begin
        begin
            ic_register_ic_register_d_flip_flop_60_0 <= ic_register_ic_register_node_49;
            ic_register_ic_register_d_flip_flop_60_1 <= ~ic_register_ic_register_node_49;
        end
    end

    assign node_83 = node_81; // Node
    assign and_82 = (not_79 & node_80); // And
    assign node_81 = node_78; // Node
    assign node_80 = node_77; // Node
    assign not_79 = ~1'b0; // Not
    assign node_78 = node_76; // Node
    assign ic_serialize_ic_serialize_node_17 = ic_serialize_ic_serialize_not_28; // Node
    assign ic_serialize_ic_serialize_node_18 = ic_serialize_ic_serialize_not_28; // Node
    assign ic_serialize_ic_serialize_node_19 = ic_serialize_ic_serialize_not_28; // Node
    assign ic_serialize_ic_serialize_node_20 = ic_serialize_ic_serialize_not_28; // Node
    assign ic_serialize_ic_serialize_and_26 = (ic_serialize_ic_serialize_node_17 & ic_serialize_ic_serialize_d_flip_flop_32_0); // And
    assign ic_serialize_ic_serialize_node_27 = ic_serialize_ic_serialize_and_26; // Node
    assign ic_serialize_ic_serialize_not_28 = ~input_clock2_slow_clk_2; // Not
    assign ic_serialize_ic_serialize_and_29 = (ic_serialize_ic_serialize_node_18 & ic_serialize_ic_serialize_d_flip_flop_35_0); // And
    assign ic_serialize_ic_serialize_or_30 = (ic_serialize_ic_serialize_and_31 | ic_serialize_ic_serialize_and_29); // Or
    assign ic_serialize_ic_serialize_and_31 = (input_clock2_slow_clk_2 & ic_serialize_ic_serialize_node_41); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock3_fast_clk_3) begin
        begin
            ic_serialize_ic_serialize_d_flip_flop_32_0 <= ic_serialize_ic_serialize_or_30;
            ic_serialize_ic_serialize_d_flip_flop_32_1 <= ~ic_serialize_ic_serialize_or_30;
        end
    end

    assign ic_serialize_ic_serialize_and_33 = (input_clock2_slow_clk_2 & ic_serialize_ic_serialize_node_40); // And
    assign ic_serialize_ic_serialize_or_34 = (ic_serialize_ic_serialize_and_33 | ic_serialize_ic_serialize_and_36); // Or
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock3_fast_clk_3) begin
        begin
            ic_serialize_ic_serialize_d_flip_flop_35_0 <= ic_serialize_ic_serialize_or_34;
            ic_serialize_ic_serialize_d_flip_flop_35_1 <= ~ic_serialize_ic_serialize_or_34;
        end
    end

    assign ic_serialize_ic_serialize_and_36 = (ic_serialize_ic_serialize_node_19 & ic_serialize_ic_serialize_d_flip_flop_37_0); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock3_fast_clk_3) begin
        begin
            ic_serialize_ic_serialize_d_flip_flop_37_0 <= ic_serialize_ic_serialize_or_45;
            ic_serialize_ic_serialize_d_flip_flop_37_1 <= ~ic_serialize_ic_serialize_or_45;
        end
    end

    assign ic_serialize_ic_serialize_node_38 = 1'b0; // Node
    assign ic_serialize_ic_serialize_node_39 = 1'b1; // Node
    assign ic_serialize_ic_serialize_node_40 = 1'b0; // Node
    assign ic_serialize_ic_serialize_node_41 = 1'b1; // Node
    assign ic_serialize_ic_serialize_and_42 = (ic_serialize_ic_serialize_node_20 & ic_serialize_ic_serialize_d_flip_flop_43_0); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock3_fast_clk_3) begin
        begin
            ic_serialize_ic_serialize_d_flip_flop_43_0 <= ic_serialize_ic_serialize_node_38;
            ic_serialize_ic_serialize_d_flip_flop_43_1 <= ~ic_serialize_ic_serialize_node_38;
        end
    end

    assign ic_serialize_ic_serialize_and_44 = (input_clock2_slow_clk_2 & ic_serialize_ic_serialize_node_39); // And
    assign ic_serialize_ic_serialize_or_45 = (ic_serialize_ic_serialize_and_44 | ic_serialize_ic_serialize_and_42); // Or
    assign node_77 = input_clock3_fast_clk_3; // Node
    assign node_76 = node_71; // Node
    assign node_75 = jk_flip_flop_70_1_q; // Node
    assign node_74 = jk_flip_flop_68_1_q; // Node
    assign node_73 = jk_flip_flop_66_1_q; // Node
    assign node_72 = jk_flip_flop_64_1_q; // Node
    assign node_71 = input_clock2_slow_clk_2; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_68_0_q or negedge node_69) begin
        if (!node_69) begin
            jk_flip_flop_70_0_q <= 1'b1;
            jk_flip_flop_70_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_70_0_q <= 1'b0; jk_flip_flop_70_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_70_0_q <= 1'b1; jk_flip_flop_70_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_70_0_q <= jk_flip_flop_70_1_q; jk_flip_flop_70_1_q <= jk_flip_flop_70_0_q; end // toggle
            endcase
        end
    end

    assign node_69 = node_67; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_66_0_q or negedge node_67) begin
        if (!node_67) begin
            jk_flip_flop_68_0_q <= 1'b1;
            jk_flip_flop_68_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_68_0_q <= 1'b0; jk_flip_flop_68_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_68_0_q <= 1'b1; jk_flip_flop_68_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_68_0_q <= jk_flip_flop_68_1_q; jk_flip_flop_68_1_q <= jk_flip_flop_68_0_q; end // toggle
            endcase
        end
    end

    assign node_67 = node_65; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_64_0_q or negedge node_65) begin
        if (!node_65) begin
            jk_flip_flop_66_0_q <= 1'b1;
            jk_flip_flop_66_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_66_0_q <= 1'b0; jk_flip_flop_66_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_66_0_q <= 1'b1; jk_flip_flop_66_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_66_0_q <= jk_flip_flop_66_1_q; jk_flip_flop_66_1_q <= jk_flip_flop_66_0_q; end // toggle
            endcase
        end
    end

    assign node_65 = node_63; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock2_slow_clk_2 or negedge node_63) begin
        if (!node_63) begin
            jk_flip_flop_64_0_q <= 1'b1;
            jk_flip_flop_64_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_64_0_q <= 1'b0; jk_flip_flop_64_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_64_0_q <= 1'b1; jk_flip_flop_64_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_64_0_q <= jk_flip_flop_64_1_q; jk_flip_flop_64_1_q <= jk_flip_flop_64_0_q; end // toggle
            endcase
        end
    end

    assign node_63 = node_62; // Node
    assign node_62 = not_61; // Node
    assign not_61 = ~input_push_button1_reset_1; // Not

    // ========= Output Assignments =========
    assign output_led1_load_shift_0_4 = node_76; // LED
    assign output_led2_l1_0_5 = and_87; // LED
    assign output_led3_l3_0_6 = and_85; // LED
    assign output_led4_l2_0_7 = and_86; // LED
    assign output_led5_l0_0_8 = and_88; // LED

endmodule // sequential

// ====================================================================
// Module sequential generation completed successfully
// Elements processed: 38
// Inputs: 3, Outputs: 5
// ====================================================================
