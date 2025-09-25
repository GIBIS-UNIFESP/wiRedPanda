// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: sequential
// Generated: Thu Sep 25 21:23:12 2025
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
wire ic_serialize_serialize_ic_in_0_9; // IC input 0
assign ic_serialize_serialize_ic_in_0_9 = 1'b1;
wire ic_serialize_serialize_ic_in_1_10; // IC input 1
assign ic_serialize_serialize_ic_in_1_10 = 1'b1;
wire ic_serialize_serialize_ic_in_2_11; // IC input 2
assign ic_serialize_serialize_ic_in_2_11 = 1'b1;
wire ic_serialize_serialize_ic_in_3_12; // IC input 3
assign ic_serialize_serialize_ic_in_3_12 = 1'b1;
wire ic_serialize_serialize_ic_in_4_13; // IC input 4
assign ic_serialize_serialize_ic_in_4_13 = input_clock2_slow_clk_2;
wire ic_serialize_serialize_ic_in_5_14; // IC input 5
assign ic_serialize_serialize_ic_in_5_14 = input_clock3_fast_clk_3;
wire ic_serialize_serialize_ic_out_0_15; // IC output 0
wire ic_serialize_serialize_ic_out_1_16; // IC output 1
wire ic_register_register_ic_in_0_17; // IC input 0
assign ic_register_register_ic_in_0_17 = ic_serialize_serialize_ic_out_0_15;
wire ic_register_register_ic_in_1_18; // IC input 1
assign ic_register_register_ic_in_1_18 = (~ic_serialize_serialize_ic_out_1_16 & input_clock3_fast_clk_3);
wire ic_register_register_ic_out_0_19; // IC output 0
wire ic_register_register_ic_out_1_20; // IC output 1
wire ic_register_register_ic_out_2_21; // IC output 2
wire ic_register_register_ic_out_3_22; // IC output 3
    wire not_23;
    wire node_24;
    wire node_25;
    reg jk_flip_flop_26_0_q = 1'b0;
    reg jk_flip_flop_26_1_q = 1'b0;
    wire node_27;
    reg jk_flip_flop_28_0_q = 1'b0;
    reg jk_flip_flop_28_1_q = 1'b0;
    wire node_29;
    reg jk_flip_flop_30_0_q = 1'b0;
    reg jk_flip_flop_30_1_q = 1'b0;
    wire node_31;
    reg jk_flip_flop_32_0_q = 1'b0;
    reg jk_flip_flop_32_1_q = 1'b0;
    wire node_33;
    wire node_34;
    wire node_35;
    wire node_36;
    wire node_37;
    wire node_38;
    wire node_39;
    wire node_40;
    wire not_41;
    wire node_42;
    wire node_43;
    wire and_44;
    wire node_45;
    wire node_46;
    wire and_47;
    wire and_48;
    wire and_49;
    wire and_50;

    // ========= Logic Assignments =========
    assign and_50 = (ic_register_register_ic_out_0_19 & input_clock2_slow_clk_2); // And
    assign and_49 = (ic_register_register_ic_out_1_20 & input_clock2_slow_clk_2); // And
    assign and_48 = (ic_register_register_ic_out_2_21 & input_clock2_slow_clk_2); // And
    assign and_47 = (ic_register_register_ic_out_3_22 & input_clock2_slow_clk_2); // And
    assign node_46 = input_clock2_slow_clk_2; // Node
    assign node_45 = input_clock2_slow_clk_2; // Node
    assign and_44 = (~ic_serialize_serialize_ic_out_1_16 & input_clock3_fast_clk_3); // And
    assign node_43 = input_clock2_slow_clk_2; // Node
    assign node_42 = input_clock3_fast_clk_3; // Node
    assign not_41 = ~ic_serialize_serialize_ic_out_1_16; // Not
    assign node_40 = input_clock2_slow_clk_2; // Node
    assign node_39 = input_clock3_fast_clk_3; // Node
    assign node_38 = input_clock2_slow_clk_2; // Node
    assign node_37 = jk_flip_flop_32_1_q; // Node
    assign node_36 = jk_flip_flop_30_1_q; // Node
    assign node_35 = jk_flip_flop_28_1_q; // Node
    assign node_34 = jk_flip_flop_26_1_q; // Node
    assign node_33 = input_clock2_slow_clk_2; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_30_0_q or negedge ~input_push_button1_reset_1) begin
        if (!~input_push_button1_reset_1) begin
            jk_flip_flop_32_0_q <= 1'b1;
            jk_flip_flop_32_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_32_0_q <= 1'b0; jk_flip_flop_32_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_32_0_q <= 1'b1; jk_flip_flop_32_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_32_0_q <= jk_flip_flop_32_1_q; jk_flip_flop_32_1_q <= jk_flip_flop_32_0_q; end // toggle
            endcase
        end
    end

    assign node_31 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_28_0_q or negedge ~input_push_button1_reset_1) begin
        if (!~input_push_button1_reset_1) begin
            jk_flip_flop_30_0_q <= 1'b1;
            jk_flip_flop_30_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_30_0_q <= 1'b0; jk_flip_flop_30_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_30_0_q <= 1'b1; jk_flip_flop_30_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_30_0_q <= jk_flip_flop_30_1_q; jk_flip_flop_30_1_q <= jk_flip_flop_30_0_q; end // toggle
            endcase
        end
    end

    assign node_29 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_26_0_q or negedge ~input_push_button1_reset_1) begin
        if (!~input_push_button1_reset_1) begin
            jk_flip_flop_28_0_q <= 1'b1;
            jk_flip_flop_28_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_28_0_q <= 1'b0; jk_flip_flop_28_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_28_0_q <= 1'b1; jk_flip_flop_28_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_28_0_q <= jk_flip_flop_28_1_q; jk_flip_flop_28_1_q <= jk_flip_flop_28_0_q; end // toggle
            endcase
        end
    end

    assign node_27 = ~input_push_button1_reset_1; // Node
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock2_slow_clk_2 or negedge ~input_push_button1_reset_1) begin
        if (!~input_push_button1_reset_1) begin
            jk_flip_flop_26_0_q <= 1'b1;
            jk_flip_flop_26_1_q <= 1'b0;
        end else begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_26_0_q <= 1'b0; jk_flip_flop_26_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_26_0_q <= 1'b1; jk_flip_flop_26_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_26_0_q <= jk_flip_flop_26_1_q; jk_flip_flop_26_1_q <= jk_flip_flop_26_0_q; end // toggle
            endcase
        end
    end

    assign node_25 = ~input_push_button1_reset_1; // Node
    assign node_24 = ~input_push_button1_reset_1; // Node
    assign not_23 = ~input_push_button1_reset_1; // Not

    // ========= Output Assignments =========
    assign output_led1_load_shift_0_4 = input_clock2_slow_clk_2; // LED
    assign output_led2_l1_0_5 = (ic_register_register_ic_out_1_20 & input_clock2_slow_clk_2); // LED
    assign output_led3_l3_0_6 = (ic_register_register_ic_out_3_22 & input_clock2_slow_clk_2); // LED
    assign output_led4_l2_0_7 = (ic_register_register_ic_out_2_21 & input_clock2_slow_clk_2); // LED
    assign output_led5_l0_0_8 = (ic_register_register_ic_out_0_19 & input_clock2_slow_clk_2); // LED

endmodule // sequential

// ====================================================================
// Module sequential generation completed successfully
// Elements processed: 38
// Inputs: 3, Outputs: 5
// ====================================================================
