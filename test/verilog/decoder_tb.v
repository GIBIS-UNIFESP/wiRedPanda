`timescale 1ns / 1ps

module decoder_tb;

    // Testbench signals - Updated for systematic improvements with input ports
    reg input_input_switch1_1 = 0;
    reg input_input_switch2_2 = 0;
    reg input_input_switch3_3 = 0;
    wire output_led1_0_4;
    wire output_led2_0_5;
    wire output_led3_0_6;
    wire output_led4_0_7;
    wire output_led5_0_8;
    wire output_led6_0_9;
    wire output_led7_0_10;
    wire output_led8_0_11;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;
    reg [7:0] output_value;
    reg [7:0] expected_output;

    // Instantiate the Device Under Test (DUT) - Updated for systematic improvements
    decoder dut (
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

    // Pack outputs into a single vector for easier checking
    always @(*) begin
        output_value = {output_led8_0_11, output_led7_0_10, output_led6_0_9, output_led5_0_8,
                       output_led4_0_7, output_led3_0_6, output_led2_0_5, output_led1_0_4};
    end

    // Test procedure for systematic 3-to-8 decoder with all input combinations
    task test_decoder_output;
        input [2:0] test_input;
        input [7:0] expected;
        begin
            test_count = test_count + 1;

            // Set decoder inputs
            {input_input_switch3_3, input_input_switch1_1, input_input_switch2_2} = test_input;

            // Wait for combinational delay
            #10;

            // Check outputs
            if (output_value === expected) begin
                $display("PASS: Test %0d - Input %3b: Output %8b (expected: %8b)",
                         test_count, test_input, output_value, expected);
                pass_count = pass_count + 1;
            end else begin
                $display("FAIL: Test %0d - Input %3b failed", test_count, test_input);
                $display("      Expected: %8b", expected);
                $display("      Got:      %8b", output_value);
                $display("      Systematic improvements: 3-to-8 decoder with boolean expression corrections");
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== SYSTEMATIC 3-TO-8 DECODER TESTBENCH ===");
        $display("Testing decoder with systematic boolean expression improvements");
        $display("All 8 input combinations will be tested for correct 3-to-8 decoding");

        // Wait for initialization
        #50;

        // Test all 8 combinations of the 3-to-8 decoder
        // Expected: one-hot encoding where input N activates output N
        test_decoder_output(3'b000, 8'b00000001); // Input 0 -> Output 0 (LED1)
        test_decoder_output(3'b001, 8'b00000010); // Input 1 -> Output 1 (LED2)
        test_decoder_output(3'b010, 8'b00000100); // Input 2 -> Output 2 (LED3)
        test_decoder_output(3'b011, 8'b00001000); // Input 3 -> Output 3 (LED4)
        test_decoder_output(3'b100, 8'b00010000); // Input 4 -> Output 4 (LED5)
        test_decoder_output(3'b101, 8'b00100000); // Input 5 -> Output 5 (LED6)
        test_decoder_output(3'b110, 8'b01000000); // Input 6 -> Output 6 (LED7)
        test_decoder_output(3'b111, 8'b10000000); // Input 7 -> Output 7 (LED8)

        // Additional monitoring
        $display("\nDetailed output analysis:");
        $display("output_led1_0_4 = %b", output_led1_0_4);
        $display("output_led2_0_5 = %b", output_led2_0_5);
        $display("output_led3_0_6 = %b", output_led3_0_6);
        $display("output_led4_0_7 = %b", output_led4_0_7);
        $display("output_led5_0_8 = %b", output_led5_0_8);
        $display("output_led6_0_9 = %b", output_led6_0_9);
        $display("output_led7_0_10 = %b", output_led7_0_10);
        $display("output_led8_0_11 = %b", output_led8_0_11);

        // Summary
        $display("\n=== TEST SUMMARY ===");
        $display("Total tests: %0d", test_count);
        $display("Passed: %0d", pass_count);
        $display("Failed: %0d", test_count - pass_count);

        if (pass_count == test_count) begin
            $display("*** ALL TESTS PASSED ***");
        end else begin
            $display("*** SOME TESTS FAILED ***");
        end

        $display("Enhanced decoder demonstrates self-contained operation");
        $finish;
    end

    // Optional: Generate VCD file for waveform viewing
    initial begin
        $dumpfile("decoder_tb.vcd");
        $dumpvars(0, decoder_tb);
    end

endmodule