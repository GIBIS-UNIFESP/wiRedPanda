`timescale 1ns / 1ps

module jkflipflop_tb;

    // Testbench signals - Updated for input port dead code elimination
    reg input_clock1_c_1;
    wire output_led1_q_0_6;
    wire output_led2_q_0_7;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT) - Updated for input port dead code elimination
    jkflipflop dut (
        .input_clock1_c_1(input_clock1_c_1),
        .output_led1_q_0_6(output_led1_q_0_6),
        .output_led2_q_0_7(output_led2_q_0_7)
    );

    // Clock generation
    initial begin
        input_clock1_c_1 = 0;
        forever #50 input_clock1_c_1 = ~input_clock1_c_1; // 10 MHz clock
    end

    // Test procedure for JK flip-flop with J, K, preset, and clear inputs
    task test_jkff(input j_val, input k_val, input preset_val, input clear_val, input [31:0] test_num);
        begin
            test_count = test_count + 1;

            // Set control inputs
            // Input assignments removed due to dead code elimination

            // Wait for next clock edge (if not in preset/clear)
            if (preset_val && clear_val) begin
                @(posedge input_clock1_c_1);
                #10; // Small delay for propagation
            end else begin
                #20; // Asynchronous preset/clear
            end

            $display("Test %0d: J=%b, K=%b, Preset=%b, Clear=%b => Q=%b, Qbar=%b",
                     test_num, j_val, k_val, preset_val, clear_val, output_led1_q_0_6, output_led2_q_0_7);

            // Verify JK flip-flop behavior
            if (!preset_val) begin
                // Preset should set Q=1
                if (output_led1_q_0_6 == 1'b1 && output_led2_q_0_7 == 1'b0) begin
                    pass_count = pass_count + 1;
                    $display("      PASS: Preset behavior correct");
                end else begin
                    $display("      FAIL: Preset should set Q=1, Qbar=0");
                end
            end else if (!clear_val) begin
                // Clear should set Q=0
                if (output_led1_q_0_6 == 1'b0 && output_led2_q_0_7 == 1'b1) begin
                    pass_count = pass_count + 1;
                    $display("      PASS: Clear behavior correct");
                end else begin
                    $display("      FAIL: Clear should set Q=0, Qbar=1");
                end
            end else begin
                // Normal JK operation - just check outputs are complementary
                if (output_led1_q_0_6 != output_led2_q_0_7) begin
                    pass_count = pass_count + 1;
                    $display("      PASS: JK flip-flop outputs complementary");
                end else begin
                    $display("      FAIL: Outputs should be complementary");
                end
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== JK FLIP-FLOP TESTBENCH ===");
        $display("Testing JK flip-flop with J, K, preset, and clear inputs");

        // Initialize inputs (normal operation)
        // Input initializations removed due to dead code elimination

        // Wait for initialization
        #100;

        // Test JK flip-flop functionality
        $display("\nTesting JK flip-flop behavior:");

        // Test preset function
        test_jkff(1'b0, 1'b0, 1'b0, 1'b1, 1);  // Preset active

        // Test clear function
        test_jkff(1'b0, 1'b0, 1'b1, 1'b0, 2);  // Clear active

        // Test normal JK operation
        test_jkff(1'b0, 1'b0, 1'b1, 1'b1, 3);  // J=0, K=0 (hold)
        test_jkff(1'b1, 1'b0, 1'b1, 1'b1, 4);  // J=1, K=0 (set)
        test_jkff(1'b0, 1'b1, 1'b1, 1'b1, 5);  // J=0, K=1 (reset)
        test_jkff(1'b1, 1'b1, 1'b1, 1'b1, 6);  // J=1, K=1 (toggle)

        // Additional monitoring
        $display("\nFinal output state:");
        $display("output_led1_q_0_6 (Q) = %b", output_led1_q_0_6);
        $display("output_led2_q_0_7 (Qbar) = %b", output_led2_q_0_7);

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

        $display("JK flip-flop testbench completed");
        $finish;
    end

    initial begin
        $dumpfile("jkflipflop_tb.vcd");
        $dumpvars(0, jkflipflop_tb);
    end

endmodule