// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: decoder
// Generated: Fri Sep 26 21:22:18 2025
// Target FPGA: Generic-Small
// Resource Usage: 66/1000 LUTs, 0/1000 FFs, 12/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module decoder (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2,
    output wire output_led3_0_3,
    output wire output_led4_0_4,
    output wire output_led5_0_5,
    output wire output_led6_0_6,
    output wire output_led7_0_7,
    output wire output_led8_0_8
);

    // ========= Internal Signals =========
    wire node_9;
    wire node_10;
    wire node_11;
    wire not_12;
    wire not_13;
    wire not_14;
    wire node_15;
    wire node_16;
    wire node_17;
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
    wire and_39;
    wire and_40;
    wire and_41;
    wire and_42;
    wire and_43;
    wire and_44;
    wire and_45;
    wire and_46;

    // ========= Logic Assignments =========
    assign and_46 = (node_38 & node_37 & node_36); // And
    assign and_45 = (node_32 & node_31 & node_35); // And
    assign and_44 = (node_26 & node_34 & node_30); // And
    assign and_43 = (node_20 & node_28 & node_29); // And
    assign and_42 = (node_33 & node_25 & node_24); // And
    assign and_41 = (node_27 & node_19 & node_23); // And
    assign and_40 = (node_22 & node_21 & node_18); // And
    assign and_39 = (node_17 & node_16 & node_15); // And
    assign node_38 = node_32; // Node
    assign node_37 = node_31; // Node
    assign node_36 = node_30; // Node
    assign node_35 = node_29; // Node
    assign node_34 = node_28; // Node
    assign node_33 = node_27; // Node
    assign node_32 = node_26; // Node
    assign node_31 = node_25; // Node
    assign node_30 = node_24; // Node
    assign node_29 = node_23; // Node
    assign node_28 = node_21; // Node
    assign node_27 = node_22; // Node
    assign node_26 = node_20; // Node
    assign node_25 = node_19; // Node
    assign node_24 = node_18; // Node
    assign node_23 = node_17; // Node
    assign node_22 = node_15; // Node
    assign node_21 = node_16; // Node
    assign node_20 = node_9; // Node
    assign node_19 = node_10; // Node
    assign node_18 = node_11; // Node
    assign node_17 = not_14; // Node
    assign node_16 = not_13; // Node
    assign node_15 = not_12; // Node
    assign not_14 = ~node_11; // Not
    assign not_13 = ~node_10; // Not
    assign not_12 = ~node_9; // Not
    assign node_11 = 1'b0; // Node
    assign node_10 = 1'b0; // Node
    assign node_9 = 1'b0; // Node

    // ========= Output Assignments =========
    assign output_led1_0_1 = and_46; // LED
    assign output_led2_0_2 = and_45; // LED
    assign output_led3_0_3 = and_44; // LED
    assign output_led4_0_4 = and_43; // LED
    assign output_led5_0_5 = and_42; // LED
    assign output_led6_0_6 = and_41; // LED
    assign output_led7_0_7 = and_40; // LED
    assign output_led8_0_8 = and_39; // LED

endmodule // decoder

// ====================================================================
// Module decoder generation completed successfully
// Elements processed: 55
// Inputs: 0, Outputs: 8
// ====================================================================
