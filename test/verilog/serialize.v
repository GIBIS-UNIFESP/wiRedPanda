// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: serialize
// Generated: Fri Sep 26 00:42:07 2025
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
    wire node_38_0;
    wire node_37_0;
    wire node_36_0;
    wire not_35_0;
    wire node_34_0;
    wire node_33_0;
    reg d_flip_flop_32_0_0 = 1'b0;
    reg d_flip_flop_32_1_1 = 1'b0;
    wire and_31_0;
    wire and_30_0;
    wire node_29_0;
    wire or_28_0;
    wire node_27_0;
    wire node_26_0;
    reg d_flip_flop_25_0_0 = 1'b0;
    reg d_flip_flop_25_1_1 = 1'b0;
    wire and_24_0;
    wire and_23_0;
    wire node_22_0;
    wire or_21_0;
    wire node_20_0;
    wire node_19_0;
    reg d_flip_flop_18_0_0 = 1'b0;
    reg d_flip_flop_18_1_1 = 1'b0;
    wire and_17_0;
    wire and_16_0;
    wire node_15_0;
    wire or_14_0;
    wire node_13_0;
    wire node_12_0;
    reg d_flip_flop_11_0_0 = 1'b0;
    reg d_flip_flop_11_1_1 = 1'b0;
    wire node_10_0;
    wire and_9_0;

    // ========= Logic Assignments =========
    assign and_9_0 = (~input_input_switch1_load__shift_1 & d_flip_flop_11_0_0); // And
    assign node_10_0 = input_input_switch1_load__shift_1; // Node
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_input_switch2_clock_2) begin
        begin
            d_flip_flop_11_0_0 <= ((input_input_switch1_load__shift_1 & input_input_switch6_d3_6) | (~input_input_switch1_load__shift_1 & d_flip_flop_18_0_0));
            d_flip_flop_11_1_1 <= ~((input_input_switch1_load__shift_1 & input_input_switch6_d3_6) | (~input_input_switch1_load__shift_1 & d_flip_flop_18_0_0));
        end
    end

    assign node_12_0 = input_input_switch1_load__shift_1; // Node
    assign node_13_0 = ~input_input_switch1_load__shift_1; // Node
    assign or_14_0 = ((input_input_switch1_load__shift_1 & input_input_switch6_d3_6) | (~input_input_switch1_load__shift_1 & d_flip_flop_18_0_0)); // Or
    assign node_15_0 = input_input_switch2_clock_2; // Node
    assign and_16_0 = (input_input_switch1_load__shift_1 & input_input_switch6_d3_6); // And
    assign and_17_0 = (~input_input_switch1_load__shift_1 & d_flip_flop_18_0_0); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_input_switch2_clock_2) begin
        begin
            d_flip_flop_18_0_0 <= ((input_input_switch1_load__shift_1 & input_input_switch5_d2_5) | (~input_input_switch1_load__shift_1 & d_flip_flop_25_0_0));
            d_flip_flop_18_1_1 <= ~((input_input_switch1_load__shift_1 & input_input_switch5_d2_5) | (~input_input_switch1_load__shift_1 & d_flip_flop_25_0_0));
        end
    end

    assign node_19_0 = ~input_input_switch1_load__shift_1; // Node
    assign node_20_0 = input_input_switch1_load__shift_1; // Node
    assign or_21_0 = ((input_input_switch1_load__shift_1 & input_input_switch5_d2_5) | (~input_input_switch1_load__shift_1 & d_flip_flop_25_0_0)); // Or
    assign node_22_0 = input_input_switch2_clock_2; // Node
    assign and_23_0 = (~input_input_switch1_load__shift_1 & d_flip_flop_25_0_0); // And
    assign and_24_0 = (input_input_switch1_load__shift_1 & input_input_switch5_d2_5); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_input_switch2_clock_2) begin
        begin
            d_flip_flop_25_0_0 <= ((input_input_switch1_load__shift_1 & input_input_switch4_d1_4) | (~input_input_switch1_load__shift_1 & d_flip_flop_32_0_0));
            d_flip_flop_25_1_1 <= ~((input_input_switch1_load__shift_1 & input_input_switch4_d1_4) | (~input_input_switch1_load__shift_1 & d_flip_flop_32_0_0));
        end
    end

    assign node_26_0 = ~input_input_switch1_load__shift_1; // Node
    assign node_27_0 = input_input_switch1_load__shift_1; // Node
    assign or_28_0 = ((input_input_switch1_load__shift_1 & input_input_switch4_d1_4) | (~input_input_switch1_load__shift_1 & d_flip_flop_32_0_0)); // Or
    assign node_29_0 = input_input_switch2_clock_2; // Node
    assign and_30_0 = (input_input_switch1_load__shift_1 & input_input_switch4_d1_4); // And
    assign and_31_0 = (~input_input_switch1_load__shift_1 & d_flip_flop_32_0_0); // And
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_input_switch2_clock_2) begin
        begin
            d_flip_flop_32_0_0 <= input_input_switch3_d0_3;
            d_flip_flop_32_1_1 <= ~input_input_switch3_d0_3;
        end
    end

    assign node_33_0 = ~input_input_switch1_load__shift_1; // Node
    assign node_34_0 = input_input_switch1_load__shift_1; // Node
    assign not_35_0 = ~input_input_switch1_load__shift_1; // Not
    assign node_36_0 = input_input_switch1_load__shift_1; // Node
    assign node_37_0 = input_input_switch2_clock_2; // Node
    assign node_38_0 = input_input_switch1_load__shift_1; // Node

    // ========= Output Assignments =========
    assign output_led1_0_7 = input_input_switch1_load__shift_1; // LED
    assign output_led2_0_8 = (~input_input_switch1_load__shift_1 & d_flip_flop_11_0_0); // LED

endmodule // serialize

// ====================================================================
// Module serialize generation completed successfully
// Elements processed: 38
// Inputs: 6, Outputs: 2
// ====================================================================
