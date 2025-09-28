//========================================================================
// ULTRATHINK Improved Decoder Testbench - Parameterized Timing
//========================================================================
// Demonstrates application of parameterized timing template to decoder module

`timescale 1ns/1ps

module decoder_improved_tb;

    //====================================================================
    // TIMING PARAMETERS - ULTRATHINK Parameterization
    //====================================================================
    parameter PROP_DELAY       = 10;    // Combinational logic propagation delay
    parameter TEST_INTERVAL    = 50;    // Time between test cases
    parameter SIMULATION_TIME  = 1000;  // Total simulation time
    parameter VERBOSE_MODE     = 1;     // Enable detailed logging

    //====================================================================
    // SIGNAL DECLARATIONS
    //====================================================================
    // Input stimulus signals
    reg input_input_switch1_1;
    reg input_input_switch2_2;
    reg input_input_switch3_3;

    // Output monitoring signals
    wire output_led1_0_4;
    wire output_led2_0_5;
    wire output_led3_0_6;
    wire output_led4_0_7;
    wire output_led5_0_8;
    wire output_led6_0_9;
    wire output_led7_0_10;
    wire output_led8_0_11;

    // Test control signals
    reg [2:0] test_input;
    wire [7:0] outputs;
    integer test_count = 0;
    integer pass_count = 0;
    integer fail_count = 0;

    //====================================================================
    // DEVICE UNDER TEST INSTANTIATION
    //====================================================================
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

    // Combine outputs for easy testing
    assign outputs = {output_led8_0_11, output_led7_0_10, output_led6_0_9, output_led5_0_8,
                     output_led4_0_7, output_led3_0_6, output_led2_0_5, output_led1_0_4};

    //====================================================================
    // TEST STIMULUS GENERATION
    //====================================================================
    initial begin
        // Initialize signals
        {input_input_switch3_3, input_input_switch1_1, input_input_switch2_2} = 3'b000;

        if (VERBOSE_MODE) $display("[%0t] Starting 3-to-8 decoder test", $time);

        // Test all 8 possible input combinations
        for (integer i = 0; i < 8; i = i + 1) begin
            test_count = test_count + 1;
            test_input = i;
            {input_input_switch3_3, input_input_switch1_1, input_input_switch2_2} = test_input;

            if (VERBOSE_MODE)
                $display("[%0t] Test %0d: inputs=%3b", $time, test_count, test_input);

            #PROP_DELAY; // Wait for propagation delay

            check_decoder_outputs(test_input, outputs);

            #TEST_INTERVAL; // Wait before next test
        end

        // Test completion
        report_results();
        #(TEST_INTERVAL * 2);
        $finish;
    end

    //====================================================================
    // DECODER-SPECIFIC VERIFICATION
    //====================================================================
    task check_decoder_outputs;
        input [2:0] input_val;
        input [7:0] output_val;

        reg [7:0] expected;
        reg test_pass;
        integer bit_count;

        begin
            // For a 3-to-8 decoder, exactly one output should be high
            // Expected output: one-hot encoding based on input
            expected = 8'b00000001 << input_val;

            // Manual bit counting for compatibility
            bit_count = output_val[0] + output_val[1] + output_val[2] + output_val[3] +
                       output_val[4] + output_val[5] + output_val[6] + output_val[7];

            test_pass = (output_val == expected) && (bit_count == 1);

            if (test_pass) begin
                pass_count = pass_count + 1;
                if (VERBOSE_MODE)
                    $display("  ✓ PASS: output=%8b, expected=%8b (one-hot)", output_val, expected);
            end else begin
                fail_count = fail_count + 1;
                $display("  ✗ FAIL: output=%8b, expected=%8b (bit_count=%0d)",
                        output_val, expected, bit_count);
            end
        end
    endtask

    //====================================================================
    // RESULT REPORTING
    //====================================================================
    task report_results;
        begin
            $display("\n" + "="*60);
            $display("ULTRATHINK 3-TO-8 DECODER TEST RESULTS");
            $display("="*60);
            $display("Total tests:  %0d", test_count);
            $display("Passed:       %0d", pass_count);
            $display("Failed:       %0d", fail_count);
            $display("Success rate: %0.1f%%", (pass_count * 100.0) / test_count);

            if (fail_count == 0) begin
                $display("*** ALL DECODER TESTS PASSED ***");
                $display("✓ Proper 3-to-8 decoder functionality verified");
            end else begin
                $display("*** %0d DECODER TESTS FAILED ***", fail_count);
            end
            $display("="*60);
        end
    endtask

    //====================================================================
    // WAVEFORM GENERATION
    //====================================================================
    initial begin
        $dumpfile("decoder_improved_tb.vcd");
        $dumpvars(0, decoder_improved_tb);
    end

    //====================================================================
    // SIMULATION TIMEOUT PROTECTION
    //====================================================================
    initial begin
        #SIMULATION_TIME;
        $display("ERROR: Simulation timeout after %0dns", SIMULATION_TIME);
        $finish;
    end

endmodule