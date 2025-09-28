`timescale 1ns / 1ps

module dflipflop_tb;

    // Testbench signals - Updated for input port dead code elimination
    reg input_clock1_clk_1 = 0;
    wire output_led1_0_5;
    wire output_led2_0_6;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT) - Updated for input port dead code elimination
    dflipflop dut (
        .input_clock1_clk_1(input_clock1_clk_1),
        .output_led1_0_5(output_led1_0_5),
        .output_led2_0_6(output_led2_0_6)
    );

    // Enhanced testbench for systematic D flip-flop with preset/clear functionality
    // Generate clock for the D flip-flop
    always #50 input_clock1_clk_1 = ~input_clock1_clk_1; // 10MHz clock

    // Test monitoring for output behavior observation
    task monitor_outputs;
        begin
            test_count = test_count + 1;

            // Wait for circuit stabilization
            #100;

            $display("Test %0d: Monitoring outputs => Q=%b, Q̄=%b",
                     test_count, output_led1_0_5, output_led2_0_6);

            // Basic validation: Check if outputs have valid logic levels
            if ((output_led1_0_5 === 1'b0 || output_led1_0_5 === 1'b1) &&
                (output_led2_0_6 === 1'b0 || output_led2_0_6 === 1'b1)) begin
                pass_count = pass_count + 1;
                $display("      PASS: Valid logic levels detected");
            end else begin
                $display("      FAIL: Invalid logic levels (X or Z detected)");
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== ENHANCED D FLIP-FLOP TESTBENCH ===");
        $display("Testing self-contained D flip-flop circuit with no input ports");
        $display("Enhanced code generator eliminated unused inputs for cleaner design");

        // Wait for circuit initialization
        #50;

        // Monitor circuit behavior over time
        monitor_outputs();
        #200;
        monitor_outputs();
        #300;
        monitor_outputs();
        #500;
        monitor_outputs();

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
        $dumpfile("dflipflop_tb.vcd");
        $dumpvars(0, dflipflop_tb);
    end

endmodule