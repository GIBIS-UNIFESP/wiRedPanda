`timescale 1ns / 1ps

module input_tb;

    // Testbench signals
    reg input_push_button1_x0_1;
    reg input_push_button2_x1_2;
    wire output_led1_xor_0_3;
    wire output_led2_not_x0_0_4;
    wire output_led3_and_0_5;
    wire output_led4_or_0_6;

    // Expected outputs for verification
    reg expected_xor, expected_not, expected_and, expected_or;
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT)
    wiredpanda_module dut (
        .input_push_button1_x0_1(input_push_button1_x0_1),
        .input_push_button2_x1_2(input_push_button2_x1_2),
        .output_led1_xor_0_3(output_led1_xor_0_3),
        .output_led2_not_x0_0_4(output_led2_not_x0_0_4),
        .output_led3_and_0_5(output_led3_and_0_5),
        .output_led4_or_0_6(output_led4_or_0_6)
    );

    // Test procedure
    task test_logic;
        input [1:0] inputs;
        begin
            test_count = test_count + 1;

            // Apply inputs
            {input_push_button1_x0_1, input_push_button2_x1_2} = inputs;

            // Calculate expected outputs
            expected_xor = input_push_button1_x0_1 ^ input_push_button2_x1_2;
            expected_not = ~input_push_button1_x0_1;
            expected_and = input_push_button1_x0_1 & input_push_button2_x1_2;
            expected_or = input_push_button1_x0_1 | input_push_button2_x1_2;

            // Wait for combinational delay
            #10;

            // Check outputs
            if (output_led1_xor_0_3 === expected_xor &&
                output_led2_not_x0_0_4 === expected_not &&
                output_led3_and_0_5 === expected_and &&
                output_led4_or_0_6 === expected_or) begin
                $display("PASS: Test %0d - Inputs: %b%b, Outputs: XOR=%b NOT=%b AND=%b OR=%b",
                         test_count, input_push_button1_x0_1, input_push_button2_x1_2,
                         output_led1_xor_0_3, output_led2_not_x0_0_4, output_led3_and_0_5, output_led4_or_0_6);
                pass_count = pass_count + 1;
            end else begin
                $display("FAIL: Test %0d - Inputs: %b%b", test_count, input_push_button1_x0_1, input_push_button2_x1_2);
                $display("      Expected: XOR=%b NOT=%b AND=%b OR=%b", expected_xor, expected_not, expected_and, expected_or);
                $display("      Got:      XOR=%b NOT=%b AND=%b OR=%b", output_led1_xor_0_3, output_led2_not_x0_0_4, output_led3_and_0_5, output_led4_or_0_6);
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== INPUT LOGIC TESTBENCH ===");
        $display("Testing basic logic gates: XOR, NOT, AND, OR");

        // Initialize inputs
        input_push_button1_x0_1 = 0;
        input_push_button2_x1_2 = 0;

        // Test all input combinations
        test_logic(2'b00);
        test_logic(2'b01);
        test_logic(2'b10);
        test_logic(2'b11);

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

endmodule