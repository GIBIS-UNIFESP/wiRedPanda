// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: decoder
// Generated: Fri Sep 26 14:36:18 2025
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
    wire node_12;
    wire node_13;
    wire node_14;
    wire not_15;
    wire not_16;
    wire not_17;
    wire node_18;
    wire node_19;
    wire node_20;
    wire node_21;
    wire node_22;
    wire node_23;
    wire node_24;
    wire node_25;
    wire node_26;
    wire node_27;
    wire node_28;
    wire node_29;
    wire node_30;
    wire node_31;
    wire node_32;
    wire node_33;
    wire node_34;
    wire node_35;
    wire node_36;
    wire node_37;
    wire node_38;
    wire node_39;
    wire node_40;
    wire node_41;
    wire and_42;
    wire and_43;
    wire and_44;
    wire and_45;
    wire and_46;
    wire and_47;
    wire and_48;
    wire and_49;
    wire node_50;
    wire node_51;
    wire node_52;
    wire node_53;
    wire node_54;
    wire node_55;

    // ========= Logic Assignments =========
    assign node_55 = ~input_input_switch2_2; // Node
    assign node_54 = input_input_switch2_2; // Node
    assign node_53 = ~input_input_switch1_1; // Node
    assign node_52 = input_input_switch1_1; // Node
    assign node_51 = ~input_input_switch3_3; // Node
    assign node_50 = input_input_switch3_3; // Node
    assign and_49 = (input_input_switch3_3 & input_input_switch1_1 & input_input_switch2_2); // And
    assign and_48 = (input_input_switch3_3 & input_input_switch1_1 & ~input_input_switch2_2); // And
    assign and_47 = (input_input_switch3_3 & ~input_input_switch1_1 & input_input_switch2_2); // And
    assign and_46 = (input_input_switch3_3 & ~input_input_switch1_1 & ~input_input_switch2_2); // And
    assign and_45 = (~input_input_switch3_3 & input_input_switch1_1 & input_input_switch2_2); // And
    assign and_44 = (~input_input_switch3_3 & input_input_switch1_1 & ~input_input_switch2_2); // And
    assign and_43 = (~input_input_switch3_3 & ~input_input_switch1_1 & input_input_switch2_2); // And
    assign and_42 = (~input_input_switch2_2 & ~input_input_switch1_1 & ~input_input_switch3_3); // And
    assign node_41 = input_input_switch3_3; // Node
    assign node_40 = input_input_switch1_1; // Node
    assign node_39 = input_input_switch2_2; // Node
    assign node_38 = ~input_input_switch2_2; // Node
    assign node_37 = ~input_input_switch1_1; // Node
    assign node_36 = ~input_input_switch3_3; // Node
    assign node_35 = input_input_switch3_3; // Node
    assign node_34 = input_input_switch1_1; // Node
    assign node_33 = input_input_switch2_2; // Node
    assign node_32 = ~input_input_switch2_2; // Node
    assign node_31 = ~input_input_switch1_1; // Node
    assign node_30 = ~input_input_switch3_3; // Node
    assign node_29 = input_input_switch3_3; // Node
    assign node_28 = input_input_switch1_1; // Node
    assign node_27 = input_input_switch2_2; // Node
    assign node_26 = ~input_input_switch2_2; // Node
    assign node_25 = ~input_input_switch3_3; // Node
    assign node_24 = ~input_input_switch1_1; // Node
    assign node_23 = input_input_switch3_3; // Node
    assign node_22 = input_input_switch1_1; // Node
    assign node_21 = input_input_switch2_2; // Node
    assign node_20 = ~input_input_switch2_2; // Node
    assign node_19 = ~input_input_switch1_1; // Node
    assign node_18 = ~input_input_switch3_3; // Node
    assign not_17 = ~input_input_switch2_2; // Not
    assign not_16 = ~input_input_switch1_1; // Not
    assign not_15 = ~input_input_switch3_3; // Not
    assign node_14 = input_input_switch2_2; // Node
    assign node_13 = input_input_switch1_1; // Node
    assign node_12 = input_input_switch3_3; // Node

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
