// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: serialize
// Generated: Thu Sep 25 21:23:13 2025
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
    assign and_38 = (~input_input_switch1_load__shift_1 & d_flip_flop_36_0_q); // And
    assign node_37 = input_input_switch1_load__shift_1; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_input_switch2_clock_2) begin
        begin
            d_flip_flop_36_0_q <= ((input_input_switch1_load__shift_1 & input_input_switch6_d3_6) | (~input_input_switch1_load__shift_1 & d_flip_flop_29_0_q));
            d_flip_flop_36_1_q <= ~((input_input_switch1_load__shift_1 & input_input_switch6_d3_6) | (~input_input_switch1_load__shift_1 & d_flip_flop_29_0_q));
        end
    end

    assign node_35 = input_input_switch1_load__shift_1; // Node
    assign node_34 = ~input_input_switch1_load__shift_1; // Node
    assign or_33 = ((input_input_switch1_load__shift_1 & input_input_switch6_d3_6) | (~input_input_switch1_load__shift_1 & d_flip_flop_29_0_q)); // Or
    assign node_32 = input_input_switch2_clock_2; // Node
    assign and_31 = (input_input_switch1_load__shift_1 & input_input_switch6_d3_6); // And
    assign and_30 = (~input_input_switch1_load__shift_1 & d_flip_flop_29_0_q); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_input_switch2_clock_2) begin
        begin
            d_flip_flop_29_0_q <= ((input_input_switch1_load__shift_1 & input_input_switch5_d2_5) | (~input_input_switch1_load__shift_1 & d_flip_flop_22_0_q));
            d_flip_flop_29_1_q <= ~((input_input_switch1_load__shift_1 & input_input_switch5_d2_5) | (~input_input_switch1_load__shift_1 & d_flip_flop_22_0_q));
        end
    end

    assign node_28 = ~input_input_switch1_load__shift_1; // Node
    assign node_27 = input_input_switch1_load__shift_1; // Node
    assign or_26 = ((input_input_switch1_load__shift_1 & input_input_switch5_d2_5) | (~input_input_switch1_load__shift_1 & d_flip_flop_22_0_q)); // Or
    assign node_25 = input_input_switch2_clock_2; // Node
    assign and_24 = (~input_input_switch1_load__shift_1 & d_flip_flop_22_0_q); // And
    assign and_23 = (input_input_switch1_load__shift_1 & input_input_switch5_d2_5); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_input_switch2_clock_2) begin
        begin
            d_flip_flop_22_0_q <= ((input_input_switch1_load__shift_1 & input_input_switch4_d1_4) | (~input_input_switch1_load__shift_1 & d_flip_flop_15_0_q));
            d_flip_flop_22_1_q <= ~((input_input_switch1_load__shift_1 & input_input_switch4_d1_4) | (~input_input_switch1_load__shift_1 & d_flip_flop_15_0_q));
        end
    end

    assign node_21 = ~input_input_switch1_load__shift_1; // Node
    assign node_20 = input_input_switch1_load__shift_1; // Node
    assign or_19 = ((input_input_switch1_load__shift_1 & input_input_switch4_d1_4) | (~input_input_switch1_load__shift_1 & d_flip_flop_15_0_q)); // Or
    assign node_18 = input_input_switch2_clock_2; // Node
    assign and_17 = (input_input_switch1_load__shift_1 & input_input_switch4_d1_4); // And
    assign and_16 = (~input_input_switch1_load__shift_1 & d_flip_flop_15_0_q); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_input_switch2_clock_2) begin
        begin
            d_flip_flop_15_0_q <= input_input_switch3_d0_3;
            d_flip_flop_15_1_q <= ~input_input_switch3_d0_3;
        end
    end

    assign node_14 = ~input_input_switch1_load__shift_1; // Node
    assign node_13 = input_input_switch1_load__shift_1; // Node
    assign not_12 = ~input_input_switch1_load__shift_1; // Not
    assign node_11 = input_input_switch1_load__shift_1; // Node
    assign node_10 = input_input_switch2_clock_2; // Node
    assign node_9 = input_input_switch1_load__shift_1; // Node

    // ========= Output Assignments =========
    assign output_led1_0_7 = input_input_switch1_load__shift_1; // LED
    assign output_led2_0_8 = (~input_input_switch1_load__shift_1 & d_flip_flop_36_0_q); // LED

endmodule // serialize

// ====================================================================
// Module serialize generation completed successfully
// Elements processed: 38
// Inputs: 6, Outputs: 2
// ====================================================================
