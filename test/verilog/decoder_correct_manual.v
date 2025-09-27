// Manual correction to verify the correct decoder logic
`timescale 1ns/1ps

module decoder_correct_manual (
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

    // ========= CORRECTED Logic Assignments =========
    // 3-to-8 decoder: inputs CBA activate corresponding output
    assign output_led1_0_4 = (~input_input_switch3_3 & ~input_input_switch2_2 & ~input_input_switch1_1); // 000 -> LED1
    assign output_led2_0_5 = (~input_input_switch3_3 & ~input_input_switch2_2 &  input_input_switch1_1); // 001 -> LED2
    assign output_led3_0_6 = (~input_input_switch3_3 &  input_input_switch2_2 & ~input_input_switch1_1); // 010 -> LED3
    assign output_led4_0_7 = (~input_input_switch3_3 &  input_input_switch2_2 &  input_input_switch1_1); // 011 -> LED4
    assign output_led5_0_8 = ( input_input_switch3_3 & ~input_input_switch2_2 & ~input_input_switch1_1); // 100 -> LED5
    assign output_led6_0_9 = ( input_input_switch3_3 & ~input_input_switch2_2 &  input_input_switch1_1); // 101 -> LED6
    assign output_led7_0_10= ( input_input_switch3_3 &  input_input_switch2_2 & ~input_input_switch1_1); // 110 -> LED7
    assign output_led8_0_11= ( input_input_switch3_3 &  input_input_switch2_2 &  input_input_switch1_1); // 111 -> LED8

endmodule