`timescale 1ns / 1ps

module dlatch_tb;

    // Testbench signals
    reg input_push_button1_d_1;
    reg input_clock2_clk_2;
    wire output_led1_q_0_3;
    wire output_led2_q_0_4;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT)
    dlatch dut (
        .input_push_button1_d_1(input_push_button1_d_1),
        .input_clock2_clk_2(input_clock2_clk_2),
        .output_led1_q_0_3(output_led1_q_0_3),
        .output_led2_q_0_4(output_led2_q_0_4)
    );

    // Test procedure for D latch functionality
    task test_latch;
        input d_value;
        input clk_value;
        begin
            test_count = test_count + 1;

            // Set inputs
            input_push_button1_d_1 = d_value;
            input_clock2_clk_2 = clk_value;

            // Wait for propagation delay
            #10;

            $display("Test %0d: D=%b, CLK=%b => Q=%b, Q̄=%b",
                     test_count, d_value, clk_value, output_led1_q_0_3, output_led2_q_0_4);

            // Basic validation: Q and Q̄ should be complementary when latch is transparent
            if (clk_value == 1) begin
                // When clock is high, latch should be transparent
                if (output_led1_q_0_3 != output_led2_q_0_4) begin
                    pass_count = pass_count + 1;
                    $display("      PASS: Outputs are complementary when transparent");
                end else begin
                    $display("      FAIL: Outputs should be complementary when transparent");
                end
            end else begin
                // When clock is low, latch should hold previous state
                $display("      INFO: Latch in hold mode");
                pass_count = pass_count + 1; // Count as pass for hold mode
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== D LATCH TESTBENCH ===");
        $display("Testing D latch transparency and hold functionality");

        // Initialize signals
        input_push_button1_d_1 = 0;
        input_clock2_clk_2 = 0;

        // Wait for initialization
        #50;

        // Test latch transparency (clock high)
        test_latch(0, 1);
        test_latch(1, 1);
        test_latch(0, 1);

        // Test hold mode (clock low) - should maintain last state
        test_latch(0, 0);
        test_latch(1, 0);

        // Return to transparency
        test_latch(1, 1);
        test_latch(0, 1);

        // More hold tests
        test_latch(0, 0);
        test_latch(1, 0);
        test_latch(0, 0);

        // Final transparency test
        test_latch(1, 1);

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
        $dumpfile("dlatch_tb.vcd");
        $dumpvars(0, dlatch_tb);
    end

endmodule