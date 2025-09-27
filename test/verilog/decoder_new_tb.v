`timescale 1ns/1ps

module decoder_new_tb;
    // Inputs
    reg input_input_switch1_1;  // A (bit 0)
    reg input_input_switch2_2;  // B (bit 1)
    reg input_input_switch3_3;  // C (bit 2)

    // Outputs
    wire output_led1_0_4;
    wire output_led2_0_5;
    wire output_led3_0_6;
    wire output_led4_0_7;
    wire output_led5_0_8;
    wire output_led6_0_9;
    wire output_led7_0_10;
    wire output_led8_0_11;

    // Instantiate decoder
    decoder_new dut (
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

    // Combined output for easy checking
    wire [7:0] output_value = {output_led8_0_11, output_led7_0_10, output_led6_0_9, output_led5_0_8,
                               output_led4_0_7, output_led3_0_6, output_led2_0_5, output_led1_0_4};

    // Test stimulus
    initial begin
        $display("DECODER FUNCTIONALITY TEST:");
        $display("Input 000 should give 00000001 (LED1 active)");

        // Test the critical case: input 000
        {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = 3'b000;
        #10;
        $display("Expected: 00000001");
        $display("Got:      %08b", output_value);

        if (output_value == 8'b00000001) begin
            $display("PASS: Input 000 correctly activates LED1");
        end else begin
            $display("FAIL: Input 000 does not activate LED1");
        end

        $finish;
    end
endmodule