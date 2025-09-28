`timescale 1ns/1ps

module decoder_functional_test;

    // Inputs
    reg input_input_switch1_1;
    reg input_input_switch2_2;
    reg input_input_switch3_3;

    // Outputs
    wire output_led1_0_4;
    wire output_led2_0_5;
    wire output_led3_0_6;
    wire output_led4_0_7;
    wire output_led5_0_8;
    wire output_led6_0_9;
    wire output_led7_0_10;
    wire output_led8_0_11;

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

    // Test vectors
    reg [2:0] test_input;
    wire [7:0] outputs;
    assign outputs = {output_led8_0_11, output_led7_0_10, output_led6_0_9, output_led5_0_8,
                     output_led4_0_7, output_led3_0_6, output_led2_0_5, output_led1_0_4};

    integer i, pass_count, fail_count;
    reg [3:0] bit_count;

    initial begin
        $display("=== DECODER FUNCTIONAL TEST ===");
        $display("Testing 3-to-8 decoder functionality");

        pass_count = 0;
        fail_count = 0;

        // Test all 8 possible input combinations
        for (i = 0; i < 8; i = i + 1) begin
            test_input = i;
            {input_input_switch3_3, input_input_switch1_1, input_input_switch2_2} = test_input;

            #10; // Wait for propagation

            $display("Input: %3b, Output: %8b", test_input, outputs);

            // For a 3-to-8 decoder, exactly one output should be high
            // Manual bit counting
            bit_count = outputs[0] + outputs[1] + outputs[2] + outputs[3] +
                       outputs[4] + outputs[5] + outputs[6] + outputs[7];

            if (bit_count == 1) begin
                $display("  ✓ PASS: Exactly one output active");
                pass_count = pass_count + 1;
            end else begin
                $display("  ✗ FAIL: Expected exactly one output active, got %0d", bit_count);
                fail_count = fail_count + 1;
            end
        end

        $display("\n=== RESULTS ===");
        $display("Tests passed: %0d", pass_count);
        $display("Tests failed: %0d", fail_count);

        if (fail_count == 0) begin
            $display("*** ALL DECODER TESTS PASSED ***");
        end else begin
            $display("*** SOME DECODER TESTS FAILED ***");
        end

        $finish;
    end

    // Generate VCD for debugging
    initial begin
        $dumpfile("decoder_functional_test.vcd");
        $dumpvars(0, decoder_functional_test);
    end

endmodule