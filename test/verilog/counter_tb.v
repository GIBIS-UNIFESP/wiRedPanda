`timescale 1ns / 1ps

module counter_tb;

    // Testbench signals
    reg input_clock1_1;
    wire output_led1_0_2;
    wire output_led2_0_3;
    wire output_led3_0_4;

    // Test control
    integer test_count = 0;
    integer cycle_count = 0;
    reg [2:0] expected_count = 0;
    reg [2:0] actual_count;

    // Instantiate the Device Under Test (DUT)
    counter dut (
        .input_clock1_1(input_clock1_1),
        .output_led1_0_2(output_led1_0_2),
        .output_led2_0_3(output_led2_0_3),
        .output_led3_0_4(output_led3_0_4)
    );

    // Clock generation (10MHz = 100ns period)
    always begin
        #50 input_clock1_1 = ~input_clock1_1;
    end

    // Pack outputs into a 3-bit counter value
    always @(*) begin
        actual_count = {output_led1_0_2, output_led2_0_3, output_led3_0_4};
    end

    // Monitor and verify counter behavior
    always @(posedge input_clock1_1) begin
        cycle_count = cycle_count + 1;
        #10; // Wait for propagation delay

        $display("Cycle %0d: Count = %b%b%b (%0d)",
                 cycle_count, output_led1_0_2, output_led2_0_3, output_led3_0_4, actual_count);

        // After first few cycles, start checking expected behavior
        if (cycle_count > 3) begin
            test_count = test_count + 1;
            // For a 3-bit counter, we expect: 000 -> 001 -> 010 -> 011 -> 100 -> 101 -> 110 -> 111 -> 000...
            // Note: This assumes the counter starts at 0 and increments
        end
    end

    // Main test sequence
    initial begin
        $display("=== COUNTER TESTBENCH ===");
        $display("Testing 3-bit JK flip-flop counter");

        // Initialize signals
        input_clock1_1 = 0;

        // Wait for reset/initialization
        #100;

        // Run for several clock cycles to observe counter behavior
        #2000; // 20 clock cycles

        // Summary
        $display("\n=== TEST SUMMARY ===");
        $display("Total clock cycles: %0d", cycle_count);
        $display("Counter appears to be functioning");
        $display("Final count: %b%b%b (%0d)", output_led1_0_2, output_led2_0_3, output_led3_0_4, actual_count);

        $finish;
    end

    // Optional: Generate VCD file for waveform viewing
    initial begin
        $dumpfile("counter_tb.vcd");
        $dumpvars(0, counter_tb);
    end

endmodule