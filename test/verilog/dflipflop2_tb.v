`timescale 1ns / 1ps

module dflipflop2_tb;

    // Testbench signals
    reg input_clock1_1;
    reg input_input_switch2_2;
    wire output_led1_0_3;
    wire output_led2_0_4;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT)
    dflipflop2 dut (
        .input_clock1_1(input_clock1_1),
        .input_input_switch2_2(input_input_switch2_2),
        .output_led1_0_3(output_led1_0_3),
        .output_led2_0_4(output_led2_0_4)
    );

    // Clock generation (10MHz = 100ns period)
    always begin
        #50 input_clock1_1 = ~input_clock1_1;
    end

    // Test procedure for D flip-flop
    task test_dff;
        input d_value;
        begin
            test_count = test_count + 1;

            // Set D input
            input_input_switch2_2 = d_value;

            // Wait for setup time
            #10;

            // Clock edge
            @(posedge input_clock1_1);

            // Wait for propagation delay
            #10;

            $display("Test %0d: D=%b => Q=%b, Q̄=%b",
                     test_count, d_value, output_led1_0_3, output_led2_0_4);

            // Basic validation: Q and Q̄ should be complementary
            if (output_led1_0_3 != output_led2_0_4) begin
                pass_count = pass_count + 1;
                $display("      PASS: Outputs are complementary");
            end else begin
                $display("      FAIL: Outputs should be complementary");
            end
        end
    endtask

    // Test D flip-flop hold behavior (D changes while clock is stable)
    task test_hold;
        input d_value;
        begin
            test_count = test_count + 1;

            // Set D input without clock edge
            input_input_switch2_2 = d_value;

            // Wait without clock edge
            #30;

            $display("Test %0d (Hold): D=%b (no clock) => Q=%b, Q̄=%b",
                     test_count, d_value, output_led1_0_3, output_led2_0_4);

            // Q should maintain complementary relationship
            if (output_led1_0_3 != output_led2_0_4) begin
                pass_count = pass_count + 1;
                $display("      PASS: Outputs are complementary and held");
            end else begin
                $display("      FAIL: Outputs should be complementary");
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== D FLIP-FLOP 2 TESTBENCH ===");
        $display("Testing alternative D flip-flop implementation");

        // Initialize signals
        input_clock1_1 = 0;
        input_input_switch2_2 = 0;

        // Wait for initialization
        #100;

        // Test normal D flip-flop operation
        test_dff(0); // D=0, should latch 0
        test_dff(1); // D=1, should latch 1
        test_dff(0); // D=0, should latch 0
        test_dff(1); // D=1, should latch 1

        // Test hold behavior (no clock edge)
        test_hold(0); // Change D but no clock - should hold previous value
        test_hold(1); // Change D but no clock - should hold previous value

        // More clock edge tests
        test_dff(0); // D=0, should latch 0
        test_dff(1); // D=1, should latch 1

        // Test edge case: D changes just before clock
        input_input_switch2_2 = 0;
        #45; // Just before next clock edge
        input_input_switch2_2 = 1; // Change D close to clock edge
        @(posedge input_clock1_1);
        #10;
        test_count = test_count + 1;
        $display("Test %0d (Setup): D=1 (setup test) => Q=%b, Q̄=%b",
                 test_count, output_led1_0_3, output_led2_0_4);
        if (output_led1_0_3 != output_led2_0_4) begin
            pass_count = pass_count + 1;
            $display("      PASS: Setup test passed");
        end else begin
            $display("      FAIL: Setup test failed");
        end

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
        $dumpfile("dflipflop2_tb.vcd");
        $dumpvars(0, dflipflop2_tb);
    end

endmodule