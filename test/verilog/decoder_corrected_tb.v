`timescale 1ns/1ps

module decoder_corrected_tb;

    // Outputs
    wire output_led1_0_1;
    wire output_led2_0_2;
    wire output_led3_0_3;
    wire output_led4_0_4;
    wire output_led5_0_5;
    wire output_led6_0_6;
    wire output_led7_0_7;
    wire output_led8_0_8;

    // Concatenate outputs for easy testing
    wire [7:0] output_value = {output_led8_0_8, output_led7_0_7, output_led6_0_6, output_led5_0_5,
                               output_led4_0_4, output_led3_0_3, output_led2_0_2, output_led1_0_1};

    // Instantiate the corrected decoder
    decoder_corrected dut (
        .output_led1_0_1(output_led1_0_1),
        .output_led2_0_2(output_led2_0_2),
        .output_led3_0_3(output_led3_0_3),
        .output_led4_0_4(output_led4_0_4),
        .output_led5_0_5(output_led5_0_5),
        .output_led6_0_6(output_led6_0_6),
        .output_led7_0_7(output_led7_0_7),
        .output_led8_0_8(output_led8_0_8)
    );

    initial begin
        $display("=== CORRECTED DECODER TEST ===");

        // Wait for combinational delay
        #10;

        $display("For hardcoded input 000:");
        $display("Expected: 00000001 (output 0 active)");
        $display("Got:      %8b", output_value);

        if (output_value === 8'b00000001) begin
            $display("✅ PASS: Corrected decoder logic works!");
        end else begin
            $display("❌ FAIL: Corrected decoder logic still wrong");
        end

        $display("Individual outputs:");
        $display("LED1=%b LED2=%b LED3=%b LED4=%b LED5=%b LED6=%b LED7=%b LED8=%b",
                 output_led1_0_1, output_led2_0_2, output_led3_0_3, output_led4_0_4,
                 output_led5_0_5, output_led6_0_6, output_led7_0_7, output_led8_0_8);

        $finish;
    end

endmodule