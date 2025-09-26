`timescale 1ns / 1ps

module decoder_tb;

    // Testbench signals
    reg input_input_switch1_1;
    reg input_input_switch2_2;
    reg input_input_switch3_3;
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
    reg [2:0] input_value;
    reg [7:0] output_value;
    reg [7:0] expected_output;

    // Instantiate the Device Under Test (DUT)
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

    // Test procedure for 3-to-8 decoder
    task test_decoder;
        input [2:0] inputs;
        begin
            test_count = test_count + 1;

            // Apply inputs
            {input_input_switch3_3, input_input_switch2_2, input_input_switch1_1} = inputs;
            input_value = inputs;

            // Calculate expected output (one-hot encoding)
            expected_output = 8'b00000001 << inputs;

            // Wait for combinational delay
            #10;

            // Check outputs
            if (output_value === expected_output) begin
                $display("PASS: Test %0d - Input: %3b (%0d), Output: %8b (expected: %8b)",
                         test_count, input_value, input_value, output_value, expected_output);
                pass_count = pass_count + 1;
            end else begin
                $display("FAIL: Test %0d - Input: %3b (%0d)", test_count, input_value, input_value);
                $display("      Expected: %8b", expected_output);
                $display("      Got:      %8b", output_value);
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== 3-TO-8 DECODER TESTBENCH ===");
        $display("Testing all 8 possible input combinations");

        // Initialize inputs
        input_input_switch1_1 = 0;
        input_input_switch2_2 = 0;
        input_input_switch3_3 = 0;

        // Wait for initialization
        #50;

        // Test all 8 input combinations (3-bit inputs: 000 to 111)
        test_decoder(3'b000); // Should activate output_led1_0_4
        test_decoder(3'b001); // Should activate output_led2_0_5
        test_decoder(3'b010); // Should activate output_led3_0_6
        test_decoder(3'b011); // Should activate output_led4_0_7
        test_decoder(3'b100); // Should activate output_led5_0_8
        test_decoder(3'b101); // Should activate output_led6_0_9
        test_decoder(3'b110); // Should activate output_led7_0_10
        test_decoder(3'b111); // Should activate output_led8_0_11

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

        $finish;
    end

    // Optional: Generate VCD file for waveform viewing
    initial begin
        $dumpfile("decoder_tb.vcd");
        $dumpvars(0, decoder_tb);
    end

endmodule