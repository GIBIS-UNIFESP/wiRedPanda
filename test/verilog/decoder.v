// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: decoder
// Generated: Fri Sep 26 00:19:55 2025
// Target FPGA: Generic-Small
// Resource Usage: 66/1000 LUTs, 0/1000 FFs, 12/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module decoder (
    // ========= Input Ports =========
    input wire input_input_switch1_1,
    input wire input_input_switch2_2,
    input wire input_input_switch3_3,

    // ========= Output Ports =========
    output wire output_led1_0_4,
    output wire output_led2_0_5,
    output wire output_led3_0_6,
    output wire output_led4_0_7,
    output wire output_led5_0_8,
    output wire output_led6_0_9,
    output wire output_led7_0_10,
    output wire output_led8_0_11
);

    // ========= Internal Signals =========
    wire node_55_0;
    wire node_54_0;
    wire node_53_0;
    wire not_52_0;
    wire not_51_0;
    wire not_50_0;
    wire node_49_0;
    wire node_48_0;
    wire node_47_0;
    wire node_46_0;
    wire node_45_0;
    wire node_44_0;
    wire node_43_0;
    wire node_42_0;
    wire node_41_0;
    wire node_40_0;
    wire node_39_0;
    wire node_38_0;
    wire node_37_0;
    wire node_36_0;
    wire node_35_0;
    wire node_34_0;
    wire node_33_0;
    wire node_32_0;
    wire node_31_0;
    wire node_30_0;
    wire node_29_0;
    wire node_28_0;
    wire node_27_0;
    wire node_26_0;
    wire and_25_0;
    wire and_24_0;
    wire and_23_0;
    wire and_22_0;
    wire and_21_0;
    wire and_20_0;
    wire and_19_0;
    wire and_18_0;
    wire node_17_0;
    wire node_16_0;
    wire node_15_0;
    wire node_14_0;
    wire node_13_0;
    wire node_12_0;

    // ========= Logic Assignments =========
    assign node_12_0 = ~input_input_switch2_2; // Node
    assign node_13_0 = input_input_switch2_2; // Node
    assign node_14_0 = ~input_input_switch1_1; // Node
    assign node_15_0 = input_input_switch1_1; // Node
    assign node_16_0 = ~input_input_switch3_3; // Node
    assign node_17_0 = input_input_switch3_3; // Node
    assign and_18_0 = (input_input_switch3_3 & input_input_switch1_1 & input_input_switch2_2); // And
    assign and_19_0 = (input_input_switch3_3 & input_input_switch1_1 & ~input_input_switch2_2); // And
    assign and_20_0 = (input_input_switch3_3 & ~input_input_switch1_1 & input_input_switch2_2); // And
    assign and_21_0 = (input_input_switch3_3 & ~input_input_switch1_1 & ~input_input_switch2_2); // And
    assign and_22_0 = (~input_input_switch3_3 & input_input_switch1_1 & input_input_switch2_2); // And
    assign and_23_0 = (~input_input_switch3_3 & input_input_switch1_1 & ~input_input_switch2_2); // And
    assign and_24_0 = (~input_input_switch3_3 & ~input_input_switch1_1 & input_input_switch2_2); // And
    assign and_25_0 = (~input_input_switch2_2 & ~input_input_switch1_1 & ~input_input_switch3_3); // And
    assign node_26_0 = input_input_switch3_3; // Node
    assign node_27_0 = input_input_switch1_1; // Node
    assign node_28_0 = input_input_switch2_2; // Node
    assign node_29_0 = ~input_input_switch2_2; // Node
    assign node_30_0 = ~input_input_switch1_1; // Node
    assign node_31_0 = ~input_input_switch3_3; // Node
    assign node_32_0 = input_input_switch3_3; // Node
    assign node_33_0 = input_input_switch1_1; // Node
    assign node_34_0 = input_input_switch2_2; // Node
    assign node_35_0 = ~input_input_switch2_2; // Node
    assign node_36_0 = ~input_input_switch1_1; // Node
    assign node_37_0 = ~input_input_switch3_3; // Node
    assign node_38_0 = input_input_switch3_3; // Node
    assign node_39_0 = input_input_switch1_1; // Node
    assign node_40_0 = input_input_switch2_2; // Node
    assign node_41_0 = ~input_input_switch2_2; // Node
    assign node_42_0 = ~input_input_switch3_3; // Node
    assign node_43_0 = ~input_input_switch1_1; // Node
    assign node_44_0 = input_input_switch3_3; // Node
    assign node_45_0 = input_input_switch1_1; // Node
    assign node_46_0 = input_input_switch2_2; // Node
    assign node_47_0 = ~input_input_switch2_2; // Node
    assign node_48_0 = ~input_input_switch1_1; // Node
    assign node_49_0 = ~input_input_switch3_3; // Node
    assign not_50_0 = ~input_input_switch2_2; // Not
    assign not_51_0 = ~input_input_switch1_1; // Not
    assign not_52_0 = ~input_input_switch3_3; // Not
    assign node_53_0 = input_input_switch2_2; // Node
    assign node_54_0 = input_input_switch1_1; // Node
    assign node_55_0 = input_input_switch3_3; // Node

    // ========= Output Assignments =========
    assign output_led1_0_4 = (input_input_switch3_3 & input_input_switch1_1 & input_input_switch2_2); // LED
    assign output_led2_0_5 = (input_input_switch3_3 & input_input_switch1_1 & ~input_input_switch2_2); // LED
    assign output_led3_0_6 = (input_input_switch3_3 & ~input_input_switch1_1 & input_input_switch2_2); // LED
    assign output_led4_0_7 = (input_input_switch3_3 & ~input_input_switch1_1 & ~input_input_switch2_2); // LED
    assign output_led5_0_8 = (~input_input_switch3_3 & input_input_switch1_1 & input_input_switch2_2); // LED
    assign output_led6_0_9 = (~input_input_switch3_3 & input_input_switch1_1 & ~input_input_switch2_2); // LED
    assign output_led7_0_10 = (~input_input_switch3_3 & ~input_input_switch1_1 & input_input_switch2_2); // LED
    assign output_led8_0_11 = (~input_input_switch2_2 & ~input_input_switch1_1 & ~input_input_switch3_3); // LED

endmodule // decoder

// ====================================================================
// Module decoder generation completed successfully
// Elements processed: 55
// Inputs: 3, Outputs: 8
// ====================================================================
