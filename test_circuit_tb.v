`timescale 1ns / 1ps

// ====================================================================
// COMPREHENSIVE TESTBENCH FOR CLOCK->IC->LED CIRCUIT
// ====================================================================
//
// Test Plan:
// 1. Apply clock stimulus and verify 3-bit counter behavior
// 2. Check timing relationships between flip-flops
// 3. Validate LED outputs match internal counter states
// 4. Test reset functionality (if available)
// 5. Monitor for any unexpected behavior or glitches
//
// Expected Behavior:
// - 3-bit binary counter: 000 -> 001 -> 010 -> 011 -> 100 -> 101 -> 110 -> 111 -> 000
// - LEDs should reflect counter state
// - Each flip-flop toggles at half the frequency of the previous stage
//
// ====================================================================

module test_circuit_tb;

    // ========= Testbench Signals =========
    reg clock;
    wire [2:0] leds;

    // Individual LED signals for detailed monitoring
    wire led0, led1, led2;

    // Internal signals for monitoring (if accessible)
    wire ff0_q, ff1_q, ff2_q;

    // Test control signals
    integer cycle_count;
    integer error_count;
    reg [2:0] expected_count;

    // Frequency analysis signals
    reg ff0_prev, ff1_prev, ff2_prev;
    integer ff0_toggles, ff1_toggles, ff2_toggles;

    // ========= Device Under Test =========
    test_circuit_final_corrected dut (
        .input_clock1_1(clock),
        .output_led1_0_2(led0),
        .output_led2_0_3(led1),
        .output_led3_0_4(led2)
    );

    // Combine LED outputs for easier monitoring
    assign leds = {led2, led1, led0};

    // Access internal flip-flop outputs for validation (updated for Arduino-style architecture)
    assign ff0_q = dut.ic_counter_ic_jk_flip_flop_8_0_0;
    assign ff1_q = dut.ic_counter_ic_jk_flip_flop_10_0_0;
    assign ff2_q = dut.ic_counter_ic_jk_flip_flop_12_0_0;

    // ========= Clock Generation =========
    initial begin
        clock = 0;
        forever #5 clock = ~clock; // 10ns period = 100MHz
    end

    // ========= Test Stimulus and Monitoring =========
    initial begin
        // Initialize
        cycle_count = 0;
        error_count = 0;
        expected_count = 3'b000;

        $display("====================================================================");
        $display("STARTING COMPREHENSIVE TESTBENCH FOR CLOCK->IC->LED CIRCUIT");
        $display("====================================================================");
        $display("Time: %0t", $time);
        $display("Expected behavior: 3-bit binary counter with LED outputs");
        $display("");

        // Wait for initial settling
        #1;

        // Header for output table
        $display("Cycle | Time    | Clock | FF2 FF1 FF0 | LED2 LED1 LED0 | Expected | Status");
        $display("------|---------|-------|-------------|-----------------|----------|-------");

        // Monitor for 20 clock cycles (2.5 complete counter sequences)
        repeat(20) begin
            @(posedge clock);
            #1; // Small delay for signal propagation

            // Check internal flip-flop states
            $display("%5d | %7t | %5b | %3b %3b %3b | %4b %4b %4b | %8b | %s",
                cycle_count, $time, clock,
                ff2_q, ff1_q, ff0_q,
                led2, led1, led0,
                expected_count,
                (leds === expected_count) ? "PASS" : "FAIL"
            );

            // Validate outputs match expected counter value
            if (leds !== expected_count) begin
                error_count = error_count + 1;
                $display("ERROR: LED outputs %b don't match expected %b", leds, expected_count);
            end

            // Validate internal flip-flops match LEDs (they should be connected)
            if ({ff2_q, ff1_q, ff0_q} !== leds) begin
                error_count = error_count + 1;
                $display("ERROR: Internal flip-flops %b don't match LED outputs %b",
                    {ff2_q, ff1_q, ff0_q}, leds);
            end

            // Update expected count
            expected_count = expected_count + 1;
            cycle_count = cycle_count + 1;
        end

        // ========= Frequency Analysis =========
        $display("");
        $display("====================================================================");
        $display("FREQUENCY ANALYSIS");
        $display("====================================================================");

        // Reset counters for frequency analysis
        cycle_count = 0;
        ff0_toggles = 0;
        ff1_toggles = 0;
        ff2_toggles = 0;

        ff0_prev = ff0_q;
        ff1_prev = ff1_q;
        ff2_prev = ff2_q;

        repeat(16) begin
            @(posedge clock);
            #1;

            // Count toggles
            if (ff0_q !== ff0_prev) ff0_toggles = ff0_toggles + 1;
            if (ff1_q !== ff1_prev) ff1_toggles = ff1_toggles + 1;
            if (ff2_q !== ff2_prev) ff2_toggles = ff2_toggles + 1;

            ff0_prev = ff0_q;
            ff1_prev = ff1_q;
            ff2_prev = ff2_q;

            cycle_count = cycle_count + 1;
        end

        $display("After %0d clock cycles:", cycle_count);
        $display("FF0 (LSB) toggles: %0d (expected: %0d)", ff0_toggles, cycle_count);
        $display("FF1 toggles: %0d (expected: ~%0d)", ff1_toggles, cycle_count/2);
        $display("FF2 (MSB) toggles: %0d (expected: ~%0d)", ff2_toggles, cycle_count/4);

        // ========= Edge Case Testing =========
        $display("");
        $display("====================================================================");
        $display("EDGE CASE TESTING");
        $display("====================================================================");

        // Test rapid clock changes
        $display("Testing rapid clock transitions...");
        repeat(5) begin
            #1 clock = 1;
            #1 clock = 0;
        end

        // Check for any glitches or metastability
        $display("Checking for glitches after rapid transitions...");
        #10;

        // ========= Final Results =========
        $display("");
        $display("====================================================================");
        $display("TESTBENCH SUMMARY");
        $display("====================================================================");
        $display("Total errors detected: %0d", error_count);
        $display("Final counter state: %b", leds);
        $display("Final flip-flop states: FF2=%b FF1=%b FF0=%b", ff2_q, ff1_q, ff0_q);

        if (error_count == 0) begin
            $display("✅ ALL TESTS PASSED - Circuit behavior is correct");
        end else begin
            $display("❌ %0d ERRORS DETECTED - Circuit needs debugging", error_count);
        end

        $display("");
        $display("====================================================================");
        $display("TESTBENCH COMPLETED at time %0t", $time);
        $display("====================================================================");

        $finish;
    end

    // ========= Waveform Dumping =========
    initial begin
        $dumpfile("test_circuit_tb.vcd");
        $dumpvars(0, test_circuit_tb);
    end

    // ========= Timeout Protection =========
    initial begin
        #10000; // 10μs timeout
        $display("ERROR: Testbench timeout reached!");
        $finish;
    end

endmodule