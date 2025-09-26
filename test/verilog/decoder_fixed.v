// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: decoder_fixed
// Generated: Fri Sep 26 20:31:05 2025
// Target FPGA: Generic-Small
// Resource Usage: 66/1000 LUTs, 0/1000 FFs, 12/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module decoder_fixed (
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

    // ========= Logic Assignments =========
    assign and_49 = (node_41 & node_40 & node_39); // And
    assign and_48 = (node_35 & node_34 & node_38); // And
    assign and_47 = (node_29 & node_37 & node_33); // And
    assign and_46 = (node_23 & node_31 & node_32); // And
    assign and_45 = (node_36 & node_28 & node_27); // And
    assign and_44 = (node_30 & node_22 & node_26); // And
    assign and_43 = (node_25 & node_24 & node_21); // And
    assign and_42 = (node_20 & node_19 & node_18); // And
    assign node_41 = node_35; // Node
    assign node_40 = node_34; // Node
    assign node_39 = node_33; // Node
    assign node_38 = node_32; // Node
    assign node_37 = node_31; // Node
    assign node_36 = node_30; // Node
    assign node_35 = node_29; // Node
    assign node_34 = node_28; // Node
    assign node_33 = node_27; // Node
    assign node_32 = node_26; // Node
    assign node_31 = node_24; // Node
    assign node_30 = node_25; // Node
    assign node_29 = node_23; // Node
    assign node_28 = node_22; // Node
    assign node_27 = node_21; // Node
    assign node_26 = node_20; // Node
    assign node_25 = node_18; // Node
    assign node_24 = node_19; // Node
    assign node_23 = node_12; // Node
    assign node_22 = node_13; // Node
    assign node_21 = node_14; // Node
    assign node_20 = not_17; // Node
    assign node_19 = not_16; // Node
    assign node_18 = not_15; // Node
    assign not_17 = ~node_14; // Not
    assign not_16 = ~node_13; // Not
    assign not_15 = ~node_12; // Not
    assign node_14 = input_input_switch2_2; // Node
    assign node_13 = input_input_switch1_1; // Node
    assign node_12 = input_input_switch3_3; // Node

    // ========= Output Assignments =========
    assign output_led1_0_4 = and_49; // LED
    assign output_led2_0_5 = and_48; // LED
    assign output_led3_0_6 = and_47; // LED
    assign output_led4_0_7 = and_46; // LED
    assign output_led5_0_8 = and_45; // LED
    assign output_led6_0_9 = and_44; // LED
    assign output_led7_0_10 = and_43; // LED
    assign output_led8_0_11 = and_42; // LED

endmodule // decoder_fixed

// ====================================================================
// Module decoder_fixed generation completed successfully
// Elements processed: 55
// Inputs: 3, Outputs: 8
// ====================================================================
