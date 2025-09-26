// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: sequential
// Generated: Fri Sep 26 00:42:06 2025
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
wire ic_serialize_serialize_ic_in_0_37; // IC input 0
assign ic_serialize_serialize_ic_in_0_37 = jk_flip_flop_33_1_1;
wire ic_serialize_serialize_ic_in_1_38; // IC input 1
assign ic_serialize_serialize_ic_in_1_38 = jk_flip_flop_31_1_1;
wire ic_serialize_serialize_ic_in_2_39; // IC input 2
assign ic_serialize_serialize_ic_in_2_39 = jk_flip_flop_29_1_1;
wire ic_serialize_serialize_ic_in_3_40; // IC input 3
assign ic_serialize_serialize_ic_in_3_40 = jk_flip_flop_27_1_1;
wire ic_serialize_serialize_ic_in_4_41; // IC input 4
assign ic_serialize_serialize_ic_in_4_41 = input_clock2_slow_clk_2;
wire ic_serialize_serialize_ic_in_5_42; // IC input 5
assign ic_serialize_serialize_ic_in_5_42 = input_clock3_fast_clk_3;
wire ic_serialize_serialize_ic_out_0_43; // IC output 0
wire ic_serialize_serialize_ic_out_1_44; // IC output 1
    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
             <= ((1'b1 & 1'b1) | (~1'b1 & 1'b0));
             <= ~((1'b1 & 1'b1) | (~1'b1 & 1'b0));
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
             <= ((1'b1 & 1'b0) | (~1'b1 & 1'b0));
             <= ~((1'b1 & 1'b0) | (~1'b1 & 1'b0));
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
             <= ((1'b1 & 1'b1) | (~1'b1 & 1'b0));
             <= ~((1'b1 & 1'b1) | (~1'b1 & 1'b0));
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
             <= 1'b0;
             <= ~1'b0;
        end
    end

