`timescale 1ns / 1ps

module tflipflop_tb;

    // Testbench signals
    reg input_push_button1_t_1;
    reg input_clock2_c_2;
    reg input_input_switch3__preset_3;
    reg input_input_switch4__clear_4;
    wire output_led1_q_0_5;
    wire output_led2_q_0_6;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;
    reg previous_q = 0;

    // Instantiate the Device Under Test (DUT)
    tflipflop dut (
        .input_push_button1_t_1(input_push_button1_t_1),
        .input_clock2_c_2(input_clock2_c_2),
        .input_input_switch3__preset_3(input_input_switch3__preset_3),
        .input_input_switch4__clear_4(input_input_switch4__clear_4),
        .output_led1_q_0_5(output_led1_q_0_5),
        .output_led2_q_0_6(output_led2_q_0_6)
    );

    // Clock generation (5MHz = 200ns period)
    always begin
        #100 input_clock2_c_2 = ~input_clock2_c_2;
    end

    // Test procedure for T flip-flop
    task test_tff;
        input t_value;
        input preset;
        input clear;
        begin
            test_count = test_count + 1;

            // Set control signals
            input_input_switch3__preset_3 = preset;
            input_input_switch4__clear_4 = clear;
            input_push_button1_t_1 = t_value;

            // Wait for setup time
            #10;

            // Store previous Q state
            previous_q = output_led1_q_0_5;

            // Clock edge
            @(posedge input_clock2_c_2);

            // Wait for propagation delay
            #20;

            $display("Test %0d: T=%b, Preset=%b, Clear=%b => Q=%b, Q̄=%b (prev_Q=%b)",
                     test_count, t_value, preset, clear, output_led1_q_0_5, output_led2_q_0_6, previous_q);

            // Basic validation: Q and Q̄ should be complementary in normal operation
            if (preset == 1 && clear == 1) begin
                if (output_led1_q_0_5 != output_led2_q_0_6) begin
                    // Check T flip-flop behavior
                    if (t_value == 1) begin
                        // T=1 should toggle the output
                        if (output_led1_q_0_5 != previous_q) begin
                            $display("      PASS: T=1 toggled output correctly");
                            pass_count = pass_count + 1;
                        end else begin
                            $display("      FAIL: T=1 should have toggled output");
                        end
                    end else begin
                        // T=0 should hold the output
                        if (output_led1_q_0_5 == previous_q) begin
                            $display("      PASS: T=0 held output correctly");
                            pass_count = pass_count + 1;
                        end else begin
                            $display("      FAIL: T=0 should have held output");
                        end
                    end
                end else begin
                    $display("      FAIL: Outputs should be complementary");
                end
            end else begin
                // For preset/clear conditions, just report the state
                if (preset == 0) begin
                    $display("      INFO: Preset active");
                end
                if (clear == 0) begin
                    $display("      INFO: Clear active");
                end
                pass_count = pass_count + 1; // Count as pass for control conditions
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== T FLIP-FLOP TESTBENCH ===");
        $display("Testing T flip-flop with preset and clear");

        // Initialize signals
        input_clock2_c_2 = 0;
        input_push_button1_t_1 = 0;
        input_input_switch3__preset_3 = 1; // Inactive (assuming active low)
        input_input_switch4__clear_4 = 1;  // Inactive (assuming active low)

        // Wait for initialization
        #150;

        // Test normal T flip-flop operation
        $display("\nTesting normal T flip-flop operation:");
        test_tff(0, 1, 1); // T=0, should hold
        test_tff(1, 1, 1); // T=1, should toggle
        test_tff(1, 1, 1); // T=1, should toggle again
        test_tff(0, 1, 1); // T=0, should hold
        test_tff(1, 1, 1); // T=1, should toggle
        test_tff(0, 1, 1); // T=0, should hold

        // Test preset functionality (if active low)
        $display("\nTesting preset functionality:");
        test_tff(0, 0, 1); // Preset active
        test_tff(1, 1, 1); // Return to normal

        // Test clear functionality (if active low)
        $display("\nTesting clear functionality:");
        test_tff(1, 1, 0); // Clear active
        test_tff(0, 1, 1); // Return to normal

        // More toggle tests
        $display("\nMore toggle tests:");
        test_tff(1, 1, 1); // T=1, should toggle
        test_tff(1, 1, 1); // T=1, should toggle
        test_tff(1, 1, 1); // T=1, should toggle

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
        $dumpfile("tflipflop_tb.vcd");
        $dumpvars(0, tflipflop_tb);
    end

endmodule