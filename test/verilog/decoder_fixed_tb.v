`timescale 1ns/1ps

module decoder_fixed_tb;
    // Inputs
    reg input_input_switch1_1;  // A (bit 0)
    reg input_input_switch2_2;  // B (bit 1)
    reg input_input_switch3_3;  // C (bit 2)

    // Outputs
    wire output_led1_0_4;   // Should activate for input 000
    wire output_led2_0_5;   // Should activate for input 001
    wire output_led3_0_6;   // Should activate for input 010
    wire output_led4_0_7;   // Should activate for input 011
    wire output_led5_0_8;   // Should activate for input 100
    wire output_led6_0_9;   // Should activate for input 101
    wire output_led7_0_10;  // Should activate for input 110
    wire output_led8_0_11;  // Should activate for input 111

    // Instantiate decoder
    decoder_fixed dut (
        .input_input_switch1_1(input_input_switch1_1),
        .input_input_switch2_2(input_input_switch2_2),
        .input_input_switch3_3(input_input_switch3_3),
        .output_led1_0_4(output_led1_0_4),
        .output_led2_0_5(output_led2_0_5),
        .output_led3_0_6(output_led3_0_6),
        .output_led4_0_7(output_led4_0_7),
        .output_led5_0_8(output_led5_0_8),
        .output_led6_0_9(output_led6_0_9),
        .output_led7_0_10(output_led7_0_10),
        .output_led8_0_11(output_led8_0_11)
    );

    // Test stimulus
    initial begin
        $dumpfile("decoder_fixed_tb.vcd");
        $dumpvars(0, decoder_fixed_tb);

        $display("Testing 3-to-8 Decoder:");
        $display("Input ABC -> Active Output (Expected: led[ABC+1])");

        // Test all 8 combinations
        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b000;
        #10;
        $display("000 -> led1=%b led2=%b led3=%b led4=%b led5=%b led6=%b led7=%b led8=%b",
                 output_led1_0_4, output_led2_0_5, output_led3_0_6, output_led4_0_7,
                 output_led5_0_8, output_led6_0_9, output_led7_0_10, output_led8_0_11);

        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b001;
        #10;
        $display("001 -> led1=%b led2=%b led3=%b led4=%b led5=%b led6=%b led7=%b led8=%b",
                 output_led1_0_4, output_led2_0_5, output_led3_0_6, output_led4_0_7,
                 output_led5_0_8, output_led6_0_9, output_led7_0_10, output_led8_0_11);

        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b010;
        #10;
        $display("010 -> led1=%b led2=%b led3=%b led4=%b led5=%b led6=%b led7=%b led8=%b",
                 output_led1_0_4, output_led2_0_5, output_led3_0_6, output_led4_0_7,
                 output_led5_0_8, output_led6_0_9, output_led7_0_10, output_led8_0_11);

        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b011;
        #10;
        $display("011 -> led1=%b led2=%b led3=%b led4=%b led5=%b led6=%b led7=%b led8=%b",
                 output_led1_0_4, output_led2_0_5, output_led3_0_6, output_led4_0_7,
                 output_led5_0_8, output_led6_0_9, output_led7_0_10, output_led8_0_11);

        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b100;
        #10;
        $display("100 -> led1=%b led2=%b led3=%b led4=%b led5=%b led6=%b led7=%b led8=%b",
                 output_led1_0_4, output_led2_0_5, output_led3_0_6, output_led4_0_7,
                 output_led5_0_8, output_led6_0_9, output_led7_0_10, output_led8_0_11);

        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b101;
        #10;
        $display("101 -> led1=%b led2=%b led3=%b led4=%b led5=%b led6=%b led7=%b led8=%b",
                 output_led1_0_4, output_led2_0_5, output_led3_0_6, output_led4_0_7,
                 output_led5_0_8, output_led6_0_9, output_led7_0_10, output_led8_0_11);

        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b110;
        #10;
        $display("110 -> led1=%b led2=%b led3=%b led4=%b led5=%b led6=%b led7=%b led8=%b",
                 output_led1_0_4, output_led2_0_5, output_led3_0_6, output_led4_0_7,
                 output_led5_0_8, output_led6_0_9, output_led7_0_10, output_led8_0_11);

        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b111;
        #10;
        $display("111 -> led1=%b led2=%b led3=%b led4=%b led5=%b led6=%b led7=%b led8=%b",
                 output_led1_0_4, output_led2_0_5, output_led3_0_6, output_led4_0_7,
                 output_led5_0_8, output_led6_0_9, output_led7_0_10, output_led8_0_11);

        $display("\nEXPECTED: Input 000 should activate led1, input 001 should activate led2, etc.");
        $display("ACTUAL BEHAVIOR: Check which LED is active for each input combination above.");

        $finish;
    end
endmodule