wire ic_register_register_ic_in_0_45; // IC input 0
assign ic_register_register_ic_in_0_45 = ic_serialize_serialize_ic_out_0_43;
wire ic_register_register_ic_in_1_46; // IC input 1
assign ic_register_register_ic_in_1_46 = (~ic_serialize_serialize_ic_out_1_44 & input_clock3_fast_clk_3);
wire ic_register_register_ic_out_0_47; // IC output 0
wire ic_register_register_ic_out_1_48; // IC output 1
wire ic_register_register_ic_out_2_49; // IC output 2
wire ic_register_register_ic_out_3_50; // IC output 3
    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
             <= 1'b0;
             <= ~1'b0;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
             <= 1'b0;
             <= ~1'b0;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
             <= 1'b0;
             <= ~1'b0;
        end
    end

    // D FlipFlop: D-Flip-Flop
    always @(posedge 1'b0) begin
        begin
             <= 1'b0;
             <= ~1'b0;
        end
    end

    wire not_36_0;
    wire node_35_0;
    wire node_34_0;
    reg jk_flip_flop_33_0_0 = 1'b0;
    reg jk_flip_flop_33_1_1 = 1'b0;
    wire node_32_0;
    reg jk_flip_flop_31_0_0 = 1'b0;
    reg jk_flip_flop_31_1_1 = 1'b0;
    wire node_30_0;
    reg jk_flip_flop_29_0_0 = 1'b0;
    reg jk_flip_flop_29_1_1 = 1'b0;
    wire node_28_0;
    reg jk_flip_flop_27_0_0 = 1'b0;
    reg jk_flip_flop_27_1_1 = 1'b0;
    wire node_26_0;
    wire node_25_0;
    wire node_24_0;
    wire node_23_0;
    wire node_22_0;
    wire node_21_0;
    wire node_20_0;
    wire node_19_0;
    wire not_18_0;
    wire node_17_0;
    wire node_16_0;
    wire and_15_0;
    wire node_14_0;
    wire node_13_0;
    wire and_12_0;
    wire and_11_0;
    wire and_10_0;
    wire and_9_0;

    // ========= Logic Assignments =========
    assign and_9_0 = (ic_register_register_ic_out_0_47 & input_clock2_slow_clk_2); // And
    assign and_10_0 = (ic_register_register_ic_out_1_48 & input_clock2_slow_clk_2); // And
    assign and_11_0 = (ic_register_register_ic_out_2_49 & input_clock2_slow_clk_2); // And
    assign and_12_0 = (ic_register_register_ic_out_3_50 & input_clock2_slow_clk_2); // And
    assign node_13_0 = input_clock2_slow_clk_2; // Node
    assign node_14_0 = input_clock2_slow_clk_2; // Node
    assign and_15_0 = (~ic_serialize_serialize_ic_out_1_44 & input_clock3_fast_clk_3); // And
    assign node_16_0 = input_clock2_slow_clk_2; // Node
    assign node_17_0 = input_clock3_fast_clk_3; // Node
    assign not_18_0 = ~ic_serialize_serialize_ic_out_1_44; // Not
    assign node_19_0 = input_clock2_slow_clk_2; // Node
    assign node_20_0 = input_clock3_fast_clk_3; // Node
    assign node_21_0 = input_clock2_slow_clk_2; // Node
    assign node_22_0 = jk_flip_flop_27_1_1; // Node
    assign node_23_0 = jk_flip_flop_29_1_1; // Node
    assign node_24_0 = jk_flip_flop_31_1_1; // Node
    assign node_25_0 = jk_flip_flop_33_1_1; // Node
    assign node_26_0 = input_clock2_slow_clk_2; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_29_0_0 or posedge input_push_button1_reset_1) begin
        if (input_push_button1_reset_1) begin
            jk_flip_flop_27_0_0 <= 1'b1;
            jk_flip_flop_27_1_1 <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_27_0_0 <= 1'b0; jk_flip_flop_27_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_27_0_0 <= 1'b1; jk_flip_flop_27_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_27_0_0 <= jk_flip_flop_27_1_1; jk_flip_flop_27_1_1 <= jk_flip_flop_27_0_0; end // toggle
            endcase
        end
    end

    assign node_28_0 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_31_0_0 or posedge input_push_button1_reset_1) begin
        if (input_push_button1_reset_1) begin
            jk_flip_flop_29_0_0 <= 1'b1;
            jk_flip_flop_29_1_1 <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_29_0_0 <= 1'b0; jk_flip_flop_29_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_29_0_0 <= 1'b1; jk_flip_flop_29_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_29_0_0 <= jk_flip_flop_29_1_1; jk_flip_flop_29_1_1 <= jk_flip_flop_29_0_0; end // toggle
            endcase
        end
    end

    assign node_30_0 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_33_0_0 or posedge input_push_button1_reset_1) begin
        if (input_push_button1_reset_1) begin
            jk_flip_flop_31_0_0 <= 1'b1;
            jk_flip_flop_31_1_1 <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_31_0_0 <= 1'b0; jk_flip_flop_31_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_31_0_0 <= 1'b1; jk_flip_flop_31_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_31_0_0 <= jk_flip_flop_31_1_1; jk_flip_flop_31_1_1 <= jk_flip_flop_31_0_0; end // toggle
            endcase
        end
    end

    assign node_32_0 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock2_slow_clk_2 or posedge input_push_button1_reset_1) begin
        if (input_push_button1_reset_1) begin
            jk_flip_flop_33_0_0 <= 1'b1;
            jk_flip_flop_33_1_1 <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_33_0_0 <= 1'b0; jk_flip_flop_33_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_33_0_0 <= 1'b1; jk_flip_flop_33_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_33_0_0 <= jk_flip_flop_33_1_1; jk_flip_flop_33_1_1 <= jk_flip_flop_33_0_0; end // toggle
            endcase
        end
    end

    assign node_34_0 = ~input_push_button1_reset_1; // Node
    assign node_35_0 = ~input_push_button1_reset_1; // Node
    assign not_36_0 = ~input_push_button1_reset_1; // Not

    // ========= Output Assignments =========
    assign output_led1_load_shift_0_4 = input_clock2_slow_clk_2; // LED
    assign output_led2_l1_0_5 = (ic_register_register_ic_out_1_48 & input_clock2_slow_clk_2); // LED
    assign output_led3_l3_0_6 = (ic_register_register_ic_out_3_50 & input_clock2_slow_clk_2); // LED
    assign output_led4_l2_0_7 = (ic_register_register_ic_out_2_49 & input_clock2_slow_clk_2); // LED
    assign output_led5_l0_0_8 = (ic_register_register_ic_out_0_47 & input_clock2_slow_clk_2); // LED

endmodule // sequential

// ====================================================================
// Module sequential generation completed successfully
// Elements processed: 38
// Inputs: 3, Outputs: 5
// ====================================================================
