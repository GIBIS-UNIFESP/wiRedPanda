`timescale 1ns / 1ps

module dflipflop2_tb;

    // Testbench signals - Updated for input port dead code elimination
    reg input_clock1_1;
    wire output_led1_0_3;
    wire output_led2_0_4;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT) - Updated for input port dead code elimination
    dflipflop2 dut (
        .input_clock1_1(input_clock1_1),
        .output_led1_0_3(output_led1_0_3),
        .output_led2_0_4(output_led2_0_4)
    );

    // Clock generation
    initial begin
        input_clock1_1 = 0;
        forever #50 input_clock1_1 = ~input_clock1_1; // 10 MHz clock
    end

    // Test procedure for D flip-flop with clock and data inputs
    task test_dff(input data_val, input [31:0] test_num);
        begin
            test_count = test_count + 1;

            // Set data input
            // Data input removed by dead code elimination

            // Wait for next clock edge
            @(posedge input_clock1_1);
            #10; // Small delay for propagation

            $display("Test %0d: D=%b, CLK edge => Q=%b, Qbar=%b",
                     test_num, data_val, output_led1_0_3, output_led2_0_4);

            // Verify D flip-flop behavior: Q should equal D after clock edge
            if (output_led1_0_3 == data_val && output_led2_0_4 == ~data_val) begin
                pass_count = pass_count + 1;
                $display("      PASS: D flip-flop behavior correct");
            end else begin
                $display("      FAIL: Expected Q=%b, Qbar=%b, Got Q=%b, Qbar=%b",
                         data_val, ~data_val, output_led1_0_3, output_led2_0_4);
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== D FLIP-FLOP 2 TESTBENCH ===");
        $display("Testing D flip-flop with clock and data inputs");

        // Initialize inputs
        // Data input initialization removed by dead code elimination

        // Wait for initialization and first clock edge
        #200;

        // Test D flip-flop functionality
        $display("\nTesting D flip-flop behavior:");
        test_dff(1'b0, 1);  // Test with D=0
        test_dff(1'b1, 2);  // Test with D=1
        test_dff(1'b0, 3);  // Test with D=0 again
        test_dff(1'b1, 4);  // Test with D=1 again

        // Additional monitoring
        $display("\nFinal output state:");
        $display("output_led1_0_3 (Q) = %b", output_led1_0_3);
        $display("output_led2_0_4 (Qbar) = %b", output_led2_0_4);

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

        $display("D flip-flop 2 testbench completed");
        $finish;
    end

    // Optional: Generate VCD file for waveform viewing
    initial begin
        $dumpfile("dflipflop2_tb.vcd");
        $dumpvars(0, dflipflop2_tb);
    end

endmodule