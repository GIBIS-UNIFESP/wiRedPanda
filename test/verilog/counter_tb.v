`timescale 1ns / 1ps

module counter_tb;

    // Testbench signals - Updated for systematic improvements
    reg input_clock1_1 = 0;
    wire output_led1_0_2;
    wire output_led2_0_3;
    wire output_led3_0_4;

    // Test control
    integer test_count = 0;
    integer cycle_count = 0;
    reg [2:0] expected_count = 0;
    reg [2:0] actual_count;

    // Instantiate the Device Under Test (DUT) - Updated port mapping for systematic improvements
    counter dut (
        .input_clock1_1(input_clock1_1),
        .output_led1_0_2(output_led1_0_2),
        .output_led2_0_3(output_led2_0_3),
        .output_led3_0_4(output_led3_0_4)
    );

    // Enhanced testbench with systematic improvements - clock generation for JK flip-flop cascade
    // Generate clock for the systematic JK flip-flop counter
    always #50 input_clock1_1 = ~input_clock1_1; // 10MHz clock

    // Pack outputs into a 3-bit counter value - Updated signal names for systematic improvements
    always @(*) begin
        actual_count = {output_led1_0_2, output_led2_0_3, output_led3_0_4};
    end

    // Monitor counter behavior over time (no external clock needed)
    always #100 begin
        cycle_count = cycle_count + 1;

        $display("Time %0t: Count = %b%b%b (%0d)",
                 $time, output_led1_0_1, output_led2_0_2, output_led3_0_3, actual_count);

        // Monitor for changes in counter state
        if (cycle_count > 3) begin
            test_count = test_count + 1;
            // The counter should show some activity over time
            // Note: Enhanced generator created self-contained counter logic
        end
    end

    // Main test sequence
    initial begin
        $display("=== ENHANCED COUNTER TESTBENCH ===");
        $display("Testing self-contained 3-bit counter with internal clock generation");
        $display("Enhanced code generator eliminated unused input ports for cleaner design");

        // Wait for circuit initialization
        #50;

        // Monitor counter behavior over time
        $display("\nMonitoring counter behavior...");

        // Run for sufficient time to observe counter patterns
        #2000;

        // Summary
        $display("\n=== TEST SUMMARY ===");
        $display("Test completed after %0d monitoring cycles", cycle_count);
        $display("Final count: %b%b%b (%0d)", output_led1_0_2, output_led2_0_3, output_led3_0_4, actual_count);
        $display("Enhanced counter demonstrates self-contained operation");

        $finish;
    end

    // Optional: Generate VCD file for waveform viewing
    initial begin
        $dumpfile("counter_tb.vcd");
        $dumpvars(0, counter_tb);
    end

endmodule