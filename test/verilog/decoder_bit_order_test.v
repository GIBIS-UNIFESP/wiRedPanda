// Test to understand the bit order issue
`timescale 1ns/1ps

module decoder_bit_order_test (
    // ========= Input Ports =========
    input wire input_input_switch1_1,  // A (bit 0)
    input wire input_input_switch2_2,  // B (bit 1)
    input wire input_input_switch3_3,  // C (bit 2)

    // ========= Output Ports =========
    output wire output_led1_0_4,   // Should activate for input 000
    output wire output_led2_0_5,   // Should activate for input 001
    output wire output_led3_0_6,   // Should activate for input 010
    output wire output_led4_0_7,   // Should activate for input 011
    output wire output_led5_0_8,   // Should activate for input 100
    output wire output_led6_0_9,   // Should activate for input 101
    output wire output_led7_0_10,  // Should activate for input 110
    output wire output_led8_0_11   // Should activate for input 111
);

    // Test both orders to see which is correct
    // Option 1: CBA order (current generated)
    wire opt1_led1 = (~input_input_switch3_3 & ~input_input_switch1_1 & ~input_input_switch2_2); // 000
    wire opt1_led2 = (~input_input_switch3_3 & ~input_input_switch1_1 &  input_input_switch2_2); // 010
    wire opt1_led3 = (~input_input_switch3_3 &  input_input_switch1_1 & ~input_input_switch2_2); // 001

    // Option 2: CBA order corrected (swap A and B in expressions)
    wire opt2_led1 = (~input_input_switch3_3 & ~input_input_switch2_2 & ~input_input_switch1_1); // 000
    wire opt2_led2 = (~input_input_switch3_3 & ~input_input_switch2_2 &  input_input_switch1_1); // 001
    wire opt2_led3 = (~input_input_switch3_3 &  input_input_switch2_2 & ~input_input_switch1_1); // 010

    // Use Option 2 (corrected order)
    assign output_led1_0_4 = (~input_input_switch3_3 & ~input_input_switch2_2 & ~input_input_switch1_1); // 000
    assign output_led2_0_5 = (~input_input_switch3_3 & ~input_input_switch2_2 &  input_input_switch1_1); // 001
    assign output_led3_0_6 = (~input_input_switch3_3 &  input_input_switch2_2 & ~input_input_switch1_1); // 010
    assign output_led4_0_7 = (~input_input_switch3_3 &  input_input_switch2_2 &  input_input_switch1_1); // 011
    assign output_led5_0_8 = ( input_input_switch3_3 & ~input_input_switch2_2 & ~input_input_switch1_1); // 100
    assign output_led6_0_9 = ( input_input_switch3_3 & ~input_input_switch2_2 &  input_input_switch1_1); // 101
    assign output_led7_0_10= ( input_input_switch3_3 &  input_input_switch2_2 & ~input_input_switch1_1); // 110
    assign output_led8_0_11= ( input_input_switch3_3 &  input_input_switch2_2 &  input_input_switch1_1); // 111

endmodule