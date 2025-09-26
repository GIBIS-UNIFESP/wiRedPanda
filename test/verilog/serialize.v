// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: serialize
// Generated: Fri Sep 26 15:11:03 2025
// Target FPGA: Generic-Small
// Resource Usage: 45/1000 LUTs, 4/1000 FFs, 8/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module serialize (
    // ========= Input Ports =========
    input wire input_input_switch1_load__shift_1,
    input wire input_input_switch2_clock_2,
    input wire input_input_switch3_d0_3,
    input wire input_input_switch4_d1_4,
    input wire input_input_switch5_d2_5,
    input wire input_input_switch6_d3_6,

    // ========= Output Ports =========
    output wire output_led1_0_7,
    output wire output_led2_0_8
);

    // ========= Internal Signals =========
    wire node_9;
    wire node_10;
    wire node_11;
    wire not_12;
    wire node_13;
    wire node_14;
    reg d_flip_flop_15_0_q = 1'b0;
    reg d_flip_flop_15_1_q = 1'b0;
    wire and_16;
    wire and_17;
    wire node_18;
    wire or_19;
    wire node_20;
    wire node_21;
    reg d_flip_flop_22_0_q = 1'b0;
    reg d_flip_flop_22_1_q = 1'b0;
    wire and_23;
    wire and_24;
    wire node_25;
    wire or_26;
    wire node_27;
    wire node_28;
    reg d_flip_flop_29_0_q = 1'b0;
    reg d_flip_flop_29_1_q = 1'b0;
    wire and_30;
    wire and_31;
    wire node_32;
    wire or_33;
    wire node_34;
    wire node_35;
    reg d_flip_flop_36_0_q = 1'b0;
    reg d_flip_flop_36_1_q = 1'b0;
    wire node_37;
    wire and_38;

    // ========= Logic Assignments =========
    assign and_38 = (node_34 & d_flip_flop_36_0_q); // And
    assign node_37 = node_35; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_32) begin
        begin
            d_flip_flop_36_0_q <= or_33;
            d_flip_flop_36_1_q <= ~or_33;
        end
    end

    assign node_34 = not_12; // Node
    assign or_33 = (and_31 | and_30); // Or
    assign node_32 = input_input_switch2_clock_2; // Node
    assign and_31 = (node_27 & input_input_switch6_d3_6); // And
    assign and_30 = (node_28 & d_flip_flop_29_0_q); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_25) begin
        begin
            d_flip_flop_29_0_q <= or_26;
            d_flip_flop_29_1_q <= ~or_26;
        end
    end

    assign node_28 = not_12; // Node
    assign node_27 = node_11; // Node
    assign or_26 = (and_23 | and_24); // Or
    assign node_25 = input_input_switch2_clock_2; // Node
    assign and_24 = (node_21 & d_flip_flop_22_0_q); // And
    assign and_23 = (node_20 & input_input_switch5_d2_5); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_18) begin
        begin
            d_flip_flop_22_0_q <= or_19;
            d_flip_flop_22_1_q <= ~or_19;
        end
    end

    assign node_21 = not_12; // Node
    assign node_20 = node_11; // Node
    assign or_19 = (and_17 | and_16); // Or
    assign node_18 = input_input_switch2_clock_2; // Node
    assign and_17 = (node_13 & input_input_switch4_d1_4); // And
    assign and_16 = (node_14 & d_flip_flop_15_0_q); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge node_10) begin
        begin
            d_flip_flop_15_0_q <= input_input_switch3_d0_3;
            d_flip_flop_15_1_q <= ~input_input_switch3_d0_3;
        end
    end

    assign node_14 = not_12; // Node
    assign node_13 = node_11; // Node
    assign not_12 = ~node_9; // Not
    assign node_10 = input_input_switch2_clock_2; // Node
    assign node_9 = input_input_switch1_load__shift_1; // Node

    // ========= Output Assignments =========
    assign output_led1_0_7 = node_37; // LED
    assign output_led2_0_8 = and_38; // LED

endmodule // serialize

// ====================================================================
// Module serialize generation completed successfully
// Elements processed: 38
// Inputs: 6, Outputs: 2
// ====================================================================
