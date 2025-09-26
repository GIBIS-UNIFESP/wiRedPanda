// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: sequential_fixed
// Generated: Fri Sep 26 01:35:43 2025
// Target FPGA: Generic-Small
// Resource Usage: 45/1000 LUTs, 74/1000 FFs, 8/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module sequential_fixed (
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
wire ic_serialize_serialize_ic_in_0_45; // IC input 0
assign ic_serialize_serialize_ic_in_0_45 = jk_flip_flop_41_1_1;
wire ic_serialize_serialize_ic_in_1_46; // IC input 1
assign ic_serialize_serialize_ic_in_1_46 = jk_flip_flop_39_1_1;
wire ic_serialize_serialize_ic_in_2_47; // IC input 2
assign ic_serialize_serialize_ic_in_2_47 = jk_flip_flop_37_1_1;
wire ic_serialize_serialize_ic_in_3_48; // IC input 3
assign ic_serialize_serialize_ic_in_3_48 = jk_flip_flop_35_1_1;
wire ic_serialize_serialize_ic_in_4_49; // IC input 4
assign ic_serialize_serialize_ic_in_4_49 = input_clock2_slow_clk_2;
wire ic_serialize_serialize_ic_in_5_50; // IC input 5
assign ic_serialize_serialize_ic_in_5_50 = input_clock3_fast_clk_3;
wire ic_serialize_serialize_ic_out_0_51; // IC output 0
wire ic_serialize_serialize_ic_out_1_52; // IC output 1
    reg ic_serialize_d_flip_flop_24_0_0 = 1'b0;
    reg ic_serialize_d_flip_flop_24_1_1 = 1'b0;
    reg ic_serialize_d_flip_flop_25_0_0 = 1'b0;
    reg ic_serialize_d_flip_flop_25_1_1 = 1'b0;
    reg ic_serialize_d_flip_flop_26_0_0 = 1'b0;
    reg ic_serialize_d_flip_flop_26_1_1 = 1'b0;
    reg ic_serialize_d_flip_flop_27_0_0 = 1'b0;
    reg ic_serialize_d_flip_flop_27_1_1 = 1'b0;
    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
            ic_serialize_d_flip_flop_24_0_0 <= ((1'b1 & 1'b1) | (~1'b1 & ic_serialize_d_flip_flop_25_0_0));
            ic_serialize_d_flip_flop_24_1_1 <= ~((1'b1 & 1'b1) | (~1'b1 & ic_serialize_d_flip_flop_25_0_0));
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
            ic_serialize_d_flip_flop_25_0_0 <= ((1'b1 & 1'b0) | (~1'b1 & ic_serialize_d_flip_flop_26_0_0));
            ic_serialize_d_flip_flop_25_1_1 <= ~((1'b1 & 1'b0) | (~1'b1 & ic_serialize_d_flip_flop_26_0_0));
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
            ic_serialize_d_flip_flop_26_0_0 <= ((1'b1 & 1'b1) | (~1'b1 & ic_serialize_d_flip_flop_27_0_0));
            ic_serialize_d_flip_flop_26_1_1 <= ~((1'b1 & 1'b1) | (~1'b1 & ic_serialize_d_flip_flop_27_0_0));
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
            ic_serialize_d_flip_flop_27_0_0 <= 1'b0;
            ic_serialize_d_flip_flop_27_1_1 <= ~1'b0;
        end
    end

wire ic_register_register_ic_in_0_53; // IC input 0
assign ic_register_register_ic_in_0_53 = ic_serialize_serialize_ic_out_0_51;
wire ic_register_register_ic_in_1_54; // IC input 1
assign ic_register_register_ic_in_1_54 = (~ic_serialize_serialize_ic_out_1_52 & input_clock3_fast_clk_3);
wire ic_register_register_ic_out_0_55; // IC output 0
wire ic_register_register_ic_out_1_56; // IC output 1
wire ic_register_register_ic_out_2_57; // IC output 2
wire ic_register_register_ic_out_3_58; // IC output 3
    reg ic_register_d_flip_flop_14_0_0 = 1'b0;
    reg ic_register_d_flip_flop_14_1_1 = 1'b0;
    reg ic_register_d_flip_flop_15_0_0 = 1'b0;
    reg ic_register_d_flip_flop_15_1_1 = 1'b0;
    reg ic_register_d_flip_flop_16_0_0 = 1'b0;
    reg ic_register_d_flip_flop_16_1_1 = 1'b0;
    reg ic_register_d_flip_flop_17_0_0 = 1'b0;
    reg ic_register_d_flip_flop_17_1_1 = 1'b0;
    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
            ic_register_d_flip_flop_14_0_0 <= ic_register_d_flip_flop_16_0_0;
            ic_register_d_flip_flop_14_1_1 <= ~ic_register_d_flip_flop_16_0_0;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
            ic_register_d_flip_flop_15_0_0 <= ic_register_d_flip_flop_14_0_0;
            ic_register_d_flip_flop_15_1_1 <= ~ic_register_d_flip_flop_14_0_0;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
            ic_register_d_flip_flop_16_0_0 <= ic_register_d_flip_flop_17_0_0;
            ic_register_d_flip_flop_16_1_1 <= ~ic_register_d_flip_flop_17_0_0;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
            ic_register_d_flip_flop_17_0_0 <= 1'b0;
            ic_register_d_flip_flop_17_1_1 <= ~1'b0;
        end
    end

    wire not_44_0;
    wire node_43_0;
    wire node_42_0;
    reg jk_flip_flop_41_0_0 = 1'b0;
    reg jk_flip_flop_41_1_1 = 1'b0;
    wire node_40_0;
    reg jk_flip_flop_39_0_0 = 1'b0;
    reg jk_flip_flop_39_1_1 = 1'b0;
    wire node_38_0;
    reg jk_flip_flop_37_0_0 = 1'b0;
    reg jk_flip_flop_37_1_1 = 1'b0;
    wire node_36_0;
    reg jk_flip_flop_35_0_0 = 1'b0;
    reg jk_flip_flop_35_1_1 = 1'b0;
    wire node_34_0;
    wire node_33_0;
    wire node_32_0;
    wire node_31_0;
    wire node_30_0;
    wire node_29_0;
    wire node_28_0;
    reg ic_serialize_d_flip_flop_24_0_0 = 1'b0; // IC internal sequential
    reg ic_serialize_d_flip_flop_24_1_1 = 1'b0; // IC internal sequential
    reg ic_serialize_d_flip_flop_25_0_0 = 1'b0; // IC internal sequential
    reg ic_serialize_d_flip_flop_25_1_1 = 1'b0; // IC internal sequential
    reg ic_serialize_d_flip_flop_26_0_0 = 1'b0; // IC internal sequential
    reg ic_serialize_d_flip_flop_26_1_1 = 1'b0; // IC internal sequential
    reg ic_serialize_d_flip_flop_27_0_0 = 1'b0; // IC internal sequential
    reg ic_serialize_d_flip_flop_27_1_1 = 1'b0; // IC internal sequential
    wire node_23_0;
    wire not_22_0;
    wire node_21_0;
    wire node_20_0;
    wire and_19_0;
    wire node_18_0;
    reg ic_register_d_flip_flop_14_0_0 = 1'b0; // IC internal sequential
    reg ic_register_d_flip_flop_14_1_1 = 1'b0; // IC internal sequential
    reg ic_register_d_flip_flop_15_0_0 = 1'b0; // IC internal sequential
    reg ic_register_d_flip_flop_15_1_1 = 1'b0; // IC internal sequential
    reg ic_register_d_flip_flop_16_0_0 = 1'b0; // IC internal sequential
    reg ic_register_d_flip_flop_16_1_1 = 1'b0; // IC internal sequential
    reg ic_register_d_flip_flop_17_0_0 = 1'b0; // IC internal sequential
    reg ic_register_d_flip_flop_17_1_1 = 1'b0; // IC internal sequential
    wire node_13_0;
    wire and_12_0;
    wire and_11_0;
    wire and_10_0;
    wire and_9_0;

    // ========= Logic Assignments =========
    assign and_9_0 = (ic_register_register_ic_out_0_55 & input_clock2_slow_clk_2); // And
    assign and_10_0 = (ic_register_register_ic_out_1_56 & input_clock2_slow_clk_2); // And
    assign and_11_0 = (ic_register_register_ic_out_2_57 & input_clock2_slow_clk_2); // And
    assign and_12_0 = (ic_register_register_ic_out_3_58 & input_clock2_slow_clk_2); // And
    assign node_13_0 = input_clock2_slow_clk_2; // Node
    assign node_18_0 = input_clock2_slow_clk_2; // Node
    assign and_19_0 = (~ic_serialize_serialize_ic_out_1_52 & input_clock3_fast_clk_3); // And
    assign node_20_0 = input_clock2_slow_clk_2; // Node
    assign node_21_0 = input_clock3_fast_clk_3; // Node
    assign not_22_0 = ~ic_serialize_serialize_ic_out_1_52; // Not
    assign node_23_0 = input_clock2_slow_clk_2; // Node
    assign node_28_0 = input_clock3_fast_clk_3; // Node
    assign node_29_0 = input_clock2_slow_clk_2; // Node
    assign node_30_0 = jk_flip_flop_35_1_1; // Node
    assign node_31_0 = jk_flip_flop_37_1_1; // Node
    assign node_32_0 = jk_flip_flop_39_1_1; // Node
    assign node_33_0 = jk_flip_flop_41_1_1; // Node
    assign node_34_0 = input_clock2_slow_clk_2; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_37_0_0 or posedge input_push_button1_reset_1) begin
        if (input_push_button1_reset_1) begin
            jk_flip_flop_35_0_0 <= 1'b1;
            jk_flip_flop_35_1_1 <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_35_0_0 <= 1'b0; jk_flip_flop_35_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_35_0_0 <= 1'b1; jk_flip_flop_35_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_35_0_0 <= jk_flip_flop_35_1_1; jk_flip_flop_35_1_1 <= jk_flip_flop_35_0_0; end // toggle
            endcase
        end
    end

    assign node_36_0 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_39_0_0 or posedge input_push_button1_reset_1) begin
        if (input_push_button1_reset_1) begin
            jk_flip_flop_37_0_0 <= 1'b1;
            jk_flip_flop_37_1_1 <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_37_0_0 <= 1'b0; jk_flip_flop_37_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_37_0_0 <= 1'b1; jk_flip_flop_37_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_37_0_0 <= jk_flip_flop_37_1_1; jk_flip_flop_37_1_1 <= jk_flip_flop_37_0_0; end // toggle
            endcase
        end
    end

    assign node_38_0 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_41_0_0 or posedge input_push_button1_reset_1) begin
        if (input_push_button1_reset_1) begin
            jk_flip_flop_39_0_0 <= 1'b1;
            jk_flip_flop_39_1_1 <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_39_0_0 <= 1'b0; jk_flip_flop_39_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_39_0_0 <= 1'b1; jk_flip_flop_39_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_39_0_0 <= jk_flip_flop_39_1_1; jk_flip_flop_39_1_1 <= jk_flip_flop_39_0_0; end // toggle
            endcase
        end
    end

    assign node_40_0 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock2_slow_clk_2 or posedge input_push_button1_reset_1) begin
        if (input_push_button1_reset_1) begin
            jk_flip_flop_41_0_0 <= 1'b1;
            jk_flip_flop_41_1_1 <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_41_0_0 <= 1'b0; jk_flip_flop_41_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_41_0_0 <= 1'b1; jk_flip_flop_41_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_41_0_0 <= jk_flip_flop_41_1_1; jk_flip_flop_41_1_1 <= jk_flip_flop_41_0_0; end // toggle
            endcase
        end
    end

    assign node_42_0 = ~input_push_button1_reset_1; // Node
    assign node_43_0 = ~input_push_button1_reset_1; // Node
    assign not_44_0 = ~input_push_button1_reset_1; // Not

    // ========= Output Assignments =========
    assign output_led1_load_shift_0_4 = input_clock2_slow_clk_2; // LED
    assign output_led2_l1_0_5 = (ic_register_register_ic_out_1_56 & input_clock2_slow_clk_2); // LED
    assign output_led3_l3_0_6 = (ic_register_register_ic_out_3_58 & input_clock2_slow_clk_2); // LED
    assign output_led4_l2_0_7 = (ic_register_register_ic_out_2_57 & input_clock2_slow_clk_2); // LED
    assign output_led5_l0_0_8 = (ic_register_register_ic_out_0_55 & input_clock2_slow_clk_2); // LED

endmodule // sequential_fixed

// ====================================================================
// Module sequential_fixed generation completed successfully
// Elements processed: 38
// Inputs: 3, Outputs: 5
// ====================================================================
